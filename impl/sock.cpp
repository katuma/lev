#include "sock.h"
#include "poll.h"
#include "buf.h"
#include "addr.h"

namespace lev {
	
	// class ISocket

	void ISocket::poll(IOPoll *io, bool r, bool w) {
		if (r) on_read(io);
		if (w) on_write(io);
	}


	// class InetSocket


	// bind address
	ISocket *InetSocket::bind(IOPoll *io, IAddr *a) {
		assert(dynamic_cast<ISockAddr*>(a));
		ISockAddr *sa = (ISockAddr*)a;
		if (h.bind(*sa))
			setflag(ERROR);
		else setflag(BOUND);
		return this;
	}

	// connect to an address
	ISocket *InetSocket::connect(IOPoll *io, const IAddr &a) {
		//assert(flags&(LISTENING|CONNECTING|CONNECTING2)==0);
		assert(dynamic_cast<const ISockAddr*>(&a));
		ISockAddr *sa = (ISockAddr*)&a;
		if (h.connect(*sa)) {
			if (errno == EINPROGRESS) {
				io->enable_write(this);
				setflag(CONNECTING);
			} else setflag(ERROR);
		} else setflag(CONNECTED);
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
	int TCPSocket::send(IOPoll *io, const u8 *packet, u32 *len, String *msg) {
		if (!*len) {
			io->disable_write(this);
			return 0;
		}
		int err = h.send(packet, len);
		if (err == EWOULDBLOCK) {
			io->enable_write(this);
			return 0;
		}
		if (!err && !*len)
			err = ECONNRESET;
		if (err && msg) h.errnostr(err, msg);
		return err;
	}
	int TCPSocket::recv(IOPoll *io, u8 *packet, u32 *len, String *msg) {
		int err = h.recv(packet, len);
		if (err == EWOULDBLOCK)
			return 0;
		if (!err && !*len)
			err = ECONNRESET;
		if (err && msg) h.errnostr(err, msg);
		return err;
	}



	// class UDPSocket
	int UDPSocket::send(IOPoll *io, const u8 *packet, u32 *len, String *msg) {
		int err = h.send(packet, len);
		if (err && msg) h.errnostr(err, msg);
		return err;
	}

	void UDPSocket::on_read(IOPoll *io) {
		u8 buf[65536];
		u32 len = sizeof(buf);
		ISockAddr ia;
		int err = h.recvfrom(buf, &len, &ia);
		if (err) {
			String msg;
			h.errnostr(err, &msg);
			on_error(io, msg, err);
			return;
		}
		return on_data(io, buf, err, ia);
	}

		

}
