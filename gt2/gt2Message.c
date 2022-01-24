///////////////////////////////////////////////////////////////////////////////
// File:	gt2Message.c
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "gt2Message.h"
#include "gt2Buffer.h"
#include "gt2Connection.h"
#include "gt2Socket.h"
#include "gt2Callback.h"
#include "gt2Utility.h"
#include <stdlib.h>

static unsigned short gti2UShortFromBuffer(const GT2Byte * buffer, int pos)
{
	unsigned short s;
	s = (unsigned short)((buffer[pos] << 8) & 0xFF00);
	pos++;
	s |= buffer[pos];

	return s;
}


static void gti2UShortToBuffer(GT2Byte * buffer, int pos, unsigned short s)
{
	buffer[pos++] = (GT2Byte)((s >> 8) & 0xFF);
	buffer[pos] = (GT2Byte)(s & 0xFF);
}

static int gti2SNDiff(unsigned short SN1, unsigned short SN2)
{
	return (short)(SN1 - SN2);
}

static GT2Bool gti2ConnectionError(GT2Connection connection, GT2Result result, GT2CloseReason reason)
{
	// First check if we're still connecting.
	if(connection->state < GTI2Connected)
	{
		// Check if the local side is the initiator.
		if(connection->initiated)
		{
			// Mark it as closed.
			gti2ConnectionClosed(connection);

			// Call the callback.
			if(!gti2ConnectedCallback(connection, result, NULL, 0))
				return GT2False;
		}
		else
		{
			// Are we waiting for accept/reject?
			if(connection->state == GTI2AwaitingAcceptReject)
				connection->freeAtAcceptReject = GT2True;

			// Mark the connection as closed.
			gti2ConnectionClosed(connection);
		}
	}
	// Report the close, as long as we're not already closed.
	else if(connection->state != GTI2Closed)
	{
		// Mark the connection as closed.
		gti2ConnectionClosed(connection);

		// Call the callback.
		if(!gti2ClosedCallback(connection, reason))
			return GT2False;
	}

	return GT2True;
}

static GT2Bool gti2ConnectionCommunicationError(GT2Connection connection)
{
	return gti2ConnectionError(connection, GT2NegotiationError, GT2CommunicationError);
}

static GT2Bool gti2ConnectionMemoryError(GT2Connection connection)
{
	// Let the remote client that the local client has accepted the connection.
	if(!gti2SendClosed(connection))
		return GT2False;

	return gti2ConnectionError(connection, GT2OutOfMemory, GT2NotEnoughMemory);
}





static GT2Bool gti2HandleESN(GT2Connection connection, unsigned short ESN)
{
	int len;
	int i;
	GTI2OutgoingBufferMessage * message;
	int shortenBy;

	// Get the number of messages in the outgoing queue.
	len = ArrayLength(connection->outgoingBufferMessages);
	if(!len)
		return GT2True;

	// Loop through until we hit a message we can't remove.
	for(i = 0 ; i < len ; i++)
	{
		// Get the message.
		message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, i);

		// Don't stop until we get to the ESN.
		if(gti2SNDiff(message->serialNumber, ESN) >= 0)
			break;
	}

	// Check for not removing any.
	if(i == 0)
		return GT2True;

	// Remove the message info structs.
	while(i--)
		ArrayDeleteAt(connection->outgoingBufferMessages, i);

	// Check yo see how many messages are left.
	len = ArrayLength(connection->outgoingBufferMessages);
	if(!len)
	{
		// Buffer is empty.
		connection->outgoingBuffer.len = 0;
		return GT2True;
	}

	// Figure out how much to move everything forward.
	message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, 0);
	shortenBy = message->start;

	// Do the move on the info structs.
	for(i = 0 ; i < len ; i++)
	{
		message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, i);
		message->start -= shortenBy;
	}

	// Move the actual data.
	gti2BufferShorten(&connection->outgoingBuffer, 0, shortenBy);

	return GT2True;
}

static GT2Bool gti2HandleAppUnreliable(GT2Connection connection, GT2Byte * message, int len)
{
	// Check the state.
	if((connection->state != GTI2Connected) && (connection->state != GTI2Closing))
		return GT2True;

	// Do we need to filter it?
	if(ArrayLength(connection->receiveFilters))
	{
		if(!gti2ReceiveFilterCallback(connection, 0, message, len, GT2False))
			return GT2False;
		return GT2True;
	}

	// We received data, call the callback.
	if(!gti2ReceivedCallback(connection, message, len, GT2False))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleAppReliable(GT2Connection connection, GT2Byte * message, int len)
{
	// Check the state.
	if((connection->state != GTI2Connected) && (connection->state != GTI2Closing))
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;
	}
	else
	{
		// Do we need to filter it?
		if(ArrayLength(connection->receiveFilters))
		{
			if(!gti2ReceiveFilterCallback(connection, 0, message, len, GT2True))
				return GT2False;
			return GT2True;
		}

		// We received data, call the callback.
		if(!gti2ReceivedCallback(connection, message, len, GT2True))
			return GT2False;
	}

	return GT2True;
}

static GT2Bool gti2HandleClientChallenge(GT2Connection connection, GT2Byte * message, int len)
{
	char response[GTI2_RESPONSE_LEN];
	char challenge[GTI2_CHALLENGE_LEN];

	// Check the state.
	if(connection->state != GTI2AwaitingClientChallenge)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Make sure the message is long enough.
	if(len < GTI2_CHALLENGE_LEN)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Generate a response to the challenge.
	gti2GetResponse((GT2Byte *)response, message);

	// Generate our own challenge.
	gti2GetChallenge((GT2Byte *)challenge);

	// Store what our response will be.
	gti2GetResponse((GT2Byte *)connection->response, (GT2Byte *)challenge);

	// Send our own challenge.
	if(!gti2SendServerChallenge(connection, response, challenge))
		return GT2False;

	// New state.
	connection->state = GTI2AwaitingClientResponse;

	return GT2True;
}



