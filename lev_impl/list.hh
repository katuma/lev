#ifndef _LEV_LIST_H
#define _LEV_LIST_H

#include <assert.h>

namespace lev {
	class List {
	public:;
		List *prev, *next;		
		List() : prev(this), next(this) {};
		List(List *parent) : prev(this), next(this) { linkto(parent); };
		// link `this` object into `parent`
		List *linkto(List *parent) {	
			this->next = parent->next;
			this->prev = parent;
			parent->next->prev = this;
			parent->next = this;
			return this;
		};
		// remove `this` from parent
		List *unlink()
		{
			this->prev->next = this->next;
			this->next->prev = this->prev;
			this->prev = this->next = this;
			return this;
		}
		// destroying the object unlinks it from the parent
		virtual ~List() {
			unlink();
		}
#if 0
		// automagically cast to covariant types
		template <typename UserType>
		operator UserType*() {
			return static_cast<UserType*>(this);
		};

		template <typename FromType, typename ToType>
//		template <typename ToType>
		ToType* operator=(const FromType *src) {
			return static_cast<ToType*>(src);
		};
#endif
	};
}

#endif
