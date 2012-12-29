#ifndef _LEV_SOCK_H
#define _LEV_SOCK_H

#include "auto.hh"
#include "addr.hh"
#include "handle.hh"
#include "buf.hh"


namespace lev {
	class IOLoop;
	class ISocket;
	
	enum SockFlags {
		NONE,
		ERROR, // an error occured, other flags should be cleared
		PAUSED, // wait for resume()
		BOUND, // socket bound successfuly, one-shot
		LISTENING, // a server socket
		CONNECTING , // connecting client
		CONNECTED, // connected client, after callback
		CLOSED, // after close
		DELETED,
	};

	// A socket interface
	class ISocket {
	public:;
#ifndef LOCAL_IOLOOP
		static
#endif
		IOLoop *io;
		inline ISocket() : flags(NONE) {};
		u32 flags;
		virtual ~ISocket() {};

		// bind/connect protocol address
		virtual ISocket *bind(IAddr *) = 0;
		virtual ISocket *connect(const IAddr &) = 0;
		virtual ISocket *bind(const char *, int) = 0;
		virtual ISocket *connect(const char *, int) = 0;

		// pause/resume reading
		virtual ISocket *pause() = 0;
		virtual ISocket *resume() = 0;

		// perform socket close - unless closed already
		inline void close() {
			if (pre_close()) return;
			setflag(CLOSED);
			on_close();
			post_close();
		};

		// perform socket error
		inline void error(const int err, void *data)
		{
			on_error(err, data);
		}

		inline void error(const int err)
		{
			error(err, 0);
		}

		void error(const char *fmt, ...) {
			char buf[512];
			va_list ap;
			va_start(ap, fmt);
			vsprintf(buf, fmt, ap);
			va_end(ap);
			on_error(0, buf);
		}

		// received (buffered) data
		virtual void on_data(const u8 *, u32, const IAddr *) {};

		// received read event
		virtual void on_read() {};

		// received connect event
		virtual void on_connect() {};

		// received write event
		virtual void on_write() {};

		// wrote (buffered) output data
		virtual void on_flush() {};

		virtual const char *errstr(const int err) {
			return 0;
		}

		// close the socket - noop by default
		virtual void on_close() { }
		virtual bool pre_close() { return hasflag(CLOSED); }
		virtual void post_close() { delete this; }

		// an error occured, kill the socket by default
		virtual void on_error(const int err, void *arg) {
			return close();
		}

		// poll for events
		virtual void poll(bool r, bool w) {
			if (r && !hasflag(PAUSED)) on_read();
			if (w) on_write();
		}
		
		// receive raw data
		virtual int recv(u8 *, uint *) { return 0; };

		// send raw data
		virtual int send(const u8 *, uint *) { return 0; };

		// flags
		inline void setflags(u32 f) {
			flags = f;
		}
		inline void setflag(const SockFlags f) {
			flags |= 1<<f;
		}
		inline bool hasflag(const SockFlags f) {
			return (flags & (1<<f))!=0;
		}
		inline void clearflag(const SockFlags f) {
			flags &= ~(1<<f);
		}
	};

}
#ifndef LOCAL_IOLOOP
lev::IOLoop *lev::ISocket::io = 0;
#endif

#include "loop.hh"

namespace lev {

	class InetSocket : public ISocket {
	protected:;
	public:;
		Handle h;
		inline InetSocket(int domain, int type, int proto) : ISocket() {
			h.socket(domain, type, proto);
		};
		inline InetSocket(Handle &nh) : ISocket(), h(nh) {};
		inline InetSocket *listen(const int n) {
			h.listen(n);
			return this;
		}
		inline InetSocket *listen() {
			return listen(128);
		}

		const char *errstr(const int err) {
			return h.errnostr(err);
		}

		ISocket *bind(const char *ip, int port) {
			// TBD
			return this;
		}
		ISocket *connect(const char *ip, int port) {
			// TBD
			return this;
		}

		ISocket *bind(IAddr *a) {
			assert(dynamic_cast<ISockAddr*>(a));
			if (h.bind(static_cast<ISockAddr*>(a)))
				setflag(ERROR);
			else setflag(BOUND);
			return this;
		}
		ISocket *connect(const IAddr &a) {
			//assert(flags&(LISTENING|CONNECTING|CONNECTING2)==0);
			assert(dynamic_cast<const ISockAddr*>(&a));
			if (h.connect(static_cast<const ISockAddr&>(a))) {
				if (errno == EINPROGRESS) {
					io->enable_write(this);
					setflag(CONNECTING);
				} else setflag(ERROR);
			} else setflag(CONNECTED);
			return this;
		}

