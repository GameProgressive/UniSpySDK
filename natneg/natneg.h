///////////////////////////////////////////////////////////////////////////////
// File:	natneg.h
// SDK:		GameSpy NAT Negotiation SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
//-------------------------------------
// Please see the GameSpy NAT Negotiation SDK documentation for more 
// information.

#ifndef _NATNEG_H_
#define _NATNEG_H_
#include "../common/gsCommon.h"
#include "NATify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
	NAT Negotiation Packet Magic Bytes
	These bytes will start each incoming packet that is part of the NAT Negotiation SDK.
	If you are sharing a game socket with the SDK, you can use these bytes to determine when to
	pass a packet to NNProcessData
*/
#define NATNEG_MAGIC_LEN 6
#define NN_MAGIC_0 0xFD
#define NN_MAGIC_1 0xFC
#define NN_MAGIC_2 0x1E
#define NN_MAGIC_3 0x66
#define NN_MAGIC_4 0x6A
#define NN_MAGIC_5 0xB2

// This external array contains all 6 magic bytes; you can use it with memcmp 
// to quickly check incoming packets for the bytes.
extern unsigned char NNMagicData[];

//////////////////////////////////////////////////////////////
// NegotiateState
// Summary
//		Possible states for the SDK. The two you will be notified for are 
//		ns_initack and ns_connectping.
// See Also
//		NegotiateProgressFunc
typedef enum 
{	
	ns_preinitsent, 
	ns_preinitack,		// When the NAT Negotiation server acknowledges your connection.
	ns_initsent,		// Initial connection request has been sent to the server (internal).
	ns_initack,			// The NAT Negotiation server has acknowledged your connection request.
	ns_connectping,		// Direct negotiation with the other client has started.
	ns_finished,		// The negotiation process has completed (internal).
	ns_canceled,		// The negotiation process has been canceled (internal).
	ns_reportsent,		// The negotiation result report has been sent to the server (internal).
	ns_reportack		// The NAT Negotiation server has acknowledged your result report (internal).
} NegotiateState;

//////////////////////////////////////////////////////////////
// NegotiateResult
// Summary
//		Possible results of the negotiation.
// See Also
//		NegotiateCompletedFunc
typedef enum 
{
	nr_success,			// Successful negotiation, other parameters can be used to continue 
	//						communications with the client.
	nr_deadbeatpartner,	// Partner did not register with the NAT Negotiation Server.
	nr_inittimeout,		// Unable to communicate with NAT Negotiation Server.
	nr_pingtimeout,		// Unable to communicate with partner.
	nr_unknownerror,	// The NAT Negotiation server indicated an unknown error condition.
	nr_noresult			// Initial negotiation status before a result is determined.
} NegotiateResult;

//////////////////////////////////////////////////////////////
// NegotiateError
// Summary
//		Possible error values that can be returned when starting a negotiation.
// See Also
//		NNBeginNegotiation
typedef enum 
{
	ne_noerror,			// No error.
	ne_allocerror,		// Memory allocation failed.
	ne_socketerror,		// Socket allocation failed.
	ne_dnserror			// DNS lookup failed.
} NegotiateError;


//////////////////////////////////////////////////////////////
// NegotiateProgressFunc
// Summary
//		The callback that gets executed from NNBeginNegotiation as 
//		negotiation is proceeding.
// Parameters
//		state		: [in] The state of the negotiation at the time of notification.
//		userdata	: [in] Data for your own use.
// Remarks
//		The two times you will get a progress notification is when the NAT 
//		Negotiation server acknowledges your connection request (ns_initack), 
//		and when the guessed port data has been received from the NAT 
//		Negotiation server and direct negotiation with the other client is in 
//		progress (ns_connectping).<p>
// See Also
//		NNBeginNegotiation
typedef void (*NegotiateProgressFunc)(NegotiateState state, void *userdata);

