#ifndef LEV_AUTO_H
#define LEV_AUTO_H

namespace lev {
	// horrible lovechild of uniqptr and autoptr.
	// retains semantics of neither = ease of use over correctness :/
	template <class X>
    class Auto {
    public:
		X *ptr;
        // construct/copy/destroy
        inline explicit Auto(X* p = 0) : ptr(p) {};
		inline Auto(Auto &p) : ptr(p.release()) {};
        template <class Y> inline Auto(Auto<Y> &p) : ptr(p.release()) {};

	    // assign same type
        inline Auto& operator=(Auto &p) {
			reset(p.release());
			return *this;
		}
		// assign covariant type
		template<class Y> inline Auto& operator=(Auto<Y> &p) {
			reset(p.release());
			return *this;
		}
		// assign direct pointer
		inline Auto& operator=(X *p) {
			reset(p);
			return *this;
		}

		// delete
		inline ~Auto() {
			X *tmp = ptr;
			ptr = 0;
			delete tmp;
		};

		// deref
        inline X& operator*() {
			return *ptr;
		}
        inline X* operator->() {
			return ptr;
		}

		// return and set to null (equivalent to ->move of uniqptr)
		inline X* release() {
			X *tmp = ptr;
			ptr = 0;
			return tmp;
		}

		// set (and delete old)
		inline void reset(X *p) {
			if (ptr != p && ptr) {
				X *tmp = ptr;
				ptr = 0;
				delete tmp;
			}
			ptr = p;
		}

		// *ugh*
		inline operator X*() {
			return ptr;
		}

		inline operator void*() {
			return ptr;
		}
		inline operator bool() {
			return ptr!=0;
		}
    };
};

#endif
