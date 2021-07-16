///////////////////////////////////////////////////////////////////////////////
// File:	peerHost.c
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

/*************
** INCLUDES **
*************/
#include "peerQR.h"
#include "peerRooms.h"

/**************
** FUNCTIONS **
**************/
#ifdef __MWERKS__ // CodeWarrior will warn if not prototyped
PEERBool piStartHosting (PEER peer, SOCKET socket, unsigned short port);	// peerOperations.c
void piStopHosting(PEER peer, PEERBool stopReporting);						// peerOperations.c
#endif

PEERBool piStartHosting
(
	PEER peer,
	SOCKET socket,
	unsigned short port
)
{
	PEER_CONNECTION;

	// Check that we're not hosting.
	////////////////////////////////
	GS_ASSERT(!connection->hosting);
	if(connection->hosting)
		return PEERFalse;

	// Now we're hosting.
	/////////////////////
	connection->hosting = PEERTrue;

	// Start reporting.
	///////////////////
	if(!piStartReporting(peer, socket, port))
		return PEERFalse;

	return PEERTrue;
}

void piStopHosting
(
	PEER peer,
	PEERBool stopReporting
)
{
	PEER_CONNECTION;

	// Stop reporting.
	//////////////////
	if(stopReporting)
		piStopReporting(peer);

	// Check that we're hosting.
	////////////////////////////
	if(!connection->hosting)
		return;

	// Reset states.
	////////////////
	connection->hosting = PEERFalse;
	connection->playing = PEERFalse;
	connection->ready = PEERFalse;

	// Set the flags.
	/////////////////
	piSetLocalFlags(peer);
}
