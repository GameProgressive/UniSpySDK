///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatform.h
// SDK:		GameSpy Common
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __GSPLATFORM_H__
#define __GSPLATFORM_H__

// Sample platform file:
//     Platform specific inclusions
//     Platform big endian? Define GSI_BIG_ENDIAN
//     Limited platform with no file access? Define GS_NO_FILE
//     Wireless define? Define GS_WIRELESS_DEVICE
//     Is platform always 64bit? Define GSI_64BIT
//     No threads? Define GSI_NO_THREADS
//     64-bit typedefs (gsi_i64, gsi_u64)
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "rsCommon.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Include common OS headers
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

//---------- __cdecl fix for __fastcall conventions ----------
#ifndef GS_STATIC_CALLBACK
	#define GS_STATIC_CALLBACK
#endif

//---------- Handle Endianess ----------------------
#ifndef GSI_BIG_ENDIAN
	#define GSI_LITTLE_ENDIAN 1
#endif

#include <ctype.h>
#include <assert.h>

//---------- Handle 64-bit data ---------- 
#ifdef GSI_64BIT
	#define PTR_ALIGNMENT 32
#else
	#define PTR_ALIGNMENT 16
#endif

#ifdef _DEBUG
	#define GSI_COMMON_DEBUG 1
#endif

#if !defined(_DEBUG_) && defined(GSI_COMMON_DEBUG)
	#define _DEBUG_ 1
#endif


#if !defined(GSI_OPEN_DOMAIN_NAME)
	#define GSI_OPEN_DOMAIN_NAME_UNI _T("%s.api." GSI_DOMAIN_NAME) // unicode supported string
	#define GSI_OPEN_DOMAIN_NAME "%s.api." GSI_DOMAIN_NAME
#endif

#if !defined(RS_HTTPS)
	#define RS_HTTP_PROTOCOL_URL "http://"
#else
	#define RS_HTTP_PROTOCOL_URL "https://"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Define GameSpy types

// common base type defines, please refer to ranges below when porting
typedef char              gsi_i8;
typedef unsigned char     gsi_u8;
typedef short             gsi_i16;
typedef unsigned short    gsi_u16;
typedef int               gsi_i32;
typedef unsigned int      gsi_u32;
typedef unsigned int      gsi_time;  // must be 32 bits

// Deprecated
typedef gsi_i32           goa_int32;  // 2003.Oct.04 - typename deprecated
typedef gsi_u32           goa_uint32; //  these types will be removed once all SDK's are updated

typedef int               gsi_bool;
#define gsi_false         ((gsi_bool)0)
#define gsi_true          ((gsi_bool)1)
#define gsi_is_false(x)   ((x) == gsi_false)
#define gsi_is_true(x)    ((x) != gsi_false)

// Max integer size
#if defined(_INTEGRAL_MAX_BITS) && !defined(GSI_MAX_INTEGRAL_BITS)
	#define GSI_MAX_INTEGRAL_BITS   _INTEGRAL_MAX_BITS
#else
	#define GSI_MAX_INTEGRAL_BITS   32
#endif

#if defined(__GNUC__)
#define GSI_FMT64	"ll"
#elif defined(_MSC_VER)
#define GSI_FMT64	"I64"
#else
#define GSI_FMT64	""
#endif

#if GSI_64BIT
	typedef gsi_u64 gsi_mptr;
#else
	typedef gsi_u32 gsi_mptr;
#endif


#ifndef GSI_UNICODE	
	#define gsi_char  char
	#define GSI_WCHAR (char *)
#else
	// is Unicode
	#define GSI_WCHAR (wchar_t *)
	
	#ifndef gsi_char
		#define gsi_char  gsi_u16
	#endif
#endif

// expected ranges for integer types
#define GSI_MIN_I8        CHAR_MIN
#define GSI_MAX_I8        CHAR_MAX
#define GSI_MAX_U8        UCHAR_MAX

#define GSI_MIN_I16       SHRT_MIN
#define GSI_MAX_I16       SHRT_MAX
#define GSI_MAX_U16       USHRT_MAX

#define GSI_MIN_I32       INT_MIN
#define GSI_MAX_I32       INT_MAX
#define GSI_MAX_U32       UINT_MAX

