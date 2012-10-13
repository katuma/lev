#ifndef _LEV_SOCK_H
#define _LEV_SOCK_H
#include "addr.h"
#include "handle.h"

namespace lev {
	//
	// Interface ISocket and it's implementations
	// Actual sockets
	//
	class IOPoll;
	
	enum EventType {
		FlushEvent,
		ErrorEvent,
	};
	
	typedef std::function<void(ISocket *, ...)> EventCB;
	
	class SockEvent : public List {
	public:;
		inline SockEvent(EventType et) : t(et), List();
		EventType type;
		EventCB cb;
		inline SockEvent *setcb(EventCB c) {
			cb = c;
			return this;
		}
	}
	
	class EventDispatcher {
	public:;
		List handlers; // list of event handlers
		SockEvent *add_handler(EventType et, EventCB cb) {
			SockEvent *e = new SockEvent(et);
			e->linkto(&this->handlers);
			return e->setcb((cb));
		}		
	}

	enum SockFlags {
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
		inline ISocket() : flags(0) {};

		SockFlags flags;
	public:;
		virtual ~ISocket();

		virtual ISocket *bind(IAddr *a) = 0;
		virtual ISocket *connect(IAddr *a) = 0;

		virtual void poll(IOLoop *io, bool r, bool w);
		
		// event handlers. switch to virtual only if needed.
		virtual void on_data();
		virtual void on_read(IOPoll *);
		virtual void on_write(IOPoll *);
		virtual void on_flush(IOPoll *);
		virtual void on_error(string *, int errno);
		virtual void on_close();
		virtual int recv(IOPoll *, u8 *packet, u32 *len, string **msg);
		virtual int send(IOPoll *, u8 *packet, u32 *len, string **msg);
	};

	class InetSocket : public virtual ISocket {
	protected:;
		Handle h;
	public:;
		// bind address
		ISocket *bind(IAddr *a);
		ISocket *connect(IAddr *a);

		int send(IOPoll *, Buffer *buf, u32 *len);
		int recv(IOPoll *, Buffer *buf, u32 *len);
		void on_error(string *, int errno);
		void on_close();
		void ~InetSocket();
	};

	class IBufferedSocket : public virtual ISocket {
	protected:;
		const int READ_CHUNK = 4096;
		Buffer input;
		Buffer output;
	public:;
		void on_read(IOPoll *);
		void on_write(IOPoll *);
	}
	
	class TCPSocket : public InetSocket, public IBufferedSocket {
	public:;
		void on_data(IOPoll *);
		void on_flush(IOPoll *);
		int recv(IOPoll *, u8 *packet, u32 *len, string **msg);
		int send(IOPoll *, u8 *packet, u32 *len, string **msg);
	}
}
#endif

