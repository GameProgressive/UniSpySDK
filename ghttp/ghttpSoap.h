///////////////////////////////////////////////////////////////////////////////
// File:	gsSoap.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SOAP_H__
#define __SOAP_H__

#include "ghttpMain.h"
#include "../common/gsCore.h"

#if defined(__cplusplus)
extern "C"
{
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef void(*GSSoapCallbackFunc)(GHTTPResult theHTTPResult, GSXmlStreamWriter theRequest, GSXmlStreamReader theResponse, void *theUserData);
typedef void(*GSSoapCustomFunc)(GHTTPPost theSoap, void* theUserData);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct 
{
	GSSoapCallbackFunc mCallbackFunc;
	GSSoapCustomFunc mCustomFunc;
	const char *mURL;
	const char *mService;

	GSXmlStreamWriter mRequestSoap;
	GSXmlStreamReader mResponseSoap;

	char *    mResponseBuffer; // so we can free it later
	char *    mHeadersBuffer;
	GHTTPPost mPostData; // so we can free it later

	void *   mUserData;
	GSTask * mCoreTask;

	GHTTPRequest mRequestId;
	GHTTPResult  mRequestResult;
	gsi_bool     mCompleted;
} GSSoapTask;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap call (Uses GameSpy core object)
GSSoapTask* gsiExecuteSoap(const char *theURL, const char *theService,
					 GSXmlStreamWriter theSoapData, GSSoapCallbackFunc theCallbackFunc,
					 void *theUserData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap call (Uses GameSpy core object)
GSSoapTask* gsiExecuteSoapWithTimeout(const char *theURL, const char *theService,
						   GSXmlStreamWriter theSoapData, GSSoapCallbackFunc theCallbackFunc,
						   gsi_time theTimeoutMs, void *theUserData);

// Alternate version with GSSoapCustomFunc parameter allows client access
// to soap object to set DIME attachments
GSSoapTask* gsiExecuteSoapCustom(const char* theURL, const char* theService, 
					 GSXmlStreamWriter theSoapData, GSSoapCallbackFunc theCallbackFunc, 
					 GSSoapCustomFunc theCustomFunc, void* theUserData);


void gsiCancelSoap(GSSoapTask * theTask);

// Parse the headers string to look for 'SessionToken' and grab the value
gsi_bool gsiSoapGetSessionTokenFromHeaders(const char *headers, char valueBuffer[SESSIONTOKEN_LENGTH]);

GSAuthErrorCode gsiSoapGetAuthErrorFromHeaders(char *headers, char valueBuffer[AUTHERROR_LENGTH]);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif

#endif // __SOAP_H__
