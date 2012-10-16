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
		static const unsigned BUF_CHUNK = 4096;
		static const unsigned BUF_COMPACT = 256*1024;
	protected:;
		u8 *buf;
		u32 bufin;
		u32 bufout;
		u32 buflen;

		void _assign(void *val, u32 repeat, u32 sz, u32 pad);
		void _reserve(u32 nsz);
		void _check(u32 sz);
	public:;
		inline Buffer() : buf(0), bufin(0), bufout(0), buflen(0) { };
		inline ~Buffer() {
			if (buf) free(buf);
		}

		inline u32 space() {
			return buflen - bufout;
		};
		
		// append buffer
		inline void append(const u8 *p, u32 plen) {
			ensure(plen);
			assert(space() >= plen);
			// write either at the end or wraparound
			memcpy(buf + bufout, p, plen);
			bufout += plen;
		}

		// get a pointer to buffer head, or null if no data to consume

		// return how much is length for fetching
		inline u32 bytes() {
			return bufout - bufin;
		}
		
		inline bool empty() {
			return !bytes();
		}

		inline u8 *input() {
			return buf + bufin;
		}

		inline u8 *input(u32 *len) {
			*len = bytes();
			return input();
		}

		inline u8 *output() {
			return buf + bufout;
		}

		inline u8 *output(u32 *len) {
			*len = buflen - bufout;
			return output();
		}

		inline u8 *output(u32 *len, const u32 en) {
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
			assert(bytes() >= len);
			bufin += len;
			return false;
		}

		u32 compact();
		u8 *ensure(u32 plen);
	};

	typedef char char_t;

	// simplified vector based on buffer
	// optional zero-padding size as second argument
	// destructors are NOT called
	template <class T, unsigned pad = 0>
	class Vector : public Buffer {
		static const unsigned sz = sizeof(T);
	public:;
		inline Vector() : Buffer() {};

		inline Vector(Vector &s) : Buffer() {
			copy(s);
		}

		inline Vector(char_t *s) : Buffer() {
			copy(s, ::strlen(s));
		}

		inline void clear() {
			reset();
			memset(buf, 0, pad);
		}

		inline u32 size() {
			return bytes()/sz;
		}

		void reserve(u32 n) {
			_reserve(n*sz + pad);
		}
		
		inline void resize(u32 n, T val) {
			if (n > size()) {
				bufout = bufin + n*sz;
				memset(buf + bufout, 0, pad);
				return;
			}
			_assign(&val, n-size(), sz, pad);
		}
		
		inline void assign(u32 repeat, T& val) {
			bufin = bufout = 0;
			_assign(&val, repeat, sz, pad);
		}

		inline void copy(const T *src, u32 len) {
			bufout = len * sz;
			bufin = 0;
			_reserve(bufout + pad);
			memcpy(buf, src, bufout + pad);
			memset(buf + bufout, 0, pad);
		}

		inline void copy(const Vector<T>&src) {
			copy(&src, src.bytes());
		}

		inline Vector<T>& operator=(const Vector<T>&src) {
			copy(src);
			return src; // what about this?
		}

		// specialized for strings
//		inline Vector<char_t>& 
		inline const char_t *operator=(const char_t *src) {
			copy(src, ::strlen(src));
			return src;
		}

		inline void push_back(T& val) {
			if (space() < sz)
				_check(sz * 4); // cache for 4 elements
			at(size()) = val;
			bufout += sz;
		}

		inline T pop_back() {
			bufout -= sz;
			T v = at(size());
			memset(&at(size()), 0, pad);
			return v;
		}
		
		inline T& at(u32 n) {
			return ((T *)output())[n];
		}
		
		inline T& front() {
			return at(0);
		}
		
		inline T& end() {
			return at(size());
		}

		// all index operations shall prevent copies
		inline T& operator[](u32 idx) {
			return at(idx);
		}

		// when asked for pointer, return one
		inline operator T*() {
			return (T*) (buf + bufin);
		};
	};



	// strings are, in fact, vectors, padding with single 0
	typedef Vector<char_t, sizeof(char_t)> String;

	// boolean is somewhat special.
	template<>
	class Vector<bool> {
	public:;
		inline Vector<bool>() : bitpos(0), buf(0) {};
		static const u32 bytes = sizeof(unsigned long);
		static const u32 bits = bytes*8;
		static const u32 mask = bits-1;

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
