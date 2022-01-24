///////////////////////////////////////////////////////////////////////////////
// File:	gt2Main.h
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GT2_MAIN_H_
#define _GT2_MAIN_H_

#include "gt2.h"
#include "../common/darray.h"
#include "../common/hashtable.h"
#include "gt2Auth.h"

/*************************
** CONFIGURABLE DEFINES **
*************************/

// These defines are internal to GT2 and are NOT guaranteed to persist from 
// version to version.


// If set, this will convert all big endian vars to little endian before 
// sending across the net, and on big endian machines, convert little 
// endian to big endian on recv.
//#define	_GT2_ENDIAN_CONVERT_ENABLE	// add this to your compiler pre-processor options

#if defined GSI_BIG_ENDIAN && defined _GT2_ENDIAN_CONVERT_ENABLE
	#define	_GT2_ENDIAN_CONVERT
#endif



// Any unreliable application message that starts with this magic string will 
// have extra overhead. The string can be changed to something that your 
// application will not use, or not use frequently.
// The only impact of this change will be to make your application incompatible 
// with other applications using either the original or another different magic 
// string. The string can consist of any number of characters, as long as 
// there is at least one character and the length define matches the string's 
// length.
#define GTI2_MAGIC_STRING         "\xFE\xFE"
#define GTI2_MAGIC_STRING_LEN     2

// The size of the buffer into which GT2 directly receives messages. This 
// buffer is declared on the stack, and as such can be fairly large on most 
// systems without having any impact. However, on some systems with small 
// stacks, this size can overflow the stack, in which case it should be 
// lowered.
// NOTE: This buffer size only needs to be slighty larger than the largest 
// message that will be sent ("slighty larger" due to overhead with reliable 
// messages, and unreliable messages starting with the magic string).
#if defined(_PS2) && defined(INSOCK)
	#define GTI2_STACK_RECV_BUFFER_SIZE  NETBUFSIZE		// Max for Insock. Otherwise SOCKET_ERROR.
#elif defined(_NITRO)
	#define GTI2_STACK_RECV_BUFFER_SIZE  1500
#elif defined (_XBOX)									// Xbox packets are 1304.
	#define GTI2_STACK_RECV_BUFFER_SIZE  4096			// when using VDP sockets, 2 bytes are used for data length.
#elif defined(_REVOLUTION)
	#define GTI2_STACK_RECV_BUFFER_SIZE  4096           // Must be multiples of 32bytes.
#else													
	#define GTI2_STACK_RECV_BUFFER_SIZE  65535
#endif

// A server will disconnect a client that does not successfully connect 
// within this time (in milliseconds). If the connectAttemptCallback has been 
// called, and GT2 is awaiting an accept/reject, the attempt will not be 
// timed-out (although the client may abort the attempt at any time).
#define GTI2_SERVER_TIMEOUT     (1 * 60 * 1000)
// The time (in milliseconds) GT2 waits between resending a message whose 
// delivery has not yet been confirmed.
#define GTI2_RESEND_TIME        1000
// The time (in milliseconds) GT2 waits after receiving a message it must 
// acknowledge before it actually sends the ack. This allows it to combine 
// acks, or include acks as part of other reliable messages it sends. If an ack 
// is pending, a new incoming message does not reset this timer.
#define GTI2_PENDING_ACK_TIME   100
// If GT2 does not send a message for this amount of time (in milliseconds), it 
// sends a keep-alive message.
#define GTI2_KEEP_ALIVE_TIME    (30 * 1000)
// If this is defined, it sets the percentage of sent datagrams to drop. This 
// is good for simulating what will happen on a high packet loss connection.
// #define GTI2_DROP_SEND_RATE     30
typedef enum
{
	GTI2UdpProtocol,			// UDP socket type for standard sockets.
	GTI2VdpProtocol		= 2,	// VDP socket type only used for Xbox VDP sockets.
	GTI2AdHocProtocol	= 3		// Socket type only used for PSP Adhoc sockets.
} GTI2ProtocolType;

// The Maximum offset of eiter UDP or VDP.
// Measured in bytes.
// Used as a buffer offset.
#define MAX_PROTOCOL_OFFSET 2

/**********
** TYPES **
**********/

typedef enum
{
	// Client-only states:
	GTI2AwaitingServerChallenge,  // Sent challenge, waiting for server's challenge.
	GTI2AwaitingAcceptance,       // Sent response, waiting for accept/reject from server.

	// Server-only states:
	GTI2AwaitingClientChallenge,  // Receiving challenge from a new client.
	GTI2AwaitingClientResponse,   // Sent challenge, waiting for client's response.
	GTI2AwaitingAcceptReject,     // Got client's response, waiting for app to accept/reject.

	// Post-negotiation states:
	GTI2Connected,                // Connected.
	GTI2Closing,                  // Sent a close message (GTI2Close or GTI2Reject), waiting for confirmation.
	GTI2Closed                    // Connection has been closed, free it as soon as possible.
} GTI2ConnectionState;

