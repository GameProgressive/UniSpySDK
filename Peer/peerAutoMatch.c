///////////////////////////////////////////////////////////////////////////////
// File:	peerAutoMatch.c
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

/*************
** INCLUDES **
*************/
#include "peerAutoMatch.h"
#include "peerMangle.h"
#include "peerOperations.h"
#include "peerSB.h"
#include "peerQR.h"
#include "peerCallbacks.h"
#include "peerPlayers.h"
#include "peerRooms.h"

#if defined(_PS3)
typedef void* amcallback_t;
#elif defined(_PSP) || defined(_NITRO) || defined(_REVOLUTION)
typedef int amcallback_t;
#else
typedef intptr_t amcallback_t;
#endif

#define PEER_AM_INTERVALS 11
#define PEER_AM_INTERVAL_SIZE 20
/**************
** FUNCTIONS **
**************/
static PEERBool piCreateAutoMatchRoom(PEER peer);

static void piCleanAutoMatch(PEER peer)
{
	PEER_CONNECTION;

	// free memory
	gsifree(connection->autoMatchFilter);

	// remove the operation
	piRemoveOperation(peer, connection->autoMatchOperation);
	
	connection->autoMatchOperation = NULL;
}

void piSetAutoMatchStatus(PEER peer, PEERAutoMatchStatus status)
{
	piOperation * operation;
	PEER_CONNECTION;

	// make sure there is an operation
	operation = connection->autoMatchOperation;
	GS_ASSERT(operation);
	if(!operation)
		return;

	// we need to create a room before we can really switch to PEERWaiting
	if((status == PEERWaiting) && !connection->inRoom[StagingRoom])
	{
		if(!piCreateAutoMatchRoom(peer)) 
		{
			piSetAutoMatchStatus(peer, connection->autoMatchSBFailed?PEERFailed:PEERSearching);
		}
		return;
	}

	// check if this is already the current status
	if(connection->autoMatchStatus != status)
	{
		// set the new status
		connection->autoMatchStatus = status; 

		// call the developer's status callback
		if (status != PEERReady) {
			// add the callback
			piAddAutoMatchStatusCallback(peer);
		}
		else {
			// set the ready time to current so that we can add the status callback after PEERREADYWAIT duration
			connection->peerReadyTime = current_time();
		}
	}

	// handle the status
	switch(status)
	{		
	case PEERFailed:
		// stop
		piSBStopListingAutoMatches(peer);
		piStopAutoMatchReporting(peer);
		piLeaveRoom(peer, StagingRoom, "");

		// clean
		piCleanAutoMatch(peer);
		break;

	case PEERSearching:
		// stop
		piStopAutoMatchReporting(peer);
		piLeaveRoom(peer, StagingRoom, "");

		// start
		if(!connection->autoMatchBrowsing)
		{
			if(!piSBStartListingAutoMatches(peer))
			{
				piSetAutoMatchStatus(peer, connection->autoMatchQRFailed?PEERFailed:PEERWaiting);
				return;
			}
		}
		break;

	case PEERWaiting:
		// start
		GS_ASSERT(connection->inRoom[StagingRoom]);
		if(!connection->autoMatchBrowsing && !connection->autoMatchSBFailed)			
			piSBStartListingAutoMatches(peer);

		if(!connection->autoMatchReporting)
		{
			if(!piStartAutoMatchReporting(peer))
			{
				piSetAutoMatchStatus(peer, connection->autoMatchSBFailed?PEERFailed:PEERSearching);
				return;
			}
		}
		break;

	case PEERStaging:
		// stop
		if(!connection->hosting)
		{
			piStopAutoMatchReporting(peer);
		}
		piSBStopListingAutoMatches(peer);

		// start
		if(connection->hosting && !connection->autoMatchReporting)
		{
			if(!piStartAutoMatchReporting(peer))
			{	
				// revert back to searching *unless* we are waiting in a staging room to find out if
				// there is a host
				if (!connection->waitingForHostFlag) 
				{
					piSetAutoMatchStatus(peer, PEERSearching);
				}
				return;
			}
		}
		break;

	case PEERReady:
		// start
		if (connection->hosting && !connection->autoMatchReporting)
			piStartAutoMatchReporting(peer);
		
		break;
	case PEERComplete:
		// stop
		piSBStopListingAutoMatches(peer);
		piStopAutoMatchReporting(peer);

		// clean
		piCleanAutoMatch(peer);
		break;
	}
}

