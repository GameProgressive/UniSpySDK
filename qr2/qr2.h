///////////////////////////////////////////////////////////////////////////////
// File:	qr2.h
// SDK:		GameSpy Query and Reporting 2 (QR2) SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _QR2_H_
#define _QR2_H_

#include "../common/gsCommon.h"

// qr2regkeys.h contains defines for all of the reserved keys currently
// available.
#include "qr2regkeys.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef GSI_UNICODE
#define qr2_init			qr2_initA
#define qr2_init_socket		qr2_init_socketA
#define qr2_parse_query		qr2_parse_queryA
#define qr2_buffer_add		qr2_buffer_addA
#else
#define qr2_init			qr2_initW
#define qr2_init_socket		qr2_init_socketW
#define qr2_parse_query		qr2_parse_queryW
#define qr2_buffer_add		qr2_buffer_addW
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

// There is no need to escape strings, more flexible querying, less bandwidth, 
// and no need to space check buffers.


//////////////////////////////////////////////////////////////
// qr2_error_t
// Summary
//		Constants returned from qr2_init and the error callback to signal an
//		 error condition.
typedef enum 
{e_qrnoerror,			// No error occurred.
e_qrwsockerror,			// A standard socket call failed (e.g., exhausted resources).
e_qrbinderror,			// The SDK was unable to find an available port on which to bind.
e_qrdnserror,			// A DNS lookup (for the master server) failed.
e_qrconnerror,			// The server is behind a NAT and does not support negotiation.
e_qrnochallengeerror,	// No challenge was received from the master. The common reasons for this error are: <br>
//					1. Not enabling the NatNegotiate flag in the peerSetTitle or qr2_init calls: this should be  
//					   PEERTrue (or 1 for QR2) for games that support NATs. Otherwise, the master server will assume  
//					   this server is not behind a NAT and can directly connect to it. <br> 
//					2. Calling qr2_buffer_add more than once on a particular key-value. <br>
//					3. Firewall or NAT configuration is blocking incoming traffic. You may need to open ports to allow
//					   communication (see related). <br> 
//					4. Using the same port for socket communications: shared socket implementations with qr2/peer 
//					   together. <br> 
//					5. The heartbeat packet has exceeded the max buffer size of 1400 bytes. Try abbreviating some of 
//					   the custom keys to fit within the 1400 byte buffer. The reason for this restriction is to 
//					   support as many routers as possible, as UDP packets beyond this data range are more inclined to
//					   be dropped. <br> 
//					6. "numplayers" or "maxplayers" being set to negative values. <br> 
//					7. Having 2 network adapters connected to an internal and external network, and the internal one 
//					   set as primary. <br>
e_qrnotauthenticated, // Did not use AuthService, deny service.
qr2_error_t_count
} qr2_error_t;


//////////////////////////////////////////////////////////////
// qr2_key_type
// Summary
//		Keytype indicates the type of keys being referenced: server, player, or
//		 team.
typedef enum 
{
	key_server,		// General information about the game in progress.
	key_player,		// Information about a specific player.
	key_team,		// Information about a specific team.
	key_type_count
} qr2_key_type;

/*********
NUM_PORTS_TO_TRY
----------------
This value is the maximum number of ports that will be scanned to find an open 
query port, starting from the value passed to qr2_init as the base port. 
Generally there is no reason to modify this value.
***********/
#define NUM_PORTS_TO_TRY 100


/*********
MAGIC VALUES
----------------
These values will be used at the start of all QR2 query packets. If you are 
processing query data on your game socket, you can use these bytes to 
determine if a packet should be forwarded to the QR2 SDK for processing.
***********/
#define QR_MAGIC_1 0xFE
#define QR_MAGIC_2 0xFD

/* The app can resolve the master server hostname for this game itself and 
store the IP here before calling qr2_init. For more information, please
visit our Developer Forums at poweredbygamespy.com.*/
extern char qr2_hostname[64];

/***********
qr2_t
----
This abstract type is used to instantiate multiple instances of the Query & 
Reporting SDK (for example, if you are running multiple servers in the same 
process).
For most games, you can ignore this value and pass NULL in to all functions
that require it. A single global instance will be used in this case.
************/
typedef struct qr2_implementation_s *qr2_t;

