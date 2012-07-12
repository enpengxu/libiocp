#ifndef __sglib_type_h
#define __sglib_type_h

namespace sglib{
	// Undo any Windows defines.
	#undef BYTE
	#undef WORD
	#undef DWORD
	#undef INT
	#undef FLOAT
	#undef MAXBYTE
	#undef MAXWORD
	#undef MAXDWORD
	#undef MAXINT
	#undef CDECL

	// Global constants.
	enum {MAXBYTE		= 0xff       };
	enum {MAXWORD		= 0xffffU    };
	enum {MAXDWORD		= 0xffffffffU};
	enum {MAXSBYTE		= 0x7f       };
	enum {MAXSWORD		= 0x7fff     };
	enum {MAXINT		= 0x7fffffff };
	enum {INDEX_NONE	= -1         };
	enum {UNICODE_BOM   = 0xfeff     };
	enum ENoInit {E_NoInit = 0};


	// unsigned base types.
	typedef unsigned char		BYTE;		// 8-bit  unsigned.
	typedef unsigned short		WORD;		// 16-bit unsigned.
	typedef unsigned int		UINT;		// 32-bit unsigned.
	typedef unsigned long		DWORD;	// 32-bit unsigned.
	typedef unsigned __int64	QWORD;	// 64-bit unsigned.

	// signed base types.
	typedef	signed char			SBYTE;	// 8-bit  signed.
	typedef signed short		SWORD;	// 16-bit signed.
	typedef signed int  		INT;		// 32-bit signed.
	typedef signed __int64		SQWORD;		// 64-bit signed.

	typedef unsigned char		byte_t;		// 8-bit  unsigned.
	typedef unsigned short		word_t;		// 16-bit unsigned.
	typedef unsigned int		uint_t;		// 32-bit unsigned.
	typedef unsigned long		dword_t;	// 32-bit unsigned.
	typedef unsigned __int64	qword_t;	// 64-bit unsigned.

	// signed base types.
	typedef	signed char			sbyte_t;	// 8-bit  signed.
	typedef signed short		sword_t;	// 16-bit signed.
	typedef signed int  		int_t;		// 32-bit signed.
	typedef signed __int64		sqword_t;		// 64-bit signed.

	// character types.
	typedef char				char_t;		// An ANSI character.
	typedef unsigned short      unichar_t;	// A unicode character.
	typedef unsigned char		ansicharu_t;// An ANSI character.
	typedef unsigned short      unicharu_t;	// A unicode character.

	// Other base types.
	typedef signed int			bool_t;		// Boolean 0 (false) or 1 (true).
	typedef float				float_t;	// 32-bit IEEE floating point.
	typedef double				double_t;	// 64-bit IEEE double.
	//typedef unsigned long       size_t;     // Corresponds to C SIZE_T.

	typedef signed int			BOOL;		// Boolean 0 (false) or 1 (true).
	typedef float				FLOAT;		// 32-bit IEEE floating point.
	typedef double				DOUBLE;		// 64-bit IEEE double.
	typedef unsigned long       SIZE_T;     // Corresponds to C SIZE_T.

#ifndef intptr_t
	#ifdef _WIN64
		typedef unsigned __int64	PTRINT;		// Integer large enough to hold a pointer.
		typedef unsigned __int64	intptr_t;	// Integer large enough to hold a pointer.
	#else
		typedef unsigned long		PTRINT;		// Integer large enough to hold a pointer.
		typedef unsigned __int32	intptr_t;		// Integer large enough to hold a pointer.
	#endif
#endif

#ifdef max
	#undef max
#endif


}
#endif