///////////////////////////////////////////////////////////////////////////////
// File:	gcdkeyc.c
// SDK:		GameSpy CD Key SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../common/md5.h"
#include "gcdkeyc.h"
#include "../common/gsCommon.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RAWSIZE 512

#ifdef __cplusplus
extern "C" {
#endif

	// method = 0 for normal auth response from game server
	// method = 1 for reauth response originating from keymaster
void gcd_compute_response(char *cdkey, char *challenge, char response[RESPONSE_SIZE], CDResponseMethod method)
{
	char rawout[RAWSIZE];
	unsigned int anyrandom;
	char randstr[9];


	/* check to make sure we weren't passed a huge cd key/challenge */
	if (strlen(cdkey) * 2 + strlen(challenge) + 8 >= RAWSIZE)
	{
		gsiSafeStrcpyA(response, "CD Key or challenge too long", RESPONSE_SIZE);
		return;
	}

	/* make sure we are randomized */
	srand((unsigned int)time(NULL) ^ 0x33333333);
	/* Since RAND_MAX is 16 bit on many systems, make sure we get a 32 bit number */
	anyrandom = (rand() << 16 | rand()); 
	sprintf(randstr,"%.8x",anyrandom);

	/* auth response   = MD5(cdkey + random mod 0xffff + challenge) */
	/* reauth response = MD5(challenge + random mode 0xffff + cdkey) */ 
	if (method == 0)
		sprintf(rawout, "%s%d%s",cdkey, anyrandom % 0xFFFF , challenge );
	else
		sprintf(rawout, "%s%d%s",challenge, anyrandom % 0xFFFF, cdkey);

	/* do the cd key md5 */
	GSMD5Digest((unsigned char *)cdkey, strlen(cdkey), response);
	/* add the random value */
	gsiSafeStrcpyA(&response[32], randstr, RESPONSE_SIZE - 32);
	/* do the response md5 */
	GSMD5Digest((unsigned char *)rawout, strlen(rawout), &response[40]);	
}


#ifdef __cplusplus
}
#endif