void piStopAutoMatch(PEER peer)
{
	//PEERBool inRoom;

	PEER_CONNECTION;

	// make sure we're AutoMatching
	if(peerIsAutoMatching(peer))
	{
		// we don't want the status callback called
		if(connection->autoMatchOperation)
			connection->autoMatchOperation->callback = (PEERCBType)NULL;

		// trick it into thinking it's not in a staging room (if it is)
		//inRoom = connection->inRoom[StagingRoom];
		//connection->inRoom[StagingRoom] = PEERFalse;
		
		// cleanup
		piSetAutoMatchStatus(peer, PEERFailed);
		connection->autoMatchNextStatus = PEERFailed;
		connection->autoMatchDelay = 0;
		connection->peerReadyTime = 0;
		connection->amStartTime = 0;
		connection->joinRoomTime = 0;
		connection->hostInRoom = 0;
		connection->waitingForHostFlag = PEERFalse;
		connection->amCustomSocket = PEERFalse;
		// reset the inRoom flag
		//connection->inRoom[StagingRoom] = inRoom;
	}
}

static void piJoinAutoMatchRoomCallback
(
	PEER peer,
	PEERBool success,
	PEERJoinResult result,
	RoomType roomType,
	void * param
)
{
	PEER_CONNECTION;
	connection->waitingForHostFlag = PEERFalse; // only set this bool if we succeed in joining the room
	connection->joinRoomTime = 0; // only set the join time if we succeed

	// if we're the only one, or if there's no host, leave
	// *note that we only have the 'op' flags at this point, which does not ensure the player is the host
	if(success && ((connection->numPlayers[StagingRoom] <= 1) || !piCountRoomOps(peer, StagingRoom, connection->nick)))
	{
		piLeaveRoom(peer, StagingRoom, "");
		success = PEERFalse;
	}

	// check if it succeeded
	if(success)
	{
		// we're waiting for the room key callback so we don't know yet whether a host is in the room
		connection->waitingForHostFlag = PEERTrue; 
		connection->joinRoomTime = current_time();

		// we're now staging
		piSetAutoMatchStatus(peer, PEERStaging);
	}

	// if we were waiting, and this failed, restart waiting
	if(!success && (connection->autoMatchStatus == PEERWaiting)) {
		piSetAutoMatchStatus(peer, PEERWaiting);
	}
		
	GSI_UNUSED(result);
	GSI_UNUSED(roomType);
	GSI_UNUSED(param);
}

PEERBool piJoinAutoMatchRoom(PEER peer, SBServer server)
{
	unsigned int publicIP;
	unsigned int privateIP;
	unsigned short privatePort;
	char room[PI_ROOM_MAX_LEN];

	PEER_CONNECTION;

	// reset the hostInRoom identifier since we are joining a new room
	connection->hostInRoom = 0;

	// Get the public and private IPs and ports.
	publicIP = SBServerGetPublicInetAddress(server);
	privateIP = SBServerGetPrivateInetAddress(server);
	if(SBServerHasPrivateAddress(server))
		privatePort = SBServerGetPrivateQueryPort(server);
	else
		privatePort = SBServerGetPublicQueryPort(server);

	if(!publicIP)
		return PEERFalse;

	// get the staging room.
	piMangleStagingRoom(room, connection->title, publicIP, privateIP, privatePort);

	// start the operation.
	if(!piNewJoinRoomOperation(peer, StagingRoom, room, NULL, piJoinAutoMatchRoomCallback, NULL, piGetNextID(peer)))
		return PEERFalse;

	// clone the server
	connection->hostServer = piSBCloneServer(server);

	return PEERTrue;
}

