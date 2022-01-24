///////////////////////////////////////////////////////////////////////////////
// File:	gt2Connection.c
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "gt2Connection.h"
#include "gt2Socket.h"
#include "gt2Message.h"
#include "gt2Callback.h"
#include "gt2Utility.h"
#include <stdlib.h>

GT2Result gti2NewOutgoingConnection(GT2Socket socket, GT2Connection * connection, unsigned int ip, unsigned short port)
{
	GT2Result result;

	// Here we create the object.
	result = gti2NewSocketConnection(socket, connection, ip, port);
	if(result != GT2Success)
		return result;

	// Here we set initial states.
	(*connection)->state = GTI2AwaitingServerChallenge;
	(*connection)->initiated = GT2True;

	return GT2Success;
}

GT2Result gti2NewIncomingConnection(GT2Socket socket, GT2Connection * connection, unsigned int ip, unsigned short port)
{
	GT2Result result;

	// Here we create the object.
	result = gti2NewSocketConnection(socket, connection, ip, port);
	if(result != GT2Success)
		return result;

	// Here we set initial states.
	(*connection)->state = GTI2AwaitingClientChallenge;
	(*connection)->initiated = GT2False;

	return GT2Success;
}

GT2Result gti2StartConnectionAttempt
(
	GT2Connection connection,
	const GT2Byte * message,
	int len,
	GT2ConnectionCallbacks * callbacks
)
{
	char challenge[GTI2_CHALLENGE_LEN];

	// Check the message and len.
	gti2MessageCheck(&message, &len);

	// Copy off the message.
	if(len > 0)
	{
		connection->initialMessage = (char *)gsimalloc((unsigned int)len);
		if(!connection->initialMessage)
			return GT2OutOfMemory;

		memcpy(connection->initialMessage, message, (unsigned int)len);
		connection->initialMessageLen = len;
	}

	// Copy the callbacks.
	if(callbacks)
		connection->callbacks = *callbacks;

	// Generate a challenge.
	gti2GetChallenge((GT2Byte *)challenge);

	// Generate and store the expected response.
	gti2GetResponse((GT2Byte *)connection->response, (GT2Byte *)challenge);

	// Send the client challenge.
	gti2SendClientChallenge(connection, challenge);

	// Update our state.
	connection->state = GTI2AwaitingServerChallenge;

	return GT2Success;
}

GT2Bool gti2AcceptConnection(GT2Connection connection, GT2ConnectionCallbacks * callbacks)
{
	// Was the connection already closed?
	if(connection->freeAtAcceptReject)
	{
		// Clear the flag.
		connection->freeAtAcceptReject = GT2False;

		// Let the app know if the connection was already closed.
		return GT2False;
	}

	// Make sure this flag gets cleared.
	connection->freeAtAcceptReject = GT2False;

	// Check state to see if we're still awaiting this connection.
	if(connection->state != GTI2AwaitingAcceptReject)
		return GT2False;

	// Let the other side know that the local client has accepted the 
	// connection.
	gti2SendAccept(connection);

	// Update our state.
	connection->state = GTI2Connected;

	// Store the callbacks.
	if(callbacks)
		connection->callbacks = *callbacks;

	return GT2True;
}

void gti2RejectConnection(GT2Connection connection, const GT2Byte * message, int len)
{
	// Make sure this flag gets cleared.
	connection->freeAtAcceptReject = GT2False;

	// Check that we're still awaiting this connection.
	if(connection->state != GTI2AwaitingAcceptReject)
		return;

	// Check the message and len.
	gti2MessageCheck(&message, &len);

	// Let the remote client that the local client has accepted the connection.
	gti2SendReject(connection, message, len);

	// Update our state.
	connection->state = GTI2Closing;
}

GT2Bool gti2ConnectionSendData(GT2Connection connection, const GT2Byte * message, int len)
{
	// Send the data on the socket.
	if(!gti2SocketSend(connection->socket, connection->ip, connection->port, message, len))
		return GT2False;

	// Mark the time (used for keep-alives).
	connection->lastSend = current_time();

	return GT2True;
}