		// pause/resume
		virtual ISocket *pause() {
			setflag(PAUSED);
			io->disable_read(this);
			return this;
		}

		virtual ISocket *resume() {
			clearflag(PAUSED);
			io->enable_read(this);
			return this;
		}

		// write event
		void on_write() {
			if (hasflag(CONNECTING)) {
				clearflag(CONNECTING);
				setflag(CONNECTED);
				ISocket::on_connect();
			}
		}

		// socket is being closed
		inline bool pre_close()
	   	{
			if (hasflag(CLOSED)) return true;
			io->del(this);
			h.close();
			return false;
		}

		~InetSocket() {
			assert(!hasflag(DELETED));
			setflag(DELETED);
			close();
		}
	};
	
	class TCPSocket : public InetSocket {
	public:;
		inline TCPSocket(Handle &nh) : InetSocket(nh) {};
		inline TCPSocket() : InetSocket(AF_INET, IPPROTO_TCP, SOCK_STREAM) {};
		void on_data(const u8 *, u32, const IAddr *) {};
		void on_flush() {};

		int recv(u8 *packet, uint *len) {
			int err = h.recv(packet, len);
			if (h.temperr(err))
				return 0;
			if (!err && !*len)
				err = ECONNRESET;
			return err;
		}

		int send(const u8 *packet, uint *len) {
			if (!*len) {
				io->disable_write(this);
				return 0;
			}
			int err = h.send(packet, len);
			if (h.temperr(err)) {
				io->enable_write(this);
				return 0;
			}
			if (!err && !*len)
				err = ECONNRESET;
			return err;
		}
	};

	template <class ClientBase>	
	class TCPServer : public ClientBase {
	public:;
		using ClientBase::h;
		using ClientBase::error;
		virtual void on_client(ClientBase *, ISockAddr &) = 0;
		inline TCPServer() : ClientBase() {};
		// a bit of misnomer - when we're ready to accept client
		void on_read() {
			ISockAddr sa;
			Handle newh;
			do {
				ClientBase *nc;
				if (int err = h.accept(&sa, &newh)) {
					if (h.temperr(err)) break;
					return error(err);
				}
				nc = new ClientBase();
				nc->h = newh;
				on_client(nc, sa);
			} while (1);
		}
	};

	class UDPSocket : public InetSocket {
		int send(const u8 *packet, uint *len) {
			return h.send(packet, len);
		}
		int sendto(const u8 *packet, uint *len, const IAddr &sa) {
			assert(dynamic_cast<const ISockAddr*>(&sa));
			return h.sendto(packet, len, static_cast<const ISockAddr&>(sa));
		}
		void on_read() {
			u8 buf[65536];
			u32 len = sizeof(buf);
			ISockAddr ia;
			int err;
			if ((err = h.recvfrom(buf, &len, &ia)))
				return error(err);
			return on_data(buf, err, &ia);
		}
	};

	template <class Base>
	class Buffered : public Base {
	public:;
		static const int READ_CHUNK = 4096;
		Buffer input;
		Buffer output;

		using Base::error;
		using Base::on_data;
		using Base::on_flush;
		using Base::send;
		using Base::recv;
		using Base::hasflag;
		using Base::setflag;
		using Base::clearflag;
		void flush() {
			if (!output.empty())
				on_write();
		}
		void on_read() {
			uint len;
			if (int err = recv(input.tail(&len, READ_CHUNK), &len))
				return error(err);
			on_data(input.head(), input.bytes(), 0);
			flush();
		}
		void on_write() {
			uint len;
			bool wasempty = output.empty();
			if (hasflag(CONNECTING)) {
				Base::on_write(); // on_connect pass through ..
				if (wasempty) return;
			}
			if (int err = send(output.head(&len), &len))
				return error(err);
			output.consume(len);
            // transitioned to empty -> notify owner data have been flushed
			if (!wasempty && output.empty())
				on_flush();			
		}
	};

	// hackety hack - breaks ISocket <-> IOLoop circular dependency
	inline Handle &IIOLoop::gethandle(ISocket *sock) {
		return ((InetSocket *) sock)->h;
	}

	template <class Base>
	class Timeout : public Base {
public:;
	};

	typedef Buffered<TCPSocket> BTCPSocket;
	typedef Timeout<TCPSocket> TOTCPSocket;
	typedef Buffered<TOTCPSocket> BTOTCPSocket;
}
#endif

