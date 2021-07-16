///////////////////////////////////////////////////////////////////////////////
// File:	peerAutoMatch.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _PEERAUTOMATCH_H_
#define _PEERAUTOMATCH_H_

/*************
** INCLUDES **
*************/
#include "peerMain.h"

#ifdef __cplusplus
extern "C" {
#endif

/************
** DEFINES **
************/
#define PI_AUTOMATCH_RATING_KEY "gsi_am_rating"

/**************
** FUNCTIONS **
**************/
void piSetAutoMatchStatus(PEER peer, PEERAutoMatchStatus status);
void piStopAutoMatch(PEER peer);
PEERBool piJoinAutoMatchRoom(PEER peer, SBServer server);

#ifdef __cplusplus
}
#endif

#endif
