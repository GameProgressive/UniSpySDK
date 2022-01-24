///////////////////////////////////////////////////////////////////////////////
// File:	ghttp.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GHTTP_H_
#define _GHTTP_H_

#include <stdlib.h>

#include "../common/gsCommon.h"
#include "../common/gsXML.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GSI_UNICODE
#define ghttpGet	ghttpGetA
#define ghttpGetEx	ghttpGetExA
#define ghttpSave	ghttpSaveA
#define ghttpSaveEx	ghttpSaveExA
#define ghttpStream	ghttpStreamA
#define ghttpStreamEx	ghttpStreamExA
#define ghttpHead	ghttpHeadA
#define ghttpHeadEx	ghttpHeadExA
#define ghttpPost	ghttpPostA
#define ghttpPostEx	ghttpPostExA
#define ghttpPostAddString	ghttpPostAddStringA
#define ghttpPostAddFileFromDisk	ghttpPostAddFileFromDiskA
#define ghttpPostAddFileFromMemory	ghttpPostAddFileFromMemoryA
#else
#define ghttpGet	ghttpGetW
#define ghttpGetEx	ghttpGetExW
#define ghttpSave	ghttpSaveW
#define ghttpSaveEx	ghttpSaveExW
#define ghttpStream	ghttpStreamW
#define ghttpStreamEx	ghttpStreamExW
#define ghttpHead	ghttpHeadW
#define ghttpHeadEx	ghttpHeadExW
#define ghttpPost	ghttpPostW
#define ghttpPostEx	ghttpPostExW
#define ghttpPostAddString	ghttpPostAddStringW
#define ghttpPostAddFileFromDisk	ghttpPostAddFileFromDiskW
#define ghttpPostAddFileFromMemory	ghttpPostAddFileFromMemoryW
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Public SDK Interface
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Enumerated Types  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// GHTTPBool
// Summary
//		Standard Boolean.
typedef enum
{
	GHTTPFalse, //False
	GHTTPTrue   //True
} GHTTPBool;

// ByteCount.
/////////////
#if (GSI_MAX_INTEGRAL_BITS >= 64)
	typedef gsi_i64  GHTTPByteCount;
#else
	typedef gsi_i32  GHTTPByteCount;
#endif

///////////////////////////////////////////////////////////////////////////////
// GHTTPState
// Summary
//		The current state of an HTTP request.
typedef enum
{
	GHTTPSocketInit,			// Socket creation and initialization.
	GHTTPHostLookup,            // Resolving hostname to IP (asynchronously if possible).
	GHTTPLookupPending,			// Asynchronous DNS lookup pending.
	GHTTPConnecting,            // Waiting for socket connect to complete.
#ifdef GS_USE_REFLECTOR
	GHTTPReflectorHeader,       // Writing the Reflector client header.
#endif
	GHTTPSecuringSession,		// Setup secure channel.
	GHTTPSendingRequest,        // Sending the request.
	GHTTPPosting,               // Positing data (skipped if not posting).
	GHTTPWaiting,               // Waiting for a response.
	GHTTPReceivingStatus,       // Receiving the response status.
	GHTTPReceivingHeaders,      // Receiving the headers.
	GHTTPReceivingFile,         // Receiving the file.
	GHTTPDisconnecting          // Special state for closing connections for TWLSSL.
} GHTTPState;

///////////////////////////////////////////////////////////////////////////////
// GHTTPResult
// Summary
//		The result of an HTTP request.
// Remarks
//		Make sure you don't accidentally use these to compare against a 
//		 GHTTPRequest value.
//		To check if a GHTTPRequest has returned an error use the IS_GHTTP_ERROR
//		 macro.
typedef enum
{
	GHTTPSuccess,               // 0:  Successfully retrieved file.
	GHTTPOutOfMemory,           // 1:  A memory allocation failed.
	GHTTPBufferOverflow,        // 2:  The user-supplied buffer was too small to hold the file.
	GHTTPParseURLFailed,        // 3:  There was an error parsing the URL.
	GHTTPHostLookupFailed,      // 4:  Failed looking up the hostname.
	GHTTPSocketFailed,          // 5:  Failed to create/initialize/read/write a socket.
	GHTTPConnectFailed,         // 6:  Failed connecting to the http server.
	GHTTPBadResponse,           // 7:  Error understanding a response from the server.
	GHTTPRequestRejected,       // 8:  The request has been rejected by the server.
	GHTTPUnauthorized,          // 9:  Not authorized to get the file.
	GHTTPForbidden,             // 10: The server has refused to send the file.
	GHTTPFileNotFound,          // 11: Failed to find the file on the server.
	GHTTPServerError,           // 12: The server has encountered an internal error.
	GHTTPFileWriteFailed,       // 13: An error occurred writing to the local file 
								//		(for ghttpSaveFile[Ex]).
	GHTTPFileReadFailed,        // 14: There was an error reading from a local file 
								//		(for posting files from disk).
	GHTTPFileIncomplete,        // 15: Download started but was interrupted. Only reported 
								//		if file size is known.
	GHTTPFileToBig,             // 16: The file is too big to be downloaded (size exceeds 
								//		range of internal data types)
	GHTTPEncryptionError,       // 17: Error with encryption engine.
	GHTTPRequestCancelled,      // 18: User requested cancel and/or graceful close.
	GHTTPRecvFileTimeout        // 19: Stuck in the Receive File state.
} GHTTPResult;

