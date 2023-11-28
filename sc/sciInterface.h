///////////////////////////////////////////////////////////////////////////////
// File:	sciInterface.h
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SCIINTERFACE_H__
#define __SCIINTERFACE_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "sci.h"
#include "sciWebServices.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// SDK instance
typedef struct
{
	SCWebServices mWebServices;
	gsi_u32       mGameId;
	//gsi_u32	  mOptionsFlags;
	gsi_bool      mInit;
	const char *  mServiceURL;
	GSCoreMgr *   mSdkCore;
	gsi_u8        mSessionId[SC_SESSION_GUID_SIZE];
	gsi_u8        mConnectionId[SC_CONNECTION_GUID_SIZE];
	//SCStatsPtr    mStats;
} SCInterface;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciInterfaceCreate (SCInterface** theInterfaceOut);
SCResult sciInterfaceInit   (SCInterface*  theInterface);
void     sciInterfaceDestroy(SCInterface*  theInterface);

void     sciInterfaceSetSessionId(SCInterface* theInterface, const char * theSessionId);
void     sciInterfaceSetConnectionId(SCInterface* theInterface, const char * theConnectionId);

gsi_u16  sciGetPlatformId(void);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SCIINTERFACE_H__
