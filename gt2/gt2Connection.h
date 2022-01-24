///////////////////////////////////////////////////////////////////////////////
// File:	gt2Connection.h
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GT2_CONNECTION_H_
#define _GT2_CONNECTION_H_

#include "gt2Main.h"

GT2Result gti2NewOutgoingConnection(GT2Socket socket, GT2Connection * connection, unsigned int ip, unsigned short port);
GT2Result gti2NewIncomingConnection(GT2Socket socket, GT2Connection * connection, unsigned int ip, unsigned short port);

GT2Result gti2StartConnectionAttempt
(
	GT2Connection connection,
	const GT2Byte * message,
	int len,
	GT2ConnectionCallbacks * callbacks
);

GT2Bool gti2AcceptConnection(GT2Connection connection, GT2ConnectionCallbacks * callbacks);

void gti2RejectConnection(GT2Connection connection, const GT2Byte * message, int len);

GT2Bool gti2ConnectionSendData(GT2Connection connection, const GT2Byte * message, int len);

GT2Bool gti2ConnectionThink(GT2Connection connection, gsi_time now);

void gti2CloseConnection(GT2Connection connection, GT2Bool hard);

void gti2ConnectionClosed(GT2Connection connection);

void gti2ConnectionCleanup(GT2Connection connection);

#endif
