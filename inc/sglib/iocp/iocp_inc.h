#ifndef __iocp_inc_h
#define __iocp_inc_h

//#undef  _WIN32_WINNT 
//#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>

#pragma comment(lib,"Ws2_32.lib")
#pragma warning(disable: 4819)

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "sglib/sglib_assert.h"

#define IO_PACK_MAX_LEN  1024*512

namespace sglib{
namespace net{

enum IO_CONDITION{
	IO_QUIT = 0,
	IO_SEND,
	IO_RECV,
};

typedef struct {
	OVERLAPPED	 olp;
	IO_CONDITION operation;
	WSABUF		 wsa_buf[2];
}OVERLAPPED_EX;

class lock {
public:
	lock (CRITICAL_SECTION *cs): _cs(cs) {  
		 EnterCriticalSection(cs);
	 }
	~lock () {  		
		LeaveCriticalSection(_cs);
	}
	CRITICAL_SECTION * _cs;
};

}
}
#endif