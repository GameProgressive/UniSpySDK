///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatformUtil.h
// SDK:		GameSpy Common
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __GSUTILITY_H__
#define __GSUTILITY_H__

#include "gsPlatform.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Async DNS lookup

// async way to resolve a hostname to an IP
typedef struct GSIResolveHostnameInfo * GSIResolveHostnameHandle;
#define GSI_STILL_RESOLVING_HOSTNAME   0
#define GSI_ERROR_RESOLVING_HOSTNAME   0xFFFFFFFF

// start resolving a hostname
// returns 0 on success, -1 on error
int  gsiStartResolvingHostname(const char * hostname, GSIResolveHostnameHandle * handle);
// cancel a resolve in progress
void gsiCancelResolvingHostname(GSIResolveHostnameHandle handle);
// returns GSI_STILL_RESOLVING if still resolving the hostname
// returns GSI_ERROR_RESOLVING if it was unable to resolve the hostname
// on success, returns the IP of the host in network byte order
unsigned int gsiGetResolvedIP(GSIResolveHostnameHandle handle);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Get rid of compiler warnings when parameters are never used
// (Mainly used in sample apps and callback for platform switches)

#ifndef GSI_UNUSED
	#define GSI_UNUSED(x)
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Cross platform random number generator
void Util_RandSeed(unsigned long seed); // to seed it
int  Util_RandInt(int low, int high);   // retrieve a random int


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Base 64 encoding (printable characters)
void B64Encode(const char *input, char *output, int inlen, int encodingType);
void B64Decode(const char *input, char *output, int inlen, int * outlen, int encodingType);

// returns the length of the binary data represented by the base64 input string
int B64DecodeLen(const char *input, int encodingType);

typedef struct
{
	const char *input;
	int len;
	int encodingType;
} B64StreamData;

void B64InitEncodeStream(B64StreamData *data, const char *input, int len, int encodingType);

// returns gsi_false if the stream has ended
gsi_bool B64EncodeStream(B64StreamData *data, char output[4]);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define XXTEA_KEY_SIZE 17
gsi_i8 * gsXxteaEncrypt(const gsi_i8 * iStr, gsi_i32 iLength, gsi_i8 key[XXTEA_KEY_SIZE], gsi_i32 *oLength);
gsi_i8 * gsXxteaDecrypt(const gsi_i8 * iStr, gsi_i32 iLength, gsi_i8 key[XXTEA_KEY_SIZE], gsi_i32 *oLength);


#define GS_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define GS_MIN(a,b)    (((a) < (b)) ? (a) : (b))


#if defined(_DEBUG)
	void gsiCheckStack(void);
#else
	#define gsiCheckStack() 
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// time functions

gsi_time current_time();         // milliseconds
gsi_time current_time_hires();   // microseconds
void msleep(gsi_time msec);      // milliseconds

// GSI equivalent of common C-lib time functions
struct tm * gsiSecondsToDate(const time_t *timp);		//gmtime
time_t  gsiDateToSeconds(struct tm *tb);				//mktime
char * gsiSecondsToString(const time_t *timp);			//ctime


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Misc utilities

	
#ifndef GS_NO_TIME_H
	#include <time.h>
#endif

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

typedef const char * (* GetUniqueIDFunction)();

extern GetUniqueIDFunction GOAGetUniqueID;

// 64-bit Integer reads and writes
gsi_i64 gsiStringToInt64(const char *theNumberStr);
void gsiInt64ToString(char theNumberStr[33], gsi_i64 theNumber);
double gsiWStringToDouble(const wchar_t *inputString);

//fopen wrapper - needed for restricted file systems (i.e. iPhone)
FILE * gsifopen(const char *filename, const char *mode);

#ifdef GS_USE_REFLECTOR

// Make sure the ip and port are in network byte order for this function's parameters.
void gsSetReflectorAddress(gsi_u32 ip, gsi_u16 port);

extern gsi_u32 gsReflectorIP;
extern gsi_u16 gsReflectorPort;

#endif // GS_USE_REFLECTOR


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif //__GSUTILITY_H__

