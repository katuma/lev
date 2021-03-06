#define _LEV_ADDR_POSIX_H

#include <sys/types.h>
#include <sys/socket.h>

#include "types.hh"

namespace lev {
	// a posix socket address
	class ISockAddr : public IAddr {
	public:
		ISockAddr() {
		}

		ISockAddr(const short family) {
			sa.sa_family = family;
		}

		ISockAddr(const struct sockaddr &s) {
			sa = s;
		}

		void decode(const String &s) {
		}

		struct sockaddr sa;
	};

	// an ipv4 address / port pair
	class IPv4 : public ISockAddr {
	public:;
		IPv4(u32 ip, u16 port) : ISockAddr(AF_INET) {
			struct sockaddr_in *sin = (struct sockaddr_in*)&sa;
			sin->sin_addr.s_addr = ip;
			sin->sin_port = htons(port);
		};
	};
}