/***********
qr2_keybuffer_t
---------------
This structure is used to store a list of keys when enumerating available keys.
Use the qr2_keybuffer_add function to add keys to the list.
************/
typedef struct qr2_keybuffer_s *qr2_keybuffer_t;


/***********
qr2_buffer_t
------------
This structure stores data that will be sent back to a client in response to 
a query. Use the qr2_buffer_add functions to add data to the buffer in your 
callbacks.
************/
typedef struct qr2_buffer_s *qr2_buffer_t;

typedef struct qr2_ipverify_node_s *qr2_ipverify_node_t;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Callback Function Definitions
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// qr2_serverkeycallback_t
// Summary
//		One of the callbacks provided to qr2_init, called when a client
//		 requests information about a specific server key.
// Parameters
//		keyid		: [in] The key being requested.
//		index		: [in] The 0-based index of the player or team being requested.
//		outbuf		: [in] The destination buffer for the value information. Use 
//							qr2_buffer_add to report the value.
//		userdata	: [in] The same userdata that was passed into qr2_init.
// Remarks
//		If you don't have a value for the provided keyid, you should add an
//		 empty ("") string to the buffer.<p>
// See Also
//		qr2_init, qr2_buffer_add
typedef void (*qr2_serverkeycallback_t)(int keyid, qr2_buffer_t outbuf, void *userdata);

//////////////////////////////////////////////////////////////
// qr2_playerteamkeycallback_t
// Summary
//		One of the callbacks provided to qr2_init; called when a client
//		 requests information about a player key or a team key.
// Parameters
//		keyid		: [in] The key being requested.
//		index		: [in] The zero-based index of the player or team being 
//							requested.
//		outbuf		: [in] The destination buffer for the value information. 
//							Use qr2_buffer_add to report the value.
//		userdata	: [in] The same userdata that was passed into qr2_init. You 
//							can use this for an object or structure pointer if 
//							needed.
// Remarks
//		As a player key callback, this is called when a client requests
//		 information about a specific key for a specific player.<p>
//		As a team key callback, this is called when a client requests the
//		 value for a team key.<p>
//		If you don't have a value for the provided keyid, you should add an
//		 empty ("") string to the buffer.
// See Also
//		qr2_init, qr2_buffer_add
typedef void (*qr2_playerteamkeycallback_t)(int keyid, int index, qr2_buffer_t outbuf, void *userdata);	


//////////////////////////////////////////////////////////////
// qr2_keylistcallback_t
// Summary
//		One of the callbacks provided to qr2_init; called when the SDK needs
//		 to determine 
//		all of the keys you game has values for.
// Parameters
//		keytype		: [in] The type of keys being requested (server, player, team). 
//							You should only add keys of this type to the keybuffer.
//		keybuffer	: [in] The structure that holds the list of keys. Use
//		 qr2_keybuffer_add to add a key to the buffer.
//		userdata	: [in] The same userdata that was passed into qr2_init.
// See Also
//		qr2_init, qr2_keybuffer_add
typedef void (*qr2_keylistcallback_t)(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata);	


//////////////////////////////////////////////////////////////
// qr2_countcallback_t
// Summary
//		One of the callbacks provided to qr2_init; called when the SDK needs to 
//		get a count of player or teams on the server.
// Parameters
//		keytype		: [in] Indicates whether the player or team count is being 
//							requested (key_player or key_team)
//		userdata	: [in] The same userdata that was passed into qr2_init.
// Returns
//		The callback should return the count for either the player or team, as 
//		indicated.
// Remarks
//		If your game does not support teams, return 0 for the count of teams.<p>
// See Also
//		qr2_init
typedef int  (*qr2_countcallback_t)(qr2_key_type keytype, void *userdata);	

//////////////////////////////////////////////////////////////
// qr2_adderrorcallback_t
// Summary
//		This callback is provided to qr2_init; called in response to a message 
//		from the master server indicating a problem listing the server.
// Parameters
//		error		: [in] The code that can be used to determine the specific 
//							listing error.
//		errmsg		: [in] A human-readable error string returned from the 
//							master server.
//		userdata	: [in] The userdata that was passed into qr2_init.
// Remarks
//		The most common error that will be reported is if the master is unable 
//		to list the server due to a firewall or proxy<p>
//		These types of errors must be appropriate handled and relayed to the user.
// See Also
//		qr2_init
typedef void (*qr2_adderrorcallback_t)(qr2_error_t error, gsi_char *errmsg, void *userdata);	