static void piCreateAutoMatchRoomCallback
(
	PEER peer,
	PEERBool success,
	PEERJoinResult result,
	RoomType roomType,
	void * param
)
{
	PEERAutoMatchStatus status;

	PEER_CONNECTION;

	if(success)
	{
		CHATChannelMode modes;
		// set the status based on the number of people in the room
		modes.Limit = connection->maxPlayers;
		modes.OpsObeyChannelLimit = CHATTrue;
		modes.NoExternalMessages = CHATTrue;
		modes.OnlyOpsChangeTopic = CHATTrue;
		modes.Private = CHATTrue;

		chatSetChannelMode(connection->chat, peerGetRoomChannel(peer, StagingRoom), &modes);

		if(connection->numPlayers[StagingRoom] <= 1)
		{
			status = PEERWaiting;
		}
		else if(connection->numPlayers[StagingRoom] >= connection->maxPlayers)
		{
			status = PEERReady;
		}
		else
		{
			status = PEERStaging;
		}

		// set the Waiting status
		piSetAutoMatchStatus(peer, status);
	}
	else
	{
		// handle the failure
		connection->autoMatchQRFailed = PEERTrue;
		piSetAutoMatchStatus(peer, connection->autoMatchSBFailed?PEERFailed:PEERSearching);
	}
	
	GSI_UNUSED(result);
	GSI_UNUSED(roomType);
	GSI_UNUSED(param);
}

static PEERBool piCreateAutoMatchRoom(PEER peer)
{
	piOperation * operation;

	PEER_CONNECTION;

	// get the AutoMatch operation
	operation = connection->autoMatchOperation;

	// start the operations
	if(!piNewCreateStagingRoomOperation(peer, connection->nick, "", connection->maxPlayers, operation->socket, operation->port, piCreateAutoMatchRoomCallback, NULL, piGetNextID(peer)))
	{
		connection->autoMatchQRFailed = PEERTrue;
		return PEERFalse;
	}

	return PEERTrue;
}

void piAutoMatchCheckWaitingForHostFlag(PEER peer)
{
	PEER_CONNECTION;

	if (connection->waitingForHostFlag == PEERTrue) 
	{
		// if there is a host, then we can update our automatch status accordingly
		if (connection->hostInRoom == 1)
		{
			connection->waitingForHostFlag = PEERFalse;
			connection->joinRoomTime = 0; // reset join time
 
			// reset next status since we've joined a server successfully
			connection->autoMatchNextStatus = PEERFailed;
			connection->autoMatchDelay = 0;

			// if we've got maxplayers, we're now Ready
			if((connection->autoMatchStatus == PEERStaging) && (connection->numPlayers[StagingRoom] >= connection->maxPlayers)) {	
				piSetAutoMatchStatus(peer, PEERReady);
			}

			// reset the hostInRoom identifier
			connection->hostInRoom = 0;
		}
		// if there is no host after a short while then we leave the room and continue automatching
		else if (connection->joinRoomTime != 0 && current_time() - connection->joinRoomTime >= AMHOSTFLAGWAIT)
		{
			connection->waitingForHostFlag = PEERFalse;
			connection->joinRoomTime = 0; // reset join time

			piLeaveRoom(peer, StagingRoom, "");

			// if we were waiting or staging and this failed, restart waiting
			if(connection->autoMatchStatus == PEERWaiting || connection->autoMatchStatus == PEERStaging) {
				piSetAutoMatchStatus(peer, PEERWaiting);
			}
		}
		// else we're still waiting to receive the host flag
	}
}

