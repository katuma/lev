#ifndef _LEV_OBJ_H
#define _LEV_OBJ_H
#include <stdint.h>

#include "list.hh"
#include "types.hh"

namespace lev {

	// base class of acyclic Object graph.
	class Object : public List {
	public:;
		List children;
		~Object() {
			Object *c;
			// delete children until list is empty
			while ((c = (Object*)children.next) != this)
				if (c->on_delete(this))
					delete c;
				else assert(children.next != c);
		}

		template <typename UserType>
		operator UserType*() {
			return static_cast<UserType*>(this);
		};

		template <typename FromType, typename ToType>
		ToType& operator=(const FromType &src) {
			return static_cast<ToType&>(src);
		};
	protected:;
		// construct with parent
		Object(Object *o) : List() {
			linkto(o);
		};
		// unowned
		Object() : List() { };
		virtual bool on_delete(Object *) {
			return true;
		}
	};
};

#endif
