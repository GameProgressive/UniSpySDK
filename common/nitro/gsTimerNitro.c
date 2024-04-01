///////////////////////////////////////////////////////////////////////////////
// File:	gsTimerNitro.c
// SDK:		GameSpy Common Nitro code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"

// note that this doesn't return the standard time() value
// because the DS doesn't know what timezone it's in
time_t time(time_t *timer)
{
	time_t t;

	assert(OS_IsTickAvailable() == TRUE);
	t = (time_t)OS_TicksToSeconds(OS_GetTick());

	if(timer)
		*timer = t;

	return t;
}