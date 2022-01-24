///////////////////////////////////////////////////////////////////////////////
// File:	ghttpCallbacks.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#ifndef _GHTTPCALLBACKS_H_
#define _GHTTPCALLBACKS_H_

#include "ghttpMain.h"
#include "ghttpConnection.h"

#ifdef __cplusplus
extern "C" {
#endif

// Call the completed callback for this connection.
void ghiCallCompletedCallback
(
	GHIConnection * connection
);

// Call the progress callback for this connection.
void ghiCallProgressCallback
(
	GHIConnection * connection,
	const char * buffer,
	GHTTPByteCount bufferLen
);

// Call the post callback for this connection.
void ghiCallPostCallback
(
	GHIConnection * connection
);

#ifdef __cplusplus
}
#endif

#endif
