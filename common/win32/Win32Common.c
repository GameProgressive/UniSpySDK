///////////////////////////////////////////////////////////////////////////////
// File:	Win32Common.c
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "../gsCommon.h"
#include "../gsMemory.h"
#include "../gsDebug.h"
#include <malloc.h>
// Debug output
#ifdef GSI_COMMON_DEBUG
	static void DebugCallback(GSIDebugCategory theCat, GSIDebugType theType,
	                          GSIDebugLevel theLevel, const char * theTokenStr,
	                          va_list theParamList)

	{
		GSI_UNUSED(theLevel);
		{
			static char    string[256];
			vsprintf(string, theTokenStr, theParamList); 			
			OutputDebugString(string);
		}
		printf("[%s][%s] ", 
				gGSIDebugCatStrings[theCat], 
				gGSIDebugTypeStrings[theType]);

		vprintf(theTokenStr, theParamList);
	}
#endif

#if (_MSC_VER <= 1300)
	//extern added for vc6 compatibility.
	extern void * __cdecl _aligned_malloc(size_t size, size_t boundary);
	extern void __cdecl _aligned_free(void * memblock);
#endif

void *  gsiMemManagedInit()
{
// Init the GSI memory manager (optional - for limiting GSI mem usage)
#if defined GSI_MEM_MANAGED
	#define aMemoryPoolSize (1024*1024*4)
	char *aMemoryPool = (char *)_aligned_malloc(aMemoryPoolSize,64);
	if(aMemoryPool == NULL)
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_HotError,
			"Failed to create memory pool - aborting\r\n");
		return NULL;
	}
	{
		gsMemMgrCreate(gsMemMgrContext_Default, "Default", aMemoryPool, aMemoryPoolSize);	
	}
	return aMemoryPool;
#else
	return NULL;
#endif

}

void gsiMemManagedClose(void * theMemoryPool)
{
	#if defined(GSI_MEM_MANAGED)
	// Optional - Dump memory leaks

	gsi_u32		MemAvail = 	gsMemMgrMemAvailGet			(gsMemMgrContext_Default);
	gsi_u32		MemUsed	 =	gsMemMgrMemUsedGet			(gsMemMgrContext_Default);
	gsi_u32		HwMark	 =	gsMemMgrMemHighwaterMarkGet	(gsMemMgrContext_Default);

	gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Memory, GSIDebugLevel_Comment,
				"MemAvail %u: MemUsed%u  MemHWMark %u\n", MemAvail,MemUsed,HwMark);
	gsMemMgrDumpStats();
	gsMemMgrDumpAllocations();
	gsMemMgrValidateMemoryPool();
	gsMemMgrDestroy(gsMemMgrContext_Default);
	#endif
	_aligned_free(theMemoryPool);
}


// sample common entry point
extern int test_main(int argc, char ** argp); 


// Common entry point
int __cdecl main(int argc, char** argp)
{
	int ret = 0;
	void *heap = NULL;

	#ifdef GSI_COMMON_DEBUG
		// Set up debugging
		gsSetDebugCallback(DebugCallback);
		gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All,    GSIDebugLevel_Verbose);
	#endif

	// set up memanager
	heap = gsiMemManagedInit();

	ret = test_main(argc, argp);

	gsiMemManagedClose(heap);

	return ret;
}
