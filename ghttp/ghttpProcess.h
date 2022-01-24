///////////////////////////////////////////////////////////////////////////////
// File:	ghttpProcess.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#ifndef _GHTTPPROCESS_H_
#define _GHTTPPROCESS_H_

#include "ghttpMain.h"
#include "ghttpConnection.h"

#ifdef __cplusplus
extern "C" {
#endif

void ghiDoSocketInit	  (GHIConnection * connection);
void ghiDoHostLookup      (GHIConnection * connection);
void ghiDoLookupPending   (GHIConnection * connection);
void ghiDoConnecting      (GHIConnection * connection);
#ifdef GS_USE_REFLECTOR
void ghiDoReflectorHeader (GHIConnection * connection);
#endif
void ghiDoSecuringSession (GHIConnection * connection);
void ghiDoSendingRequest  (GHIConnection * connection);
void ghiDoPosting         (GHIConnection * connection);
void ghiDoWaiting         (GHIConnection * connection);
void ghiDoReceivingStatus (GHIConnection * connection);
void ghiDoReceivingHeaders(GHIConnection * connection);
void ghiDoReceivingFile   (GHIConnection * connection);

#ifdef __cplusplus
}
#endif

#endif
