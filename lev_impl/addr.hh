#ifndef _LEV_ADDR_H
#define _LEV_ADDR_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "obj.hh"
#include "buf.hh"

namespace lev {
	// Socket address handling

	// An arbitrary address interface
	class IAddr : public Object {
	public:;
		//IAddr() : Object(0) {};
		virtual void decode(const String &s);
		// cast to covariants
		//template <class T> operator T*() { return static_cast<T*>(this); };
	};
}

#ifdef __unix__
#include "addr_posix.hh"
#elif defined(_WIN32)
#include "addr_win32.hh"
#else
#error "No `struct sockaddr` for the target operating system"
#endif

#endif
