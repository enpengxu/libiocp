#ifdef WIN32

#include <windows.h>

inline bool mutex::create()
{
	InitializeCriticalSection(&_cs);
	return true;
}

inline bool mutex::destroy()
{
	DeleteCriticalSection(&_cs);
	return true;
}

inline void mutex::lock()
{
	EnterCriticalSection(&_cs);
}

inline void mutex::unlock()
{
	LeaveCriticalSection(&_cs);
}

#endif