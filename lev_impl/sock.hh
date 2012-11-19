#ifndef _LEV_SOCK_H
#define _LEV_SOCK_H

#include "addr.hh"
#include "handle.hh"
#include "buf.hh"


namespace lev {
	class IOLoop;
	class ISocket;
	
	enum EventType {
		FlushEvent,
		ErrorEvent,
	};

/*
	typedef std::function<void(ISocket *, ...)> EventCB;
	
	class SockEvent : public List {
	public:;
		SockEvent(EventType et) : List(), type(et) {};
		EventType type;
		EventCB *cb;
		SockEvent *setcb(EventCB *c) {
			cb = c;
			return this;
		}
	};
	
	class EventDispatcher {
	public:;
		List handlers; // list of event handlers
		SockEvent *add_handler(EventType et, EventCB *cb) {
			SockEvent *e = new SockEvent(et);
			e->linkto(&this->handlers);
			return e->setcb(cb);
		}
	};*/

	enum SockFlags {
		NONE,
		ERROR, // an error occured, other flags should be cleared
		PAUSED, // ->pause() was called
		// we can write w/o EAGAIN.
		// On write():
		// 1. if this flag is not set, buffers the write
		// 2. if flag is set, but gets EAGAIN, clears the flag and enable_write
		//
		// ->poll():
		//    on writeable socket put up this flag
		// ->run():
		//    if flag is set, this try to flush the buffer. on egain do the same
		//    as write()
		BOUND, // socket bound successfuly, one-shot
		LISTENING, // a server socket
		CONNECTING , // connecting client

		CONNECTED, // connected client, after callback
	};

	// An arbitrary socket interface
	class ISocket : public Object {

	protected:;
		ISocket() : flags(NONE) {};

		u32 flags;
	public:;
		virtual ~ISocket();
		virtual ISocket *bind(IOLoop *, IAddr *) = 0;
		virtual ISocket *connect(IOLoop *, const IAddr &) = 0;

		virtual ISocket *bind(IOLoop *, const String &, int);
		virtual ISocket *connect(IOLoop *, const String &, int);

		virtual void poll(IOLoop *io, bool r, bool w) {
			if (r) on_read(io);
			if (w) on_write(io);
		}
		
		// event handlers. switch to virtual only if needed.
		virtual void on_data(IOLoop *, const u8 *, u32, const IAddr &) = 0;
		virtual void on_read(IOLoop *) = 0;
		virtual void on_write(IOLoop *) = 0;
		virtual void on_flush(IOLoop *) = 0;
		virtual void on_error(IOLoop *, const String &, const int) = 0;
		virtual bool on_close(Object *) = 0;
		virtual int recv(IOLoop *, u8 *, uint *, String *) = 0;
		virtual int send(IOLoop *, const u8 *, uint *, String *) = 0;
		
		int recv(IOLoop *io, u8 *b, uint *len) {
			return recv(io, b, len, 0);
		}
		int send(IOLoop *io, const u8 *b, uint *len) {
			return send(io, b, len, 0);
		}
		void setflags(u32 f) {
			flags = f;
		}
		void setflag(const SockFlags f) {
			flags |= 1<<f;
		}
		bool hasflag(const SockFlags f) {
			return (flags & (1<<f))!=0;
		}
		void clearflag(const SockFlags f) {
			flags &= ~(1<<f);
		}
	};

}

#include "loop.hh"

namespace lev {

	class InetSocket : public ISocket {
	protected:;
	public:;
		Handle h;
		ISocket *bind(IOLoop *, const String &, int);
		ISocket *connect(IOLoop *, const String &, int);

		ISocket *bind(IOLoop *io, IAddr *a) {
			assert(dynamic_cast<ISockAddr*>(a));
			ISockAddr *sa = (ISockAddr*)a;
			if (h.bind(*sa))
				setflag(ERROR);
			else setflag(BOUND);
			return this;
		}
		ISocket *connect(IOLoop *io, const IAddr &a) {
			//assert(flags&(LISTENING|CONNECTING|CONNECTING2)==0);
			assert(dynamic_cast<const ISockAddr*>(&a));
			ISockAddr *sa = (ISockAddr*)&a;
			if (h.connect(*sa)) {
				if (errno == EINPROGRESS) {
					io->enable_write(this);
					setflag(CONNECTING);
				} else setflag(ERROR);
			} else setflag(CONNECTED);
			return this;
		}

//		using ISocket::connect;

