#ifdef WIN32
#include <windows.h>
#pragma warning(push)
#pragma warning(disable: 4311 4312)


inline int interlocked_inc(int volatile * m){
	return (int)InterlockedIncrement((LONG volatile*)m);
}
inline int interlocked_dec(int volatile * m){
	return InterlockedDecrement((LONG volatile*)m);  
}

/* ===============================
   static functions 
  =============================== */
inline DWORD WINAPI threadproc(LPVOID p){
	thread * t = (thread *)p;
	SG_ASSERT(t);
	t->run_loop();
	return 1;
}

/* ===============================
   member function 
  =============================== */
inline void thread::set_priority(){
	int priority = THREAD_PRIORITY_NORMAL;
	
	switch(_priority){
	case 0:
		priority = THREAD_PRIORITY_NORMAL;
		break;
	case 1:
		priority = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case -1:
		priority = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case 2:
		priority = THREAD_PRIORITY_HIGHEST;
		break;
	case -2:
		priority = THREAD_PRIORITY_LOWEST;
		break;
	default:
		break;
	}
	if(_priority < -2){
		priority = THREAD_PRIORITY_IDLE;
	}
	if(_priority > 2){
		priority = THREAD_PRIORITY_TIME_CRITICAL;
	}
	if(!SetThreadPriority((HANDLE)_h, priority)){
		DWORD err = GetLastError();
	}
}
inline int thread::run(void * p)
{
	_p = p;
	_loop = 0;
	_loop_func = 0;
	DWORD id;
	HANDLE h = ::CreateThread(NULL, 0, threadproc, this, 0/*CREATE_SUSPENDED*/, &id);
	_h = (intptr_t)h;
	_id = (dword_t)id;
	return (h == NULL) ? 0 : 1;
}
inline int thread::run(void (*loop)(void *), void * p)
{
	SG_ASSERT(loop);
	_p = p;
	_loop = 0;
	_loop_func = loop;
	DWORD id;
	HANDLE h = ::CreateThread(NULL, 0, threadproc, this, 0/*CREATE_SUSPENDED*/, &id);
	_h  = (intptr_t)h;
	_id = (dword_t)id;
	return (h == NULL) ? 0 : 1;
}

inline int thread::run(threadloop & loop, void * p)
{
	_p = p;
	_loop = &loop;
	_loop_func = 0;
	DWORD id;
	HANDLE h = ::CreateThread(NULL, 0, threadproc, this, 0/*CREATE_SUSPENDED*/, &id);
	_h = (intptr_t)h;
	_id = id;
	return (h == NULL) ? 0 : 1;
}

inline int  thread::wait(int time){
	HANDLE h = (HANDLE)_h;
	DWORD  t = time == -1 ? INFINITE : time;
	DWORD  r = WaitForSingleObject(h, t);
	if(r == WAIT_FAILED) 
		return 0;
    CloseHandle(h);
	_h = 0;
	return 1;
}

inline void thread::kill(){
	if(_h)
		CloseHandle((HANDLE)_h);
	_h = 0;
}

inline intptr_t thread::cur_thread_id (){
	return (intptr_t)GetCurrentThreadId();
}

#pragma warning( pop )
#endif
