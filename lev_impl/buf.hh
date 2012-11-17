#ifndef LEV_BUF_H
#define LEV_BUF_H

#include "obj.hh"
#include "vector.hh"
#include "str.hh"

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
		// or even convert straight into single bswap (clang)
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
		static const uint BUF_CHUNK = 4096;
		static const uint BUF_COMPACT = 256*1024;
		static const uint BUF_ERROR = (uint)(-1);
	protected:;
	public:;
		u32 bufhead; // read head
		Buffer() : Vector(), bufhead(0) { };

		// throw consumed buffer head
		uint compact() {
			memmove(p, (u8*)p + bufhead, bufhead);
			pos -= bufhead;
			bufhead = 0;
			return capacity();
		}

		// return how much is length for fetching
		uint bytes() {
			return pos - bufhead;
		}

		// get a pointer to buffer head, or null if no data to consume
		u8 *head() {
			return P + bufhead;
		}

		u8 *head(uint *len) {
			*len = bytes();
			return head();
		}

		u8 *tail() {
			return P + pos;
		}

		u8 *tail(uint *len) {
			*len = getsize() - pos;
			return tail();
		}

		u8 *tail(uint *len, const uint en) {
			ensure(en);
			return tail(len);
		}
		
		void reset() {
			pos = bufhead = 0;
		}

		// set buffer head position
		void seek(uint pos) {
			bufhead = pos;
		}

		// consume 'len'
		bool consume(uint len) {
			assert(bytes() >= len);
			bufhead += len;
			return false;
		}

		u8 *ensure(uint plen);

		////////////////////////////////
		// packers
		////////////////////////////////
		template <typename T>
		Buffer& be(T &var) {
			ensure(sizeof(T));
			T val = endian(var, 0);
			append((u8*)&val, sizeof(val));
			return *this;
		}
		template <typename T>
		Buffer& le(T &var) {
			ensure(sizeof(T));
			T val = endian(var, 1);
			append((u8*)&val, sizeof(val));
			return *this;
		}
		
		// pack asciiz
		Buffer& str_z(String &src) {
			ensure(src.size());
			append((u8*)src.p, src.size());
			return *this;
		}

		////////////////////////////////
		// unpackers
		////////////////////////////////

		// mark bufhead position
		Buffer& unpack(u32 *pos) {
			*pos = bufhead;
			return *this;
		}

		// check if enough bytes, otherwise error
		bool check(uint len) {
			if (bufhead == BUF_ERROR) return false;
			if (len > bytes()) {
				bufhead = BUF_ERROR;
				return false;
			}
			return true;
		}

		template <typename T>
		Buffer& be(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)head()), 0);
				consume(sizeof(T));
			}
			return *this;
		}
		template <typename T>
		Buffer& le(T *var) {
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
			bufhead = BUF_ERROR;
			return *this;
		}
		
		// on error, restore old buffer position
		bool commit(u32 opos) {
			if (bufhead == BUF_ERROR) {
				 bufhead = opos;
				 return false;
			}
			return true;
		}
		
	};
}
#undef P

#endif
