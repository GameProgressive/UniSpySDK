///////////////////////////////////////////////////////////////////////////////
// File:	ghttpConnection.c
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#include "ghttpConnection.h"
#include "ghttpCommon.h"

// Initial size and increment amount for the connections array.
///////////////////////////////////////////////////////////////
#define CONNECTIONS_CHUNK_LEN      4

// This is an array of pointers to GHIConnection objects.
// A GHTTPRequest is an index into this array.
static GHIConnection ** ghiConnections;
static int ghiConnectionsLen;
static int ghiNumConnections;
static int ghiNextUniqueID;

// This finds a gsifree slot in the ghiConnections array.
// If there are no gsifree slots, the array size will be increased.
static int ghiFindFreeSlot
(
	void
)
{
	int i;
	GHIConnection ** tempPtr;
	int oldLen;
	int newLen;

	// Look for an open slot.
	for(i = 0 ; i < ghiConnectionsLen ; i++)
	{
		if(!ghiConnections[i]->inUse)
			return i;
	}

	GS_ASSERT(ghiNumConnections == ghiConnectionsLen);

	// No open slots were found, so resize the array.
	oldLen = ghiConnectionsLen;
	newLen = (ghiConnectionsLen + CONNECTIONS_CHUNK_LEN);
	tempPtr = (GHIConnection **)gsirealloc(ghiConnections, sizeof(GHIConnection *) * newLen);
	if(!tempPtr)
		return -1;
	ghiConnections = tempPtr;

	// Create the new connection objects.
	for(i = oldLen ; i < newLen ; i++)
	{
		ghiConnections[i] = (GHIConnection *)gsimalloc(sizeof(GHIConnection));
		if(!ghiConnections[i])
		{
			for(i-- ; i >= oldLen ; i--)
				gsifree(ghiConnections[i]);
			return -1;
		}
		ghiConnections[i]->inUse = GHTTPFalse;
	}

	// Update the length.
	ghiConnectionsLen = newLen;

	return oldLen;
}

GHIConnection * ghiNewConnection
(
	void
)
{
	int slot;
	GHIConnection * connection;
	GHTTPBool bResult;

	ghiLock();

	// Find a gsifree slot.
	slot = ghiFindFreeSlot();
	if(slot == -1)
	{
		ghiUnlock();
		return NULL;
	}

	// Get a pointer to the object.
	connection = ghiConnections[slot];

	// Initialize the object.
	memset(connection, 0, sizeof(GHIConnection));
	connection->inUse = GHTTPTrue;
	connection->request = (GHTTPRequest)slot;
	connection->uniqueID = ghiNextUniqueID++;
	connection->type = GHIGET;
	connection->state = GHTTPSocketInit;
	connection->URL = NULL;
	connection->serverAddress = NULL;
	connection->serverIP = INADDR_ANY;
	connection->serverPort = (unsigned short)0;
	connection->requestPath = NULL;
	connection->sendHeaders = NULL;
	connection->recvHeaders = NULL;
	connection->saveFile = NULL;
	connection->blocking = GHTTPFalse;
	connection->persistConnection = GHTTPFalse;
	connection->result = GHTTPSuccess;
	connection->progressCallback = NULL;
	connection->completedCallback = NULL;
	connection->callbackParam = NULL;
	connection->socket = INVALID_SOCKET;
	connection->socketError = 0;
	connection->userBufferSupplied = GHTTPFalse;
	connection->statusMajorVersion = 0;
	connection->statusMinorVersion = 0;
	connection->statusCode = 0;
	connection->statusStringIndex = 0;
	connection->headerStringIndex = 0;
	connection->completed = GHTTPFalse;
	connection->fileBytesReceived = 0;
	connection->totalSize = -1;
	connection->redirectURL = NULL;
	connection->redirectCount = 0;
	connection->chunkedTransfer = GHTTPFalse;
	connection->processing = GHTTPFalse;
	connection->throttle = GHTTPFalse;
	connection->lastThrottleRecv = 0;
	connection->post = NULL;
	connection->maxRecvTime = 500; // Prevent blocking in async mode with systems that never generate WSAEWOULDBLOCK
	connection->proxyOverridePort = GHI_DEFAULT_PORT;	
	connection->proxyOverrideServer = NULL;
	connection->encryptor.mInterface = NULL;
	connection->encryptor.mInitialized = GHTTPFalse;
	connection->receiveFileIdleTime = 0;

// This handle is used for asynch DNS lookups.
#if !defined(GSI_NO_THREADS)
	connection->handle = NULL;
#endif

	bResult = ghiInitBuffer(connection, &connection->sendBuffer, SEND_BUFFER_INITIAL_SIZE, SEND_BUFFER_INCREMENT_SIZE);
	if(bResult)
		bResult = ghiInitBuffer(connection, &connection->encodeBuffer, ENCODE_BUFFER_INITIAL_SIZE, ENCODE_BUFFER_INCREMENT_SIZE);
	if(bResult)
		bResult = ghiInitBuffer(connection, &connection->recvBuffer, RECV_BUFFER_INITIAL_SIZE, RECV_BUFFER_INCREMENT_SIZE);
	if (bResult)
		bResult = ghiInitBuffer(connection, &connection->decodeBuffer, DECODE_BUFFER_INITIAL_SIZE, DECODE_BUFFER_INCREMENT_SIZE);
	
	if(!bResult)
	{
		ghiFreeConnection(connection);
		ghiUnlock();
		return NULL;
	}

	// One more connection.
	ghiNumConnections++;

	ghiUnlock();

	return connection;
}

