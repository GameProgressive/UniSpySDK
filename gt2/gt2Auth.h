///////////////////////////////////////////////////////////////////////////////
// File:	gt2Auth.h
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GT2_AUTH_H_
#define _GT2_AUTH_H_

#define GTI2_CHALLENGE_LEN           32
#define GTI2_RESPONSE_LEN            32

#ifdef __cplusplus
extern "C" {
#endif

GT2Byte * gti2GetChallenge
(
	GT2Byte * buffer
);

GT2Byte * gti2GetResponse
(
	GT2Byte * buffer,
	const GT2Byte * challenge
);

GT2Bool gti2CheckResponse
(
	const GT2Byte * response1,
	const GT2Byte * response2
);

#ifdef __cplusplus
}
#endif

#endif
