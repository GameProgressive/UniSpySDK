///////////////////////////////////////////////////////////////////////////////
// File:	gpiPS3.h
// SDK:		GameSpy Presence and Messaging SDK 
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ***********************************************************************
// Please see the GameSpy Presence SDK documentation for more information
// ***********************************************************************

#ifndef _GPIPS3_H_
#define _GPIPS3_H_

//INCLUDES
#include "gpi.h"
#include <np.h>
#include <np\common.h>
#include <sysutil/sysutil_common.h>


//DEFINES
#define	GPI_NP_SYNC_DELAY		5000	// Wait 5 seconds after login before doing any syncs.
#define	GPI_NP_STATUS_TIMEOUT	5000	// Timeout after 5 second max if NP status is not online.
#define	GPI_NP_NUM_TRANSACTIONS	32	    // Max num of simultaneous NP lookup transactions.

//STRUCTURES
typedef struct 
{
    int       npTransId;
    SceNpId   *npIdForAdd;
    gsi_bool  npLookupDone;
} npIdLookupTrans;

//FUNCTIONS
GPResult gpiInitializeNpBasic();
GPResult gpiCheckNpStatus(GPConnection *connection);
GPResult gpiDestroyNpBasic(GPConnection *connection);
GPResult gpiProcessNp(GPConnection *connection);
int gpiNpBasicCallback(int event, int retCode, uint32_t reqId, void *arg);
void gpiNpManagerCallback(int incomingEvent, int result, void *arg);

GPResult gpiSyncNpBuddies(GPConnection *connection);
void gpiSyncNpBuddiesCallback(GPConnection *pconnection, GPProfileSearchResponseArg *arg, void *param);
void gpiSyncNpDeletedBuddiesCallback(GPConnection *pconnection, GPGetInfoResponseArg *arg, void *param);
void gpiSyncNpDeletedBlocksCallback(GPConnection *pconnection, GPGetInfoResponseArg *arg, void *param);
void gpiGetUniquenickCallback(GPConnection *pConnection, GPGetInfoResponseArg *arg, void *param);

GPResult gpiSyncNpBlockList(GPConnection *connection);
void gpiSyncNpBlockListCallback(GPConnection *pconnection, GPProfileSearchResponseArg *arg, void *param);

GPResult gpiAddToNpBlockList(GPConnection *connection, int profileid);
void gpiAddToNpBlockListInfoCallback(GPConnection *pconnection, GPGetInfoResponseArg *arg, void *param);

#endif
