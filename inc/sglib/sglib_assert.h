#ifndef __sglib_assert_h
#define __sglib_assert_h

#ifndef  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _DEBUG
	#include <assert.h>
	#undef      ASSERT
	//#define     ASSERT(a)	if(!(a)) { __asm int 3 } --- not avaliable in x86
	//#define   ASSERT(a)	if(!(a)) *(int *)0 = 0
	#define   ASSERT(a)	assert(a)
	#define		_SGLIB_DEBUG	1
#else
	#define		ASSERT(a)		(void)(a)
	#define		_SGLIB_DEBUG	0
#endif

#define	SG_ASSERT	ASSERT


#endif
