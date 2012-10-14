#ifndef _LEV_LIST_H
#define _LEV_LIST_H

#include <assert.h>

namespace lev {
	class List {
	public:;
		List *prev, *next;		
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
		// automagically cast to covariant types
		template <typename UserType>
		inline operator UserType*() {
			return static_cast<UserType*>(this);
		};

		template <typename FromType, typename ToType>
//		template <typename ToType>
		inline ToType* operator=(const FromType *src) {
			return static_cast<ToType*>(src);
		};
	};
}

#endif
