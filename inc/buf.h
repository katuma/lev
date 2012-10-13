#ifndef LEV_BUF_H
#define LEV_BUF_H

#include "obj.h"



namespace lev {

	template <typename T>
	T endian(T w, u32 endian)
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
	class Buffer {
		const int BUF_CHUNK = 4096;
		const int BUF_COMPACT = 256*1024;

		inline Buf() : buf(0), ipos(0), opos(0) { };
		u8 *buf;
		u32 bufin;
		u32 bufout;
		u32 buflen;

		~Buf() {
			if (buf) free(buf);
		}
		
		inline void space() {
			return buflen - bufout;
		}
		
		inline void compact() {
			if (bufin)
				memmove(buf, buf + bufin, bufout -= bufin);
			bufin = 0;			
		}
		
		inline u8 *ensure(u32 plen)
		{
			// compact if the hole is gaping enough
			if (bufin > BUF_COMPACT)
				compact();

			if (space() > buflen)
				realloc(buf, buflen = bufout + plen + BUF_CHUNK)
			return output();
		}
		
		// append buffer
		inline void append(u8 *p, u32 plen) {
			ensure(plen);
			assert(space() >= plen);
			// write either at the end or wraparound
			memcpy(buf + bufout, p, plen);
			bufout += plen;
		}

		// get a pointer to buffer head, or null if no data to consume

		// return how much is length for fetching
		inline u32 length() {
			return bufout - bufin;
		}
		
		inline bool empty() {
			return !length();
		}

		inline u8 *input() {
			return buf + bufin;
		}

		inline u8 *input(u32 *len) {
			*len = length();
			return input();
		}

		inline u8 *output() {
			return buf + bufout;
		}

		inline u8 *output(u32 *len) {
			*len = buflen - bufout;
			return output();
		}

		inline u8 *output(u32 *len, u32 en) {
			ensure(en);
			return output(len);
		}
		
		inline void reset() {
			bufin = bufout = 0;
		}

		// set buffer position
		inline void rewind(u32 pos) {
			bufin = pos;
		}

		// consume 'len'
		inline bool consume(u32 len) {
			assert(length() >= len);
			bufin += len;
			return false;
		}
		
		// fetch arbitrary structure
		template <typename T>
		inline bool fetch(T *out) {
			if (sizeof(T) < length()) return true;
			memcpy(out, data(), sizeof(T));
			return consume(sizeof(T));
		};

		// fetch arbitrary integer of given endian
		template <typename T>
		inline fetch(T *out, bool e) {
			if (sizeof(T) < length()) return true;
			// convert from given endian to host endian
			*out = endian(*((T *)data()));
			return consume(sizeof(T));
		}

		// specialize for types signalling explicit endian
		inline bool fetch(u16le *out) { return fetch(out, false); }
		inline bool fetch(u16be *out) { return fetch(out, true); }
		inline bool fetch(u32le *out) { return fetch(out, false); }
		inline bool fetch(u32be *out) { return fetch(out, true); }
		inline bool fetch(u64le *out) { return fetch(out, false); }
		inline bool fetch(u64be *out) { return fetch(out, true); }
		
		// fetch terminated string and append it to output string
		inline bool fetch(string *out, u8 term) {
			u8 *p = data();
			if (u8 *p2 = memchr(p, term, length())) {
				out->append((const char*)p, p2 - p);
				return consume(p2 - p);
			}
			return true;
		}
		
		// fetch string of given length
		inline bool fetch(string *out, u32 len) {
			if (len < length()) return false;
			out->append((const char *)data(), len);
			return consume(len);
		}

		// put arbitrary structure
		template <typename T>
		inline void store(T *out) {
			append((u8*)out, sizeof(T));
		}
		
		// store arbitrary integer of given endian
		template <typename T>
		inline void store(T *in, bool e) {
			ensure(sizeof(T));
			// convert from given endian to host endian
			T val = endian(*out, e);
			store(&val);
		}

		// specialize for types signalling explicit endian
		inline bool store(u16le *in) { return store(in, false); }
		inline bool store(u16be *in) { return store(in, true); }
		inline bool store(u32le *in) { return store(in, false); }
		inline bool store(u32be *in) { return store(in, true); }
		inline bool store(u64le *in) { return store(in, false); }
		inline bool store(u64be *in) { return store(in, true); }
	}
}

#endif
