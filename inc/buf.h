#ifndef LEV_BUF_H
#define LEV_BUF_H

#include "obj.h"
#include "vector.h"
#include "str.h"

#include <memory.h>
#include <stdlib.h>
#include <limits.h>



namespace lev {
	// convert to or from endian
	template <typename T>
	T endian(T w, uint endian)
	{
		// this gets optimized out into if (endian == host_endian) return w;
		union { uint64_t quad; uint32_t islittle; } t;
		t.quad = 1;
		if (t.islittle ^ endian) return w;
		T r = 0;

		// decent compilers will unroll this (gcc)
		// or even convert straight into siřngle bswap (clang)
		for (int i = 0; i < sizeof(r); i++) {
			r <<= 8;
			r |= w & 0xff;
			w >>= 8;
		}
		return r;
	};

	

	// buffer for input and output.
#define P static_cast<u8*>(p)
	class Buffer : public Vector<u8> {
		static const unsigned BUF_CHUNK = 4096;
		static const unsigned BUF_COMPACT = 256*1024;
	protected:;
	public:;
		u32 bufhead; // read head
		inline Buffer() : Vector(), bufhead(0) { };

	// buffer stuff
		inline uint compact() {
			memmove(p, (u8*)p + bufhead, bufhead);
			pos -= bufhead;
			bufhead = 0;
			return capacity();
		}
		// get a pointer to buffer head, or null if no data to consume

		// return how much is length for fetching
		inline uint bytes() {
			return pos - bufhead;
		}

		inline u8 *head() {
			return P + bufhead;
		}

		inline u8 *head(uint *len) {
			*len = bytes();
			return head();
		}

		inline u8 *tail() {
			return P + pos;
		}

		inline u8 *tail(uint *len) {
			*len = getsize() - pos;
			return tail();
		}

		inline u8 *tail(uint *len, const uint en) {
			ensure(en);
			return tail(len);
		}
		
		inline void reset() {
			pos = bufhead = 0;
		}

		// set buffer head position
		inline void seek(uint pos) {
			bufhead = pos;
		}

		// consume 'len'
		inline bool consume(uint len) {
			assert(bytes() >= len);
			bufhead += len;
			return false;
		}

		u8 *ensure(uint plen);

		////////////////////////////////
		// packers
		////////////////////////////////
		template <typename T>
		inline Buffer& be(T &var) {
			ensure(sizeof(T));
			T val = endian(var, 0);
			append((u8*)&val, sizeof(val));
			return *this;
		}
		template <typename T>
		inline Buffer& le(T &var) {
			ensure(sizeof(T));
			T val = endian(var, 1);
			append((u8*)&val, sizeof(val));
			return *this;
		}
		
		// pack asciiz
		inline Buffer& str_z(String &src) {
			ensure(src.size());
			append((u8*)src.p, src.size());
			return *this;
		}

		////////////////////////////////
		// unpackers
		////////////////////////////////

		// mark bufhead position
		inline Buffer& unpack(u32 *pos) {
			*pos = bufhead;
			return *this;
		}

		// check if enough bytes, otherwise error
		inline bool check(uint len) {
			if (pos > bufhead) return false;
			if (len > bytes()) {
				pos = bufhead+1;
				return false;
			}
			return true;
		}

		template <typename T>
		inline Buffer& be(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)head()), 0);
				consume(sizeof(T));
			}
			return *this;
		}
		template <typename T>
		inline Buffer& le(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)head()), 1);
				consume(sizeof(T));
			}
			return *this;
		}

		// unpack zero-terminated string
		Buffer& str_z(String *str) {
			u8 *h = head();
			uint n = bytes();
			for (uint i = 0; i < n; i++) {
				if (!h[i]) {
					str->copy((char_t*)h, i);
					consume(i+1);
					return *this;
				}
			}
			// not enough bytes
			pos = bufhead+1;
			return *this;
		}
		
		// on error, restore old buffer position
		bool commit(u32 opos) {
			if (pos > bufhead) {
				 bufhead = opos;
				 return false;
			}
			return true;
		}
		
	};

	/*
	class BufPacker : Buffer {
	private:;
		inline BufPacker() {};
	public:;
		template <typename T>

	};
	
	class BufUnpacker {
	private:;
		inline BufPacker() {};
	public:;
		inline bool check(uint len) {
			if (bufhead > pos) return false;
			if (len > bytes()) {
				bufhead = pos+1;
				return false;
			}
			return true;
		}
		
		template <typename T>
		BufUnpacker &be(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)b->head()), 0);
				b->consume(sizeof(T));
			}
			return *this;
		}
		template <typename T>
		BufUnpacker &le(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)b->head()), 1);
				b->consume(sizeof(T));
			}
			return *this;
		}

		// copy zero-terminated string
		BufUnpacker &str_z(String &str) {
			u8 *h = b->head();
			uint n = b->bytes();
			for (uint i = 0; i < n; i++) {
				if (!h[i]) {
					str.copy((char_t*)h, i);
					b->consume(i+1);
					return *this;
				}
			}
			err = true;
			return *this;
		}
		
		bool commit(u32 pos) {
			volatile bool e = err;
			if (e) b->bufhead = pos;
			// wow, fuck this
			delete this;
			return !e;
		}
	};*/

}
#undef P

#endif