///////////////////////////////////////////////////////////////////////////////
// GHTTPEncryptionEngine
// Summary
//		Used to select a method of encryption for HTTP requests.
typedef enum
{
	GHTTPEncryptionEngine_None,			// 0: No encryption.
	GHTTPEncryptionEngine_GameSpy,		// 1: Use standard GameSpy encryption.
	GHTTPEncryptionEngine_MatrixSsl,	// 2: Use GameSpy Chat server encryption; must
										//    #define MATRIXSSL and include matrixssl 
										//    source files.
	GHTTPEncryptionEngine_RevoEx,		// 3: Use Nintendo encryption; must #define 
										//    REVOEXSSL and include RevoEX SSL source files.
    GHTTPEncryptionEngine_Twl,          // 4: #define TWLSSL and include Twl SSL source files.

	GHTTPEncryptionEngine_Default,		// 5: Use GameSpy unless another engine is defined
										//    with MATRIXSSL or REVOEXSSL.

	GHTTPEncryptionEngine_OpenSSL,		// 6: Use OpenSSL encryption for all the services
										//    and platforms; must #define OPENSSL and include
										//    openssl source files.
} GHTTPEncryptionEngine;

///////////////////////////////////////////////////////////////////////////////
// GHTTPRequest
// Summary
//		A type that represents an HTTP file request.
typedef int GHTTPRequest;

///////////////////////////////////////////////////////////////////////////////
// GHTTPRequestError
// Summary
//		Possible error values returned from GHTTP functions.
#ifdef GHTTP_EXTENDEDERROR
	typedef enum
	{
		GHTTPErrorStart				= -8,
		GHTTPFailedToOpenFile,
		GHTTPInvalidPost,
		GHTTPInsufficientMemory,
		GHTTPInvalidFileName,	
		GHTTPInvalidBufferSize,
		GHTTPInvalidURL,
		GHTTPUnspecifiedError		= -1
	} GHTTPRequestError;
#else
	// Backwards compatibility; developers may have relied on -1 as the only 
	// error code.
	typedef enum
	{
		GHTTPErrorStart				= -1,
		GHTTPFailedToOpenFile		= -1,
		GHTTPInvalidPost			= -1,
		GHTTPInsufficientMemory		= -1,
		GHTTPInvalidFileName		= -1,	
		GHTTPInvalidBufferSize		= -1,
		GHTTPInvalidURL				= -1,
		GHTTPUnspecifiedError		= -1
	} GHTTPRequestError;
#endif

#define IS_GHTTP_ERROR(x) ((x)<0)

//DOM-IGNORE-BEGIN 
//	Don't try to access this object directly, use the ghttpPost*() functions.
typedef struct GHIPost * GHTTPPost;
//DOM-IGNORE-END

//////////////////////////////////////////////////////////////////////////////
// ghttpResultString
// Summary
// 		Given a GHTTPResult, returns a meaningful character string describing 
//		 the result.
// Parameters
//		result        : [in] A GHTTPResult value.
// Returns
//		A nul-byte terminated character string describing the HTTP result 
//		 corresponding to the value given.
// Remarks
//		The returned string is read-only and should not be modified.
// See Also
//		ghttpGetEx, ghttpSaveEx, ghttpStream, ghttpStreamEx, ghttpHeadEx,
//		 ghttpPostEx
COMMON_API const char* ghttpResultString(
	int		result
);

