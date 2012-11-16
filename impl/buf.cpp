#include "buf.h"

namespace lev {
	


	u8 *Buffer::ensure(uint plen) {
		// compact if the hole is gaping enough
		if (bufhead > BUF_COMPACT)
			compact();

		if (avail() < plen)
			_ensure(plen, 1);
		return tail();
	}


	// vector stuff
	void VectorBase::_reserve(uint nsz) {
		uint old = getsize();
		if (old >= nsz) return;
		p = realloc(p, setsize(nsz));
		//memset(buf + old, 0, nsz - old);
	}
	
	u32 VectorBase::_ensure(uint nelm, uint sz) {
		_reserve((pos+nelm) * sz);
		return nelm * sz;
	}

	// insert is seldom used, so it's rather generalized
	// do NOT use for bools
	void VectorBase::_insert(uint into, void *val, uint repeat, uint sz, uint pad) {
		// reserve enough space
		_reserve((pos + repeat + pad) * sz);
		// shuffle the data around
		u8 *ptr = ((u8*)p)+(into*sz);
		uint tomove = pos - into;
		pos += repeat;
		memmove(ptr + repeat*sz, ptr, tomove);
		// fill, XXX val might be an iterator in the future
		while (repeat--) {
			memcpy(ptr, val, sz);
			ptr += sz;
		}
		if (tomove) memset(ptr, 0, pad * sz);
	}

	/*
	void Vector<bool>::resize(u32 n, bool val) {
		if (n < bitpos) {
			bitpos = n;
			return;
		}
		if (nwords(n) > nwords(bitpos))
			reserve(n);
		u32 i;
		for (i = bitpos; i < n && (i % bits != 0); i++)
			setbit(i, val);
		i /= bits;
		n /= bits;
		unsigned long l = ((unsigned long)-1L) * val;
		while (i < n)
			buf[i] = l;
	}

	void Vector<bool>::reserve(u32 n) {
		u32 nw = nwords(n);
		if (nw > nwords(bitpos))
			buf = (unsigned long*)realloc(buf, nw * bytes);
	}

	Vector<bool>& Vector<bool>::operator=(Vector<bool>&src) {
		reserve(src.size());
		memcpy(buf, src.buf, nwords(src.size() * bytes));
		return src;
	}*/
}
