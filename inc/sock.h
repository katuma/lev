#include "addr.h"
#include "handle.h"

namespace lev {
	//
	// Interface ISocket and it's implementations
	// Actual sockets
	//
	class IOPoll;

	// An arbitrary socket interface
	class ISocket : public Object {
	protected:;
		inline ISocket(Object *parent) : Object(parent) {};
	public:;
		Handle h;
		// bind an address
		// optional
		virtual ISocket *bind(string host) = 0;
		virtual ISocket *bind(string host, int port) = 0;
		// mandatory
		virtual ISocket *bind(IAddr *a) = 0;
		virtual void poll(IOPoll *, bool, bool);
	};
	
	
	// An internet socket
	class InetSocket : public ISocket {
		virtual ISocket *bind(IAddr *a) {
			assert(dynamic_cast<ISockAddr*>(a));
			ISockAddr *sa = (ISockAddr*)a;
			h.bind(sa);
			/*if (h.bind() {
				emit(ErrorEvent);
			} else {
				emit(BoundEvent, sin);
			}*/
		}
	};
}
