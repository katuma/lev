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
		virtual bool on_delete(Object *);
	public:;
		List children;
		~Object();
		template <typename UserType>
		inline operator UserType*() {
			return static_cast<UserType*>(this);
		};

		template <typename FromType, typename ToType>
		inline ToType& operator=(const FromType &src) {
			return static_cast<ToType&>(src);
		};
	};
};

#endif
