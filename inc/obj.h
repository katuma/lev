#ifndef _LEV_OBJ_H
#define _LEV_OBJ_H
#include <stdint.h>

#include "list.h"
#include "types.h"

namespace lev {

	// base class of acyclic Object graph.
	class Object : public List {
	protected:;
		// construct with parent
		inline Object(Object *o) : List() {
			linkto(o);
		};
		// unowned
		inline Object() : List() { };
		virtual bool on_delete(Object *) = 0;
	public:;
		List children;
		~Object();

		// automagically cast to covariant types
		template <class T> inline operator T*() { return static_cast<T*>(this); };
	};
	~Object();
};

#endif
