#include "sock.h"
#include "poll.h"
#include "buf.h"

namespace lev {
	
	// class ISocket

	void ISocket::poll(IOPoll *io, bool r, bool w) {
		if (r) on_read(io);
		if (w) on_write(io);
	}


	// class InetSocket


	// bind address
	ISocket *InetSocket::bind(IAddr *a) {
		assert(dynamic_cast<ISockAddr*>(a));
		ISockAddr *sa = (ISockAddr*)a;
		if (h.bind(*sa))
			setflag(ERROR);
		else addflag(BOUND);
		return this;
	}

	// connect to an address
	ISocket *InetSocket::connect(IAddr &a) {
		//assert(flags&(LISTENING|CONNECTING|CONNECTING2)==0);
		assert(dynamic_cast<ISockAddr*>(&a));
		ISockAddr *sa = (ISockAddr*)&a;
		if (h.connect(*sa)) {
			if (errno == EINPROGRESS) {
				addflag(CONNECTING);
			}
			setflag(ERROR);
			flags = ERROR;
		} else addflag(CONNECTING2);
		return this;
	}

	// socket is being destroyed
	bool InetSocket::on_delete(Object *parent) {
		if (on_close(parent)) {
			h.close();
			return ISocket::on_delete(parent);
		}
		return false;
	}


	// class TCPSocket
	int _TCPSocket::send(IOPoll *io, u8 *packet, u32 *len, String &msg) {
		if (!*len) {
			io->disable_write(this);
			return 0;
		}
		int err = h.send(packet, len);
		if (err == EWOULDBLOCK) {
			io->enable_write(this);
			return 0;
		}
		if (!*len)
			err = ECONNRESET;
		if (err) h.errnostr(err, msg);
		return err;
	}
	int _TCPSocket::recv(IOPoll *io, u8 *packet, u32 *len, String &msg) {
		int err = h.recv(packet, len);
		if (err == EWOULDBLOCK)
			return 0;
		if (!*len)
			err = ECONNRESET;
		if (err) h.errnostr(err, msg);
		return err;
	}
}
