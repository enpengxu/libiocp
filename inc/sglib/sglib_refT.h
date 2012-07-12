#ifndef __sglib_refT_h
#define __sglib_refT_h

#include "sglib/thread/sglib_thread.h"

namespace sglib {

template <typename T>
class refT {
public:
	refT(): _ref(0){
	}
	
	inline int deref(){
		--_ref;
		if(_ref <= 0){
			(dynamic_cast<T*>(this))->destroy();
			return 0;
		}
		return _ref;
	}
	inline int ref(){
		return ++_ref;
	}

	inline int get_ref(){
		return _ref;
	}
protected:
	virtual void destroy(){
		delete dynamic_cast<T*>(this);
	}
protected:
	int _ref;
};


//extern "C" void _ReadWriteBarrier(void);
//#pragma intrinsic(_ReadWriteBarrier)
//		long const res = *x;
//     _ReadWriteBarrier();
//      return res;

template <typename T>
class threadrefT {
public:
	threadrefT(): _ref(1){
	}
	inline int deref(){
		if(interlocked_dec(&_ref) <= 0){
			(dynamic_cast<T*>(this))->destroy();
			return 0;
		}
		return _ref;
	}
	inline int ref(){
		return interlocked_inc(&_ref);
	}
protected:
	virtual void destroy(){
		delete dynamic_cast<T*>(this);
	}

protected:
	int _ref; //volatile _ref;
};


}
#endif