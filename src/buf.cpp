#include "buf.h"

namespace lev {
	u32 Buffer::compact() {
		if (bufin)
			memmove(buf, buf + bufin, bufout -= bufin);
		bufin = 0;
		return space();
	}

	u8 *Buffer::ensure(u32 plen) {
		// compact if the hole is gaping enough
		if (bufin > BUF_COMPACT)
			compact();

		if (space() < plen)
			buf = (u8*) realloc(buf, buflen = bufout + plen + BUF_CHUNK);
		return output();
	}
	
	void Buffer::_reserve(u32 nsz) {
		if (buflen >= nsz) return;
		compact();
		if (buflen >= nsz) return;
		buf = (u8*)realloc(buf, buflen = nsz);
	}

	void Buffer::_assign(void *val, u32 repeat, u32 sz, u32 pad) {
		_reserve(repeat * sz + pad);
		while (--repeat) {
			memcpy(buf + bufout, val, sz);
			bufout += sz;
		}
		memset(buf + bufout, 0, pad);
	}
	
	void Buffer::_check(u32 sz) {
		if (compact() < sz)
			buf = (u8*)realloc(buf, buflen = buflen + sz);			
	}
	
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
	}
}
