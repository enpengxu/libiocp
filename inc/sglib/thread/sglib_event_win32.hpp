#ifdef WIN32

#include <windows.h>

#pragma warning(push)
#pragma warning(disable: 4311 4312)

inline sync_event::sync_event(bool broadcast) {
	_event = (intptr_t)CreateEvent( NULL    //security
								  , broadcast ? TRUE: FALSE	// manul reset or not
								  , FALSE	// init state
								  , NULL	// name	
								  );
}

inline sync_event::~sync_event(){
	CloseHandle((HANDLE)_event);
}

inline bool sync_event::wait(int timeout){
	ASSERT(_event);
	DWORD t = (timeout < 0) ? INFINITE : timeout;
	DWORD r = WaitForSingleObject((HANDLE)_event, t);
	if(r == WAIT_OBJECT_0)
		return true;
	if(r== WAIT_TIMEOUT)
		return false;
	return false;
}

inline int sync_event::signal(){
	ASSERT(_event);
	return SetEvent((HANDLE)_event) ? 1 : 0;
}

inline int sync_event::reset(){
	ASSERT(_event);
	return ResetEvent((HANDLE)_event) ? 1 : 0;
}

inline intptr_t sync_event::handle() {
	return _event;
}

// =======================
// sync_events
// =======================
inline sync_events::sync_events(int n) : _n(n) {
	_handles = (intptr_t *)new HANDLE[n];
	_events  = new sync_event *[n];
	for(int i=0; i<n; i++){
		_events[i] = new sync_event();
		_handles[i] = _events[i]->handle(); 
	}
}

inline sync_events::~sync_events(){
	for( int i=0; i<_n; i++){
		delete _events[i];
	}
	delete []_events;
	delete []_handles;
}

inline sync_event * sync_events::get_item (int n){
	ASSERT(n>=0 && n<_n);
	return _events[n];
}

inline bool sync_events::wait(int timeout){
	ASSERT(_handles);
	DWORD t = (timeout < 0) ? INFINITE : timeout;
	DWORD r = WaitForMultipleObjects ( _n        // number of objects in array
									 , (const HANDLE *)_handles  // array of objects 
									 , TRUE      // wait for any object
								     , t
									 );      
	if(r == WAIT_OBJECT_0)  // 
		return true;
	return false;
}

#pragma warning( pop )
#endif