static GT2Bool gti2CheckTimeout(GT2Connection connection, gsi_time now)
{
	// Are we still trying to connect?
	if(connection->state < GTI2Connected)
	{
		GT2Bool timedOut = GT2False;

		// Is this connection the initiator?
		if(connection->initiated)
		{
			// Do we have a timeout?
			if(connection->timeout)
			{
				// Check the time taken against the timeout.
				if((now - connection->startTime) > connection->timeout)
					timedOut = GT2True;
			}
		}
		else
		{
			// Don't time them out if they're waiting for us.
			if(connection->state < GTI2AwaitingAcceptReject)
			{
				// Check the time taken against the timeout.
				if((now - connection->startTime) > GTI2_SERVER_TIMEOUT)
					timedOut = GT2True;
			}
		}

		// Check if we timed out.
		if(timedOut)
		{
			// Let them know.
			gti2SendClosed(connection);

			// Mark it as closed.
			gti2ConnectionClosed(connection);

			// Call the callback.
			if(!gti2ConnectedCallback(connection, GT2TimedOut, NULL, 0))
				return GT2False;
		}
	}

	return GT2True;
}

static GT2Bool gti2SendRetries(GT2Connection connection, gsi_time now)
{
	int i;
	int len;
	GTI2OutgoingBufferMessage * message;

	// Go through the list of outgoing messages awaiting confirmation.
	len = ArrayLength(connection->outgoingBufferMessages);
	for(i = 0 ; i < len ; i++)
	{
		// Get the message.
		message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, i);

		// Check if it's time to resend it.
		if((now - message->lastSend) > GTI2_RESEND_TIME)
		{
			if(!gti2ResendMessage(connection, message))
				return GT2False;
		}
	}

	return GT2True;
}

static GT2Bool gti2CheckPendingAck(GT2Connection connection, gsi_time now)
{
	// Check that nothing is pending.
	if(!connection->pendingAck)
		return GT2True;

	// Check how long it has been pending.
	if((now - connection->pendingAckTime) > GTI2_PENDING_ACK_TIME)
	{
		if(!gti2SendAck(connection))
			return GT2False;
	}

	return GT2True;
}

static GT2Bool gti2CheckKeepAlive(GT2Connection connection, gsi_time now)
{
	if((now - connection->lastSend) > GTI2_KEEP_ALIVE_TIME)
	{
		if(!gti2SendKeepAlive(connection))
			return GT2False;
	}

	return GT2True;
}

GT2Bool gti2ConnectionThink(GT2Connection connection, gsi_time now)
{
	// Check timeout.
	if(!gti2CheckTimeout(connection, now))
		return GT2False;

	// Check keep alives.
	if(!gti2CheckKeepAlive(connection, now))
		return GT2False;

	// Send retries.
	if(!gti2SendRetries(connection, now))
		return GT2False;

	// Check the pending ack.
	if(!gti2CheckPendingAck(connection, now))
		return GT2False;

	return GT2True;
}

void gti2CloseConnection(GT2Connection connection, GT2Bool hard)
{
	// Check to see whether the connection should be hard or soft closed.
	if(hard)
	{
		// Check to see if it's already closed.
		if(connection->state >= GTI2Closed)
			return;

		// Mark it as closed.
		gti2ConnectionClosed(connection);

		// Send a closed message.
		gti2SendClosed(connection);

		// Call the callback.
		gti2ClosedCallback(connection, GT2LocalClose);

		// Try to free it.
		gti2FreeSocketConnection(connection);
	}
	else
	{
		// Mark it as closing.
		connection->state = GTI2Closing;

		// Send the close.
		gti2SendClose(connection);
	}
}

void gti2ConnectionClosed(GT2Connection connection)
{
	// Check to see if the connection is already closed.
	if(connection->state == GTI2Closed)
		return;

	// Mark the connection as closed.
	connection->state = GTI2Closed;

	// Remove it from the connected list.
	TableRemove(connection->socket->connections, &connection);

	// Add it to the closed list.
	ArrayAppend(connection->socket->closedConnections, &connection);
}

void gti2ConnectionCleanup(GT2Connection connection)
{
	if(connection->initialMessage)
		gsifree(connection->initialMessage);

	if(connection->incomingBuffer.buffer)
		gsifree(connection->incomingBuffer.buffer);
	if(connection->outgoingBuffer.buffer)
		gsifree(connection->outgoingBuffer.buffer);

	if(connection->incomingBufferMessages)
		ArrayFree(connection->incomingBufferMessages);
	if(connection->outgoingBufferMessages)
		ArrayFree(connection->outgoingBufferMessages);
	
	if(connection->sendFilters)
		ArrayFree(connection->sendFilters);
	if(connection->receiveFilters)
		ArrayFree(connection->receiveFilters);

	gsifree(connection);
}
