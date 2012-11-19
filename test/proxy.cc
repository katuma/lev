#include <lev>

using namespace lev;

#if 0

using namespace lev;

class SOCKSRemote : public Buffered<TCPSocket> {
public:;
    Buffered<TCPSocket> *client;
	void on_data(IOLoop *io, u8 *buf, u32 len, IAddr *from) {
		if (client->ok) {
			client->output->pipe(input)
			client->flush();
		}
	}
    void on_connect() {
        // version 0, response 90, dstport, dstip
        client->output.pack().be((u8)0).be((u8)90).be(port).be(ip);
		client->flush();
    }
    void on_close(IOLoop *io, Object *parent) {
        // we dont own client, hence explicit close
        client->close();
    }
}


class SOCKSClient : public Buffered<TCPSocket> {
public:;
	SOCKSClient() : Buffered<TCPSocket>(), ok(false) {};
	Buffered<TCPSocket> *conn;
	bool ok;
	void on_flush(IOLoop *io) {
		// short-circuit the two
		conn->resume();
		resume();
		ok = true;
	}
	
	void on_data(IOLoop *io, const u8 *buf, const u32 len, const IAddr &from) {
        u8 ver, cmd;
        u32 dstip;
        u16 dstport;
        u32 pos;
        String userid, remote;
		if (ok) {
			conn->output->pipe(input)
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
	server->bind(loop, new String(argc>1?argv[1]:0), argc>2?atoi(argv[2]):1080);
	server->listen();
	loop->run();
}
#endif

int main(int argc, char const *argv[])
{
	/* code */
	return 0;
}

