#ifndef _LEV_LOOP_H
#define _LEV_LOOP_H
#include "sock.hh"

namespace lev {
	class IOLoop;

	class IIOLoop : public Object {
	protected:;
		Handle &gethandle(ISocket *sock);
	public:;
		IIOLoop(Object *o) : Object(o) {};
		IOLoop *add(ISocket *sock);
		IOLoop *del(ISocket *sock);
		IOLoop *enable_read(ISocket *s);
		IOLoop *disable_read(ISocket *s);
		IOLoop *enable_write(ISocket *s);
		IOLoop *disable_write(ISocket *s);
		u64 poll(int timeout);
	};
}

#ifdef __unix__
#include "loop_posix.hh"
#elif defined(_WIN32)
#include "loop_win32.hh"
#else
#error "No poll() implementation for the target operating system"
#endif

#endif
