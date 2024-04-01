///////////////////////////////////////////////////////////////////////////////
// File:	gsSocketX360.c
// SDK:		GameSpy Common Xbox 360 code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsPlatformUtil.h"
#include "../gsPlatformThread.h"
#include "../gsAssert.h"
#include "../gsDebug.h"



void gsiInitializeCriticalSection(GSICriticalSection *theCrit) { InitializeCriticalSection(theCrit); }
void gsiEnterCriticalSection     (GSICriticalSection *theCrit) { EnterCriticalSection(theCrit);      }
void gsiLeaveCriticalSection     (GSICriticalSection *theCrit) { LeaveCriticalSection(theCrit);      }
void gsiDeleteCriticalSection    (GSICriticalSection *theCrit) { DeleteCriticalSection(theCrit);     }

gsi_u32 gsiHasThreadShutdown(GSIThreadID theThreadID) 
{ 
	DWORD result = WaitForSingleObject(theThreadID, 0); 
	if (result == WAIT_ABANDONED || result == WAIT_OBJECT_0)
		return 1; // thread is dead
	else
		return 0; // keep waiting
}

GSISemaphoreID gsiCreateSemaphore(gsi_i32 theInitialCount, gsi_i32 theMaxCount, char* theName)
{
	GSISemaphoreID aSemaphore = CreateSemaphoreA(NULL, theInitialCount, theMaxCount, theName);
	if (aSemaphore == NULL)
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"Failed to create semaphore\r\n");
	}
	return aSemaphore;
}

// Waits for -- and signals -- the semaphore
gsi_u32 gsiWaitForSemaphore(GSISemaphoreID theSemaphore, gsi_u32 theTimeoutMs)
{
	DWORD result = WaitForSingleObject((HANDLE)theSemaphore, (DWORD)theTimeoutMs);
	return (gsi_u32)result;
}

// Allow other objects to access the semaphore
void gsiReleaseSemaphore(GSISemaphoreID theSemaphore, gsi_i32 theReleaseCount)
{
	ReleaseSemaphore(theSemaphore, theReleaseCount, NULL);
}

void gsiCloseSemaphore(GSISemaphoreID theSemaphore)
{
	CloseHandle(theSemaphore);
}


int gsiStartThread(GSThreadFunc func, gsi_u32 theStackSize, void *arg, GSIThreadID * id)
{
	HANDLE handle;
	DWORD threadID;

	// create the thread
	handle = CreateThread(NULL, theStackSize, func, arg, 0, &threadID);
	if(handle == NULL)
		return -1;

	// store the id
	*id = handle;

	return 0;
}

void gsiCancelThread(GSIThreadID id)
{
	//there is no way to terminate a thread
	//we should move to always signalling threads to close
	GSI_UNUSED(id);
}

void gsiExitThread(GSIThreadID id)
{
	GSI_UNUSED(id);
}

void gsiCleanupThread(GSIThreadID id)
{
	CloseHandle(id);
}


