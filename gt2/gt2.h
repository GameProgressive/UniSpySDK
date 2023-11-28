///////////////////////////////////////////////////////////////////////////////
// File:	gt2.h
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ------------------------------------
// See "configurable defines" in gt2Main.h for certain performance settings 
// that can be changed.

#ifndef _GT2_H_
#define _GT2_H_

#include "../common/gsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********
** TYPES **
**********/

// boolean
typedef int GT2Bool;
#define GT2False 0
#define GT2True 1

// A byte
typedef unsigned char GT2Byte;

// A handle to a socket object (can be used to accept connections and initiate
// connections).
typedef struct GTI2Socket * GT2Socket;

// A handle to an object representing a connection to a specific IP and port
// the local endpoint is a GT2Socket.
typedef struct GTI2Connection * GT2Connection;

// The id of a reliably sent message; unreliable messages don't have ids.
typedef unsigned short GT2MessageID;

//////////////////////////////////////////////////////////////
// GT2Result
// Summary
//		Result of a GT2 operation. Check individual function definitions to 
//		see possible results.
typedef enum
{
	GT2Success,             // Success.
	// Errors:
	GT2OutOfMemory,         // Ran out of memory.
	GT2Rejected,            // Attempt rejected.
	GT2NetworkError,        // Networking error (could be local or remote).
	GT2AddressError,        // Invalid or unreachable address.
	GT2DuplicateAddress,    // A connection was attempted to an address that already has a connection on the socket.
	GT2TimedOut,			// Time out reached.
	GT2NegotiationError,	// There was an error negotiating with the remote side.
	GT2InvalidConnection,	// The connection didn't exist.
	GT2InvalidMessage,		// Used for vdp reliable messages containing voice data, no voice data in reliable messages.
	GT2SendFailed			// The send failed.
} GT2Result;

//////////////////////////////////////////////////////////////
// GT2ConnectionState
// Summary
//		Possible states for any GT2Connection.
typedef enum
{
	GT2Connecting,   // Negotiating the connection.
	GT2Connected,    // The connection is active.
	GT2Closing,      // The connection is being closed.
	GT2Closed        // The connection has been closed and can no longer be used.
} GT2ConnectionState;

//////////////////////////////////////////////////////////////
// GT2CloseReason
// Summary
//		Reason the connection was closed.
typedef enum
{
	GT2LocalClose,         // The connection was closed with gt2CloseConnection.
	GT2RemoteClose,        // The connection was closed remotely.
	// errors:
	GT2CommunicationError, // An invalid message was received (it was either unexpected or incorrectly formatted).
	GT2SocketError,        // An error with the socket forced the connection to close.
	GT2NotEnoughMemory     // There wasn't enough memory to store an incoming or outgoing message.
} GT2CloseReason;

/************
** GLOBALS **
************/

// The challenge key is a 32 character string that is used in the 
// authentication process. The key can be set before GT2 is used so that the 
// key will be application-specific.
extern char GT2ChallengeKey[33];

/*********************
** SOCKET CALLBACKS **
*********************/

//////////////////////////////////////////////////////////////
// gt2SocketErrorCallback
// Summary
//		This callback is used to notify the application of a closed socket 
//		or fatal socket error condition.
// Parameters
//		socket	: [in] The handle to the socket.
// Remarks
//		Once this callback returns, the socket and all of its connections are 
//		invalid and can no longer be used. All connections that use this socket 
//		are terminated, and their gt2CloseCallback callbacks will be called 
//		before this callback is called (with the reason set to 
//		GT2SocketError).<p>
// See Also
//		gt2CreateSocket
typedef void ( * gt2SocketErrorCallback)
(
	GT2Socket socket
);

/*********************
** SOCKET FUNCTIONS **
*********************/

//////////////////////////////////////////////////////////////
// gt2CreateSocket
// Summary
//		Creates a new socket, which can be used for making outgoing
//		 connections or accepting incoming connections.
// Parameters
//		socket				: [out] Pointer to the socket handle.
//		localAddress		: [in] The address to bind to locally.  Typically of the form
//									":&lt;port&gt;", e.g., ":7777".  Can be NULL or "".
//		outgoingBufferSize	: [in] The byte size of the per-connection
//		 buffer for reliable outgoing messages.  
//									Can be 0 to use the internal default.
//		incomingBufferSize	: [in] The byte size of the per-connection
//		 buffer for out-of-order reliable incoming messages.  
//									Can be 0 to use the internal default.
//		callback			: [in] The callback to be called if there is a
//		 fatal error with the socket.
// Returns
//		If the function returns GT2Success then the socket was successfully
//		 created.  
//		Otherwise, GT2 was unable to create the socket.
// Remarks
//		A socket is an endpoint on the local machine that allows an
//		 application to communicate 
//		with other applications (through their own sockets) that are
//		 typically on remote machines, 
//		although they can also be on the local machine (the other
//		 application will often be referred 
//		to as the "remote machine", even though technically it may be the
//		 same machine).
//		A single socket allows an application to both accept connections
//		 from remote machines and 
//		make connections to remote machines.
//		For most applications, only one socket needs to be created.
//		All incoming connections can be accepted on the socket, and all
//		 outgoing connections can be made using the socket.
//		A socket is created with the gt2CreateSocket function.
//		If the function returns GT2Success then the socket was successfully
//		 created and bound 
//		to the local address (if one was provided).
//		The socket that the "socket" parameter points to is valid until it
//		 is closed with gt2CloseSocket, 
//		or an error is reported to the gt2SocketErrorCallback callback parameter.
//		It is now ready to be used for making outgoing connections, and can
//		 be readied for allowing 
//		incoming connections by calling gt2Listen.
//		If the return result is anything other than GT2Success, GT2 was
//		 unable to create the socket.<p>
// See Also
//		gt2SocketErrorCallback, gt2CloseSocket, gt2Listen, gt2Connect
GT2Result gt2CreateSocket
(
	GT2Socket * socket,  // if the result is GT2Success, the socket object handle will be stored at this address
	const char * localAddress,  // the local address to bind to
	int outgoingBufferSize,  // size of per-connection buffer where sent messages waiting to be confirmed are held, use 0 for default
	int incomingBufferSize,  // size of per-connection buffer where out-of-order received messages are held, use 0 for default
	gt2SocketErrorCallback callback  // a callback that is called if there is an error with the socket
);

//////////////////////////////////////////////////////////////
// gt2CreateAdHocSocket
// Summary
//		Creates a new socket, which can be used for making outgoing
//		 connections or accepting incoming connections.
//		See gt2CreateSocket for details.
// Remarks
//		AdHoc Sockets use MAC address instead of IP address.<p>
// See Also
//		gt2CreateSocket
GT2Result gt2CreateAdHocSocket
(
	GT2Socket * socket,			// if the result is GT2Success, the socket object handle will be stored at this address
	const char * localAddress,  // the local address to bind to
	int outgoingBufferSize,		// size of per-connection buffer where sent messages waiting to be confirmed are held, use 0 for default
	int incomingBufferSize,		// size of per-connection buffer where out-of-order received messages are held, use 0 for default
	gt2SocketErrorCallback callback  // a callback that is called if there is an error with the socket
);

#ifdef _XBOX
// creates a local VDP socket on the Xbox platform
// if the IP of the local address is 0, then any/all ips will be bound.
// if the port of the local address is 0, then a port will be assigned.
// if either buffer sizes is set to 0, a default value will be used (currently 4K).
// the buffer needs to be able to hold all messages waiting for confirmation of delivery,
// and it needs to hold any messages that arrive out of order. if either buffer runs out
// of space the connection will be dropped.
GT2Result gt2CreateVDPSocket
(
	GT2Socket * socket,  // if the result is GT2Success, the socket object handle will be stored at this address
	const char * localAddress,  // the local address to bind to
	int outgoingBufferSize,  // size of per-connection buffer where sent messages waiting to be confirmed are held, use 0 for default
	int incomingBufferSize,  // size of per-connection buffer where out-of-order received messages are held, use 0 for default
	gt2SocketErrorCallback callback  // a callback that is called if there is an error with the socket
);
#endif

