#ifndef LEV_STR_H
#define LEV_STR_H

#include "vector.hh"

#include <memory.h>
#include <stdlib.h>
#include <limits.h>



namespace lev {
	typedef char char_t;

	// strings are, in fact, vectors, padded with single 0
	class String : public Vector<char_t, 1> {
	public:;
		inline String() : Vector<char_t,1>() {};
		inline String(char_t *s) : Vector<char_t,1>() {
			copy(s, ::strlen(s));
		}
		inline String& operator=(const char *src) {
			//copy(src.p, src.getsize() / sz);
			copy(src, ::strlen(src));
			return *this;
		}
		inline operator const char *() {
			return (const char*)begin();
		}
	};
}

#endif
