#include "poll.h"

#ifdef _LEV_POLL_POSIX_H
namespace lev {
	u64 IOPoll::poll(int timeout) {
		if (usepoll)
			return _poll_poll(timeout);
		return _poll_select(timeout);
	}
}
#endif
