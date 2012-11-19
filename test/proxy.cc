#include <lev>

using namespace lev;

class SOCKSRemote : public Buffered<TCPSocket> {
public:;
    Buffered<TCPSocket> *client;
    bool on_close(Object *parent) {
        // we dont own client, hence explicit close
        client->close();
		return true;
    }

	void on_data(IOLoop *io, const u8 *buf, uint len, const IAddr &from)
	{
		client->output.append(buf, len);
		input.consume(len);
		client->flush();
	}

	void on_connect(IOLoop *io) {
		client->flush();
		client->resume();
		resume();
	}
};



class SOCKSClient : public Buffered<TCPSocket> {
public:;
	SOCKSClient() : Buffered<TCPSocket>>(), ok(false), conn(0) {};
	Buffered<TCPSocket> *conn;

	void on_data(IOLoop *io, const u8 *buf, const u32 len, const IAddr &from) {
        u8 ver, cmd;
        u32 dstip;
        u16 dstport;
        u32 pos;
        String userid, remote;

		// this will keep buffering until 'conn' is truly connected..
		if (conn) {
			conn->output.append(buf, len);
			input.consume(len);
			conn->flush();
			return;
		}

        if (!client->input.unpack(&pos).be(&ver).be(&cmd).be(&dstip).str_z(&userid).commit(pos))
            return;
        if (ver != 4)
            kill << "unknown protocol ver " + ver;
        // other sanity.
        if (cmd != 1)
            kill << "unknown command " + cmd;

        // note auto const char * -> string cast
        if (!dstip)
            kill << "ip can't be null";

        if (!dstport)
            kill << "port can't be null";

        // socks4a marker - unpack hostname
        if (dstip&0xffffff00==0)
            // unpack the hostname requested
            if (!input.unpack().str_z(&remote).commit(pos))
                return;
            else if remote.empty()
                kill << "socks4a hostname should not be empty";

		// prepare reply for the client - note that this will not be written
        // version 0, response 90, dstport, dstip
		// because we didn't flush() yet
		output.be((u8)0).be((u8)90).be(port).be(ip);
		pause();
        // implies our ownership of 'conn'
        conn = create<SOCKSRemote>();
        conn->client = this;
		conn->pause();
		if (remote.empty())
			conn->connect(io, IPv4(dstip, dstport));
		else
			conn->connect(io, remote, dstport);
	}
}


class SOCKSServer : public TCPServer<SOCKSClient> {
	void on_client(IOLoop *io, SOCKSClient *client) {
		elog << "Client connected : " + client;
		client->watermark(2 + 4 + 2 + 1);
	}
	void on_error(IOLoop *io, String &msg, int err) {
		elog << "Server bind error " + msg + " code: " + err;
	}
}
int main(int argc, char **argv)
{
	auto loop = IOLoop.create(30, 300);
	auto server = loop->create<SOCKSServer>();
//	server->bind(loop, new String(argc>1?argv[1]:0), argc>2?atoi(argv[2]):1080);
//	server->listen();
	loop->run();
}


#endif
