#ifndef __sglib_mutex_h
#define __sglib_mutex_h

#include "sglib/sglib_utility.h"
#ifdef WIN32
#include <windows.h>
#endif

namespace sglib {

class mutex {
	friend class cond;
public:
	mutex() {
		bool b = create();
		SG_ASSERT(b);
	}
	~mutex() {
		destroy();
	}
	void lock();
	void unlock();
protected:
	bool create();
	bool destroy();

#ifdef WIN32
	CRITICAL_SECTION _cs;
#else
	void * _m; 
#endif
};

class mutex_locker{
public:
	mutex_locker(mutex *m) : _m(m) {
		m->lock();
	}
	~mutex_locker(){
		if(_m)	_m->unlock();
	}
	inline void unlock(){
		if(_m){
			_m->unlock();
			_m = 0;
		}
	}
protected:
	mutex * _m;
};

#ifdef WIN32
	#include "sglib_mutex_win32.hpp"
#else
#ifdef SDL
	#include "sglib_mutex_sdl.hpp"
#endif
#endif


} // thread, sglib
#endif

