#ifndef _LEV_LIST_H
#define _LEV_LIST_H

#include <assert.h>

namespace lev {
	class List {
	protected:;
		List *prev, *next;		
	public:;
		inline List() : prev(this), next(this) {};
		inline List(List *parent) : prev(this), next(this) { linkto(parent); };
		// link `this` object into `parent`
		inline List *linkto(List *parent) {	
			this->next = parent->next;
			this->prev = parent;
			parent->next->prev = this;
			parent->next = this;
			return this;
		};
		// remove `this` from parent
		inline List *unlink()
		{
			this->prev->next = this->next;
			this->next->prev = this->prev;
			this->prev = this->next = this;
			return this;
		}
		virtual ~List();
	};
}

#endif
