#ifndef _LEV_LOOP_H
#define _LEV_LOOP_H
#include "sock.hh"

namespace lev {
	class IOLoop;

	class IIOLoop {
	protected:;
		Handle &gethandle(ISocket *sock);
	public:;
		IIOLoop() {};
		/*
		IOLoop *add(ISocket *sock);
		IOLoop *del(ISocket *sock);
		IOLoop *enable_read(ISocket *s);
		IOLoop *disable_read(ISocket *s);
		IOLoop *enable_write(ISocket *s);
		IOLoop *disable_write(ISocket *s);
		static IOLoop* factory();
		*/
		virtual u64 poll(int timeout) = 0;
		inline void run() {
			while (1) poll(1000);
		}
		template <class Base>
		inline Base *create() {
			Base *b = new Base();
			b->io = (IOLoop*)this; // XXX hackish cast
			b->io->add(b);
			return b;
		}
	};
}

#ifdef __unix__
#include "loop_posix.hh"
#elif defined(_WIN32)
#include "loop_win32.hh"
#else
#error "No poll() implementation for the target operating system"
#endif

#endif
