// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
///////////////////////////////////////////////////////////////////////////////
// Common code for GameSpy samples
//   Note:  This code is not intended to be used in a retail program
//
// Portions taken from PS3 sample applications.  Please refer to Sony
// documentation and samples for application startup procedures.

#include <libsysmodule.h>
#include <net.h>
#include <libnetctl.h>
#include <libhttp.h>
#include <np.h>

#include "../gsPlatform.h"
#include "../gsPlatformUtil.h"
#include "../gsMemory.h"

#define SSL_HEAP_SIZE	(300 * 1024)
#define HTTP_HEAP_SIZE	(40 * 1024)

// entry point for GameSpy samples
extern int test_main(int argc, char ** argp); 

gsi_bool _LoadModules()
{
	int ret;

	/* Load Sysmodule */
	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	if (ret < 0) 
	{
		printf("SCE_SYSMODULE_NET Failed!\n");
		return gsi_false;
	}

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
	if (ret < 0) 
	{
		printf("SCE_SYSMODULE_HTTPS Failed!\n");
		return gsi_false;
	}

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NP);
	if (ret < 0) 
	{
		printf("SCE_SYSMODULE_NP Failed!\n");
		return gsi_false;
	}

	return gsi_true;
}

// unloads modules to free up memory
void _UnloadModules()
{
	int ret;

	// Check to see if loaded
	if(sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) == SCE_OK)
	{
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
		if(ret != 0)
		{
			printf("SCE_SYSMODULE_NET Failed! ret = 0x%x\n", ret);
		}
	}

	if(sceSysmoduleIsLoaded(SCE_SYSMODULE_HTTPS) == SCE_OK)
	{
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTPS);
		if(ret != 0)
		{
			printf("SCE_SYSMODULE_HTTPS Failed! ret = 0x%x\n", ret);
		}
	}

	if(sceSysmoduleIsLoaded(SCE_SYSMODULE_NP) == SCE_OK)
	{
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_NP);
		if(ret != 0)
		{
			printf("SCE_SYSMODULE_NP Failed! ret = 0x%x\n", ret);
		}
	}
}


gsi_bool _NetworkInit()
{
	int ret;
	SceNetInitParam param;
	static char memory[16 * 1024];

	param.memory = memory;
	param.size = sizeof(memory);
	param.flags = 0;

	ret = sceNetInit(&param);
	if( ret < 0 )
	{
		printf("sceNetInit Failed!\n");
		return gsi_false;
	}

	ret = sceNetCtlInit();
	if( ret < 0 )
	{
		printf("sceNetCtlInit Failed!\n");
		return gsi_false;
	}

	ret = sceSslInit(SSL_HEAP_SIZE);
	if(ret < 0)
	{
		printf("sceSslInit() failed. ret = 0x%x\n", ret);
		return gsi_false;
	}

	ret = sceHttpInit(HTTP_HEAP_SIZE);
	if(ret < 0)
	{
		printf("sceHttpInit() failed. ret = 0x%x\n", ret);
		return gsi_false;
	}

	return gsi_true;
}

void _NetworkClose()
{
	int ret;

	ret = sceNetTerm();
	if( ret != 0)
	{
		printf("sceNet was not Initialized!\n");
	}

	sceNetCtlTerm();

	ret = sceSslTerm();
	if(ret != 0)
	{
		printf("sceSsl was not Initialized!\n");
	}

	ret = sceHttpTerm();
	if(ret != 0)
	{
		printf("sceHttp was not Initialized!\n");
	}
}

void *  gsiMemManagedInit()
{
	// Init the GSI memory manager (optional - for limiting GSI mem usage)
#if defined GSI_MEM_MANAGED
#define aMemoryPoolSize (1024*1024*8)
	char *aMemoryPool = calloc(aMemoryPoolSize,32);
	if(aMemoryPool == NULL)
	{
		printf("Failed to create memory pool - aborting\r\n");
		return NULL;
	}
	else
	{
		gsMemMgrContext	c = gsMemMgrCreate(gsMemMgrContext_Default, "Default",aMemoryPool, aMemoryPoolSize);
		GSI_UNUSED(c);
	}
	return aMemoryPool;
#else
	return NULL;
#endif

}

void gsiMemManagedClose(void * aMemoryPool)
{
#if defined(GSI_MEM_MANAGED)
	// Optional - Dump memory leaks

	gsi_u32		MemAvail = 	gsMemMgrMemAvailGet			(gsMemMgrContext_Default);
	gsi_u32		MemUsed	 =	gsMemMgrMemUsedGet			(gsMemMgrContext_Default);
	gsi_u32		HwMark	 =	gsMemMgrMemHighwaterMarkGet	(gsMemMgrContext_Default);

	printf("MemAvail %u: MemUsed%u  MemHWMark %u\n", MemAvail,MemUsed,HwMark);
	gsMemMgrDumpStats();
	gsMemMgrDumpAllocations();
	gsMemMgrValidateMemoryPool();
	gsMemMgrDestroy(gsMemMgrContext_Default);
	free(aMemoryPool);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argp)
{
	//sys_pid_t pid = 0;
	//int result = 0;
	//int count = 0;
	//int id_list[10];
	//int i=0;
	void *heap;
	printf("\nGameSpy Test App Initializing\n" 
		"----------------------------------\n");
	/*
	// spawn IO module
	if (sys_process_spawn(&pid, "brio.elf", NULL, 0, PRIO, 0) != SUCCEEDED) {
	printf("sys_process_spawn(brio.elf) failed\n");
	return (0);
	}
	*/

	// load required modules
	if(!_LoadModules())
	{
		printf("_LoadModules() failed\n");
		return (0);
	}

	// initialize network using hardcoded settings above
	if(!_NetworkInit())	
	{
		printf("_NetworkInit() failed\n");
		return (0);
	}

	heap = gsiMemManagedInit();

	// start the actual program
	printf("\nGameSpy Test App Starting\n" 
		"----------------------------------\n");
	test_main(argc, argp);

	// do any needed cleanup
	printf("\nGameSpy Test App Exiting\n" 
		"----------------------------------\n");

	// close up the memory manager
	gsiMemManagedClose(heap);

	// close network
	_NetworkClose();

	// unload modules
	_UnloadModules();

	return 0;
}

