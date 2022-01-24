// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef __GSRESULTCODES_H__
#define __GSRESULTCODES_H__
#include "gsCommon.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Standardized Result Codes
//
//  Usage:
//      1) Build result codes as combination SDK and Detail code
//      2) Use macros to build return code values
//      3) Build up uses common schemes, similar to HRESULT (winerror.h)
//
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-+---------+-----------+---------------+-----------------------+
// |S| Reserved|    SDK    |    Section    |          Code         |
// +-+---------+-----------+---------------+-----------------------+
//
// Code    [12 bits] = 4096 Detail codes
// Section [ 8 bits] =  256 Section (aka Facility) codes
// SDK     [ 6 bits] =   64 SDK codes
// Reserved[ 3 bits] = expansion bits
// S       [ 1 bit ] =    1 Severity (1=Fail, 0=Success)
//
// Bit lengths selected for easy reading...
//		[0x80204002] = 8-02-04-002
//    - Codes that are negative are errors. [0x80000000]
//    - 02 is the SDK, in this case Direct2Game
//    - 04 is the Section, in this case State
//    - 002 is the code, in this case NoAvailabilityCheck

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Macros
typedef gsi_i32 GSResult;

#define GS_RESULT_CODEBITS    12
#define GS_RESULT_SECTIONBITS  8
#define GS_RESULT_SDKBITS      6

#define GS_RESULT(s,sdk,sec,code)   (GSResult)((s)<<31 | (sdk)<<20 | (sec)<<12 | (code))
#define GS_ERROR(sdk, sec, code)    GS_RESULT(1,sdk,sec,code)

#define GS_RESULT_CODE(result)      (GSResult)((result) & 0x00000FFF)
#define GS_RESULT_SECTION(result)   (GSResult)((result) & 0x000FF000)
#define GS_RESULT_SDK(result)       (GSResult)((result) & 0x03F00000)
#define GS_FAILED(result)           (gsi_bool)(((result) & 0x80000000)!=0)
#define GS_SUCCEEDED(result)        (gsi_bool)(((result) & 0x80000000)==0)

#define GS_SUCCESS 0 // generic success

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Result Set struct - used for multiple result responses
typedef struct
{
    int          mNumResults;
    GSResult    *mResults;
    UCS2String   mErrorMessage;
} GSResultSet;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Result SDKs (64)
typedef enum
{
	GSResultSDK_NULL = 0,
	GSResultSDK_Common,
	GSResultSDK_Direct2Game,
    GSResultSDK_Brigades,
    GSResultSDK_Sake,
    GSResultSDK_Unknown, 
	GSReturnCode_Count
} GSResultSDK;

// Result Sections (256)
typedef enum
{
	GSResultSection_NULL = 0,
	GSResultSection_Network,
	GSResultSection_File,
	GSResultSection_Memory,
	GSResultSection_State,
	GSResultSection_Soap,
	GSResultSection_Account,  
	GSResultSection_SdkSpecifc,
	// max
	GSReturnSection_Count
} GSResultSection;

// Result codes (4096)
//    - To view most codes, just & with 0xFF
typedef enum
{
	// General Error Codes
	GSResultCode_Success = 0,
	GSResultCode_UnknownError,     
	GSResultCode_NoAvailabilityCheck,
	GSResultCode_InvalidParameters,
	GSResultCode_OutOfMemory, 
	GSResutlCode_SocketError,
	GSResultCode_ServerError,
	GSResultCode_NotFound,
	GSResultCode_CouldNotParseXML,
	GSResultCode_ArgumentNull, 
	GSResultCode_NotImplemented,
	GSResultCode_GameNotFound,
	GSResultCode_ElementNotFound,
	GSResultCode_SizeExceedsMaximum,
	GSResultCode_OperationCancelled,
	GSResultCode_OperationFailed,
	GSResultCode_RequestTimedOut,
	GSResultCode_HttpError,
	GSResultCode_InvalidCertificate,
	GSResultCode_AccountNotFound,
	GSResultCode_ThreadCreateFailed,
	GSResultCode_BadUrl,
	GSResultCode_FileNotFound,
	GSResultCode_FileWriteError,
	GSResultCode_FileTooLarge,
    GSResultCode_FileFailedToOpen,
    GSResultCode_FileIsNotOwnedByUser,
    GSResultCode_HttpForbidden,
    GSResultCode_ServiceUninitialized,
    GSResultCode_FileCannotCreateDirectory,

	// response codes dealing with errors in response headers
	GSResultCode_INVALID_GAMEID,			// make sure GameID is properly set with wsSetCredentials
	GSResultCode_INVALID_SESSIONTOKEN,     // make sure wsSetCredentials was called with valid credentials and you have logged in via AuthService
	GSResultCode_SESSIONTOKEN_EXPIRED,      // re-login via AuthService to refresh your 'session'

	// Add new ones here

	GSResultCode_Count
} GSResultCode;

// helper function to clean up memory
void gsResultCodesFreeResultSet(GSResultSet *resultSet);
GSResult gsResultCodesCreateResultSet(GSResultSet **resultSet);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif // __GSRESULTCODES_H__
