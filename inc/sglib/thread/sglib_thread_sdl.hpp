#include "SDL.h"
#include "SDL_thread.h"
#include "sglib_thread.h"

namespace sglib{
namespace thread{

	/* ===============================
	   static functions 
	  =============================== */
	int thread::thread_func(void * p)
	{
		thread * t = (thread *)p;
		//SG_ASSERT(t && t->_thread);
		return t->run();
	}
	int  thread::cur_thread_id (){
		return SDL_ThreadID();
	}
	void thread::run (thread * t){
		SG_ASSERT(t && !t->_thread);
		t->create_run();
	}
	int  thread::wait(thread * t){
		int status;
		SG_ASSERT(t && t->_thread);
		SDL_WaitThread((SDL_Thread *)(t->_thread), &status);
		t->_thread = 0;
		return status;
	}
	void thread::kill(thread * t){
		SG_ASSERT(t && t->_thread);
		SDL_KillThread((SDL_Thread *)(t->_thread));
		t->_thread = 0;
	}
	int thread::thread_id(thread * t){
		SG_ASSERT(t && t->_thread);
		return SDL_GetThreadID((SDL_Thread *)(t->_thread));
	}

	/* ===============================
	   member function 
	  =============================== */
	void thread::create_run(){
		_thread = (void *)SDL_CreateThread(thread::thread_func, this);
	}
	
}}