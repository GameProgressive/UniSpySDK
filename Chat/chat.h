///////////////////////////////////////////////////////////////////////////////
// File:	chat.h
// SDK:		GameSpy Chat SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _CHAT_H_
#define _CHAT_H_

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../common/gsCommon.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Public SDK Interface
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Constants
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// User and channel message types.
//////////////////////////////////
#define CHAT_MESSAGE        0
#define CHAT_ACTION         1
#define CHAT_NOTICE         2
#define CHAT_UTM            3
#define CHAT_ATM            4

// User modes.
// PANTS|03.12.01 - These are now bitflags!
// Both CHAT_VOICE and CHAT_OP can be set at the same time.
///////////////////////////////////////////////////////////
#define CHAT_NORMAL         0
#define CHAT_VOICE          1
#define CHAT_OP             2

// Part reasons (see the chatUserParted callback).
//////////////////////////////////////////////////
#define CHAT_LEFT           0  // The user left the channel.
#define CHAT_QUIT           1  // The user quit the chat network.
#define CHAT_KICKED         2  // The user was kicked from the channel.
#define CHAT_KILLED         3  // The user was kicked off the chat network.

// Possible nick errors while connecting.
/////////////////////////////////////////
#define CHAT_NICK_OK                0
#define CHAT_IN_USE                 1
#define CHAT_INVALID                2
#define CHAT_UNIQUENICK_EXPIRED     3
#define CHAT_NO_UNIQUENICK          4
#define CHAT_INVALID_UNIQUENICK     5
#define CHAT_NICK_TOO_LONG          6

// Reasons why a connect attempt could fail.
////////////////////////////////////////////
#define CHAT_DISCONNECTED           0
#define CHAT_NICK_ERROR             1
#define CHAT_LOGIN_FAILED           2

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Enumerated Types  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// CHATBool
// Summary
//		Standard Boolean.
typedef enum { CHATFalse, CHATTrue } CHATBool;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Types
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// A CHAT object represents a client connection to a chat server.
/////////////////////////////////////////////////////////////////
typedef void * CHAT;

//////////////////////////////////////////////////////////////
// CHATChannelMode
// Summary
//		The mode settings of a chat channel.
typedef struct CHATChannelMode
{
	CHATBool InviteOnly;			// Channel is invite-only.
	CHATBool Private;				// Channel is private.
	CHATBool Secret;				// Channel is secret.
	CHATBool Moderated;				// Channel is moderated,.
	CHATBool NoExternalMessages;	// External messages to channel are not allowed.
	CHATBool OnlyOpsChangeTopic;	// Topic is limited; only chanops may change it.
	CHATBool OpsObeyChannelLimit;
	int Limit;						// The maximum number of  of users allowed.
} CHATChannelMode;

//////////////////////////////////////////////////////////////
// CHATEnterResult
// Summary
//		The result of a channel enter attempt, passed into the 
//		chatEnterChannelCallback().
typedef enum
{
	CHATEnterSuccess,        // The channel was successfully entered.
	CHATBadChannelName,      // The channel name was invalid.
	CHATChannelIsFull,       // The channel is at its user limit.
	CHATInviteOnlyChannel,   // The channel is invite only.
	CHATBannedFromChannel,   // The local user is banned from this channel.
	CHATBadChannelPassword,  // The channel has a password, and a bad password (or none) was given.
	CHATTooManyChannels,     // The server won't allow this user in any more channels.
	CHATEnterTimedOut,       // The attempt to enter timed out.
	CHATBadChannelMask       // Not sure if any servers use this, or what it means! (ERR_BADCHANMASK)
} CHATEnterResult;

typedef enum CHATOperationResult
{
	CHATOperationResult_SUCCESS,
	CHATOperationResult_INVALID_PARAM,
	CHATOperationResult_ENUM_ALREADY_PENDING,
	CHATOperationResult_MAX
} CHATOperationResult;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef GSI_UNICODE
#define chatConnect					chatConnectA
#define chatConnectSpecial			chatConnectSpecialA
#define chatConnectSecure			chatConnectSecureA
#define chatConnectLogin			chatConnectLoginA
#define chatConnectPreAuth			chatConnectPreAuthA
#define chatRetryWithNick			chatRetryWithNickA
#define chatRegisterUniqueNick		chatRegisterUniqueNickA
#define chatSendRaw					chatSendRawA
#define chatChangeNick				chatChangeNickA
#define chatFixNick					chatFixNickA
#define chatTranslateNick			chatTranslateNickA
#define chatAuthenticateCDKey		chatAuthenticateCDKeyA
#define chatEnumChannels			chatEnumChannelsA
#define chatEnumChannelsWithLimit   chatEnumChannelsWithLimitA
#define chatEnterChannel			chatEnterChannelA
#define chatLeaveChannel			chatLeaveChannelA
#define chatSendChannelMessage		chatSendChannelMessageA
#define chatSetChannelTopic			chatSetChannelTopicA
#define chatGetChannelTopic			chatGetChannelTopicA
#define chatSetChannelMode			chatSetChannelModeA
#define chatGetChannelMode			chatGetChannelModeA
#define chatSetChannelPassword		chatSetChannelPasswordA	
#define chatGetChannelPassword		chatGetChannelPasswordA		
#define chatSetChannelLimit			chatSetChannelLimitA	
#define chatEnumChannelBans			chatEnumChannelBansA
#define chatAddChannelBan			chatAddChannelBanA
#define chatRemoveChannelBan		chatRemoveChannelBanA
#define chatSetChannelGroup			chatSetChannelGroupA
#define chatGetChannelNumUsers		chatGetChannelNumUsersA
#define chatInChannel				chatInChannelA
#define chatEnumUsers				chatEnumUsersA
#define chatSendUserMessage			chatSendUserMessageA
#define chatGetUserInfo				chatGetUserInfoA
#define chatGetBasicUserInfo		chatGetBasicUserInfoA
#define chatGetBasicUserInfoNoWait	chatGetBasicUserInfoNoWaitA
#define chatGetChannelBasicUserInfo	chatGetChannelBasicUserInfoA
#define chatInviteUser				chatInviteUserA
#define chatKickUser				chatKickUserA
#define chatBanUser					chatBanUserA
#define chatSetUserMode				chatSetUserModeA
#define chatGetUserMode				chatGetUserModeA
#define chatGetUserModeNoWait		chatGetUserModeNoWaitA
#define chatSetGlobalKeys			chatSetGlobalKeysA
#define chatSetChannelKeys			chatSetChannelKeysA
#define chatGetGlobalKeys			chatGetGlobalKeysA
#define chatGetChannelKeys			chatGetChannelKeysA
#define chatGetNick					chatGetNickA
#define chatGetUdpRelay				chatGetUdpRelayA
#else
#define chatConnect					chatConnectW
#define chatConnectSpecial			chatConnectSpecialW
#define chatConnectSecure			chatConnectSecureW
#define chatConnectLogin			chatConnectLoginW
#define chatConnectPreAuth			chatConnectPreAuthW
#define chatRetryWithNick			chatRetryWithNickW
#define chatRegisterUniqueNick		chatRegisterUniqueNickW
#define chatSendRaw					chatSendRawW
#define chatChangeNick				chatChangeNickW
#define chatFixNick					chatFixNickW
#define chatTranslateNick			chatTranslateNickW
#define chatAuthenticateCDKey		chatAuthenticateCDKeyW
#define chatEnumChannels			chatEnumChannelsW
#define chatEnumChannelsWithLimit   chatEnumChannelsWithLimitW
#define chatEnterChannel			chatEnterChannelW
#define chatLeaveChannel			chatLeaveChannelW
#define chatSendChannelMessage		chatSendChannelMessageW
#define chatSetChannelTopic			chatSetChannelTopicW
#define chatGetChannelTopic			chatGetChannelTopicW
#define chatSetChannelMode			chatSetChannelModeW
#define chatGetChannelMode			chatGetChannelModeW
#define chatSetChannelPassword		chatSetChannelPasswordW
#define chatGetChannelPassword		chatGetChannelPasswordW
#define chatSetChannelLimit			chatSetChannelLimitW
#define chatEnumChannelBans			chatEnumChannelBansW
#define chatAddChannelBan			chatAddChannelBanW
#define chatRemoveChannelBan		chatRemoveChannelBanW
#define chatSetChannelGroup			chatSetChannelGroupW
#define chatGetChannelNumUsers		chatGetChannelNumUsersW
#define chatInChannel				chatInChannelW
#define chatEnumUsers				chatEnumUsersW
#define chatSendUserMessage			chatSendUserMessageW
#define chatGetUserInfo				chatGetUserInfoW
#define chatGetBasicUserInfo		chatGetBasicUserInfoW
#define chatGetBasicUserInfoNoWait	chatGetBasicUserInfoNoWaitW
#define chatGetChannelBasicUserInfo	chatGetChannelBasicUserInfoW
#define chatInviteUser				chatInviteUserW
#define chatKickUser				chatKickUserW
#define chatBanUser					chatBanUserW
#define chatSetUserMode				chatSetUserModeW
#define chatGetUserMode				chatGetUserModeW
#define chatGetUserModeNoWait		chatGetUserModeNoWaitW
#define chatSetGlobalKeys			chatSetGlobalKeysW
#define chatSetChannelKeys			chatSetChannelKeysW
#define chatGetGlobalKeys			chatGetGlobalKeysW
#define chatGetChannelKeys			chatGetChannelKeysW
#define chatGetNick					chatGetNickW
#define chatGetUdpRelay				chatGetUdpRelayW
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/**********************
** GLOBALS CALLBACKS **
**********************/

