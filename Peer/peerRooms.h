///////////////////////////////////////////////////////////////////////////////
// File:	peerRooms.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _PEERROOMS_H_
#define _PEERROOMS_H_

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
PEERBool piRoomsInit(PEER peer);
void piRoomsCleanup(PEER peer);
void piStartedEnteringRoom(PEER peer, RoomType roomType, const char * room);
void piFinishedEnteringRoom(PEER peer, RoomType roomType, const char * name);
void piLeaveRoom(PEER peer, RoomType roomType, const char * reason);
PEERBool piRoomToType(PEER peer, const char * room, RoomType * roomType);
void piSetLocalFlags(PEER peer);

#ifdef __cplusplus
}
#endif

#endif
