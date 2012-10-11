#ifndef _LEV_OBJ_H
#define _LEV_OBJ_H
#include <stdint.h>

#include "list.h"

namespace lev {
	typedef uint64_t u64;
	typedef uint32_t u32;
	typedef uint16_t u16;
	// base class of acyclic Object graph.
	class Object : public List {
	protected:;
		// construct with parent
		inline Object(Object *o) {
			linkto(o);
		};
		// unowned
		inline Object() : List() { };
	public:;
		inline Object *detach()
		{
			unlink();
			return this;
		}
		// change parent of this object
		inline Object *attach(Object *parent) {
			detach();
			linkto(parent);
			return this;
		}

		// automagically cast to covariant types
		template <class T> inline operator T*() { return static_cast<T*>(this); };
	};
};

#endif
