#ifndef __queueT_h
#define __queueT_h

#include <deque>
#include "sglib/thread/sglib_mutex.h"
#include "sglib/thread/sglib_cond.h"
#include "sglib_utility.h"



namespace sglib{

// thread safe queue implementation.
// ****************************************************************
// note: do not support broadcast function, so better limited in 
//	     two threads(put & get).
// ****************************************************************
// usage:
//	begin:
//		sglib::queue * queue = new sglib::queue();
//		thread1:: queue->put
//		thread2:: queue->get
//		thread3:: queue->put
//		....
//	end:
//		queue->quit();  // *** release any thread which is blocked 
//		thread1::wait	// *** make sure thread exit normally.
//		thread2::wait	// ....
//		thread3::wait	// ....
//		....
//	done:
//		queue->clear();	// now it is safe to clear content.
//		
// 
//

// queueT<T> is a unbounded queue. only get operation will be blocked.
template<class T>
class queueT{
	inline bool isempty(){
		return _count == 0;
	}
public:
	queueT() : _count(0), _stop(false) {
	}
   ~queueT() {
	   SG_ASSERT(_queue.size() == 0);
	   SG_ASSERT(_count == 0);
    }

   int size(){
	   sglib::mutex_locker lock(&_mutex);
	   if(_stop)  
			return 0;	
	   return _count;	
   }
	// return -1: quit, get a stop signal
	// return  1: ok
    int put(const T & p){
		sglib::mutex_locker lock(&_mutex);
		if(_stop)  
			return -1;
		_queue.push_back(p);
		++_count;
		_cond_not_empty.signal();
		return 1;
    }
	// return  1: get an element without any problem
	// return  0: empty queue & do not want to wait
	// return -1: user fired stop signal. give up waiting.
    int get(T * t, bool wait=true){
		int r = -1;
		sglib::mutex_locker lock(&_mutex);
		if(_stop)  
			return -1;

		if(_count ){
			*t = _queue.front();
			_queue.pop_front();
			--_count;
			if(_count == 0)
				_cond_not_empty.reset();
			return 1;
		}
		// empty 
		if(!wait) // do not want to wait...
			return 0;
		
		//wait for singnals, which is fired by put (thread)function
		while(isempty()){
			_cond_not_empty.wait_signal(&_mutex);
			if(_stop)
				break;
		}
		if(_stop) 
			return -1;
		ASSERT(_count);
		--_count;
		*t = _queue.front();
		_queue.pop_front();
		return 1;
	}
	/** call quit for quit a possible waiting thread */
	void quit (){
		sglib::mutex_locker lock(&_mutex);
		_stop = true;
		_cond_not_empty.signal();
	}
	/** call clear for cleanup after call quit*/
	void clear(){
		sglib::mutex_locker lock(&_mutex);
		_queue.clear();
		_count = 0;
		_stop = true;  // after clear call. the queue is ready to run again.
	}
	/** call init for use the queue again */
	void init(){
		sglib::mutex_locker lock(&_mutex);
		_queue.clear();
		_count = 0;
		_stop = false;  // after clear call. the queue is ready to run again.
	}
protected:
	int  _count;
	bool _stop;
	std::deque<T>	 _queue;
	sglib::mutex     _mutex;
	sglib::condition _cond_not_empty;
};


template<class T>
class queue_boundedT {
	inline bool isfull(){
		return _count == _bound;
	}
	inline bool isempty(){
		return _count == 0;
	}
public:
	queue_boundedT(int bound) 
			: _cond_not_full(true)
			, _cond_not_empty(true)
			, _bound(bound)
			, _count(0)
			, _stop(false) {
	}
   ~queue_boundedT() {
	   SG_ASSERT(_queue.size() == 0);
	   SG_ASSERT(_count == 0);
    }

	// return -1 , means stop fired by user. quit
	// others, get a element normally.
    int put(const T & p){
		int r = -1;
		sglib::mutex_locker lock(&_mutex);
		while(isfull()) {
			_cond_not_full.wait_signal(&_mutex);
			if(_stop)
				break;
		}
		if(!_stop){
			_queue.push_back(p);
			r = ++_count;
			if(isfull()){ 
				// reset full condition for stoping  other threads 
				_cond_not_full.reset();
			}
			_cond_not_empty.signal();
		}
		return r;
    }

	// return -1 , means stop fired by user. quit
	// others, get a element normally.
    int get(T * p){
		int r = -1;
		sglib::mutex_locker lock(&_mutex);
		while(isempty()) {
			_cond_not_empty.wait_signal(&_mutex);
			if(_stop) 
				break;
		}
		if(!_stop){
			r = --_count;
			if(isempty()){
				_cond_not_empty.reset();
			}
			*p = _queue.front();
			_queue.pop_front();
			_cond_not_full.signal();
		}
		return r;
	}
	