//////////////////////////////////////////////////////////////
// qr2_natnegcallback_t
// Summary
//		This callback is set via qr2_register_natneg_callback; it is called 
//		when a NAT negotiation request is received.
// Parameters
//		cookie		: [in] The cookie associated with the NAT Negotiation 
//							request.
//		userdata	: [in] The userdata that was passed into qr2_init.
// See Also
//		qr2_init, qr2_register_natneg_callback
typedef void (*qr2_natnegcallback_t)(int cookie, void *userdata);	

//////////////////////////////////////////////////////////////
// qr2_clientmessagecallback_t
// Summary
//		This callback is set via qr2_register_clientmessage_callback; called
//		 when a client message is received.
// Parameters
//		data		: [in] The buffer containing the message
//		len			: [in] The length of the data buffer
//		userdata	: [in] The userdata that was passed into qr2_init.
// See Also
//		qr2_init, qr2_register_clientmessage_callback
typedef void (*qr2_clientmessagecallback_t)(gsi_char *data, int len, void *userdata);	

//////////////////////////////////////////////////////////////
// qr2_publicaddresscallback_t
// Summary
//		This callback is set via qr2_register_publicaddress_callback; called
//		 when the local client's public address is received.
// Parameters
//		ip			: [in] IP address in string form: xxx.xxx.xxx.xxx
//		port		: [in] Port number
//		userdata	: [in] The userdata that was passed into qr2_init.
// Remarks
//		The address is that of the externalmost NAT or firewall device, and 
//		is determined by the GameSpy master server during the qr2_init 
//		process.<p>
// See Also
//		qr2_init, qr2_register_publicaddress_callback
typedef void (*qr2_publicaddresscallback_t)(unsigned int ip, unsigned short port, void *userdata);

//////////////////////////////////////////////////////////////
// qr2_clientconnectedcallback_t
// Summary
//		This callback is set via qr2_register_clientconnected_callback; called 
//		when a client has connected to the server.
// Parameters
//		gameSocket	: [in] The socket on which the client connected.
//		remoteaddr	: [in] The client's address and port.
//		userdata	: [in] The userdata that was passed into qr2_init.
// See Also
//		qr2_init, qr2_register_clientconnected_callback
typedef void (*qr2_clientconnectedcallback_t)(SOCKET gamesocket, SOCKADDR_IN *remoteaddr, void *userdata);

//#if defined(QR2_IP_FILTER)
typedef void (*qr2_denyqr2responsetoipcallback_t)(void *userdata, unsigned int sender_ip, int * result);
//#endif //#if defined(QR2_IP_FILTER)
//////////////////////////////////////////////////////////////
// qr2_hostregisteredcallback_t
// Summary
//		This callback is set via qr2_register_hostregistered_callback; it is
//		called when the master server has registered the game host as 
//		available.
// Parameters
//		userdata	: [in] The userdata that was passed into qr2_init.
// See Also
//		qr2_register_hostregistered_callback
typedef void (*qr2_hostregisteredcallback_t)(void *userdata);

//////////////////////////////////////////////////////////////
// qr2_register_natneg_callback
// Summary
//		Sets the function that will be triggered when a NAT negotiation 
//		request is received.
// Parameters
//		qrec		: [in] QR2 SDK initialized with qr2_init.
//		nncallback	: [in] Function to be called when a nat negotiation 
//							request is received.
// See Also 
//      qr2_init, qr2_natnegcallback_t
COMMON_API void qr2_register_natneg_callback(qr2_t qrec, qr2_natnegcallback_t nncallback);

//////////////////////////////////////////////////////////////
// qr2_register_clientmessage_callback
// Summary
//		Sets the function that will be triggered when a client message is 
//		received.
// Parameters
//		qrec		: [in] QR2 SDK initialized with qr2_init.
//		cmcallback	: [in] Function to be called when a client message is 
//							received.
// Remarks
//		The qr2_register_clientmessage_callback function is used to set a
//		 function that will be triggered when a client message is received.<p>
// See Also
//		qr2_init, qr2_clientmessagecallback_t
COMMON_API void qr2_register_clientmessage_callback(qr2_t qrec, qr2_clientmessagecallback_t cmcallback);

