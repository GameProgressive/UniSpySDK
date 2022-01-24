///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatformThread.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef __GSPLATFORMTHREAD_H__
#define __GSPLATFORMTHREAD_H__

#include "gsPlatform.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Thread types
#if defined(_WIN32)
	typedef CRITICAL_SECTION   GSICriticalSection;
	typedef HANDLE GSISemaphoreID;
	typedef HANDLE GSIThreadID;
	typedef DWORD (WINAPI *GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE DWORD WINAPI
#define GS_THREAD_RETURN return (DWORD)0
#define GS_THREAD_RETURN_NEGATIVE return (DWORD)-1
#elif defined(_PS2)
	typedef int GSIThreadID;
	typedef int GSISemaphoreID;
	typedef struct 
	{
		GSISemaphoreID mSemaphore;
		GSIThreadID mOwnerThread;
		gsi_u32 mEntryCount; // track re-entry
		gsi_u32 mPad; // make 16bytes
	} GSICriticalSection;
	typedef void (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return
#elif defined(_NITRO)
	typedef OSMutex GSICriticalSection;
	typedef struct
	{
		OSMutex mLock;
		gsi_i32 mValue;
		gsi_i32 mMax;
	} GSISemaphoreID;
	typedef struct
	{
		OSThread mThread;
		void * mStack;
	} GSIThreadID;
	typedef void (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return
#elif defined(_REVOLUTION)
	typedef OSMutex GSICriticalSection;
	typedef OSSemaphore GSISemaphoreID;
	typedef struct
	{
		OSThread *mThread;
		void * mStack;
	} GSIThreadID;
	typedef void *(*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void *
#define GS_THReAD_RETURN return 0
#define GS_THReAD_RETURN_NEGATIVE return -1
#elif defined(_PSP) || defined (_PSP2)
	// Todo: Test PSP thread code, then remove this define
	#define GSI_NO_THREADS
	typedef int GSIThreadID;
	typedef int GSISemaphoreID;
	typedef struct 
	{
		GSISemaphoreID mSemaphore;
		GSIThreadID mOwnerThread;
		gsi_u32 mEntryCount; // track re-entry
		gsi_u32 mPad; // make 16bytes
	} GSICriticalSection;
	typedef void (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return
#elif defined(_PS3)
	// Todo: Test PS3 ppu thread code, then remove this define
	//#define GSI_NO_THREADS
	typedef sys_ppu_thread_t GSIThreadID;
	typedef int GSISemaphoreID;
	typedef sys_mutex_t GSICriticalSection;
	typedef void (*GSThreadFunc)(uint64_t arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return
#elif defined(ANDROID)

#define GSI_NO_THREADS
	
	//	These are stubs!
	typedef int GSIThreadID;
	typedef int GSISemaphoreID;
	typedef struct 
	{
		GSISemaphoreID mSemaphore;
		GSIThreadID mOwnerThread;
		gsi_u32 mEntryCount; // track re-entry
		gsi_u32 mPad; // make 16bytes
	} GSICriticalSection;
	typedef void (*GSThreadFunc)(void *arg);
#elif defined(_UNIX) //_LINUX || _MACOSX || _IPHONE

// WD : THOMAS : added this so threads would be accessible in ANDROID
// JMH : NOTE: This will not currently get hit with the above code turning off threads
#ifdef ANDROID
#include <unistd.h>
#include <nv_thread/nv_thread.h>
#endif

	typedef pthread_mutex_t GSICriticalSection;
	typedef struct
	{
		pthread_mutex_t mLock;
		gsi_i32 mValue;
		gsi_i32 mMax;
	} GSISemaphoreID;
	typedef struct  
	{
		pthread_t thread;
		pthread_attr_t attr;
	} GSIThreadID;
	typedef void * (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void *
#define GS_THREAD_RETURN return 0
#define GS_THREAD_RETURN_NEGATIVE return -1
#else
	#define GSI_NO_THREADS
#endif

#if defined(WIN32)
	#define GSI_INFINITE INFINITE
#else
	#define GSI_INFINITE (gsi_u32)(-1)
#endif


#if !defined(GSI_NO_THREADS)
	// The increment/read operations must not be preempted
	#if defined(_WIN32)
		#define gsiInterlockedIncrement(a) InterlockedIncrement((long*)a)
		#define gsiInterlockedDecrement(a) InterlockedDecrement((long*)a)
	#elif defined(_PS2)
		gsi_u32 gsiInterlockedIncrement(gsi_u32* num);
		gsi_u32 gsiInterlockedDecrement(gsi_u32* num);
	#elif defined(_PS3)
		gsi_u32 gsiInterlockedIncrement(gsi_u32* num);
		gsi_u32 gsiInterlockedDecrement(gsi_u32* num);
	#elif defined(_NITRO)
		gsi_u32 gsiInterlockedIncrement(gsi_u32* num);
		gsi_u32 gsiInterlockedDecrement(gsi_u32* num);
	#elif defined(_REVOLUTION)
		gsi_u32 gsiInterlockedIncrement(gsi_u32* num);
		gsi_u32 gsiInterlockedDecrement(gsi_u32* num);
	#elif defined(_UNIX)
		gsi_u32 gsiInterlockedIncrement(gsi_u32* num);
		gsi_u32 gsiInterlockedDecrement(gsi_u32* num);
	#endif
	
#else
	// Don't worry about concurrancy when GSI_NO_THREADS is defined
	#define gsiInterlockedIncrement(a) (++(*a))
	#define gsiInterlockedDecrement(a) (--(*a))
#endif



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if !defined(GSI_NO_THREADS)
    int  gsiStartThread(GSThreadFunc aThreadFunc,  gsi_u32 theStackSize, void *arg, GSIThreadID* theThreadIdOut);
    void gsiCancelThread(GSIThreadID theThreadID);
    void gsiExitThread(GSIThreadID theThreadID);
    void gsiCleanupThread(GSIThreadID theThreadID);

    // Thread Synchronization - Startup/Shutdown
    gsi_u32 gsiHasThreadShutdown(GSIThreadID theThreadID);

    // Thread Synchronization - Critical Section
    void gsiInitializeCriticalSection(GSICriticalSection *theCrit);
    void gsiEnterCriticalSection(GSICriticalSection *theCrit);
    void gsiLeaveCriticalSection(GSICriticalSection *theCrit);
    void gsiDeleteCriticalSection(GSICriticalSection *theCrit);

    // Thread Synchronization - Semaphore
    GSISemaphoreID gsiCreateSemaphore(gsi_i32 theInitialCount, gsi_i32 theMaxCount, char* theName);
    gsi_u32        gsiWaitForSemaphore(GSISemaphoreID theSemaphore, gsi_u32 theTimeoutMs);
    void           gsiReleaseSemaphore(GSISemaphoreID theSemaphore, gsi_i32 theReleaseCount);
    void           gsiCloseSemaphore(GSISemaphoreID theSemaphore);

#else
	// NO THREADS - stub everything to unused
    #define gsiStartThread(a, b, c, d) (-1) // must return something
    #define gsiCancelThread(a)
    #define gsiExitThread(a)
    #define gsiCleanupThread(a)

    #define gsiHasThreadShutdown(a) (1)  // must return something

    #define gsiInitializeCriticalSection(a)
    #define gsiEnterCriticalSection(a)
    #define gsiLeaveCriticalSection(a)
    #define gsiDeleteCriticalSection(a)

    #define gsiCreateSemaphore(a,b,c)  (-1)
    #define gsiWaitForSemaphore(a,b) (0)
    #define gsiReleaseSemaphore(a,b)
    #define gsiCloseSemaphore(a)

#endif // GSI_NO_THREADS
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif // __GSPLATFORMTHREAD_H__
