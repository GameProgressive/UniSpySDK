///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatform.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#import "macAutoreleasePool.h"
#import <Foundation/Foundation.h>

macAutoreleasePoolRef macAutoreleasePoolCreate()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	return (macAutoreleasePoolRef)pool;
}

void macAutoreleasePoolRelease(macAutoreleasePoolRef pool)
{
	if (pool != NULL)
		[(NSAutoreleasePool*)pool release];
}

int GetPlatformPath(char **filepath)
{
	static char* documentsPathCString = NULL;
	static size_t documentsPathCStringLen = 0;
	
	if (documentsPathCString == NULL)
	{
		macAutoreleasePoolRef pool = macAutoreleasePoolCreate();	// CF types need this for automatic cleanup.
		CFArrayRef paths = (CFArrayRef)NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		CFStringRef documentsPathString = (CFStringRef)CFArrayGetValueAtIndex(paths, 0);
		CFIndex bufferSize = CFStringGetMaximumSizeOfFileSystemRepresentation(documentsPathString);
		char* buf = (char*)malloc(bufferSize);
		CFStringGetFileSystemRepresentation(documentsPathString, buf, bufferSize);
		macAutoreleasePoolRelease(pool);
		documentsPathCString = realloc(buf, strlen(buf) + 1);
		
		if (documentsPathCString != buf)
			free(buf);
		
		documentsPathCStringLen = strlen(buf);
	}
	
	*filepath = documentsPathCString;
	
	return documentsPathCStringLen;
}
