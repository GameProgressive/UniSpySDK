///////////////////////////////////////////////////////////////////////////////
// File:	gt2Auth.c
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// -----------------------

#include "gt2Main.h"
#include "gt2Auth.h"
#include <stdlib.h>

#define CALCULATEODDMODE(buffer, i, oddmode) ((buffer[i-1] & 1) ^ (i & 1) ^ oddmode ^ (buffer[0] & 1) ^ ((buffer[0] < 79) ? 1 : 0) ^ ((buffer[i-1] < buffer[0]) ? 1 : 0));

char GT2ChallengeKey[33] = "3b8dd8995f7c40a9a5c5b7dd5b481341";

static int gti2VerifyChallenge(const GT2Byte *buffer)
{
	int oddmode = 0;
	int i;
	for (i = 1; i < GTI2_CHALLENGE_LEN ; i++)
	{
		oddmode = CALCULATEODDMODE(buffer,i, oddmode);
		if ((oddmode && (buffer[i] & 1) == 0) || (!oddmode && ((buffer[i] & 1) == 1)))
			return 0; //failed!!
	}
	return 1;
}

GT2Byte * gti2GetChallenge
(
	GT2Byte * buffer
)
{
	int i;
	int oddmode;
	GS_ASSERT(buffer);

	srand((unsigned int)current_time());
	buffer[0] = (GT2Byte)(33 + rand() % 93); // Use chars in the range of 33 - 125.
	oddmode = 0;
	for (i = 1; i < GTI2_CHALLENGE_LEN ; i++)
	{
		oddmode = CALCULATEODDMODE(buffer,i, oddmode);
		buffer[i] = (GT2Byte)(33 + rand() % 93); // Use chars in the range of 33 - 125.
		// If oddmode, make sure the char is odd; otherwise, make sure the char is even.
		if ((oddmode && (buffer[i] & 1) == 0) || (!oddmode && ((buffer[i] & 1) == 1)))
			buffer[i]++;

	}
	return buffer;
}

GT2Byte * gti2GetResponse
(
	GT2Byte * buffer,
	const GT2Byte * challenge
)
{
	int i;
	int valid;
	char cchar;
	const int keylen = (int)sizeof(GT2ChallengeKey)/sizeof(GT2ChallengeKey[0]) - 1;
	int chalrand;
	valid = gti2VerifyChallenge(challenge); // It's an invalid challenge, so give them a bogus response.
	for (i = 0 ; i < GTI2_RESPONSE_LEN ; i++)
	{
		// Use random values for spots 0 and 13.
		if (!valid || i == 0 || i == 13)
			buffer[i] = (GT2Byte)(33 + rand() % 93); // Use chars in the range of 33 - 125.
		else
		{ // NOTE: Set the character to look back at, never use the random ones!
			if (i == 1 || i == 14)
				cchar = (char)challenge[i];
			else
				cchar = (char)challenge[i-1];
			chalrand = abs((challenge[((i * challenge[i]) + GT2ChallengeKey[(i + challenge[i]) % keylen]) % GTI2_CHALLENGE_LEN] ^ GT2ChallengeKey[(i * 17991 * cchar) % keylen]));
			buffer[i] = (GT2Byte)(33 + chalrand % 93);
		}
	}
	return buffer;
}


GT2Bool gti2CheckResponse
(
	const GT2Byte * response1,
	const GT2Byte * response2
)
{
	int i; // When comparing, ignore the ones that are random.
	for (i = 0 ; i < GTI2_RESPONSE_LEN ; i++)
	{
		if (i != 0 && i != 13 && response1[i] != response2[i])
			return GT2False;
	}
	return GT2True;
}

