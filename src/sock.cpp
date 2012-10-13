#include "sock.h"

namespace lev {
	
	// class ISocket

	void ISocket::poll(IOLoop *io, bool r, bool w) {
		if (r) on_read(io);
		if (w) on_write(io);
	}

	// class IBufferedSocket
	
	// descriptor ready for reading, read into the buffer
	// and call on_data() if there are new bytes
	void IBufferedSocket::on_read(IOPoll *io) {
		string *s;
		u32 len;
		if (int err = recv(input.output(READ_CHUNK, &len), &len, &s))
			return on_error(io, s, err);
		return on_data(io);
	}

	// descriptor ready for writing
	// and call on_flush() if the write went through
	void IBufferedSocket::on_write(IOPoll *io) {
		u32 len;
		string *s;
		if (int err = send(output.input(&len), &len, &s))
			return on_error(io, s, err);
		if (output.empty())
			on_flush(io);
	}


	// class InetSocket


	// bind address
	ISocket *InetSocket::bind(IAddr *a) {
		assert(dynamic_cast<ISockAddr*>(a));
		ISockAddr *sa = (ISockAddr*)a;
		if (h.bind(sa))
			flags = ERROR | errno;
		else flags |= BOUND;
		return this;
	}

	// connect to an address
	ISocket *InetSocket::connect(IAddr *a) {
		assert(flags&(LISTENING|CONNECTING|CONNECTING2)==0);
		assert(dynamic_cast<ISockAddr*>(a));
		ISockAddr *sa = (ISockAddr*)a;
		if (h.connect(sa)) {
			if (errno == EINPROGRESS) {
				flags |= CONNECTING;
			}
			flags = ERROR;
			flags |= errno;
		} else flags |= CONNECTING2;
		return this;
	}

	// socket is being destroyed
	void InetSocket::on_delete(Object *parent) {
		if (on_close(parent)) {
			h.close();
			return ISocket::on_delete(this, parent);
		}
		return false;
	}


	// class TCPSocket

	int TCPSocket::send(IOPoll *, u8 *packet, u32 *len, string **msg) {
		if (!*len) {
			h.disable_write();
			return 0;
		}
		int err = h.send(packet, &len);
		if (err == EWOULDBLOCK) {
			h.enable_write();
			return 0;
		}
		if (!*len)
			err = ECONNRESET;
		if (err) *msg = h.strerror(err);
		return err;
	}
	int TCPSocket::recv(IOPoll *, u8 *packet, u32 *len, string **msg) {
		int err = h.recv(packet, &len);
		if (err == EWOULDBLOCK)
			return 0;
		if (!*len)
			err = ECONNRESET;
		if (err) *msg = h.strerror(err);
		return err;
	}
}
