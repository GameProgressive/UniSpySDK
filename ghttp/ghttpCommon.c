///////////////////////////////////////////////////////////////////////////////
// File:	ghttpCommon.c
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#include "ghttpCommon.h"

#ifdef WIN32
// A lock.
typedef void * GLock;

// The lock used by ghttp.
static GLock ghiGlobalLock;
#endif

// Proxy server.
char * ghiProxyAddress;
unsigned short ghiProxyPort;

// Throttle settings.
int ghiThrottleBufferSize = 125;
gsi_time ghiThrottleTimeDelay = 250;

// Number of connections.
extern int ghiNumConnections;


#ifdef WIN32
// Creates a lock.
static GLock GNewLock(void)
{
	CRITICAL_SECTION * criticalSection;

	criticalSection = (CRITICAL_SECTION *)gsimalloc(sizeof(CRITICAL_SECTION));
	if(!criticalSection)
		return NULL;

	InitializeCriticalSection(criticalSection);

	return (GLock)criticalSection;
}

// Frees a lock.
static void GFreeLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	DeleteCriticalSection(criticalSection);

	gsifree(criticalSection);
}

// Locks a lock.
static void GLockLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	EnterCriticalSection(criticalSection);
}

// Unlocks a lock.
static void GUnlockLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	LeaveCriticalSection(criticalSection);
}
#endif

// Creates the ghttp lock.
void ghiCreateLock(void)
{
#ifdef WIN32
	// We shouldn't already have a lock.
	GS_ASSERT(!ghiGlobalLock);

	// Create the lock.
	ghiGlobalLock = GNewLock();
#endif
}

// Frees the ghttp lock.
void ghiFreeLock(void)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GFreeLock(ghiGlobalLock);
	ghiGlobalLock = NULL;
#endif
}

// Locks the ghttp lock.
void ghiLock
(
	void
)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GLockLock(ghiGlobalLock);
#endif
}

// Unlocks the ghttp lock.
void ghiUnlock
(
	void
)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GUnlockLock(ghiGlobalLock);
#endif
}

// Logs traffic.
#ifdef HTTP_LOG
void ghiLogToFile(const char * buffer, int len, const char* fileName)
{
#ifdef _NITRO
	int i;

	if(!buffer || !len)
		return;

	for(i = 0 ; i < len ; i++)
		OS_PutChar(buffer[i]);
#else
	FILE * file;

	if(!buffer || !len)
		return;

	file = gsifopen(fileName, "ab");
	if(file)
	{
		fwrite(buffer, 1, len, file);
		fclose(file);
	}
#endif
}
#endif

// Reads encrypted data from decodeBuffer.
// Appends decrypted data to recvBuffer.
// Returns GHTTPFalse if there was a fatal error.
GHTTPBool ghiDecryptReceivedData(struct GHIConnection * connection)
{
	// Decrypt data from decodeBuffer to recvBuffer.
	GHIEncryptionResult aResult = GHIEncryptionResult_None;

	// Data to be decrypted.
	char* aReadPos  = NULL;
	char* aWritePos = NULL;
	int   aReadLen  = 0;
	int   aWriteLen = 0;

	do
	{
		// Call the decryption function.
		do 
		{
			aReadPos  = connection->decodeBuffer.data + connection->decodeBuffer.pos;
			aReadLen  = connection->decodeBuffer.len  - connection->decodeBuffer.pos; 
			aWritePos = connection->recvBuffer.data + connection->recvBuffer.len;
			aWriteLen = connection->recvBuffer.size - connection->recvBuffer.len;    // the amount of room in recvbuffer

			aResult = (connection->encryptor.mDecryptFunc)(connection, &connection->encryptor, 
				aReadPos, &aReadLen, aWritePos, &aWriteLen);
			if (aResult == GHIEncryptionResult_BufferTooSmall)
			{
				// Make some more room.
				if (GHTTPFalse == ghiResizeBuffer(&connection->recvBuffer, connection->recvBuffer.sizeIncrement))
					return GHTTPFalse; // An error has occured.
			}
			else if(aResult == GHIEncryptionResult_Error)
			{
				return GHTTPFalse;
			}
		} while (aResult == GHIEncryptionResult_BufferTooSmall && aWriteLen == 0);

		// Adjust GHIBuffer sizes so they account for transfered data.
		if(aReadLen > connection->decodeBuffer.len)
		{
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
				"ghiDecryptReceivedData read past the end of connection->decodeBuffer! (%d\\%d bytes)\r\n",
				aReadLen, connection->decodeBuffer.len);										  
			return GHTTPFalse;
		}

		connection->decodeBuffer.pos += aReadLen;
		connection->recvBuffer.len   += aWriteLen;

	} while(aWriteLen > 0);

	// Discard data from the decodedBuffer in chunks.
	if (connection->decodeBuffer.pos > 0xFF)
	{
		int bytesToKeep = connection->decodeBuffer.len - connection->decodeBuffer.pos;
		if (bytesToKeep == 0)
			ghiResetBuffer(&connection->decodeBuffer);
		else
		{
			memmove(connection->decodeBuffer.data,
					connection->decodeBuffer.data + connection->decodeBuffer.pos,
					(size_t)bytesToKeep);
			connection->decodeBuffer.pos = 0;
			connection->decodeBuffer.len = bytesToKeep;
		}
	}

	return GHTTPTrue; 
}