//////////////////////////////////////////////////////////////
// qr2_register_publicaddress_callback
// Summary
//		Sets the function that will be triggered when the local client's public 
//		address is received.
// Parameters
//		qrec		: [in] QR2 SDK initialized with qr2_init.
//		pacallback	: [in] Function to be called when the local client's public 
//							address is received.
// See Also 
//      qr2_init, qr2_publicaddresscallback_t
//
COMMON_API void qr2_register_publicaddress_callback(qr2_t qrec, qr2_publicaddresscallback_t pacallback);

//////////////////////////////////////////////////////////////
// qr2_register_clientconnected_callback
// Summary
//		Sets the function that will be triggered when a client has connected.
// Parameters
//		qrec		: [in] QR2 SDK initialized with qr2_init.
//		cccallback	: [in] Function to be called when a client has connected.
// See Also
//		qr2_init, qr2_clientconnectedcallback_t
COMMON_API void qr2_register_clientconnected_callback(qr2_t qrec, qr2_clientconnectedcallback_t cccallback);

//////////////////////////////////////////////////////////////
// qr2_register_hostregistered_callback
// Summary
//		Sets the function that will be called when the master server has
//		 registered the game host as available for connections.
// Parameters
//		qrec		: [in] QR2 SDK initialized with qr2_init.
//		hrcallback	: [in] Function to be called when the game host has
//		 been registered.
// See Also
//		qr2_init, qr2_clientconnectedcallback_t
COMMON_API void qr2_register_hostregistered_callback(qr2_t qrec, qr2_hostregisteredcallback_t hrcallback);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// qr2_register_key
// Summary
//		Register a key with the qr2 SDK. This tells the SDK that the 
//		application will report values for this key.
// Parameters
//		keyid	: [in] Id of the key. See remarks.
//		key		: [in] Name of the key. Player keys should end in "_" (such as 
//					"score_") and team keys should end in "_t".
// Remarks
//		The qr2_register_key function tell the qr2 SDK that it should report 
//		values for the specified key.
//		Key IDs 0 through NUM_RESERVED_KEYS are reserved for common key names. 
//		Keys upward to MAX_REGISTERED_KEYS are available for custom use.<p>
//		All custom keys should be registered prior to calling qr2_init. 
//		Reserved keys are already registered and should not be passed to this 
//		function.
//		<p>
//		The names of player keys reported with this function must end in an "_"
//		character and team keys always end in a "_t".
COMMON_API void qr2_register_key(int keyid, const gsi_char *key);

//////////////////////////////////////////////////////////////
// qr2_init
// Summary
//		Initialize the Query and Reporting 2 SDK.
// Parameters
//		qrec						: [out] The initialized QR2 SDK object.
//		ip							: [in] Optional IP address to which to 
//											bind; useful for multi-homed 
//											machines. Usually pass NULL.
//		baseport					: [in] Port to accept queries on. See 
//											remarks.
//		gamename					: [in] The gamename, assigned by GameSpy.
//		secret_key					: [in] The secret key for the specified 
//											gamename, also assigned by GameSpy.
//		ispublic					: [in] Set to 1 for an Internet listed 
//											server, 0 for a LAN only server.
//		natnegotiate				: [in] Set to 1 to allow server to be 
//											listed if it's behind a NAT (e.g., 
//											set to 1 if you support NAT 
//											Negotiation).
//											NOTE: if you do not set this to 1 
//											and you are behind a NAT you will 
//											receive the following error message 
//											from the GameSpy Master	Server: 
//											"Unable to query the server. You 
//											may need to open port x for 
//											incoming traffic."
//		server_key_callback			: [in] Callback that is triggered when 
//											server keys are requested.
//		player_key_callback			: [in] Callback that is triggered when 
//											player keys are requested.
//		team_key_callback			: [in] Callback that is triggered when team 
//											keys are requested.
//		key_list_callback			: [in] Callback that is triggered when the 
//											key list is requested.
//		playerteam_count_callback	: [in] Callback that is triggered when the 
//											number of teams is requested.
//		adderror_callback			: [in] Callback that is triggered when 
//											there has been an error adding it 
//											to the list.
//		userdata					: [in] Pointer to user data. This is 
//											optional and will be passed 
//											unmodified to the callback 
//											functions.
// Returns
//		This function returns e_qrnoerrorfor a successful result. Otherwise a
//		 valid qr2_error_t is returned.
// Remarks
//		The qr2_init function initializes the qr2 SDK. The baseport parameter 
//		specifies which local port should be used to accept queries on. If this 
//		port is in use, the next port value will be tried. 
//		The qr2 SDK will try up to NUM_PORTS_TO_TRYports. (Currently set at
//		 100.).<p>
COMMON_API qr2_error_t qr2_init(/*[out]*/qr2_t *qrec, const gsi_char *ip, int baseport, const gsi_char *gamename, const gsi_char *secret_key,
	int ispublic, int natnegotiate,
	qr2_serverkeycallback_t server_key_callback,
	qr2_playerteamkeycallback_t player_key_callback,
	qr2_playerteamkeycallback_t team_key_callback,
	qr2_keylistcallback_t key_list_callback,
	qr2_countcallback_t playerteam_count_callback,
	qr2_adderrorcallback_t adderror_callback,
	void *userdata);

