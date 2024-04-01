///////////////////////////////////////////////////////////////////////////////
// File:	gsUtilNitro.c
// SDK:		GameSpy Common Nitro code
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

void  _gsDebugAssert(const char *string)
{
#if SDK_FINALROM != 1
	OS_TPanic("%s",string);
#else
	GSI_UNUSED(string);
#endif
}

void gsiDebugPrint(const char* format, va_list params)
{
	VPrintf(format, params);
}

void* _gsi_memalign(size_t boundary, size_t size)
{
	void *ptr = calloc((size)/(boundary), (boundary));
	// check alignment
	GS_ASSERT((((gsi_u32)ptr)% boundary)==0);
	return ptr;
}
