#ifndef _LEV_OBJ_H
#define _LEV_OBJ_H
#include <stdint.h>

namespace lev {
	typedef uint64_t u64;
	typedef uint32_t u32;
	typedef uint16_t u16;
	// base class of acyclic Object graph.
	class Object {
	protected:;
		// construct with parent
		inline Object(Object *o) {
			this->setParent(o);
		};
		// unowned
		inline Object() {
			this->setParent(0);
		}
		class Object *prev, *next;
	public:;
		// change parent of this object
		inline Object *setParent(Object *parent) {
			if (!parent)
				this->next = this->prev = 0;
			else {
				this->next = parent->next;
				this->prev = parent;
			}
			return this;
		};
		// automagically cast to covariant types
		template <class T> inline operator T*() { return static_cast<T*>(this); };
	};
};

#endif
