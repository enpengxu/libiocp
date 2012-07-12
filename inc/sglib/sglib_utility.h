#ifndef __sglib_utility_h
#define __sglib_utility_h

#include "sglib_assert.h"
#include "sglib_type.h"
#include <assert.h>

#ifndef  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#endif

namespace sglib {

typedef 	     __int32	int32;
typedef unsigned __int32	uint32;

// align ptr xxx bits
#define SGLIB_ALIGN_BYTES(ptr, n)  ((((intptr_t)(ptr)) +  n-1) & ~(n-1)) 



}

#endif
