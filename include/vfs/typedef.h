#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

#if defined(_MSC_VER) || ((__BORLANDC__ >= 0x530) && !defined(__STRICT_ANSI__))

typedef __int8										var8;
typedef __int16										var16;
typedef __int32										var32;

typedef unsigned __int8								uvar8;
typedef unsigned __int16							uvar16;
typedef unsigned __int32							uvar32;

#else

typedef signed char									var8;
typedef signed short								var16;
typedef signed int									var32;

typedef unsigned char								uvar8;
typedef unsigned short								uvar16;
typedef unsigned int								uvar32;

#endif

#if defined(_MSC_VER) || ((__BORLANDC__ >= 0x530) && !defined(__STRICT_ANSI__))
	typedef __int64									var64;
	typedef unsigned __int64						uvar64;
#elif __GNUC__
	#if __WORDSIZE == 64
		typedef long int							var64;
		typedef unsigned long int					uvar64;
	#else
		__extension__ typedef long long				var64;
		__extension__ typedef unsigned long long	uvar64;
	#endif
#else
	typedef long long								var64;
	typedef unsigned long long						uvar64;
#endif

#include <wchar.h>
#if defined(_MSC_VER) && _MSC_VER > 1310 && !defined (_WIN32_WCE)
	#define swprintf swprintf_s
	#define snprintf sprintf_s
#else
	#define swprintf _snwprintf
	#define snprintf _snprintf
#endif

#ifdef _MSC_VER
	#ifndef _WCHAR_T_DEFINED
		#define _WCHAR_T_DEFINED
		typedef uvar16 	wchar_t;
	#endif 
#endif

#define MAKE_CC_ID(c0, c1, c2, c3) \
		((uvar32)(uvar8)(c0) | ((uvar32)(uvar8)(c1) << 8) | \
		((uvar32)(uvar8)(c2) << 16) | ((uvar32)(uvar8)(c3) << 24 ))

#endif 

