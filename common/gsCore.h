///////////////////////////////////////////////////////////////////////////////
// File:	gsCore.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ------------------------------------
// Core task/callback manager.

#ifndef __GSCORE_H__
#define __GSCORE_H__

#include "gsCommon.h"
#include "darray.h"

#if defined(__cplusplus)
extern "C"
{
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define GSICORE_DYNAMIC_TASK_LIST
#define GSICORE_MAXTASKS       40
#define SESSIONTOKEN_LENGTH    37
#define SESSIONTOKEN_HEADER    "SessionToken:"
#define AUTHERROR_LENGTH       100
#define AUTHERROR_HEADER       "Error:"
#define AUTHERRORCODE_LENGTH   3
#define AUTHERRORCODE_HEADER   "ErrorCode:"
#define GAMEID_LENGTH          10
#define GAMEID_HEADER          "GameID:"
#define PROFILEID_LENGTH       20
#define PROFILEID_HEADER       "ProfileID:"
#define HEADERS_LENGTH         300
#define ACCESSKEY_HEADER  "AccessKey:"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
	GSCore_IN_USE,
	GSCore_SHUTDOWN_PENDING,
	GSCore_SHUTDOWN_COMPLETE
} GSCoreValue;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
	GSTaskResult_None,
	GSTaskResult_InProgress,
	GSTaskResult_Canceled,
	GSTaskResult_TimedOut,
	GSTaskResult_Finished
} GSTaskResult;

typedef enum
{
	GSAuthErrorCode_None,
	GSAuthErrorCode_InvalidGameID,
	GSAuthErrorCode_InvalidAccessKey,
	GSAuthErrorCode_InvalidSessionToken,
	GSAuthErrorCode_SessionTokenExpired,
	GSAuthErrorCode_Unknown
} GSAuthErrorCode;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// delegates (optional, may be NULL)
typedef void(*GSTaskExecuteFunc) (void* theTaskData);
typedef void(*GSTaskCallbackFunc)(void* theTaskData, GSTaskResult theResult);
typedef void(*GSTaskCancelFunc)  (void* theTaskData);
typedef gsi_bool(*GSTaskCleanupFunc) (void* theTaskData); // post run cleanup
typedef GSTaskResult(*GSTaskThinkFunc)(void* theTaskData);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// "Private" struct for dispatching tasks.  Once tasks have been put in the queue
// they should only be modified from the think thread.
//      - When creating a task, you should set only the task data and delegates
typedef struct 
{
	int mId;
	gsi_time mTimeout;
	gsi_time mStartTime;
	gsi_bool mAutoThink;

	// These are not exclusive states (use bit flags?)
	gsi_i32  mIsStarted;   
	gsi_i32  mIsRunning;
	gsi_i32  mIsCanceled;
	gsi_i32  mIsCallbackPending; // does the task require a callback?

	// delegates
	void* mTaskData;
	GSTaskExecuteFunc  mExecuteFunc;
	GSTaskCallbackFunc mCallbackFunc;
	GSTaskCancelFunc   mCancelFunc;
	GSTaskCleanupFunc  mCleanupFunc;
	GSTaskThinkFunc    mThinkFunc;
} GSTask;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct 
{
	gsi_u32  mRefCount;

	// these are retrieved from the response headers after apigee verfies the api key
	char sessionToken[SESSIONTOKEN_LENGTH];
	char authError[AUTHERROR_LENGTH];
	GSAuthErrorCode authErrorCode;
	char gameId[GAMEID_LENGTH];
	char profileId[PROFILEID_LENGTH];

	gsi_bool volatile mIsStaticInitComplete;  // once per application init
	gsi_bool volatile mIsInitialized;  // gsi_true when ready to use
	gsi_bool volatile mIsShuttingDown; // gsi_true when shutting down

	GSICriticalSection mQueueCrit;
	#ifdef GSICORE_DYNAMIC_TASK_LIST
		DArray mTaskArray;
	#else
		GSTask* mTaskArray[GSICORE_MAXTASKS];
	#endif
		
} GSCoreMgr;


///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////// 
COMMON_API void gsCoreInitialize       (void);
void gsiCoreSetProfileId(const char profileId[PROFILEID_LENGTH]);
gsi_bool gsiCoreGetProfileId(char profileId[PROFILEID_LENGTH]);
void gsiCoreSetGameId(const char gameId[GAMEID_LENGTH]);
gsi_bool gsiCoreGetGameId(char gameId[GAMEID_LENGTH]);
void gsiCoreSetAuthError(const char* authError);
gsi_bool gsiCoreGetAuthError(char authError[AUTHERROR_LENGTH]);
void gsiCoreSetAuthErrorCode(GSAuthErrorCode authErrorCode);
GSAuthErrorCode gsiCoreGetAuthErrorCode();
gsi_bool gsiCoreGetSessionToken(char sessionToken[SESSIONTOKEN_LENGTH]);
void gsiCoreSetSessionToken (const char sessionToken[SESSIONTOKEN_LENGTH]);
COMMON_API void gsCoreThink            (gsi_time theMS);
COMMON_API void gsCoreShutdown         (void);
COMMON_API GSCoreValue gsCoreIsShutdown(void);

GSTaskResult gsCoreTaskThink(GSTask* theTask);
void gsiCoreExecuteTask     (GSTask* theTask, gsi_time theTimeoutMs);
COMMON_API void gsiCoreCancelTask      (GSTask* theTask);

GSTask* gsiCoreCreateTask(void);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
}
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __GSCORE_H__