static GT2Bool gti2HandleServerChallenge(GT2Connection connection, GT2Byte * message, int len)
{
	char response[GTI2_RESPONSE_LEN];

	// Check the state.
	if(connection->state != GTI2AwaitingServerChallenge)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Make sure the message is long enough.
	if(len < (GTI2_RESPONSE_LEN + GTI2_CHALLENGE_LEN))
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Check the response.
	if(!gti2CheckResponse(message, (GT2Byte *)connection->response))
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Generate our response to the server's challenge.
	gti2GetResponse((GT2Byte *)response, message + GTI2_RESPONSE_LEN);

	// Send the response, including our intial message.
	if(!gti2SendClientResponse(connection, response, connection->initialMessage, connection->initialMessageLen))
		return GT2False;

	// Free the initial message.
	if(connection->initialMessage)
	{
		gsifree(connection->initialMessage);
		connection->initialMessage = NULL;
	}

	// New state.
	connection->state = GTI2AwaitingAcceptance;

	return GT2True;
}

static GT2Bool gti2HandleClientResponse(GT2Connection connection, GT2Byte * message, int len)
{
	int latency;

	// Check the state.
	if(connection->state != GTI2AwaitingClientResponse)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Make sure the message is long enough.
	if(len < (GTI2_RESPONSE_LEN))
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Check the response.
	if(!gti2CheckResponse(message, (GT2Byte *)connection->response))
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// We need to make sure the connection didn't just stop listening.
	if(!connection->socket->connectAttemptCallback)
	{
		// Send them a closed.
		if(!gti2SendClosed(connection))
			return GT2False;

		// Mark the connection as closed.
		gti2ConnectionClosed(connection);

		return GT2True;
	}

	// New state.
	connection->state = GTI2AwaitingAcceptReject;

	// Calculate the approximate latency.
	latency = (int)(current_time() - connection->challengeTime);

	// The app should now finish the rest.
	if(!gti2ConnectAttemptCallback(connection->socket, connection, connection->ip, connection->port, latency, message + GTI2_RESPONSE_LEN, len - GTI2_RESPONSE_LEN))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleAccept(GT2Connection connection)
{
	// Check the state.
	if(connection->state != GTI2AwaitingAcceptance)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// New state.
	connection->state = GTI2Connected;

	// Call the callback.
	if(!gti2ConnectedCallback(connection, GT2Success, NULL, 0))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleReject(GT2Connection connection, GT2Byte * message, int len)
{
	// Check the state.
	if(connection->state != GTI2AwaitingAcceptance)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Mark the connection as closed.
	gti2ConnectionClosed(connection);

	// Send a closed reply.
	if(!gti2SendClosed(connection))
		return GT2False;

	// Call the callback.
	if(!gti2ConnectedCallback(connection, GT2Rejected, message, len))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleClose(GT2Connection connection)
{
	GT2Bool localClose;

	// Send a closed reply.
	if(!gti2SendClosed(connection))
		return GT2False;

	// Were we attempting to close this connection?
	localClose = (connection->state == GTI2Closing);

	// Handle it as an error (so the right callbacks are called).
	if(!gti2ConnectionError(connection, GT2Rejected, localClose?GT2LocalClose:GT2RemoteClose))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2DeliverReliableMessage(GT2Connection connection, GTI2MessageType type, GT2Byte * message, int len)
{
	// Bump our ESN.
	connection->expectedSerialNumber++;

	if(type == GTI2MsgAppReliable)
	{
		if(!gti2HandleAppReliable(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgClientChallenge)
	{
		if(!gti2HandleClientChallenge(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgServerChallenge)
	{
		if(!gti2HandleServerChallenge(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgClientResponse)
	{
		if(!gti2HandleClientResponse(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgAccept)
	{
		if(!gti2HandleAccept(connection))
			return GT2False;
	}
	else if(type == GTI2MsgReject)
	{
		if(!gti2HandleReject(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgClose)
	{
		if(!gti2HandleClose(connection))
			return GT2False;
	}
	else if(type == GTI2MsgKeepAlive)
	{
		// Ignore.
	}

	return GT2True;
}
#ifdef WIN32
static int __cdecl gti2IncomingBufferMessageCompare(const void * elem1, const void * elem2)
#else
static int gti2IncomingBufferMessageCompare(const void * elem1, const void * elem2)
#endif
{
	const GTI2IncomingBufferMessage * message1 = (GTI2IncomingBufferMessage *)elem1;
	const GTI2IncomingBufferMessage * message2 = (GTI2IncomingBufferMessage *)elem2;

	return gti2SNDiff(message1->serialNumber, message2->serialNumber);
}

static GT2Bool gti2BufferIncomingMessage(GT2Connection connection, GTI2MessageType type, unsigned short SN, GT2Byte * message, int len, GT2Bool * overflow)
{
	GTI2IncomingBufferMessage messageInfo;
	GTI2IncomingBufferMessage * bufferedMessage;
	int num;
	int i;

	// Check the number of messages being held.
	num = ArrayLength(connection->incomingBufferMessages);

	// Check if this message is already buffered.
	for(i = 0 ; i < num ; i++)
	{
		// Get the message.
		bufferedMessage = (GTI2IncomingBufferMessage *)ArrayNth(connection->incomingBufferMessages, i);

		// Check if this is the same message.
		if(bufferedMessage->serialNumber == SN)
		{
			*overflow = GT2False;
			return GT2True;
		}

		// Check if we've already past the target SN.
		if(gti2SNDiff(bufferedMessage->serialNumber, SN) > 0)
			break;
	}

	// Check that there's enough space to store the message.
	if(gti2GetBufferFreeSpace(&connection->incomingBuffer) < len)
	{
		*overflow = GT2True;
		return GT2True;
	}

	// Setup the message info.
	messageInfo.start = connection->incomingBuffer.len;
	messageInfo.len = len;
	messageInfo.type = type;
	messageInfo.serialNumber = SN;

	// Add it to the list.
	ArrayInsertSorted(connection->incomingBufferMessages, &messageInfo, gti2IncomingBufferMessageCompare);

	// Make sure the length is one more.
	if(ArrayLength(connection->incomingBufferMessages) != (num + 1))
	{
		*overflow = GT2True;
		return GT2True;
	}

	// Copy the message into the buffer.
	gti2BufferWriteData(&connection->incomingBuffer, message, len);

	// Check for sending a nack.
	// We want to send one when we think a message or messages were probably dropped.
	if(num == 0)
	{
		// If we're the only message in the hold, send a nack.
		if(!gti2SendNack(connection, connection->expectedSerialNumber, (unsigned short)(SN - 1)))
			return GT2False;
	}
	else
	{
		GTI2IncomingBufferMessage * msg;

		// Are we the highest message?
		msg = (GTI2IncomingBufferMessage *)ArrayNth(connection->incomingBufferMessages, num);
		if(msg->serialNumber == SN)
		{
			GTI2IncomingBufferMessage * prev;
			unsigned short diff;

			// If we're not right after the second-highest SN, the ones in between were probably dropped.
			prev = (GTI2IncomingBufferMessage *)ArrayNth(connection->incomingBufferMessages, num - 1);
			diff = (unsigned short)gti2SNDiff(SN, prev->serialNumber);
			if(diff > 1)
			{
				if(!gti2SendNack(connection, (unsigned short)(prev->serialNumber + 1), (unsigned short)(SN - 1)))
					return GT2False;
			}
		}
	}

	*overflow = GT2False;
	return GT2True;
}

static void gti2RemoveHoldMessage(GT2Connection connection, GTI2IncomingBufferMessage * message, int index)
{
	int moveAfter;
	int shortenBy;
	int moveEnd = 0;
	int num;
	int i;

	// Save off info about the message.
	moveAfter = message->start;
	shortenBy = message->len;

	// Delete the message.
	ArrayDeleteAt(connection->incomingBufferMessages, index);

	// Loop through and fix up messages stored after this one in the buffer.
	// Also figure out exactly how much data we'll need to move.
	num = ArrayLength(connection->incomingBufferMessages);
	for(i = 0 ; i < num ; i++)
	{
		// Check if this message needs to be moved forward.
		message = (GTI2IncomingBufferMessage *)ArrayNth(connection->incomingBufferMessages, i);
		if(message->start > moveAfter)
		{
			message->start -= shortenBy;
			moveEnd = GS_MAX(moveEnd, message->start + message->len);
		}
	}

	// Fix up the buffer itself.
	gti2BufferShorten(&connection->incomingBuffer, moveAfter, shortenBy);
}

static GT2Bool gti2DeliverHoldMessages(GT2Connection connection)
{
	GTI2IncomingBufferMessage * message;
	int num;
	int i;

	// Loop through the buffered messages, checking if there are any that can now be delivered.
	// Loop through backwards to ease removal.
	num = ArrayLength(connection->incomingBufferMessages);
	for(i = (num - 1) ; i >= 0 ; i--)
	{
		message = (GTI2IncomingBufferMessage *)ArrayNth(connection->incomingBufferMessages, i);

		// We should deliver this if it's what we're expecting.
		if(message->serialNumber == connection->expectedSerialNumber)
		{
			// Deliver the message.
			if(!gti2DeliverReliableMessage(connection, message->type, connection->incomingBuffer.buffer + message->start, message->len))
				return GT2False;

			// Remove the message.
			gti2RemoveHoldMessage(connection, message, i);

			// We need to go through this loop again.
			num = ArrayLength(connection->incomingBufferMessages);
		}
	}

	return GT2True;
}

static void gti2SetPendingAck(GT2Connection connection)
{
	// If there's not a pending ack, set one.
	if(!connection->pendingAck)
	{
		connection->pendingAck = GT2True;
		connection->pendingAckTime = current_time();
	}
}

static GT2Bool gti2HandleReliableMessage(GT2Connection connection, GTI2MessageType type, GT2Byte * message, int len)
{
	unsigned short SN;
	unsigned short ESN;
	const int headerLength = connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 1 + 2 + 2;
	GT2Bool overflow;

	// Check to see if th message is long enough.
	if(len < (connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 1 + 2 + 2))  // Magic string + type + SN + ESN.
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Get the SN
	SN = gti2UShortFromBuffer(message + connection->socket->protocolOffset, GTI2_MAGIC_STRING_LEN + 1);

	// Get the ESN
	ESN = gti2UShortFromBuffer(message + connection->socket->protocolOffset, GTI2_MAGIC_STRING_LEN + 3);

	// Update the message and length to point to the actual message
	if (connection->socket->protocolType == GTI2VdpProtocol && type == GTI2MsgAppReliable)
	{
		message[connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 3] = message[0];
		message[connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 4] = message[1];
		message += headerLength - connection->socket->protocolOffset;
		len -= headerLength - connection->socket->protocolOffset;
	}
	else
	{
		message += headerLength;
		len -= headerLength;
	}
	
	// Handle the ESN.
	if(!gti2HandleESN(connection, ESN))
		return GT2False;

	// Check if it's the SN we expected.
	if(SN == connection->expectedSerialNumber)
	{
		// Make sure we ack this message.
		// Do this before delivering, because we might send an ack as part of a
		// reliable reply.
		gti2SetPendingAck(connection);

		// Deliver the message.
		if(!gti2DeliverReliableMessage(connection, type, message, len))
			return GT2False;

		// Check to see if there are any messages in the hold that can now be 
		// delivered.
		if(!gti2DeliverHoldMessages(connection))
			return GT2False;

		return GT2True;
	}

	// Check to see if the message is a duplicate.
	if(gti2SNDiff(SN, connection->expectedSerialNumber) < 0)
	{
		// The message is a duplicate, ack it again.
		gti2SetPendingAck(connection);

		// Ignore the message.
		return GT2True;
	}

	// We can't deliver the message yet, so put it in the hold.
	if(!gti2BufferIncomingMessage(connection, type, SN, message, len, &overflow))
		return GT2False;

	// Check for a buffer overflow.
	if(overflow)
	{
		if(!gti2ConnectionMemoryError(connection))
			return GT2False;
	}

	return GT2True;
}

static GT2Bool gti2HandleAck(GT2Connection connection, const GT2Byte * message, int len)
{
	unsigned short ESN;

	// Make sure there is enough space for the ESN.
	if(len != 2)
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Get the ESN.
	ESN = gti2UShortFromBuffer(message, 0);

	// Handle the message.
	if(!gti2HandleESN(connection, ESN))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleNack(GT2Connection connection, const GT2Byte * message, int len)
{
	unsigned short SNMin;
	unsigned short SNMax;
	int num;
	int i;
	GTI2OutgoingBufferMessage * messageInfo;

	// Read based on length.
	SNMin = gti2UShortFromBuffer(message, 0);
	if(len == 2)
	{
		SNMax = SNMin;
	}
	else if(len == 4)
	{
		SNMax = gti2UShortFromBuffer(message, 2);
	}
	else
	{
		if(!gti2ConnectionCommunicationError(connection))
			return GT2False;

		return GT2True;
	}

	// Loop through the messages, resending any specified ones.
	num = ArrayLength(connection->outgoingBufferMessages);
	for(i = 0 ; i < num ; i++)
	{
		messageInfo = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, i);
		if((gti2SNDiff(messageInfo->serialNumber, SNMin) >= 0) && (gti2SNDiff(messageInfo->serialNumber, SNMax) <= 0))
		{
			if(!gti2ResendMessage(connection, messageInfo))
				return GT2False;
		}
	}

	return GT2True;
}

static GT2Bool gti2HandlePing(GT2Connection connection, GT2Byte * message, int len)
{
	// Send the message right back.
	return gti2SendPong(connection, message, len);
}

static GT2Bool gti2HandlePong(GT2Connection connection, const GT2Byte * message, int len)
{
	gsi_time startTime;

	// Do we care about this ping?
	if(!connection->callbacks.ping)
		return GT2True;

	// Is this a pong we're interested in?
	// "time" + ping-sent-time
	if(len != (4 + sizeof(gsi_time)))
		return GT2True;
	if(memcmp(message, "time", 4) != 0)
		return GT2True;

	// Get the start time.
	memcpy(&startTime, message + 4, sizeof(gsi_time));

	// Call the callback.
	if(!gti2PingCallback(connection, (int)(current_time() - startTime)))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleClosed(GT2Connection connection)
{
	GT2Bool localClose;

	// Are we already closed?
	if(connection->state == GTI2Closed)
		return GT2True;

	// Were we attempting to close this connection?
	localClose = (connection->state == GTI2Closing);

	// Handle it as an error (so the right callbacks are called).
	if(!gti2ConnectionError(connection, GT2Rejected, localClose?GT2LocalClose:GT2RemoteClose))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2HandleUnreliableMessage(GT2Connection connection, GTI2MessageType type, GT2Byte * message, int len)
{
	int headerLength;
	GT2Byte * dataStart;
	int dataLen;

	// Most unreliable messages don't need the header.
	headerLength = (connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 1);
	dataStart = (message + headerLength);
	dataLen = (len - headerLength);
	
	// Handle unreliable messages based on type.
	if(type == GTI2MsgAck)
	{
		if(!gti2HandleAck(connection, dataStart, dataLen))
			return GT2False;
	}
	else if(type == GTI2MsgNack)
	{
		if(!gti2HandleNack(connection, dataStart, dataLen))
			return GT2False;
	}
	else if(type == GTI2MsgPing)
	{
		if(!gti2HandlePing(connection, message, len))
			return GT2False;
	}
	else if(type == GTI2MsgPong)
	{
		if(!gti2HandlePong(connection, dataStart, dataLen))
			return GT2False;
	}
	else if(type == GTI2MsgClosed)
	{
		if(!gti2HandleClosed(connection))
			return GT2False;
	}

	return GT2True;
}

// VDP sockets have data length which needs to be stripped off.
static GT2Bool gti2HandleMessage(GT2Socket socket, GT2Byte * message, int len, unsigned int ip, unsigned short port)
{
	GT2Connection connection;
	GT2Bool magicString;
	GT2Result result;
	GTI2MessageType type;
	GT2Bool handled;
	int actualLength = len - socket->protocolOffset;

	// VDP messages have 2 byte header which is removed based on protocol.
	GT2Byte *actualMessage = message + socket->protocolOffset;
	
	// Find out if we have an existing connection for this address.
	connection = gti2SocketFindConnection(socket, ip, port);

	// Let the dump handle this.
	if(socket->receiveDumpCallback)
	{
		if(!gti2DumpCallback(socket, connection, ip, port, GT2False, message, len, GT2False))
			return GT2False;
	}
	
	// Check if the message starts with the magic string.
	// Use greater than for the len compare because it also must have a type.
	magicString = ((actualLength > GTI2_MAGIC_STRING_LEN) && (memcmp(actualMessage, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN) == 0));
	
	// Check if we don't have a connection
	if(!connection)
	{
		// If we don't know who this is from, let the unrecognized message callback have first crack at it.
		if(!gti2UnrecognizedMessageCallback(socket, ip, port, message, len, &handled))
			return GT2False;

		// If they handled it, we don't care about it.
		if(handled)
			return GT2True;

		// If this isn't a connection request, tell them the connection is closed.
		if(!magicString || (actualMessage[GTI2_MAGIC_STRING_LEN] != GTI2MsgClientChallenge))
		{
			// If this is a closed message, don't send one back (to avoid recursion).
			if(!magicString || (actualMessage[GTI2_MAGIC_STRING_LEN] != GTI2MsgClosed))
			{
				if(!gti2SendClosedOnSocket(socket, ip, port))
					return GT2False;
			}
			return GT2True;
		}	
		
		// If we're not listening, we just ignore this.
		if(!socket->connectAttemptCallback)
			return GT2True;

		// Create a connection.
		result = gti2NewIncomingConnection(socket, &connection, ip, port);
		if(result != GT2Success)
		{
			// As long as this wasn't a duplicate address error, tell them we're closed.
			// In the case of duplicates, we don't want to close the existing connection.
			if(result != GT2DuplicateAddress)
			{
				if(!gti2SendClosedOnSocket(socket, ip, port))
					return GT2False;
			}
			return GT2True;
		}
	}

	// Is the connection already closed?
	if(connection->state == GTI2Closed)
	{
		// If this is a closed message, don't send one back (to avoid recursion).
		if(!magicString || (actualMessage[GTI2_MAGIC_STRING_LEN] != GTI2MsgClosed))
		{
			if(!gti2SendClosed(connection))
				return GT2False;
		}

		return GT2True;
	}
	
	// Check if this is an unreliable app message with a magic string header.
	if(magicString && ((actualLength >= (GTI2_MAGIC_STRING_LEN * 2)) && (memcmp(actualMessage + GTI2_MAGIC_STRING_LEN, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN) == 0)))
	{
		message[3] = message[1];
		message[2] = message[0];
		message += GTI2_MAGIC_STRING_LEN;
		actualMessage += GTI2_MAGIC_STRING_LEN;
		actualLength -= GTI2_MAGIC_STRING_LEN;
		len -= GTI2_MAGIC_STRING_LEN;
		magicString = GT2False;
	}
	
	// If it doesn't have a magic string, it's an unreliable app message.
	if(!magicString)
	{
		// First determine if the connection found has gone throught the 
		// internal challenge response.
		if (connection->state < GTI2Connected)
		{
			// Pass any message that doesn't have a magic string to the app so 
			// that the SDK doesn't drop them.
			if(!gti2UnrecognizedMessageCallback(socket, ip, port, message, len, &handled))
				return GT2False;
		}
		else
		{
			if(!gti2HandleAppUnreliable(connection, message, len))
				return GT2False;
		}

		return GT2True;
	}

	// Get the message type.
	type = (GTI2MessageType)actualMessage[GTI2_MAGIC_STRING_LEN];

	// Check to see if the message is reliable.
	if(type < GTI2NumReliableMessages)
	{
		// Handle the message.
		if(!gti2HandleReliableMessage(connection, type, message, len))
			return GT2False;
		return GT2True;
	}

	// Handle unreliable messages.
	if(!gti2HandleUnreliableMessage(connection, type, message, len))
		return GT2False;

	return GT2True;
}


GT2Bool gti2HandleConnectionReset(GT2Socket socket, unsigned int ip, unsigned short port)
{
	GT2Connection connection;

	// Find the connection for the reset.
	connection = gti2SocketFindConnection(socket, ip, port);

	// Let the dump know about this.
	if(socket->receiveDumpCallback)
	{
		if(!gti2DumpCallback(socket, connection, ip, port, GT2True, NULL, 0, GT2False))
			return GT2False;
	}

	// There's no connection, so ignore it.
	if(!connection)
		return GT2True;

	// Are we waiting for a response from the server?
	if(connection->state == GTI2AwaitingServerChallenge)
	{
		// Are we still within the timeout time?
		if(!connection->timeout || ((current_time() - connection->startTime) < connection->timeout))
			return GT2True;

		// Report this as a timeout.
		if(!gti2ConnectionError(connection, GT2TimedOut, GT2RemoteClose))
			return GT2False;
	}
	else
	{
		// Report the error.
		if(!gti2ConnectionError(connection, GT2Rejected, GT2RemoteClose))
			return GT2False;
	}

	return GT2True;
}

GT2Bool gti2HandleHostUnreachable(GT2Socket socket, unsigned int ip, unsigned short port, GT2Bool send)
{
	GT2Connection connection;

	// Find the connection for the reset.
	connection = gti2SocketFindConnection(socket, ip, port);

	// Let the dump know about this.
	if(socket->receiveDumpCallback)
	{
		if(!gti2DumpCallback(socket, connection, ip, port, GT2True, NULL, 0, send))
			return GT2False;
	}

	// There's no connection, so ignore it.
	if(!connection)
		return GT2True;


	// Report the error.
	if(!gti2ConnectionError(connection, GT2TimedOut, GT2RemoteClose))
		return GT2False;

	return GT2True;
}


#ifdef GSI_ADHOC



// Return length if successful.
// <=0 on error.
gsi_bool _NetworkAdHocSocketRecv(int socket_id,
							char	*buf,
							int		bufferlen,
							int		flags,
							char	*saddr,		// struct SceNetEtherAddr  = char[6];
							gsi_u16 *sport);








// Return 0 if no data, -1 if error,  >0 if data to read.
int _NetworkAdHocCanReceiveOnSocket(int socket_id);

GT2Bool gti2ReceiveAdHocMessages(GT2Socket socket,char *buffer, int buffersize)
{
	int rcode;
	SOCKADDR_IN address;
	int addressLen;//, datasize;

	// Check for messages.
	while	(1)
	{
		int datasize =  _NetworkAdHocCanReceiveOnSocket(socket->socket);
		if (datasize < 0)	// There was an error.
		{
			gti2SocketError(socket);
			return GT2False;
		}

		if (datasize == 0)
			break;		// There is no data.
		{
			// We have data to receive.
			// Receive the message.
			char		mac[6];
			gsi_u16		port;
			//gsi_u32		ip;

			addressLen = sizeof(address);

			rcode = _NetworkAdHocSocketRecv(socket->socket, buffer,buffersize , 0, mac,&port);
			if(rcode < 0)	// Fatal socket error.
			{
				#if(0)	// Notes
					if(0)//rcode == WSAECONNRESET)
					{
							// Handle the reset.
							if(!gti2HandleConnectionReset(socket, address.sin_addr.s_addr, ntohs(address.sin_port)))
								return GT2False;
					}
					else 
					if (rcode == WSAEHOSTUNREACH)
					{
						if (!gti2HandleHostUnreachable(socket, address.sin_addr.s_addr, ntohs(address.sin_port), GT2False))
							return GT2False;
					}			
					else
				#endif
					{
						gti2SocketError(socket);
						return GT2False;
					}
			}
			if(rcode == 0)	// There is no data.
			{
				return GT2False;
			}

			// At this point we have valid data.

			// Change ethernet to IP address.
			address.sin_addr.s_addr = gt2MacToIp(mac);
			address.sin_port = port;

			#ifdef RECV_LOG
				// Log the message.
				gti2LogMessage(address.sin_addr.s_addr, ntohs(address.sin_port),
					socket->ip, socket->port,
					buffer, rcode);
			#endif
			// Handle the message.
			if(!gti2HandleMessage(socket, (GT2Byte *)buffer, rcode, address.sin_addr.s_addr, address.sin_port))
				return GT2False;
		}
	}

	return GT2True;
}
#endif

GT2Bool gti2ReceiveMessages(GT2Socket socket)
{
	int rcode;
	SOCKADDR_IN address;
	socklen_t addressLen;


	// Avoid overflowing stack.
	#if (GTI2_STACK_RECV_BUFFER_SIZE > 1600)
		static char buffer[GTI2_STACK_RECV_BUFFER_SIZE];
	#else
		char buffer[GTI2_STACK_RECV_BUFFER_SIZE];
	#endif


	#ifdef GSI_ADHOC
	if(socket->protocolType == GTI2AdHocProtocol)
	{
		return gti2ReceiveAdHocMessages(socket,buffer,GTI2_STACK_RECV_BUFFER_SIZE);
	}
	#endif

	// Check for messages.
	while	(CanReceiveOnSocket(socket->socket))
	{
		// mj todo: get this plat specific stuff out of here.  Belongs in play specific layer.
		// abstract recvfrom

		// Receive the message.
		addressLen = sizeof(address);
		
		rcode = recvfrom(socket->socket, buffer, sizeof(buffer), 0, (SOCKADDR *)&address, &addressLen);
		
		if (gsiSocketIsError(rcode))
		{
			rcode = GOAGetLastError(socket->socket);
			if(rcode == WSAECONNRESET)
			{
				// Handle the reset.
				if(!gti2HandleConnectionReset(socket, address.sin_addr.s_addr, ntohs(address.sin_port)))
					return GT2False;
			}
			#ifndef SN_SYSTEMS
				else if (rcode == WSAEHOSTUNREACH)
				{
					if (!gti2HandleHostUnreachable(socket, address.sin_addr.s_addr, ntohs(address.sin_port), GT2False))
						return GT2False;
				}
			#endif
			else if(rcode != WSAEMSGSIZE)
			{
				// Fatal socket error.
				gti2SocketError(socket);
				return GT2False;
			}
		}
		else
		{
			#ifdef RECV_LOG
				// Log the message.
				gti2LogMessage(address.sin_addr.s_addr, ntohs(address.sin_port),
					socket->ip, socket->port,
					buffer, rcode);
			#endif
			// Handle the message.
			if(!gti2HandleMessage(socket, (GT2Byte *)buffer, rcode, address.sin_addr.s_addr, ntohs(address.sin_port)))
				return GT2False;
		}
	}

	return GT2True;
}

static GT2Bool gti2StoreOutgoingReliableMessageInfo(GT2Connection connection, unsigned short SN, int len)
{
	GTI2OutgoingBufferMessage messageInfo;
	int num;

	// Setup the message info.
	memset(&messageInfo, 0, sizeof(messageInfo));
	messageInfo.start = connection->outgoingBuffer.len;
	messageInfo.len = len;
	messageInfo.serialNumber = SN;
	messageInfo.lastSend = current_time();

	// Check the number of messages before we do the add.
	num = ArrayLength(connection->outgoingBufferMessages);

	// Add it to the list.
	ArrayAppend(connection->outgoingBufferMessages, &messageInfo);

	// Make sure the length is one more.
	if(ArrayLength(connection->outgoingBufferMessages) != (num + 1))
		return GT2False;

	return GT2True;
}

static GT2Bool gti2BeginReliableMessage(GT2Connection connection, GTI2MessageType type, int len, GT2Bool * overflow)
{
	int freeSpace;

	// VDP data length needed in the front of every packet.
	unsigned short vdpDataLength = (unsigned short)(len - connection->socket->protocolOffset);
	
	// Check how much free space is in the outgoing buffer.
	freeSpace = gti2GetBufferFreeSpace(&connection->outgoingBuffer);

	// Do we have the space to hold it?
	if(freeSpace < len)
	{
		if(!gti2ConnectionMemoryError(connection))
			return GT2False;

		*overflow = GT2True;
		return GT2True;
	}

	// Store the message's info.
	if(!gti2StoreOutgoingReliableMessageInfo(connection, connection->serialNumber, len))
	{
		if(!gti2ConnectionMemoryError(connection))
			return GT2False;

		*overflow = GT2True;
		return GT2True;
	}

	// Setup the header.
	if (connection->socket->protocolType == GTI2VdpProtocol)
		gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)&vdpDataLength, connection->socket->protocolOffset);
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	gti2BufferWriteByte(&connection->outgoingBuffer, (GT2Byte)type);
	gti2BufferWriteUShort(&connection->outgoingBuffer, connection->serialNumber++);
	gti2BufferWriteUShort(&connection->outgoingBuffer, connection->expectedSerialNumber);

	*overflow = GT2False;
	return GT2True;
}

static GT2Bool gti2EndReliableMessage(GT2Connection connection)
{
	GTI2OutgoingBufferMessage * message;
	int len;

	// The message we're sending is the last one.
	len = ArrayLength(connection->outgoingBufferMessages);
	GS_ASSERT(len > 0);
	message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, len - 1);

	// Send the message.
	if(!gti2ConnectionSendData(connection, connection->outgoingBuffer.buffer + message->start, message->len))
		return GT2False;

	// We just did an ack (as part of the message).
	connection->pendingAck = GT2False;

	return GT2True;
}

GT2Bool gti2SendAppReliable(GT2Connection connection, const GT2Byte * message, int len)
{
	int totalLen;
	GT2Bool overflow;

	// This totalLen is: magic string + type + SN + ESN + message.
	totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2 + len);

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgAppReliable, totalLen, &overflow))
		return GT2False;
	if(overflow)
		return GT2True;

	// Write the message.
	gti2BufferWriteData(&connection->outgoingBuffer, message, len);

	// End the message
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendClientChallenge(GT2Connection connection, const char challenge[GTI2_CHALLENGE_LEN])
{
	// This totalLen is: magic string + type + SN + ESN + challenge.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2 + GTI2_CHALLENGE_LEN);
	GT2Bool overflow;

	// Bbegin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgClientChallenge, totalLen + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// Write the challenge.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)challenge, GTI2_CHALLENGE_LEN);

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendServerChallenge(GT2Connection connection, const char response[GTI2_RESPONSE_LEN], const char challenge[GTI2_CHALLENGE_LEN])
{
	// This totalLen is: magic string + type + SN + ESN + response + challenge.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2 + GTI2_RESPONSE_LEN + GTI2_CHALLENGE_LEN);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgServerChallenge, totalLen + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// Write the response.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)response, GTI2_RESPONSE_LEN);

	// Write the challenge.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)challenge, GTI2_CHALLENGE_LEN);

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	// Save the time.
	connection->challengeTime = connection->lastSend;

	return GT2True;
}

GT2Bool gti2SendClientResponse(GT2Connection connection, const char response[GTI2_RESPONSE_LEN], const char * message, int len)
{
	// This totalLen is: magic string + type + SN + ESN + response + message.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2 + GTI2_RESPONSE_LEN + len);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgClientResponse, totalLen + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// Write the response.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)response, GTI2_RESPONSE_LEN);

	// Write the message.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)message, len);

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendAccept(GT2Connection connection)
{
	// This totalLen is: magic string + type + SN + ESN.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgAccept, totalLen  + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendReject(GT2Connection connection, const GT2Byte * message, int len)
{
	// This totalLen is: magic string + type + SN + ESN + message.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2 + len);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgReject, totalLen + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// Write the message.
	gti2BufferWriteData(&connection->outgoingBuffer, message, len);

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendClose(GT2Connection connection)
{
	// This totalLen is: magic string + type + SN + ESN.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgClose, totalLen + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendKeepAlive(GT2Connection connection)
{
	// This totalLen is: magic string + type + SN + ESN.
	int totalLen = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2);
	GT2Bool overflow;

	// Begin the message.
	if(!gti2BeginReliableMessage(connection, GTI2MsgKeepAlive, totalLen  + connection->socket->protocolOffset, &overflow))
		return GT2False;

	if(overflow)
		return GT2True;

	// End the message.
	if(!gti2EndReliableMessage(connection))
		return GT2False;

	return GT2True;
}

GT2Bool gti2SendAppUnreliable(GT2Connection connection, const GT2Byte * message, int len)
{
	int freeSpace;
	int totalLen;
	GT2Byte * start;

	// Check to see if we can send the message right away (unreliable that 
	// doesn't start with the magic string).
	if((len < GTI2_MAGIC_STRING_LEN) || 
		(memcmp(message + connection->socket->protocolOffset, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN) != 0))
	{
		if(!gti2ConnectionSendData(connection, message, len))
			return GT2False;

		return GT2True;
	}
	
	// This totalLen is: magic string + message.
	totalLen = (GTI2_MAGIC_STRING_LEN + len);

	// Check how much free space is in the outgoing buffer.
	freeSpace = gti2GetBufferFreeSpace(&connection->outgoingBuffer);

	// Do we have the space to hold the message?
	if(freeSpace < totalLen)
	{
		// Since we don't have the space, just drop the message.
		return GT2True;
	}

	// Store the start of the actual message in the buffer.
	start = (connection->outgoingBuffer.buffer + connection->outgoingBuffer.len);

	// Copy the VDP data length if necessary.
	if (connection->socket->protocolType == GTI2VdpProtocol)
		gti2BufferWriteData(&connection->outgoingBuffer, message, 2);
	
	// Copy the message in, repeating the magic string at the beginning.
	gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	
	// Copy the data at the starting position + offset based on the protocol.
	gti2BufferWriteData(&connection->outgoingBuffer, message + connection->socket->protocolOffset, 
		(int)(len - connection->socket->protocolOffset));
	
	// Do the send.
	if(!gti2ConnectionSendData(connection, start, totalLen))
		return GT2False;

	// We don't need to save the message.
	gti2BufferShorten(&connection->outgoingBuffer, -1, totalLen);
	
	return GT2True;
}

GT2Bool gti2SendAck(GT2Connection connection)
{
	// Always allocate data length + magic string + type + ESN.
	// Part of the buffer may not be used, but it's more efficient to be on the 
	// stack.
	char buffer[MAX_PROTOCOL_OFFSET + GTI2_MAGIC_STRING_LEN + 1 + 2];
	int pos = 0;
	
	// Write the VDP data length.
	if (connection->socket->protocolType == GTI2VdpProtocol)
	{
		short dataLength = (GTI2_MAGIC_STRING_LEN + 1 + 2);
		memcpy(buffer, &dataLength, 2);
		pos += 2;
	}

	// Write the magic string.
	memcpy(buffer + pos, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	pos += GTI2_MAGIC_STRING_LEN;

	// Write the type.
	buffer[pos++] = GTI2MsgAck;

	// Write the ESN.
	gti2UShortToBuffer((GT2Byte *)buffer, pos, connection->expectedSerialNumber);
	pos += 2;
	
	// send the message.
	if(!gti2ConnectionSendData(connection, (const GT2Byte *)buffer, pos))
		return GT2False;

	// We just did an ack.
	connection->pendingAck = GT2False;

	return GT2True;
}


GT2Bool gti2SendNack(GT2Connection connection, unsigned short SNMin, unsigned short SNMax)
{
	// data length + magic string + type + SNMin [+ SNMax]
	// Part of the buffer may not be used, but it's more efficient to be on the
	// stack.
	char buffer[MAX_PROTOCOL_OFFSET + GTI2_MAGIC_STRING_LEN + 1 + 2 + 2];
	int pos = 0;

	// Write the VDP data length.
	if (connection->socket->protocolType == GTI2VdpProtocol)
	{
		short dataLength = (GTI2_MAGIC_STRING_LEN + 1 + 2 + 2);
		memcpy(buffer, &dataLength, 2);
		pos += 2;
	}

	// Write the magic string.
	memcpy(buffer + pos, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	pos += GTI2_MAGIC_STRING_LEN;

	// Write the type.
	buffer[pos++] = GTI2MsgNack;

	// Write the SNMin.
	gti2UShortToBuffer((GT2Byte *)buffer, pos, SNMin);
	pos += 2;

	// Write the SNMax if it's different.
	if(SNMin != SNMax)
	{
		gti2UShortToBuffer((GT2Byte *)buffer, pos, SNMax);
		pos += 2;
	}

	// Send the message.
	if(!gti2ConnectionSendData(connection, (const GT2Byte *)buffer, pos))
		return GT2False;

	return GT2True;
}


GT2Bool gti2SendPing(GT2Connection connection)
{
	// data length + magic string + type + "time" + current time
	// Part of the buffer may not be used, but it's more efficient to be on the 
	// stack.
	char buffer[MAX_PROTOCOL_OFFSET + GTI2_MAGIC_STRING_LEN + 1 + 4 + sizeof(gsi_time)];
	int pos = 0;
	gsi_time now;

	// Write the VDP data length.
	if (connection->socket->protocolType == GTI2VdpProtocol)
	{
		short dataLength = (GTI2_MAGIC_STRING_LEN + 1 + 4 + sizeof(gsi_time));
		memcpy(buffer, &dataLength, 2);
		pos += 2;
	}
	// Write the magic string.
	memcpy(buffer + pos, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	pos += GTI2_MAGIC_STRING_LEN;

	// Write the type.
	buffer[pos++] = GTI2MsgPing;

	// Copy the time id.
	memcpy(buffer + pos, "time", 4);
	pos += 4;

	// Write the current time.
	now = current_time();
	memcpy(buffer + pos, &now, sizeof(gsi_time));

	pos += (int)sizeof(gsi_time);
	// Send the message.
	if(!gti2ConnectionSendData(connection, (const GT2Byte *)buffer, pos))
		return GT2False;

	return GT2True;
}


GT2Bool gti2SendPong(GT2Connection connection, GT2Byte * message, int len)
{
	// Change the ping to a pong.
	message[GTI2_MAGIC_STRING_LEN] = GTI2MsgPong;

	// Send it.
	return gti2ConnectionSendData(connection, message, len);
}

GT2Bool gti2SendClosed(GT2Connection connection)
{
	// Normal close.
	return gti2SendClosedOnSocket(connection->socket, connection->ip, connection->port);
}


GT2Bool gti2SendClosedOnSocket(GT2Socket socket, unsigned int ip, unsigned short port)
{
	// Vdp data length (not including voice) + magic string + type
	// Part of the buffer may not be used, but it's more efficient to be on the 
	// stack.
	char buffer[MAX_PROTOCOL_OFFSET + GTI2_MAGIC_STRING_LEN + 1]; 
	int pos = 0;

	// Write the data length.
	if (socket->protocolType == GTI2VdpProtocol)
	{
		short dataLength = GTI2_MAGIC_STRING_LEN + 1;
		memcpy(buffer, &dataLength, 2);
		pos += 2;
	}

	// Write the magic string.
	memcpy(buffer + pos, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
	pos += GTI2_MAGIC_STRING_LEN;

	// Write the type.
	buffer[pos++] = GTI2MsgClosed;

	// Send it.
	if(!gti2SocketSend(socket, ip, port, (const GT2Byte *)buffer, pos))
		return GT2False;

	return GT2True;
}


GT2Bool gti2ResendMessage(GT2Connection connection, GTI2OutgoingBufferMessage * message)
{
	GTI2MessageType type;
	int pos;

	// Replace the ESN (it's after the magic string, the type, and the SN).
	pos = (message->start + connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN + 1 + 2);
	
	gti2UShortToBuffer(connection->outgoingBuffer.buffer, pos, connection->expectedSerialNumber);

	// Send the message.
	if(!gti2ConnectionSendData(connection, connection->outgoingBuffer.buffer + message->start, message->len))
		return GT2False;

	// Update the last time sent.
	message->lastSend = connection->lastSend;

	// If it was a server challenge, update that time too.
	type = (GTI2MessageType)connection->outgoingBuffer.buffer[message->start + connection->socket->protocolOffset + GTI2_MAGIC_STRING_LEN];
	
	if(type == GTI2MsgServerChallenge)
		connection->challengeTime = connection->lastSend;

	return GT2True;
}

GT2Bool gti2Send(GT2Connection connection, const GT2Byte * message, int len, GT2Bool reliable)
{
	if (reliable)
		return gti2SendAppReliable(connection, message, len);
	// Send unreliable messages.
	return gti2SendAppUnreliable(connection, message, len);
}

GT2Bool gti2WasMessageIDConfirmed(const GT2Connection connection, GT2MessageID messageID)
{
	GTI2OutgoingBufferMessage * message;
	int len;

	// If there are no reliable messages awaiting confirmation, then this has 
	// already been confirmed.
	len = ArrayLength(connection->outgoingBufferMessages);
	if(!len)
		return GT2True;

	// Get the oldest message awaiting confirmation.
	message = (GTI2OutgoingBufferMessage *)ArrayNth(connection->outgoingBufferMessages, 0);

	// If the message id we are looking for is older than the first one 
	// awaiting confirmation, then it has already been confirmed.
	if(gti2SNDiff(messageID, message->serialNumber) < 0)
		return GT2True;

	// The message hasn't been confirmed yet.
	return GT2False;
}