//////////////////////////////////////////////////////////////
// qr2_init_socket
// Summary
//		Initialize the Query and Reporting 2 SDK. Allows control over the
//		 qr2 socket object.
// Parameters
//		qrec						: [out] The initialized QR2 SDK object.
//		s							: [in] Socket to be used for query traffic. 
//											This socket must have already been 
//											initialized.
//		boundport					: [in] The port that the socket was bound 
//											to. Chosen by the developer.
//		gamename					: [in] The gamename, assigned by GameSpy.
//		secret_key					: [in] The secret key for the specified 
//											gamename, also assigned by GameSpy.
//		ispublic					: [in] Set to 1 for an internet listed 
//											server, 0 for a LAN only server.
//		natnegotiate				: [in] Set to 1 to allow server to be
//											listed if it's behind a NAT (e.g., 
//											set to 1 if you support NAT 
//											Negotiation). Note if you do not 
//											set this to 1 and are behind a NAT 
//											you will receive the following 
//											error message from the GameSpy 
//											Master Server: "Unable to query the 
//											server. You may need to open port 
//											x for incoming traffic."
//		server_key_callback			: [in] Callback that is triggered when 
//											server keys are requested.
//		player_key_callback			: [in] Callback that is triggered when 
//											player keys are requested.
//		team_key_callback			: [in] Callback that is triggered when 
//											team keys are requested.
//		key_list_callback			: [in] Callback that is triggered when 
//											the key list is requested.
//		playerteam_count_callback	: [in] Callback that is triggered when 
//											the number of teams is requested.
//		adderror_callback			: [in] Callback that is triggered when 
//											there has been an error adding it 
//											to the list.
//		userdata					: [in] Pointer to user data. This is
//											optional and will be passed 
//											unmodified to the callback 
//											functions.
// Returns
//		This function returns e_qrnoerrorfor a successful result. Otherwise a 
//		valid qr2_error_t is returned.
// Remarks
//		The qr2_init_socket function initializes the qr2 SDK. Instead of 
//		creating its own internal socket, the qr2 SDK will use the passed in 
//		socket for all traffic. The developer is responsible for receiving 
//		on this socket and passing received qr2 messages to qr2_parse_query.<p> 
//		This version of qr2_init allows the game to specify the UDP socket to 
//		use for sending heartbeats and query replies. This enables the game and 
//		the QR2 SDK to share a single UDP socket for all networking, which can 
//		make hosting games behind a NAT proxy possible (see the documentation 
//		for more information).
COMMON_API qr2_error_t qr2_init_socket(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const gsi_char *gamename, const gsi_char *secret_key,
	int ispublic, int natnegotiate,
	qr2_serverkeycallback_t server_key_callback,
	qr2_playerteamkeycallback_t player_key_callback,
	qr2_playerteamkeycallback_t team_key_callback,
	qr2_keylistcallback_t key_list_callback,
	qr2_countcallback_t playerteam_count_callback,
	qr2_adderrorcallback_t adderror_callback,
	void *userdata);

//////////////////////////////////////////////////////////////
// qr2_think
// Summary
//		Allow the qr2 SDK to continue processing. Server queries can only be 
//		processed during this call.
// Parameters
//		qrec	: [in] The initialized QR2 SDK.
// Remarks
//		The qr2_think function allows the qr2 SDK to continue processing. This 
//		processing includes responding to user queries and triggering local 
//		callbacks. If q2_think is not called often, server responses may be 
//		delayed thereby increasing perceived latency. We recommend that you 
//		call qr2_think as frequently as possible. (10-15ms is not unusual.).<p>
COMMON_API void qr2_think(qr2_t qrec);

