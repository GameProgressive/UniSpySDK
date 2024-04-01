///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformX360.h
// SDK:		GameSpy Common Xbox 360 code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Xbox360:          _WIN32 + _XBOX + _X360

#ifndef __RSPLATFORMX360_H__
#define __RSPLATFORMX360_H__

#define _X360
#include <Xtl.h>

#if (_MSC_VER > 1300)
	#define itoa(v, s, r) _itoa(v, s, r)
#endif

#ifndef WIN32
	#define WIN32
#endif

//---------- __cdecl fix for __fastcall conventions ----------
#define GS_STATIC_CALLBACK __cdecl

// Platform dependent types
#if (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
	typedef __int64               gsi_i64;
	typedef unsigned __int64      gsi_u64;
#endif

#ifdef GSI_UNICODE
	#define _tsnprintf _snwprintf
#else
	#define _tsnprintf _snprintf
#endif

#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#define GSI_NO_STR_EXT 1
#define PRE_ALIGN(x)	__declspec(align(x))	// ignore Win32 directive
#define POST_ALIGN(x)	// ignore

#define GSI_BIG_ENDIAN 1
#define GS_NO_FILE 1

// gsPlatformThread
#define GSI_INFINITE INFINITE
#define gsiInterlockedIncrement(a) InterlockedIncrement((long*)a)
#define gsiInterlockedDecrement(a) InterlockedDecrement((long*)a)

typedef CRITICAL_SECTION   GSICriticalSection;
typedef HANDLE GSISemaphoreID;
typedef HANDLE GSIThreadID;
typedef DWORD (WINAPI *GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE DWORD WINAPI
#define GS_THREAD_RETURN return (DWORD)0
#define GS_THREAD_RETURN_NEGATIVE return (DWORD)-1

// gsPlatformSocket
#define GOAGetLastError(s) WSAGetLastError()
typedef int socklen_t;

		// hostent support
		struct hostent
		{
			char* h_name;       
			char** h_aliases;    
			gsi_u16 h_addrtype; // AF_INET
			gsi_u16 h_length;   
			char** h_addr_list; 
		};

		typedef struct hostent HOSTENT;
		struct hostent * gethostbyname(const char* name);
char * inet_ntoa(IN_ADDR in_addr);

#define GS_HAVE_SOCKET_TYPEDEF 1
#define GS_HAVE_IOCTLSOCKET 1
#define GSI_UNUSED(x) x

#endif // __RSPLATFORMX360_H__
