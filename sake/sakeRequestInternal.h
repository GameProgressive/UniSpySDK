///////////////////////////////////////////////////////////////////////////////
// File:	sakeRequestInternal.h
// SDK:		GameSpy Sake Persistent Storage SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SAKEREQUESTINTERNAL_H__
#define __SAKEREQUESTINTERNAL_H__

#include "sakeMain.h"
#include "../ghttp/ghttpSoap.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define SAKEI_REQUEST_SAFE_MALLOC(dest, type) SAKEI_REQUEST_SAFE_MALLOC_ARRAY(dest, type, 1)
#define SAKEI_REQUEST_SAFE_MALLOC_ARRAY(dest, type, num) {\
	dest = (type*)gsimalloc(sizeof(type)*num); /*malloc*/ \
	if(!dest) goto out_of_mem_cleanup;         /*check*/  \
	memset(dest, 0, sizeof(type)*num); }       /*zero*/

#define SAKEI_FUNC_NAME_STRINGS(func) func,\
	"SOAPAction: \"http://gamespy.net/sake/" func "\"",\
	func "Response",\
	func "Result"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	size_t      mSakeOutputSize;
	const char *mFuncName;
	const char *mSoapAction;
	const char *mResponseTag;
	const char *mResultTag;

	SAKEStartRequestResult (*mValidateInputFunc)(SAKERequest request);
	SAKEStartRequestResult (*mFillSoapRequestFunc)(SAKERequest request);
	SAKERequestResult      (*mProcessSoapResponseFunc)(SAKERequest request);
	void                   (*mFreeDataFunc)(SAKERequest request);
} SAKEIRequestInfo;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SAKEStartRequestResult SAKE_CALL sakeiStartRequest(SAKERequest request, SAKEIRequestInfo * info);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SAKEREQUESTINTERNAL_H__