GHTTPBool ghiCloseConnection(GHIConnection * connection)
{
	GS_ASSERT(connection);
	GS_ASSERT(connection->request >= 0);
	GS_ASSERT(connection->request < ghiConnectionsLen);
	GS_ASSERT(connection->inUse);

	// Check arguments.
	if(!connection)
		return GHTTPFalse;
	if(!connection->inUse)
		return GHTTPFalse;
	if(connection->request < 0)
		return GHTTPFalse;
	if(connection->request >= ghiConnectionsLen)
		return GHTTPFalse;

    ghiLock();
#if !defined(GSI_NO_THREADS)
    // Cancel and free asynchronous lookup if it has not already been done.
    if (connection->handle)
    {
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_State, GSIDebugLevel_Comment, 
            "Cancelling Thread and freeing memory\n");
        gsiCancelResolvingHostname(connection->handle);
        
        // The handle is already freed, clear the handle here.
        connection->handle = NULL;
    }
#endif

	if(connection->socket != INVALID_SOCKET)
	{
		shutdown(connection->socket, 2);
#if (defined(_NITRO) && defined(TWLSSL))
        if (closesocketsure(connection->socket, &connection->encryptor.mNotInCloseSocket) == 
            gsi_false)
        {
            ghiUnlock();
            return GHTTPFalse;
        }
#else
		closesocket(connection->socket);
#endif	
        connection->socket = INVALID_SOCKET;
	}
	ghiUnlock();
	return GHTTPTrue;
}

GHTTPBool ghiFreeConnectionData( GHIConnection * connection)
{
	GS_ASSERT(connection);
	GS_ASSERT(connection->request >= 0);
	GS_ASSERT(connection->request < ghiConnectionsLen);
	GS_ASSERT(connection->inUse);

	// Check arguments.
	if(!connection)
		return GHTTPFalse;
	if(!connection->inUse)
		return GHTTPFalse;
	if(connection->request < 0)
		return GHTTPFalse;
	if(connection->request >= ghiConnectionsLen)
		return GHTTPFalse;

	ghiLock();

	// Free the data.
	gsifree(connection->URL);
	connection->URL = NULL;
	gsifree(connection->saveFileName);
	connection->saveFileName = NULL;
	gsifree(connection->serverAddress);
	connection->serverAddress = NULL; 
	gsifree(connection->requestPath);
	connection->requestPath = NULL;
	gsifree(connection->sendHeaders);
	connection->sendHeaders = NULL;
	gsifree(connection->recvHeaders);
	connection->recvHeaders = NULL;
	gsifree(connection->redirectURL);
	connection->redirectURL = NULL;
	gsifree(connection->proxyOverrideServer);
	connection->proxyOverrideServer = NULL;
#ifndef NOFILE
	if(connection->saveFile)
		fclose(connection->saveFile);
#endif
	
	ghiFreeBuffer(&connection->sendBuffer);
	ghiFreeBuffer(&connection->encodeBuffer);
	ghiFreeBuffer(&connection->recvBuffer);
	ghiFreeBuffer(&connection->decodeBuffer);
	ghiFreeBuffer(&connection->getFileBuffer);
	if(connection->postingState.states)
		ghiPostCleanupState(connection);
   
	// Check for an encryptor.
	if (connection->encryptor.mInitialized != GHTTPFalse)
	{
		if (connection->encryptor.mCleanupFunc)
			(connection->encryptor.mCleanupFunc)(connection, &connection->encryptor);
		connection->encryptor.mInitialized = GHTTPFalse;
	}

	// Clear this value just in case.
	connection->receiveFileIdleTime = 0;
	// Free the slot.
	connection->inUse = GHTTPFalse;

	// One less connection.
	ghiNumConnections--;

	ghiUnlock();
	return GHTTPTrue;
}

GHTTPBool ghiFreeConnection
(
	GHIConnection * connection
)
{
	GS_ASSERT(connection);
	GS_ASSERT(connection->request >= 0);
	GS_ASSERT(connection->request < ghiConnectionsLen);
	GS_ASSERT(connection->inUse);

	// Check arguments.
	if(!connection)
		return GHTTPFalse;
	if(!connection->inUse)
		return GHTTPFalse;
	if(connection->request < 0)
		return GHTTPFalse;
	if(connection->request >= ghiConnectionsLen)
		return GHTTPFalse;

    if (ghiCloseConnection(connection)!=GHTTPTrue)
    {
        return GHTTPFalse;
    }
	return ghiFreeConnectionData(connection);
}

