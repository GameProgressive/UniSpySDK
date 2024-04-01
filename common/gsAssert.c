///////////////////////////////////////////////////////////////////////////////
// File:	gsAssert.c
// SDK:		GameSpy Common
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "gsCommon.h"
#include "gsDebug.h"

// This is the platform specific default assert condition handler
extern void _gsDebugAssert(const char * string);

static gsDebugAssertCallback gsDebugAssertHandler = _gsDebugAssert;

//  Call this function to override the default assert handler  
//	New function should render message / log message based on string passed
void gsDebugAssertCallbackSet(gsDebugAssertCallback theCallback)
{
	gsDebugAssertHandler = theCallback ? theCallback : _gsDebugAssert;
}


// This is the default assert condition handler
void gsDebugAssert(const char *szError,
	const char *szText, const char *szFile, int line)
{
	char String[256];
	// format into buffer 
	sprintf(&String[0], szError,szText,szFile,line);

	// call plat specific handler
	(*gsDebugAssertHandler)(String);
}


// ****************************************************