#if (GSI_MAX_INTEGRAL_BITS >= 64)
#define GSI_MIN_I64     (-9223372036854775807 - 1)
#define GSI_MAX_I64       9223372036854775807
#define GSI_MAX_U64       0xffffffffffffffffui64
#endif

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Common platform string functions
#undef _vftprintf
#undef _ftprintf
#undef _stprintf
#undef _tprintf
#undef _tcscpy
#undef _tcsncpy
#undef _tcscat
#undef _tcslen
#undef _tcschr
#undef _tcscmp
#undef _tfopen
#undef _T
#undef _tsnprintf
#undef _tremove
#undef _trename

#ifdef GSI_UNICODE
	#define _vftprintf  vfwprintf
	#define _ftprintf   fwprintf
	#define _tprintf    wprintf
	#define _tcscpy     wcscpy
	#define _tcsncpy(d, s, l)	wcsncpy((wchar_t *)d, (wchar_t *)s, l)
	#define _tcscat     wcscat
	#define _tcslen     wcslen
	#define _tcschr     wcschr
	#define _tcsrchr   wcsrchr
	#define _tcscmp(s1, s2)     wcscmp((wchar_t *)s1, (wchar_t *)s2)
	#define _tfopen     _wfopen
	#define _tremove _wremove
    #define _trename _wrename
	#define _T(a)       L##a
	#define _STR_CST(a)		(gsi_u16 *)##a
	#define _WRD_CST(a)		(wchar_t *)##a

	#ifndef _tsnprintf
		#define _tsnprintf swprintf
	#endif

	#ifndef _stprintf
		#define _stprintf   snwprintf
	#endif
	
	#ifndef _vswprintf
		#define _vswprintf  vsnwprintf
	#endif
#else
	#define _vftprintf  vfprintf
	#define _ftprintf   fprintf
	#define _stprintf   sprintf
	#define _tprintf    printf
	#define _tcscpy     strcpy
	#define _tcsncpy	strncpy
	#define _tcscat     strcat
	#define _tcslen     strlen
	
#if defined (_MSC_VER)

#if (_MSC_VER < 1400)
	#define _tcschr	    strchr
#endif

#else
	#define _tcschr	    strchr
#endif

	#define _tcsrchr	strrchr
	#define _tcscmp     strcmp
	#define _tfopen     fopen
	#define _tremove    remove
    #define _trename    rename
#ifndef _T	
	#define _T(a)       a
#endif
	#define _STR_CST(a)		a
	#define _WRD_CST(a)		a

	#ifndef _tsnprintf
		#define _tsnprintf snprintf
	#endif
#endif // GSI_UNICODE

#ifndef GSI_NO_STR_EXT
	char *_strlwr(char *string);
	char *_strupr(char *string);
#endif

char * goastrdup(const char *src);
unsigned short * goawstrdup(const unsigned short *src);


// ------ Cross Plat Alignment macros ------------
/* ex use
PRE_ALIGN(16)	struct VECTOR			
{
	float	x,y,z,_unused;	
}	POST_ALIGN(16);

// another example when defining a variable:
PRE_ALIGN(16);
static char _mempool[MEMPOOL_SIZE]	POST_ALIGN(16);

*/

#ifndef PRE_ALIGN
	#define PRE_ALIGN(x)
#endif

#ifndef POST_ALIGN
	#define POST_ALIGN(x)
#endif

#define DIM( x )				( sizeof( x ) / sizeof((x)[ 0 ]))

unsigned char * gsiFloatSwap(unsigned char buf[4], float);
float gsiFloatUnswap(unsigned char buf[4]); 
extern gsi_u16 gsiByteOrderSwap16(gsi_u16);
extern gsi_u32 gsiByteOrderSwap32(gsi_u32);
extern gsi_u64 gsiByteOrderSwap64(gsi_u64);


// Wide-string character defines for use in printf
//
// Usage:
//   GS_USTR - for gsi_char string that flips between unicode and ascii depending on GSI_UNICODE define
//   GS_STR  - for char string that remains as ascii
//

#if !defined(GS_STR) && !defined(GS_USTR)
#if defined(GSI_UNICODE)
	#define GS_STR  _T("%S")
	#define GS_USTR _T("%s")
#else
    #define GS_STR  _T("%s")
    #define GS_USTR _T("%s")
#endif
#endif

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __GSPLATFORM_H__

