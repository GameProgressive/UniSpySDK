///////////////////////////////////////////////////////////////////////////////
// File:	XBOXCommon.c
// SDK:		GameSpy Common Xbox code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////.
#include "../gsCommon.h"

// ErrorMessage: Displays message and goes into an infinite loop
// continues rendering
void  _gsDebugAssert(const char *string)
{
	//DebugBreak();
	OutputDebugString( string);
	exit(0);
}

void gsiDebugPrint(const char* format, va_list params)
{
	static char string[256];
	vsprintf(string, format, params); 			
	OutputDebugStringA(string);
}


#if defined(_MSC_VER) && (_MSC_VER < 1300)
	//extern added for vc6 compatability.
	extern void* __cdecl _aligned_malloc(size_t size, int boundary);
#endif
	
void* GS_STATIC_CALLBACK _gsi_memalign(size_t boundary, size_t size)
{
	return  _aligned_malloc(size, (int)boundary);
}
