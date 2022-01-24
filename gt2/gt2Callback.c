///////////////////////////////////////////////////////////////////////////////
// File:	gt2Callback.c
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "gt2Callback.h"
#include "gt2Socket.h"

/*********************
** SOCKET CALLBACKS **
*********************/

GT2Bool gti2SocketErrorCallback
(
	GT2Socket socket
)
{
	GS_ASSERT(socket);
	if(!socket)
		return GT2True;

	if(!socket->socketErrorCallback)
		return GT2True;

	socket->callbackLevel++;

	socket->socketErrorCallback(socket);

	socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(socket->close && !socket->callbackLevel)
	{
		gti2CloseSocket(socket);
		return GT2False;
	}

	return GT2True;
}

GT2Bool gti2ConnectAttemptCallback
(
	GT2Socket socket,
	GT2Connection connection,
	unsigned int ip,
	unsigned short port,
	int latency,
	GT2Byte * message,
	int len
)
{
	GS_ASSERT(socket && connection);
	if(!socket || !connection)
		return GT2True;

	if(!socket->connectAttemptCallback)
		return GT2True;

	// Check for an empty message..
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	socket->callbackLevel++;
	connection->callbackLevel++;

	socket->connectAttemptCallback(socket, connection, ip, port, latency, message, len);

	socket->callbackLevel--;
	connection->callbackLevel--;

	// Check to see if the socket should be closed.
	if(socket->close && !socket->callbackLevel)
	{
		gti2CloseSocket(socket);
		return GT2False;
	}

	return GT2True;
}

/*************************
** CONNECTION CALLBACKS **
*************************/

GT2Bool gti2ConnectedCallback
(
	GT2Connection connection,
	GT2Result result,
	GT2Byte * message,
	int len
)
{
	GS_ASSERT(connection);
	if(!connection)
		return GT2True;
	
	// Store the result.
	connection->connectionResult = result;

	if(!connection->callbacks.connected)
		return GT2True;

	// Check for an empty message..
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	connection->callbacks.connected(connection, result, message, len);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

GT2Bool gti2ReceivedCallback
(
	GT2Connection connection,
	GT2Byte * message,
	int len,
	GT2Bool reliable
)
{
	GS_ASSERT(connection);
	if(!connection)
		return GT2True;

	if(!connection->callbacks.received)
		return GT2True;

	// Check for an empty message.
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	connection->callbacks.received(connection, message, len, reliable);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

GT2Bool gti2ClosedCallback
(
	GT2Connection connection,
	GT2CloseReason reason
)
{
	GS_ASSERT(connection);
	if(!connection)
		return GT2True;

	if(!connection->callbacks.closed)
		return GT2True;

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	connection->callbacks.closed(connection, reason);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

GT2Bool gti2PingCallback
(
	GT2Connection connection,
	int latency
)
{
	GS_ASSERT(connection);
	if(!connection)
		return GT2True;

	if(!connection->callbacks.ping)
		return GT2True;

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	connection->callbacks.ping(connection, latency);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

/*********************
** FILTER CALLBACKS **
*********************/

GT2Bool gti2SendFilterCallback
(
	GT2Connection connection,
	int filterID,
	const GT2Byte * message,
	int len,
	GT2Bool reliable
)
{
	gt2SendFilterCallback * callback;

	GS_ASSERT(connection);
	if(!connection)
		return GT2True;

	callback = (gt2SendFilterCallback *)ArrayNth(connection->sendFilters, filterID);
	if(!callback)
		return GT2True;

	// Check for an empty message.
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	(*callback)(connection, filterID, message, len, reliable);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

GT2Bool gti2ReceiveFilterCallback
(
	GT2Connection connection,
	int filterID,
	GT2Byte * message,
	int len,
	GT2Bool reliable
)
{
	gt2ReceiveFilterCallback * callback;

	GS_ASSERT(connection);
	if(!connection)
		return GT2True;

	callback = (gt2ReceiveFilterCallback *)ArrayNth(connection->receiveFilters, filterID);
	if(!callback)
		return GT2True;

	// Check for an empty message.
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	connection->callbackLevel++;
	connection->socket->callbackLevel++;

	(*callback)(connection, filterID, message, len, reliable);

	connection->callbackLevel--;
	connection->socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(connection->socket->close && !connection->socket->callbackLevel)
	{
		gti2CloseSocket(connection->socket);
		return GT2False;
	}

	return GT2True;
}

/*******************
** DUMP CALLBACKS **
*******************/

GT2Bool gti2DumpCallback
(
	GT2Socket socket,
	GT2Connection connection,
	unsigned int ip,
	unsigned short port,
	GT2Bool reset,
	const GT2Byte * message,
	int len,
	GT2Bool send
)
{
	gt2DumpCallback callback;

	GS_ASSERT(socket);
	if(!socket)
		return GT2True;

	if(send)
		callback = socket->sendDumpCallback;
	else
		callback = socket->receiveDumpCallback;

	if(!callback)
		return GT2True;

	// Check for an empty message.
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	socket->callbackLevel++;
	if(connection)
		connection->callbackLevel++;

	callback(socket, connection, ip, port, reset, message, len);

	socket->callbackLevel--;
	if(connection)
		connection->callbackLevel--;

	// Check to see if the socket should be closed.
	if(socket->close && !socket->callbackLevel)
	{
		gti2CloseSocket(socket);
		return GT2False;
	}

	return GT2True;
}

/*****************************
** SOCKET SHARING CALLBACKS **
*****************************/

GT2Bool gti2UnrecognizedMessageCallback
(
	GT2Socket socket,
	unsigned int ip,
	unsigned short port,
	GT2Byte * message,
	int len,
	GT2Bool * handled
)
{
	*handled = GT2False;

	GS_ASSERT(socket);
	if(!socket)
		return GT2True;

	if(!socket->unrecognizedMessageCallback)
		return GT2True;

	// Check for an empty message.
	if(!len || !message)
	{
		message = NULL;
		len = 0;
	}

	socket->callbackLevel++;

	*handled = socket->unrecognizedMessageCallback(socket, ip, port, message, len);

	socket->callbackLevel--;

	// Check to see if the socket should be closed.
	if(socket->close && !socket->callbackLevel)
	{
		gti2CloseSocket(socket);
		return GT2False;
	}

	return GT2True;
}
