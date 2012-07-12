#ifdef SDL

#include "SDL.h"
#include "SDL_thread.h"

inline bool mutex::create()
{
	_m = (void *)SDL_CreateMutex();
	return _m ? true : false;
}

inline bool mutex::destroy()
{
	SDL_DestroyMutex((SDL_mutex *)_m);
	_m = 0;
	return true;
}

inline void mutex::lock()
{
	SG_ASSERT(_m);
	SDL_LockMutex((SDL_mutex *)_m);

}
inline void mutex::unlock()
{
	SG_ASSERT(_m);
	SDL_UnlockMutex((SDL_mutex *)_m);
}

#endif