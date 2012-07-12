#ifndef __sglib_event_h
#define __sglib_event_h

#include "sglib/sglib_utility.h"

namespace sglib{

class sync_event { 
public:
	sync_event(bool broadcast=false);
	~sync_event();

	bool wait(int timeout=-1);
	int  signal();
	int  reset();
	intptr_t handle();
protected:
	intptr_t _event;
};

class sync_events {
public:
	sync_events(int n);
	~sync_events();
	sync_event * get_item (int n);
	bool wait(int timeout);
protected:
	int _n;
	intptr_t    * _handles;
	sync_event ** _events;
};

#ifdef WIN32
	#include "sglib_event_win32.hpp"
#else
	#ifdef SDL
	#include "sglib_event_sdl.hpp"
	#endif
#endif

}// sglib
#endif