// Receive some data.
GHIRecvResult ghiDoReceive
(
	GHIConnection * connection,
	char buffer[],
	int * bufferLen
)
{
	int rcode;
	int socketError;
	int len;

	// How much to try to receive.
	len = (*bufferLen - 1);

	// Check whether or not the connection is throttled.
	if(connection->throttle)
	{
		unsigned long now;

		// Don't receive too often.
		now = current_time();
		if(now < (connection->lastThrottleRecv + ghiThrottleTimeDelay))
			return GHINoData;

		// Update the receive time.
		connection->lastThrottleRecv = (unsigned int)now;

		// Don't receive too much.
		len = GS_MIN(len, ghiThrottleBufferSize);
	}

	// Receive some data.
	if (connection->encryptor.mEngine != GHTTPEncryptionEngine_None &&
		connection->encryptor.mSessionEstablished == GHTTPTrue &&
		connection->encryptor.mEncryptOnSend == GHTTPTrue )
	{
		GHIEncryptionResult result;
		int recvLength = len;
		
		result = ghiEncryptorSslDecryptRecv(connection, &connection->encryptor, buffer, &recvLength);
		
		// Check for an error.
		if (result == GHIEncryptionResult_Success) {
			if (recvLength == -1) {
				// This means to try again after waiting.
				return GHINoData;
			} else {
				rcode = recvLength;
			}
		} else {
			// There was a real error.
			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = 0;
			connection->connectionClosed = GHTTPTrue;

			return GHIError;
		}
	}
	else
	{
		rcode = recv(connection->socket, buffer, len, 0);
		
		// There was an error.
		if(gsiSocketIsError(rcode))
		{
			// Get the error code.
			socketError = GOAGetLastError(connection->socket);


			// Check for a closed connection.
			if(socketError == WSAENOTCONN)
			{
				connection->connectionClosed = GHTTPTrue;
                gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_HotError,
                    "Got an error from recv: %d. Connection Closed\n", socketError);
				return GHIConnClosed;
			}

			// Check to make sure there's nothing to wait for.
			if((socketError == WSAEWOULDBLOCK) || (socketError == WSAEINPROGRESS) || (socketError == WSAETIMEDOUT))
            {
                gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_HotError,
                    "Got an error from recv: %d. No Data\n", socketError);
				return GHINoData;
            }
			// There was a real error.
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_HotError,
                "Got an error from recv: %d\n", socketError);

			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = socketError;
			connection->connectionClosed = GHTTPTrue;

			return GHIError;
		}
	}

	// The connection was closed.
	if(rcode == 0)
	{
		connection->connectionClosed = GHTTPTrue;
		return GHIConnClosed;
	}

	// Cap the buffer.
	buffer[rcode] = '\0';
	*bufferLen = rcode;

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_RawDump, "Received %d bytes\n", rcode);

	// Notify app.
	return GHIRecvData;
}

int ghiDoSend
(
	struct GHIConnection * connection,
	const char * buffer,
	int len
)
{
	int rcode;

	if (buffer == NULL || len == 0)
		return 0;
	
	// Do the send.
	if (connection->encryptor.mEngine != GHTTPEncryptionEngine_None &&
		connection->encryptor.mSessionEstablished == GHTTPTrue &&
		connection->encryptor.mEncryptOnSend == GHTTPTrue)
	{
		int bytesSent = 0;
		GHIEncryptionResult result;
		
		// send through encryption engine
		result = ghiEncryptorSslEncryptSend(connection, &connection->encryptor, buffer, len, &bytesSent);
	
		// Check for an error.
		if (result == GHIEncryptionResult_Success) {
			if (bytesSent == -1) {
				// This means to try again after waiting.
				return -2;
			} else {
				rcode = bytesSent;
			}
		} else {
			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = 0;
			
			return -1;
		}
	}
	else
	{
		// Send directly to the socket.
		rcode = send(connection->socket, buffer, len, 0);
		
		// Check for an error.
		if(gsiSocketIsError(rcode))
		{
			int error;

			// 'Would block' just means 0 bytes sent.
			error = GOAGetLastError(connection->socket);
			if((error == WSAEWOULDBLOCK) || (error == WSAEINPROGRESS) || (error == WSAETIMEDOUT))
				return 0;

			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = error;

			return -1;
		}
	}

	// Do not add CRLF as part of bytes posted; make sure waitPostContinue is false.
	if(connection->state == GHTTPPosting && connection->postingState.waitPostContinue == GHTTPFalse)
	{
		connection->postingState.bytesPosted += rcode;
		ghiLogRequest(buffer, rcode);
	}

	return rcode;
}