//////////////////////////////////////////////////////////////
// NegotiateCompletedFunc
// Summary
//		The callback that gets executed from NNBeginNegotiation when 
//		negotiation is complete.
// Parameters
//		result		: [in] Indicates the result of the negotiation attempt.
//		gamesocket	: [in] The socket you should use to continue communications 
//							with the client.
//		remoteaddr	: [in] The remote address and port you should use to 
//							communicate with the new client.
//		userdata	: [in] Data for your own use.
// Remarks
//		Once your completed function is called, you can begin sending data to 
//		the other client immediately using the socket and address provided.<p>
//		Possible values for the value of the result parameter are:
//		nr_success 
//		Successful negotiation, an open channel has now been established. 
//		nr_deadbeatpartner 
//		Partner did not register with the NAT Negotiation Server. 
//		nr_inittimeout 
//		Unable to communicate with NAT Negotiation Server.
//		nr_pingtimeout 
//		Unable to communicate directly with partner.
//		nr_unknownerror 
//		NAT Negotiation server indicated an unknown error condition.<p>
//		If you used NNBeginNegotiationWithSocket then the socket parameter will 
//		be the socket you passed in originally. Otherwise it will be a new 
//		socket allocated by the NAT Negotiation SDK.<p>
//		Make sure you copy the remoteaddr structure before the callback 
//		returns.<p>
//		This function needs to be handled properly to pass source code review.
// See Also
//		NNBeginNegotiation
typedef void( *NegotiateCompletedFunc)(
	NegotiateResult result, 
	SOCKET gamesocket, 
	SOCKADDR_IN *remoteaddr, 
	void *userdata
);

//////////////////////////////////////////////////////////////
// NatDetectionResultsFunc
// Summary
//		The callback that gets executed from NNStartNatDetection when the
//		 detection is complete.
// Parameters
//		success	: [in] If gsi_true the NAT detection was successful.
//		nat		: [in] When detection is successful, this contains the NAT 
//						device's properties.
// Remarks
//		Once your detection callback function is called, check the success 
//		 parameter.
//		If it is gsi_false, then the detection could not be completed and 
//		should be retried.
//		If it is gsi_true, then the nat parameter will contain the properties 
//		of the detected NAT device.<p>
// See Also
//		NNStartNatDetection, NAT
typedef void( *NatDetectionResultsFunc)(gsi_bool success, NAT nat);


//////////////////////////////////////////////////////////////
// NNBeginNegotiation
// Summary
//		Starts the negotiation process.
// Parameters
//		cookie				: [in] Shared cookie value that both players will 
//									use so that the NAT Negotiation Server can 
//									match them up.
//		clientindex			: [in] One client must use clientindex 0, the 
//									other must use clientindex 1.
//		progresscallback	: [in] Callback function that will be called as 
//									the state changes.
//		completedcallback	: [in] Callback function that will be called 
//									when negotiation is complete.
//		userdata			: [in] Pointer for your own use that will be 
//									passed into the callback functions.
// Returns
//		ne_noerror if successful; otherwise one of the ne_ error values. See 
//		Remarks for detail.
// Remarks
//		Possible errors that can be returned when starting a negotiation:<p>
//		ne_noerror: No error.
//		ne_allocerror: Memory allocation failed.
//		ne_socketerror: Socket allocation failed.
//		ne_dnserror: DNS lookup failed,<p>
//		This should only be performed when connecting to a server. It should 
//		not be used	during server browsing as it will create unnecessary load 
//		on the GameSpy NAT service.
// See Also
//		NNBeginNegotiationWithSocket, NegotiateCompletedFunc
COMMON_API NegotiateError NNBeginNegotiation(
	int cookie, 
	int clientindex, 
	NegotiateProgressFunc progresscallback, 
	NegotiateCompletedFunc completedcallback, 
	void *userdata
);

//////////////////////////////////////////////////////////////
// NNBeginNegotiationWithSocket
// Summary
//		Starts the negotiation process using the socket provided, which will
//		 be shared with the game.
// Parameters
//		gamesocket			: [in] The socket to be used to start the negotiation
//		cookie				: [in] Shared cookie value that both players will use so that the 
//								NAT Negotiation Server can match them up.
//		clientindex			: [in] One client must use clientindex 0, the 
//									other must use clientindex 1.
//		progresscallback	: [in] Callback function that will be called as 
//									the state changes.
//		completedcallback	: [in] Callback function that will be called 
//									when negotiation is complete.
//		userdata			: [in] Pointer for your own use that will be 
//									passed into the callback functions.
// Returns
//		Possible errors that can be returned when starting a negotiation:
// Remarks
//		Incoming traffic is not processed automatically; you will need to read 
//		the data off the socket and pass NN packets to NNProcessData.<p>
//		This should only be performed when connecting to a server. It should 
//		not be used	during server browsing as it will create unnecessary load 
//		on the GameSpy NAT service.
// See Also
//		NNBeginNegotiation
COMMON_API NegotiateError NNBeginNegotiationWithSocket(
	SOCKET gamesocket, 
	int cookie, 
	int clientindex, 
	NegotiateProgressFunc progresscallback, 
	NegotiateCompletedFunc completedcallback, 
	void *userdata
);