		void on_error(IOLoop *, const String &, const int);
		bool on_close(Object *);
		bool on_delete(Object *parent) {
			if (on_close(parent)) {
				h.close();
				// chain-up
				return ISocket::on_delete(parent);
			}
			return false;
		}
		~InetSocket();
	};
	
	class TCPSocket : public InetSocket {
	public:;
		void on_data(IOLoop *, const u8 *, u32, const IAddr &);
		void on_flush(IOLoop *);

		int recv(IOLoop *io, u8 *packet, uint *len, String *msg) {
			int err = h.recv(packet, len);
			if (err == EWOULDBLOCK)
				return 0;
			if (!err && !*len)
				err = ECONNRESET;
			if (err && msg) h.errnostr(err, msg);
			return err;
		}

		int send(IOLoop *io, const u8 *packet, uint *len, String *msg) {
			if (!*len) {
				io->disable_write(this);
				return 0;
			}
			int err = h.send(packet, len);
			if (err == EWOULDBLOCK) {
				io->enable_write(this);
				return 0;
			}
			if (!err && !*len)
				err = ECONNRESET;
			if (err && msg) h.errnostr(err, msg);
			return err;
		}
	};
	
	class TCPServer : public InetSocket {
		// a bit of misnomer - when we're ready to accept client
		void on_write(IOLoop *);
	};

	class UDPSocket : public InetSocket {
		void on_data(IOLoop *, const u8 *, u32, const IAddr &);
		int recv(IOLoop *, u8 *, uint *, String *) = 0; // no-op
		int send(IOLoop *io, const u8 *packet, uint *len, String *msg) {
			int err = h.send(packet, len);
			if (err && msg) h.errnostr(err, msg);
			return err;
		}
		int sendto(IOLoop *io, const u8 *packet, uint *len, const IAddr &sa, String *msg) {
			assert(dynamic_cast<const ISockAddr*>(&sa));
			int err = h.sendto(packet, len, static_cast<const ISockAddr&>(sa));
			if (err && msg) h.errnostr(err, msg);
			return err;
		}
		void on_read(IOLoop *io) {
			u8 buf[65536];
			u32 len = sizeof(buf);
			ISockAddr ia;
			int err = h.recvfrom(buf, &len, &ia);
			if (err) {
				String msg;
				h.errnostr(err, &msg);
				on_error(io, msg, err);
				return;
			}
			return on_data(io, buf, err, ia);
		}
	};

	template <class Base>
	class Buffered : public Base {
	public:;
		using Base::on_error;
		using Base::on_data;
		using Base::on_flush;
		using Base::send;
		using Base::recv;
		using Base::hasflag;
		using Base::setflag;
		using Base::clearflag;
		void flush(IOLoop *io) {
			if (!output.empty())
				on_write(io);
		}
		void on_read(IOLoop *io) {
			String s;
			uint len;
			if (int err = recv(io, input.tail(&len, READ_CHUNK), &len, &s))
				return on_error(io, s, err);
			on_data(io, input.head(), input.bytes(), 0);
			flush();
		}
		void on_write(IOLoop *io) {
			uint len;
			String s;
			bool wasempty = output.empty();
			if (hasflag(CONNECTING)) {
				clearflag(CONNECTING);
				setflag(CONNECTED);
				if (wasempty) return;
			}
			if (int err = send(io, output.head(&len), &len, &s))
				return on_error(io, s, err);
			output.consume(len);
            // transitioned to empty -> notify owner data have been flushed
			if (!wasempty && output.empty())
				on_flush(io);			
		}
	protected:;
		static const int READ_CHUNK = 4096;
		Buffer input;
		Buffer output;
	};

	// hackety hack - breaks ISocket <-> IOLoop circular dependency
	inline Handle &IIOLoop::gethandle(ISocket *sock) {
		return ((InetSocket *) sock)->h;
	}
}
#endif