//////////////////////////////////////////////////////////////
// qr2_parse_query
// Summary
//		When using the shared socket method with qr2_init_socket, use this 
//		function to pass qr2 messages to the qr2 SDK.
// Parameters
//		qrec	: [in] Initialize QR2 SDK initialized with qr2_init_socket.
//		query	: [in] The QR2 packet received on the socket. See remarks.
//		len		: [in] The length of the QR2 packet.
//		sender	: [in] The sender of the packet.
// Remarks
//		The qr2_parse_query function should be used in the shared socket 
//		implementation on qr2. In this implementation, the developer is 
//		responsible for creating and receiving on the socket, and forwarding 
//		qr2 messages to the SDK. The qr2 messages may be identified by the 
//		packet header. QR1 packets begin with a single backslash '\' character, 
//		QR2 packets	begin with the QR_MAGIC_1 character followed by the 
//		QR_MAGIC_2 character.<p>
COMMON_API void qr2_parse_query(qr2_t qrec, gsi_char *query, int len, SOCKADDR *sender);

//////////////////////////////////////////////////////////////
// qr2_send_statechanged
// Summary
//		Notify the GameSpy Master Server of a change in gamestate.
// Parameters
//		qrec	: [in] Initialized QR2 SDK object.
// Remarks
//		The qr2_send_statechanged function notifies the GameSpy backend of a 
//		change in game state. This call is typically reserved for major changes 
//		such as mapname or gametype. Only one statechange message may be sent 
//		per 10-second interval. If a statechange is requested within this 
//		timeframe, it will be automatically delayed until the 10-second 
//		interval has elapsed. Under no circumstances should you call this 
//		function on a regular timer.<p>
COMMON_API void qr2_send_statechanged(qr2_t qrec);

//////////////////////////////////////////////////////////////
// qr2_shutdown
// Summary
//		Frees memory allocated by the qr2 SDK. This includes freeing 
//		user-registered keys.
// Parameters
//		qrec	: [in] QR2 SDK initialized with qr2_init.
// Remarks
//		The qr2_shutdown function may be used to free memory allocated by the 
//		qr2 SDK. The qr2 SDK should not be used after this call. This call 
//		will cease server reporting and remove the server from the backend 
//		list.<p> If you pass in a qrec that was returned from qr_init, all 
//		resources associated with that qrec will be freed. If you passed NULL 
//		into qr_int, you can pass NULL in here as well.
COMMON_API void qr2_shutdown(qr2_t qrec);



//////////////////////////////////////////////////////////////
// qr2_keybuffer_add
// Summary
//		Add a key identifier to the qr2_keybuffer_t. This is used when
//		 enumerating the supported list of keys.
// Parameters
//		keybuffer	: [in] Buffer to which to append the key ID.
//		keyid		: [in] The ID of the supported key. Add one ID for each 
//							key supported.
// Remarks
//		The qr2_keybuffer_add function is used to when enumerating the locally 
//		supported list of keys. Add the appropriate id number for each key 
//		supported.<p>
COMMON_API gsi_bool qr2_keybuffer_add(qr2_keybuffer_t keybuffer, int keyid);

/*****************
QR2_BUFFER_ADD / ADD_INT
------------
These functions are used to add a key's value to the outgoing buffer when
requested in a callback function.
******************/

//////////////////////////////////////////////////////////////
// qr2_buffer_add
// Summary
//		Add a string or integer to the qr2 buffer. This is used when 
//		responding to a qr2 query callback.
// Parameters
//		outbuf	: [in] Buffer to which to add the value. This is obtained 
//						from the qr2callback.
//		value	: [in] String or integer value to append to the buffer.
// Return Value 
//      gsi_bool
// Remarks
//		The qr2_buffer_add function appends a string to the buffer. The 
//		qr2_buffer_add_int function appends an integer to the buffer. These 
//		buffers are used to construct responses to user queries and 
//		typically contain information pertaining to the game status.<p>
// See Also
//		qr2_buffer_add_int
COMMON_API gsi_bool qr2_buffer_add(qr2_buffer_t outbuf, const gsi_char *value);