//////////////////////////////////////////////////////////////
// gt2CloseSocket
// Summary
//		Closes a socket.
// Parameters
//		socket	: [in] The handle to the socket.
// Remarks
//		All existing connections will be hard closed, as if
//		 gt2CloseAllConnectionsHard was called for this socket.
//		All connections send a close message to the remote side, and any
//		 closed callbacks will be called from within 
//		this function.<p>
// See Also
//		gt2CreateSocket
void gt2CloseSocket(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2Think
// Summary
//		Does any thinking for this socket and its connections.
// Parameters
//		socket	: [in] The handle to the socket.
// Remarks
//		Callbacks are typically called from within this function (although
//		 they can also be called from other places).
//		It is possible that during this think the socket or any of its
//		 connections may be closed, 
//		so care must be taken if calling other GT2 functions immediately
//		 after thinking.
//		The more frequently this function is called, the faster GT2 will be
//		 able to respond (and reply to) messages.
//		The general rule is to call it at frequently as you can, although
//		 calling it faster 
//		than every 10-20 milliseconds is probably unnecessary.
//		If you are using gt2Ping to measure ping times, then the accuracy of
//		 the latency measurement will 
//		increase with the frequency at which this function is called.<p>
void gt2Think(GT2Socket socket);

// sends a raw UDP datagram through the socket
// this function bypasses the normal connection logic
// note that all messages sent this way will be unreliable
// to broadcast a datagram, omit the IP from the remoteAddress (e.g., ":12345")
GT2Result gt2SendRawUDP
(
	GT2Socket socket,  // the socket through which to send the raw UDP datagram
	const char * remoteAddress,  // the address to which to send the datagram
	const GT2Byte * message,  // the message to send, or NULL for an empty datagram
	int len  // the len of the message (0 for an empty message, ignored if message==NULL)
);

/*************************
** CONNECTION CALLBACKS **
*************************/

//////////////////////////////////////////////////////////////
// gt2ConnectedCallback
// Summary
//		This callback is called when a connection attempt with gt2Connect finishes.
// Parameters
//		connection	: [in] The handle to the connection.
//		result		: [in] The result of the connection attempt.  Anything
//		 aside from GT2Success indicates failure.
//		message		: [in] If result is GT2Rejected, this is the
//		 rejection message.  May be NULL.
//		len			: [in] If result is GT2Rejected, the length of message.  May be 0.
// Remarks
//		If result is GT2Success, then this connection attempt succeeded.
//		The connection object can now be used for sending/receiving messages.
//		Any other result indicates connection failure, and the connection
//		 object cannot be used 
//		again after this callback returns.
//		If the result is GT2Rejected, then message contains an optional
//		 rejection message sent by the listener.
//		If result is not GT2Rejected, then message will be NULL and len will
//		 be 0.<p>
// See Also
//		gt2Connect
typedef void (* gt2ConnectedCallback)
(
	GT2Connection connection,       // The connection object.
	GT2Result result,               // Result from connect attempt.
	GT2Byte * message,              // If result==GT2Rejected, the reason.  Otherwise, NULL.
	int len                         // If result==GT2Rejected, the length of the reason.  Otherwise, 0.
);

//////////////////////////////////////////////////////////////
// gt2ReceivedCallback
// Summary
//		This callback is called when a message is sent from the remote
//		 system with a gt2Send.
// Parameters
//		connection	: [in] The handle to the connection.
//		message		: [in] The message that was sent.  May be NULL.
//		len			: [in] The length of the message.  May be 0.
//		reliable	: [in] GT2True if the message was sent reliably.
// Remarks
//		If an message is sent from the remote end of the connection
//		 reliably, then it will always 
//		be received with this callback.
//		If it is not sent reliably, then the message might not be received,
//		 it might be received 
//		out of order, or it might be received more than once (very rare).<p>
// See Also
//		gt2Send
typedef void (* gt2ReceivedCallback)
(
	GT2Connection connection,       // The connection the message was received on.
	GT2Byte * message,              // The message that was received.  Will be NULL if an empty message.
	int len,                        // The length of the message in bytes.  Will be 0 if an empty message.
	GT2Bool reliable                // True if this is was sent reliably.
);

//////////////////////////////////////////////////////////////
// gt2ClosedCallback
// Summary
//		This callback is called when the connection has been closed.
// Parameters
//		connection	: [in] The handle to the connection.
//		reason		: [in] The reason the connection closed.
// Remarks
//		A connection close can be caused by either side calling
//		 gt2CloseConnection (or gt2CloseConnectionHard), 
//		either side closing the socket, or some sort of error.
//		The connection cannot be used again once this callback returns.<p>
// See Also
//		gt2CloseConnection, gt2CloseConnectionHard, gt2Connect, gt2Accept
typedef void (* gt2ClosedCallback)
(
	GT2Connection connection,       // The connection that was closed.
	GT2CloseReason reason           // The reason the connection was closed.
);

//////////////////////////////////////////////////////////////
// gt2PingCallback
// Summary
//		This callback is called when a response to a ping sent on this
//		 connection is received.
// Parameters
//		connection	: [in] The handle to the connection.
//		latency		: [in] The round trip time of the ping, in milliseconds.
// Remarks
//		This callback gives a measure of the time it takes for a datagram to
//		 make a round-trip from one 
//		connection to the other.
//		The latency reported in this callback will typically be larger than
//		 that reported by using ICMP 
//		pings between the two machines (the "ping" program uses ICMP pings),
//		 because ICMP pings happen at 
//		a lower level in the operating system.
//		However, the ping reported in this callback will much more
//		 accurately reflect the latency of the application, 
//		as the application's messages must go through the same path as these
//		 pings, as opposed to ICMP.<p>
// See Also
//		gt2Ping
typedef void (* gt2PingCallback)
(
	GT2Connection connection,        // the connection the ping was sent on
	int latency                      // the round-trip time for the ping, in milliseconds
);

// Callbacks set for each connection.
// The connected callback is ignored
// when this is passed to gt2Accept.
typedef struct
{
	gt2ConnectedCallback connected; // Called when gt2Connect is complete.
	gt2ReceivedCallback received;   // Called when a message is received.
	gt2ClosedCallback closed;       // Called when the connection is closed (remotely or locally).
	gt2PingCallback ping;           // Called when a ping reply is received.
} GT2ConnectionCallbacks;

/*************************
** CONNECTION FUNCTIONS **
*************************/

//////////////////////////////////////////////////////////////
// gt2Connect
// Summary
//		Initiates a connection between a local socket and a remote socket.
// Parameters
//		socket			: [in] The handle to the socket.
//		connection		: [out] A pointer to where the connection handle will be stored.
//		remoteAddress	: [in] The address to connect to.
//		message			: [in] An optional initial message (may be NULL).
//		len				: [in] Length of the initial message (may be 0, or -1 for strlen)
//		timeout			: [in] Timeout in milliseconds (may be 0 for infinite retries)
//		callbacks		: [in] GT2Connection related callbacks.
//		blocking		: [in] If GT2True, don't return until the attempt
//		 has completed (successfully or unsuccessfuly).
// Returns
//		If blocking is true, GT2Success means the connect attempt succeeded,
//		 and anything else means it failed.
// Remarks
//		The gt2Connect function is used to initiate a connection attempt to
//		 a remote socket on the Internet.
//		After the remote socket is contacted, both it and the local
//		 connector will authenticate the 
//		other during a negotation phase.
//		Once the remote socket accepts the connection attempt, the
//		 connection will be established.
//		The connection lasts until the closed callback gets called, which
//		 can happen because one side 
//		closed the connection with gt2CloseConnection (or
//		 gt2CloseConnectionHard), there was some sort 
//		of error on the connection, or the socket either connection uses is
//		 closed.<p>
// See Also
//		gt2ConnectedCallback, gt2ClosedCallback, gt2CloseConnection,
//		 gt2AddressToString
GT2Result gt2Connect
(
	GT2Socket socket,  // the local socket to use for the connection
	GT2Connection * connection,  // if the result is GT2Success, and blocking is false, the connection  object handle is stored here
	const char * remoteAddress,  // the address to connect to
	const GT2Byte * message,  // an optional initial message (may be NULL)
	int len,  // length of the initial message (may be 0, or -1 for strlen)
	int timeout,  // timeout in milliseconds (may be 0 for infinite retries)
	GT2ConnectionCallbacks * callbacks,  // callbacks for connection related stuff
	GT2Bool blocking  // if true, don't return until complete (successfuly or unsuccessfuly)
);

//////////////////////////////////////////////////////////////
// gt2Send
// Summary
//		Sends data over a connection, reliably or unreliably.
// Parameters
//		connection	: [in] The handle to the connection.
//		message		: [in] The message to send.  Can be NULL.
//		len			: [in] The length of the message.  Can be 0.  A len
//		 of -1 is equivalent to (strlen(message) + 1).
//		reliable	: [in] if GT2True, send the message reliably, otherwise
//		 send it unreliably.
// Remarks
//		Once a connection has been established, messages can be sent back
//		 and forth on it.
//		To send a message, use the gt2Send function.
//		If message is NULL or len is 0, then an empty message will be sent.
//		When an empty message is received, message will be NULL and len will be 0.
//		If the message is sent reliably, it is guaranteed to arrive, arrive
//		 only once, 
//		and arrive in order (relative to other reliable messages).
//		If the message is sent unreliably, then it is not guaranteed to
//		 arrive, and if it does arrive, 
//		it is not guaranteed to arrive in order, or only once.
//		  Note that the 7 byte header must be accounted for in the message
//		 if the function sends the message reliably.<p>
// See Also
//		gt2ReceivedCallback
GT2Result gt2Send
(
	GT2Connection connection,  // the connection to send the message on
	const GT2Byte * message,  // the message to send, or NULL for an empty message0
	int len,  // the len of the message (0 for an empty message, ignored if message==NULL)
	GT2Bool reliable  // if true, send the message reliably
);

//////////////////////////////////////////////////////////////
// gt2Ping
// Summary
//		Sends a ping on a connection in an attempt to determine latency.
// Parameters
//		connection	: [in] The handle to the connection.
// Remarks
//		The ping callback will, which is set as part of the
//		 GT2ConnectionCallbacks in either gt2Connect or gt2Accept, 
//		will be called if and when a ping finishes making a round-trip
//		 between the local end of the connection and the 
//		remote end.
//		The ping is unreliable, and either it or the pong sent in reply
//		 could be dropped, resulting in the callback 
//		never being called.
//		Or it could even arrive multiple times, resulting in multiple calls
//		 to the callback (this case is very rare).<p>
// See Also
//		gt2PingCallback
void gt2Ping(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2CloseConnection
// Summary
//		Starts closing a connection.
// Parameters
//		connection	: [in] The handle to the connection.
// Remarks
//		This function attempts to synchronize the close with the remote side
//		 of the connection.
//		This means that the connection does not close immediately, and
//		 messages may be received 
//		while attempting the close.
//		When the close is completed, the connection's closed callback will
//		 be called.
//		Use gt2CloseConnectionHard to immediately close a connection.<p>
// See Also
//		gt2CloseConnectionHard, gt2CloseAllConnections, gt2CloseAllConnectionsHard
void gt2CloseConnection(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2CloseConnectionHard
// Summary
//		Closes a connection immediately.
// Parameters
//		connection	: [in] The handle to the connection.
// Remarks
//		This function closes a connection without waiting for confirmation
//		 from the remote side of the connection.
//		Messages in transit may be lost.
//		The connection's closed callback will be called from within this
//		 function.<p>
// See Also
//		gt2CloseConnection, gt2CloseAllConnections, gt2CloseAllConnectionsHard
void gt2CloseConnectionHard(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2CloseAllConnections
// Summary
//		Closes all of a socket's connections.
// Parameters
//		socket	: [in] The handle to the socket.
// Remarks
//		Same effect as calling gt2CloseConnection on all of the socket's
//		 connections.<p>
// See Also
//		gt2CloseConnection, gt2CloseConnectionHard, gt2CloseAllConnectionsHard
void gt2CloseAllConnections(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2CloseAllConnectionsHard
// Summary
//		Does a hard close on all of a socket's connections.
// Parameters
//		socket	: [in] The handle to the socket.
// Remarks
//		Has the same effect as calling gt2CloseConnectionHard on all of the
//		 socket's connection.<p>
// See Also
//		gt2CloseConnection, gt2CloseConnectionHard, gt2CloseAllConnections
void gt2CloseAllConnectionsHard(GT2Socket socket);

/*********************
** LISTEN CALLBACKS **
*********************/

//////////////////////////////////////////////////////////////
// gt2ConnectAttemptCallback
// Summary
//		This notifies the socket that a remote system is attempting a connection.
// Parameters
//		socket		: [in] The handle to the socket.
//		connection	: [in] The handle to the connection.
//		ip			: [in] The IP (network byte order) from which the
//		 connect attempt is coming.
//		port		: [in] The port (host byte order) from which the connect
//		 attempt is coming
//		latency		: [in] estimate of the round-trip time between the
//		 two machines (in milliseconds).
//		message		: [in] Optional initial data sent with the connect
//		 attempt.  May be NULL.
//		len			: [in] Length of the initial data.  May be 0.
// Remarks
//		The IP and port of the remote system is provided, along with an
//		 optional initial message, and a latency estimate.
//		These can be used to validate/authenticate the connecting system.
//		This connection must either be accepted with gt2Accept, or rejected
//		 with gt2Reject.
//		These can be called from within this callback, however they do not
//		 need to be.
//		They can be called at any time after this callback is received.
//		This is very useful for systems that need to check with another
//		 machine to authenticate the user 
//		(such as for a CDKey system).
//		The latency is only an estimate, however it can be used for things
//		 such as only allowing low-ping or 
//		high-ping users onto a server.<p>
// See Also
//		gt2Listen, gt2Connect, gt2Accept, gt2Reject
typedef void (* gt2ConnectAttemptCallback)
(
	GT2Socket socket,  // the socket the attempt came in on
	GT2Connection connection,  // a connection object for the incoming connection attempt
	unsigned int ip,  // the IP being used remotely for the connection attempt
	unsigned short port,  // the port being used remotely for the connection attempt
	int latency,  // the approximate latency on the connection
	GT2Byte * message,  // an optional message sent with the attempt.  Will be NULL if an empty message.
	int len  // the length of the message, in characters.  Will be 0 if an empty message.
);

/*********************
** LISTEN FUNCTIONS **
*********************/

//////////////////////////////////////////////////////////////
// gt2Listen
// Summary
//		Start (or stop) listening for incoming connections on a socket.
// Parameters
//		socket	: [in] The handle to the socket.
//		callback	: [in] Function to be called when the operation completes
// Remarks
//		Once a socket starts listening, any connections attempts will cause
//		 the callback to be called.<p>
//		If the socket is already listening, this callback will replace the
//		 existing callback being used
//		If the callback is NULL, this will cause the connection to stop listening
// See Also
//		gt2CreateSocket, gt2ConnectAttemptCallback
void gt2Listen(GT2Socket socket, gt2ConnectAttemptCallback callback);

//////////////////////////////////////////////////////////////
// gt2Accept
// Summary
//		Accepts an incoming connection attempt.
// Parameters
//		connection	: [in] The handle to the connection.
//		callbacks	: [in] The set of callbacks associated with the connection.
// Returns
//		GT2False means the connection was closed between when the
//		 gt2ConnectAttemptCallback was 
//		called and this function was called.  The connection cannot be used.
// Remarks
//		After a socket's gt2ConnectAttemptCallback has been called, this
//		 function can be used to 
//		accept the incoming connection attempt.
//		It can be called from either within the callback or some later time.
//		As soon as it is called the connection is active, and messages can
//		 be sent and received.
//		The remote side of the connection will have it's connected callback
//		 called with the result set to GT2Success.
//		The callbacks that are passed in to this function are the same
//		 callbacks that get passed to gt2Connect, 
//		with the exception that the connected callback can be ignored, as
//		 the connection is already established.
//		If this function returns GT2True, then the connection has been
//		 successfully accepted.
//		If it returns GT2False, then the remote side has already closed the
//		 connection attempt.
//		In that case, the connection is considered closed, and it cannot be
//		 referenced again.<p>
// See Also
//		gt2Listen, gt2ConnectAttemptCallback, gt2Reject
GT2Bool gt2Accept(GT2Connection connection, GT2ConnectionCallbacks * callbacks);

//////////////////////////////////////////////////////////////
// gt2Reject
// Summary
//		Rejects a connection attempt.
// Parameters
//		connection	: [in] The handle to the connection.
//		message		: [in] Rejection message.  May be NULL.
//		len			: [in] Length of the rejection message.  May be 0.  A
//		 len of -1 is equivalent to (strlen(message) + 1).
// Remarks
//		After a socket's gt2ConnectAttemptCallback has been called, this
//		 function can be used 
//		to reject the incoming connection attempt.
//		It can be called from either within the callback or some later time.
//		Once the function is called the connection is considered closed and
//		 cannot be referenced again.
//		The remote side attempting the connection will have its connected
//		 callback called with the 
//		result set to gt2Rejected.
//		If the message is not NULL and the len is not 0, the message will be
//		 sent with the rejection, 
//		and passed into the remote side's connected callback.
//		Note that the 7 byte header must be accounted for in the message
//		 since this function will 
//		send the rejection message reliably.<p>
// See Also
//		gt2Listen, gt2ConnectAttemptCallback, gt2Accept
void gt2Reject(GT2Connection connection, const GT2Byte * message, int len);

/*************************
** MESSAGE CONFIRMATION **
*************************/

//////////////////////////////////////////////////////////////
// gt2GetLastSentMessageID
// Summary
//		Gets the message id for the last reliably sent message.
//		Unreliable messages do not have an id.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The message ID of the last reliably sent message.
// Remarks
//		This should be called immediately after gt2Send.
//		Waiting until after a call to gt2Think can result in an invalid
//		 message id being returned.
//		Note that the use of filters that can either drop or delay messages
//		 can complicate the process, 
//		because in those cases a call to gt2Send does not guarantee that a
//		 message will actually be sent.
//		In those cases, gt2GetLastSentMessageID should be called after
//		 gt2FilteredSend, because the actual 
//		message will be sent from within that function.<p>
// See Also
//		gt2WasMessageIDConfirmed
GT2MessageID gt2GetLastSentMessageID(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2WasMessageIDConfirmed
// Summary
//		Checks if confirmation has been received that the remote end
//		 received a particular reliable message.
// Parameters
//		connection	: [in] The handle to the connection.
//		messageID	: [in] The ID of the message to check for confirmation.
// Returns
//		GT2True if confirmation was received locally that the reliable
//		 message represented by messageID was received by the remote end of
//		 the connection, GT2False if confirmation was not yet received.
// Remarks
//		This should only be called on message ids that were returned by
//		 gt2GetLastSendMessageID, 
//		and should be used relatively soon after the message was sent, due
//		 to message ids wrapping around 
//		after a period of time.<p>
// See Also
//		gt2GetLastSentMessageID
GT2Bool gt2WasMessageIDConfirmed(GT2Connection connection, GT2MessageID messageID);

/*********************
** FILTER CALLBACKS **
*********************/

//////////////////////////////////////////////////////////////
// gt2SendFilterCallback
// Summary
//		Callback for filtering outgoing data.
// Parameters
//		connection	: [in] The handle to the connection.
//		filterID	: [in] Pass this ID to gt2FilteredSend.
//		message		: [in] The message being sent.  Will be NULL if an empty message.
//		len			: [in] The length of the message being sent, in
//		 bytes. Will be 0 if an empty message.
//		reliable	: [in] If the message is being sent reliably.
// Remarks
//		Call gt2FilteredSend with the filtered data, either from within the
//		 callback or later.<p>
//		The message points to the same memory location as the message passed
//		 to gt2Send (or gt2FilteredSend).  
//		So if the call to gt2FilteredSend is delayed, it is the filter's
//		 responsibility to 
//		make sure the data is still around when and if it is needed.
// See Also
//		gt2AddSendFilter, gt2RemoveSendFilter, gt2FilteredSend
typedef void (* gt2SendFilterCallback)
(
	GT2Connection connection,  // The connection on which the message is being sent.
	int filterID,              // Pass this ID to gt2FilteredSend.
	const GT2Byte * message,   // The message being sent.  Will be NULL if an empty message.
	int len,                   // The length of the message being sent, in bytes. Will be 0 if an empty message.
	GT2Bool reliable           // If the message is being sent reliably.
);

//////////////////////////////////////////////////////////////
// gt2ReceiveFilterCallback
// Summary
//		Callback for filtering incoming data.
// Parameters
//		connection	: [in] The handle to the connection.
//		filterID	: [in] Pass this ID to gtFilteredReceive.
//		message		: [in] The message that was received.  Will be NULL
//		 if an empty message.
//		len			: [in] The length of the message in bytes.  Will be 0
//		 if an empty message.
//		reliable	: [in] True if this is a reliable message.
// Description
//		Callback for filtering incoming data.
//		Call gt2FilteredRecieve with the filtered data,
//		either from within the callback or later.
//		the message may point to a memory location supplied to
//		 gt2FilteredReceive by a previous filter.
//		so if this filter's call to gt2FilteredReceive is delayed, it is the
//		 filter's responsibility
//		to make sure the data is still around when and if it is needed.
// Remarks
//		Call gt2FilteredRecieve with the filtered data, either from within
//		 the callback or later.<p>
//		The message may point to a memory location supplied to
//		 gt2FilteredReceive by a previous filter.  
//		So if this filter's call to gt2FilteredReceive is delayed, it is the
//		 filter's responsibility 
//		to make sure the data is still around when and if it is needed.
// See Also
//		gt2AddReceiveFilter, gt2RemoveReceiveFilter, gt2FilteredReceive
typedef void (* gt2ReceiveFilterCallback)
(
	GT2Connection connection,       // The connection the message was received on.
	int filterID,                   // Pass this ID to gtFilteredReceive.
	GT2Byte * message,              // The message that was received.  Will be NULL if an empty message.
	int len,                        // The length of the message in bytes.  Will be 0 if an empty message.
	GT2Bool reliable                // True if this is a reliable message.
);

/*********************
** FILTER FUNCTIONS **
*********************/

//////////////////////////////////////////////////////////////
// gt2AddSendFilter
// Summary
//		Adds a filter to the connection's outgoing data filter list.
// Parameters
//		connection	: [in] The handle to the connection.
//		callback	: [in] The filtering callback.
// Returns
//		Returns GT2False if there was an error adding the filter (due to no
//		 free memory).
// Remarks
//		The callback will get called when a message is being sent.
//		Callbacks will be called in the order they were added to the
//		 connection's filter list.<p>
// See Also
//		gt2SendFilterCallback, gt2RemoveSendFilter, gt2FilteredSend
GT2Bool gt2AddSendFilter
(
	GT2Connection connection,       // The connection on which to add the filter.
	gt2SendFilterCallback callback  // The callback the outgoing data is filtered through.
);

//////////////////////////////////////////////////////////////
// gt2RemoveSendFilter
// Summary
//		Removes a filter from the connection's outgoing data filter list.
// Parameters
//		connection	: [in] The handle to the connection.
//		callback	: [in] The filtering callback to remove.  NULL removes all filters.
// Remarks
//		Filters should not be removed while a message is being filtered.
//		If the callback is NULL, all of the filters will be removed.<p>
// See Also
//		gt2SendFilterCallback, gt2AddSendFilter, gt2FilteredSend
void gt2RemoveSendFilter
(
	GT2Connection connection,       // The connection on which to remove the filter.
	gt2SendFilterCallback callback  // The callback to remove.
);

//////////////////////////////////////////////////////////////
// gt2FilteredSend
// Summary
//		Called in response to a gt2SendFilterCallback being called.
//		It can be called from within the callback, or at any later time.
// Parameters
//		connection	: [in] The handle to the connection.
//		filterID	: [in] The ID passed to the gt2SendFilterCallback.
//		message	: [in] The message that was sent.  May be NULL.
//		len	: [in] The length of the message in bytes.  May be 0.
//		reliable	: [in] True if this is a reliable message.
// Remarks
//		Used to pass on a message after a filter callback has been called.
//		This will cause the message to either be passed to the next filter
//		 or, if this was the last filter, to be sent.
//		If this is called from the filter callback, the message passed in
//		 can be the same 
//		message that was passed into the callback.
//		Note that the 7 byte header must be accounted for in the message if
//		 the function sends the message reliably.<p>
// See Also
//		gt2SendFilterCallback, gt2AddSendFilter, gt2RemoveSendFilter
void gt2FilteredSend
(
	GT2Connection connection,  // The connection on which the message is being sent.
	int filterID,              // The ID passed to the gt2SendFilterCallback.
	const GT2Byte * message,   // The message being sent. May be NULL.
	int len,                   // The lengt2h of the message being sent, in bytes. May be 0 or -1.
	GT2Bool reliable           // If the message should be sent reliably.
);

//////////////////////////////////////////////////////////////
// gt2AddReceiveFilter
// Summary
//		Adds a filter to the connection's incoming data filter list.
// Parameters
//		connection	: [in] The handle to the connection.
//		callback	: [in] The filtering callback.
// Returns
//		Returns GT2False if there was an error adding the filter (due to no
//		 free memory).
// Remarks
//		The callback will get called when a message is being received.
//		Callbacks will be called in the order they were added to the
//		 connection's filter list.<p>
// See Also
//		gt2ReceiveFilterCallback, gt2RemoveReceiveFilter, gt2FilteredReceive
GT2Bool gt2AddReceiveFilter
(
	GT2Connection connection,          // The connection on which to add the filter.
	gt2ReceiveFilterCallback callback  // The callback the incoming data is filtered through.
);

//////////////////////////////////////////////////////////////
// gt2RemoveReceiveFilter
// Summary
//		Removes a filter from the connection's incoming data filter list.
// Parameters
//		connection	: [in] The handle to the connection.
//		callback	: [in] The filtering callback to remove.  NULL removes all filters.
// Remarks
//		Filters should not be removed while a message is being filtered.
//		If the callback is NULL, all of the filters will be removed.<p>
// See Also
//		gt2ReceiveFilterCallback, gt2AddReceiveFilter, gt2FilteredReceive
void gt2RemoveReceiveFilter
(
	GT2Connection connection,          // The connection on which to remove the filter.
	gt2ReceiveFilterCallback callback  // The callback to remove.
);

//////////////////////////////////////////////////////////////
// gt2FilteredReceive
// Summary
//		Called in response to a gt2ReceiveFilterCallback being called.
//		It can be called from within the callback, or at any later time.
// Parameters
//		connection	: [in] The handle to the connection.
//		filterID	: [in] The ID passed to the gt2ReceiveFilterCallback.
//		message		: [in] The message that was received.  May be NULL.
//		len			: [in] The length of the message in bytes.  May be 0.
//		reliable	: [in] True if this is a reliable message.
// Remarks
//		Used to pass on a message after a filter callback has been called.
//		This will cause the message to either be passed to the next filter
//		 or, if this was the last filter, 
//		to be received.
//		If this is called from the filter callback, the message passed in
//		 can be the same message that 
//		was passed into the callback.<p>
// See Also
//		gt2ReceiveFilterCallback, gt2AddReceiveFilter, gt2RemoveReceiveFilter
void gt2FilteredReceive
(
	GT2Connection connection,       // The connection the message was received on.
	int filterID,                   // The ID passed to the gt2ReceiveFilterCallback.
	GT2Byte * message,              // The message that was received.  May be NULL.
	int len,                        // The lengt2h of the message in bytes.  May be 0.
	GT2Bool reliable                // True if this is a reliable message.
);

/*****************************
** SOCKET SHARING CALLBACKS **
*****************************/

//////////////////////////////////////////////////////////////
// gt2UnrecognizedMessageCallback
// Summary
//		This callback gets called when the sock receives a message that it
//		 cannot match to an existing connection.
// Parameters
//		socket	: [in] The handle to the socket.
//		ip		: [in] The IP address of the remote machine the message
//		 came from (in network byte order).
//		port	: [in] The port on the remote machine (in host byte order).
//		message	: [in] The message (may be NULL for an empty message).
//		len		: [in] The length of the message (may be 0).
// Returns
//		GT2True if the callback recognizes the message and handles it. 
//		 GT2False if GT2 should handle the message.
// Remarks
//		If the callback recognizes the message and handles it, it should
//		 return GT2True, 
//		which will tell the socket to ignore the message.
//		If the callback does not recognize the message, it should return GT2False, 
//		which tells the socket to let the other side know there is no connection.<p>
// See Also
//		gt2SetUnrecognizedMessageCallback, gt2GetSocketSOCKET
typedef GT2Bool (* gt2UnrecognizedMessageCallback)
(
	GT2Socket socket,     // the socket the message was received on
	unsigned int ip,      // the ip of the remote machine the message came from (in network byte order)
	unsigned short port,  // the port on the remote machine (in host byte order)
	GT2Byte * message,    // the message (may be NULL for an empty message)
	int len               // the length of the message (may be 0)
);

/*****************************
** SOCKET SHARING FUNCTIONS **
*****************************/

//////////////////////////////////////////////////////////////
// gt2GetSocketSOCKET
// Summary
//		This function returns the actual underlying socket for a GT2Socket.
// Parameters
//		socket	: [in] The handle to the socket.
// Returns
//		The underlying socket associated with the GT2Socket.
// Remarks
//		This can be used for socket sharing purposes, along with the
//		 gt2UnrecognizedMessageCallback.<p>
// See Also
//		gt2SetUnrecognizedMessageCallback
SOCKET gt2GetSocketSOCKET(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2SetUnrecognizedMessageCallback
// Summary
//		Used to handle unrecognized messages, usually used for sharing a
//		 socket with another SDK.
// Parameters
//		socket		: [in] The handle to the socket.
//		callback	: [in] Function to be called when an unrecognized
//		 message is received.
// Remarks
//		This is used to set a callback to be called every time a socket
//		 receives a message that it cannot 
//		match up to an existing connection.
//		If a GT2Socket object's underlying socket is being shared, this
//		 allows an application to check for 
//		data that was not meant for GT2.
//		If the callback parameter is NULL, then any previously set callback
//		 will be removed.<p>
//		This is typically used when you are sharing a GT2Socket with another
//		 SDK, such as QR2 or NAT Negotiation. 
//		Setting an unrecognized callback allows you to pass messages meant
//		 for another SDK to the appropriate place.
// See Also
//		gt2UnrecognizedMessageCallback, gt2GetSocketSOCKET
void gt2SetUnrecognizedMessageCallback(GT2Socket socket, gt2UnrecognizedMessageCallback callback);

/*******************
** INFO FUNCTIONS **
*******************/

//////////////////////////////////////////////////////////////
// gt2GetConnectionSocket
// Summary
//		Returns the socket which this connection exists on.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The socket on which the connection was created or accepted.
// Remarks
//		All connections are created through either gt2Connect or
//		 gt2ConnectAttemptCallback.
//		This function will return the socket associated with the connection.<p>
// See Also
//		gt2Connect, gt2ConnectAttemptCallback
GT2Socket gt2GetConnectionSocket(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetConnectionState
// Summary
//		Gets the connection's state.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		GT2Connecting, GT2Connected, GT2Closing, or GT2Closed
// Remarks
//		A connection is either connecting, connected, closing, or closed.<p>
//		GT2Connecting - the connection is still being negotiated
//		GT2Connected - the connection is active (has successfully connected,
//		 and not yet closing)
//		GT2Closing - the connection is in the process of closing (sent a
//		 close message and waiting for confirmation)
//		GT2Closed - the connection has already been closed and will soon be freed.
GT2ConnectionState gt2GetConnectionState(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetRemoteIP
// Summary
//		Gets the connection's remote IP.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The remote IP in network byte order.
// Remarks
//		Gets the IP of the computer on the remote side of the connection.<p>
// See Also
//		gt2GetRemotePort, gt2GetLocalIP, gt2GetLocalPort
unsigned int gt2GetRemoteIP(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetRemotePort
// Summary
//		Get's the connection's remote port.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The remote port in host byte order.
// Remarks
//		Gets the port of the computer on the remote side of the connection.<p>
// See Also
//		gt2GetRemoteIP, gt2GetLocalIP, gt2GetLocalPort
unsigned short gt2GetRemotePort(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetLocalIP
// Summary
//		Gets a socket's local IP.
// Parameters
//		socket	: [in] The handle to the socket.
// Returns
//		The local IP in network byte order.
// See Also
//		gt2GetRemoteIP, gt2GetRemotePort, gt2GetLocalPort
unsigned int gt2GetLocalIP(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2GetLocalPort
// Summary
//		Get's a socket's local port.
// Parameters
//		socket	: [in] The handle to the socket.
// Returns
//		The local port in host byte order.
// See Also
//		gt2GetRemoteIP, gt2GetRemotePort, gt2GetLocalIP
unsigned short gt2GetLocalPort(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2GetIncomingBufferSize
// Summary
//		Gets the total size of the connection's incoming buffer.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The size in bytes of the connection's incoming buffer.
// See Also
//		gt2CreateSocket, gt2GetIncomingBufferFreeSpace,
//		 gt2GetOutgoingBufferSize, gt2GetOutgoingBufferFreeSpace
int gt2GetIncomingBufferSize(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetIncomingBufferFreeSpace
// Summary
//		Gets the amount of available space in the connection's incoming buffer.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The size in bytes of the free space in the connection's incoming buffer.
// See Also
//		gt2CreateSocket, gt2GetIncomingBufferSize, gt2GetOutgoingBufferSize,
//		 gt2GetOutgoingBufferFreeSpace
int gt2GetIncomingBufferFreeSpace(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetOutgoingBufferSize
// Summary
//		Gets the total size of the connection's outgoing buffer.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The size in bytes of the connection's outgoing buffer.
// See Also
//		gt2CreateSocket, gt2GetIncomingBufferSize,
//		 gt2GetIncomingBufferFreeSpace, gt2GetOutgoingBufferFreeSpace
int gt2GetOutgoingBufferSize(GT2Connection connection);

//////////////////////////////////////////////////////////////
// gt2GetOutgoingBufferFreeSpace
// Summary
//		Gets the amount of available space in the connection's outgoing buffer.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		The size in bytes of the free space in the connection's ougoing buffer.
// See Also
//		gt2CreateSocket, gt2GetIncomingBufferSize,
//		 gt2GetIncomingBufferFreeSpace, gt2GetOutgoingBufferSize
int gt2GetOutgoingBufferFreeSpace(GT2Connection connection);

/************************
** USER DATA FUNCTIONS **
************************/

//////////////////////////////////////////////////////////////
// gt2SetSocketData
// Summary
//		Stores a user data pointer with this socket.
// Parameters
//		socket	: [in] The handle to the socket.
//		data	: [in] A pointer to this socket's user data.
// Remarks
//		Each socket has a user data pointer associated with it that can be
//		 used by the application for any purpose.<p>
// See Also
//		gt2GetSocketData, gt2SetConnectionData, gt2GetConnectionData
void gt2SetSocketData(GT2Socket socket, void * data);

//////////////////////////////////////////////////////////////
// gt2GetSocketData
// Summary
//		Returns the user data pointer stored with this socket.
// Parameters
//		socket	: [in] The handle to the socket.
// Returns
//		A pointer to this socket's user data.
// Remarks
//		Each socket has a user data pointer associated with it that can be
//		 used by the application for any purpose.<p>
// See Also
//		gt2SetSocketData, gt2SetConnectionData, gt2GetConnectionData
void * gt2GetSocketData(GT2Socket socket);

//////////////////////////////////////////////////////////////
// gt2SetConnectionData
// Summary
//		Stores a user data pointer with this connection.
// Parameters
//		connection	: [in] The handle to the connection.
//		data	: [in] A pointer to this connection's user data.
// Remarks
//		Each connection has a user data pointer associated with it that can
//		 be used by the application for any purpose.<p>
// See Also
//		gt2SetSocketData, gt2GetSocketData, gt2GetConnectionData
void gt2SetConnectionData(GT2Connection connection, void * data);

//////////////////////////////////////////////////////////////
// gt2GetConnectionData
// Summary
//		Returns the user data pointer stored with this connection.
// Parameters
//		connection	: [in] The handle to the connection.
// Returns
//		A pointer to this connection's user data.
// Remarks
//		Each connection has a user data pointer associated with it that can
//		 be used by the application for any purpose.<p>
// See Also
//		gt2SetSocketData, gt2GetSocketData, gt2SetConnectionData
void * gt2GetConnectionData(GT2Connection connection);

/*************************
** BYTE ORDER FUNCTIONS **
*************************/

//////////////////////////////////////////////////////////////
// gt2NetworkToHostInt
// Summary
//		Convert an int from network to host byte order.
// Parameters
//		i	: [in] Int to convert.
// Returns
//		The int in host byte order.
// Remarks
//		This is a utility function to help deal with byte order differences
//		 for multi-platform applications.
//		Convert from host to network byte order before sending over the
//		 network, then convert back 
//		to host byte order when receiving.<p>
// See Also
//		gt2HostToNetworkInt, gt2NetworkToHostShort, gt2HostToNetworkShort
unsigned int gt2NetworkToHostInt(unsigned int i);

//////////////////////////////////////////////////////////////
// gt2HostToNetworkInt
// Summary
//		Convert an int from host to network byte order.
// Parameters
//		i	: [in] Int to convert.
// Returns
//		The int in network byte order.
// Remarks
//		This is a utility function to help deal with byte order differences
//		 for multi-platform applications.
//		Convert from host to network byte order before sending over the network, 
//		then convert back to host byte order when receiving.<p>
// See Also
//		gt2NetworkToHostInt, gt2NetworkToHostShort, gt2HostToNetworkShort
unsigned int gt2HostToNetworkInt(unsigned int i);

//////////////////////////////////////////////////////////////
// gt2HostToNetworkShort
// Summary
//		Convert a short from host to network byte order.
// Parameters
//		s	: [in] Short to convert.
// Returns
//		The short in network byte order.
// Remarks
//		This is a utility function to help deal with byte order differences
//		 for multi-platform applications.
//		Convert from host to network byte order before sending over the network, 
//		then convert back to host byte order when receiving.<p>
// See Also
//		gt2NetworkToHostInt, gt2HostToNetworkInt, gt2NetworkToHostShort
unsigned short gt2HostToNetworkShort(unsigned short s);

//////////////////////////////////////////////////////////////
// gt2NetworkToHostShort
// Summary
//		Convert a short from network to host byte order.
// Parameters
//		s	: [in] Short to convert.
// Returns
//		The short in host byte order.
// Remarks
//		This is a utility function to help deal with byte order differences
//		 for multi-platform applications.
//		Convert from host to network byte order before sending over the network, 
//		then convert back to host byte order when receiving.<p>
// See Also
//		gt2NetworkToHostInt, gt2HostToNetworkInt, gt2HostToNetworkShort
unsigned short gt2NetworkToHostShort(unsigned short s);

/**********************
** ADDRESS FUNCTIONS **
**********************/

//////////////////////////////////////////////////////////////
// gt2AddressToString
// Summary
//		Converts an IP and a port into a text string.
// Parameters
//		ip		: [in] IP in network byte order.  Can be 0.
//		port	: [in] Port in host byte order.  Can be 0.
//		string	: [out] String will be placed in here.  Can be NULL.
// Returns
//		The string is returned.  If the string paramater is NULL, then an
//		 internal static string will be used.  There are two internal strings
//		 that are alternated between.
// Remarks
//		The IP must be in network byte order, and the port in host byte order.
//		The string must be able to hold at least 22 characters (including
//		 the NUL).<p>
//		"XXX.XXX.XXX.XXX:XXXXX"
//		If both the IP and port are non-zero, the string will be of the form
//		 "1.2.3.4:5" ("<IP>:<port>").
//		If the port is zero, and the IP is non-zero, the string will be of
//		 the form "1.2.3.4" ("<IP>").
//		If the IP is zero, and the port is non-zero, the string will be of
//		 the form ":5" (":<port>").
//		If both the IP and port are zero, the string will be an empty string ("")
//		The string is returned.  If the string parameter is NULL, then an
//		 internal static string will be
//		used.  There are two internal strings that are alternated between.
// See Also
//		gt2StringToAddress
COMMON_API const char * gt2AddressToString
(
	unsigned int ip,                // IP in network byte order.  Can be 0.
	unsigned short port,            // Port in host byte order.  Can be 0.
	char string[22]                 // String will be placed in here.  Can be NULL.
);

//////////////////////////////////////////////////////////////
// gt2StringToAddress
// Summary
//		Converts a string address, which is either a hostname
//		 ("www.gamespy.net") or a dotted IP ("1.2.3.4")
//		into an IP and a port.
// Parameters
//		string	: [in] String to convert.
//		ip		: [out] IP in network byte order.  Can be NULL.
//		port	: [out] Port in host byte order.  Can be NULL.
// Returns
//		Returns GT2False if there was an error parsing the string, or if a
//		 supplied hostname can't be resolved.
// Remarks
//		The IP is stored in network byte order, and the port is stored in
//		 host byte order.<p>
//		Possible string forms:
//		NULL => all IPs, any port (localAddress only).
//		"" => all IPs, any port (localAddress only).
//		"1.2.3.4" => 1.2.3.4 IP, any port (localAddress only).
//		"host.com" => host.com's IP, any port (localAddress only).
//		":2786" => all IPs, 2786 port (localAddress only).
//		"1.2.3.4:0" => 1.2.3.4 IP, any port (localAddress only).
//		"host.com:0" => host.com's IP, any port (localAddress only).
//		"0.0.0.0:2786" => all IPs, 2786 port (localAddress only).
//		"1.2.3.4:2786" => 1.2.3.4 IP, 2786 port (localAddress or remoteAddress).
//		"host.com:2786" => host.com's IP, 2786 port (localAddress or remoteAddress).
//		If this function needs to resolve a hostname ("host.com") it may
//		 need to contact a DNS server, which can
//		cause the function to block for an indefinite period of time. 
//		 Usually it is < 2 seconds, but on certain
//		systems, and under certain circumstances, it can take 30 seconds or longer.
// See Also
//		gt2Connect, gt2CreateSocket, gt2StringToHostInfo
COMMON_API GT2Bool gt2StringToAddress
(
	const char * string,            // The string to convert.
	unsigned int * ip,              // The IP is stored here, in network byte order.  Can be NULL.
	unsigned short * port           // The port is stored here, in host byte order.  Can be NULL.
);

//////////////////////////////////////////////////////////////
// gt2IPToHostInfo
// Summary
//		Looks up DNS host information based on an IP.
// Parameters
//		ip		: [in] IP to look up, in network byte order.
//		aliases	: [out] On success, the variable passed in will point to
//		 a NULL-terminated list 
//						of alternate names for the host.  Can be NULL.
//		ips		: [out] On success, the variable passed in will point to
//		 a NULL-terminated list 
//						of alternate IPs for the host.  Can be NULL.
// Returns
//		The hostname associated with the IP, or NULL if no information was
//		 available for the host.
// Description
//		Gets the host information for a machine on the Internet.  The first
//		 version takes an IP in network byte order,
//		and the second version takes a string that is either a dotted ip
//		 ("1.2.3.4"), or a hostname ("www.gamespy.com").
//		If the function can successfully lookup the host's info, the host's
//		 main hostname will be returned.  If it
//		cannot find the host's info, it returns NULL.
//		For the aliases parameter, pass in a pointer to a variable of type
//		 (char **).  If this parameter is not NULL,
//		and the function succeeds, the variable will point to a
//		 NULL-terminated list of alternate names for the host.
//		For the ips parameter, pass in a pointer to a variable of type (int
//		 **).  If this parameter is not NULL, and
//		the function succeeds, the variable will point to a NULL-terminated
//		 list of altername IPs for the host.  Each
//		element in the list is actually a pointer to an unsigned int, which
//		 is an IP address in network byte order.
//		The return value, aliases, and IPs all point to an internal data
//		 structure, and none of these values should
//		be modified directly.  Also, the data is only valid until another
//		 call needs to use the same data structure
//		(virtually ever internet address function will use this data
//		 structure). If the data will be needed in the
//		future, it should be copied off.
//		If this function needs to resolve a hostname ("host.com") it may
//		 need to contact a DNS server, which can
//		cause the function to block for an indefinite period of time. 
//		 Usually it is < 2 seconds, but on certain
//		systems, and under certain circumstances, it can take 30 seconds or longer.
// Remarks
//		If the function can successfully lookup the host's info, the host's
//		 main hostname will be returned.
//		If it cannot find the host's info, it returns NULL.<p>
//		For the aliases parameter, pass in a pointer to a variable of type
//		 (char **). If this parameter is not NULL, 
//		and the function succeeds, the variable will point to a
//		 NULL-terminated list of alternate names for the host.
//		For the ips parameter, pass in a pointer to a variable of type (int
//		 **). If this parameter is not NULL, 
//		and the function succeeds, the variable will point to a
//		 NULL-terminated list of altername IPs for the host. 
//		Each element in the list is actually a pointer to an unsigned int,
//		 which is an IP address in network byte order.
//		The return value, aliases, and IPs all point to an internal data
//		 structure, and none of these values should 
//		be modified directly. Also, the data is only valid until another
//		 call needs to use the same data structure 
//		(virtually ever internet address function will use this data
//		 structure). If the data will be 
//		needed in the future, it should be copied off.
//		This function may need to contact a DNS server, which can cause the
//		 function to block for an 
//		indefinite period of time. Usually it is < 2 seconds, but on certain
//		 systems, and under certain circumstances, 
//		it can take 30 seconds or longer.
// See Also
//		gt2StringToHostInfo, gt2IPToHostname, gt2IPToAliases, gt2IPToIPs
const char * gt2IPToHostInfo(unsigned int ip, char *** aliases, unsigned int *** ips);

//////////////////////////////////////////////////////////////
// gt2StringToHostInfo
// Summary
//		Looks up DNS host information based on a hostname or dotted IP.
// Parameters
//		string	: [in] Hostname ("www.gamespy.net") or dotted IP
//		 ("1.2.3.4") to lookup.
//		aliases	: [in] On success, the variable passed in will point to a
//		 NULL-terminated 
//						list of alternate names for the host.  Can be NULL.
//		ips		: [in] On success, the variable passed in will point to a
//		 NULL-terminated 
//						list of alternate IPs for the host.  Can be NULL.
// Returns
//		The hostname associated with the string, or NULL if no information
//		 was available for the host.
// Remarks
//		If the function can successfully lookup the host's info, the host's
//		 main hostname will be returned.
//		If it cannot find the host's info, it returns NULL.<p>
//		For the aliases parameter, pass in a pointer to a variable of type
//		 (char **). If this parameter is not NULL, 
//		and the function succeeds, the variable will point to a
//		 NULL-terminated list of alternate names for the host.
//		For the ips parameter, pass in a pointer to a variable of type (int
//		 **). If this parameter is not NULL, 
//		and the function succeeds, the variable will point to a
//		 NULL-terminated list of altername IPs for the host. 
//		Each element in the list is actually a pointer to an unsigned int,
//		 which is an IP address in network byte order.
//		The return value, aliases, and IPs all point to an internal data
//		 structure, and none of these values should 
//		be modified directly. Also, the data is only valid until another
//		 call needs to use the same data structure 
//		(virtually ever internet address function will use this data
//		 structure). If the data will be needed in 
//		the future, it should be copied off.
//		This function may need to contact a DNS server, which can cause the
//		 function to block for an indefinite period 
//		of time. Usually it is < 2 seconds, but on certain systems, and
//		 under certain circumstances, 
//		it can take 30 seconds or longer.
// See Also
//		gt2IPToHostInfo, gt2StringToHostname, gt2StringToAliases, gt2StringToIPs
const char * gt2StringToHostInfo(const char * string, char *** aliases, unsigned int *** ips);

// The following functions are shortcuts for the above two functions
//		 (gt2*ToHostInfo()), and each performs a subset
// of the functionality.  They are provided so that code that only needs
//		 certain information can be a little simpler.
// Before using these, read the comments for the gt2*ToHostInfo() functions,
//		 as the info also applies to these functions.

//////////////////////////////////////////////////////////////
// gt2IPToHostname
// Summary
//		Get the hostname associated with an IP address.
// Parameters
//		ip	: [in] IP to lookup, in network byte order.
// Returns
//		Hostname associated with the IP address.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2IPToHostInfo.
//		See the gt2IPToHostInfo documentation for important details.<p>
// See Also
//		gt2IPToHostInfo
const char * gt2IPToHostname(unsigned int ip);

//////////////////////////////////////////////////////////////
// gt2StringToHostname
// Summary
//		Get the hostname associated with a hostname or dotted IP.
// Parameters
//		string	: [in] Hostname or IP to lookup.
// Returns
//		Hostname associated with a hostname or dotted IP.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2StringToHostInfo.
//		See the gt2StringToHostInfo documentation for important details.<p>
// See Also
//		gt2StringToHostInfo
const char * gt2StringToHostname(const char * string);

//////////////////////////////////////////////////////////////
// gt2IPToAliases
// Summary
//		Get the aliases associated with an IP address.
// Parameters
//		ip	: [in] IP to lookup, in network byte order.
// Returns
//		Aliases associated with the IP address.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2IPToHostInfo.
//		See the gt2IPToHostInfo documentation for important details.<p>
// See Also
//		gt2IPToHostInfo
char ** gt2IPToAliases(unsigned int ip);

//////////////////////////////////////////////////////////////
// gt2StringToAliases
// Summary
//		Get the aliases associated with a hostname or dotted IP.
// Parameters
//		string	: [in] Hostname or IP to lookup.
// Returns
//		Aliases associated with a hostname or dotted IP.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2StringToHostInfo.
//		See the gt2StringToHostInfo documentation for important details.<p>
// See Also
//		gt2StringToHostInfo
char ** gt2StringToAliases(const char * string);

//////////////////////////////////////////////////////////////
// gt2IPToIPs
// Summary
//		Get the IPs associated with an IP address.
// Parameters
//		ip	: [in] IP to lookup, in network byte order.
// Returns
//		IPs associated with the IP address.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2IPToHostInfo.
//		See the gt2IPToHostInfo documentation for important details.<p>
// See Also
//		gt2IPToHostInfo
unsigned int ** gt2IPToIPs(unsigned int ip);

//////////////////////////////////////////////////////////////
// gt2StringToIPs
// Summary
//		Get the IPs associated with a hostname or dotted IP.
// Parameters
//		string	: [in] Hostname or IP to lookup.
// Returns
//		IPs associated with a hostname or dotted IP.
// Remarks
//		This is a utility function which provides a subset of the
//		 functionality of gt2StringToHostInfo.
//		See the gt2StringToHostInfo documentation for important details.<p>
// See Also
//		gt2StringToHostInfo
unsigned int ** gt2StringToIPs(const char * string);

#ifdef _XBOX
COMMON_API unsigned int gt2XnAddrToIP(XNADDR theAddr, XNKID theKeyId);
COMMON_API GT2Bool gt2IPToXnAddr(int ip, XNADDR *theAddr, XNKID *theKeyId);
#endif

// these are for getting around adhoc which requires a 48 bit address v.s. a
//		 32 bit inet address
COMMON_API void gt2IpToMac(gsi_u32 ip,char *mac);
// change IP address to mac ethernet
COMMON_API gsi_u32 gt2MacToIp(const char *mac);
// change mac ethernet to IP address

/*******************
** DUMP CALLBACKS **
*******************/

//////////////////////////////////////////////////////////////
// gt2DumpCallback
// Summary
//		Called whenever data is sent or received over a socket.
// Parameters
//		socket		: [in] The handle to the socket.
//		connection	: [in] The handle to the connection associated with
//		 this message, or NULL if there 
//							is no connection for this message.
//		ip			: [in] The remote IP address, in network byte order.
//		port		: [in] The remote host, in host byte order.
//		reset		: [in] If true, the connection has been reset (only
//		 used by the receive callback).
//		message		: [in] The message (should not be modified).
//		len			: [in] The length of the message.
// Remarks
//		Trying to send a message from within the send dump callback, or
//		 letting the socket think from 
//		within the receive dump callback can cause serious problems, and
//		 should not be done.<p>
// See Also
//		gt2SetSendDump, gt2SetReceiveDump
typedef void (* gt2DumpCallback)
(
	GT2Socket socket,          // the socket the message was on
	GT2Connection connection,  // the connection the message was on, or NULL if there is no connection for this message
	unsigned int ip,           // the remote ip, in network byte order
	unsigned short port,       // the remote port, in host byte order
	GT2Bool reset,             // if true, the connection has been reset (only used by the receive callback)
	const GT2Byte * message,   // the message (should not be modified)
	int len                    // the length of the message
);

/*******************
** DUMP FUNCTIONS **
*******************/

//////////////////////////////////////////////////////////////
// gt2SetSendDump
// Summary
//		Sets a callback to which all outgoing UDP packets are passed.
//		This is at a lower level than the filters, can only be used for monitoring, 
//		and is designed for debugging purposes.
// Parameters
//		socket		: [in] The handle to the socket.
//		callback	: [in] The dump callback to set.
// Remarks
//		Sets a callback to be called whenever a UDP datagram is sent.
//		Pass in a callback of NULL to remove the callback.
//		The dump sit at a lower level than the filters, and allow an app to
//		 keep an eye on exactly 
//		what datagrams are being sent, allowing for close monitoring.
//		The dump cannot be used to modify data, only monitor it.
//		The dump is useful for debugging purposes, and to keep track of data
//		 send rates 
//		(e.g., the Quake 3 engine's netgraph). Note that these are the
//		 actual UDP datagrams being sent - 
//		datagrams may be dropped, repeated, or out-of-order.
//		Control datagrams (those used internally by the protocol) will be
//		 passed to the dump callback, 
//		and certain application messages will have a header at the beginning.<p>
// See Also
//		gt2DumpCallback, gt2SetReceiveDump
void gt2SetSendDump(GT2Socket socket, gt2DumpCallback callback);

//////////////////////////////////////////////////////////////
// gt2SetReceiveDump
// Summary
//		Sets a callback to which all incoming UDP packets are passed.
//		This is at a lower level than the filters, can only be used for
//		 monitoring, and is designed for debugging purposes.
// Parameters
//		socket		: [in] The handle to the socket.
//		callback	: [in] The dump callback to set.
// Remarks
//		Sets a callback to be called whenever a UDP datagram or a connection
//		 reset is received.
//		Pass in a callback of NULL to remove the callback.
//		The dump sit at a lower level than the filters, and allow an app to
//		 keep an eye on exactly what datagrams 
//		are being received, allowing for close monitoring.
//		The dump cannot be used to modify data, only monitor it.
//		The dump is useful for debugging purposes, and to keep track of data
//		 receive rates 
//		(e.g., the Quake 3 engine's netgraph). Note that these are the
//		 actual UDP datagrams being received - 
//		datagrams may be dropped, repeated, or out-of-order.
//		Control datagrams (those used internally by the protocol) will be
//		 passed to the dump callback, 
//		and certain application messages will have a header at the beginning.<p>
// See Also
//		gt2DumpCallback, gt2SetSendDump
void gt2SetReceiveDump(GT2Socket socket, gt2DumpCallback callback);



#ifdef __cplusplus
}
#endif

#endif
