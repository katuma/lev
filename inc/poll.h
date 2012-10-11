#ifndef _LEV_POLL_H
#define _LEV_POLL_H
#include "sock.h"


namespace lev {
	class IOPoll;
	class IIOPoll : public Object {
	public:;
		inline IIOPoll(Object *o) : Object(o) {};
		IOPoll *add(ISocket *sock);
		IOPoll *del(ISocket *sock);
		IOPoll *enable_read(ISocket *s);
		IOPoll *disable_read(ISocket *s);
		IOPoll *enable_write(ISocket *s);
		IOPoll *disable_write(ISocket *s);
		u64 poll(int timeout);
	};
}

#ifdef __unix__
#include "poll_posix.h"
#elif defined(_WIN32)
#include "poll_win32.h"
#else
#error "No poll() implementation for the target operating system"
#endif

#endif
