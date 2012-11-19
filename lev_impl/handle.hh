#ifndef _LEV_HANDLE_H
#define _LEV_HANDLE_H

#include <string>
#include "buf.hh"

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
		int socket(const int domain, const int type, const int proto);
		int pipe(Handle *h);
		int socketpair(Handle *h, const int d, const int t, const int p);
		int bind(ISockAddr *addr);
		int connect(const ISockAddr &addr);
		int recv(u8 *buf, u32 *len);
		int send(const u8 *buf, u32 *len);
		int sendto(const u8 *buf, u32 *len, const ISockAddr &);
		void errnostr(const int, String *s);
	};
}

#ifdef __unix__
#include "handle_posix.hh"
#elif defined(_WIN32)
#include "handle_win32.hh"
#else
#error "No socket/files implementation for the target operating system"
#endif

#endif