//////////////////////////////////////////////////////////////
// chatRaw
// Summary
//		Used in conjunction with the chatConnect functions;  
//		all raw incoming network traffic gets passed to this function.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		raw		: [in] The raw data.
//		param	: [in] Pointer to user data. The same param that was passed 
//						to chatConnect through the callbacks structure.
// See Also
//		chatConnect, chatSendRaw
typedef void (* chatRaw)(CHAT chat,
	const gsi_char * raw,
	void * param);

//////////////////////////////////////////////////////////////
// chatDisconnected
// Summary
//		Called when a disconnection occurs.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		reason	: [in] The text string which states the reason for disconnect
//		param	: [in] Pointer to user data. The same param that was passed to
//					chatConnect through the callback structure.
// Remarks
//		The chatDisconnected callback function is called after a
//		 disconnection occurs.
//		The connection can be ended at any time by called chatDisconnect().
//		If the connection gets disconnected for any other reason 
//		(such as an intermediate router going down), 
//		the chatDisconnected() callback will be called.<p>
typedef void (* chatDisconnected)(CHAT chat,
	const gsi_char * reason,
	void * param);

//////////////////////////////////////////////////////////////
// chatPrivateMessage
// Summary
//		Used in conjunction with the chatConnect functions;  
//		called when a message is received from another user.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		user	: [in] The user who sent the message.
//		message	: [in] The text of the message.
//		type	: [in] The type of message.
//		param	: [in] Pointer to user data. The same param that was passed to 
//						chatConnect through the callback structure.
// Remarks
//		If user is NULL, this is a message from the server.<p>
// See Also
//		chatConnect
typedef void (* chatPrivateMessage)(CHAT chat,
	const gsi_char * user,
	const gsi_char * message,
	int type,  // See defined message types above.
	void * param);

//////////////////////////////////////////////////////////////
// chatInvited
// Summary
//		Used in conjunction with the chatConnect functions; 
//		called when the local user gets invited to a channel.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel to which this user was invited.
//		user	: [in] The user who offered the invite.
//		param	: [in] Pointer to user data. The same param that was passed to
//						chatConnect through the callback structure.
// See Also
//		chatConnect, chatInviteUser
typedef void (* chatInvited)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	void * param);

//////////////////////////////////////////////////////////////
// chatGlobalCallbacks
// Summary
//		A connection's global callbacks.
typedef struct chatGlobalCallbacks
{
	chatRaw raw;						// Gets raw incoming network traffic.
	chatDisconnected disconnected;		// Called when the user has been disconnected.
	chatPrivateMessage privateMessage;	// Called when a private message from another user is received.
	chatInvited invited;				// Called when invited into a channel.
	void * param;						// A pointer to data that will be passed into each of the callbacks when triggered.
} chatGlobalCallbacks;

/**********************
** CHANNEL CALLBACKS **
**********************/

//////////////////////////////////////////////////////////////
// chatChannelMessage
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when a message is received in the channel.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the user who sent the message.
//		message	: [in] The text of the message.
//		type	: [in] The type of the message: 
//						one of the pre-defined chat types.
//		param	: [in] Pointer to user data. The same param that was passed 
//						to chatEnterChannel	through the callbacks structure.
// See Also
//		chatEnterChannel, chatSendChannelMessage
typedef void (* chatChannelMessage)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	const gsi_char * message,
	int type,  // See defined message types above.
	void * param);

//////////////////////////////////////////////////////////////
// chatKicked
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when the local user gets kicked from the channel.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the user being kicked from the channel.
//		reason	: [in] The same reason string sent into chatKickUser.
//		param	: [in] Pointer to user data. The same param that was passed to
//					chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatKickUser
typedef void (* chatKicked)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	const gsi_char * reason,
	void * param);

//////////////////////////////////////////////////////////////
// chatUserJoined
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when a user joins the channel.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the joining user.
//		mode	: [in] The joining user's mode.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel
typedef void (* chatUserJoined)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int mode,    // See defined user modes above.
	void * param);

//////////////////////////////////////////////////////////////
// chatUserParted
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when a user parts the channel.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the parting user.
//		why	: [in] Code indicating reason user parted.
//		reason	: [in] Explanation string.
//		kicker	: [in] If reason is "kicked", identifies the kicker.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatKickUser
typedef void (* chatUserParted)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int why,    // See defined part reasons above.
	const gsi_char * reason,
	const gsi_char * kicker,
	void * param);

//////////////////////////////////////////////////////////////
// chatUserChangedNick
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when a user in the channel changes their nickname.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		oldNick	: [in] The old nickname of the user.
//		newNick	: [in] The new nickname.
//		param	: [in] Pointer to user data. The same param that was passed
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatChangeNick
typedef void (* chatUserChangedNick)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * oldNick,
	const gsi_char * newNick,
	void * param);

//////////////////////////////////////////////////////////////
// chatTopicChanged
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when the channel topic changes.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		topic	: [in] The new topic (description) of the channel.
//		param	: [in] Pointer to user data. The same param that was passed
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatSetChannelTopic
typedef void (* chatTopicChanged)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * topic,
	void * param);

//////////////////////////////////////////////////////////////
// chatChannelModeChanged
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when the mode of a user in the channel changes.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		mode	: [in] Properties of the new mode set on the channel.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatSetChannelMode
typedef void (* chatChannelModeChanged)(CHAT chat,
	const gsi_char * channel,
	CHATChannelMode * mode,
	void * param);

//////////////////////////////////////////////////////////////
// chatUserModeChanged
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when the mode of a user in the channel changes.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the user whose mode changed.
//		mode	: [in] The new mode of the user.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel, chatSetUserMode
typedef void (* chatUserModeChanged)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int mode,  // See defined user modes above.
	void * param);

//////////////////////////////////////////////////////////////
// chatUserListUpdated
// Summary
//		Used in conjunction with chatEnterChannel; 
//		called when the channel's user list changes.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
// See Also
//		chatEnterChannel
typedef void (* chatUserListUpdated)(CHAT chat,
	const gsi_char * channel,
	void * param);

//////////////////////////////////////////////////////////////
// chatNewUserList
// Summary
//		Used in conjunction with chatEnterChannel; Called when the chat server
//		sends an entire new user list for a channel we're in.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		num		: [in] The number of users in the list.
//		users	: [in] List of users.
//		modes	: [in] List of user modes.
//		param	: [in] Pointer to user data. The same param that was passed 
//					to chatEnterChannel through the callbacks structure.
typedef void (* chatNewUserList)(CHAT chat,
	const gsi_char * channel,
	int num,
	const gsi_char ** users,
	int * modes,
	void * param);

//////////////////////////////////////////////////////////////
// chatBroadcastKeyChanged
// Summary
//		Called when a player changes a broadcast key
//		in a channel the local player is in.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		channel	: [in] The channel the local player is in.
//		user	: [in] The nickname of the user who changed the key
//		key	: [in] The broadcast key that was changed
//		value	: [in] The broadcast key value
//		param	: [in] Pointer to user data. The same param that was passed
//					to chatEnterChannel through the callbacks structure.
// Remarks
//		The chatBroadcastKeyChanged function is called when another 
//		player changes a broadcast key in the channel the local player is in.<p>
typedef void (* chatBroadcastKeyChanged)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	const gsi_char * key,
	const gsi_char * value,
	void * param);

//////////////////////////////////////////////////////////////
// chatChannelCallbacks
// Summary
//		A channel's callbacks.
typedef struct chatChannelCallbacks
{
	chatChannelMessage channelMessage;				// Called when a message is received in a channel.
	chatKicked kicked;								// Called when the local user is kicked from a channel.
	chatUserJoined userJoined;						// Called when a user joins a channel we're in.
	chatUserParted userParted;						// Called when a user parts a channel we're in.
	chatUserChangedNick userChangedNick;			// Called when a user in a channel we're in changes nicks.
	chatTopicChanged topicChanged;					// Called when the topic changes in a channel we're in.
	chatChannelModeChanged channelModeChanged;		// Called when the mode changes in a channel we're in.
	chatUserModeChanged userModeChanged;			// Called when a user's mode changes in a channel we're in.
	chatUserListUpdated userListUpdated;			// Called when the user list changes (due to a join or a part)
	//													in a channel we're in.
	chatNewUserList newUserList;					// Called when the chat server sends an entire new user 
	//													list for a channel we're in.
	chatBroadcastKeyChanged broadcastKeyChanged;	// Called when a user changes a broadcast key in a channel we're in.
	void * param;									// A pointer to data that will be passed into each of the 
	//													callbacks when triggered.
} chatChannelCallbacks;

/************
** GENERAL **
************/

