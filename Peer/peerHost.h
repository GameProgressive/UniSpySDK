///////////////////////////////////////////////////////////////////////////////
// File:	peerHost.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _PEERHOST_H_
#define _PEERHOST_H_

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
PEERBool piStartHosting(PEER peer, SOCKET socket, unsigned short port);
void piStopHosting(PEER peer, PEERBool stopReporting);

#ifdef __cplusplus
}
#endif

#endif
