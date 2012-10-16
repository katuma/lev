#define _LEV_ADDR_POSIX_H

#include <sys/types.h>
#include <sys/socket.h>

namespace lev {
	// a posix socket address
	class ISockAddr : public IAddr {
	public:;
		inline ISockAddr(const short family) {
			sa.sa_family = family;
		};
		inline ISockAddr(const struct sockaddr &s) {
			sa = s;
		};
		inline IAddr *clone() {
			IAddr *ia = new ISockAddr(this->sa);
			ia->linkto(this);
			return ia;
		};
		void decode(const String &s);
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
