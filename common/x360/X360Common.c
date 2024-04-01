///////////////////////////////////////////////////////////////////////////////
// File:	X360Common.c
// SDK:		GameSpy Common Xbox 360 code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
	}
#endif

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

#if (_MSC_VER <= 1300)
	//extern added for vc6 compatability.
	extern void * __cdecl _aligned_malloc(size_t size, int boundary);
	extern void * __cdecl _aligned_free(void * memblock);
#endif
void *  gsiMemManagedInit()
{
// Init the GSI memory manager (optional - for limiting GSI mem usage)
#if defined GSI_MEM_MANAGED
	#define aMemoryPoolSize (1024*1024*4)
	char *aMemoryPool = _aligned_malloc(aMemoryPoolSize,64);
	if(aMemoryPool == NULL)
	{
		printf("Failed to create memory pool - aborting\r\n");
		return NULL;
	}
	{
		gsMemMgrCreate(gsMemMgrContext_Default, "Default",aMemoryPool, aMemoryPoolSize);	
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
	
	#endif
	_aligned_free(theMemoryPool);
}

// ErrorMessage: Displays message and goes into an infinite loop
// continues rendering
void  _gsDebugAssert(const char *string)
{
	//DebugBreak();
	OutputDebugString( string);
	exit(0);
}
	
// sample common entry point
extern int test_main(int argc, char ** argp); 


// Common entry point
int __cdecl main(int argc, char** argp)
{
	int		ret		= 0;
	// set up memanager
	void	*heap	= gsiMemManagedInit();

	#ifdef GSI_COMMON_DEBUG
		// Set up debugging
		gsSetDebugCallback(DebugCallback);
		gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All,    GSIDebugLevel_Verbose);
	#endif

	ret = test_main(argc, argp);

	gsiMemManagedClose(heap);

	return ret;
}




