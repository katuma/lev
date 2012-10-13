#include "obj.h"

namespace lev {
	// derived classes can override this,
	// returning false signals they dont want to be destroyed
	// (but they have to call ->unlink() anyway!)
	bool Object::on_delete(Object *parent) {
		return true;
	}
	Object::~Object() {
		// delete children until list is empty
		while ((Object *c = children.next) != this)
			if (c->on_delete(this))
				delete c;
			else assert(children.next != c);
	}
}
