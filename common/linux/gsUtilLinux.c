///////////////////////////////////////////////////////////////////////////////
// File:	gsUtilLinux.c
// SDK:		GameSpy Common Linux code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"

gsi_i64 gsiStringToInt64(const char *theNumberStr)
{
	return atoll(theNumberStr);
}

void gsiInt64ToString(char theNumberStr[33], gsi_i64 theNumber)
{
	// you want to fit the number! 
	// give me a valid string!
	GS_ASSERT(theNumberStr != NULL);
	
	sprintf(theNumberStr, "%lld", theNumber);
}

void gsiDebugPrint(const char* format, va_list params) // unics
{
	vprintf(format, params);
}

void* _gsi_memalign(size_t boundary, size_t size)
{
	void *ptr = calloc((size)/(boundary), (boundary));
	// check alignment
	GS_ASSERT((((intptr_t)ptr)% boundary)==0);
	return ptr;
}