GHIConnection * ghiRequestToConnection
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	GS_ASSERT(request >= 0);
	GS_ASSERT(request < ghiConnectionsLen);

	ghiLock();

	// Check arguments.
	if((request < 0) || (request >= ghiConnectionsLen))
	{
		ghiUnlock();
		return NULL;
	}

	connection = ghiConnections[request];

	// Check to make sure that the connection is not in use.
	if(!connection->inUse)
		connection = NULL;

	ghiUnlock();

	return connection;
}

void ghiEnumConnections
(
	GHTTPBool (* callback)(GHIConnection *)
)
{
	int i;

	// Check for no connections.
	if(ghiNumConnections <= 0)
		return;

	ghiLock();
	for(i = 0 ; i < ghiConnectionsLen ; i++)
		if(ghiConnections[i]->inUse)
			callback(ghiConnections[i]);
	ghiUnlock();
}

void ghiRedirectConnection
(
	GHIConnection * connection
)
{
	GS_ASSERT(connection);
	GS_ASSERT(connection->redirectURL);
	
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_State, GSIDebugLevel_Comment, "Redirecting Connection\n");


#if !defined(GSI_NO_THREADS)
    // Cancel and free asynchronous lookup if it has not already been done.
    if (connection->handle)
    {
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_State, GSIDebugLevel_Comment, 
            "Cancelling Thread and freeing memory\n");
        gsiCancelResolvingHostname(connection->handle);
        connection->handle = NULL;
    }
#endif



	// Close the socket.
	shutdown(connection->socket, 2);
#if (defined(_NITRO) && defined(TWLSSL))
    if (!closesocketsure(connection->socket, &connection->encryptor.mNotInCloseSocket))
    {
        // Let's wait until the socket closes to initialize the new connection.
        return;
    }
#else
	closesocket(connection->socket);
#endif

    // The following initializes the new connection.
    
	// New URL.
	gsifree(connection->URL);
	connection->URL = connection->redirectURL;
	connection->redirectURL = NULL;

	// Reset stuff parsed from the URL.
	gsifree(connection->serverAddress);
	connection->serverAddress = NULL;
	connection->serverIP = 0;
	connection->serverPort = 0;
	gsifree(connection->requestPath);
	connection->requestPath = NULL;

	// Reset state.
	connection->state = GHTTPSocketInit;
	connection->socket = INVALID_SOCKET;

	// Reset buffers.
	ghiResetBuffer(&connection->sendBuffer);
	ghiResetBuffer(&connection->encodeBuffer);
	ghiResetBuffer(&connection->recvBuffer);
	ghiResetBuffer(&connection->decodeBuffer);

	// Reset status.
	connection->statusMajorVersion = 0;
	connection->statusMinorVersion = 0;
	connection->statusCode = 0;
	connection->statusStringIndex = 0;

	connection->headerStringIndex = 0;

	// The connection isn't closed.
	connection->connectionClosed = GHTTPFalse;

	// Check for an encryptor.
	if (connection->encryptor.mInitialized != GHTTPFalse)
	{
		// Cleanup the encryptor.
		if (connection->encryptor.mCleanupFunc)
			(connection->encryptor.mCleanupFunc)(connection, &connection->encryptor);
		connection->encryptor.mInitialized = GHTTPFalse;

		// If the redirect isn't secure, clear it.
		if(strncmp("https://", connection->URL, 8) != 0)
		{
			connection->encryptor.mEngine = GHTTPEncryptionEngine_None;
			connection->encryptor.mInterface = NULL;
		}
	}

	// One more redirect.
	connection->redirectCount++;
}

void ghiCleanupConnections
(
	void
)
{
	int i;

	if(!ghiConnections)
		return;

	// Cleanup all running connections.
	ghiEnumConnections(ghiFreeConnection);

	// Cleanup the connection states.
	for(i = 0 ; i < ghiConnectionsLen ; i++)
		gsifree(ghiConnections[i]);
	gsifree(ghiConnections);
	ghiConnections = NULL;
	ghiConnectionsLen = 0;
	ghiNumConnections = 0;
}

char* ghiGetServerAddressFromUrl(char *serverUrl)
{
    char    *url    = serverUrl;
    char    *serverAddress = NULL;
    char    tempChar = '\0';
    int     nIndex   = 0;
    
     // Check for "http://" or "https://".
	if(strncmp(url, "http://", 7) == 0)
	{
		url += 7;
	}
	else if (strncmp(url, "https://", 8) == 0)
	{
		url += 8;
	}
	else
	{
		return NULL;
	}
    
	nIndex = (int)strcspn(url, ":/");
	tempChar = url[nIndex];
	url[nIndex] = '\0';
	
	serverAddress = goastrdup(url);
	
	url[nIndex] = tempChar;
	url += nIndex;

	return serverAddress;
}