//DOM-IGNORE-BEGIN
//////////////////////////////////////////////////////////////
// NNInternalBeginNegotiationWithSocket
// Summary
//		Internal function that starts the negotiation process using the socket 
//		provided, which will be shared with the game. Use INVALID_SOCKET const 
//		to signify that the SDK should create a new socket to use for the 
//		negotiation.
// Parameters
//		gamesocket			: [in] The socket to be used to start the 
//									negotiation.
//		cookie				: [in] Shared cookie value that both players will 
//									use so that the NAT Negotiation Server can 
//									match them up.
//      useQueue            : [in] gsi_true to use ACE queuing functionality, 
//									gsi_false to ignore.
//		clientindex			: [in] One client must use clientindex 0, the 
//									other must use clientindex 1.
//		progresscallback	: [in] Callback function that will be called as 
//									the state changes.
//		completedcallback	: [in] Callback function that will be called 
//									when negotiation is complete.
//		userdata			: [in] Pointer for your own use that will be 
//									passed into the callback functions.
// Returns
//		Possible errors that can be returned when starting a negotiation:
// Remarks
//		Incoming traffic is not processed automatically; you will need to read 
//		the data off the socket and pass NN packets to NNProcessData.<p>
// See Also
//		NNInternalBeginNegotiation
NegotiateError NNInternalBeginNegotiationWithSocket(
	SOCKET gamesocket, 
	int cookie, 
	int clientindex, 
	gsi_bool useQueue,
	NegotiateProgressFunc progresscallback, 
	NegotiateCompletedFunc completedcallback, 
	void *userdata
);
//DOM-IGNORE-END

//////////////////////////////////////////////////////////////
// NNThink
// Summary
//		Processes any negotiation or NAT detection requests that are in 
//		progress.
// Remarks
//		After you've begun a negotiation and/or NAT detection, you need to call 
//		the NNThink function on regular intervals (recommended: 100ms) to 
//		process the connection. You may call NNThink when no negotiations are 
//		in progress as well; it will simply return immediately.<p>
COMMON_API void NNThink();


//////////////////////////////////////////////////////////////
// NNProcessData
// Summary
//		Processes data received from a shared socket.
// Parameters
//		data		: [in] The data packets read from the gamesocket.
//		len			: [in] Length of the data.
//		fromaddr	: [in] The address from which the data packets came.
// Remarks
//		When sharing a socket with the NAT Negotiation SDK, you must read 
//		incoming data and pass NN packets to NNProcessData.<p>
// See Also
//		NNBeginNegotiationWithSocket
COMMON_API void NNProcessData(
	char *data, 
	int len, 
	SOCKADDR_IN *fromaddr
);


//////////////////////////////////////////////////////////////
// NNCancel
// Summary
//		Cancels a NAT Negotiation request in progress.
// Parameters
//		cookie	: [in] The cookie associated with this negotiation.
COMMON_API void NNCancel(int cookie);


//////////////////////////////////////////////////////////////
// NNFreeNegotiateList
// Summary
//		De-allocates the memory used by for the negotiate list when you are 
//		done with NAT Negotiation.
// Remarks
//		Once you have finished negotiating, the internal SDK memory must be 
//		freed using NNFreeNegotiatorList. Any outstanding negotiations will be 
//		cancel them. Calling this will NOT close the game sockets -- you are 
//		free to continue game communications.
COMMON_API void NNFreeNegotiateList();


//////////////////////////////////////////////////////////////
// NNStartNatDetection
// Summary
//		Starts the NAT detection process.
// Parameters
//		resultscallback	: [in] Callback function that will be called when NAT 
//							detection is complete.
// Returns
//		ne_noerror if successful; otherwise one of the ne_ error values. See 
//		Remarks for detail.
// Remarks
//		Possible errors that can be returned when starting a negotiation:<p>
//		ne_noerror: No error.
//		ne_socketerror: Socket allocation failed.
//		ne_dnserror: DNS lookup failed.
// See Also
//		NatDetectionResultsFunc, NAT
COMMON_API NegotiateError NNStartNatDetection(NatDetectionResultsFunc resultscallback);


// This is used for over-riding the default negotiation hostnames and should 
// not be used normally.
extern char *Matchup3Hostname;
extern char *Matchup2Hostname;
extern char *Matchup1Hostname;

#ifdef __cplusplus
}
#endif

#endif
