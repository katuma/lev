#ifndef LEV_BUF_H
#define LEV_BUF_H

#include "obj.h"

#include <memory.h>
#include <stdlib.h>



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
	protected:;
		u8 *buf;
		u32 bufin;
		u32 bufout;
		u32 buflen;
	public:;
		inline Buffer() : buf(0), bufin(0), bufout(0), buflen(0) { };


		~Buffer() {
			if (buf) free(buf);
		}

		inline u32 capacity() {
			return buflen;
		}
		
		inline u32 space() {
			return buflen - bufout;
		}
		
		inline u32 compact() {
			if (bufin)
				memmove(buf, buf + bufin, bufout -= bufin);
			bufin = 0;
			return space();
		}
		
		inline u8 *ensure(u32 plen)
		{
			// compact if the hole is gaping enough
			if (bufin > BUF_COMPACT)
				compact();

			if (space() < plen)
				realloc(buf, buflen = bufout + plen + BUF_CHUNK);
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

		void _reserve(u32 nsz);
		void _check(u32 sz);

/*
		// fetch arbitrary structure
		template <typename T>
		inline bool fetch(T *out) {
			if (sizeof(T) < length()) return true;
			memcpy(out, input(), sizeof(T));
			return consume(sizeof(T));
		};

		// fetch arbitrary integer of given endian
		template <typename T>
		inline bool fetch(T *out, bool e) {
			if (sizeof(T) < length()) return true;
			// convert from given endian to host endian
			T in = *((T *)input());
			*out = endian(in);
			return consume(sizeof(T));
		}

		// specialize for types signalling explicit endian
		inline bool fetch(u16le *out) { return fetch(out, false); }
		//inline bool fetch(u16be *out) { return fetch(out, true); }
		inline bool fetch(u32le *out) { return fetch(out, false); }
		//inline bool fetch(u32be *out) { return fetch(out, true); }
		inline bool fetch(u64le *out) { return fetch(out, false); }
		//inline bool fetch(u64be *out) { return fetch(out, true); }
		
		// fetch terminated string and append it to output string
		inline bool fetch(string *out, u8 term) {
			u8 *p = input();
			if (u8 *p2 = (u8*)memchr((void*)p, term, length())) {
				out->append((const char*)p, p2 - p);
				return consume(p2 - p);
			}
			return true;
		}
		
		// fetch string of given length
		inline bool fetch(string *out, u32 len) {
			if (len < length()) return false;
			out->append((const char *)input(), len);
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
			T val = endian((T*)in, e);
			store(&val);
		}

		// specialize for types signalling explicit endian
		inline void store(u16le *in) { return store(in, false); }
		//inline bool store(u16be *in) { return store(in, true); }
		inline void store(u32le *in) { return store(in, false); }
		//inline bool store(u32be *in) { return store(in, true); }
		inline void store(u64le *in) { return store(in, false); }
		//inline bool store(u64be *in) { return store(in, true); }*/
	};


	// simplified vector based on buffer
	template <class T>
	class Vector : Buffer {
		const int sz = sizeof(T);
	public:;
		inline void clear() {
			reset();
			// XXX destructors
		}
		
		inline u32 size() {
			return length()/sz;
		}
		
		void reserve(u32 n) {
			_reserve(n*sz);
		}
		
		inline void resize(u32 n, T val) {
			u32 nsz = n*sz;
			if (length() >= nsz) {
				// XXX destructors of trimmed members
				bufout = bufin + nsz;
				return;
			}
			_reserve(nsz);
			// allocate new fields
			while (length() < nsz)
				push_back(val);
		}
		
		inline Vector<T>& operator=(const Vector<T>&) {
			assert(0 && "not yet");
		}

		inline void push_back(T val) {
			if (space() < sz)
				_check(sz);
			at(size()) = val;
			bufout += sz;
		}
		
		inline T& at(u32 n) {
			return ((T *)output())[n];
		}
		
		inline T& front() {
			return at(0);
		}

		inline T& operator[](u32 idx) {
			return at(idx);
		}
	};

	// boolean is somewhat special.
	template<>
	class Vector<bool> {
	public:;
		inline Vector<bool>() : bitpos(0), buf(0) {};
		const u32 bytes = sizeof(unsigned long);
		const u32 bits = bytes*8;
		const u32 mask = bits-1;

		u32 bitpos;
		unsigned long *buf;

		inline void clear() {
			bitpos = 0;
		}
		
		inline u32 size() {
			return bitpos;
		}
		
		inline u32 nwords(u32 n) {
			return (n + mask) & ~mask;
		}

		void reserve(u32 n);
		void resize(u32 n, bool val);

		inline void setbit(u32 idx, bool val) {
			if (val)
				buf[idx/bits] |= 1<<(idx%bits);
			else
				buf[idx/bits] &= ~(1<<(idx%bits));
		}

		inline void push_back(bool val) {
			if (bitpos % bits == 0) // speedup
				return resize(bitpos+1, val);
			setbit(bitpos++, val);
		}
		
		inline bool getbit(u32 idx) {
			return buf[idx/bits] & (1<<(idx%bits));
		}
		
		inline bool operator[](u32 idx) {
			return getbit(idx);
		}

		Vector<bool>& operator=(Vector<bool>&);

	};
}

#endif
