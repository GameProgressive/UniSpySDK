///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatform.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _MACAUTORELEASEPOOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct macAutoreleasePool* macAutoreleasePoolRef;

macAutoreleasePoolRef macAutoreleasePoolCreate();
void macAutoreleasePoolRelease(macAutoreleasePoolRef pool);
int GetPlatformPath(char **filepath);

#ifdef __cplusplus
}
#endif

#endif