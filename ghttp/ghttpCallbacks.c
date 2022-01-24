///////////////////////////////////////////////////////////////////////////////
// File:	ghttpCallbacks.c
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#include "ghttpCallbacks.h"
#include "ghttpPost.h"

void ghiCallCompletedCallback
(
	GHIConnection * connection
)
{
	GHTTPBool freeBuffer;
	char * buffer;
	GHTTPByteCount bufferLen;

	GS_ASSERT(connection);
	
#ifdef GSI_COMMON_DEBUG
	if(connection->result != GHTTPSuccess)
	{
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Socket Error: %d\n", connection->socketError);
	}
#endif

	// Check for no callback.
	if(!connection->completedCallback)
		return;

	// Figure out the buffer/bufferLen parameters.
	if(connection->type == GHIGET)
	{
		buffer = connection->getFileBuffer.data;
	}
	else
	{
		buffer = NULL;
	}
	bufferLen = connection->fileBytesReceived;

	// This prevents us from passing a null pointer to the callback.
	if (connection->recvHeaders == NULL)
		connection->recvHeaders = goastrdup("");

	// Call the callback.
	freeBuffer = connection->completedCallback(
		connection->request,
		connection->result,
		buffer,
		bufferLen,
		connection->recvHeaders,
		connection->callbackParam);

	// Check for gsifree.
	if(buffer && !freeBuffer)
		connection->getFileBuffer.dontFree = GHTTPTrue;
}

void ghiCallProgressCallback
(
	GHIConnection * connection,
	const char * buffer,
	GHTTPByteCount bufferLen
)
{	
	GS_ASSERT(connection);

	// Check for no callback.
	if(!connection->progressCallback)
		return;

	// Call the callback.
	connection->progressCallback(
		connection->request,
		connection->state,
		buffer,
		bufferLen,
		connection->fileBytesReceived,
		connection->totalSize,
		connection->callbackParam
		);
}

void ghiCallPostCallback
(
	GHIConnection * connection
)
{
	GS_ASSERT(connection);

	// Check for no callback.
	if(!connection->postingState.callback)
		return;

	// Call the callback.
	connection->postingState.callback(
		connection->request,
		connection->postingState.bytesPosted,
		connection->postingState.totalBytes,
		connection->postingState.index,
		ArrayLength(connection->postingState.states),
		connection->callbackParam
		);
}
