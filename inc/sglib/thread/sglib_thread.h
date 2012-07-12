#ifndef __sglib_thread_h
#define __sglib_thread_h

//#include "sglib/sglib_utility.h"
#include "sglib_mutex.h"
#include "sglib_event.h"
#include "sglib_cond.h"

namespace sglib{

int interlocked_inc(int volatile * m);
int interlocked_dec(int volatile * m);

class threadsafe {
public:
	threadsafe(){}

	#define SCOPE_LOCK   mutex_locker _lock_(&_thread_mutex);
protected:
	mutable mutex _thread_mutex;
};

class threadloop {
public:
	threadloop () {}
	virtual void operator ()(void * p =0) = 0;
};

class thread {
public:
	thread(int priority = 0) : _h(0), _id(0), _loop_func(0) , _loop(0), _priority(priority){
	}
    virtual ~thread() {
	   //SG_ASSERT(_h ==0);
	}
	void setpriority(int priority){
		_priority = priority;
	}
	inline void run_loop(){
		set_priority();
		if(_loop)  
			 (*_loop)(_p);
		else if(_loop_func)
			_loop_func(_p);
		else 
			loop();
	}

	inline intptr_t thread_handle(){
		return _h;
	}
	
	inline dword_t thread_id(){
		return _id;
	}
	int  run  (void * p);  // loop inside version
	int  run  (void (*loop)(void *), void * p); // specify a loop function
	int  run  (threadloop & loop, void *);				   // specify a loop class
	int  wait (int ms=-1);
	void kill ();

public:
	virtual void loop() { 
		SG_ASSERT(0);
	}
	static intptr_t cur_thread_id ();
protected:
	void set_priority();
protected:
	void       * _p;			 // parameter passed in
	threadloop * _loop;			 // a loop class pointer
	void (* _loop_func)(void *); // or a loop function
	intptr_t _h;				 // thread handle
	dword_t  _id;				 // thread id
	int _priority;
};


class thread_pool {
	typedef struct {
		thread * t;
		sync_event control_event;
		sync_event thread_event;
		void (* loop_f)(void *); // function
		void *  loop_p; // param
		bool abort;

	}thread_t;
public:
	enum THREAD_STATUS {
		THREAD_IDLE = 0,
		THREAD_BUSY , 
		THREAD_ABORTED, 
	};

	static void thread_run_loop(void *p){
		thread_t* t = (thread_t*)p;
		ASSERT(t);
		while(true){
			// wait until we get a signal to run!
			t->control_event.wait(-1);
			if(t->abort){
				t->thread_event.signal();
				return ;
			}
			// run a piece of code
			t->loop_f(t->loop_p);
			t->thread_event.signal();
		}			
	}

	thread_pool() :_num(0) {
	}
	~thread_pool(){
		abort();
	}

	bool create(int nthreads){
		_num = nthreads;
		if(_num >=128){
			return false;
		}
		for(int i=0; i<_num; i++){
			_threads[i].t = new thread;
			_threads[i].control_event.reset();
			_threads[i].thread_event.reset();
			_threads[i].abort = false;
			_threads[i].loop_f = 0;
			_threads[i].loop_p = 0;
		}
		return true;
	}
	bool run(){
		for(int i=0; i<_num; i++){
			_threads[i].t->run(thread_pool::thread_run_loop, (void *)&_threads[i]);
		}
		return true;
	}

	THREAD_STATUS thread_status(int n){
		ASSERT(n >=0 && n < _num);
		if(_threads[n].thread_event.wait(0)){
			// if thread event is signaled. it means thread finish its task
			//_threads[n].thread_event.wait(0)){
			return THREAD_IDLE;
		}
		return THREAD_BUSY;
	}
	bool thread_run(int n, void (* func)(void *), void * param){
		ASSERT(n >=0 && n < _num);
		_threads[n].loop_f = func;
		_threads[n].loop_p = param;
		_threads[n].control_event.signal();
		return true;
	}
	bool thread_waitfinished(int n){
		ASSERT(n >=0 && n < _num);
		return _threads[n].thread_event.wait(-1);
	}	
	void thread_abort(int n){
		ASSERT(n >=0 && n < _num);
		_threads[n].abort = true;
		_threads[n].control_event.signal();
		_threads[n].thread_event.wait(-1);
	}	
	void abort(){
		for(int n=0; n<_num; n++){
			_threads[n].abort = true;
			_threads[n].control_event.signal();
		}
		for(int n=0; n<_num; n++){
			_threads[n].t->wait(-1);
			delete _threads[n].t;
		}
		_num = 0;
	}
protected:
	int _num;
	thread_t _threads[128];
};

#ifdef _WIN32
#include "sglib_thread_win32.hpp"
#else
#ifdef SDL
	#include "sglib_thread_sdl.hpp"
#endif
#endif

}//thread, sglib
#endif