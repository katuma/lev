#include <lev>

using namespace lev;

// this->close(), marked closing
// delete this
// this->client = 0 via auto des
// delete client invoked
// client->close()
// client->conn marked closing, skip

class SOCKSRemote : public BTOTCPSocket {
public:;
	inline SOCKSRemote() : BTOTCPSocket() {};
    Auto<BTOTCPSocket> client; // should be SOCKSClient, but this works too
	void on_data(const u8 *buf, uint len, const IAddr *from)
	{
		client->output.append(buf, len);
		input.consume(len);
		client->flush();
	}

	void on_connect() {
		client->flush();
		client->resume();
		resume();
	}
};



class SOCKSClient : public BTOTCPSocket {
public:;
	inline SOCKSClient() : BTOTCPSocket(), conn(0) {};
	Auto<SOCKSRemote> conn;

	void on_data(const u8 *buf, const u32 len, const IAddr *from) {
        u8 ver, cmd;
        u32 dstip;
        u16 dstport;
        u32 pos = 0;
        String userid, remote;

		// this will keep buffering until 'conn' is truly connected..
		if (conn) {
			conn->output.append(buf, len);
			input.consume(len);
			conn->flush();
			return;
		}

        if (!input.unpack(&pos).be(&ver).be(&cmd).be(&dstport).be(&dstip).str_z(&userid).commit(pos))
            return;
        if (ver != 4)
            return error("unknown protocol ver %d", ver);
        // other sanity.
        if (cmd != 1)
            return error("unknown command %d", cmd);

        // note auto const char * -> string cast
        if (!dstip)
            return error("ip can't be null");

        if (!dstport)
            return error("port can't be null");

        // socks4a marker - unpack hostname
        if ((dstip&0xffffff00)==0) {
            // unpack the hostname requested
            if (!input.unpack(&pos).str_z(&remote).commit(pos))
                return;
            else if (remote.empty())
                return error("socks4a hostname should not be empty");
		}

		// prepare reply for the client - note that this will not be written
        // version 0, response 90, dstport, dstip
		// because we didn't flush() yet
		output.be((u8)0).be((u8)90).be(dstport).be(dstip);
		pause();
        conn = io->create<SOCKSRemote>();
        conn->client = this;
		conn->pause();
		if (remote.empty())
			conn->connect(IPv4(dstip, dstport));
		else
			conn->connect(remote, dstport);
	}
};


class SOCKSServer : public TCPServer<SOCKSClient> {
	void on_client(SOCKSClient *client, ISockAddr &sa) {
//		elog << "Client connected : " + client;
//		client->watermark(2 + 4 + 2 + 1);
	}
	void on_error(int err, void *data) {
//		elog << "Server bind error " + msg + " code: " + err;
	}
};


int main(int argc, char **argv)
{
	auto io = new IOLoop();
	auto server = io->create<SOCKSServer>();
	server->bind(argc>1?argv[1]:"0.0.0.0", argc>2?atoi(argv[2]):1080);
	server->listen();
	io->run();
}

