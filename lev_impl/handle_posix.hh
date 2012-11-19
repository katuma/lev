#define _LEV_HANDLE_POSIX_H

#include "addr.hh"
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "buf.hh"

namespace lev {
	// This class shields the socket-is-a-fd property, hopefully
	// making our life easier on stupid systems where this invariant
	// doesn't hold.
	//
	// A fair word of warning: NEVER EVER DO "new Handle". Always
	// use some sort of static initialization (typically class member).
	struct Handle : IHandle {
	public:;
		int fd;
		
		Handle() : fd(-1) {};
		Handle(int f) : fd(f) {};
		
		~Handle() {
			if (fd >= 0)
				close();
			fd = -1;
		};
		
		int close() {
			assert(fd>=0);
			int res = ::close(fd);
			fd = -1;
			return res<0?errno:0;
		};
		
		int closesocket() {
			return close();
		};
		
		int socket(const int domain, const int type, const int proto) {
			assert(fd<0);
			fd = ::socket(domain, type, proto);
			return fd<0?errno:0;
		};
		
		// pipes are a bit obnoxious:
		// Handle r, w;
		// r.pipe(&w)
		int pipe(Handle *h) {
			Handle x;
			assert(fd<0);
			assert(h->fd<0);
			int f[2];
			int res = ::pipe(f);
			if (res<0)
				return errno;
			fd = f[0];
			h->fd = f[1];
			return 0;
		};
		
		int socketpair(Handle *h, const int d, const int t, const int p) {
			assert(fd<0);
			assert(h->fd<0);
			int f[2];
			int res = ::socketpair(d, t, p, f);
			if (res<0)
				return errno;
			fd = f[0];
			h->fd = f[1];
			return 0;
		};
		
		int bind(ISockAddr *sa) {
			assert(fd>=0);
			::bind(fd, &sa->sa, sizeof(sa->sa));
			return fd<0?errno:0;
		};

		int connect(const ISockAddr &sa) {
			assert(fd>=0);
			::connect(fd, &sa.sa, sizeof(sa.sa));
			return fd<0?errno:0;
		};


		int setblocking(const bool block) {
			assert(fd>=0);
			int flags = ::fcntl(fd, F_GETFL, 0);
			if (flags<0)
				return errno;
			if (block)
				flags |= O_NONBLOCK;
			else
				flags &= ~O_NONBLOCK;
			return fcntl(fd, F_SETFL, flags)?errno:0;
		};
		
		void errnostr(const int err, String *str) {
			*str = ::strerror(err);
		}
		
		int send(const u8 *buf, u32 *len) {
			int res;
			if ((res = ::send(fd, buf, *len, 0)) < 0) {
				return errno;
			}
			*len = res;
			return 0;
		}

		int sendto(const u8 *buf, u32 *len, const ISockAddr &sa) {
			int res;
			if ((res = ::sendto(fd, buf, *len, 0, &sa.sa, sizeof(sa))) < 0) {
				return errno;
			}
			*len = res;
			return 0;
		}


		int recv(u8 *buf, u32 *len) {
			int res;
			if ((res = ::recv(fd, buf, *len, 0)) < 0) {
				return errno;
			}
			*len = res;
			return 0;
		}

		int recvfrom(u8 *buf, u32 *len, ISockAddr *ia) {
			int res;
			socklen_t salen = sizeof(ia->sa);
			if ((res = ::recvfrom(fd, buf, *len, 0, &ia->sa, &salen)) < 0) {
				return errno;
			}
			*len = res;
			return 0;
		}
	};
}