//////////////////////////////////////////////////////////////
// qr2_buffer_add_int
// Summary
//		Add a string or integer to the qr2 buffer. This is used when 
//		responding to a qr2 query callback.
// Parameters
//		outbuf	: [in] Buffer to whicn to add the value. This is obtained from 
//						the qr2callback.
//		value	: [in] String or integer value to append to the buffer.
// Return Value 
//      gsi_bool
// Remarks
//		The qr2_buffer_add function appends a string to the buffer. The 
//		qr2_buffer_add_int function appends an integer to the buffer. These 
//		buffers are used to construct responses to user queries and typically 
//		contain information pertaining to the game status.<p>
// See Also
//		qr2_buffer_add
COMMON_API gsi_bool qr2_buffer_add_int(qr2_buffer_t outbuf, int value);


/* for CDKey SDK integration */
#define REQUEST_KEY_LEN 4
#define RECENT_CLIENT_MESSAGES_TO_TRACK 10
	typedef void (*cdkey_process_t)(char *buf, int len, SOCKADDR *fromaddr);

	/* ip verification / spoof prevention */
#define QR2_IPVERIFY_TIMEOUT        4000  // Timeout after 4 seconds round trip time.
#define QR2_IPVERIFY_ARRAY_SIZE     200   // Allowed outstanding queries in those 4 seconds.
#define QR2_IPVERIFY_MAXDUPLICATES  5     // Allow maximum of 5 requests per IP/PORT.
struct qr2_ipverify_info_s
{
	SOCKADDR_IN addr;      // addr = 0 when not in use.
	gsi_u32            challenge;
	gsi_time           createtime; 
};

struct qr2_implementation_s
{
	SOCKET hbsock;
	char gamename[64];
	char secret_key[64];
	char instance_key[REQUEST_KEY_LEN];
	qr2_serverkeycallback_t server_key_callback;
	qr2_playerteamkeycallback_t player_key_callback;
	qr2_playerteamkeycallback_t team_key_callback;
	qr2_keylistcallback_t key_list_callback;
	qr2_countcallback_t playerteam_count_callback;
	qr2_adderrorcallback_t adderror_callback;					
	qr2_natnegcallback_t nn_callback;
	qr2_clientmessagecallback_t cm_callback;
	qr2_publicaddresscallback_t pa_callback;
	qr2_clientconnectedcallback_t cc_callback;
	qr2_hostregisteredcallback_t hr_callback;
//#if defined(QR2_IP_FILTER)
	qr2_denyqr2responsetoipcallback_t denyresp2_ip_callback;
//#endif //#if defined(QR2_IP_FILTER)


	gsi_time lastheartbeat;
	gsi_time lastka;
	int userstatechangerequested;
	int listed_state;
	int ispublic;	 
	int qport;
	int read_socket;
	int nat_negotiate;
	SOCKADDR_IN hbaddr;
	cdkey_process_t cdkeyprocess;
	int client_message_keys[RECENT_CLIENT_MESSAGES_TO_TRACK];
	int cur_message_key;
	unsigned int publicip;
	unsigned short publicport;
	void *udata;

	gsi_u8 backendoptions; // Received from server inside challenge packet.
	struct qr2_ipverify_info_s ipverify[QR2_IPVERIFY_ARRAY_SIZE];
};

// These need to be defined, even in GSI_UNICODE MODE.
void qr2_parse_queryA(qr2_t qrec, char *query, int len, SOCKADDR *sender);
gsi_bool qr2_buffer_addA(qr2_buffer_t outbuf, const char *value);
qr2_error_t qr2_initA(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
	int ispublic, int natnegotiate,
	qr2_serverkeycallback_t server_key_callback,
	qr2_playerteamkeycallback_t player_key_callback,
	qr2_playerteamkeycallback_t team_key_callback,
	qr2_keylistcallback_t key_list_callback,
	qr2_countcallback_t playerteam_count_callback,
	qr2_adderrorcallback_t adderror_callback,
	void *userdata);
qr2_error_t qr2_init_socketA(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
	int ispublic, int natnegotiate,
	qr2_serverkeycallback_t server_key_callback,
	qr2_playerteamkeycallback_t player_key_callback,
	qr2_playerteamkeycallback_t team_key_callback,
	qr2_keylistcallback_t key_list_callback,
	qr2_countcallback_t playerteam_count_callback,
	qr2_adderrorcallback_t adderror_callback,
	void *userdata);


#ifdef __cplusplus
}
#endif


#endif