void piAutoMatchRestartThink(PEER peer)
{
	gsi_time now;
	PEER_CONNECTION;

	// reset the restart timer since we are in a success state
	if (connection->autoMatchStatus == PEERReady || connection->autoMatchStatus == PEERComplete)
	{
		connection->amStartTime = current_time();
		return;
	}

	// if we've been automatching for AMRESTARTTIME without success, restart the automatch process
	now = current_time();
	if (now - connection->amStartTime >= AMRESTARTTIME && connection->peerReadyTime == 0)
	{
		SOCKET socketForRestart;
		unsigned short portForRestart;
		int maxPlayersForRestart;
		char * filterForRestart;

		// intptr_t are only defined on certain platforms (handles 64-bit OSes)
		amcallback_t  statusCBForRestart;
		amcallback_t rateCBForRestart;
		void * cbParamForRestart;

		// store all the 'startAutoMatch' parameters before we stopAutomatch and clear them
		maxPlayersForRestart = connection->maxPlayers;
		filterForRestart = goastrdup(connection->autoMatchFilter);
		statusCBForRestart = (amcallback_t)connection->autoMatchOperation->callback;
		rateCBForRestart = (amcallback_t)connection->autoMatchOperation->callback2;
		cbParamForRestart = connection->autoMatchOperation->callbackParam;

		// preserve the socket/port if the SDK does not own it (eg. developer used 'withSocket')
		if (connection->amCustomSocket == PEERTrue) 
		{
			socketForRestart = connection->autoMatchOperation->socket;
			portForRestart = connection->autoMatchOperation->port;
		}
		else
		{
			socketForRestart = INVALID_SOCKET;
			portForRestart = 0;
		}

		// stop automatch, then restart
		piStopAutoMatch(peer);
		peerStartAutoMatchWithSocket(peer, maxPlayersForRestart, filterForRestart, socketForRestart, portForRestart,
			(peerAutoMatchStatusCallback)statusCBForRestart, (peerAutoMatchRateCallback)rateCBForRestart, cbParamForRestart, PEERFalse);
	}
}

// check if we have remained in PEERReady status long enough to be considered stable and call the developer's callback
void piAutoMatchReadyTimeThink(PEER peer)
{	
	gsi_time now;

	PEER_CONNECTION;

	// if the status has reverted out of PEERReady, reset the 'ready time'
	if (connection->autoMatchStatus != PEERReady)
	{
		connection->peerReadyTime = 0;
	}
	else  
	{
		now = current_time();

		// check if enough time has passed, if so, trigger developer's status callback
		if((now - connection->peerReadyTime) >= PEERREADYWAIT)
		{
			piAddAutoMatchStatusCallback(peer);

			// reset the 'ready time' since we called the status callback
			connection->peerReadyTime = 0;
		}
	}
}

void piAutoMatchDelayThink(PEER peer)
{	
	PEER_CONNECTION;
	if (connection->autoMatchDelay > 0 && connection->autoMatchNextStatus != PEERFailed)
	{
		connection->autoMatchDelay--;
	}
	else if (connection->autoMatchDelay ==  0 && connection->autoMatchNextStatus != PEERFailed)
	{
		// check to make sure we are not currently in the process of joining a room/server
		if (!connection->rooms[StagingRoom][0]) {
			// delay is complete, set 'next status'
			piSetAutoMatchStatus(peer, connection->autoMatchNextStatus);
		}
		else 
		{
			if (connection->autoMatchStatus != PEERReady)
			{
				// don't set the next status yet, we are attempting to join a room
				connection->autoMatchDelay = 5;
			}
			else 
			{
				connection->autoMatchNextStatus = PEERFailed; // don't set next status since we are in ready state
			}

			return;
		}
		connection->autoMatchNextStatus = PEERFailed;		
	}
}

unsigned int piGetAutoMatchDelay()
{
	unsigned int randomeDelay = (unsigned int)(rand() % PEER_AM_INTERVALS * PEER_AM_INTERVAL_SIZE);
	return randomeDelay;
}