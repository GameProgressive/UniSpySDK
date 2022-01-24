///////////////////////////////////////////////////////////////////////////////
// File:	ghttpSoap.c
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "ghttpSoap.h"
#include "ghttpASCII.h"

// GAMESPY DEVELOPERS ->  Use gsiExecuteSoap


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Soap task delegates
static void gsiSoapTaskExecute(void* theTask);
static void gsiSoapTaskCallback(void* theTask, GSTaskResult theResult);
static void gsiSoapTaskCancel(void* theTask);
static gsi_bool gsiSoapTaskCleanup(void* theTask);
static GSTaskResult gsiSoapTaskThink(void* theTask);

// Http triggered callbacks (don't take action now, wait for task callbacks)
static GHTTPBool gsiSoapTaskHttpCompletedCallback(GHTTPRequest request, GHTTPResult result, 
											 char * buffer, GHTTPByteCount bufferLen, char * headers, 
											 void * param);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap function (this should be the only call made from other SDKs)
GSSoapTask* gsiExecuteSoap(const char* theURL, const char* theService,
					 GSXmlStreamWriter theRequestSoap, GSSoapCallbackFunc theCallbackFunc, 
					 void* theUserData)
{
	return gsiExecuteSoapWithTimeout(theURL, theService, theRequestSoap, theCallbackFunc, 0, theUserData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap function (this should be the only call made from other SDKs)
GSSoapTask* gsiExecuteSoapWithTimeout(const char* theURL, const char* theService,
						   GSXmlStreamWriter theRequestSoap, GSSoapCallbackFunc theCallbackFunc, 
						   gsi_time theTimeoutMs, void* theUserData)
{
	GSSoapTask* aSoapTask = NULL;
	GSTask* aCoreTask = NULL;

	aSoapTask = (GSSoapTask*)gsimalloc(sizeof(GSSoapTask));
	if (aSoapTask == NULL)
		return NULL; // out of memory

	aSoapTask->mCallbackFunc = theCallbackFunc;
	aSoapTask->mCustomFunc   = NULL;
	aSoapTask->mURL          = theURL;
	aSoapTask->mService      = theService;
	aSoapTask->mRequestSoap  = theRequestSoap;
	aSoapTask->mPostData     = NULL;
	aSoapTask->mResponseSoap = NULL;
	aSoapTask->mResponseBuffer = NULL;
	aSoapTask->mUserData     = theUserData;
	aSoapTask->mRequestResult= (GHTTPResult)0;
	aSoapTask->mCompleted    = gsi_false;

	aCoreTask = gsiCoreCreateTask();
	if (aCoreTask == NULL)
	{
		gsifree(aSoapTask);
		return NULL; // out of memory
	}

	aCoreTask->mCallbackFunc = gsiSoapTaskCallback;
	aCoreTask->mExecuteFunc  = gsiSoapTaskExecute;
	aCoreTask->mThinkFunc    = gsiSoapTaskThink;
	aCoreTask->mCleanupFunc  = gsiSoapTaskCleanup;
	aCoreTask->mCancelFunc   = gsiSoapTaskCancel;
	aCoreTask->mTaskData     = (void*)aSoapTask;

	aSoapTask->mCoreTask = aCoreTask;

	gsiCoreExecuteTask(aCoreTask, theTimeoutMs);

	return aSoapTask;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap function with a GSSoapCustomFunc that can access the soap
// structure prior to execution. This allows the client to set DIME
// attachments. (The GSSoapCustomFunc parameter could be added to
// gsiExecuteSoap itself as long as existing client code is updated)
GSSoapTask* gsiExecuteSoapCustom(const char* theURL, const char* theService,
					 GSXmlStreamWriter theRequestSoap, GSSoapCallbackFunc theCallbackFunc, 
					 GSSoapCustomFunc theCustomFunc, void* theUserData)
{
	GSSoapTask* aSoapTask = NULL;
	GSTask* aCoreTask = NULL;

	aSoapTask = (GSSoapTask*)gsimalloc(sizeof(GSSoapTask));
	aSoapTask->mCallbackFunc = theCallbackFunc;
	aSoapTask->mCustomFunc   = theCustomFunc;
	aSoapTask->mURL          = theURL;
	aSoapTask->mService      = theService;
	aSoapTask->mRequestSoap  = theRequestSoap;
	aSoapTask->mPostData     = NULL;
	aSoapTask->mResponseSoap = NULL;
	aSoapTask->mResponseBuffer = NULL;
	aSoapTask->mUserData     = theUserData;
	aSoapTask->mRequestResult= (GHTTPResult)0;
	aSoapTask->mCompleted    = gsi_false;
	
	aCoreTask = gsiCoreCreateTask();
	aCoreTask->mCallbackFunc = gsiSoapTaskCallback;
	aCoreTask->mExecuteFunc  = gsiSoapTaskExecute;
	aCoreTask->mThinkFunc    = gsiSoapTaskThink;
	aCoreTask->mCleanupFunc  = gsiSoapTaskCleanup;
	aCoreTask->mCancelFunc   = gsiSoapTaskCancel;
	aCoreTask->mTaskData     = (void*)aSoapTask;

	aSoapTask->mCoreTask = aCoreTask;

	gsiCoreExecuteTask(aCoreTask, 0);

	return aSoapTask;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Cancels a soap task.  
//    - Because of network race conditions, the task may complete before it 
//      can be cancelled. If this happens, the task callback will be triggered
//      with status GHTTPRequestCancelled and the result data will be discarded.
void gsiCancelSoap(GSSoapTask * theTask)
{
	GS_ASSERT(theTask != NULL);

	// Still in progress? cancel it!
	if (gsi_is_false(theTask->mCompleted))
		gsiCoreCancelTask(theTask->mCoreTask);
}

// Check soap response headers for "Error:" - eg "Error: Invalid Access Key" - and "ErrorCode:"
GSAuthErrorCode gsiSoapGetAuthErrorFromHeaders(char *headers, char valueBuffer[AUTHERROR_LENGTH])
{
	char * header;
	char * errorCodeString;
	char * headersCopy;
	GSAuthErrorCode errorCode = GSAuthErrorCode_None;
	const char * errorHeaderName = AUTHERROR_HEADER;
	const char * errorCodeHeaderName = AUTHERRORCODE_HEADER;

	GS_ASSERT(headers);

	headersCopy = goastrdup(headers);

	// find the error header in the list of headers
	header = strstr(headers, errorHeaderName);
	if(header)
	{
		char * tempValueBuffer;
		// skip the header name
		header += strlen(errorHeaderName);

		tempValueBuffer = strtok(header, "\n");
		tempValueBuffer++; // remove leading space
		
		strcpy(valueBuffer, tempValueBuffer);
	}

	// find the errorCode header in the list of headers
	header = strstr(headersCopy, errorCodeHeaderName);
	if(header)
	{
		// skip the header name
		header += strlen(errorCodeHeaderName);

		errorCodeString = strtok(header, "\n");
		errorCodeString++; // remove leading space

		errorCode = (GSAuthErrorCode) atoi(errorCodeString);
	}
	
	gsifree(headersCopy);
	headersCopy = NULL;

	return errorCode;
}

// Check soap response headers for "SessionToken:" - passed after Access Key is verified by apigee
gsi_bool gsiSoapGetSessionTokenFromHeaders(const char *headers, char valueBuffer[SESSIONTOKEN_LENGTH])
{
	char * header;
	int rcode;
	const char * headerName = SESSIONTOKEN_HEADER;

	GS_ASSERT(headers);

	// find this header in the list of headers
	header = strstr(headers, headerName);
	if(header)
	{
		// skip the header name
		header += strlen(headerName);

		// scan in the result
		rcode = sscanf(header, " %36s", valueBuffer);
		
		if(rcode == 1)
			return gsi_true;
	}

	return gsi_false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
				    //////////  HTTP CALLBACKS  //////////

static GHTTPBool gsiSoapTaskHttpCompletedCallback(GHTTPRequest request, GHTTPResult result, 
											 char * buffer, GHTTPByteCount bufferLen, char * headers, 
											 void * param)
{
	gsi_bool parseResult = gsi_false;
	char sessionToken[SESSIONTOKEN_LENGTH];
	char authError[AUTHERROR_LENGTH];
	GSAuthErrorCode errorCode;

	GSSoapTask* aSoapTask = (GSSoapTask*)param;
	aSoapTask->mRequestResult = result;
	aSoapTask->mCompleted = gsi_true;
	aSoapTask->mResponseBuffer = buffer;
	aSoapTask->mHeadersBuffer = headers;

	// check response headers for error
	errorCode = gsiSoapGetAuthErrorFromHeaders(headers, authError);
	if (errorCode != GSAuthErrorCode_None)
	{
		gsiCoreSetAuthError(authError);
		gsiCoreSetAuthErrorCode(errorCode);
	}

	// set session token if we have one in the response headers (eg. after auth service login)
	if (gsiSoapGetSessionTokenFromHeaders(headers, sessionToken))
		gsiCoreSetSessionToken(sessionToken);

	if (result == GHTTPSuccess)
	{
		aSoapTask->mResponseSoap = gsXmlCreateStreamReader();
		if (aSoapTask->mResponseSoap == NULL)
		{
			// OOM!
			aSoapTask->mRequestResult = GHTTPOutOfMemory;
		}
		else
		{
			parseResult = gsXmlParseBuffer(aSoapTask->mResponseSoap, buffer, (int)bufferLen);
			if (gsi_is_false(parseResult))
			{
				// Todo: handle multiple error conditions
				aSoapTask->mRequestResult = GHTTPBadResponse;
			}
		}
	}

	GSI_UNUSED(request);

	return GHTTPFalse; // don't let http free the buffer
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
				//////////  SOAP EXECUTE TASK  //////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Checks to see if the soap task has completed
//   - return GSTaskResult_InProgress for "keep checking"
//   - return anything else           for "finished - trigger callback and delete"
static GSTaskResult gsiSoapTaskThink(void* theTask)
{
	// is the request still processing?
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;
	if (gsi_is_true(aSoapTask->mCompleted))
		return GSTaskResult_Finished;
	else
	{
		ghttpRequestThink(aSoapTask->mRequestId);
		return GSTaskResult_InProgress;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Spawns the soap thread and begins execution
static void gsiSoapTaskExecute(void* theTask)
{
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;
	//int threadID = 0;
	char headerString[HEADERS_LENGTH];
	char sessionToken[SESSIONTOKEN_LENGTH];
	char gameIdString[GAMEID_LENGTH];
	char profileIdString[PROFILEID_LENGTH];
	//int gameIdLength = intDigits(gameId);
	//char * gameIdString;

	// make sure we aren't reusing a task without first resetting it
	GS_ASSERT(gsi_is_false(aSoapTask->mCompleted));

	aSoapTask->mPostData = ghttpNewPost();
	if (aSoapTask->mPostData == NULL)
	{
		// OOM: abort task
		aSoapTask->mCompleted = gsi_true;
		aSoapTask->mRequestResult = GHTTPOutOfMemory;
		return;
	}

	ghttpPostSetAutoFree(aSoapTask->mPostData, GHTTPFalse);
	ghttpPostAddXml(aSoapTask->mPostData, aSoapTask->mRequestSoap);

	// Allow client to further configure soap object if desired
	if (aSoapTask->mCustomFunc != NULL)
		(aSoapTask->mCustomFunc)(aSoapTask->mPostData, aSoapTask->mUserData);

	// make sure to include session token (and other stuff) in headers if we have one set 
	// (needed for apigee verification)
	strcpy(headerString, aSoapTask->mService);
	if (gsiCoreGetSessionToken(sessionToken))
	{
		strcat(headerString, "\r\n" SESSIONTOKEN_HEADER " ");
		strcat(headerString, sessionToken);
	}
	if (gsiCoreGetGameId(gameIdString))
	{
		strcat(headerString, "\r\n" GAMEID_HEADER " ");
		strcat(headerString, gameIdString);
	}
	if (gsiCoreGetProfileId(profileIdString))
	{
		strcat(headerString, "\r\n" PROFILEID_HEADER " ");
		strcat(headerString, profileIdString);
	}

	aSoapTask->mRequestId = ghttpGetExA(aSoapTask->mURL, headerString, 
		NULL, 0, aSoapTask->mPostData, GHTTPFalse, GHTTPFalse, NULL,
		gsiSoapTaskHttpCompletedCallback, (void*)aSoapTask);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called when the soap task needs to be canceled
static void gsiSoapTaskCancel(void* theTask)
{
	GSSoapTask * soapTask = (GSSoapTask*)theTask;
	if (gsi_is_false(soapTask->mCompleted))
	{
		if (soapTask->mRequestId >= 0)
			ghttpCancelRequest(soapTask->mRequestId);
		soapTask->mRequestResult = GHTTPRequestCancelled;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called when the soap task completes or is cancelled/timed out
static void gsiSoapTaskCallback(void* theTask, GSTaskResult theResult)
{
	// Call the developer callback
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;

	//ghttpGetHeaders(aSoapTask->mRequestId); // http 'connection' not in use so this returns null..

	(aSoapTask->mCallbackFunc)(aSoapTask->mRequestResult, aSoapTask->mRequestSoap, 
		aSoapTask->mResponseSoap, aSoapTask->mUserData);

	GSI_UNUSED(theResult);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// After the soap call has completed, launch a separate cleanup event (see comments)
static gsi_bool gsiSoapTaskCleanup(void *theTask)
{
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;

	if (aSoapTask->mResponseSoap != NULL)
		gsXmlFreeReader(aSoapTask->mResponseSoap);
	if (aSoapTask->mResponseBuffer != NULL)
		gsifree(aSoapTask->mResponseBuffer);
	if (aSoapTask->mPostData != NULL)
	{
	    
		ghttpFreePostAndUpdateConnection(aSoapTask->mRequestId, aSoapTask->mPostData); // this also frees the request soap xml
    }
	gsifree(aSoapTask);
	
	return gsi_true;
}
