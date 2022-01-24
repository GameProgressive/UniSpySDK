///////////////////////////////////////////////////////////////////////////////
// File:	ghttpPost.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#ifndef _GHTTPPOST_H_
#define _GHTTPPOST_H_

#include "ghttp.h"
#include "ghttpBuffer.h"
#include "../common/darray.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	GHIPostingError,
	GHIPostingDone,
	GHIPostingPosting,
	GHIPostingWaitForContinue
} GHIPostingResult;

typedef struct GHIPostingState
{
	DArray states;
	int index;
	int bytesPosted;
	int totalBytes;
	ghttpPostCallback callback;
	void * param;
	GHTTPBool waitPostContinue; // does DIME need to wait for continue?
	GHTTPBool completed; // prevent re-post in the event of a redirect.
} GHIPostingState;

GHTTPPost ghiNewPost
(
	void
);

void ghiPostSetAutoFree
(
	GHTTPPost post,
	GHTTPBool autoFree
);

GHTTPBool ghiIsPostAutoFree
(
	GHTTPPost post
);

void ghiFreePost
(
	GHTTPPost post
);

void ghiFreePostAndUpdateConnection
(
    GHTTPRequest requestId,
    GHTTPPost post
);

GHTTPBool ghiPostAddString
(
	GHTTPPost post,
	const char * name,
	const char * string
);

GHTTPBool ghiPostAddFileFromDisk
(
	GHTTPPost post,
	const char * name,
	const char * filename,
	const char * reportFilename,
	const char * contentType
);

GHTTPBool ghiPostAddFileFromMemory
(
	GHTTPPost post,
	const char * name,
	const char * buffer,
	int bufferLen,
	const char * reportFilename,
	const char * contentType
);

GHTTPBool ghiPostAddXml
(
	GHTTPPost post,
	GSXmlStreamWriter xmlSoap
);

void ghiPostSetCallback
(
	GHTTPPost post,
	ghttpPostCallback callback,
	void * param
);

const char * ghiPostGetContentType
(
	struct GHIConnection * connection
);

GHTTPBool ghiPostInitState
(
	struct GHIConnection * connection
);

void ghiPostCleanupState
(
	struct GHIConnection * connection
);

GHIPostingResult ghiPostDoPosting
(
	struct GHIConnection * connection
);

#ifdef __cplusplus
}
#endif

#endif
