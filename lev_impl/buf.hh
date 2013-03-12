#ifndef LEV_BUF_H
#define LEV_BUF_H

#include "vector.hh"
#include "str.hh"

#include <memory.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>



namespace lev {

	// convert value of type T according to endian (1 = big, 0 = little)
	template <typename T>
	inline T endian(T w, uint endian)
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

	// specialized for u8
	inline u8 endian(u8 w, uint endian) {
		return w;
	}


	// buffers for input and output, based on vectors
	class Buffer : public Vector<u8> {
	private:;
		static const uint BUF_COMPACT = 256*1024; // compact the buffer if more than this is wasted
		static const uint BUF_UNPACK_ERROR = (uint)(-1); // .bufhead marker denoting error during unpack
	public:;
		u32 bufhead; // buffer read head
		inline Buffer() : Vector(), bufhead(0) { };

               // compact the unused space of .bufhead
		inline uint compact() {
			memmove(p, (u8*)p + bufhead, bufhead);
			pos -= bufhead;
			bufhead = 0;
			return capacity();
		}

		// return how much is length for fetching
		inline uint bytes() {
			return pos - bufhead;
		}

		// get a pointer to buffer head, or null if no data to consume
		inline u8 *head() {
			return P() + bufhead;
		}

		inline u8 *head(uint *len) {
			*len = bytes();
			return head();
		}

               // get pointer to current buffer tail, ie &buf[pos]
		inline u8 *tail() {
			return P() + pos;
		}

		inline u8 *tail(uint *len) {
			*len = getsize() - pos;
			return tail();
		}

		inline u8 *tail(uint *len, const uint en) {
			ensure(en);
			return tail(len);
		}
		
               // reset the buffer to empty state
		inline void reset() {
			pos = bufhead = 0;
		}

		// set buffer head position
               inline void seek(uint to) {
                       bufhead = to;
		}

               // consume, len bytes will be chopped off the beggining
		bool consume(uint len) {
			assert(bytes() >= len);
			bufhead += len;
			return false;
		}

		// make sure plen can be appended to the buffer
		u8 *ensure(uint plen) {
			// compact if the hole is gaping enough
			if (bufhead > BUF_COMPACT)
				compact();

			if (avail() < plen)
				_ensure(plen, 1);
			return tail();
		}


		////////////////// packers //////////////////
		// (these could really use some traits stuff at some point)

		// pack big-endian integer
		template <typename T>
		inline Buffer& be(T var) {
			ensure(sizeof(T));
			T val = endian(var, 0);
			append((u8*)&val, sizeof(val));
			return *this;
		}

		// pack little-endian integer
		template <typename T>
		inline Buffer& le(T var) {
			ensure(sizeof(T));
			T val = endian(var, 1);
                       append((u8*)&val, sizeof(val)); // sizeof sic u8 cast
			return *this;
		}

		// bytes have no endian, so specialize the method name
		inline Buffer& byte(const u8 data) {
			return be(data);
		}

		// pack a string (no trailing \0 in the result)
		inline Buffer& str(const char_t *src, const uint n) {
			ensure(n);
			append((u8*)src, n);
			return *this;
		}

		// c-strings
		inline Buffer& str(const char_t *src) {
			return str(src, ::strlen(src));
		}

		// our strings
		inline Buffer& str(String &src) {
			return str((const char_t *)src.p, src.size());
		}

		inline Buffer& zero(const uint n) {
			u8 zerobyte = 0;
			ensure(n);
			resize(pos + n, zerobyte);
			return *this;
		}

		////////////////// unpackers //////////////////

		// mark bufhead position
		inline Buffer& unpack(u32 *pos) {
			*pos = bufhead;
			return *this;
		}

		// check if enough bytes, otherwise error
		inline bool check(uint len) {
                       if (bufhead == BUF_UNPACK_ERROR) return false;
			if (len > bytes()) {
                               bufhead = BUF_UNPACK_ERROR;
				return false;
			}
			return true;
		}

               // unpack big-endian integer
		template <typename T>
		inline Buffer& be(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)head()), 0);
				consume(sizeof(T));
			}
			return *this;
		}

               // unpack little endian integer
		template <typename T>
		inline Buffer& le(T *var) {
			if (check(sizeof(T))) {
				*var = endian(*((T*)head()), 1);
				consume(sizeof(T));
			}
			return *this;
		}

		// unpack zero-terminated string
		inline Buffer& str_z(String *str) {
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
			bufhead = BUF_UNPACK_ERROR;
			return *this;
		}

		// unpack specified-length string
		inline Buffer& str(String *str, uint want) {
			if (want < bytes()) bufhead = BUF_UNPACK_ERROR;
			else {
				str->copy((char_t*)head(), want);
					consume(want);
			}
			return *this;
		}

		// on error, restore old buffer position
		inline bool commit(u32 opos) {
			if (bufhead == BUF_UNPACK_ERROR) {
				bufhead = opos;
				return false;
			}
			return true;
		}
	};
}

#endif