//////////////////////////////////////////////////////////////
// chatConnectCallback
// Summary
//		Called when a chatConnect* attempt is made.
// Parameters
//		chat			: [in] The initialized chat interface object.
//		success			: [in] CHATTrue if success, CHATFalse if failure.
//		failureReason	: [in] The string giving reason for failure
//		param			: [in] Pointer to user data. Passed through
//		 unmodified from the initiating function.
// Remarks
//		The chatConnectCallback is called after an attempt of a call to one 
//		of the connect functions that the Chat SDK provides.<p>
typedef void (* chatConnectCallback)(CHAT chat,
	CHATBool success,
	int failureReason,  // CHAT_DISCONNECTED, CHAT_NICK_ERROR, etc.
	void * param);

//////////////////////////////////////////////////////////////
// chatNickErrorCallback
// Summary
//		Used in conjunction with the chatConnect functions; called if there
//		 was an error with the provided nickname.
// Parameters
//		chat				: [in] The initialized chat interface object.
//		type				: [in] The type of error: indicates whether the
//		 nick was invalid or if it was in use.
//		nick				: [in] The problematic nickname.
//		numSuggestedNicks	: [in] The number of suggested nicknames.
//		suggestedNicks		: [in] A list of suggested alternative nicknames.
//		param				: [in] Pointer to user data. The same param
//		 that was passed to chatConnect.
// Remarks
//		To retry with a new nick, call chatRetryWithNick.
//		Otherwise, call chatDisconnect to stop the connection.
//		Suggested nicks are only provided if type is CHAT_INVALID_UNIQUENICK.<p>
// See Also
//		chatConnect, chatRetryWithNick, chatRegisterUniqueNick, chatDisconnect
typedef void (* chatNickErrorCallback)(CHAT chat,
	int type,  // CHAT_IN_USE, CHAT_INVALID, etc.
	const gsi_char * nick,
	int numSuggestedNicks,
	const gsi_char ** suggestedNicks,
	void * param);

//////////////////////////////////////////////////////////////
// chatFillInUserCallback
// Summary
//		Used in conjuction with the chatConnectSpecial and chatConnectSecure 
//		functions; called to fill in the user field after the actual network
//		 connection to the chat server has been made.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		IP		: [in] The IP address in string form: "xxx.xxx.xxx.xxx" to encode
//		user	: [in] The user name to encode
//		param	: [in] Pointer to user data. The same param that was passed
//		 to chatConnectSecure or chatConnectSpecial.
// Remarks
//		This is used by the Peer SDK to encode the local machine's IP address 
//		(as known to the chat server) in the user field.<p>
// See Also
//		chatConnectSecure, chatConnectSpecial
typedef void (* chatFillInUserCallback)(CHAT chat,
	unsigned int IP, // PANTS|08.21.00 - changed from unsigned long
	gsi_char user[128],
	void * param);

