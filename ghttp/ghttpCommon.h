///////////////////////////////////////////////////////////////////////////////
// File:	ghttpCommon.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#ifndef _GHTTPCOMMON_H_
#define _GHTTPCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ghttp.h"
#include "ghttpConnection.h"

// HTTP Line-terminator.
#define CRLF    "\xD\xA"

// HTTP URL Encoding 
#define GHI_LEGAL_URLENCODED_CHARS      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_@-.*"
#define GHI_DIGITS                      "0123456789ABCDEF"

// Default HTTP port.
#define GHI_DEFAULT_PORT                      80
#define GHI_DEFAULT_SECURE_PORT               443
#define GHI_DEFAULT_THROTTLE_BUFFER_SIZE      125
#define GHI_DEFAULT_THROTTLE_TIME_DELAY       250

// Proxy server.
extern char * ghiProxyAddress;
extern unsigned short ghiProxyPort;

// Throttle settings.
extern int ghiThrottleBufferSize;
extern gsi_time ghiThrottleTimeDelay;

// Our thread lock.
void ghiCreateLock(void);
void ghiFreeLock(void);
void ghiLock(void);
void ghiUnlock(void);

// Do logging.
#ifdef HTTP_LOG
void ghiLogToFile
(
	const char * buffer,
	int len,
	const char * fileName
);
#define ghiLogRequest(b,c)  ghiLogToFile(b,c,"request.log");
#define ghiLogResponse(b,c)	ghiLogToFile(b,c,"response.log");
#define ghiLogPost(b,c)	    ghiLogToFile(b,c,"post.log");
#else
#define ghiLogRequest(b,c)
#define ghiLogResponse(b,c)
#define ghiLogPost(b,c)
#endif


// Possible results from ghiDoReceive.
typedef enum
{
	GHIRecvData,    // Data was received.
	GHINoData,      // No data was available.
	GHIConnClosed,	// The connection was closed.
	GHIError        // There was a socket error.
} GHIRecvResult;

// Receive some data.
GHIRecvResult ghiDoReceive
(
	GHIConnection * connection,
	char buffer[],
	int * bufferLen
);

// Do a send on the connection's socket.
// It will return the number of bytes sent as 0 or more.
// If there's an error, it will return -1.
// If the socket needs to pause for a while, it will return -2.
int ghiDoSend
(
	GHIConnection * connection,
	const char * buffer,
	int len
);

// Results for ghtTrySendThenBuffer.
typedef enum
{
	GHITrySendError,     // There was an error sending.
	GHITrySendSent,      // Everything was sent.
	GHITrySendBuffered   // Some or all of the data was buffered.
} GHITrySendResult;

// This sends whatever it can on the socket.
// It also buffers whatever can't be sent in the sendBuffer.
GHITrySendResult ghiTrySendThenBuffer
(
	GHIConnection * connection,
	const char * buffer,
	int len
);

// Set the proxy server
GHTTPBool ghiSetProxy
(
	const char * server
);

// Set the proxy server for a specific request
GHTTPBool ghiSetRequestProxy
(
	GHTTPRequest request,
	const char * server
);

// Set the throttle settings.
void ghiThrottleSettings
(
	int bufferSize,
	gsi_time timeDelay
);

// Decrypt data from the decode buffer into the receive buffer.
GHTTPBool ghiDecryptReceivedData(struct GHIConnection * connection);


#ifdef __cplusplus
}
#endif

#endif
