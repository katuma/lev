#define _LEV_POLL_POSIX_H

//#include <unordered_map>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>

#include "sock.h"


namespace lev {
	// A posix-compliant, hybrid cross-platform select()/poll().
	// select() uses hacked FD_SETSIZE. If kernel returns an error
	// other than EINTR & friends, we'll switch to poll.
	class IOPoll : public IIOPoll {
	private:;
		// XXX benchmark if u64 is actually faster than u32 on amd64?
		typedef unsigned long WORDTYPE;
		const int WORDBITS = sizeof(WORDTYPE) * 8;
		// poll() makes more sense after this (>4kb fdsets).
		const int SELECT_CUTOFF = 16384;
		// recompute pfds every 1ms
		const int PFDS_THROTTLE = 1;

		bool dirty;
		bool usepoll;

		int maxfd;
		u64 pfdlast;

		Vector<bool> rset, wset, rres, wres;
		Vector<struct pollfd> pfds;
		//unordered_map<int, ISocket *> sockmap;
		Vector<ISocket *> sockmap;
		
		inline void _recompute_pfds() {
			if (now < (pfdlast+PFDS_THROTTLE)) return;
			pfdlast = now;
			pfds.clear();
			for (int i = 0; i < maxfd; i++)
				if (short events = _calcevents(i)) {
					struct pollfd pf = {
						.fd = i,
						.events = events,
						.revents = 0
					};
					pfds.push_back(pf);
				}
			dirty = false;
		};

		inline short _calcevents(int fd) {
			u32 events;
			if (rset[fd])
				events = POLLIN;
			if (wset[fd])
				events |= POLLOUT;
			if (!events) return 0;
			return events | POLLERR | POLLHUP;
		};

		// Modify requested poll type. This MUST be O(1), as it
		// happens a lot, hence the heavy hackery.
		inline IOPoll *_enable(Vector<bool> &set, int fd) {
			if (!set[fd])
				set.setbit(fd, dirty = true);
			return this;
		};

		inline IOPoll *_disable(Vector<bool> &set, int fd) {
			if (!set[fd]) {
				set.setbit(fd, false);
				dirty = true;
			}
			return this;
		};

		inline u64 _poll_poll(int timeout) {
			if (dirty)
				_recompute_pfds();
			int n = pfds.size();
			if (int res = ::poll(&pfds.front(), n, timeout)>=0) {
				_getnow();
				for (int i = 0; res > 0 && i < n; i++) {
					struct pollfd *pfd = &pfds[i];
					
					res -= !!pfd->revents;
					pfd->events = _calcevents(pfd->fd);
					pfd->revents &= (pfd->events|POLLNVAL);
					bool r = pfd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL);
					bool w = pfd->revents & (POLLOUT|POLLERR|POLLHUP|POLLNVAL);

					if (r|w)
						sockmap[pfd->fd]->poll(this, r, w);
				}
				return now;
			}
			return 0;
		};
		// poll for fds, runs callbacks, returns current timestamp.
		// timeout is in milliseconds.
		inline u64 _poll_select(int timeout) {
			int res;
			
			struct timeval tv;
			tv.tv_sec = timeout/1000;
			tv.tv_usec = (timeout%1000)*1000;
			
			// XXX memcpy?
			rres = rset;
			wres = wset;

			if ((res = ::select(maxfd, (fd_set*)rres.buf, (fd_set*)wres.buf, 0, &tv))>=0) {
				_getnow();
				for (int fd = 0; res > 0 && fd < maxfd; fd++) {
					bool r = rres[fd];
					bool w = wres[fd];
					if (r|w) {
						res--;
						sockmap[fd]->poll(this, r, w);
					}
				}
				return now;
			}
			
			// fallback
			if (errno != EINTR && errno != ETIMEDOUT && errno != EAGAIN) {
				usepoll = true;
				return poll(timeout);
			}
			return 0;
		};

		inline void _getnow() {
			struct timeval tv;
			gettimeofday(&tv, 0);
			now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		};
		inline int getfd(ISocket *sock) {
			return ((InetSocket *) sock)->h.fd;
		}
	public:;
		u64 now;
		inline IOPoll(Object *o) :
			IIOPoll(o),
			dirty(false),
			usepoll(false),
			maxfd(0) {};

		// register fd
		inline IOPoll *add(ISocket *sock) {
			int fd = getfd(sock);
			assert(fd>=0);
			sockmap[fd] = sock;
			if (++fd > maxfd) {
				maxfd = fd;
				rset.resize(maxfd, false);
				wset.resize(maxfd, false);
				sockmap.reserve(maxfd);
			}
			return this;
		};


		inline IOPoll *del(ISocket *sock) {
			//sockmap.erase(getfd(sock));
			disable_read(sock);
			disable_write(sock);
			return this;
		};


		// NOTE: these are not a single monolithic function with boolean
		// flags because that would just trash the branch predictor.
		inline IOPoll *enable_read(ISocket *s) {
			return _enable(rset, getfd(s));
		};

		inline IOPoll *disable_read(ISocket *s) {
			return _disable(rset, getfd(s));
		};

		inline IOPoll *enable_write(ISocket *s) {
			return _enable(wset, getfd(s));
		};

		inline IOPoll *disable_write(ISocket *s) {
			return _disable(wset, getfd(s));
		};
		
		u64 poll(int timeout);
	};
}