GHITrySendResult ghiTrySendThenBuffer
(
	GHIConnection * connection,
	const char * buffer,
	int len
)
{
	int rcode = 0;

	// **SSL pattern 1: buffer everything into an SSL record**
	if (connection->encryptor.mEngine != GHTTPEncryptionEngine_None &&
		connection->encryptor.mSessionEstablished == GHTTPTrue &&
		connection->encryptor.mEncryptOnBuffer == GHTTPTrue)
	{
		if (!ghiEncryptDataToBuffer(&connection->sendBuffer, buffer + rcode, len - rcode))
			return GHITrySendError;

		// Try to send immediately.
		if (ghiSendBufferedData(connection) == GHTTPFalse)
			return GHITrySendError;
		if (connection->sendBuffer.pos >= connection->sendBuffer.len)
		{
			ghiResetBuffer(&connection->sendBuffer);
			return GHITrySendSent; // Everything was sent.
		}
		return GHITrySendBuffered;
	}

	// **Plain text or SSL encrypt on send**

	// If we already have something buffered, don't send.
	if(connection->sendBuffer.pos >= connection->sendBuffer.len)
	{
		// Try to send.
		rcode = ghiDoSend(connection, buffer, len);
		switch (rcode) {
		case -1: // There was an error.
			return GHITrySendError;
		case -2: // Try to send again.
			return GHITrySendBuffered;
		}

		// Was it all sent?
		if(rcode == len)
			return GHITrySendSent;	
	}
	
	// Buffer whatever wasn't sent.
	if(!ghiAppendDataToBuffer(&connection->sendBuffer, buffer + rcode, len - rcode))
		return GHITrySendError;
	return GHITrySendBuffered;
}

static GHTTPBool ghiParseProxyServer
(
	const char * server,
	char ** proxyAddress,       // [out] The proxy address.
	unsigned short * proxyPort  // [out] The proxy port.
)
{
	char * strPort;

	// Make sure each pointer and is valid, as well as what it points to.
	GS_ASSERT(server && *server);
	GS_ASSERT(proxyAddress && !*proxyAddress);
	GS_ASSERT(proxyPort);

	// Copy off the server address.
	*proxyAddress = goastrdup(server);
	if(!*proxyAddress)
		return GHTTPFalse;

	// Check for a port.
	if((strPort = strchr(*proxyAddress, ':')) != NULL)
	{
		*strPort++ = '\0';

		// Try getting the port.
		*proxyPort = (unsigned short)atoi(strPort);
		if(!*proxyPort)
		{
			gsifree(*proxyAddress);
			*proxyAddress = NULL;
			return GHTTPFalse;
		}
	}
	else
	{
		*proxyPort = GHI_DEFAULT_PORT;
	}
	return GHTTPTrue;
}

GHTTPBool ghiSetProxy
(
	const char * server
)
{
	// Free any existing proxy address.
	if(ghiProxyAddress)
	{
		gsifree(ghiProxyAddress);
		ghiProxyAddress = NULL;
	}
	ghiProxyPort = 0;

	// If a server was supplied, try to parse it.
	if(server && *server)
		return ghiParseProxyServer(server, &ghiProxyAddress, &ghiProxyPort);

	// No server supplied-results are in the proxy being cleared.
	return GHTTPTrue;
}

GHTTPBool ghiSetRequestProxy
(
	GHTTPRequest request,
	const char * server
)
{
	// Obtain the connection for this request.
	GHIConnection* connection = ghiRequestToConnection(request);
	if (connection == NULL)
		return GHTTPFalse;

	// Free any existing proxy address.
	if(connection->proxyOverrideServer)
	{
		gsifree(connection->proxyOverrideServer);
		connection->proxyOverrideServer = NULL;
		connection->proxyOverridePort = GHI_DEFAULT_PORT;
	}

	// If a server was supplied, try to parse it.
	if(server && *server)
		return ghiParseProxyServer(server, &connection->proxyOverrideServer, &connection->proxyOverridePort);
	
	// No server supplied-results are in the proxy being cleared.
	return GHTTPTrue;
}

void ghiThrottleSettings
(
	int bufferSize,
	gsi_time timeDelay
)
{
	ghiThrottleBufferSize = bufferSize;
	ghiThrottleTimeDelay = timeDelay;
}
