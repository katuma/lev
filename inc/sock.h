#ifndef _LEV_SOCK_H
#define _LEV_SOCK_H

#include <functional>
#include "addr.h"
#include "handle.h"
#include "buf.h"


namespace lev {
	//
	// Interface ISocket and it's implementations
	// Actual sockets
	//
	class IOPoll;
	class ISocket;
	
	enum EventType {
		FlushEvent,
		ErrorEvent,
	};
	
	typedef std::function<void(ISocket *, ...)> EventCB;
	
	class SockEvent : public List {
	public:;
		inline SockEvent(EventType et) : List(), type(et) {};
		EventType type;
		EventCB *cb;
		inline SockEvent *setcb(EventCB *c) {
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
	};

	enum SockFlags {
		NONE = 0,
		ERROR = 1, // an error occured, other flags should be cleared
		CANREAD = 2, // received via ->poll, ->run can read input data
		PAUSED = 4, // ->pause() was called
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
		CANWRITE = 8, 
		BOUND = 16, // socket bound successfuly, one-shot
		LISTENING = 32, // a server socket
		CONNECTING = 64, // connecting client
		CONNECTING2 = 128, // connected client, but callback not called
		CONNECTED = 256, // connected client, after callback
	};

	// An arbitrary socket interface
	class ISocket : public Object {

	protected:;
		inline ISocket() : flags(NONE) {};

		SockFlags flags;
	public:;
		virtual ~ISocket();

		virtual ISocket *bind(IAddr *) = 0;
		virtual ISocket *connect(IAddr &) = 0;

		virtual void poll(IOPoll *io, bool r, bool w);
		
		// event handlers. switch to virtual only if needed.
		virtual void on_data(IOPoll *) = 0;
		virtual void on_read(IOPoll *) = 0;
		virtual void on_write(IOPoll *) = 0;
		virtual void on_flush(IOPoll *) = 0;
		virtual void on_error(IOPoll *, String &, const int) = 0;
		virtual bool on_close(Object *) = 0;
		virtual int recv(IOPoll *, u8 *, uint *, String &) = 0;
		virtual int send(IOPoll *, u8 *, uint *, String &) = 0;
		inline void setflag(const SockFlags f) {
			flags = f;
		}
		inline void addflag(const SockFlags f) {
			flags = (SockFlags)(f | flags);
		}
	};

	class InetSocket : public ISocket {
	protected:;
	public:;
		Handle h;
		// bind address
		ISocket *bind(IAddr *a);
		ISocket *connect(IAddr &a);
		//using ISocket::bind;
		using ISocket::connect;

//		int send(IOPoll *, Buffer *buf, uint *len, String *msg);
//		int recv(IOPoll *, Buffer *buf, uint *len, String *msg);

		void on_error(IOPoll *, String &, const int);
		bool on_close(Object *);
		bool on_delete(Object *);
//		using ISocket::on_error;
//		using ISocket::on_close;
//		using ISocket::on_delete;
		~InetSocket();
	};
	
	class _TCPSocket : public InetSocket {
	public:;
		void on_data(IOPoll *);
		void on_flush(IOPoll *);
//		using ISocket::on_data;
//		using ISocket::on_flush;
//		using ISocket::recv;
//		using ISocket::send;
		int recv(IOPoll *, u8 *, uint *, String &);
		int send(IOPoll *, u8 *, uint *, String &);
	};

	template <class Base>
	class TBufferedSocket : public Base {
	protected:;
		static const int READ_CHUNK = 4096;
		Buffer input;
		Buffer output;
	public:;
		using Base::on_error;
		using Base::on_data;
		using Base::on_flush;
		void on_read(IOPoll *io) {
			String s;
			uint len;
			if (int err = recv(io, input.tail(&len, READ_CHUNK), &len, &s))
				return on_error(io, s, err);
			return on_data(io);	
		}
		void on_write(IOPoll *io) {
			uint len;
			String s;
			if (int err = send(io, output.head(&len), &len, &s))
				return on_error(io, s, err);
			if (output.empty())
				on_flush(io);			
		}
	};
	
	typedef TBufferedSocket<_TCPSocket> TCPSocket;
}
#endif