//////////////////////////////////////////////////////////////////////////////
// ghttpProgressCallback
// Summary
// 		Called with updates on the current state of the request.
// Parameters
//		request       : [in] A valid request object. 
//		state         : [in] The current state of the request.
//		buffer        : [in] The file's bytes so far, NULL if state <
//								GHTTPReceivingFile. 
//		bufferLen     : [in] The number of bytes in the buffer, 0 if state <
//								GHTTPReceivingFile. 
//		bytesReceived : [in] The total number of bytes received, 0 if state
//								< GHTTPReceivingFile. 
//		totalSize     : [in] The total size of the file, -1 if unknown. 
//		param         : [in] Optional free-format user data to send to the 
//								callback.
// Remarks
//		The buffer should not be accessed once this callback returns.
//		<p />
//		If ghttpGetFile[Ex] was used, the buffer contains all of the data that
//		 has been received so far, and bufferSize is the total number of bytes
//		 received.
//		<p />
//		If ghttpSaveFile[Ex] was used, the buffer only contains the most recent
//		 data that has been received. This same data is saved to the file. The
//		 buffer will not be valid after this callback returns.
//		<p />
//		If ghttpStreamFileEx was used, the buffer only contains the most recent 
//		 data that has been received. This data will be lost once the callback
//		 returns, and should be copied if it needs to be saved. bufferSize is 
//		 the number of bytes in the current block of data.
// See Also
//		ghttpGetEx, ghttpSaveEx, ghttpStream, ghttpStreamEx, ghttpHeadEx,
//		 ghttpPostEx
typedef void (* ghttpProgressCallback)
(
	GHTTPRequest	request,		// The request.
	GHTTPState		state,			// The current state of the request.
	const char		* buffer,		// The file's bytes so far, NULL if 
									// (state < GHTTPReceivingFile).
	GHTTPByteCount	bufferLen,		// The number of bytes in the buffer, 
									// 0 if (state < GHTTPReceivingFile).
	GHTTPByteCount	bytesReceived,	// The total number of bytes received, 
									// 0 if (state < GHTTPReceivingFile ).
	GHTTPByteCount	totalSize,		// The total size of the file, -1 if 
									// unknown.
	void			* param			// Optional user-data.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpCompletedCallback
// Summary
// 		Called when the entire file has been received.
// Parameters
//		request   : [in] A valid request object.
//		result    : [in] The result (success or an error). 
//		buffer    : [in] The file's bytes (only valid if ghttpGetFile[Ex]
//							was used). 
//		bufferLen : [in] The file's length. 
//		param     : [in] Optional free-format user data for use by the 
//							callback.
// Returns
//		If ghttpGetFile[Ex] was used, return true to have the buffer freed,
//		 false if the app will free the buffer.
// Remarks
//		If ghttpStreamFileEx or ghttpSaveFile[Ex] was used, buffer is NULL, 
//		 bufferLen is the number of bytes in the file, and the return value
//		 is ignored.
//		<p />
//		If ghttpGetFile[Ex] was used, return true to have the buffer freed, 
//		 false if the app will free the buffer. If true, the buffer cannot be 
//		 accessed once the callback returns. If false, the app can use the 
//		 buffer even after this call returns, but must free it at some later 
//		 point. There will always be a file, even if there was an error, 
//		 although for errors it may be an empty file.
//		<p>
//		The routine ghttpCompletedCallback only returns GHTTPFalse when it is 
//		 necessary to save the buffer that was passed to the callback for later
//		 use. Otherwise (e.g., when the buffer is no longer need or has been 
//		 copied with the callback), it returns GHTTPTrue.
//		<p>
//		The GHTTP SDK will free the buffer if the callback returns GHTTPTrue. 
//		 Overuse of	GHTTPFalse can lead to memeory leaks.
// See Also
//		ghttpGet, ghttpGetEx, ghttpSave, ghttpSaveEx, ghttpStream, ghttpStreamEx, 
//		ghttpHead, ghttpHeadEx, ghttpPost, ghttpPostEx
typedef GHTTPBool (* ghttpCompletedCallback)(
	GHTTPRequest	request,     // The request.
	GHTTPResult		result,      // The result (success or an error).
	char			* buffer,    // The file's bytes (only valid if ghttpGetFile[Ex] was used).
	GHTTPByteCount	bufferLen,   // The file's length.
	char			* headers,
	void			* param      // User-data.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpStartup
// Summary
// 		Initialize the HTTP SDK.
// Remarks
//		Startup/Cleanup is reference counted, so always call ghttpStartup
//		 and ghttpCleanup in pairs.
// See Also
//		ghttpCleanup
COMMON_API void ghttpStartup
(
	void
);

//////////////////////////////////////////////////////////////////////////////
// ghttpCleanup
// Summary
// 		Clean up and close down the HTTP SDK.  Free internally allocated memory.
// Remarks
//		One call to ghttpCleanup should be made for each call to ghttpStartup.
// See Also
//		ghttpStartup
COMMON_API void ghttpCleanup(void);

//////////////////////////////////////////////////////////////////////////////
// ghttpGet
// Summary
// 		Make an HTTP GET request and save the response to memory.
// Parameters
//		URL               : [in] URL 
//		blocking          : [in] If true, this call doesn't return until the
//							 file has been received. 
//		completedCallback : [in] Called when the file has been received. 
//		param             : [in] Optional free-format user data for use by
//							 the callback. 
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to download the contents of a web page to memory.
//		The application can provide the memory by supplying a buffer to this
//		function, or the SDK can be allocate the memory internally.
//		<p />
//		Use ghttpGetEx for extra optional parameters.
// See Also
//		ghttpGetEx, ghttpSave, ghttpStream, ghttpHead, ghttpPost
COMMON_API GHTTPRequest ghttpGet(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has been received.
	ghttpCompletedCallback	completedCallback,	// Called when the file has been received.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpGetEx
// Summary
// 		Make an HTTP GET request and save the response to memory.
// Parameters
//		URL               : [in] URL 
//		headers           : [in] Optional headers to pass with the request.
//									 Can be NULL or "". 
//		buffer            : [in] Optional user-supplied buffer. Set to NULL
//									 to have one allocated. Must be (size+1) to 
//									 allow null terminating character. 
//		bufferSize        : [in] The size of the user-supplied buffer in
//									 bytes. 0 if buffer is NULL. 
//		post              : [in] Optional data to be posted. Can be NULL. 
//		throttle          : [in] If true, throttle this connection's
//									 download speed. 
//		blocking          : [in] If true, this call doesn't return until the
//									 file has been received. 
//		progressCallback  : [in] Called periodically with progress updates.
//									 Can be NULL. 
//		completedCallback : [in] Called when the file has been received. Can
//									 be NULL. 
//		param             : [in] Optional free-format user data to send to 
//									 the callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to download the contents of a web page to memory.
//		The application can provide the memory by supplying a buffer to this
//		 function, or the SDK can be allocate the memory internally. Use 
//		 ghttpGet for a simpler version of this function.
// See Also
//		ghttpGet, ghttpSaveEx, ghttpStreamEx, ghttpHeadEx, ghttpPostEx
COMMON_API GHTTPRequest ghttpGetEx(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* headers,			// Optional headers to pass with the request.  Can be NULL or "".
	char					* buffer,           // Optional user-supplied buffer. Set to NULL to have one allocated.
	int						bufferSize,         // The size of the user-supplied buffer in bytes. 0 if buffer is NULL.
	GHTTPPost				post,				// Optional data to be posted.
	GHTTPBool				throttle,			// If true, throttle this connection's download speed.
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has been received.
	ghttpProgressCallback	progressCallback,   // Called periodically with progress updates.
	ghttpCompletedCallback	completedCallback,	// Called when the file has been received.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSave
// Summary
// 		Make an HTTP GET request and save the response to disk.
// Parameters
//		URL               : [in] URL 
//		filename          : [in] The path and name to store the file as locally.
//		blocking          : [in] If true, this call doesn't return until the
//									 file has been received. 
//		completedCallback : [in] Called when the file has been received. Can
//									 be NULL. 
//		param             : [in] Optional free-format user data for use by
//									 the callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		value. Otherwise it identifies the request.
// Remarks
//		This function is used to download the contents of a web page
//		 directly to disk. 
//		The application supplies the path and filename at which to save the
//		 response.
//		<p />
//		Use ghttpSaveEx for extra optional parameters.
// See Also
//		ghttpGet, ghttpSaveEx, ghttpStream, ghttpHead, ghttpPost
COMMON_API GHTTPRequest ghttpSave(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* filename,			// The path and name to store the file as locally.
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has been received.
	ghttpCompletedCallback	completedCallback,	// Called when the file has been received.
	void					* param				// User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSaveEx
// Summary
// 		Make an HTTP GET request and save the response to disk.
// Parameters
//		URL					: [in] URL 
//		filename			: [in] The path and name to store the file as 
//									 locally.
//		headers				: [in] Optional headers to pass with the request. 
//									 Can be NULL or "". 
//		post				: [in] Optional data to be posted. Can be NULL. 
//		throttle			: [in] If true, throttle this connection's download
//									 speed. 
//		blocking			: [in] If true, this call doesn't return until the 
//									 file has been received. 
//		progressCallback	: [in] Called periodically with progress updates.
//									 Can be NULL. 
//		completedCallback	: [in] Called when the file has been received. Can
//									 be NULL. 
//		param				: [in] Optional free-format user data to send to 
//									 the callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to download the contents of a web page
//		 directly to disk. 
//		<p />
//		The application supplies the path and filename at which to save the
//		 response.
//		<p />
//		Use ghttpSave for a simpler version of this function.
// See Also
//		ghttpGetEx, ghttpSave, ghttpStreamEx, ghttpHeadEx, ghttpPostEx
COMMON_API GHTTPRequest ghttpSaveEx(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* filename,			// The path and name to store the file as locally.
	const gsi_char			* headers,			// Optional headers to pass with the request.  Can be NULL or "".
	GHTTPPost				post,				// Optional data to be posted.
	GHTTPBool				throttle,			// If true, throttle this connection's download speed.
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has been received.
	ghttpProgressCallback	progressCallback,   // Called periodically with progress updates.
	ghttpCompletedCallback	completedCallback,	// Called when the file has been received.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpStream
// Summary
// 		Make an HTTP GET request and stream in the response without saving
//		 it in memory.
// Parameters
//		URL               : [in] URL 
//		blocking          : [in] If true, this call doesn't return until the
//									 file has finished streaming. 
//		progressCallback  : [in] Called whenever new data is received. Can be 
//									 NULL. 
//		completedCallback : [in] Called when the file has finished streaming. 
//									 Can be NULL. 
//		param             : [in] Optional free-format user data to send to the
//									 callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to stream in the contents of a web page. The
//		 response body is not stored in memory or to disk. It is only passed to 
//		 the progressCallback as it is received, and the application can do 
//		 what it wants with the data.
//		<p />
//		Use ghttpStreamEx for extra optional parameters.
// See Also
//		ghttpGet, ghttpSave, ghttpStreamEx, ghttpHead, ghttpPost
COMMON_API GHTTPRequest ghttpStream(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has finished streaming.
	ghttpProgressCallback	progressCallback,	// Called whenever new data is received.
	ghttpCompletedCallback	completedCallback,	// Called when the file has finished streaming.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpStreamEx
// Summary
// 		Make an HTTP GET request and stream in the response without saving
//		 it in memory.
// Parameters
//		URL               : [in] URL 
//		headers           : [in] Optional headers to pass with the request.
//									 Can be NULL or "". 
//		post              : [in] Optional data to be posted. Can be NULL. 
//		throttle          : [in] If true, throttle this connection's download
//									 speed. 
//		blocking          : [in] If true, this call doesn't return until the
//									 file has finished streaming. 
//		progressCallback  : [in] Called whenever new data is received. Can be 
//									 NULL. 
//		completedCallback : [in] Called when the file has finished streaming. 
//									 Can be NULL. 
//		param             : [in] Optional free-format user data to send to the
//									 callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to stream in the contents of a web page. 
//		The response body is not stored in memory or to disk. It is only 
//		passed to the progressCallback as it is received, and the application 
//		can do what it wants with the data.
//		<p />
//		Use ghttpStreamEx for extra optional parameters.
// See Also
//		ghttpGetEx, ghttpSaveEx, ghttpStream, ghttpHeadEx, ghttpPostEx
COMMON_API GHTTPRequest ghttpStreamEx(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* headers,			// Optional headers to pass with the request.  Can be NULL or "".
	GHTTPPost				post,				// Optional data to be posted.
	GHTTPBool				throttle,			// If true, throttle this connection's download speed.
	GHTTPBool				blocking,			// If true, this call doesn't return until the file has finished streaming.
	ghttpProgressCallback	progressCallback,	// Called whenever new data is received.
	ghttpCompletedCallback	completedCallback,	// Called when the file has finished streaming.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpHead
// Summary
// 		Make an HTTP HEAD request, which will only retrieve the response
//		 headers and not the normal response body.
// Parameters
//		URL               : [in] URL 
//		blocking          : [in] If true, this call doesn't return until 
//									finished.
//		completedCallback : [in] Called when the request has finished. 
//		param             : [in] Optional free-format user data to send to 
//									the callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise, it identifies the request.
// Remarks
//		This function is similar to ghttpGet, except it only gets the
//		 response headers.
//		<p />
//		This is done by making an HEAD request instead of a GET request, 
//		 which instructs the HTTP server to leave the body out of the response.
//		<p />
//		Use ghttpHeadEx for extra optional parameters.
// See Also
//		ghttpGet, ghttpSave, ghttpStream, ghttpHeadEx, ghttpPost
COMMON_API GHTTPRequest ghttpHead(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	GHTTPBool				blocking,			// If true, this call doesn't return until finished
	ghttpCompletedCallback	completedCallback,	// Called when the request has finished.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpHeadEx
// Summary
// 		Make an HTTP HEAD request, which will only retrieve the response
//		 headers and not the normal response body.
// Parameters
//		URL               : [in] URL 
//		headers           : [in] Optional headers to pass with the request.
//									 Can be NULL or "".
//		throttle          : [in] If true, throttle this connection's download
//									 speed.
//		blocking          : [in] If true, this call doesn't return until 
//									 finished.
//		progressCallback  : [in] Called whenever new data is received. Can be 
//									 NULL. 
//		completedCallback : [in] Called when the request has finished. Can be 
//									 NULL. 
//		param             : [in] Optional free-format user data to send to 
//									 the callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise, it identifies the request.
// Remarks
//		This function is similar to ghttpGetEx, except it only gets the
//		 response headers.
//		<p />
//		This is done by making an HEAD request instead of a GET request, which
//		 instructs the HTTP server to leave the body out of the response.
//		<p />
//		Use ghttpHead for a simpler version of this function.
// See Also
//		ghttpGetEx, ghttpSaveEx, ghttpStreamEx, ghttpHead, ghttpPostEx
COMMON_API GHTTPRequest ghttpHeadEx
(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* headers,			// Optional headers to pass with the request.  Can be NULL or "".
	GHTTPBool				throttle,			// If true, throttle this connection's download speed.
	GHTTPBool				blocking,			// If true, this call doesn't return until finished
	ghttpProgressCallback	progressCallback,	// Called whenever new data is received.
	ghttpCompletedCallback	completedCallback,	// Called when the request has finished.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPost
// Summary
// 		Do an HTTP POST, which can be used to upload data to a web server.
// Parameters
//		URL               : [in] URL 
//		post              : [in] The data to be posted. 
//		blocking          : [in] If true, this call doesn't return until 
//									 finished.
//		completedCallback : [in] Called when the file has finished streaming.
//									 Can be NULL. 
//		param             : [in] Optional free-format user data to send to the
//									 callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise it identifies the request.
// Remarks
//		This function is used to post data to a web page, ignoring any possible 
//		 response body sent by the server (response status and response headers
//		 can still be checked). If you want to post data and receive a 
//		 response, use ghttpGetEx, ghttpSaveEx, or ghttpStreamEx.
//		<p />
//		Use ghttpPostEx for extra optional parameters.
//		<p />
//		Use this function to do an HTTP Post, don't try to access a GHTTPPost
//		 object directly.
// See Also
//		ghttpGet, ghttpGetEx, ghttpSave, ghttpSaveEx, ghttpStream,
//		ghttpStreamEx, ghttpHead, ghttpPostEx
GHTTPRequest ghttpPost
(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	GHTTPPost				post,				// The data to be posted.
	GHTTPBool				blocking,			// If true, this call doesn't return until finished
	ghttpCompletedCallback	completedCallback,	// Called when the file has finished streaming.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostEx
// Summary
// 		Do an HTTP POST, which can be used to upload data to a web server.
// Parameters
//		URL               : [in] URL 
//		headers           : [in] Optional headers to pass with the request.
//									 Can be NULL or "". 
//		post              : [in] The data to be posted. 
//		throttle          : [in] If true, throttle this connection's download
//									 speed. 
//		blocking          : [in] If true, this call doesn't return until 
//									 finished. 
//		progressCallback  : [in] Called whenever new data is received. Can be
//									 NULL. 
//		completedCallback : [in] Called when the file has finished streaming.
//									 Can be NULL.
//		param             : [in] Optional free-format user data to send to the
//									 callback.
// Returns
//		If less than 0, the request failed and this is a GHTTPRequestError
//		 value. Otherwise, it identifies the request.
// Remarks
//		This function is used to post data to a web page, ignoring any possible
//		 response body sent by the server (response status and response headers
//		 can still be checked). 
//		<p />
//		If you want to post data and receive a response, use ghttpGetEx,
//		ghttpSaveEx, or ghttpStreamEx.
//		<p />
//		Use ghttpPost for a simpler version of this function.
//		<p />
//		Use this function to do an HTTP Post, don't try to access a GHTTPPost
//		object directly.
// See Also
//		ghttpGetEx, ghttpSaveEx, ghttpStreamEx, ghttpHeadEx, ghttpPost
COMMON_API GHTTPRequest ghttpPostEx
(
	const gsi_char			* URL,				// The URL for the file ("http://host.domain[:port]/path/filename").
	const gsi_char			* headers,			// Optional headers to pass with the request.  Can be NULL or "".
	GHTTPPost				post,				// The data to be posted.
	GHTTPBool				throttle,			// If true, throttle this connection's download speed.
	GHTTPBool				blocking,			// If true, this call doesn't return until finished
	ghttpProgressCallback	progressCallback,   // Called whenever new data is received.
	ghttpCompletedCallback	completedCallback,	// Called when the file has finished streaming.
	void					* param             // User-data to be passed to the callbacks.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpThink
// Summary
// 		Processes all current HTTP requests.
// Remarks
//		Any application that uses GHTTP in non-blocking mode (i.e., that calls
//		 ghttp functions with the blocking argument set to GHTTPFalse) needs to
//		 call ghttpThink to let the library do any necessary processing. 
//		 Non-blocking mode should be use as much as possible.
//		<p />
//		This call will process any current requests and call any callbacks if 
//		 necessary. It should typically be called as part of the application's
//		 main loop. While it can be called as seldom as a few times a second,
//		 it should be called closer to 10-20 times a second. If downloading
//		 larger files, it may be desirable to call it even more often to 
//		 ensure that incoming buffers are emptied to make room for more
//		 incoming data.
//		<p />
//		Threads note: Making GHTTP requests concurrently from multiple threads
//		 is currently only supported on Windows. When using GHTTP from multiple
//		 threads, instead of calling ghttpThink, use ghttpRequestThink for each
//		 individual request. This allows that request's callback to be called
//		 from within the same thread in which it was started.
// See Also
//		ghttpRequestThink
COMMON_API void ghttpThink(void);

//////////////////////////////////////////////////////////////////////////////
// ghttpRequestThink
// Summary
// 		Process one particular HTTP request on Windows.
// Parameters
//		request : [in] A valid request object to process. 
// Returns
//		GHTTPFalse if the request cannot be found.
// Remarks
//		This allows an HTTP request to be processed in a separate thread.
//		This function is only supported on Windows.
// See Also
//		ghttpThink
COMMON_API GHTTPBool ghttpRequestThink(
	GHTTPRequest request
);

//////////////////////////////////////////////////////////////////////////////
// ghttpCancelRequest
// Summary
// 		Cancel an HTTP request in progress.
// Parameters
//		request : [in] A valid GHTTPRequest object. 
// Remarks
//		The GHTTPRequest should not be referenced once this function returns.
COMMON_API void ghttpCancelRequest(
	GHTTPRequest	request
);

//DOM-IGNORE-BEGIN
// Closes a request gracefully using shutdown(s, SD_SEND)
// PS2-Insock does not support partial shutdown.
COMMON_API void ghttpCloseRequest(
	GHTTPRequest	request
);
//DOM-IGNORE-END

//////////////////////////////////////////////////////////////////////////////
// ghttpGetState
// Summary
// 		Obtain the current state of an HTTP request.
// Parameters
//		request : [in] A valid request object 
// Returns
//		The state of an HTTP request.
COMMON_API GHTTPState ghttpGetState(
	GHTTPRequest	request
);

//////////////////////////////////////////////////////////////////////////////
// ghttpGetResponseStatus
// Summary
// 		Get an HTTP response's status string and status code.
// Parameters
//		request		: [in] A valid request object 
//		statusCode	: [out] Status code. 
// Returns
//		The response's status string.
// Remarks
//		Can only be used if the state has passed GHTTPReceivingStatus.
//		The status string is a user-readable representation of the result of 
//		 the request.
//		<p />
//		The status code is a 3 digit number which can be used to get more
//		 details on the result of the request.
//		<p />
//		There are 5 possible values for the first digit:
//		<emit \<ul\>>
//			<emit \<li\>>
//		1xx: Informational
//			<emit \</li\>>
//			<emit \<li\>>
//		2xx: Success
//			<emit \</li\>>
//			<emit \<li\>>
//		3xx: Redirection
//			<emit \</li\>>
//			<emit \<li\>>
//		4xx: Client Error
//			<emit \</li\>>
//			<emit \<li\>>
//		5xx: Server Error
//			<emit \</li\>>
//		<emit \</ul\>>
//		<p />
//		See RFC2616 (HTTP 1.1) and any follow-up RFCs for more information on
//		 specific codes.
// See Also
//		ghttpGetState
COMMON_API const char * ghttpGetResponseStatus(
	GHTTPRequest	request,       // The request of which to get the response state.
	int				* statusCode            // If not NULL, the status code is stored here.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpGetHeaders
// Summary
// 		Get the response headers from an HTTP request.
// Parameters
//		request : [in] A valid request object 
// Returns
//		The headers returned in the response.
// Remarks
//		Only valid if the request's state is GHTTPReceivingHeaders.
// See Also
//		ghttpGetState
COMMON_API const char * ghttpGetHeaders(
	GHTTPRequest	request
);

//////////////////////////////////////////////////////////////////////////////
// ghttpGetURL
// Summary
// 		Used to obtain the URL associated with an HTTP request.
// Parameters
//		request : [in] A valid request object 
// Returns
//		The URL associated with the request.
// Remarks
//		If the request has been redirected, this function will return the
//		 new URL, not the original URL.
COMMON_API const char * ghttpGetURL(
	GHTTPRequest request
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSetProxy
// Summary
// 		Sets a proxy server address.
// Parameters
//		server : [in] The address of the proxy server. 
// Returns
//		GHTTPFalse if the server format is invalid.
// Remarks
//		The address must be of the form "<server>[:port]". If port is omitted,
//		 80 will be used.
//		<p />
//		If server is NULL or "", no proxy server will be used. This should not
//		 be called while there are any current requests.
// See Also
//		ghttpSetRequestProxy
COMMON_API GHTTPBool ghttpSetProxy(
	const char	* server
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSetRequestProxy
// Summary
// 		Sets a proxy server for a specific request.
// Parameters
//		request : [in] A valid request object 
//		server  : [in] The address of the proxy server. 
// Returns
//		GHTTPFalse if the server format is invalid or the request is invalid.
// Remarks
//		The address must be of the form "<server>[:port]". If port is omitted,
//		 80 will be used.
//		<p />
//		If server is NULL or "", no proxy server will be used. This should not
//		 be called while there are any current requests.
COMMON_API GHTTPBool ghttpSetRequestProxy(
	GHTTPRequest	request,
	const char		* server
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSetThrottle
// Summary
// 		Used to start/stop throttling an existing connection.
// Parameters
//		request  : [in] A valid request object 
//		throttle : [in] True or false to enable or disable throttling.
// Remarks
//		This may not be as efficient as starting a request with the desired
//		 setting.
// See Also
//		ghttpThrottleSettings
COMMON_API void ghttpSetThrottle(
	GHTTPRequest	request,
	GHTTPBool		throttle
);

//////////////////////////////////////////////////////////////////////////////
// ghttpThrottleSettings
// Summary
// 		Used to adjust the throttle settings.
// Parameters
//		bufferSize : [in] The number of bytes to get each receive. 
//		timeDelay : [in] How often to receive data, in milliseconds.
// Remarks
//		The throttle settings affect any request initiated with throttling,	or 
//		 for which throttling is enabled with ghttpSetThrottle.
// See Also
//		ghttpSetThrottle
COMMON_API void ghttpThrottleSettings(
	int			bufferSize,     // The number of bytes to get each receive.
	gsi_time	timeDelay  // How often to receive data, in milliseconds.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpSetMaxRecvTime
// Summary
// 		Used to throttle based on time, not on bandwidth.
// Parameters
//		request     : [in] A valid request object 
//		maxRecvTime : [in] Maximum receive time 
// Remarks
//		Prevents recv-loop blocking on ultrafast connections without directly
//		 limiting transfer rate.
COMMON_API void ghttpSetMaxRecvTime(
	GHTTPRequest	request,
	gsi_time		maxRecvTime
);

//////////////////////////////////////////////////////////////////////////////
// ghttpNewPost
// Summary
// 		Creates a new post object, which is used to represent data to send
//		 a web server as part of an HTTP request.
// Returns
//		The newly created post object, or NULL if it cannot be created.
// Remarks
//		After getting the post object, use the ghttpPostAdd*() functions to
//		 add data to the object, and ghttpPostSetCallback() to add a callback 
//		 to monitor the progress of the data upload.
//		<p />
//		By default, post objects automatically free themselves after posting.
//		 To use the same post with more than one request, set auto-free to 
//		 false, then use ghttpFreePost to free it after all requests it's 
//		 being used for are completed.
// See Also
//		ghttpPostAddString, ghttpPostAddFileFromDisk, ghttpPostAddFileFromMemory, 
//		ghttpPostSetAutoFree, ghttpFreePost, ghttpPostSetCallback
COMMON_API GHTTPPost ghttpNewPost(void);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostSetAutoFree
// Summary
// 		Sets a post object's auto-free flag.
// Parameters
//		post     : [in] Post object 
//		autoFree : [in] True if object should be auto-freed 
// Remarks
//		By default post objects automatically free themselves after posting.
//		To use the same post with more than one request, set auto-free to
//		 false, then use ghttpFreePost to free it after every request it's
//		 being used in is completed.
//		<p />
//		Use this function to do an HTTP Post, don't try to access a GHTTPPost
//		object directly.
// See Also
//		ghttpNewPost, ghttpFreePost, ghttpPost
COMMON_API void ghttpPostSetAutoFree(
	GHTTPPost	post,
	GHTTPBool	autoFree
);

//////////////////////////////////////////////////////////////////////////////
// ghttpFreePost
// Summary
// 		Release the specified post object.
// Parameters
//		post : [in] Post object created with ghttpNewPost. 
// Remarks
//		By default, post objects created with ghttpNewPost will be
//		 automatically freed after being used in a request. However,
//		 ghttpPostSetAutoFree can be used to turn off the post object's
//		 auto-free property. This can be useful if a single post object 
//		 will be used in multiple requests.
//		<p />
//		You should then use this function to manually free the post
//		 object after the last request it has been used in completes.
// See Also
//		ghttpNewPost, ghttpPostSetAutoFree
COMMON_API void ghttpFreePost(
	GHTTPPost	post	// The post object to free.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpFreePostandUpdateConnection
// Summary
// 		Free a post object.
// Parameters
//		post : [in] Post object created with ghttpNewPost. 
// Remarks
//		By default, post objects created with ghttpNewPost will be
//		 automatically freed after being used in a request.	However
//		 ghttpPostSetAutoFree can be used to turn off the post object's 
//		 auto-free property. This can be useful if a single post object will be
//		 used in multiple requests. You should then use this function to
//		 manually free the post object after the last request it has been used
//		 in completes.
// See Also
//		ghttpNewPost, ghttpPostSetAutoFree
COMMON_API void ghttpFreePostAndUpdateConnection(
    GHTTPRequest	requestId,
    GHTTPPost		post		// The post object to free.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostAddString
// Summary
// 		Adds a string to the specified post object.
// Parameters
//		post   : [in] Post object 
//		name   : [in] The name to attach to this string. 
//		string : [in] The string to send. 
// Returns
//		GHTTPTrue if the string was added successfully.
// Remarks
//		If a post object only contains string, the content type for the
//		upload will be the "application/x-www-form/urlencoded". If any 
//		files are added, the content type for the upload will become 
//		"multipart/form-data".
//		<p />
//		Use this function to do an HTTP Post, don't try to access a GHTTPPost
//		object directly.
// See Also
//		ghttpNewPost, ghttpPost, ghttpPostAddFileFromDisk,
//		ghttpPostAddFileFromMemory
COMMON_API GHTTPBool ghttpPostAddString(
	GHTTPPost post,             // The post object to add to.
	const gsi_char * name,      // The name to attach to this string.
	const gsi_char * string     // The actual string.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostAddFileFromDisk
// Summary
// 		Adds a disk file to the specified post object.
// Parameters
//		post           : [in] Post object 
//		name           : [in] The name to attach to this file. 
//		filename       : [in] The name (and possibly path) to the file to 
//								upload. 
//		reportFilename : [in] The filename given to the web server. 
//		contentType    : [in] The MIME type for this file. 
// Returns
//		GHTTPTrue if the file was added successfully.
// Remarks
//		The reportFilename is what is reported to the server as the filename.
//		 If NULL or empty, the filename will be used (including any possible
//		 path). The contentType is the MIME type to report for this file. If
//		 NULL, "application/octet-stream" is used.
//		<p />
//		The file isn't read from until the data is actually sent to the 
//		 server. When uploading files the content type of the overall 
//		 request (as opposed to the content this of this file) will be 
//		 "multipart/form-data".
//		<p />
//		Use this function to do an HTTP Post, don't try to access a 
//		 GHTTPPost object directly.
// See Also
//		ghttpNewPost, ghttpPost, ghttpPostAddString, ghttpPostAddFileFromMemory
COMMON_API GHTTPBool ghttpPostAddFileFromDisk(
	GHTTPPost		post,            // The post object to add to.
	const gsi_char	* name,          // The name to attach to this file.
	const gsi_char	* filename,      // The name (and possibly path) to the file to upload.
	const gsi_char	* reportFilename,// The filename given to the web server.
	const gsi_char	* contentType    // The MIME type for this file.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostAddFileFromMemory
// Summary
// 		Adds a file from memory to the specified post object.
// Parameters
//		post           : [in] Post object 
//		name           : [in] The name to attach to this file. 
//		buffer         : [in] The data to send. 
//		bufferLen      : [in] The number of bytes of data to send. 
//		reportFilename : [in] The filename given to the web server. 
//		contentType    : [in] The MIME type for this file.
// Returns
//		GHTTPTrue if the file was added successfully.
// Remarks
//		The reportFilename is what is reported to the server as the
//		 filename. It cannot be NULL or empty.
//		<p />
//		The contentType is the MIME type to report for this file. If NULL,
//		 "application/octet-stream" is used.
//		<p />
//		The data is not copied off in this call. The data pointer is read from
//		 as the data is actually sent to the server. The pointer must remain
//		 valid during requests.When uploading files the content type of the
//		 overall request (as opposed to the content this of this file) will be
//		 "multipart/form-data".
//		<p />
//		Use this function to do an HTTP Post, don't try to access a GHTTPPost
//		 object directly.
// See Also
//		ghttpNewPost, ghttpPost, ghttpPostAddFileFromDisk, ghttpPostAddString
COMMON_API GHTTPBool ghttpPostAddFileFromMemory(
	GHTTPPost		post,				// The post object to add to.
	const gsi_char	* name,				// The name to attach to this string.
	const char		* buffer,			// The data to send.
	int				bufferLen,          // The number of bytes of data to send.
	const gsi_char	* reportFilename,	// The filename given to the web server.
	const gsi_char	* contentType		// The MIME type for this file.
);

//DOM-IGNORE-BEGIN
//////////////////////////////////////////////////////////////////////////////
// ghttpPostAddXml
// Summary
//		Adds an XML SOAP object to the post object.
// Remarks
//		Content-Type = text/xml
//		The most common use of this function is to add ghttpSoap data.
// See Also
//		ghttpNewSoap
GHTTPBool ghttpPostAddXml(
	GHTTPPost			post,
	GSXmlStreamWriter	xmlSoap
);
//DOM-IGNORE-END

//////////////////////////////////////////////////////////////////////////////
// ghttpPostCallback
// Summary
// 		Called during requests to let the app know how much of the post data 
//		 has been uploaded.
// Parameters
//		request 		: [in] A valid request object 
//		bytesPosted		: [in] The number of bytes of data posted so far. 
//		totalBytes 		: [in] The total number of bytes being posted. 
//		objectsPosted 	: [in] The total number of data objects uploaded so far. 
//		totalObjects 	: [in] The total number of data objects to upload. 
//		param 			: [in] Optional free-format user data for use by the callback
// See Also
//		ghttpNewPost, ghttpPostSetCallback
typedef void (* ghttpPostCallback)(
	GHTTPRequest	request,		// The request.
	int				bytesPosted,    // The number of bytes of data posted so far.
	int				totalBytes,     // The total number of bytes being posted.
	int				objectsPosted,	// The total number of data objects uploaded so far.
	int				totalObjects,   // The total number of data objects to upload.
	void			* param         // User-data.
);

//////////////////////////////////////////////////////////////////////////////
// ghttpPostSetCallback
// Summary
// 		Sets the callback for a post object.
// Parameters
//		post 		: [in] The post object to set the callback on. 
//		callback 	: [in] The callback to call when using this post object. 
//		param 		: [in] User data passed to the callback.
// See Also
//		ghttpNewPost
COMMON_API void ghttpPostSetCallback(
	GHTTPPost			post,       // The post object to set the callback on.
	ghttpPostCallback	callback,	// The callback to call when using this post object.
	void				* param     // User-data passed to the callback.
);

//DOM-IGNORE-BEGIN
//////////////////////////////////////////////////////////////////////////////
// ghttpSetRequestEncryptionEngine
// Summary
//		Select which encryption method to use.
// Remarks
//		Content-Type = text/xml
//		<p />
//		The most common use of this function is to add ghttpSoap data.
// See Also
//		GHTTPEncryptionEngine
GHTTPBool ghttpSetRequestEncryptionEngine(
	GHTTPRequest			request,
	GHTTPEncryptionEngine	engine
);
//DOM-IGNORE-END
// ghttpSetRootCAList
// Summary
//      This function is used for setting the root certificate list when making
//		 https calls, if the SSL Engine used supports the setting of expected
//		 server root certificate from validation purposes.
// Parameters
//      url       : [in] 
//      theRootCA : [in] A void pointer to the certificate data.
// Returns
//		GHTTPTrue if the rootCA set successfully.
// Remarks
//      Currently, it is ONLY used by TWL applications. Others will return
//		 failure (GHTTPFalse).    
//      The application must allocate/de-allocate the memory passed as
//       theRootCA. This pointer must be valid until the application has
//		 completed. 
GHTTPBool ghttpSetRootCAList( char *url, void *theRootCA);

// ghttpCleanupRootCAList
// Summary
//      This function is used resetting the root certificate list.
// Parameters
//      url : [in] url used to initialized the certificate.
// Returns
//		GHTTPTrue if the rootCA reset successfully.
// Remarks
//      Currently, it is ONLY used by TWL applications. Others will 
//       return failure (GHTTPFalse). 
GHTTPBool ghttpCleanupRootCAList(char *url);  


// These are defined for backwards compatibility with the "file" function names.
////////////////////////////////////////////////////////////////////////////////
#define ghttpGetFile(a, b, c, d)                      ghttpGet(a, b, c, d)
#define ghttpGetFileEx(a, b, c, d, e, f, g, h, i, j)  ghttpGetEx(a, b, c, d, e, f, g, h, i, j)
#define ghttpSaveFile(a, b, c, d, e)                  ghttpSave(a, b, c, d, e)
#define ghttpSaveFileEx(a, b, c, d, e, f, g, h, i)    ghttpSaveEx(a, b, c, d, e, f, g, h, i)
#define ghttpStreamFile(a, b, c, d, e)                ghttpStream(a, b, c, d, e)
#define ghttpStreamFileEx(a, b, c, d, e, f, g, h)     ghttpStreamEx(a, b, c, d, e, f, g, h)
#define ghttpHeadFile(a, b, c, d)                     ghttpHead(a, b, c, d)
#define ghttpHeadFileEx(a, b, c, d, e, f, g)          ghttpHeadEx(a, b, c, d, e, f, g)

//DOM-IGNORE-BEGIN
// This ASCII version needs to be defined even in UNICODE mode
COMMON_API GHTTPRequest ghttpGetA
(
	const char * URL,       // The URL for the file ("http://host.domain[:port]/path/filename").
	GHTTPBool blocking,     // If true, this call doesn't return until the file has been received.
	ghttpCompletedCallback completedCallback,  // Called when the file has been received.
	void * param            // User-data to be passed to the callbacks.
);
//DOM-IGNORE-END

#define ghttpGetFileA(a, b, c, d)                      ghttpGetA(a, b, c, d)

#ifdef __cplusplus
}
#endif

#endif
