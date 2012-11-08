#ifndef LEV_VECTOR_H
#define LEV_VECTOR_H

#include "obj.h"

#include <memory.h>
#include <stdlib.h>
#include <limits.h>



namespace lev {

	// common vector class
	class VectorBase {
protected:;
		void _reserve(uint nsz);
		uint _ensure(uint size, uint sz);
		void _insert(uint into, void *val, uint repeat, uint sz, uint pad);
		inline VectorBase() : p(0) {};
public:;
		void *p;
		uint pos; // in elements, not bytes

		// no _msize() - we have keep track
#ifndef _MSIZE_HACK
		uint alloc_size;
		inline uint getsize() const {
			return alloc_size;
		}
		inline uint setsize(uint n) {
			return (alloc_size = n);
		}
#else
#ifdef _MSIZE_DLMALLOC
		inline uint getsize() {
			(!p) return 0;
			// uber-UB
			return ((size_t *)(p))[-1];
		}

#elif defined(_MSIZE_WIN32)
		inline getsize() {
			(!p) return 0;
			return _msize(p);
		}
#endif
		// this will hopefully bail
		inline setsize(uint n) {
			assert(n == getsize());
			return n;
		}
#endif

public:;
		// check if empty
		inline bool empty() {
			return !pos;
		}
		// return maximum size
		inline uint max_size() {
			return INT_MAX;
		}
		// return size
		inline uint size() {
			return pos;
		}
		inline ~VectorBase() {
			if (p) free(p);
		}
	};
	
#define P static_cast<T*>(p)
	template <typename T, uint pad = 0>
	class Vector : public VectorBase {
	protected:;
		static const uint sz = sizeof(T);
		static const uint factor = sz * 8;

		typedef T* iterator;
	public:;
		inline Vector() : VectorBase() {};
		inline Vector(Vector &s) : VectorBase() {
			copy(s);
		}

		// amount of available space
		inline uint avail() {
			return capacity() - pos;
		}
				
		// append array of elements. no padding.
		inline Vector& append(const T *ptr, uint plen) {
			uint nsz = _ensure(plen, sz);
			memcpy(P + pos, ptr, nsz);
			pos += plen;
			return *this;
		}

		// return elm at n. negative indexes are valid.
		inline T& at(int n) {
			if (n < 0)
				n+=pos;
			return P[n];
		}


		// assign repeated element
		/*inline Vector<T>& assign(const uint repeat, const &T val) {
			pos = 0;
			_insert(0, &val, repeat, sz, pad);
			return this;
		}*/
		
		// return last element
		inline T& back() {
			return at(-1);
		}


		// return first iterator
		inline iterator begin() {
			return P;
		}
		
		// storage available
		inline uint capacity() {
			return getsize()/sz;
		}

		// clear elements
		inline Vector& clear() {
			pos = 0;
			return *this;
		}


		// return end of vector
		inline iterator end() {
			return P + pos;
		}

		// erase elements
		inline iterator erase(iterator first, iterator last) {
			memmove(first, last, (&P[pos] - last) * sz);
			pos = first - P;
			return first;
		}

		// erase element
		inline iterator erase(iterator first) {
			return erase(first, first + 1);
		}

		// first element
		inline T& front() {
			return at(0);
		}

		// XXX allocators not implemented
		
		// insert element at position
		inline Vector& insert(iterator position, uint n, const T& x) {
			_insert(position, &x, n, sz, pad);
			return *this;
		}
		
		// pad
		inline Vector<T>& addpad() {
			memset(P + pos, 0, pad+sz);			
			return *this;
		}

		// copy elements
		inline Vector& copy(const T *src, uint len) {
			pos = len;
			_reserve((len+pad) * sz);
			memcpy(p, src, len * sz);;
			return *this;
		}

		inline Vector& copy(const Vector<T>&src) {
			copy(src.p, src.getsize() / sz);
			return this;
		}

		// index element
		inline T& operator[](int idx) {
			return at(idx);
		}

		// remove and return last element
		inline T& pop_back() {
			T &val = P[--pos];
			addpad();
			return val;
		}

		// push element to the end, and return it's iterator
		Vector<T>& push_back(T &val) {
			_ensure(1 + pad, sz);
			P[pos++] = val;
			addpad();
			return *this;
		}

		// reserve buffer for n elms
		inline Vector<T>& reserve(const uint n) {
			_reserve((n+sz)*sz);
			return *this;
		}

		// resize vector
		inline Vector<T>& resize(uint n, T &val) {
			if (n <= pos) {
				pos = n;
				addpad();
				return;
			}
			uint repeat = n - pos;
			pos = n;
			_insert(n, &val, repeat, sz, pad);
		}
		
		// swap
		inline Vector<T>& swap(Vector<T> &other) {
			void *tmp = p;
			uint tmpsize = getsize();
			p = other.p;
			setsize(other.getsize());
			other.p = tmp;
			other.setsize(tmpsize);
			return this;
		}
	};

	template<>
	class Vector<bool> : public VectorBase {
	protected:;
		typedef ulong T;
		static const uint sz = sizeof(T);
		static const uint factor = sz * 8;

		inline const ulong _mask(uint n) {
			return 1<<(n % factor);
		}

		inline const uint _word(uint n) {
			return n / factor;
		}

		// take number of bits, align to word and return number of bytes
		inline const uint _align(uint n) {
			return _word(n + (factor-1)) * sz;
		}

	public:;
		inline Vector() : VectorBase() {};

		// amount of available space
		inline uint avail() {
			return capacity() - pos;
		}

		// return elm at n. negative indexes are valid.
		inline bool at(int n) {
			if (n < 0)
				n+=pos;
			return P[_word(n)] % _mask(n);
		}

		// return last element
		inline const bool back() {
			return at(-1);
		}

		// storage available
		inline uint capacity() {
			return getsize() * factor;
		}
		
		// clear elements
		inline Vector<bool>& clear() {
			pos = 0;
			return *this;
		}

		// first element
		inline const bool front() {
			return at(0);
		}

		// XXX allocators not implemented


		inline Vector<bool>& copy(const Vector<bool>&src) {
			_reserve(src.getsize());
			memcpy(p, src.p, _align(src.pos));
			return *this;
		}

		inline const Vector<bool>& operator=(const Vector<bool>&src) {
			copy(src);
			return src; // what about `this`?
		}

		// index element
		inline bool operator[](const int idx) {
			return at(idx);
		}

		// remove and return last element
		inline bool pop_back() {
			return at(--pos);
		}

		// set bit
		inline void setat(uint pos, bool val) {
			if (val)
				P[_word(pos)] |= _mask(pos);
			else
				P[_word(pos)] &= ~_mask(pos);			
		}

		// push element to the end, and return it's iterator
		Vector<bool>& push_back(bool val) {
			reserve(pos + 1);
			setat(pos++, val);
			return *this;
		}

		// reserve buffer for n elms
		inline Vector<bool>& reserve(const uint n) {
			_reserve(_align(n));
			return *this;
		}

		// resize vector
		inline Vector<bool>& resize(uint n, bool val) {
			if (n <= pos) {
				pos = n;
				return *this;
			}
			reserve(n);
			while (pos < n)
				setat(pos++, val);
			return *this;
		}
		
		// return size
		inline uint size() {
			return pos;
		}
		
		// swap
		inline Vector<bool>& swap(Vector<bool> &other) {
			void *tmp = p;
			uint tmpsize = getsize();
			p = other.p;
			setsize(other.getsize());
			other.p = tmp;
			other.setsize(tmpsize);
			return *this;
		}
	};
#undef P	
}

#endif