// Message types:
typedef enum
{
	// Reliable messages:
	// All start with '<magic-string> <type> <serial-number> <expected-serial-number>'.
	// The type is 1 byte, SN and ESN are 2 bytes each.
	GTI2MsgAppReliable,			// Reliable application message.
	GTI2MsgClientChallenge,		// Client's challenge to the server (initial connection request).
	GTI2MsgServerChallenge,		// Server's response to the client's challenge, and his challenge to the client.
	GTI2MsgClientResponse,		// Client's response to the server's challenge.
	GTI2MsgAccept,				// Server accepting client's connection attempt.
	GTI2MsgReject,				// Server rejecting client's connection attempt.
	GTI2MsgClose,				// Message indicating the connection is closing.
	GTI2MsgKeepAlive,			// Keep-alive used to help detect dropped connections.

	GTI2NumReliableMessages,

	// Unreliable messages:
	GTI2MsgAck = 100,			// Acknowledge receipt of reliable message(s).
	GTI2MsgNack,				// Alert sender to missing reliable message(s).
	GTI2MsgPing,				// Used to determine latency.
	GTI2MsgPong,				// A reply to a ping.
	GTI2MsgClosed				// Confirmation of connection closure (GTI2MsgClose or GTI2MsgReject); 
								// this is also sent in response to bad messages from unknown addresses.

	// Unreliable messages don't really have a message type, just the magic string repeated at the start.
} GTI2MessageType;

/***************
** STRUCTURES **
***************/

typedef struct GTI2Buffer
{
	GT2Byte * buffer;         // The buffer's bytes.
	int size;                 // Number of bytes in buffer.
	int len;                  // Length of actual data in buffer.
} GTI2Buffer;

typedef struct GTI2IncomingBufferMessage
{
	int start;						// The start of the message.
	int len;						// The length of the message.
	GTI2MessageType type;			// The type.
	unsigned short serialNumber;	// The serial number.
} GTI2IncomingBufferMessage;

typedef struct GTI2OutgoingBufferMessage
{
	int start;						// The start of the message.
	int len;						// The length of the message.
	unsigned short serialNumber;	// The serial number.
	gsi_time lastSend;				// The last time this message was sent.
} GTI2OutgoingBufferMessage;

typedef struct GTI2Socket
{
	SOCKET socket;				// The network socket used for all network communication.

	unsigned int ip;			// The ip this socket is bound to.
	unsigned short port;		// The port this socket is bound to.

	HashTable connections;		// The connections that are using this socket.
	DArray closedConnections;	// Connections that are closed no longer get a spot in the hash table.

	GT2Bool close;	// If true, a close was attempted inside a callback, and it should be closed as soon as possible.
	GT2Bool error;  // If true, there was a socket error using this socket.

	int callbackLevel;	// If >0, then we're inside a callback (or recursive callbacks).
	gt2ConnectAttemptCallback connectAttemptCallback;	// If set, callback used to handle incoming connection attempts.
	gt2SocketErrorCallback socketErrorCallback;			// If set, call this in case of an error.
	gt2DumpCallback sendDumpCallback;					// If set, gets called for every datagram sent.
	gt2DumpCallback receiveDumpCallback;				// If set, gets called for every datagram and connection reset received.
	gt2UnrecognizedMessageCallback unrecognizedMessageCallback;		// If set, gets called for all unrecognized messages.

	void * data;  // User data.

	int outgoingBufferSize;  // Per-connection buffer sizes.
	int incomingBufferSize;

	GTI2ProtocolType protocolType;  // Set to UDP or VDP protocol depending on the call to create socket.
									// Also used as an offset for VDP sockets.
	int protocolOffset;
	GT2Bool broadcastEnabled;  // Set to true if the socket has already been broadcast enabled.
} GTI2Socket;

typedef struct GTI2Connection
{
	// Ip and port uniquely identify this connection on this socket.
	unsigned int ip;		// The ip on the other side of this connection (network byte order).
	unsigned short port;	// The port on the other side of this connection (host byte order).

	GTI2Socket * socket;  // The parent socket.

	GTI2ConnectionState state;  // Connection state.

	GT2Bool initiated;  // If true, the local side of the connection initiated the connection (client).

	GT2Bool freeAtAcceptReject;  // If true, don't free the connection until accept/reject is called.

	GT2Result connectionResult;  // The result of the connect attempt.

	gsi_time startTime; // The time the connection was created.
	gsi_time timeout;	// The timeout value passed into gt2Connect.

	int callbackLevel;  // If >0, then we're inside a callback (or recursive callbacks).
	GT2ConnectionCallbacks callbacks;  // Connection callbacks.
	
	char * initialMessage;  // This is the initial message for the client.
	int initialMessageLen;  // The initial message length.

	void * data;  // User data.

	GTI2Buffer incomingBuffer;		// Buffer for incoming data.
	GTI2Buffer outgoingBuffer;		// Buffer for outgoing data.
	DArray incomingBufferMessages;  // Identifies incoming messages stored in the buffer.
	DArray outgoingBufferMessages;  // Identifies outgoing messages stored in the buffer.

	unsigned short serialNumber;			// Serial number of the next message to be sent out.
	unsigned short expectedSerialNumber;	// The next serial number we're expecting from the remote side.

	char response[GTI2_RESPONSE_LEN];  // After the challenge is sent during negotiation, this is the response we're expecting.

	gsi_time lastSend;			// The last time something was sent on this connection.
	gsi_time challengeTime;		// The time the challenge was sent.

	GT2Bool pendingAck;  // If true, there is an ack waiting to go out, either on its own or as part of a reliable message.

	gsi_time pendingAckTime;  // The time at which the pending ack was first set.
	
	DArray sendFilters;		// Filters that apply to outgoing data.
	DArray receiveFilters;  // Filters that apply to incoming data.

} GTI2Connection;

// Store last 32 ips in a ring buffer.
#define MAC_TABLE_SIZE 32	// Must be a power of 2.
typedef struct
{
	gsi_u32 ip;
	char	mac[6];
} GTI2MacEntry;

#ifdef GSI_ADHOC
static int lastmactableentry = 0;
static GTI2MacEntry MacTable[MAC_TABLE_SIZE];
#endif // GSI_ADHOC

#endif