	int peek(T * p, bool remove=false){
		sglib::mutex_locker lock(&_mutex);
		int r = _count; 
		if(r > 0 && p) {
			*p = _queue.front();
			if(remove){
				_queue.pop_front();
				--_count;
				_cond_not_full.signal();
			}
		}
		return r;
    }

	// it should be called from outer thread for quit
	void quit (){
		sglib::mutex_locker lock(&_mutex);
		_stop = true;
		_cond_not_empty.signal();
		_cond_not_full.signal();
	}
	void quit_thread_get(){
		sglib::mutex_locker lock(&_mutex);
		_stop = true;
		_cond_not_empty.signal();
	}
	void quit_thread_put(){
		sglib::mutex_locker lock(&_mutex);
		_stop = true;
		_cond_not_full.signal();
	}
	
	// after quit() call, call clear to clear resources.
	void clear(){
		sglib::mutex_locker lock(&_mutex);
		_queue.clear();
		_count = 0;
		_stop = false;
	}
	void reset(){
		sglib::mutex_locker lock(&_mutex);
		_cond_not_full.reset();
		_cond_not_empty.reset();
		_count = 0;
		_stop  = false;
		_queue.clear();
	}
protected:
	int _bound;
	int _count;
	bool _stop;
	std::deque<T>	 _queue;
	sglib::mutex     _mutex;
	sglib::condition _cond_not_full;
	sglib::condition _cond_not_empty;
};


template<class T>
class ringbufT{
	inline int _size(){
		return (_first - _last- 1 + _n)%_n;
	}
	inline bool _isempty(){
		return _size() == 0 ;
	}
	inline bool _isfull(){
		return _first == _last;
	}
	inline void _put(const T & t){
		_buf[_first] = t;
		_first = (_first+1)%_n;
	}
	inline void _get(T * t){
		_last = (_last+1)%_n;
		*t = _buf[_last] ;
	}
public:
	ringbufT() 
			: _buf(0)
			, _n(0)
			, _stop(false) 
			, _cond_not_full(true)
			, _cond_not_empty(true){
	}
   ~ringbufT() {
    }
	
    int size(){
	   sglib::mutex_locker lock(&_mutex);
	   if(_stop)  
			return 0;	
	   return _size();	
    }
	// return -1: quit, get a stop signal
	// return  1: ok
    int put(const T & t, bool wait=true){
		sglib::mutex_locker lock(&_mutex);
		if(_stop)  
			return -1;
		while(_isfull()){
			//full. wait until we can put one
			if(!wait) 
				return 0;
			_cond_not_full.wait_signal(&_mutex);
			if(_stop)
				return -1;
		}
		_put(t);
		_cond_not_empty.signal();
		return 1;
    }
	// return  1: get an element without any problem
	// return  0: empty queue & do not want to wait
	// return -1: user fired stop signal. give up waiting.
    int get(T * t, bool wait=true){
		int r = -1;
		sglib::mutex_locker lock(&_mutex);
		if(_stop)  
			return -1;
		while(_isempty()){
			// empty 
			if(!wait) // do not want to wait...
				return 0;
			//wait for singnals, which is fired by put (thread)function
			_cond_not_empty.wait_signal(&_mutex);
			if(_stop)
				return -1;
		}
		_get(t);
		_cond_not_full.signal();
		return 1;
	}
	/** call quit for quit a possible waiting thread */
	void stop (){
		sglib::mutex_locker lock(&_mutex);
		_stop = true;
		_cond_not_empty.signal();
		_cond_not_full.signal();
	}
	/** call clear for cleanup after call quit*/
	void clear(){
		sglib::mutex_locker lock(&_mutex);
		if(_buf)
			delete []_buf;
		_buf = 0;
		//_stop = true;  // after clear call. the queue is ready to run again.
	}
	/** call init for use the queue again */
	void init(int n){
		sglib::mutex_locker lock(&_mutex);
		_n = n+1;
		_first = 0;
		_last = _n-1;
		_buf = new T[_n];
		_stop = false;  // after clear call. the queue is ready to run again.
		_cond_not_empty.reset();
		_cond_not_full.reset();
	}
protected:
	T * _buf;
	int _first;
	int _last;
	int  _n;
	bool _stop;
	
	sglib::mutex     _mutex;
	sglib::condition _cond_not_full;
	sglib::condition _cond_not_empty;
};

}
#endif