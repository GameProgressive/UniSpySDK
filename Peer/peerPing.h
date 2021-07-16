///////////////////////////////////////////////////////////////////////////////
// File:	peerPing.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _PEERPING_H_
#define _PEERPING_H_

/*************
** INCLUDES **
*************/
#include "peerMain.h"


#ifdef __cplusplus
extern "C" {
#endif


/**************
** FUNCTIONS **
**************/
PEERBool piPingInit(PEER peer);
void piPingCleanup(PEER peer);
void piPingThink(PEER peer);
PEERBool piPingInitPlayer(PEER peer, piPlayer * player);
void piPingPlayerJoinedRoom(PEER peer, piPlayer * player, RoomType roomType);
void piPingPlayerLeftRoom(PEER peer, piPlayer * player);
void piUpdateXping(PEER peer, const char * nick1, const char * nick2, int ping);
PEERBool piGetXping(PEER peer, const char * nick1, const char * nick2, int * ping);

#ifdef __cplusplus
}
#endif

#endif
