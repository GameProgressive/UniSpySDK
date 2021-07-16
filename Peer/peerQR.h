///////////////////////////////////////////////////////////////////////////////
// File:	peerQR.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _PEERQR_H_
#define _PEERQR_H_

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
#define PI_QUERYPORT       6500

/**************
** FUNCTIONS **
**************/
PEERBool piStartReporting(PEER peer, SOCKET socket, unsigned short port);
void piStopReporting(PEER peer);
void piSendStateChanged(PEER peer);
void piQRThink(PEER peer);
PEERBool piStartAutoMatchReporting(PEER peer);
void piStopAutoMatchReporting(PEER peer);

#ifdef __cplusplus
}
#endif

#endif
