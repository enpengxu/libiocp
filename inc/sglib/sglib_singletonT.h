#ifndef __sglib_singletonT_h
#define	__sglib_singletonT_h

#include "sglib_utility.h"
namespace sglib {

/*
 * Usage: 
 * class class_xxx : public singletonT<class_xxx> {}
 * class_xxx::get_singleton()->....
 */

template <typename T> 
class singletonT
{
private:
	inline static T * instance(T * ptr=0) {
		static T * _instance = 0;
		if(ptr ==0) {
			SG_ASSERT(_instance);
		}
		else {
			SG_ASSERT(_instance==0);
			_instance = ptr;
		}
		return _instance;
	}
public:
	singletonT(){
#if defined( _MSC_VER ) && _MSC_VER < 1200	 
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		T * t = (T*)((int)this + offset);
#else
		T * t = static_cast< T* >(this);
#endif
		instance(t);
	}
	~singletonT( void ){  
	}
	
	inline static T * get_singleton(void){	
		return instance();
	}
};

}
#endif