//////////////////////////////////////////////////////////////
// chatConnect
// Summary
//		The chatConnect function initializes the Chat SDK and initiates a
//		 connection to the chat server.
// Parameters
//		serverAddress		: [in] Address of the chat server being connect
//		 to.
//		port				: [in] Port of the chat server.
//		nick				: [in] Nickname in use while chatting. Not
//		 associated with a user account in any way.
//		user				: [in] User's username
//		name				: [in] User's real name, or any other optional info.
//		callbacks			: [in] Structure for specifying global handlers.
//		nickErrorCallback	: [in] Optional user-supplied function to be
//		 called if nickname is invalid or in use.
//		connectCallback		: [in] Optional user-supplied function to be
//		 called when the operation has completed.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//									operation has completed; otherwise, return immediately.
// Returns
//		This function returns the initialized Chat SDK interface. A return
//		 value of NULL indicates an error.
// Remarks
//		The server address and port for the connect functions can be left empty.
//		In other words, serverAddress can be NULL, and the port can be
//		 specified to be 0.
//		The SDK will automatically take care of using the default address
//		 and port.<p>
//		Note: to use the GameSpy Chat server (peerchat.gamespy.com) you need to use chatConnectSecure, 
//		chatConnectLogin, or chatConnectPreAuth (as opposed to chatConnect), as the server requires an encrypted 
//		connection.
//		Note #2: This function can still block even if the 'blocking' argument 
//		is set to CHATFalse in situations where DNS response is slow, as 
//		it depends on a blocking gethostbyname() call.
// Example
//		<code lang="c++">
// int CMyGame::OnConnect(...)
// {
//			m_chat = chatConnect("irc.mygame.com", 6667, "nick", "user",
//		 "email@email.com", &callbacks, callback, this, CHATFalse);
//			if (m_chat == NULL)
//				Error();
// }
//		</code>
// See Also
//		chatConnectLogin, chatConnectPreAuth, chatConnectSecure, chatConnectSpecial
CHAT chatConnect(const gsi_char * serverAddress,
	int port,
	const gsi_char * nick,
	const gsi_char * user,
	const gsi_char * name,
	chatGlobalCallbacks * callbacks,
	chatNickErrorCallback nickErrorCallback,
	chatConnectCallback connectCallback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatConnectSpecial
// Summary
//		Initializes the Chat SDK and initiates a connection to the chat server. 
//		The chatConnectSpecial function provides ability to fill in the user
//		 field after 
//		the local machine's IP address is known.
// Parameters
//		serverAddress		: [in] Address of the chat server to connect
//		 to; usually "peerchat.gamespy.com".
//		port				: [in] Port of the chat server; usually 6667.
//		nick				: [in] Nickname in use while chatting. Not
//		 associated with a user account in any way.
//		name				: [in] User's real name, or any other optional info.
//		callbacks			: [in] Structure for specifying global handlers.
//		nickErrorCallback	: [in] Callback that is triggered if nick is
//		 invalid or in use.
//		fillInUserCallback	: [in] Optional user-supplied function to be
//		 called when the SDK requires the user name.
//		connectCallback		: [in] Optional user-supplied function to be
//		 called when the connection attempt has completed.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//									operation has completed; otherwise, return immediately.
// Returns
//		This function returns the initialized Chat SDK interface. A return
//		 value of NULL indicates an error.
// Remarks
//		The server address and port for the connect functions can be left empty.
//		In other words, serverAddress can be NULL, and the port can be
//		 specified to be 0.
//		The SDK will automatically take care of using the default address
//		 and port.<p>
//		Note: This function can still block even if the 'blocking' argument 
//		is set to CHATFalse in situations where DNS response is slow, as 
//		it depends on a blocking gethostbyname() call.
// See Also
//		chatConnect, chatConnectLogin, chatConnectPreAuth, chatConnectSecure
CHAT chatConnectSpecial(const gsi_char * serverAddress,
	int port,
	const gsi_char * nick,
	const gsi_char * name,
	chatGlobalCallbacks * callbacks,
	chatNickErrorCallback nickErrorCallback,
	chatFillInUserCallback fillInUserCallback,
	chatConnectCallback connectCallback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatConnectSecure
// Summary
//		Initializes the Chat SDK and initiates a connection to the chat server. 
//		The chatConnectSecure function encrypts the connection.
// Parameters
//		serverAddress		: [in] Address of the chat server to connect
//		 to; usually "peerchat.gamespy.com".
//		port				: [in] Port of the chat server; usually 6667.
//		nick				: [in] Nickname in use while chatting. Not
//		 associated with a user account in any way.
//		name				: [in] User's real name, or any other optional info.
//		gamename			: [in] GameName of the title this client is connecting from.
//		secretKey			: [in] Assigned secret key for the specified gamename.
//		callbacks			: [in] Structure for specifying global handlers.
//		nickErrorCallback	: [in] Optional user-supplied function to be
//		 called if nickname is invalid or in use.
//		fillInUserCallback	: [in] Optional user-supplied function to be
//		 called when the SDK requires the user name.
//		connectCallback		: [in] Optional user-supplied function to be
//		 called when the operation has completed.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//		 operation has completed; 
//									otherwise, return immediately.
// Returns
//		Returns the initialized Chat SDK interface. A return value of NULL
//		 indicates an error.
// Remarks
//		The server address and port for the connect functions can be left empty.
//		In other words, serverAddress can be NULL, and the port can be
//		 specified to be 0.
//		The SDK will automatically take care of using the default address
//		 and port.<p>
//		Note: This function can still block even if the 'blocking' argument 
//		is set to CHATFalse in situations where DNS response is slow, as 
//		it depends on a blocking gethostbyname() call.
// See Also
//		chatConnect, chatConnectLogin, chatConnectPreAuth, chatConnectSpecial
CHAT chatConnectSecure(const gsi_char * serverAddress,
	int port,
	const gsi_char * nick,
	const gsi_char * name,
	const gsi_char * gamename,
	const gsi_char * secretKey,
	chatGlobalCallbacks * callbacks,
	chatNickErrorCallback nickErrorCallback,
	chatFillInUserCallback fillInUserCallback,
	chatConnectCallback connectCallback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatConnectLogin
// Summary
//		Initializes the Chat SDK and initiates a connection to the chat server. 
//		The chatConnectLogin function provides the ability to login to chat
//		 using a registered unique nickname.
// Parameters
//		serverAddress		: [in] Address of the chat server being connect
//		 to; usually "peerchat.gamespy.com".
//		port				: [in] Port of the chat server; usually 6667.
//		namespaceID			: [in] ID of the unique name namespace in
//		 which the users nickname is registered.
//		email				: [in] E-mail address of the local client's GameSpy profile.
//		profilenick			: [in] Nickname used when creating profile.
//		 May be different from the registered unique nick.
//		uniquenick			: [in] Unique nickname registered to the
//		 profile with which user is logging in.
//		password			: [in] Password of the GameSpy profile.
//		name				: [in] User's real name, or any other optional info.
//		gamename			: [in] Assigned gamename from which the local
//		 client is logging in.
//		secretKey			: [in] Assigned secret key for the specified gamename.
//		callbacks			: [in] Structure for specifying global handlers.
//		nickErrorCallback	: [in] Optional user-supplied function to be
//		 called if nickname is invalid or in use.
//		fillInUserCallback	: [in] Optional user-supplied function to be
//		 called when the SDK requires the user name.
//		connectCallback		: [in] Optional user-supplied function to be
//		 called when the operation has completed.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//		 operation has completed;
//									otherwise, return immediately.
// Returns
//		This function returns the initialized Chat SDK interface. A return
//		 value of NULL indicates an error.
// Remarks
//		The server address and port for the connect functions can be left empty.
//		In other words, serverAddress can be NULL, and the port can be
//		 specified to be 0.
//		The SDK will automatically take care of using the default address
//		 and port.<p>
//		Note: This function can still block even if the 'blocking' argument 
//		is set to CHATFalse in situations where DNS response is slow, as 
//		it depends on a blocking gethostbyname() call.
// See Also
//		chatConnect, chatConnectPreAuth, chatConnectSecure, chatConnectSpecial
CHAT chatConnectLogin(const gsi_char * serverAddress,
	int port,
	int namespaceID,
	const gsi_char * email,
	const gsi_char * profilenick,
	const gsi_char * uniquenick,
	const gsi_char * password,
	const gsi_char * name,
	const gsi_char * gamename,
	const gsi_char * secretKey,
	chatGlobalCallbacks * callbacks,
	chatNickErrorCallback nickErrorCallback,
	chatFillInUserCallback fillInUserCallback,
	chatConnectCallback connectCallback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatConnectPreAuth
// Summary
//		Initializes the Chat SDK and initiates a connection to the chat server.  
//		The chatConnectPreAuth function provides the ability to specify
//		 authtoken and partnerchallenge. (Not for common use).
// Parameters
//		serverAddress		: [in] Address of the chat server to connect
//		 to; usually "peerchat.gamespy.com".
//		port				: [in] Port of the chat server; usually 6667.
//		authtoken			: [in] Authentication token for this login.
//		partnerchallenge	: [in] Partner challenge for this login.
//		name				: [in] The user's real name, or any other optional info.
//		gamename			: [in] GameName of the title this client is connecting from.
//		secretKey			: [in] Assigned secret key for the specified gamename.
//		callbacks			: [in] Structure for specifying global handlers.
//		nickErrorCallback	: [in] Optional user-supplied function to be
//		 called if nickname is invalid or in use.
//		fillInUserCallback	: [in] Optional user-supplied function to be
//		 called when the SDK requires the user name.
//		connectCallback		: [in] Optional user-supplied function to be
//		 called when the operation has completed.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//		 operation has completed;
//									otherwise, return immediately.
// Returns
//		This function returns the initialized Chat SDK interface. A return
//		 value of NULL indicates an error.
// Remarks
//		The server address and port for the connect functions can be left empty.
//		In other words, serverAddress can be NULL, and the port can be
//		 specified to be 0.
//		The SDK will automatically take care of using the default address
//		 and port.<p>
//		Note: This function can still block even if the 'blocking' argument 
//		is set to CHATFalse in situations where DNS response is slow, as 
//		it depends on a blocking gethostbyname() call.
// See Also
//		chatConnect, chatConnectLogin, chatConnectSecure, chatConnectSpecial
CHAT chatConnectPreAuth(const gsi_char * serverAddress,
	int port,
	const gsi_char * authtoken,
	const gsi_char * partnerchallenge,
	const gsi_char * name,
	const gsi_char * gamename,
	const gsi_char * secretKey,
	chatGlobalCallbacks * callbacks,
	chatNickErrorCallback nickErrorCallback,
	chatFillInUserCallback fillInUserCallback,
	chatConnectCallback connectCallback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatRetryWithNick
// Summary
//		Use in response to a nickErrorCallback. This function allows the local 
//		client to retry the connection attempt with a different chat nickname.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		nick	: [in] Alternate chat nickname
// Remarks
//		The chatRetryWithNick function should be used in response to a
//		 nickErrorCallback. 
//		Most often, this occurs when a requested nickname is already in use.  
//		chatRetryWithNick should be called with an alternate nickname such
//		 as "oldnick{1}" to continue the login process. 
//		If another nickError occurs, the nickErrorCallback will be triggered
//		 again.<p>
void chatRetryWithNick(CHAT chat,
	const gsi_char * nick);

//////////////////////////////////////////////////////////////
// chatRegisterUniqueNick
// Summary
//		Registers a unique nick and cdkey to the local client.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		namespaceID	: [in] ID of the namespace community. Assigned by GameSpy.
//		uniquenick	: [in] Nickname being registered.
//		cdkey		: [in] User's CD key; this uniquely identifies the account.
// Remarks
//		The chatRegisterUniqueNick function should be used in response to a
//		 chatNickErrorCallback. 
//		This function requests that a specified unique nick be associated
//		 with the local client and cdkey. 
//		If an error occurs, another chatNickErrorCallback will be trigged. 
//		Take care that this does not result in an infinite loop.<p>
void chatRegisterUniqueNick(CHAT chat,
	int namespaceID,
	const gsi_char * uniquenick,
	const gsi_char * cdkey);

//////////////////////////////////////////////////////////////
// chatDisconnect
// Summary
//		Disconnect from the chat server. Performs necessary cleanup of the Chat SDK.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
// Remarks
//		The chatDisconnect function disconnects the SDK from the chat server and
//		performs necessary cleanup on the CHAT object. The CHAT object is
//		 invalid after 
//		this call has completed. To continue using the chat SDK you must
//		 reinitialize 
//		using one of the chat connect methods.<p>
// See Also
//		ChatConnect
void chatDisconnect(CHAT chat);

//////////////////////////////////////////////////////////////
// chatThink
// Summary
//		Allow the Chat SDK to continue processing.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
// Remarks
//		All network communications, callbacks and other events will happen only
//		during this call. The frequency with which this method is called
//		 will affect general performance on the SDK.<p>
// See Also
//		ChatConnect, ChatDisconnect
void chatThink(CHAT chat);

//////////////////////////////////////////////////////////////
// chatSendRaw
// Summary
//		Send a raw command to the chat server. This does not automatically
//		 send to a player.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		command	: [in] Raw command to send to the chat server.
// Remarks
//		The chatSendRaw function may be used to send a raw command to the server. 
//		Special care should be taken when using this command, as undesired 
//		behavior may result from malformed command sequences. 
//		If in doubt, please contact developer support on the use of this command.<p>
void chatSendRaw(CHAT chat,
	const gsi_char * command);

//////////////////////////////////////////////////////////////
// chatChangeNickCallback
// Summary
//		Callback for chatChangeNick when a player changes his/her nick.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		oldNick	: [in] The old nickname.
//		newNick	: [in] The new nickname.
//		param	: [in] User data; the same param pointer that was passed to
//		 chatChangeNick.
// Remarks
//		The chatChangedNickCallback is called when any player in the
//		 specified room changes his/her nick.
//		The new nick is assigned to the player if the change was validated
//		 by the server.
//		Otherwise, there will be no difference between the old nick or the new nick.
//		The change is determined by "success" which is either CHATTrue or
//		 CHATFalse.<p>
typedef void (* chatChangeNickCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * oldNick,
	const gsi_char * newNick,
	void * param);
//////////////////////////////////////////////////////////////
// chatChangeNick
// Summary
//		Change the chat nickname associated with the local client. This does
//		 not affect the account name.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		newNick		: [in] Nickname to assign to the local user.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatChangeNick function may be used to change a user's nickname
//		 as it appears in chat. 
//		This has no affect on GameSpy profile names such as those used for
//		 presence detection and buddy lists. 
//		Only one instance of a nickname may be in use at a time.<p>
void chatChangeNick(CHAT chat,
	const gsi_char * newNick,
	chatChangeNickCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatGetNick
// Summary
//		Gets the chat nickname of the local client. This may not be the same
//		 as the profile nickname.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
// Returns
//		The nickname of the local client.
const gsi_char * chatGetNick(CHAT chat);

// Copies the oldNick to the newNick, replacing any invalid characters with
//		 legal ones.
///////////////////////////////////////////////////////////////////////////////////////
void chatFixNick(gsi_char * newNick,
	const gsi_char * oldNick);

//////////////////////////////////////////////////////////////
// chatFixNick
// Summary
//		Repairs an illegal chat nickname.
// Parameters
//		newNick	: [out] Receives corrected nickname; may be identical to
//		 original nickname if no issues are detected.
//		oldNick	: [in] Nickname to be corrected or verified.
// Remarks
//		The chatFixNick function replaces illegal characters in the nickname
//		with the underscore ("_") character. This function will also replace leading
//		numbers and illegal whitespace combinations.<p>
// See Also
//		ChatConnect, ChatDisconnect
const gsi_char * chatTranslateNick(gsi_char * nick,
	const gsi_char * extension);

//////////////////////////////////////////////////////////////
// chatGetUserID
// Summary
//		Gets the user id of the local client.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using
//		 chatConnectLogin or chatConnectPreAuth.
// Returns
//		Returns the user id of the local client.
// Remarks
//		The chat SDK must have been initialized using chatConnectLogin or
//		 chatConnectPreAuth.<p>
int chatGetUserID(CHAT chat);

//////////////////////////////////////////////////////////////
// chatGetProfileID
// Summary
//		Gets the profile id of the local client.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using
//		 chatConnectLogin or chatConnectPreAuth.
// Returns
//		Returns the profile id of the local client.
// Remarks
//		The chat SDK must have been initialized using chatConnectLogin or
//		 chatConnectPreAuth.<p>
// See Also
//		chatConnectLogin, chatConnectPreAuth
int chatGetProfileID(CHAT chat);

//////////////////////////////////////////////////////////////
// chatSetQuietMode
// Summary
//		Sets the chat SDK to quiet mode or disables quiet mode.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		quiet	: [in] If CHATTrue, enable quiet mode; otherwise, disable.
// Remarks
//		The chatSetQuietMode function is used to toggle quiet mode. 
//		When in quiet mode the chat SDK will not receive chat or other messages. 
//		This allows the user to remain logged into chat without disrupting
//		 gameplay with extraneous traffic.<p>
void chatSetQuietMode(CHAT chat,
	CHATBool quiet);

//////////////////////////////////////////////////////////////
// chatAuthenticateCDKeyCallback
// Summary
//		Called when chatAuthenticateCDKey and attempt to authenticate the
//		 CD-Key is finished.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		result	: [in] Indicates the result of the attempt.
//		message	: [in] The text message representing the result.
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatAuthenticateCDKeyCallback function gets called when an
//		 attempt to authenticate a CD key is finished.
//		If the result has a value of 1, the CD key was authenticated.
//		Otherwise, the CD key was not authenticated.<p>
typedef void (* chatAuthenticateCDKeyCallback)(CHAT chat,
	int result,
	const gsi_char * message,
	void * param);

//////////////////////////////////////////////////////////////
// chatAuthenticateCDKey
// Summary
//		Allows pre-chat cd key authentication via the chat server.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		cdkey	: [in] CD key to validate; should be a valid CD key for the
//		 set game title.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param	: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatAuthenticateCDKey function may be used to authenticate a
//		 user's cdkey before they enter the chat room. 
//		This should not be a substitute for a cdkey during gameplay. 
//		Arcade does not support this call, so users in Arcade will be able
//		 to enter chat without this validation. 
//		This method most usefull for developers who opt-out of the Arcade 
//		compatability requirements or have a separate chat area for in-game
//		 clients.<p>
// See Also
//		ChatConnect
void chatAuthenticateCDKey(CHAT chat,
	const gsi_char * cdkey,
	chatAuthenticateCDKeyCallback callback,
	void * param,
	CHATBool blocking);

/*************
** CHANNELS **
*************/

//////////////////////////////////////////////////////////////
// chatEnumChannelsCallbackEach
// Summary
//		Called after an attempt to enumerate each channel.
// Parameters
//		chat		: [in] The initialized chat interface object.
//		success		: [in] CHATTrue if success, CHATFalse if failure.
//		index		: [in] The index of this channel
//		channel		: [in] The name of the channel
//		topic		: [in] A string containing the topic of the channel
//		numUsers	: [in] The number of users in this channel
//		param		: [in] Pointer to user data. Passed through unmodified
//		 from the initiating function.
// Remarks
//		The chatEnumChannelsCallbackEach function is called when an attempt
//		to enumerate each channel on the server is complete.
//		The successful attempt will have a channel with an index, the name of 
//		the channel, the topic for that channel, the number of users.<p>
typedef void (* chatEnumChannelsCallbackEach)(CHAT chat,
	CHATBool success,
	int index,
	const gsi_char * channel,
	const gsi_char * topic,
	int numUsers,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnumChannelsCallbackAll
// Summary
//		Called when an attempt to enumerate all the channels is complete.
// Parameters
//		chat		: [in] The initialized chat interface object.
//		success		: [in] CHATTrue if success, CHATFalse if failure.
//		numChannels	: [in] The number of channels in the list
//		channels	: [in] The List of channels
//		topics		: [in] The List of topics associated with the list of channels
//		numUsers	: [in] The number of users for each channel
//		param		: [in] Pointer to user data. Passed through unmodified
//		 from the initiating function.
// Remarks
//		The chatEnumChannelsCallbackAll function is called when an
//		 enumeration attempt of all channels has completed.
//		The function will contain all the data necessary to update the
//		list of channels including names of channels, number of people in
//		 each channel, and channel topics.
//		It is also called after each is enumerated
//		 (chatEnumChannelsCallbackEach).<p>
typedef void (* chatEnumChannelsCallbackAll)(CHAT chat,
	CHATBool success,
	int numChannels,
	const gsi_char ** channels,
	const gsi_char ** topics,
	int * numUsers,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnumChannels
// Summary
//		Enumerates the chat channels on the server.
// Parameters
//		chat			: [in] Chat SDK object, previously initialized using
//		 one of the chatConnect methods.
//		filter			: [in] String comparison used to filter the channel results.
//								Example "#gsp!mygame!". Use the "*" for the wildcard.
//		callbackEach	: [in] Optional user-supplied function to be called
//		 once for each channel in the list.
//		callbackAll		: [in] Optional user-supplied function to be
//		 called once for the full channel list.
//		param			: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking		: [in] If CHATTrue, return only after the operation
//		 has completed; otherwise, return immediately.
// Remarks
//		The chatEnumChannels function enumerates the chat channels which
//		 match the currect search criteria.
//		Typical information returned on each channel includes the topic and
//		 number of users.
//		The filter can contain wildcards used to get all channels when
//		 passing in a partial name and wildcard.<p>
// See Also
//		ChatConnect, ChatDisconnect
CHATOperationResult chatEnumChannels(CHAT chat,
	const gsi_char * filter,
	chatEnumChannelsCallbackEach callbackEach,
	chatEnumChannelsCallbackAll callbackAll,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatEnumChannelsWithLimit
// Summary
//		Requests chat server to send a limited list of channels to enumerate
//		 through.
// Parameters
//		chat				: [in] Chat SDK object, previously initialized
//		 using one of the chatConnect methods.
//		maxNumberOfChannels	: [in] Maximum number of channels the server
//		 responds with.
//		filter				: [in] String comparison used to filter the channel results.
//									Example "#gsp!mygame!". Use the "*" for the wild card.
//		callbackEach		: [in] Optional user-supplied function to be
//		 called once for each channel in the list.
//		callbackAll			: [in] Optional user-supplied function to be
//		 called once for the full channel list.
//		param				: [in] Optional pointer to user data; will be
//		 passed unmodified to the callback function.
//		blocking			: [in] If CHATTrue, return only after the
//		 operation has completed; otherwise, return immediately.
// Remarks
//		A filter is optional in this function. Both callbacks are required
//		 but one of them can be empty.
//		Channels are not sent back in any particular order.
//		The maxNumberOfChannels should not be zero.
//		Use chatEnumChannels if there is no limit requested.<p>
CHATOperationResult chatEnumChannelsWithLimit(CHAT chat,
	gsi_u32 maxNumberOfChannels,
	const gsi_char * filter,
	chatEnumChannelsCallbackEach callbackEach,
	chatEnumChannelsCallbackAll callbackAll,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatEnumJoinedChannelsCallback
// Summary
//		Called after an attempt to enumerate joined channels.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		index	: [in] An index of the joined channels for this channel
//		channel	: [in] The name of the channel
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatEnumJoinedChannelsCallback function is calle when an attempt
//		to enumerate--the channels the local player has joined--is complete.
//		The function will contain the channel name, an index to the channel 
//		which refers to the position in the list of joined channels.<p>
typedef void (* chatEnumJoinedChannelsCallback)(CHAT chat,
	int index,
	const gsi_char * channel,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnumJoinedChannels
// Summary
//		Enumerates the chat channels on the server which the local client
//		 has joined. 
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		callback	: [in] Optional user-supplied function to be called once
//		 for each channel in the list.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
// Remarks
//		For each channel, a channel index value is returned that may be 
//		used to retrieve further information about the channel.<p>
// See Also
//		ChatConnect, ChatDisconnect, ChatEnumChannels
void chatEnumJoinedChannels(CHAT chat,
	chatEnumJoinedChannelsCallback callback,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnterChannelCallback
// Summary
//		Called when an attempt to enter the channel has completed.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		result	: [in] Indicates the result of the attempt
//		channel	: [in] The name of channel entered
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatEnterChannelCallback function is called when the attempt
//		to enter the channel by the local player is completed.
//		The entrance of the channel can be successful or a failure, and is
//		 indicated by the "result" of the attempt.
//		The "result" can be of the following value:<p>
//		<emit \<ul\>>
//		<emit \<li\>>CHATEnterSuccess -- The channel was successfully
//		 entered.<emit \</li\>>
//		<emit \<li\>>CHATBadChannelName -- The channel name was
//		 invalid.<emit \</li\>>
//		<emit \<li\>>CHATChannelIsFull -- The channel is at its user
//		 limit.<emit \</li\>>
//		<emit \<li\>>CHATInviteOnlyChannel -- The channel is invite
//		 only.<emit \</li\>>
//		<emit \<li\>>CHATBannedFromChannel -- The local user is banned from
//		 this channel.<emit \</li\>>
//		<emit \<li\>>CHATBadChannelPassword -- The channel has a password,
//		 and a bad password (or none) was given.<emit \</li\>>
//		<emit \<li\>>CHATTooManyChannels -- The server won't allow this user
//		 in any more channels.<emit \</li\>>
//		<emit \<li\>>CHATEnterTimedOut -- The attempt to enter timed
//		 out.<emit \</li\>>
//		<emit \<li\>>CHATBadChannelMask -- The channel mask was bad (rarely
//		 used).<emit \</li\>>
//		<emit \</ul\>>
typedef void (* chatEnterChannelCallback)(CHAT chat,
	CHATBool success,
	CHATEnterResult result,
	const gsi_char * channel,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnterChannel
// Summary
//		Joins a chat channel. Creates the channel first if it does not already exist.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel being joined.
//		password	: [in] Password of the channel. Ignored if no password
//		 has been set.
//		callbacks	: [in] Structure for specifying global handlers; for
//		 channel-specific traffic such as user messages.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatEnterChannel function is used to add the local client to a
//		 chat channel. 
//		If the channel is password protected the valid password must be supplied. 
//		If it is not, the callback will be triggered with an invalid
//		 password result.<p>
// See Also
//		ChatConnect, ChatDisconnect
void chatEnterChannel(CHAT chat,
	const gsi_char * channel,
	const gsi_char * password,
	chatChannelCallbacks * callbacks,
	chatEnterChannelCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatLeaveChannel
// Summary
//		Leave a chat channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel being left.
//		reason	: [in] Optional reason for leaving. This may be displayed
//		 to the remaining users.
// Remarks
//		The chatLeaveChannel function is used to remove the local client
//		 from a chat channel.<p>
// See Also
//		ChatConnect, ChatDisconnect
void chatLeaveChannel(CHAT chat,
	const gsi_char * channel,
	const gsi_char * reason); 

//////////////////////////////////////////////////////////////
// chatSendChannelMessage
// Summary
//		Send a message to all members of the specified channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel to which the message is being sent.
//		message	: [in] Message.
//		type	: [in] One of the predefined chat types. Used to send chat,
//		 hidden messages, notices, and other types.
// Remarks
//		The chatSendChannelMessage function is used to send a message to all
//		 users of a specified channel. 
//		The type of message that may be sent can be chat, UTM, notices or
//		 actions.<p>
void chatSendChannelMessage(CHAT chat,
	const gsi_char * channel,
	const gsi_char * message,
	int type);

//////////////////////////////////////////////////////////////
// chatSetChannelTopic
// Summary
//		Set the topic (description) of a chat channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose topic is being set.
//		topic	: [in] Description of new topic.
// Remarks
//		The chatSetChannelTopic function is used to set the topic
//		 (description) of a chat channel. 
//		Some channels, such as the title and group rooms, will not allow
//		 users to set the topic.<p>
// See Also
//		chatGetChannelTopic
void chatSetChannelTopic(CHAT chat,
	const gsi_char * channel,
	const gsi_char * topic);

//////////////////////////////////////////////////////////////
// chatGetChannelTopicCallback
// Summary
//		Called after an attempt to get the channel's topic.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The name of channel
//		topic	: [in] A string containing the topic
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetChannelTopicCallback function is called when an attempt
//		 to obtain the channel's topic is complete.
//		If successful, the text message containing the topic for that
//		 channel will be available.<p>
typedef void (* chatGetChannelTopicCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	const gsi_char * topic,
	void * param);
//////////////////////////////////////////////////////////////
// chatGetChannelTopic
// Summary
//		Queries the server for the specified channel's topic. Also known as
//		 the room description.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose topic is being retrieved.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// See Also
//		chatSetChannelTopic
void chatGetChannelTopic(CHAT chat,
	const gsi_char * channel,
	chatGetChannelTopicCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatSetChannelMode
// Summary
//		Set a channel's mode.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose mode is being set.
//		mode	: [in] Properties to set on the target channel.
// Remarks
//		The mode includes standard IRC properties such as "InviteOnly,
//		 Private and Moderated".<p>
// See Also
//		CHATChannelMode, chatGetChannelMode
void chatSetChannelMode(CHAT chat,
	const gsi_char * channel,
	CHATChannelMode * mode);

//////////////////////////////////////////////////////////////
// chatGetChannelModeCallback
// Summary
//		Called after an attempt to get the channel mode.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The name of channel
//		mode	: [in] One of the predefined modes
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetChannelModeCallback function is called when an attempt to
//		 get the channel mode is complete.
//		If successful, the function will have the channel name and its mode.<p>
typedef void (* chatGetChannelModeCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	CHATChannelMode * mode,
	void * param);
//////////////////////////////////////////////////////////////
// chatGetChannelMode
// Summary
//		Retrieves the "mode" of a channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose mode is being retrieved.
//		callback	: [in] User-supplied function to receive mode information.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// See Also
//		CHATChannelMode, chatGetChannelModeCallback
void chatGetChannelMode(CHAT chat,
	const gsi_char * channel,
	chatGetChannelModeCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatSetChannelPassword
// Summary
//		Sets or clears a password on the specified channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose password is being set.
//		enable		: [in] If CHATTrue, enable the password; otherwise, disable.
//		password	: [in] Password string which users must supply to join the channel.
// Remarks
//		Set the value to NULL or "" to clear the value.<p>
void chatSetChannelPassword(CHAT chat,
	const gsi_char * channel,
	CHATBool enable,
	const gsi_char * password);

//////////////////////////////////////////////////////////////
// chatGetChannelPasswordCallback
// Summary
//		Called after an attempt to get the channel's password.
// Parameters
//		chat		: [in] The initialized chat interface object.
//		success		: [in] CHATTrue if success, CHATFalse if failure.
//		channel		: [in] The name of channel
//		enabled		: [in] CHATTrue if enabled, CHATFalse if otherwise
//		password	: [in] The channel password
//		param		: [in] Pointer to user data. Passed through unmodified
//		 from the initiating function.
// Remarks
//		The chatGetChannelPasswordCallback function is called when an attempt to 
//		obtain the channel's password is complete.
//		If successful, the password for that channel will be available.<p>
typedef void (* chatGetChannelPasswordCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	CHATBool enabled,
	const gsi_char * password,
	void * param);

//////////////////////////////////////////////////////////////
// chatGetChannelPassword
// Summary
//		Queries the server for the specified channel's password.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose password is being retrieved.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// See Also
//		chatSetChannelPassword
void chatGetChannelPassword(CHAT chat,
	const gsi_char * channel,
	chatGetChannelPasswordCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatSetChannelLimit
// Summary
//		Set the maximum number of users allowed in a channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose limit is being set.
//		limit	: [in] Maximum number of users on channel.
// Remarks
//		The chatSetChannelLimit function may be used to set the maximum
//		 number of users on a chat room.<p>
void chatSetChannelLimit(CHAT chat,
	const gsi_char * channel,
	int limit);

//////////////////////////////////////////////////////////////
// chatEnumChannelBansCallback
// Summary
//		Called after an attempt to enumerate channel bans.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The channel that the enumeration was attempted
//		numBans	: [in] The number of bans in the list
//		bans	: [in] The List of bans
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatEnumChannelBansCallback function is called when an attempt
//		 to enumerate channel bans has completed.
//		The available results are whether the attempt was successful, the
//		 list of the bans,
//		number of bans, the channel that the attempt was made on.<p>
typedef void (* chatEnumChannelBansCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	int numBans,
	const gsi_char ** bans,
	void * param);
//////////////////////////////////////////////////////////////
// chatEnumChannelBans
// Summary
//		Retrieves a list of clients banned from a channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose ban list is being retrieved.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation
//							has completed; will be passed the list of banned clients.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The local client must have operator privileges to execute this command.<p>
void chatEnumChannelBans(CHAT chat,
	const gsi_char * channel,
	chatEnumChannelBansCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatAddChannelBan
// Summary
//		Ban a nickname from the specified channel. Local client must have
//		 moderator privileges.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of chat channel from which user is being banned.
//		ban		: [in] Chat nickname of user being banned.
void chatAddChannelBan(CHAT chat,
	const gsi_char * channel,
	const gsi_char * ban);

//////////////////////////////////////////////////////////////
// chatRemoveChannelBan
// Summary
//		Removes a banned player from a channel's ban list. This will once
//		 again allow the user to join the channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose ban list is being modified..
//		ban		: [in] Nickname to remove from the ban list.
void chatRemoveChannelBan(CHAT chat,
	const gsi_char * channel,
	const gsi_char * ban);

//////////////////////////////////////////////////////////////
// chatSetChannelGroup
// Summary
//		Assign a user-defined grouping to a channel. The group is a string
//		 identifier which is linked to the channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel to which a group is being assigned.
//		group	: [in] Group string to assign to channel.
// Remarks
//		The chatSetChannelGroup function may be used to attach a
//		 user-defined string to a channel. 
//		This string exists locally and is not sent across the network. 
//		This string may be used as a local grouping for channels.<p>
void chatSetChannelGroup(CHAT chat,
	const gsi_char * channel,
	const gsi_char * group);

//////////////////////////////////////////////////////////////
// chatGetChannelNumUsers
// Summary
//		Returns the number of users in the already joined channel. This is a
//		 cached value, and not a server query.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose user count is being retrieved.
// Returns
//		Returns the number of users in the channel. If the local client has
//		 not joined the channel, -1 will be returned.
int chatGetChannelNumUsers(CHAT chat,
	const gsi_char * channel);


//////////////////////////////////////////////////////////////
// chatInChannel
// Summary
//		Determine whether the local client is a member of the specified channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel being inspected.
// Returns
//		This function will return CHATTrue if the local client is a member
//		 of the specified channel, CHATFalse otherwise.
// Remarks
//		The chatInChannel function checks the local list of channels to
//		 determine whether the local client is a member.
//		No communication with the server is attempted during this call.<p>
CHATBool chatInChannel(CHAT chat,
	const gsi_char * channel);


/**********
** USERS **
**********/

//////////////////////////////////////////////////////////////
// chatEnumUsersCallback
// Summary
//		Called after an attempt to enumerate the users in a channel.
// Parameters
//		chat		: [in] The initialized chat interface object.
//		success		: [in] CHATTrue if success, CHATFalse if failure.
//		channel		: [in] The name of the channel
//		numUsers	: [in] The number of users in the channel
//		users		: [in] The list of users names in the channel
//		modes		: [in] The list of modes for the channel
//		param		: [in] Pointer to user data. Passed through unmodified
//		 from the initiating function.
// Remarks
//		The chatEnumUsersCallback is called when an attempt to enumerate all
//		 of the users in a given channel is made.
//		The function will have the information of the users in the channel
//		 if success is CHATTrue.<p>
typedef void (* chatEnumUsersCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel, //PANTS|02.11.00|added parameter
	int numUsers,
	const gsi_char ** users,
	int * modes,
	void * param);

//////////////////////////////////////////////////////////////
// chatEnumUsers
// Summary
//		Retrieves the list of users in the specified channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel whose user list is being retrieved.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed; 
//							will be passed the user list.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
void chatEnumUsers(CHAT chat,
	const gsi_char * channel,
	chatEnumUsersCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatSendUserMessage
// Summary
//		Send a private message to a user.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		user	: [in] Nickname of the user to whom the private message is being sent.
//		message	: [in] Message; generally chat text, but may also be a
//		 raw data message.
//		type	: [in] One of the ChatType predefined types; can signify a
//		 chat message or a raw data message.
// Remarks
//		The chatSendUserMessage function to send a private message to a
//		 specified user. 
//		The recipient does not need to be in the same room as the sender.<p>
void chatSendUserMessage(CHAT chat,
	const gsi_char * user,
	const gsi_char * message,
	int type);

//////////////////////////////////////////////////////////////
// chatGetUserInfoCallback
// Summary
//		Called after an attempt to get user information.
// Parameters
//		chat		: [in] The initialized chat interface object.
//		success		: [in] CHATTrue if success, CHATFalse if failure.
//		nick		: [in] The local player's chat nickname
//		user		: [in] The nickname of the target user
//		name		: [in] The name of the user to get info from
//		address		: [in] The IP address of the user
//		numChannels	: [in] The number of channels the user is in
//		channels	: [in] The actual list of channels the user is in
//		param		: [in] Pointer to user data. Passed through unmodified
//		 from the initiating function.
// Remarks
//		The chatGetUserInfoCallback function is called when an attempt to
//		 get the user information
//		about another player is completed.
//		If successful, the user's nickname, IP address, the channels s/he is
//		 on will be available.<p>
typedef void (* chatGetUserInfoCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * nick,  //PANTS|02.14.2000|added nick and user
	const gsi_char * user,
	const gsi_char * name,
	const gsi_char * address,
	int numChannels,
	const gsi_char ** channels,
	void * param);
//////////////////////////////////////////////////////////////
// chatGetUserInfo
// Summary
//		Gets information on the specified user.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		user		: [in] User's chat nickname.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The user information includes the user's profile nickname, username,
//		 real name and address. 
//		The callback also contains the channels that this user is a member of.<p>
void chatGetUserInfo(CHAT chat,
	const gsi_char * user,
	chatGetUserInfoCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatGetBasicUserInfoCallback
// Summary
//		Called after an attempt to get basic information on a user.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		nick	: [in] The user's chat nickname
//		user	: [in] The nickname of the target user
//		address	: [in] The IP address of the user
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetBasicUserInfoCallback function is called when an attempt
//		 to get basic 
//		information on a user is completed.
//		If successful, the information will contain the user's chat
//		 nickname, the IP address of that user.<p>
typedef void (* chatGetBasicUserInfoCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * nick,
	const gsi_char * user,
	const gsi_char * address,
	void * param);

//////////////////////////////////////////////////////////////
// chatGetBasicUserInfo
// Summary
//		Retrieves basic information on the specified user.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		user		: [in] User's assigned GameName.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation 
//							has completed; will be passed the user's info.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatGetBasicUserInfo function is used to retrieve basic
//		 information on a user. 
//		This information consists of the chat nickname, user profile name,
//		 and IP address.<p>
// See Also
//		chatGetBasicUserInfoNoWait
void chatGetBasicUserInfo(CHAT chat,
	const gsi_char * user,
	chatGetBasicUserInfoCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatGetBasicUserInfoNoWait
// Summary
//		Retrieves basic information on the specified user. Information is
//		 returned through function parameters.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		nick	: [out] Receives the user's nickname
//		user	: [out] Receives the user's username
//		address	: [out] Receives the user's IP address.
// Returns
//		Returns CHATTrue if info is available, CHATFalse otherwise.
// Remarks
//		chatGetBasicUserInfoNoWait is used to retrieve basic information on a user. 
//		This information consists of the chat nickname, user profile name
//		 and IP address.<p>
// See Also
//		chatGetBasicUserInfo
CHATBool chatGetBasicUserInfoNoWait(CHAT chat,
	const gsi_char * nick,
	const gsi_char ** user,
	const gsi_char ** address);

//////////////////////////////////////////////////////////////
// chatGetChannelBasicUserInfoCallback
// Summary
//		Called when an attempt to get everyone's basic user info is made.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The name of the channel
//		nick	: [in] The local player's chat nickname
//		user	: [in] The nickname of the target user
//		address	: [in] The IP address  of the target user
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetChannelBasicUserInfoCallback function is called with a
//		 user's basic info for everyone in a channel.
//		Called with a NULL nick/user/address at the end.<p>
typedef void (* chatGetChannelBasicUserInfoCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	const gsi_char * nick,
	const gsi_char * user,
	const gsi_char * address,
	void * param);

//////////////////////////////////////////////////////////////
// chatGetChannelBasicUserInfo
// Summary
//		Retrieves basic user info for every member of the specified channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel from which user
//		 information is being retrieved
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatGetChannelBasicUserInfo function retreives basic information for 
//		each of the users in the specified channel. 
//		The information returned consists of the nickname, profilename and
//		 IP address.<p>
void chatGetChannelBasicUserInfo(CHAT chat,
	const gsi_char * channel,
	chatGetChannelBasicUserInfoCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatInviteUser
// Summary
//		Invite a user to join a channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel to which the user is being invited.
//		user	: [in] User's chat nickname.
// Remarks
//		The chatInviteUser function may be used to invite a user to a
//		 particular chat room.<p>
void chatInviteUser(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user);

//////////////////////////////////////////////////////////////
// chatKickUser
// Summary
//		Forcefully remove a user from a specified channel. 
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel from which the user is
//		 being removed.
//		user	: [in] User's chat nickname.
//		reason	: [in] Optional text string that will be sent along with
//		 the kick message. 
//						This message will appear in the user kick callback.
// Remarks
//		The local client must have operator privileges to execute this command.<p>
void chatKickUser(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	const gsi_char * reason);

//////////////////////////////////////////////////////////////
// chatBanUser
// Summary
//		Ban a user from the chat room. The user may not rejoin.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of chat channel from which user is being banned.
//		user	: [in] Chat nickname of user being banned.
// Remarks
//		The caller of this function must have operator privileges for the
//		 channel in which the ban is to be performed.<p>
// See Also
//		ChatConnect
void chatBanUser(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user);

//////////////////////////////////////////////////////////////
// chatSetUserMode
// Summary
//		Set the IRC mode of the specified user. This mode is applied in the
//		 specified channel.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the user's chat channel.
//		user	: [in] User's chat nickname on that channel.
//		mode	: [in] User mode flags. See Remarks.
// Remarks
//		The chatSetUserMode function may be used to set a user's mode in a
//		 particular channel. 
//		Modes are used to track which users have operator and speaking
//		 privileges.<p>
//		The following user mode flags are defined:
//		CHAT_NORMAL -- Normal (no speaking privileges; no operator privileges)
//		CHAT_VOICE -- User has speaking privileges.
//		CHAT_OP -- User has operator privileges.<p>
//		User mode flags may be OR'ed together. CHAT_NORMAL is superseded by
//		 any other user mode flag.

void chatSetUserMode(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int mode);

//////////////////////////////////////////////////////////////
// chatGetUserModeCallback
// Summary
//		Called after an attempt to get the user's mode.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The name of channel
//		user	: [in] The nickname of the target user
//		mode	: [in] One of the predefined modes
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetUserModeCallback function is called when an attempt to
//		 get the user mode is completed.
//		If successful, the user's nickname and mode will be available.<p>
typedef void (* chatGetUserModeCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	const gsi_char * user,
	int mode,
	void * param);
//////////////////////////////////////////////////////////////
// chatGetUserMode
// Summary
//		Get the mode of a user in a specified channel.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel being inspected.
//		user		: [in] User's chat nickname on that channel.
//		callback	: [in] Optional user-supplied function to be called when the 
//							operation has completed; will be passed user's mode.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatGetUserMode function may be used to check a user's "mode" in
//		 a specified chat channel. 
//		A mode may specify a channel operator.<p>
// See Also
//		chatGetUserModeNoWait
void chatGetUserMode(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	chatGetUserModeCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatGetUserModeNoWait
// Summary
//		Get the mode of a user in a specified channel, returning it through
//		 a function parameter.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel being inspected.
//		user	: [in] User's chat nickname on that channel.
//		mode	: [out] Receives the mode of target  user.
// Returns
//		True if the user is present in the chat channel, false if not or if
//		 there's an error in the connection.
// Remarks
//		The chatGetUserModeNoWait function may be used to check a user's
//		 "mode" in a specified chat channel. 
//		A mode may specify a channel operator.<p>
// See Also
//		chatGetUserMode
CHATBool chatGetUserModeNoWait(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int * mode);


// Called in response to a request for the UDP relay for a channel
////////////////////////////////////////////////////////////////////
typedef void (* chatGetUdpRelayCallback)(CHAT chat,
	const gsi_char * channel,
	const gsi_char * udpIp,
	unsigned short udpPort,
	int udpKey,
	void * param);

// Get the UDP relay address for a channel
///////////////////////////////////
void chatGetUdpRelay(CHAT chat,
	const gsi_char * channel,
	chatGetUdpRelayCallback callback,
	void * param,
	CHATBool blocking);


/*********
** KEYS **
*********/

//////////////////////////////////////////////////////////////
// chatSetGlobalKeys
// Summary
//		Set key/values on the local client. 
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		num		: [in] Number of key/value pairs being set.
//		keys	: [in] Array of keys being set.
//		values	: [in] Array of values being set, in the same order as their keys.
// Remarks
//		Set the value to NULL or "" to clear the value.<p>
void chatSetGlobalKeys(CHAT chat,
	int num,
	const gsi_char ** keys,
	const gsi_char ** values);

//////////////////////////////////////////////////////////////
// chatGetGlobalKeysCallback
// Summary
//		Called after an attempt to get the global keys for the user(s).
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		user	: [in] The nickname of the target user or the name of the channel
//		num		: [in] The number of key/value pairs in the array
//		keys	: [in] The array of key names whose values will be retrieved
//		values	: [in] The array of values associated with the key array
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetGlobalKeysCallback function is called when an attempt to
//		 obtain the global keys of a user or al.
//		users is complete.
//		If successful, the keys for those user(s) will be available.<p>
//		If used for a set of users, will be called with user==NULL when done.
typedef void (* chatGetGlobalKeysCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * user,
	int num,
	const gsi_char ** keys,
	const gsi_char ** values,
	void * param);

//////////////////////////////////////////////////////////////
// chatGetGlobalKeys
// Summary
//		Retrieves a list of global keys for a single user, or all users.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		target		: [in] Target name, or NULL to specify all users.
//		num			: [in] Number of keys to retrieve for each target.
//		keys		: [in] Array of key names to request values for.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		To get the global key/values for one user, pass in that user's nick
//		 as the target.
//		To get the global key/values for every user in a channel, use the
//		 channel name as the target.
// See Also
//		chatSetGlobalKeys
void chatGetGlobalKeys(CHAT chat,
	const gsi_char * target,
	int num,
	const gsi_char ** keys,
	chatGetGlobalKeysCallback callback,
	void * param,
	CHATBool blocking);

//////////////////////////////////////////////////////////////
// chatSetChannelKeys
// Summary
//		Set key/values on a channel or the local user.
// Parameters
//		chat	: [in] Chat SDK object, previously initialized using one of
//		 the chatConnect methods.
//		channel	: [in] Name of the chat channel whose keys are being set.
//		user	: [in] User to assign keys to. May be NULL. Only channel
//		 operators may set keys on other players.
//		num		: [in] Number of key/value pairs being set.
//		keys	: [in] Array of keys being set.
//		values	: [in] Array of values being set, in the same order as their keys.
// Remarks
//		The chatSetChannelKeys function may be used to set channel keys on a
//		 member or on the channel itself.  
//		Only channel operators may set keys on other players.<p>
//		If user is NULL or "", the keys will be set on the channel.
//		Otherwise, they will be set on the user,
//		Only ops can set channel keys on other users.
//		Set a value to NULL or "" to clear that key.
//		Note: Broadcast keys are a special class of channel keys and start with 'b_'. New broadcast keys and 
//		updates are immediately propagated to everyone else in the room via the chatBroadcastKeyChanged() callback.
void chatSetChannelKeys(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int num,
	const gsi_char ** keys,
	const gsi_char ** values);

//////////////////////////////////////////////////////////////
// chatGetChannelKeysCallback
// Summary
//		Called after an attempt to get the channel keys or user(s) keys.
// Parameters
//		chat	: [in] The initialized chat interface object.
//		success	: [in] CHATTrue if success, CHATFalse if failure.
//		channel	: [in] The name of the channel
//		user	: [in] The nickname of the target user
//		num		: [in] The number of key/value pairs in the array
//		keys	: [in] The array of key names whose values will be retrieved
//		values	: [in] The array of values associated with the array of keys
//		param	: [in] Pointer to user data. Passed through unmodified from
//		 the initiating function.
// Remarks
//		The chatGetChannelKeysCallback function is called when an attempt to either 
//		get either the channel or user(s) keys is completed.
//		If the call to chatGetChannelKeys was made on a set of users, then this 
//		function will get called for all users and have a NULL for "user" when done.
//		If the call was for the channel keys, then the "user" will be NULL.<p>
typedef void (* chatGetChannelKeysCallback)(CHAT chat,
	CHATBool success,
	const gsi_char * channel,
	const gsi_char * user,
	int num,
	const gsi_char ** keys,
	const gsi_char ** values,
	void * param);

//////////////////////////////////////////////////////////////
// chatGetChannelKeys
// Summary
//		Retrieves a list of key/value pairs for a channel or user.
// Parameters
//		chat		: [in] Chat SDK object, previously initialized using one
//		 of the chatConnect methods.
//		channel		: [in] Name of the chat channel from which key/value
//		 pairs are being retrieved
//		user		: [in] Name of the user whose key/value pairs are being retrieved, 
//							or "*" to indicate the channel itself.
//		num			: [in] Number of keys in the keys array.
//		keys		: [in] Array of keys for which values will be returned.
//		callback	: [in] Optional user-supplied function to be called when
//		 the operation has completed.
//		param		: [in] Optional pointer to user data; will be passed
//		 unmodified to the callback function.
//		blocking	: [in] If CHATTrue, return only after the operation has
//		 completed; otherwise, return immediately.
// Remarks
//		The chatGetChannelKeys function retrieves a list of key/value pairs
//		 for the specified channel or user. 
//		If the user parameter is set to a user nickname, key/value pairs
//		 will be returned only for the specified user. 
//		If the user parameter is set to "*", values on the channel itself
//		 will be returned.<p>
void chatGetChannelKeys(CHAT chat,
	const gsi_char * channel,
	const gsi_char * user,
	int num,
	const gsi_char ** keys,
	chatGetChannelKeysCallback callback,
	void * param,
	CHATBool blocking);



#ifdef GSI_UNICODE
	CHATBool chatGetBasicUserInfoNoWaitA(CHAT chat,
		const char * nick,
		const char ** user,
		const char ** address);
#endif
	/*
	void chatGetBasicUserInfoA(CHAT chat,
	const char * user,
	chatGetBasicUserInfoCallback callback,
	void * param,
	CHATBool blocking);
	*/

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}	// extern "C"
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // _CHAT_H_
