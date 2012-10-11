#ifndef _LEV_ADDR_H
#define _LEV_ADDR_H

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "obj.h"

namespace lev {
    using namespace std;
	// Socket address handling

	// An arbitrary address interface
	class IAddr : public Object {
	public:;
		//inline IAddr() : Object(0) {};
		virtual void decode(string s) = 0;
		// cast to covariants
		//template <class T> inline operator T*() { return static_cast<T*>(this); };
	};
}

#ifdef __unix__
#include "addr_posix.h"
#elif defined(_WIN32)
#include "addr_win32.h"
#else
#error "No `struct sockaddr` for the target operating system"
#endif

#endif
