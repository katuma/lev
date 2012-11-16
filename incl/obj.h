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
		Object(Object *o) : List() {
			linkto(o);
		};
		// unowned
		Object() : List() { };
		virtual bool on_delete(Object *);
	public:;
		List children;
		~Object();
		template <typename UserType>
		operator UserType*() {
			return static_cast<UserType*>(this);
		};

		template <typename FromType, typename ToType>
		ToType& operator=(const FromType &src) {
			return static_cast<ToType&>(src);
		};
	};
};

#endif
