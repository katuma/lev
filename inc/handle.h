#ifndef _LEV_HANDLE_H
#define _LEV_HANDLE_H

namespace lev {
	// This class shields the socket-is-a-fd property, hopefully
	// making our life easier on stupid systems where this invariant
	// doesn't hold.
	//
	// A fair word of warning: NEVER EVER DO "new Handle". Always
	// use some sort of static initialization (typically class member).
	struct Handle;

	struct IHandle {
		int close();
		int closesocket();
		int socket(int domain, int type, int proto);
		int pipe(Handle *h);
		int socketpair(Handle *h, int d, int t, int p);
		int bind(ISockAddr *addr);
		int connect(ISockAddr *addr);
	};
}

#ifdef __unix__
#include "handle_posix.h"
#elif defined(_WIN32)
#include "handle_win32.h"
#else
#error "No socket/files implementation for the target operating system"
#endif

#endif
