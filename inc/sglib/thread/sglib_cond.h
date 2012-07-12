#ifndef __sglib_cond_h
#define __sglib_cond_h

#include "sglib_mutex.h"
#include "sglib_event.h"

namespace sglib {

class condition {
public:		
	condition(bool broadcast=false) : _e(broadcast) {
	}
	~condition(){
	}
	/* Restart one of the threads that are waiting on the condition variable,
	   returns 0 or -1 on error.*/
	int signal(){
		return _e.signal();
	}
	int reset(){
		return _e.reset();
	}

	/* Restart all threads that are waiting on the condition variable,
	   returns 0 or -1 on error. */
	int broadcast(){
		// TODO not impl yet
		return _e.signal();
	}

	/* Wait on the condition variable, unlocking the provided mutex.
	   The mutex must be locked before entering this function!
	   The mutex is re-locked once the condition variable is signaled.
	   Returns 0 when it is signaled, or -1 on error.*/	
	int wait_signal(mutex * m, unsigned int ms = -1){
		m->unlock();
		int r = _e.wait(ms);
		m->lock();
		return r;
	}

protected:
	sync_event _e; 
};


} // thread, sglib
#endif