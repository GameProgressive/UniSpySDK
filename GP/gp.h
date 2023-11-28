///////////////////////////////////////////////////////////////////////////////
// File:	gp.h
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a manner not expressly authorized by IGN or GameSpy Technology 
// is prohibited.

#ifndef _GP_H_
#define _GP_H_

// This is the main header file for the GameSpy Presence and Messaging (GP) SDK.
// All structures and SDK API functions are defined in this file.

// This is necessary for gsi_char and UNICODE support.
#include "../common/gsCommon.h"

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
// Enumerated Types  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// GPEnum
// Summary
//		Presence and Messaging SDK's general enum list. These are arguments 
//		and return values for many GP functions.
typedef enum _GPEnum
{
	// Callback Types
	GP_ERROR 				= 0, // Callback called whenever GP_NETWORK_ERROR or GP_SERVER_ERROR occur.
	GP_RECV_BUDDY_REQUEST	= 1, // Callback called when another profile requests to add you to their buddy list.
	GP_RECV_BUDDY_STATUS	= 2, // Callback called when one of your buddies changes status. 
	GP_RECV_BUDDY_MESSAGE	= 3, // Callback called when someone has sent you a buddy message.
	GP_RECV_BUDDY_UTM		= 4, // Callback called when someone has sent you a UTM message.
	GP_RECV_GAME_INVITE		= 5, // Callback called when someone invites you to a game.
	GP_TRANSFER_CALLBACK	= 6, // Callback called for status updates on a file transfer. 
	GP_RECV_BUDDY_AUTH		= 7, // Callback called when someone authorizes your buddy request. 
	GP_RECV_BUDDY_REVOKE	= 8, // Callback called when another profile stops being your buddy.

	// Global states set with the gpEnable() function.
	GP_INFO_CACHING 						= 256, // 0x100, Turns on full caching of profiles with gpEnable().
	GP_SIMULATION							= 257, // 0x101, Turns on simulated GP function calls without 
												   // network traffic with gpEnable().
	GP_INFO_CACHING_BUDDY_AND_BLOCK_ONLY	= 258, // 0x102, Recommended: Turns on caching of only buddy and 
												   // blocked list profiles with gpEnable().

#ifdef _PS3
	GP_NP_SYNC = 259,  // 0x103,Turns on/off the NP to GP buddy sync   
#endif

	// Blocking settings for certain GP functions, such as gpConnect().
	GP_BLOCKING 	= 1, // Tells the function call to stop and wait for a callback.
	GP_NON_BLOCKING = 0, // Recommended: Tells the function call to return and continue processing, 
						 // but gpProcess() must be called periodically.

	// Firewall settings for the gpConnect() functions.
	GP_FIREWALL		= 1, // Sets gpConnect() to send buddy messages through the GP backend.
	GP_NO_FIREWALL	= 0, // Recommended: Sets gpConnect() to try to send buddy messages directly, 
						 // then fall back to the GP backend.

	// Cache settings for the gpGetInfo() function.
	GP_CHECK_CACHE		= 1, // Recommended: gpGetInfo() checks the local cache first for 
							 // profile data, then the GP backend.
	GP_DONT_CHECK_CACHE = 0, // gpGetInfo() only queries the GP backend for profile data.

	// Search result of gpIsValidEmail() given in GPIsValidEmailResponseArg.isValid.
	GP_VALID	= 1, // Indicates in GPIsValidEmailResponseArg.isValid that a gpIsValidEmail() 
					 // call found the specified email address.
	GP_INVALID	= 0, // Indicates in GPIsValidEmailResponseArg.isValid that a gpIsValidEmail() 
					 // call did NOT find the specified email address.

	// Error severity indicator given in GPErrorArg.fatal.
	GP_FATAL		= 1, // Indicates in GPErrorArg.fatal that a fatal GP_ERROR has occurred.
	GP_NON_FATAL	= 0, // Indicates in GPErrorArg.fatal that a non-fatal GP_ERROR has occurred.

	// Profile query result of gpGetInfo() given in GPGetInfoResponseArg.sex.
	GP_MALE 	= 1280, // 0x500, Indicates in GPGetInfoResponseArg.sex that a 
						// gpGetInfo() call returned a male profile.
	GP_FEMALE	= 1281, // 0x501, Indicates in GPGetInfoResponseArg.sex that a 
						// gpGetInfo() call returned a female profile.
	GP_PAT		= 1282, // 0x502, Indicates in GPGetInfoResponseArg.sex that a 
						// gpGetInfo() call returned an sexless profile.

	// Result of gpProfileSearch() given in GPProfileSearchResponseArg.more.
	GP_MORE = 1536, // 0x600, Indicates in GPProfileSearchResponseArg.more that a 
					// gpProfileSearch() call has more matching records.
	GP_DONE = 1537, // 0x601, Indicates in GPProfileSearchResponseArg.more that a 
					// gpProfileSearch() call has no more matching records.

	// Profile fields used in gpGetInfo() and gpSetInfo() calls.
	GP_NICK 			= 1792, // 0x700, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 30.
	GP_UNIQUENICK		= 1793, // 0x701, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 20.
	GP_EMAIL			= 1794, // 0x702, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 50.
	GP_PASSWORD			= 1795, // 0x703, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 30.
	GP_FIRSTNAME		= 1796, // 0x704, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 30.
	GP_LASTNAME			= 1797, // 0x705, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 30.
	GP_ICQUIN			= 1798, // 0x706, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_HOMEPAGE			= 1799, // 0x707, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 75.
	GP_ZIPCODE			= 1800, // 0x708, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 10.
	GP_COUNTRYCODE		= 1801, // 0x709, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit:  2.
	GP_BIRTHDAY			= 1802, // 0x70A, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_SEX				= 1803, // 0x70B, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_CPUBRANDID		= 1804, // 0x70C, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_CPUSPEED			= 1805, // 0x70D, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_MEMORY			= 1806, // 0x70E, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_VIDEOCARD1STRING	= 1807, // 0x70F, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_VIDEOCARD1RAM	= 1808, // 0x710, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_VIDEOCARD2STRING	= 1809, // 0x711, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_VIDEOCARD2RAM	= 1810, // 0x712, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_CONNECTIONID		= 1811, // 0x713, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_CONNECTIONSPEED	= 1812, // 0x714, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_HASNETWORK		= 1813, // 0x715, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_OSSTRING			= 1814, // 0x716, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_AIMNAME			= 1815, // 0x717, Profile info used in gpGetInfo() and gpSetInfo() calls, length limit: 50.
	GP_PIC				= 1816, // 0x718, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_OCCUPATIONID		= 1817, // 0x719, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_INDUSTRYID		= 1818, // 0x71A, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_INCOMEID			= 1819, // 0x71B, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_MARRIEDID		= 1820, // 0x71C, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_CHILDCOUNT		= 1821, // 0x71D, Profile info used in gpGetInfo() and gpSetInfo() calls.
	GP_INTERESTS1		= 1822, // 0x71E, Profile info used in gpGetInfo() and gpSetInfo() calls.

	// gpNewProfile() overwrite settings.
	GP_REPLACE		= 1, // Tells gpNewProfile() to overwrite a matching user profile.
	GP_DONT_REPLACE = 0, // Recommended: Tells gpNewProfile() to notify you instead of 
						 // overwriting a matching user profile.

	// Connection status indicators given by gpIsConnected().
	GP_CONNECTED		= 1, // Output by gpIsConnected() when the GPConnection object 
							 // has a connection to the server.
	GP_NOT_CONNECTED	= 0, // Output by gpIsConnected() when the GPConnection object 
							 // does NOT have a connection to the server.

	// Bitwise OR-able field visibilities set in gpSetInfoMask() and returned by the gpGetInfo() callback.
	GP_MASK_NONE        =  0, // 0x00, Indicates that none of the profile's fields are visible.
	GP_MASK_HOMEPAGE    =  1, // 0x01, Indicates that the profile's homepage field is visible.
	GP_MASK_ZIPCODE     =  2, // 0x02, Indicates that the profile's zipcode field is visible.
	GP_MASK_COUNTRYCODE =  4, // 0x04, Indicates that the profile's country code field is visible.
	GP_MASK_BIRTHDAY    =  8, // 0x08, Indicates that the profile's birthday field is visible.
	GP_MASK_SEX         = 16, // 0x10, Indicates that the profile's sex field is visible.
	GP_MASK_EMAIL       = 32, // 0x20, Indicates that the profile's email field is visible.
	GP_MASK_BUDDYLIST	= 64, // 0x40, Indicates that the profile's buddy list is visible.
	GP_MASK_ALL         = -1, // 0xFFFFFFFF, Indicates that all of a profile's fields are visible.

		// Hidden buddy list indicator returned by the gpGetProfileBuddyList() 
	// callback in GPGetProfileBuddyListArg.hidden.
	GP_HIDDEN = 1, // Indicates in GPGetProfileBuddyListArg.hidden that a 
	// gpGetProfileBuddyList() call requested a profile that hides its buddies.
	GP_NOT_HIDDEN = 0, // Indicates in GPGetProfileBuddyListArg.hidden that a 
	// gpGetProfileBuddyList() call requested a profile that does NOT hide its buddies.

	// Buddy statuses given by a gpGetBuddyStatus() call in GPBuddyStatus.status.
	GP_OFFLINE  = 0, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is not available.
	GP_ONLINE   = 1, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is available.
	GP_PLAYING  = 2, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is playing a game.
	GP_STAGING  = 3, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is getting ready to play a game.
	GP_CHATTING = 4, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is communicating.
	GP_AWAY     = 5, // Indicates in GPBuddyStatus.status that a gpGetBuddyStatus() call 
					 // found a buddy that is not at his PC.

	//DOM-IGNORE-BEGIN 
	// Session types, reserved for future use.
	GP_SESS_IS_CLOSED		=  1, // 0x01
	GP_SESS_IS_OPEN			=  2, // 0x02
	GP_SESS_HAS_PASSWORD	=  4, // 0x04
	GP_SESS_IS_BEHIND_NAT	=  8, // 0x08
	GP_SESS_IS_RANKED		= 16, // 0x10
	//DOM-IGNORE-END

	// CPU brands specified in GP_CPUBRANDID when calling gpSetInfoi().
	GP_INTEL 	= 1, // Tells gpSetInfoi() the user's GP_CPUBRANDID is Intel.
	GP_AMD		= 2, // Tells gpSetInfoi() the user's GP_CPUBRANDID is AMD.
	GP_CYRIX	= 3, // Tells gpSetInfoi() the user's GP_CPUBRANDID is Cyrix.
	GP_MOTOROLA	= 4, // Tells gpSetInfoi() the user's GP_CPUBRANDID is Motorola.
	GP_ALPHA	= 5, // Tells gpSetInfoi() the user's GP_CPUBRANDID is Alpha.

	// Internet connection types specified in GP_CONNECTIONID when calling gpSetInfoi().
	GP_MODEM 		= 1, // Tells gpSetInfoi() the user's GP_CONNECTIONID is a modem.
	GP_ISDN			= 2, // Tells gpSetInfoi() the user's GP_CONNECTIONID is ISDN.
	GP_CABLEMODEM	= 3, // Tells gpSetInfoi() the user's GP_CONNECTIONID is a cable modem.
	GP_DSL			= 4, // Tells gpSetInfoi() the user's GP_CONNECTIONID is DSL.
	GP_SATELLITE	= 5, // Tells gpSetInfoi() the user's GP_CONNECTIONID is a satellite.
	GP_ETHERNET		= 6, // Tells gpSetInfoi() the user's GP_CONNECTIONID is ethernet.
	GP_WIRELESS		= 7, // Tells gpSetInfoi() the user's GP_CONNECTIONID is wireless.

	// File transfer status update values received through GPTransferCallbackArg.type.
	GP_TRANSFER_SEND_REQUEST 	= 2048, // 0x800, Indicates in GPTransferCallbackArg.type that a 
										// remote profile wants to send files to the local profile.
	GP_TRANSFER_ACCEPTED		= 2049, // 0x801, Indicates in GPTransferCallbackArg.type that a 
										// transfer request has been accepted.
	GP_TRANSFER_REJECTED		= 2050, // 0x802, Indicates in GPTransferCallbackArg.type that a 
										// transfer request has been rejected.
	GP_TRANSFER_NOT_ACCEPTING	= 2051, // 0x803, Indicates in GPTransferCallbackArg.type that the 
										// remote profile is not accepting file transfers.
	GP_TRANSFER_NO_CONNECTION	= 2052, // 0x804, Indicates in GPTransferCallbackArg.type that a 
										// direct connection with the remote profile could not be established.
	GP_TRANSFER_DONE			= 2053, // 0x805, Indicates in GPTransferCallbackArg.type that the 
										// file transfer has finished successfully.
	GP_TRANSFER_CANCELLED		= 2054, // 0x806, Indicates in GPTransferCallbackArg.type that the 
										// file transfer has been cancelled before completing.
	GP_TRANSFER_LOST_CONNECTION	= 2055, // 0x807, Indicates in GPTransferCallbackArg.type that the 
										// direct connection with the remote profile has been lost.
	GP_TRANSFER_ERROR			= 2056, // 0x808, Indicates in GPTransferCallbackArg.type that 
										// there was an error during the transfer.
	GP_TRANSFER_THROTTLE		= 2057, // 0x809, Reserved for future use.
	GP_FILE_BEGIN				= 2058, // 0x80A, Indicates in GPTransferCallbackArg.type that a 
										// file is about to be transferred.
	GP_FILE_PROGRESS			= 2059, // 0x80B, Indicates in GPTransferCallbackArg.type that 
										// file data has been either sent or received.
	GP_FILE_END					= 2060, // 0x80C, Indicates in GPTransferCallbackArg.type that a 
										// file has finished transferring successfully.
	GP_FILE_DIRECTORY			= 2061, // 0x80D, Indicates in GPTransferCallbackArg.type that the 
										// current "file" being transferred is a directory name.
	GP_FILE_SKIP				= 2062, // 0x80E, Indicates in GPTransferCallbackArg.type that the 
										// current file is being skipped.
	GP_FILE_FAILED				= 2063, // 0x80F, Indicates in GPTransferCallbackArg.type that the 
										// current file being transferred has failed.

	// File transfer error codes received through GPTransferCallbackArg.num.
	GP_FILE_READ_ERROR	= 2304, // 0x900, Indicates in GPTransferCallbackArg.num that the 
								// sender had an error reading the file.
	GP_FILE_WRITE_ERROR	= 2305, // 0x901, Indicates in GPTransferCallbackArg.num that the 
								// sender had an error writing the file.
	GP_FILE_DATA_ERROR	= 2306, // 0x902, Indicates in GPTransferCallbackArg.num that the 
								// MD5 check of the data being transferred failed.

	// File transfer direction indicator given by gpGetTransferSide().
	GP_TRANSFER_SENDER 		= 2560, // 0xA00, Output by gpGetTransferSide() when the 
									// local profile is sending the file.
	GP_TRANSFER_RECEIVER	= 2561, // 0xA01, Output by gpGetTransferSide() when the 
									// local profile is receiving the file.

	// Flag for sending UTM messages directly (not backend routed) for gpSendBuddyUTM().
	GP_DONT_ROUTE = 2816, // 0xB00, Tells gpSendBuddyUTM() to send this UTM message 
						  // directly to the buddy, instead of routing it through the backend.

	// Bitwise OR-able quiet-mode flags used in the gpSetQuietMode() function.
	GP_SILENCE_NONE       =  0, // Indicates to gpSetQuietMode() that no message types should be silenced.
	GP_SILENCE_MESSAGES   =  1, // Indicates to gpSetQuietMode() that messages should be silenced.
	GP_SILENCE_UTMS       =  2, // Indicates to gpSetQuietMode() that UTM type messages should be silenced.
	GP_SILENCE_LIST       =  4, // Indicates to gpSetQuietMode() that list type messages should be silenced.
	GP_SILENCE_ALL        = -1, // 0xFFFFFFFF, Indicates to gpSetQuietMode() that 
								// all message types should be silenced.

	//DOM-IGNORE-BEGIN 
	// New status info settings, reserved for future use.
	GP_NEW_STATUS_INFO_SUPPORTED		= 3072, // 0xC00
	GP_NEW_STATUS_INFO_NOT_SUPPORTED	= 3073  // 0xC01
	//DOM-IGNORE-END

} GPEnum;

///////////////////////////////////////////////////////////////////////////////
// GPResult
// Summary
//		Presence and Messaging SDK's possible Results which can be returned 
//		from GP functions. Check individual function definitions to see 
//		possible results.
typedef enum _GPResult
{
	GP_NO_ERROR			= 0, // Success. 
	GP_MEMORY_ERROR		= 1, // A call to allocate memory failed, probably due to insufficient memory.
	GP_PARAMETER_ERROR	= 2, // A parameter passed to a function is either null or has an invalid value. 
	GP_NETWORK_ERROR	= 3, // An error occurred while reading or writing across the network. 
	GP_SERVER_ERROR		= 4, // One of the backend servers returned an error. 
	GP_MISC_ERROR		= 5, // An error occurred that was not covered by the other error conditions.
	GP_COUNT			= 6  // The number of GPResults; reserved for internal use.

} GPResult;

///////////////////////////////////////////////////////////////////////////////
// GPErrorCode
// Summary
//		Error codes which can occur in Presence and Messaging.
//#define GP_ERROR_TYPE(errorCode)  ((errorCode) >> 8)
typedef enum _GPErrorCode
{
	// General error codes.
	GP_GENERAL					= 0, // There was an unknown error. 
	GP_PARSE					= 1, // Unexpected data was received from the server. 
	GP_NOT_LOGGED_IN			= 2, // The request cannot be processed because user has not logged in. 
	GP_BAD_SESSKEY				= 3, // The request cannot be processed because of an invalid session key.
	GP_DATABASE					= 4, // There was a database error.
	GP_NETWORK					= 5, // There was an error connecting a network socket.
	GP_FORCED_DISCONNECT		= 6, // This profile has been disconnected by another login.
	GP_CONNECTION_CLOSED		= 7, // The server has closed the connection.
	GP_UDP_LAYER				= 8, // There was a problem with the UDP layer.

	// Error codes that can occur while logging in.
	GP_LOGIN						= 256, // 0x100, There was an error logging in to the GP backend.
	GP_LOGIN_TIMEOUT				= 257, // 0x101, The login attempt timed out.
	GP_LOGIN_BAD_NICK				= 258, // 0x102, The nickname provided was incorrect.
	GP_LOGIN_BAD_EMAIL				= 259, // 0x103, The email address provided was incorrect.
	GP_LOGIN_BAD_PASSWORD			= 260, // 0x104, The password provided was incorrect.
	GP_LOGIN_BAD_PROFILE			= 261, // 0x105, The profile provided was incorrect.
	GP_LOGIN_PROFILE_DELETED		= 262, // 0x106, The profile has been deleted.
	GP_LOGIN_CONNECTION_FAILED		= 263, // 0x107, The server has refused the connection.
	GP_LOGIN_SERVER_AUTH_FAILED		= 264, // 0x108, The server could not be authenticated.
	GP_LOGIN_BAD_UNIQUENICK			= 265, // 0x109, The uniquenick provided was incorrect.
	GP_LOGIN_BAD_PREAUTH			= 266, // 0x10A, There was an error validating the pre-authentication.
	GP_LOGIN_BAD_LOGIN_TICKET		= 267, // 0x10B, The login ticket was unable to be validated.
	GP_LOGIN_EXPIRED_LOGIN_TICKET	= 268, // 0x10C, The login ticket had expired and could not be used.

	// Error codes that can occur while creating a new user.
	GP_NEWUSER						= 512, // 0x200, There was an error creating a new user.
	GP_NEWUSER_BAD_NICK				= 513, // 0x201, A profile with that nick already exists.
	GP_NEWUSER_BAD_PASSWORD			= 514, // 0x202, The password does not match the email address.
	GP_NEWUSER_UNIQUENICK_INVALID	= 515, // 0x203, The uniquenick is invalid.
	GP_NEWUSER_UNIQUENICK_INUSE		= 516, // 0x204, The uniquenick is already in use.

	// Error codes that can occur while updating user information.
	GP_UPDATEUI						= 768, // 0x300, There was an error updating the user information.
	GP_UPDATEUI_BAD_EMAIL			= 769, // 0x301, A user with the email address provided already exists.

	// Error codes that can occur while creating a new profile.
	GP_NEWPROFILE					= 1024, // 0x400, There was an error creating a new profile.
	GP_NEWPROFILE_BAD_NICK			= 1025, // 0x401, The nickname to be replaced does not exist.
	GP_NEWPROFILE_BAD_OLD_NICK		= 1026, // 0x402, A profile with the nickname provided already exists.

	// Error codes that can occur while updating profile information.
	GP_UPDATEPRO					= 1280, // 0x500, There was an error updating the profile information. 
	GP_UPDATEPRO_BAD_NICK			= 1281, // 0x501, A user with the nickname provided already exists.

	// Error codes that can occur while adding someone to your buddy list.
	GP_ADDBUDDY						= 1536, // 0x600, There was an error adding a buddy. 
	GP_ADDBUDDY_BAD_FROM			= 1537, // 0x601, The profile requesting to add a buddy is invalid.
	GP_ADDBUDDY_BAD_NEW				= 1538, // 0x602, The profile requested is invalid.
	GP_ADDBUDDY_ALREADY_BUDDY		= 1539, // 0x603, The profile requested is already a buddy.
	GP_ADDBUDDY_IS_ON_BLOCKLIST = 1540,		// 0x604, The profile requested is on the local profile's block list.
	//DOM-IGNORE-BEGIN 
	GP_ADDBUDDY_IS_BLOCKING = 1541,			// 0x605, Reserved for future use.
	//DOM-IGNORE-END
	// 
	// Error codes that can occur while being authorized to add someone to your buddy list.
	GP_AUTHADD						= 1792, // 0x700, There was an error authorizing an add buddy request.
	GP_AUTHADD_BAD_FROM				= 1793, // 0x701, The profile being authorized is invalid. 
	GP_AUTHADD_BAD_SIG				= 1794, // 0x702, The signature for the authorization is invalid.
	GP_AUTHADD_IS_ON_BLOCKLIST = 1795,		// 0x703, The profile requesting authorization is on a block list.
	//DOM-IGNORE-BEGIN 
	GP_AUTHADD_IS_BLOCKING = 1796,			// 0x704, Reserved for future use.
	//DOM-IGNORE-END
	// 
	// Error codes that can occur with status messages.
	GP_STATUS						= 2048, // 0x800, There was an error with the status string.

	// Error codes that can occur while sending a buddy message.
	GP_BM							= 2304, // 0x900, There was an error sending a buddy message.
	GP_BM_NOT_BUDDY					= 2305, // 0x901, The profile the message was to be sent to is not a buddy.
	GP_BM_EXT_INFO_NOT_SUPPORTED	= 2306, // 0x902, The profile does not support extended info keys.
	GP_BM_BUDDY_OFFLINE				= 2307, // 0x903, The buddy to send a message to is offline.

	// Error codes that can occur while getting profile information.
	GP_GETPROFILE					= 2560, // 0xA00, There was an error getting profile info. 
	GP_GETPROFILE_BAD_PROFILE		= 2561, // 0xA01, The profile info was requested on is invalid.

	// Error codes that can occur while deleting a buddy.
	GP_DELBUDDY						= 2816, // 0xB00, There was an error deleting the buddy.
	GP_DELBUDDY_NOT_BUDDY			= 2817, // 0xB01, The buddy to be deleted is not a buddy. 

	// Error codes that can occur while deleting your profile.
	GP_DELPROFILE					= 3072, // 0xC00, There was an error deleting the profile.
	GP_DELPROFILE_LAST_PROFILE		= 3073, // 0xC01, The last profile cannot be deleted.

	// Error codes that can occur while searching for a profile.
	GP_SEARCH						= 3328, // 0xD00, There was an error searching for a profile.
	GP_SEARCH_CONNECTION_FAILED		= 3329, // 0xD01, The search attempt failed to connect to the server.
	GP_SEARCH_TIMED_OUT				= 3330, // 0XD02, The search did not return in a timely fashion.

	// Error codes that can occur while checking whether a user exists.
	GP_CHECK						= 3584, // 0xE00, There was an error checking the user account.
	GP_CHECK_BAD_EMAIL				= 3585, // 0xE01, No account exists with the provided email address.
	GP_CHECK_BAD_NICK				= 3586,	// 0xE02, No such profile exists for the provided email address.
	GP_CHECK_BAD_PASSWORD			= 3587, // 0xE03, The password is incorrect.

	// Error codes that can occur while revoking buddy status.
	GP_REVOKE						= 3840, // 0xF00, There was an error revoking the buddy.
	GP_REVOKE_NOT_BUDDY				= 3841, // 0xF01, You are not a buddy of the profile.

	// Error codes that can occur while registering a new unique nick.
	GP_REGISTERUNIQUENICK				= 4096, // 0x1000, There was an error registering the uniquenick.
	GP_REGISTERUNIQUENICK_TAKEN			= 4097, // 0x1001, The uniquenick is already taken.
	GP_REGISTERUNIQUENICK_RESERVED		= 4098, // 0x1002, The uniquenick is reserved. 
	GP_REGISTERUNIQUENICK_BAD_NAMESPACE	= 4099, // 0x1003, Tried to register a nick with no namespace set. 

	// Error codes that can occur while registering a CDKey.
	GP_REGISTERCDKEY				= 4352, // 0x1100, There was an error registering the cdkey.
	GP_REGISTERCDKEY_BAD_KEY		= 4353, // 0x1101, The cdkey is invalid. 
	GP_REGISTERCDKEY_ALREADY_SET	= 4354, // 0x1102, The profile has already been registered with a different cdkey.
	GP_REGISTERCDKEY_ALREADY_TAKEN	= 4355, // 0x1103, The cdkey has already been registered to another profile. 

	// Error codes that can occur while adding someone to your block list.
	GP_ADDBLOCK						= 4608, // 0x1200, There was an error adding the player to the blocked list. 
	GP_ADDBLOCK_ALREADY_BLOCKED		= 4609, // 0x1201, The profile specified is already blocked.

	// Error codes that can occur while removing someone from your block list.
	GP_REMOVEBLOCK					= 4864, // 0x1300, There was an error removing the player from the blocked list. 
	GP_REMOVEBLOCK_NOT_BLOCKED		= 4865  // 0x1301, The profile specified was not a member of the blocked list.

} GPErrorCode;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Constants
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GP_NICK_LEN                 31
#define GP_UNIQUENICK_LEN           21
#define GP_FIRSTNAME_LEN            31
#define GP_LASTNAME_LEN             31
#define GP_EMAIL_LEN                51
#define GP_PARNTERBUFFER_LEN        11
#define GP_PASSWORD_LEN             31
#define GP_PASSWORDENC_LEN          ((((GP_PASSWORD_LEN+2)*4)/3)+1)
#define GP_HOMEPAGE_LEN             76
#define GP_ZIPCODE_LEN              11
#define GP_COUNTRYCODE_LEN          3
#define GP_PLACE_LEN                128
#define GP_AIMNAME_LEN              51
#define GP_REASON_LEN               1025
#define GP_STATUS_STRING_LEN        256
#define GP_LOCATION_STRING_LEN      256
#define GP_ERROR_STRING_LEN         256
#define GP_AUTHTOKEN_LEN            256
#define GP_PARTNERCHALLENGE_LEN     256
#define GP_CDKEY_LEN                65
#define GP_CDKEYENC_LEN             ((((GP_CDKEY_LEN+2)*4)/3)+1)
#define GP_LOGIN_TICKET_LEN         25

#define GP_RICH_STATUS_LEN          256
#define GP_STATUS_BASIC_STR_LEN     33

// Random number seed for PASSWORDENC and CDKEYENC 
//   MUST MATCH SERVER - If you change this, you'll have to 
//                       release an updated server.
#define GP_XOR_SEED                 0x79707367 // "gspy"

// Maximum number of namespaces that can be searched for a uniquenick.
#define GP_MAX_NAMESPACEIDS         16

// Well known values for partner ID.
#define GP_PARTNERID_GAMESPY		0
#define GP_PARTNERID_IGN			10

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Types
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// GPConnection
// Summary
//		An instance of this type represents a GP connection. It is created at the 
//		beginning of a GP session with a call to gpInitialize. A pointer to a GP 
//		object is passed as the first argument to every GP function.
// See Also
//		gpInitialize
typedef void * GPConnection;

///////////////////////////////////////////////////////////////////////////////
// GPProfile
// Summary
//		An instance of this type represents a particular GP profile, such as the 
//		profile that the local user is logged into or queried information about 
//		another user's profile. A GPProfile object is passed to functions like 
//		gpSendBuddyMessage and gpGetInfo, and it is returned in callbacks such 
//		as the GP_RECV_BUDDY_STATUS callback. A GPProfile object is equivalent to 
//		a profile ID. They are both int types, and can be used interchangeably.
typedef int GPProfile;

///////////////////////////////////////////////////////////////////////////////
// GPTransfer
// Summary
//		An instance of this type represents a particular buddy-to-buddy direct 
//		file transfer created with a call to gpSendFiles and managed by various 
//		gpGetTransfer and gpSetTransfer functions.
// See Also
//		gpSendFiles
typedef int GPTransfer;

///////////////////////////////////////////////////////////////////////////////
// GPCallback
// Summary
//		A generic callback function type used to specify the callback supplied to 
//		GP SDK functions often with gpSetCallback.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize.
//		arg			: [in] A pointer to a response structure whose content depends on 
//							the task in progress
//		param		: [in] The user-data, if any, that was passed into the function 
//							that triggered this callback event.
// Remarks
//		This isn't an actual function, but a type of function which your callback
//		function must adhere to. The arg parameter content varies depending on the 
//		task. For example, a callback that is specified when calling gpGetInfo() 
//		should cast its incoming arg pointer to type GPGetInfoResponseArg.
// See Also
//		gpSetCallback
typedef void (* GPCallback)(
  GPConnection * connection,
  void * arg,
  void * param
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Structures
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// GPErrorArg
// Summary
//		Contains information about an error which has occurred.
typedef struct
{
	GPResult		result;		// The result of a call to a GP function; GP_NO_ERROR if successful. 
	GPErrorCode	errorCode;		// The specific cause of the error. 
	gsi_char *	errorString;	// A readable text string representation of the errorCode. 
	GPEnum		fatal;			// Either GP_FATAL or GP_NON_FATAL to indicate whether error is fatal. 

} GPErrorArg;

///////////////////////////////////////////////////////////////////////////////
// GPConnectResponseArg
// Summary
//		The arg parameter passed through to a GPCallback call after attempting 
//		to connect is of this type.
typedef struct
{
	GPResult result;						// The result of a call to a GP function; GP_NO_ERROR if successful. 
	GPProfile profile;						// The profile of the user being connected.
	gsi_char uniquenick[GP_UNIQUENICK_LEN];	// The uniquenick for the newly connected user. 

} GPConnectResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPNewUserResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpNewUser is of this type.
typedef struct
{
	GPResult result;	// The result of the creation attempt; GP_NO_ERROR if successful.
	GPProfile profile;	// The profile created for the new user, if successful.

} GPNewUserResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPCheckResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpCheckUser is of this type.
typedef struct
{
	GPResult result;	// The result of the check; GP_NO_ERROR if successful.
	GPProfile profile;	// The profile of the user being checked. 

} GPCheckResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPSuggestUniqueNickResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpSuggestUniqueNick is of this type.
typedef struct
{
	GPResult result;			// The result of the suggest uniquenick operation; GP_NO_ERROR if successful.
	int numSuggestedNicks;		// The number of suggested uniquenicks contained in this struct.
	gsi_char ** suggestedNicks;	// An array of suggested uniquenicks. The number of elements in the array is 
								// specified by numSuggestedNicks.

} GPSuggestUniqueNickResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPRegisterUniqueNickResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpRegisterUniqueNick is of this type.
typedef struct
{
	GPResult result;	// The result of the register uniquenick operation; GP_NO_ERROR if successful. 

} GPRegisterUniqueNickResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPRegisterCdKeyResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpRegisterCdKey is of this type.
//		<emit \<ul\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY				= 4352, // 0x1100, There was an error registering the cdkey.
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_BAD_KEY		= 4353, // 0x1101, The cdkey is invalid. 
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_ALREADY_SET	= 4354, // 0x1102, The profile has already been registered with a different cdkey.
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_ALREADY_TAKEN	= 4355, // 0x1103, The cdkey has already been registered to another profile.
//			<emit \</li\>>
//		<emit \</ul\>>
typedef struct
{
	GPResult result;	// The result of the register CDKey operation; GP_NO_ERROR if successful. 

} GPRegisterCdKeyResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPNewProfileResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpNewProfile is of this type.
typedef struct
{
	GPResult result;	// The result of the create new profile operation; GP_NO_ERROR if successful.
	GPProfile profile;	// The newly created profile, if successful.

} GPNewProfileResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPDeleteProfileResponseArg
// Summary
//		This arg data type contains the data for a delete profile operation. 
//		It is generated by a call to the callback passed to gpDeleteProfile.
typedef struct  
{
	GPResult result;	// The result of the delete profile operation; GP_NO_ERROR if successful.
	GPProfile profile;	// The deleted profile, if successful.

} GPDeleteProfileResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPProfileSearchMatch
// Summary
//		Information about a profile which is returned by a requested search. 
//		These structs are often collected in lists, such as those found in 
//		GPGetReverseBuddiesResponseArg or GPProfileSearchResponseArg.
typedef struct
{
	GPProfile profile;						// The matching profile.
	gsi_char nick[GP_NICK_LEN];				// The profile's nickname.
	gsi_char uniquenick[GP_UNIQUENICK_LEN];	// The profile's uniquenick.
	int namespaceID;						// The namespace in which this profile lives.
	gsi_char firstname[GP_FIRSTNAME_LEN];	// The first name associated with the profile.
	gsi_char lastname[GP_LASTNAME_LEN];		// The last name associated with the profile.
	gsi_char email[GP_EMAIL_LEN];			// The email address associated with the profile.

} GPProfileSearchMatch;

///////////////////////////////////////////////////////////////////////////////
// GPProfileSearchResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpProfileSearch is of this type. Contains information about the 
//		profiles that matched the search criteria.
typedef struct
{
	GPResult result;					// The result of the profile search; GP_NO_ERROR if successful. 
	int numMatches;						// The number of profiles in this set of matches.
	GPEnum more;						// GP_MORE if there is another set of matches; GP_DONE if this is the last (or only) set of matches.
	GPProfileSearchMatch * matches;		// A set of profile search matches.

} GPProfileSearchResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPGetInfoResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpGetInfo is of this type. The structure provides information about 
//		the specified profile.
typedef struct
{
	GPResult result;							// The result of the inquiry; GP_NO_ERROR if successful. 
	GPProfile profile;							// The profile for which further information was requested.
	gsi_char nick[GP_NICK_LEN];					// The nick associated with this profile.
	gsi_char uniquenick[GP_UNIQUENICK_LEN];		// The uniquenick associated with this profile.
	gsi_char email[GP_EMAIL_LEN];				// The email address associated with this profile.
	gsi_char firstname[GP_FIRSTNAME_LEN];		// The first name associated with this profile.
	gsi_char lastname[GP_LASTNAME_LEN];			// The last name associated with this profile.
	gsi_char homepage[GP_HOMEPAGE_LEN];			// The web page associated with this profile.
	int icquin;									// The ICQ UIN (User Identification Number) associated with this profile.
	gsi_char zipcode[GP_ZIPCODE_LEN];			// The ZIP (postal) code associated with this profile.
	gsi_char countrycode[GP_COUNTRYCODE_LEN];	// The country code associated with this profile.
	float longitude;							// The longitude associated with this profile. Negative is west; 
												// positive is east; 0,0 means unknown.
	float latitude;								// The latitude associated with this profile. Negative is south; 
												// positive is north; 0,0 means unknown.
	gsi_char place[GP_PLACE_LEN];				// A place name string associated with this profile (e.g., "USA|California|Irvine", 
												// "South Korea|Seoul", or "Turkey").
	int birthday;								// The birth day of month (1-31) associated with this profile.
	int birthmonth;								// The birth month (1-12) associated with this profile.
	int birthyear;								// The birth year associated with this profile.
	GPEnum sex;									// An enum indicating the sex associated with this profile info: 
												// GP_MALE (male), GP_FEMALE (female), or GP_PAT (unknown).
	GPEnum publicmask;							// A collection of bitwise-ORable flags indicating which fields 
												// of this profile's info are publicly visible. If the value of 
												// publicmask is GP_MASK_NONE then no fields are visible. If it 
												// is GP_MASK_ALL then all of the mask-able fields are visible. 
												// If any of the following bits are set, then the corresponding 
												// field is visible. If the bit is not set, then the field is 
												// hidden/masked:
												// <p />
												// GP_MASK_HOMEPAGE:    The web page associated with this profile.<p />
												// GP_MASK_ZIPCODE:     The ZIP (postal) code associated with this profile.<p />
												// GP_MASK_COUNTRYCODE: The country code associated with this profile.<p />
												// GP_MASK_BIRTHDAY:    The birthday fields associated with this profile.<p />
												// GP_MASK_SEX:         The sex associated with this profile.<p />
												// If the flag for a field is not set, then its value in the 
												// structure should not be used. For example, if the GP_MASK_BIRTHDAY 
												// bit is not set, the birthday, birth month, and birth year fields 
												// will not be available.
	gsi_char aimname[GP_AIMNAME_LEN];			// The AOL IM screen name associated with this profile.
	int pic;									// The GameSpy Comrade/Arcade profile picture associated with this 
												// profile.
	int occupationid;							// The occupation id associated with this profile. 
	int industryid;								// The industry id associated with this profile. 
	int incomeid;								// The income associated with this profile. 
	int marriedid;								// The marital status associated with this profile. 
	int childcount;								// The number of children associated with this profile. 
	int interests1;								// The bit-packed interest values associated with this profile. 
	int ownership1;								// The bit-packed owned platform values associated with this profile.
	int conntypeid;								// The connection type associated with this profile. 
} GPGetInfoResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyRequestArg
// Summary
//		Information sent to the GP_RECV_BUDDY_REQUEST callback.
typedef struct
{ 
	GPProfile profile;				// The profile of the buddy who has made the request. 
	unsigned int date;				// The timestamp of the request, represented as seconds elapsed 
									// since 00:00:00 January 1st, 1970 UTC. 
	gsi_char reason[GP_REASON_LEN];	// The reason for the request. 

} GPRecvBuddyRequestArg;

///////////////////////////////////////////////////////////////////////////////
// GPBuddyStatus
// Summary
//		The availability level of a buddy.
typedef struct
{ 
	GPProfile profile;								// The profile of the buddy.
	GPEnum status;									// A value of GPEnum which represents the "Status" of the buddy.
	gsi_char statusString[GP_STATUS_STRING_LEN];	// The buddy "Status" in human-readable form.
	gsi_char locationString[GP_LOCATION_STRING_LEN];// A URL indicating the game location of the buddy in the 
													// form "gamename://IP.address:port/extra/info". 
	unsigned int ip;								// The buddy's IP address in network byte order (big-endian). 
													// This is used for buddy-to-buddy messaging.
	int port;										// The buddy's TCP listening port. If this is 0, the buddy is 
													// behind a firewall. This is used for buddy-to-buddy messaging. 
	GPEnum quietModeFlags;							// A set of bit-flags indicating what message types are silenced 
													// by this buddy.

} GPBuddyStatus;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyStatusArg
// Summary
//		This structure indicates that a buddy's status has changed in the 
//		GP_RECV_BUDDY_STATUS callback without providing the new status. 
//		A separate call must be made to gpGetBuddyStatus from inside the 
//		callback to get the buddy's new status.
typedef struct
{
	GPProfile profile;	// The profile of the buddy whose status has changed. 
	unsigned int date;	// The timestamp of the status change, represented as seconds elapsed 
						// since 00:00:00 January 1st, 1970 UTC.
	int index;			// This buddy's index in the buddy list. This index can be used in a call 
						// to gpGetBuddyStatus from inside the callback to get the buddy's new status.

} GPRecvBuddyStatusArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyMessageArg
// Summary
//		A buddy message received in the GP_RECV_BUDDY_MESSAGE callback.
typedef struct
{ 
	GPProfile profile;	// The profile of the buddy who sent the message.
	unsigned int date;	// The timestamp of the message, represented as seconds elapsed 
						// since 00:00:00 January 1st, 1970 UTC.
	gsi_char * message;	// The text of the message.

} GPRecvBuddyMessageArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyUTMArg
// Summary
//		A buddy UTM (Under-the-Table Message) received in the 
//		GP_RECV_BUDDY_UTM callback.
typedef struct 
{
	GPProfile profile;	// The profile of the buddy who sent the UTM.
	unsigned int date;	// The timestamp of the UTM, represented as seconds elapsed 
						// since 00:00:00 January 1st, 1970 UTC.
	gsi_char * message;	// The text of the UTM.

} GPRecvBuddyUTMArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyAuthArg
// Summary
//		An authorization add the profile as a buddy, received in the 
//		GP_RECV_BUDDY_AUTH callback.
typedef struct  
{
	GPProfile profile;	// The profile that authorized the request.
	unsigned int date;	// The timestamp of when the auth was accepted, represented as seconds elapsed 
						// since 00:00:00 January 1st, 1970 UTC. 

} GPRecvBuddyAuthArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvBuddyRevokeArg
// Summary
//		A revocation of buddy status, received in the GP_RECV_BUDDY_REVOKE 
//		callback.
typedef struct 
{
	GPProfile profile;	// The profile that revoked buddy status.
	unsigned int date;	// The timestamp of when the revocation occurred, represented as seconds elapsed 
						// since 00:00:00 January 1st, 1970 UTC.

} GPRecvBuddyRevokeArg;

///////////////////////////////////////////////////////////////////////////////
// GPTransferCallbackArg;
// Summary
//		The arg parameter passed to a Transfer Callback.
typedef struct
{
	GPTransfer transfer;	// The transfer object this callback is for. 
	GPEnum type;			// The type of information being passed to the application. See the 
							// "Transfer callback type" section of GPEnum. 
	int index;				// If this callback is related to a specific file being transferred, 
							// this is that file's index. 
	int num;				// An integer used in conjunction with certain "type" values to pass 
							// supplementary information to the program. 
	gsi_char * message;		// If the type is GP_TRANSFER_SEND_REQUEST, GP_TRANSFER_ACCEPTED, 
							// or GP_TRANSFER_REJECTED, then this may point to a user-readable 
							// text message sent with the request or reply. The message will be 
							// invalid once this callback returns.

} GPTransferCallbackArg;

///////////////////////////////////////////////////////////////////////////////
// GPIsValidEmailResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpIsValidEmail is of this type.
typedef struct
{
	GPResult result;
	gsi_char email[GP_EMAIL_LEN];
	GPEnum isValid;

} GPIsValidEmailResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPGetUserNicksResponseArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpGetUserNicks is of this type.
typedef struct
{
	GPResult result;				// The result of the get nicks query; GP_NO_ERROR if successful. 
	gsi_char email[GP_EMAIL_LEN];	// The email address that was queried. 
	int numNicks;					// The number of profiles found to match the given email/password. 
									// If 0, then the email and password did not match. If you are unsure 
									// if the email address passed to gpGetUserNicks is valid, call 
									// gpIsValidEmail first. Then a value of 0 numNicks will always mean 
									// that the email address was valid but the password was incorrect. 
	gsi_char ** nicks;				// The list of nicknames for the queried profile, numNicks in length.
	gsi_char ** uniquenicks;		// The list of profile uniquenicks, numNicks in length.

} GPGetUserNicksResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPRecvGameInviteArg
// Summary
//		An invitation to a game received in the GP_RECV_GAME_INVITE callback.
typedef struct
{
	GPProfile profile;							// The profile of the buddy who sent the invite. 
	int productID;								// The product ID of the game to which the remote 
												// profile is inviting the local profile. 
	gsi_char location[GP_LOCATION_STRING_LEN];	// The game location string for the game to which one is being invited 
												// in the form "gamename://ip.address:port/extra/info". 

} GPRecvGameInviteArg;

///////////////////////////////////////////////////////////////////////////////
// GPGetProfileBuddyListArg
// Summary
//		The arg parameter passed to a callback generated by a call to 
//		gpGetProfileBuddyList.
typedef struct
{
	GPResult result;			// The result of the buddy query; GP_NO_ERROR if successful. 
	GPProfile profileQueried;	// The profile that owns this buddy list. 
	GPEnum hidden;				// GP_NOT_HIDDEN is returned when the queried profile allows others to 
								// view their buddy list. If others are not allowed to view their buddy 
								// list, GP_HIDDEN is returned, numProfiles is 0, and profiles is NULL. 
	int numProfiles;			// The number of profiles returned. A profile can have no buddies so 0 
								// can be returned. 
	GPProfile * profiles;		// The list of profiles found. 

} GPGetProfileBuddyListArg;

///////////////////////////////////////////////////////////////////////////////
// GPGetReverseBuddiesResponseArg
// Summary
//		The arg parameter result of a reverse buddy lookup ("Who has me as 
//		their buddy?") done with gpGetReverseBuddies.
typedef struct
{
	GPResult result;					// The result of the reverse buddy query; GP_NO_ERROR if successful. 
	int numProfiles;					// The number of profiles that have the queried profile on their 
										// buddy list. Zero if none were found.
	GPProfileSearchMatch * profiles;	// A list of profiles of length numProfiles of those who had
										// the queried profile on their buddy list.

} GPGetReverseBuddiesResponseArg;

///////////////////////////////////////////////////////////////////////////////
// GPUniqueMatch
// Summary
//		A structure to hold both a profile and a corresponding unique nick 
//		for searches that return lists that include both for each search hit.
typedef struct  
{
	GPProfile profile;						// A profile that matched the search hit.
	gsi_char uniqueNick[GP_UNIQUENICK_LEN];	// A unique nick belonging to that profile.

} GPUniqueMatch;

///////////////////////////////////////////////////////////////////////////////
// GPGetReverseBuddiesListResponseArg
// Summary
//		The arg parameter result of a reverse buddy lookup ("Who has me as 
//		their buddy?") done with gpGetReverseBuddiesList.
typedef struct  
{
	GPResult result;			// The result of the reverse buddy query; GP_NO_ERROR if successful. 
	int numOfUniqueMatches;		// The number of profiles with uniquenicks that have the queried 
								// profile on their buddy list. 0 if none were found.
	GPUniqueMatch *matches;		// A list of profiles plus uniquenicks of length numOfUniqueMatches 
								// of those who had the queried profile on their buddy list.

} GPGetReverseBuddiesListResponseArg;

//DOM-IGNORE-BEGIN 
///////////////////////////////////////////////////////////////////////////////
// GPBuddyStatusInfo
// Summary
//		The new buddy status info, reserved for future use.
typedef struct
{
	GPProfile profile;								// The profile of the buddy.
	GPEnum statusState;								// A value of GPEnum which represents the "Status" of the buddy.
	unsigned int buddyIp;							// 
	unsigned short buddyPort;						// 
	unsigned int hostIp;							// 
	unsigned int hostPrivateIp;						// 
	unsigned short queryPort;						// 
	unsigned short hostPort;						// 
	unsigned int sessionFlags;						// 
	gsi_char richStatus[GP_RICH_STATUS_LEN];		// The buddy "Status" in human-readable form.
	gsi_char gameType[GP_STATUS_BASIC_STR_LEN];		// 
	gsi_char gameVariant[GP_STATUS_BASIC_STR_LEN];	// 
	gsi_char gameMapName[GP_STATUS_BASIC_STR_LEN];	// 
	GPEnum quietModeFlags;							// A set of bit-flags indicating what message types are silenced 
													// by this buddy.
	GPEnum newStatusInfoFlag;						// 

} GPBuddyStatusInfo;
//DOM-IGNORE-END

//DOM-IGNORE-BEGIN 
///////////////////////////////////////////////////////////////////////////////
// GPGetBuddyStatusInfoKeysArg
// Summary
//		The result of a new buddy status keys request, reserved for future use.
typedef struct  
{
	GPProfile profile;	// The profile of the buddy from whom keys were requested.
	gsi_char **keys;	// A list of the requested keys.
	gsi_char **values;	// A list of the corresponding values for those keys.
	int numKeys;		// The number of keys returned.

} GPGetBuddyStatusInfoKeysArg;
//DOM-IGNORE-END

//DOM-IGNORE-BEGIN 
///////////////////////////////////////////////////////////////////////////////
// GPFindPlayerMatch
// Summary
//		Part of the old gpFindPlayers functionality, which has been deprecated 
//		and mostly removed.
typedef struct
{
	GPProfile profile;
	gsi_char nick[GP_NICK_LEN];
	GPEnum status;
	gsi_char statusString[GP_STATUS_STRING_LEN];

} GPFindPlayerMatch;
//DOM-IGNORE-END

//DOM-IGNORE-BEGIN 
///////////////////////////////////////////////////////////////////////////////
// GPFindPlayersResponseArg
// Summary
//		Part of the old gpFindPlayers functionality, which has been deprecated 
//		and mostly removed.
typedef struct
{
	GPResult result;	// The result of the query; GP_NO_ERROR if successful. 
	int productID;  
	int numMatches;
	GPFindPlayerMatch * matches;
} GPFindPlayersResponseArg;
//DOM-IGNORE-END

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// The hostname of the connection manager server. If the app resolves 
// this hostname, the IP(s) can be stored in this string before 
// calling gpInitialize.
extern char GPConnectionManagerHostname[64];

// The hostname of the search manager server. If the app resolves 
// this hostname, the IP(s) can be stored in this string before 
// calling gpInitialize.
extern char GPSearchManagerHostname[64];

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef GSI_UNICODE
#define gpConnect					gpConnectA
#define gpConnectNewUser			gpConnectNewUserA
#define gpConnectUniqueNick			gpConnectUniqueNickA
#define gpConnectPreAuthenticated	gpConnectPreAuthenticatedA
#define gpConnectLoginTicket		gpConnectLoginTicketA
#define gpCheckUser					gpCheckUserA
#define gpNewUser					gpNewUserA
#define gpSuggestUniqueNick			gpSuggestUniqueNickA
#define gpRegisterUniqueNick		gpRegisterUniqueNickA
#define gpRegisterCdKey				gpRegisterCdKeyA
#define gpGetErrorString			gpGetErrorStringA
#define gpNewProfile				gpNewProfileA
#define gpProfileSearch				gpProfileSearchA
#define gpProfileSearchUniquenick	gpProfileSearchUniquenickA
#define gpSetInfos					gpSetInfosA
#define gpSendBuddyRequest			gpSendBuddyRequestA
#ifndef GP_NEW_STATUS_INFO
#define gpSetStatus					gpSetStatusA
#endif
#ifdef GP_NEW_STATUS_INFO
// BETA
#define gpSetStatusInfo				gpSetStatusInfoA
#endif
#define gpSendBuddyMessage			gpSendBuddyMessageA
#define gpSendBuddyUTM				gpSendBuddyUTMA
#define gpIsValidEmail				gpIsValidEmailA
#define gpGetUserNicks				gpGetUserNicksA
#define gpSetInfoCacheFilename		gpSetInfoCacheFilenameA
#define gpSendFiles					gpSendFilesA
#define gpAcceptTransfer			gpAcceptTransferA
#define gpRejectTransfer			gpRejectTransferA
#define gpSetTransferDirectory		gpSetTransferDirectoryA
#define gpGetFileName				gpGetFileNameA
#define gpGetFilePath				gpGetFilePathA
#define gpInvitePlayer				gpInvitePlayerA
#ifdef GP_NEW_STATUS_INFO
// BETA
#define gpAddStatusInfoKey			gpAddStatusInfoKeyA
#define gpSetStatusInfoKey			gpSetStatusInfoKeyA
#define gpGetStatusInfoKeyVal		gpGetStatusInfoKeyValA
#define gpDelStatusInfoKey			gpDelStatusInfoKeyA
#endif
#else
#define gpConnect					gpConnectW
#define gpConnectNewUser			gpConnectNewUserW
#define gpConnectUniqueNick			gpConnectUniqueNickW
#define gpConnectPreAuthenticated	gpConnectPreAuthenticatedW
#define gpCheckUser					gpCheckUserW
#define gpNewUser					gpNewUserW
#define gpSuggestUniqueNick			gpSuggestUniqueNickW
#define gpRegisterUniqueNick		gpRegisterUniqueNickW
#define gpRegisterCdKey				gpRegisterCdKeyW
#define gpGetErrorString			gpGetErrorStringW
#define gpNewProfile				gpNewProfileW
#define gpProfileSearch				gpProfileSearchW
#define gpProfileSearchUniquenick	gpProfileSearchUniquenickW
#define gpSetInfos					gpSetInfosW
#define gpSendBuddyRequest			gpSendBuddyRequestW
#ifndef GP_NEW_STATUS_INFO
#define gpSetStatus					gpSetStatusW
#endif
#ifdef GP_NEW_STATUS_INFO
// BETA
#define gpSetStatusInfo				gpSetStatusInfoW
#endif
#define gpSendBuddyMessage			gpSendBuddyMessageW
#define gpSendBuddyUTM				gpSendBuddyUTMW
#define gpIsValidEmail				gpIsValidEmailW
#define gpGetUserNicks				gpGetUserNicksW
#define gpSetInfoCacheFilename		gpSetInfoCacheFilenameW
#define gpSendFiles					gpSendFilesW
#define gpAcceptTransfer			gpAcceptTransferW
#define gpRejectTransfer			gpRejectTransferW
#define gpSetTransferDirectory		gpSetTransferDirectoryW
#define gpGetFileName				gpGetFileNameW
#define gpGetFilePath				gpGetFilePathW
#define gpInvitePlayer				gpInvitePlayerW
// #ifdef GP_NEW_STATUS_INFO
// BETA
// #define gpAddStatusInfoKey       gpAddStatusInfoKeyW
// #define gpSetStatusInfoKey       gpSetStatusInfoKeyW
// #define gpGetStatusInfoKeyVal    gpGetStatusInfoKeyValW
// #define gpDelStatusInfoKey       gpDelStatusInfoKeyW
// #endif
#endif

///////////////////////////////////////////////////////////////////////////////
// gpInitialize
// Summary
//		This function is used to initialize a connection object.
// Parameters
//		connection	: [in] A GPConnection object.
//		productID	: [in] The application's product ID. 
//		namespaceID	: [in] The application's namespace ID. This is typically 
//							set to the value defined by 
//							GSI_NAMESPACE_GAMESPY_DEFAULT.
//		partnerID	: [in] The application's partner ID. This is typically set
//							to the value defined by 
//							GSI_PARTNERID_GAMESPY_DEFAULT.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid
//		GPResult is returned.
// Remarks
//      This function initializes a connection object. As long as there are no 
//		errors, this object should stay valid until gpDestroy is called. After 
//		the object is initialized by this function, callbacks can be set for 
//		the connection using gpSetCallback and states such as info-caching can
//		be turned on using gpEnable.
//		<p />
//		Use GSI_NAMESPACE_GAMESPY_DEFAULT as the namespaceID for most normal use.
//		Use namespace 0 for no namespace.
//		<p />
//		Use GP_PARTNERID_GAMESPY_DEFAULT as the partnerID for most normal use.
// See Also
//		gpSetCallback, gpEnable, gpDisable, gpDestroy
COMMON_API GPResult gpInitialize(
	GPConnection * connection,
	int productID,				// The productID is a unique ID that identifies your product
	int namespaceID,			// The namespaceID identifies which namespace to login under. 
								// Use GSI_NAMESPACE_GAMESPY_DEFAULT for the default GameSpy namespace.
								// Use 0 to indicate that no namespace should be used. 
	int partnerID				// The partnerID identifies the account system being used.
								// Use GSI_PARTNERID_GAMESPY_DEFAULT for GameSpy accounts.
								// Use GSI_PARTNERID_IGN for IGN accounts.
 );

///////////////////////////////////////////////////////////////////////////////
// gpDestroy
// Summary
//		This function is used to destroy a connection object.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize.
// Remarks
//		This function destroys a connection object. This should be called when
//		a GPConnection object is no longer needed. The object cannot be used 
//		after it has been destroyed.
void gpDestroy(
	GPConnection * connection
);

///////////////////////////////////////////////////////////////////////////////
// gpEnable
// Summary
//		This function enables a certain state.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		state		: [in] A GPEnum value representing the state to enable. 
// Remarks
//		This function is used to enable ("turn on") states in the connection
//		once gpInitialize has been completed successfully. To disable a state
//		use gpDisable. The following states are available:
//		<emit \<dl\>>
//				<emit \<dt\>>
//			GP_INFO_CACHING_BUDDY_AND_BLOCK_ONLY
//				<emit \</dt\>>
//				<emit \<dd\>>
//			Buddy and blocked list info caching caches information queried 
//			about other users' profiles when they are on the local profile's
//			buddy list or blocked list, potentially improving profile query 
//			performance. This is the recommended mode.
//				<emit \</dd\>>
//				<emit \<dt\>>
//			GP_INFO_CACHING
//				<emit \</dt\>>
//				<emit \<dd\>>
//			General info caching caches information queried about all other 
//			users' profiles, potentially improving profile query performance.
//				<emit \</dd\>>
//				<emit \<dt\>>
//			GP_SIMULATION
//				<emit \</dt\>>
//				<emit \<dd\>>
//			Simulation mode goes through all GameSpy calls until the network 
//			layer is reached, but does not actually make the underlying network 
//			calls. This can be useful for testing code without hitting the 
//			GameSpy backend services.
//				<emit \</dd\>>
//				<emit \<dt\>>
//			GP_NP_SYNC
//				<emit \</dt\>>
//				<emit \<dd\>>
//			*PS3 only* The NP to GP friend sync is enabled by default and 
//			should only be disabled temporarily when using other NP 
//			functionality that may cause contention (then re-enabled 
//			immediately afterward).
//				<emit \</dd\>>
//			<emit \</dl\>>
// See Also
//		gpInitialize, gpDisable
COMMON_API GPResult gpEnable(
	GPConnection * connection, 
	GPEnum state
);

///////////////////////////////////////////////////////////////////////////////
// gpDisable
// Summary
//		This function disables a certain state.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		state		: [in] A GPEnum value representing the state to enable. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is used to disable ("turn off") states in the connection 
//		once gpInitialize has been completed successfully. To enable a state 
//		use gpEnable. See gpEnable or GPEnum for the available states.
// See Also
//		gpInitialize, gpEnable
COMMON_API GPResult gpDisable(
	GPConnection * connection, 
	GPEnum state
);

///////////////////////////////////////////////////////////////////////////////
// gpProcess
// Summary
//		This function checks for incoming callback responses from the GP 
//		backend	and does some necessary processing to maintain an active 
//		GPConnection. It should be called frequently to maintain GP 
//		responsiveness.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function does any necessary processing that needs to be done in 
//		the connection. This includes checking for buddy messages, checking for 
//		buddy status changes, and completing any non-blocking operations. This 
//		function should be called frequently, typically in the application's 
//		main loop. If an operation is finished during a call to this function, 
//		gpProcess will call that operation's registered callback function.
COMMON_API GPResult gpProcess(
	GPConnection * connection
);

///////////////////////////////////////////////////////////////////////////////
// gpSetCallback
// Summary
//		This function is used to set callbacks. The callbacks that get set with
//		this function are called as a result of data received from the server, 
//		such as messages or status updates.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		func		: [in] An enum that indicates which callback is being set. 
//		callback	: [in] The user-supplied function that will be called. 
//		param		: [in] Pointer to user-defined data. This value will be passed 
//							unmodified to the callback function.  
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function sets what callback to call when data is received from the 
//		server, such as messages or status updates, or an error is generated. 
//		If no callback is set for a certain situation, then no alert will be 
//		given when that situation occurs. For example, if no 
//		GP_RECV_BUDDY_REQUEST callback is set, then there will be no way of 
//		detecting when a remote profile wants to add the local profile as a 
//		buddy.
//		<p />
//		These callbacks can be generated during any function that checks for 
//		data received from the server, typically gpProcess or a blocking 
//		operation function.
//		<p />
//		The following can be used as parameters for callback type:
//		<p />
//		GP_ERROR, <p />
//		GP_RECV_BUDDY_REQUEST, <p />
//		GP_RECV_BUDDY_STATUS, <p />
//		GP_RECV_BUDDY_MESSAGE, <p />
//		GP_RECV_GAME_INVITE, <p />
//		GP_TRANSFER_CALLBACK, <p />
//		GP_RECV_BUDDY_AUTH, <p />
//		GP_RECV_BUDDY_REVOKE.
COMMON_API GPResult gpSetCallback(
	GPConnection * connection,
	GPEnum func,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpConnect
// Summary
//		This function is used to establish a connection to the server. It 
//		establishes a connection with an existing profile, which is identified 
//		based on the nick and email and is validated by the password.
// Parameters
//      connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//      nick 		: [in] The profile nickname.
//      email 		: [in] The profile email address.
//      password  	: [in] The profile password.
//      firewall   	: [in] GP_FIREWALL or GP_NO_FIREWALL. This option may limit 
//							the users ability to transfer files. 
//      blocking  	: [in] GP_BLOCKING or GP_NON_BLOCKING.
//      callback  	: [in] A user-supplied callback with an arg type of 
//							GPConnectResponseArg. 
//      param  		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function establishes a connection with the server. If the local 
//		machine is behind a firewall, the firewall parameter should be set to 
//		GP_FIREWALL so that buddy messages are sent through the server.
//		<p />
//		gpDisconnect should be called when this connection is ready to be 
//		disconnected. When the connection is complete, the callback will be 
//		called.
//		<p>
//		If the user is not given the option of selecting whether or not they're 
//		behind a firewall, GP_NO_FIREWALL is passed for the "firewall" 
//		parameter.
//		<p>
//		The SDK will fall back to firewall support if needed. All buddy 
//		messages will travel through the server even if a user is not behind a 
//		firewall.
//		<p>
//		If the user is only connecting to GP to get a loginTicket to use with 
//		other services (e.g., Sake), it is important that GP is disconnected 
//		immediately after the loginTicket is acquired. This is important 
//		because it ensures that the user will not mistakenly receive friend 
//		requests, messages, or game invites when the game doesn't support any 
//		way to handle such items.
// Note
//		gpConnectW and gpConnectA are UNICODE and ANSI mapped versions of 
//		gpConnect. The arguments of gpConnectA are ANSI strings; those of 
//		gpConnectW are wide-character strings.
// See Also 
//      GPConnectResponseArg
COMMON_API GPResult gpConnect(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	GPEnum firewall,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

//DOM-IGNORE-BEGIN
///////////////////////////////////////////////////////////////////////////////
// gpConnectNewUser
// Summary
//		DEPRECATED
//		This function can lead to problems when the new user action step can be 
//		fulfilled but the connect step cannot, for example, and under other 
//		conditions. We recommended that the gpNewUser and gpConnect functions 
//		be used separately so that any error or unexpected results can be 
//		handled more gracefully and	granularly.
//		<p />
//		This function is used to create a new user account and profile and to 
//		then establish a connection using that profile.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		nick		: [in] The desired nickname for the initial profile. The 
//							nickname can be up to GP_NICK_LEN characters long, 
//							including the NULL. 
//		uniquenick	: [in] The desired unique nickname for the profile. 
//		email		: [in] The desired email address for the user. Can be up to 
//							GP_EMAIL_LEN characters long, including the NUL 
//							terminator. 
//		password	: [in] The desired password for the profile. The password 
//							can be up to GP_PASSWORD_LEN characters long, 
//							including the NUL. 
//		cdkey		: [in] An optional cdkey to associate with the unique nick. 
//							Normally left blank. 
//		firewall	: [in] GP_FIREWALL or GO_NO_FIREWALL. This option may limit 
//							the user's ability to send files. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg type of 
//						GPConnectResponseArg 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is identical to gpConnect (see above), except that it 
//		first creates a new user and profile, and then connects the profile. 
//		If this function is used to try to connect a profile that already 
//		exists, the operation will fail. If the email and password identify 
//		an existing user, but the nick does not match any of that user's 
//		profiles, a new profile will be created and logged in.
//		<p />
//		If using uniquenicks, then you will normally want to use the same 
//		string for both the nick and uniquenick parameters. If namespaceID is 0 
//		then the uniquenick and cdkey parameters should be NULL.
// Note
//		gpConnectNewUserW and gpConnectNewUserA are UNICODE and ANSI mapped 
//		versions of gpConnectNewUser. The arguments of gpConnectNewUserA are 
//		ANSI strings; those of gpConnectNewUserW are wide-character strings.
// See Also 
//      GPConnectResponseArg
GPResult gpConnectNewUser(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	const gsi_char cdkey[GP_CDKEY_LEN],
	GPEnum firewall,
	GPEnum blocking,
	GPCallback callback,
	void * param
);
//DOM-IGNORE-END

///////////////////////////////////////////////////////////////////////////////
// gpConnectUniqueNick
// Summary
//		This function is used to establish a connection to the server. It 
//		establishes a connection with an existing profile, which is identified 
//		based on the uniquenick and is validated by the password.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		uniquenick	: [in] The uniquenick. 
//		password	: [in] The profile password. 
//		firewall	: [in] GP_FIREWALL or GO_NO_FIREWALL. This option may limit 
//							the user's ability to send files. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user-supplied callback with an arg type of 
//							GPConnectResponseArg.
//		param		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function establishes a connection with the server. If the local 
//		machine is behind a firewall, the firewall parameter should be set to 
//		GP_FIREWALL so that buddy messages are sent through the server.
//		<p />
//		gpDisconnect should be called when this connection is ready to be 
//		disconnected. When the connection is complete, the callback will be 
//		called.
// Note
//		gpConnectUniqueNickW and gpConnectUniqueNickA are UNICODE and ANSI mapped 
//		versions of gpConnectUniqueNick. The arguments of gpConnectUniqueNickA are 
//		ANSI strings; those of gpConnectUniqueNickW are wide-character strings.
// See Also 
//      GPConnectResponseArg
COMMON_API GPResult gpConnectUniqueNick(
	GPConnection * connection,
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	GPEnum firewall,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpConnectPreAuthenticated
// Summary
//		This function is used to establish a connection to the server. It 
//		establishes a connection using an authtoken and a partnerchallenge, 
//		both obtained from a partner authentication system.
// Parameters
//		connection			: [in] A GP connection object initialized with 
//									gpInitialize.
//		authtoken			: [in] An authentication token generated by a 
//									partner database. 
//		partnerchallenge	: [in] The challenge received from the partner 
//									database. 
//		firewall			: [in] GP_FIREWALL or GO_NO_FIREWALL. This option 
//									may limit the user's ability to send files. 
//		blocking			: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback			: [in] A user-supplied callback with an arg type of 
//									GPConnectResponseArg 
//		param				: [in] Pointer to user-defined data. This value 
//									will be passed unmodified to the callback 
//									function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function establishes a connection with the server. 
//		gpDisconnect should be called when this connection is ready to be 
//		disconnected. When the connection is complete, the callback will be 
//		called.
//		<p />
//		The namespaceID & partnerID parameters passed to gpInitialize will be 
//		overwritten in the SDK to their correct values (based on the 
//		authtoken/partnerchallenge used) after the callback is called.
// Note
//		gpConnectPreAuthenticatedW and gpConnectPreAuthenticatedA are UNICODE 
//		and ANSI mapped versions of gpConnectPreAuthenticated. The arguments of 
//		gpConnectPreAuthenticatedA are ANSI strings; those of 
//		gpConnectPreAuthenticatedW are wide-character strings.
// See Also 
//      GPConnectResponseArg
COMMON_API GPResult gpConnectPreAuthenticated(
	GPConnection * connection,
	const gsi_char authtoken[GP_AUTHTOKEN_LEN],
	const gsi_char partnerchallenge[GP_PARTNERCHALLENGE_LEN],
	GPEnum firewall,
	GPEnum blocking,
	GPCallback callback,
	void * param
);


///////////////////////////////////////////////////////////////////////////////
// gpDisconnect
// Summary
//		This function is used to establish a connection to the server. It 
//		establishes a connection with an existing profile, which is identified 
//		based on the uniquenick and is validated by the password.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//						gpInitialize.
// Remarks
//		This function should be called to disconnect a connection when it is no 
//		longer needed. After this call, connection can be reused for a new 
//		connection.
// See Also
//		gpDestroy
COMMON_API void gpDisconnect(
	GPConnection * connection
);

///////////////////////////////////////////////////////////////////////////////
// gpIsConnected
// Summary
//		Determine whether the GPConnection object has established a connection 
//		with the server.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize.
//		connected	: [out] The connected state. GP_CONNECTED or 
//							GP_NOT_CONNECTED. (See remarks.) 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise, a valid 
//		GPResult is returned.
// Remarks
//      If the connection parameter has not been initialized with gpInitialize, 
//		the connected parameter will be invalid and the return value will be 
//		GP_PARAMETER_ERROR.
COMMON_API GPResult gpIsConnected(
	GPConnection * connection,
	GPEnum * connected
);

///////////////////////////////////////////////////////////////////////////////
// gpCheckUser
// Summary
//		Validates a user's info, without logging into the account.
// Parameters
//      connection	: [in] A GP connection object initialized with 
//							gpInitialize. (Does not have to be connected).
//      nick 		: [in] The profile nickname.
//      email 		: [in] The profile email address.
//      password  	: [in] The profile password.
//      blocking  	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//      callback  	: [in] A user supplied callback with an arg type of 
//							GPConnectResponseArg.
//      param  		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is rarely used but may be useful in certain situations. 
//		The main advantage is that a user's info may be verified without 
//		disrupting other external connections. 
//		gpConnect will usurp any previous connections.
// Note
//		gpCheckUserW and gpCheckUserA are UNICODE and ANSI mapped versions of 
//		gpCheckUser. The arguments of gpCheckUserA are ANSI strings; those of 
//		gpCheckUserW are wide-character strings.
// See Also 
//      GPCheckResponseArg
COMMON_API GPResult gpCheckUser(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpNewUser
// Summary
//		This function creates a new user account and a new profile under that 
//		user account, and optionally a new uniquenick under that profile. The 
//		local user does not does not need to be signed in via gpConnect to use 
//		this function, although gpInitialize and the Available Services Check 
//		should first be successfully completed.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		nick		: [in] The desired profile nickname for the initial profile 
//							of the new account. 
//		uniquenick	: [in] The desired uniquenick for the initial profile of 
//							the new account. 
//		email		: [in] The desired email for the initial profile of the new 
//							account. 
//		password	: [in] The desired password for the initial profile of the 
//							new account. 
//		cdkey		: [in] An optional CDKey to associate with the uniquenick. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] User supplied callback function with an arg type of 
//							GPNewUserResponseArg. 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		<p> This function attempts to create a new user account and a new 
//		profile under that user account, and optionally a new uniquenick under 
//		that profile. The local user does not does not need to be signed in 
//		via one of the gpConnect functions to use this function, although 
//		gpInitialize and the Available Services Check should be completed 
//		successfully first.</p>
//		
//		<p> The nick, email, and password are required parameters; uniquenick 
//		and cdkey are optional. This function cannot be used to create a new 
//		profile under an existing user account (use gpNewProfile after 
//		successfully connecting to a profile for that).	This function also 
//		cannot be used to create a uniquenick under an existing user account 
//		and profile (use gpRegisterUniqueNick after successfully connecting to 
//		a profile for that). Despite these limitations, gpNewUser can create a 
//		profile or a uniquenick when it is used to create a new user account. </p>
//		
//		<p> The specified callback function will be called when the new user 
//		is created, or if an email address, nick, or password conflict is 
//		encountered. </p>
//		<emit \<ol\>>
//		<emit \<li\>> If an email address and nick are specified and a 
//		uniquenick that's already in use is provided, then the specified 
//		callback function will receive a GP_NEWUSER_UNIQUENICK_INUSE error code 
//		regardless of whether or not the specified email address is available 
//		or whether or not the specified password is correct. <emit \</li\>>
//		
//		<emit \<li\>> If the specified email address and password match an 
//		existing account and either the uniquenick isn't specified or it is 
//		specified and available, then the specified callback function will 
//		receive a GP_NEWUSER_BAD_NICK error code because gpNewUser cannot be 
//		used to create a new profile or uniquenick under an existing user 
//		account. See the gpNewProfile and gpRegisterUniqueNick functions for 
//		that. <emit \</li\>>
//		
//		<emit \<li\>> If the specified email address and nick match an existing 
//		account, and the uniquenick is either not specified or available, and 
//		the password isn't correct for the specified email address, then the 
//		specified callback function will receive a GP_NEWUSER_BAD_PASSWORD 
//		error code. <emit \</li\>>
//		<emit \</ol\>>
// Note
//		gpNewUserW and gpNewUserA are UNICODE and ANSI mapped versions of 
//		gpNewUser. The arguments of gpNewUserA are ANSI strings; those of 
//		gpNewUserW are wide-character strings.
// See Also 
//      gpNewProfile, gpRegisterUniqueNick, GPNewUserResponseArg 
COMMON_API GPResult gpNewUser(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	const gsi_char cdkey[GP_CDKEY_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpSuggestUniqueNick
// Summary
//		This function gets suggested uniquenicks from the backend.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		desirednick	: [in] The desired uniquenick, which can be up to 
//							GP_UNIQUENICK_LEN characters long, including the 
//							NUL. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg type of 
//							GPSuggestUniqueNickResponseArg. 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function gets a set of suggested nicks based on the desirednick. 
//		A request is sent to the backend for suggestions based on the provided 
//		desirednick. After getting a response, the callback is called with a 
//		list of uniquenicks based on the desirednick. These suggested 
//		uniquenicks can then be used in a call to gpNewUser, 
//		gpRegisterUniqueNick, or gpSetInfos. 
// Note
//		gpSuggestUniqueNickW and gpSuggestUniqueNickA are UNICODE and ANSI 
//		mapped versions of gpSuggestUniqueNick. The arguments of 
//		gpSuggestUniqueNickA are ANSI strings; those of 
//		gpSuggestUniqueNickW are wide-character strings.
// See Also
//		GPSuggestUniqueNickResponseArg
COMMON_API GPResult gpSuggestUniqueNick(
	GPConnection * connection,
	const gsi_char desirednick[GP_UNIQUENICK_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpRegisterUniqueNick
// Summary
//		This function attempts to register a uniquenick and associate it with 
//		the local profile.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize.
//		uniquenick	: [in] The desired uniquenick; it can be up to 
//							GP_UNIQUENICK_LEN characters long, including the 
//							NUL. 
//		cdkey		: [in] An optional CDKey to associate with the uniquenick. 
//							If not using CDKeys this should be NULL. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg type of 
//							GPRegisterUniqueNickResponseArg. 
//		param		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function attempts to register a uniquenick and associate it with 
//		the local profile. It should only be used if the namespaceID passed to 
//		gpInitialize was greater than 0. The backend makes certain checks on a 
//		uniquenick before it is allowed to be registered. For details on what 
//		is checked, see the "Uniquenick Checks" section of the Presence and 
//		Messaging SDK Overview.
// Note
//		gpRegisterUniqueNickW and gpRegisterUniqueNickA are UNICODE and ANSI 
//		mapped versions of gpRegisterUniqueNick. The arguments of 
//		gpRegisterUniqueNickA are ANSI strings; those of gpRegisterUniqueNickW 
//		are wide-character strings.
// See Also 
//      GPRegisterUniqueNickResponseArg
COMMON_API GPResult gpRegisterUniqueNick(
	GPConnection * connection,
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const gsi_char cdkey[GP_CDKEY_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpRegisterCdKey
// Summary
//		This function attempts to register a cdkey and associate it with the 
//		local profile.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		cdkey		: [in] A CDKey to associate with the currently signed-in 
//							profile.
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg type of 
//							GPRegisterCdKeyResponseArg. 
//		param		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function. 
//		gameId		: [in] Game ID
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid GPResult 
//		is returned.
// Remarks
//		The gpRegisterCdKey() function attempts to register the specified 
//		CDKey value to the currently signed-in GP profile, and hence will 
//		only work when GP is connected. As a security measure, there is no way 
//		to retrieve the CDKey value once it is registered. It is assumed that 
//		the CDKey is available to the local game client. 
//		<br /><br />
//		Once a CDKey is registered to a GP profile, this function can be 
//		called again as	an anti-piracy measure, given that the callback 
//		function you assign in the gpRegisterCdKey() call will indicate success 
//		if the attempt to register a new CDKey	to the current profile matches 
//		the CDKey already registered to that profile, or will be passed one of 
//		the following GPErrorCode values if not:
//		<emit \<ul\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY				= 4352, // 0x1100, There was an error registering the cdkey.
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_BAD_KEY		= 4353, // 0x1101, The cdkey is invalid. 
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_ALREADY_SET	= 4354, // 0x1102, The profile has already been registered with a different cdkey.
//			<emit \</li\>>
//			<emit \<li\>>
//		GP_REGISTERCDKEY_ALREADY_TAKEN	= 4355, // 0x1103, The cdkey has already been registered to another profile.
//			<emit \</li\>>
//		<emit \</ul\>>

//      Note that only one CDKey can be associated with a single profile for 
//		single game. Once a CDKey has been associated, it cannot be associated 
//		with any other profiles.
// Note
//		gpRegisterCdKeyW and gpRegisterCdKeyA are UNICODE and ANSI mapped 
//		versions of gpRegisterCdKey. The arguments of gpRegisterCdKeyA are ANSI 
//		strings; those of gpRegisterCdKeyW are wide-character strings.
// See Also 
//      GPRegisterCdKeyResponseArg
COMMON_API GPResult gpRegisterCdKey(
	GPConnection * connection,
	const gsi_char cdkey[GP_CDKEY_LEN],
	int gameId,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpGetErrorCode
// Summary
//		This function gets the current error code for a connection.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		errorCode	: [out] The current error code.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function gets the current error code for connection. It can be 
//		used to determine the specific cause of the most recent error. See the 
//		GP header, gp.h, for all of the possible error codes.
// See Also
//		GPErrorCode
COMMON_API GPResult gpGetErrorCode(
	GPConnection * connection,
	GPErrorCode * errorCode
);

///////////////////////////////////////////////////////////////////////////////
// gpGetErrorString
// Summary
//		This function gets the current error string for a connection.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		errorString	: [in] A text description of the current error. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function gets the current error string for connection. The error 
//		string is a text description of the most recent error that occurred on 
//		this connection. If no errors have occurred on this connection, the 
//		error string will be empty ("").
// Note
//		gpGetErrorStringW and gpGetErrorStringA are UNICODE and ANSI mapped 
//		versions of gpGetErrorString. The arguments of gpGetErrorStringA are 
//		ANSI strings; those of gpGetErrorStringW are wide-character strings.
COMMON_API GPResult gpGetErrorString(
	GPConnection * connection,
	gsi_char errorString[GP_ERROR_STRING_LEN]
);

///////////////////////////////////////////////////////////////////////////////
// gpNewProfile
// Summary
//		This function creates a new profile for the local user.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		nick		: [in] The new profile nickname. 
//		replace		: [in] Replacement option. (See remarks.) 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user-supplied callback with an arg type of 
//							GPNewProfileResponseArg. 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function creates a new profile for the local user. This function 
//		does not make the new profile the current profile. To switch to the 
//		newly created profile, the user must disconnect and then connect with 
//		the new nickname. If the nick for the new profile is the same as the 
//		nick for an existing profile, an error will be generated, unless 
//		replace is set to GP_REPLACE. An application should use GP_DONT_REPLACE 
//		by default. If an error with the error code of GP_NEWPROFILE_BAD_NICK 
//		is received, this means that a profile with the provided nickname 
//		already exists. The application should at this point ask the user if 
//		he wants to replace the old profile. If the user does want to replace 
//		the old profile, gpNewProfile should be called again with replace set 
//		to GP_REPLACE. When the new profile is created, the callback will be 
//		called.
// Note
//		gpNewProfileW and gpNewProfileA are UNICODE and ANSI mapped versions 
//		of gpNewProfile. The arguments of gpNewProfileA are ANSI strings; those 
//		of gpNewProfileW are wide-character strings.
// See Also 
//      GPNewProfileResponseArg
COMMON_API GPResult gpNewProfile(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	GPEnum replace,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpDeleteProfile
// Summary
//		This function deletes the local profile. Note that this is a blocking 
//		call.
// Parameters
//		connection	: [in] A GP connection interface with an established 
//							connection. 
//		callback	: [in] The callback used to confirm the deleted profile. 
//		arg			: [in] User data.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function deletes the local profile. Because the connection is 
//		between the local profile and the server, this automatically ends this 
//		connection (gpDisconnect does not need to be called). There is no way 
//		to delete any profile other than the current connected profile. The 
//		operation will fail if the connected profile is the user's only 
//		profile. A successful delete will result in the callback getting 
//		called. The callback will have the data about the delete profile and 
//		whether it was successful or not.
COMMON_API GPResult gpDeleteProfile(
	GPConnection * connection,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpProfileFromID
// Summary
//		Translates a profile id into a GPProfile.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [out] The GPProfile for the given profile ID. 
//		id			: [in]  The profile ID.  
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is deprecated as GPProfiles are now the same as profile 
//		ids.
//		This function will be removed in a future version of the SDK.
COMMON_API GPResult gpProfileFromID(
	GPConnection * connection, 
	GPProfile * profile, 
	int id
);

///////////////////////////////////////////////////////////////////////////////
// gpIDFromProfile
// Summary
//		A GPProfile is now the same as a profileid.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in]  The GPProfile 
//		id			: [out] The profile ID of the GPProfile. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is deprecated as GPProfiles are now the same as profile 
//		ids.
//		This function will be removed in a future version of the SDK.
COMMON_API GPResult gpIDFromProfile(
	GPConnection * connection,
	GPProfile profile,
	int * id
);

///////////////////////////////////////////////////////////////////////////////
// gpUserIDFromProfile
// Summary
//		This function gets a profile's user ID.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in]  The profile ID. 
//		userid		: [out] The user ID associated with the specified profile 
//							ID. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		Every profile is associated with a user account, and each user account 
//		has a user id associated with it. This functions gets the user id for a 
//		given profile's user account. 
COMMON_API GPResult gpUserIDFromProfile(
	GPConnection * connection,
	GPProfile profile,
	int * userid
);

///////////////////////////////////////////////////////////////////////////////
// gpProfileSearch
// Summary
//		This function searches for profiles based on certain criteria.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		nick		: [in] If not NULL or "", search for profiles with this nick. 
//		uniquenick	: [in] If not NULL or "", search for profiles with this uniquenick. 
//		email		: [in] If not NULL or "", search for profiles with this email. 
//		firstname	: [in] If not NULL or "", search for profiles with this firstname. 
//		lastname	: [in] If not NULL or "", search for profiles with this lastname. 
//		icquin		: [in] If not 0, search for profiles with this icquin. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg type of 
//							GPProfileSearchResponseArg. 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function contacts the Search Manager and attempts to find all 
//		profiles that match the search criteria. A profile matches the provided 
//		search criteria only if its corresponding values are the same as those 
//		provided. Currently, there is no substring matching, and the criteria 
//		is case-sensitive.
//		<p />
//		When the search is complete, the callback will be called.
// Note
//		gpProfileSearchW and gpProfileSearchA are UNICODE and ANSI mapped 
//		versions of gpProfileSearch. The arguments of gpProfileSearchA are ANSI 
//		strings; those of gpProfileSearchW are wide-character strings.
// See Also 
//      GPProfileSearchResponseArg
COMMON_API GPResult gpProfileSearch(
	GPConnection * connection,
	const gsi_char nick[GP_NICK_LEN],
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char firstname[GP_FIRSTNAME_LEN],
	const gsi_char lastname[GP_LASTNAME_LEN],
	int icquin,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

//DOM-IGNORE-BEGIN
// gpProfileSearchUniquenick
	GPResult gpProfileSearchUniquenick(
	GPConnection * connection,
	const gsi_char uniquenick[GP_UNIQUENICK_LEN],
	const int namespaceIDs[GP_MAX_NAMESPACEIDS],
	int numNamespaces,
	GPEnum blocking,
	GPCallback callback,
	void * param
);
//DOM-IGNORE-END

///////////////////////////////////////////////////////////////////////////////
// gpGetInfo
// Summary
//		This function gets info on a particular profile.
// Parameters
//		connection		: [in] A GP connection object initialized with 
//								gpInitialize. 
//		profile			: [in] The profile ID of the user to get info on. 
//		checkCache		: [in] When set to GP_CHECK_CACHE the SDK will use the 
//								currently known info. 
//		blocking		: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback		: [in] A user-supplied callback with an argument type 
//								of GPGetInfoResponseArg 
//		param			: [in] Pointer to user-defined data. This value will be 
//								passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function gets profile info for the profile object profile. When 
//		the info has been retrieved, the callback will be called.. If 
//		info-caching is enabled, the info may be available locally, in which 
//		case it will be returned immediately if checkCache is GP_CHECK_CACHE. 
//		Otherwise, the server will be contacted for the info. If the server 
//		needs to be contacted, then the function will return immediately in 
//		non-blocking mode. If info-caching is enabled, any info retrieved from 
//		the server will be cached.
// See Also 
//      GPGetInfoResponseArg
COMMON_API GPResult gpGetInfo(
	GPConnection * connection,
	GPProfile profile, 
	GPEnum checkCache,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

//DOM-IGNORE-BEGIN
// gpGetInfoNoWait
GPResult gpGetInfoNoWait(
	GPConnection * connection,
	GPProfile profile,
	GPGetInfoResponseArg * arg
);
//DOM-IGNORE-END

///////////////////////////////////////////////////////////////////////////////
// gpSetInfoi
// Summary
//		These functions are used to set local info.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		info		: [in] An enum indicating what info to update. 
//		value		: [in] The integer value. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		These functions are used to set local info. The info does not actually 
//		get updated (sent to the server) until the next call to gpProcess. If a 
//		string is longer than the allowable length for that info, it will be 
//		truncated without warning.
COMMON_API GPResult gpSetInfoi(
	GPConnection * connection, 
	GPEnum info, 
	int value
);

///////////////////////////////////////////////////////////////////////////////
// gpSetInfos
// Summary
//		These functions are used to set local info.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		info		: [in] An enum indicating what info to update. 
//		value		: [in] The string value. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		These functions are used to set local info. The info does not actually 
//		get updated (sent to the server) until the next call to gpProcess. If a 
//		string is longer than the allowable length for that info, it will be 
//		truncated without warning. 
// Note
//		gpSetInfosW and gpSetInfosA are UNICODE and ANSI mapped versions of 
//		gpSetInfos. The arguments of gpSetInfosA are ANSI strings; those of 
//		gpSetInfosW are wide-character strings.
COMMON_API GPResult gpSetInfos(
	GPConnection * connection,
	GPEnum info,
	const gsi_char * value
);

///////////////////////////////////////////////////////////////////////////////
// gpSetInfod
// Summary
//		These functions are used to set local info.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		info		: [in] An enum indicating what info to update. 
//		day			: [in] The day. 
//		month		: [in] The month. 
//		year		: [in] The year.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		These functions are used to set local info. The info does not actually 
//		get updated (sent to the server) until the next call to gpProcess. If a 
//		string is longer than the allowable length for that info, it will be 
//		truncated without warning.
COMMON_API GPResult gpSetInfod(
	GPConnection * connection,
	GPEnum info,
	int day,
	int month,
	int year
);

///////////////////////////////////////////////////////////////////////////////
// gpSetInfoMask
// Summary
//		Sets a profile information mask with any combination of masks described 
//		in GPEnum enumeration.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		mask		: [in] The info type. See remarks. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		The possible mask values are:
//		<p />
//		GP_MASK_NONE <p />
//		GP_MASK_HOMEPAGE <p /> 
//		GP_MASK_ZIPCODE <p />
//		GP_MASK_COUNTRYCODE <p />
//		GP_MASK_BIRTHDAY <p />
//		GP_MASK_SEX <p />
//		GP_MASK_EMAIL <p />
//		GP_MASK_BUDDYLIST <p />
//		GP_MASK_ALL <p />
//		<p />
//		The mask can be any one or a combination of the above enumerations in 
//		GPEnum.
// See Also
//		GPEnum
COMMON_API GPResult gpSetInfoMask(
	GPConnection * connection,
	GPEnum mask
);

///////////////////////////////////////////////////////////////////////////////
// gpSendBuddyRequest
// Summary
//		This function sends a request to a remote profile to ask for permission 
//		to add the remote profile to the local profile's buddy list.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The remote profile to which the buddy request is 
//							being made. 
//		reason		: [in] A text string that (optionally) explains why the 
//							user is making the buddy request.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function sends a request to the given remote profile, asking if 
//		the local profile can make the remote profile a buddy. There is no 
//		immediate response to this message. If the remote profile authorizes 
//		the request, a buddy message and a status message will be received from 
//		the new buddy. However, this can take any amount of time. This message 
//		causes the gpRecvBuddyRequest callback to be called for the remote 
//		profile.
// Note
//		gpSendBuddyRequestW and gpSendBuddyRequestA are UNICODE and ANSI 
//		mapped versions of gpSendBuddyRequest. The arguments of 
//		gpSendBuddyRequestA are ANSI strings; those of gpSendBuddyRequestW are 
//		wide-character strings.
COMMON_API GPResult gpSendBuddyRequest(
	GPConnection * connection,
	GPProfile profile,
	const gsi_char reason[GP_REASON_LEN]
);

///////////////////////////////////////////////////////////////////////////////
// gpAuthBuddyRequest
// Summary
//		This function authorizes a buddy request. It is called in response to 
//		the gpRecvBuddyRequest callback getting called.
// Parameters
//      connection	: [in] The connection on which to authorize the request. 
//      profile 	: [in] The remote profile whose buddy request is being 
//							authorized. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is used to authorize a buddy request received with the 
//		gpRecvBuddyRequest callback. It is used only to authorize. This 
//		function does not need to be called immediately after a request has 
//		been received, however the request will be lost as soon as the local 
//		profile is disconnected. This function causes a status message to be 
//		sent to the remote profile.
COMMON_API GPResult gpAuthBuddyRequest(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpDenyBuddyRequest
// Summary
//		This function denies a buddy request. It is called in response to the 
//		gpRecvBuddyRequest callback getting called.
// Parameters
//		connection	: [in] A GP connection interface with an established 
//							connection. 
//		profile		: [in] The profile ID of the player who sent the 
//							AddBuddyRequest (i.e., the player you are denying).
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is used to deny a buddy request received with the 
//		gpRecvBuddyRequest callback. This function does not need to be called 
//		immediately after a request has been received. Nothing is sent to the 
//		remote profile letting them know the request was denied.
COMMON_API GPResult gpDenyBuddyRequest(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpDeleteBuddy
// Summary
//		This function deletes a buddy from the local profile's buddy list.
// Parameters
//		connection	: [in] A GP connection interface with an established 
//							connection.
//		profile		: [in] The profile ID of the buddy to delete.  
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function deletes the buddy indicated by profile from the local 
//		profile's buddy list.
COMMON_API GPResult gpDeleteBuddy(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpAddToBlockedList
// Summary
//		Adds a remote profile to the local player's blocked list.
// Parameters
//      connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//      profile 	: [in] The profileid of the player to be blocked. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      A blocked player is essentially invisible to player who has him	
//		blocked. The local player will not receive any communication from the 
//		blocked player, nor will the local player be able to contact the 
//		blocked player in any way. This function will only work when GP is 
//		connected. This function will not return any callback on success, but 
//		the GP_ERROR callback will be called should an error occur during the 
//		add attempt.
// See Also 
//      gpRemoveFromBlockedList, gpGetNumBlocked, gpGetBlockedProfile, gpIsBlocked
COMMON_API GPResult gpAddToBlockedList(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpRemoveFromBlockedList
// Summary
//		Removes a remote profile from the local player's blocked list.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profileid of the player to be removed from the 
//							blocked list. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      A blocked player is essentially invisible to player who has him/her 
//		blocked. The local player will not receive any communication from the 
//		blocked player, nor will the local player be able to contact the 
//		blocked player in any way. This function will only work when GP is 
//		connected. This function will not return any callback on success, but 
//		the GP_ERROR callback will be called should an error occur during the 
//		removal attempt.
// See Also 
//      gpAddToBlockedList, gpGetNumBlocked, gpGetBlockedProfile, gpIsBlocked
COMMON_API GPResult gpRemoveFromBlockedList(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpGetNumBlocked
// Summary
//		Gets the total number of blocked players in the local profile's 
//		blocked list.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		numBlocked	: [out] The total number of blocked players in the local 
//							profile's blocked list. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function will return 0 when GP is not connected. The blocked list 
//		is fully obtained after the login process is complete.
// See Also 
//      gpGetBlockedProfile, gpIsBlocked
COMMON_API GPResult gpGetNumBlocked(
	GPConnection * connection,
	int * numBlocked
);

///////////////////////////////////////////////////////////////////////////////
// gpGetBlockedProfile
// Summary
//		This function gets the profileid for a particular player on the blocked 
//		list.
// Parameters
//		connection		: [in]  A GP connection object initialized with 
//								gpInitialize. 
//		index			: [in]  The array index of the blocked player. 
//		profile			: [out] The profileid of the blocked player.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      The blocked list is fully obtained after the login process is complete. 
//		Index is a number greater than or equal to 0 and less than the total 
//		number of blocked players; this is generally called in conjunction with 
//		gpGetNumBlocked to enumerate through the list.
// See Also 
//      gpGetNumBlocked, gpIsBlocked
COMMON_API GPResult gpGetBlockedProfile(
	GPConnection * connection, 
	int index,
	GPProfile * profile
);

///////////////////////////////////////////////////////////////////////////////
// gpIsBlocked
// Summary
//		Returns gsi_true if the given ProfileID is blocked, gsi_false if not 
//		blocked.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//						gpInitialize. 
//		profile		: [in] The profile ID of the player to check.
// Returns 
//      Returns gsi_true if the given ProfileID is blocked, gsi_false if not 
//		blocked.
COMMON_API gsi_bool gpIsBlocked(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpGetNumBuddies
// Summary
//		This function gets the number of buddies on the local profile's buddy 
//		list.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		numBuddies	: [out] The number of buddies.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function gets the number of buddies on the local profile's buddy 
//		list. It may take some time to receive the total number of buddies from 
//		the server, so this function may report a number smaller than the 
//		actual total while the complete buddy list is being received. To see 
//		the	status of each buddy, call gpGetBuddyStatus. The number of buddies 
//		is only valid until a buddy is added to or deleted from the buddy list.
COMMON_API GPResult gpGetNumBuddies(
	GPConnection * connection,
	int * numBuddies
);

///////////////////////////////////////////////////////////////////////////////
// gpGetBuddyIndex
// Summary
//		This function checks a remote profile to see if it is a buddy. If it is 
//		a buddy, the buddy's index is returned. If it is not a buddy, the index 
//		will be set to -1.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in]  The profile ID of the buddy. 
//		index		: [out] The internal array index of the buddy. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to check if a remote profile is a buddy and to 
//		get its buddy index if it is a buddy. This buddy index can then be used 
//		in a call to gpGetBuddyStatus. The buddy index may become invalid after 
//		a buddy is added to or deleted from the buddy list. If the profile is 
//		not a buddy, GP_NO_ERROR will be returned (as long as no other errors 
//		happen), and index will be set to -1.
COMMON_API GPResult gpGetBuddyIndex(
	GPConnection * connection, 
	GPProfile profile, 
	int * index
);

///////////////////////////////////////////////////////////////////////////////
// gpIsBuddy
// Summary
//		Returns 1 if the given ProfileID is a buddy, 0 if not a buddy.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile ID of the player to check.
// Returns 
//      Returns 1 if the given ProfileID is a buddy, 0 if not a buddy.
// Remarks
//      Returns 1 if the given ProfileID is a buddy, 0 if not a buddy.
COMMON_API int gpIsBuddy(
	GPConnection * connection,
	GPProfile profile
);

///////////////////////////////////////////////////////////////////////////////
// gpIsBuddyConnectionOpen
// Summary
//		Returns 1 if the given ProfileID is connected for direct peer-to-peer
//		messaging. Returns 0 otherwise.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile ID of the player to check.
// Returns 
//      Returns 1 if the given ProfileID is directly connected, 0 if not.
// Remarks
//		Returns 1 if the given ProfileID is connected for direct peer-to-peer
//		messaging. Returns 0 otherwise.
COMMON_API int gpIsBuddyConnectionOpen(
	GPConnection * connection,
	GPProfile profile
);

#ifndef GP_NEW_STATUS_INFO

///////////////////////////////////////////////////////////////////////////////
// gpSetStatus
// Summary
//		This function sets the local profile's status.
// Parameters
//		connection		: [in] A GP connection object initialized with 
//								gpInitialize. 
//		status			: [in] An enum indicating the status to set. 
//		statusString	: [in] A text string with a user-readable explanation 
//								of the status. 
//		locationString	: [in] A URL indicating the local profile's game 
//								location in the form 
//								"gamename://IP.address:port/extra/info".
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function sets the local profile's status. The status consists of 
//		an enum specifying a mode (online, offline, playing, etc.), a text 
//		explanation of the status, and a URL specifying a game location and 
//		port in the form "quake://12.34.56.78:9999". 
// Note
//		gpSetStatusW and gpSetStatusA are UNICODE and ANSI mapped versions of 
//		gpSetStatus. The arguments of gpSetStatusA are ANSI strings; those of 
//		gpSetStatusW are wide-character strings. 
COMMON_API GPResult gpSetStatus(
	GPConnection * connection,
	GPEnum status,
	const gsi_char statusString[GP_STATUS_STRING_LEN],
	const gsi_char locationString[GP_LOCATION_STRING_LEN]
);

#endif

///////////////////////////////////////////////////////////////////////////////
// gpSendBuddyMessage
// Summary
//		This function sends a message to a buddy.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile object for the buddy to whom the message 
//							is going. 
//		message		: [in] A user-readable text string containing the message to 
//							send to the buddy.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      If a direct connection to the buddy is possible (the buddy is not 
//		behind a firewall), the message can be any size. However, if the buddy 
//		is behind a firewall, then the message needs to be sent through the server. 
//		In this case, there is a limit of 1024 characters. Any message longer than 
//		1024 characters that needs to be sent through the server will be truncated 
//		without warning or notice.
// Note
//		gpSendBuddyMessageW and gpSendBuddyMessageA are UNICODE and ANSI mapped 
//		versions of gpSendBuddyMessage. The arguments of gpSendBuddyMessageA 
//		are ANSI strings; those of gpSendBuddyMessageW are wide-character 
//		strings.
// See Also
//		gpSendBuddyUTM
COMMON_API GPResult gpSendBuddyMessage(
	GPConnection * connection,
	GPProfile profile,
	const gsi_char * message
);

///////////////////////////////////////////////////////////////////////////////
// gpSendBuddyUTM
// Summary
//		Sends a UTM (under-the-table message) to a buddy.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile object for the buddy to whom the message 
//							is going. 
//		message		: [in] A user-readable text string containing the message 
//							to send to the buddy. 
//		sendOption	: [in] UTM sending options - defined in GPEnum. Pass in 0 
//							for no options.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		A UTM is a special type of message you can send that is usually not 
//		intended for direct display to users.
//		<p />
//      If a direct connection to the buddy is possible (i.e., the buddy is not 
//		behind a firewall), the message can be any size. However, if the buddy 
//		is behind a firewall, then the message needs to be sent through the 
//		server. In this case, there is a limit of 1024 characters. Any message 
//		longer than 1024 characters that needs to be sent through the server 
//		will be truncated without warning or notice.
//		<p />
//		If GP_DONT_ROUTE is listed as a sendOption, the SDK will only attempt 
//		to send this message directly to the player and not route it through 
//		the server.
// Note
//		gpSendBuddyUTMW and gpSendBuddyUTMA are UNICODE and ANSI mapped 
//		versions of gpSendBuddyUTM. The arguments of gpSendBuddyUTMA are ANSI 
//		strings; those of gpSendBuddyUTMW are wide-character strings.
// See Also
//		gpSendBuddyMessage
COMMON_API GPResult gpSendBuddyUTM(
	GPConnection * connection,
	GPProfile profile,
	const gsi_char * message,
	int sendOption // GP_DONT_ROUTE
);

///////////////////////////////////////////////////////////////////////////////
// gpIsValidEmail
// Summary
//		This function checks if there is an account with the given email 
//		address.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		email		: [in] The email address to list accounts for. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A user supplied callback with an arg of the type 
//							GPIsValidEmailResponseArg 
//		param		: [in] Pointer to user defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function contacts the Search Manager and checks to see if there is 
//		a user with the given email address.
// Note
//		gpIsValidEmailW and gpIsValidEmailA are UNICODE and ANSI mapped 
//		versions of gpIsValidEmail. The arguments of gpIsValidEmailA are ANSI 
//		strings; those of gpIsValidEmailW are wide-character strings.
// See Also 
//      GPIsValidEmailResponseArg
COMMON_API GPResult gpIsValidEmail(
	GPConnection * connection,
	const gsi_char email[GP_EMAIL_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpGetUserNicks
// Summary
//		This function gets the nicknames for a given email/password (which 
//		identifies a user).
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize (Does not have to be connected). 
//		email		: [in] The account email address. 
//		password	: [in] The account password. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING. 
//		callback	: [in] A user-supplied callback with an arg type of 
//							GPGetUserNicksResponseArg 
//		param		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function contacts the Search Manager and gets a list of this 
//		user's nicks (profiles).
//		<p />
//		If you are unsure if the email address provided to this function is a 
//		valid email address, call gpIsValidEmail first.
// Note
//		gpGetUserNicksW and gpGetUserNicksA are UNICODE and ANSI mapped 
//		versions of gpGetUserNicks. The arguments of gpGetUserNicksA are ANSI 
//		strings; those of gpGetUserNicksW are wide-character strings.
// See Also 
//      GPGetUserNicksResponseArg, gpIsValidEmail
COMMON_API GPResult gpGetUserNicks(
	GPConnection * connection,
	const gsi_char email[GP_EMAIL_LEN],
	const gsi_char password[GP_PASSWORD_LEN],
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpInvitePlayer
// Summary
//		This function invites a player to play a certain game.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile ID of the player to invite. 
//		productID	: [in] The product ID of the game to which to invite the 
//							player. 
//		location	: [in] A message to send along with the invite. See 
//							remarks.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is used to invite another profile to join the local 
//		profile in a game's title room. The remote profile will receive a 
//		GP_RECV_GAME_INVITE callback.
//		<p />
//		gpInvitePlayer may now take an optional text message to be sent along 
//		with the invite. This usually contains the server IP and other 
//		connecting information. This parameter may be NULL. The max length for 
//		the location info is 255 characters. When compiling in Unicode mode, 
//		the location will be converted to ASCII.
// Note
//		gpInvitePlayerW and gpInvitePlayerA are UNICODE and ANSI mapped 
//		versions of gpInvitePlayer. The arguments of gpInvitePlayerA are ANSI 
//		strings; those of gpInvitePlayerW are wide-character strings.
GPResult gpInvitePlayer(
	GPConnection * connection,
	GPProfile profile,
	int productID,
	const gsi_char location[GP_LOCATION_STRING_LEN]
);

///////////////////////////////////////////////////////////////////////////////
// gpGetProfileBuddyList
// Summary
//		Get the buddies for a profile.
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		profile		: [in] The profile to get the buddy list for. 
//		maxBuddies	: [in] The maximum number of buddies to return. If 0 is passed in, 
//							all buddies will be returned. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A GP callback that will be passed a GPGetProfileBuddyListArg. 
//		param		: [in] Pointer to user-defined data. This value will be passed unmodified 
//							to the callback function. 
// Returns 
//      GPResult is described in the GP enums section.
// Remarks
//      This function returns a valid GPResult. Common return values are: <p>
//		GP_NO_ERROR on success. <p>
//		GP_PARAMETER_ERROR is returned if connection is NULL, profile is 0, callback is 
//		NULL, or the connection is not connected. GP_MEMORY_ERROR is returned when an 
//		allocation fails. <p> 
//		GP_NETWORK_ERROR is returned when there is a problem connecting to the 
//		Presence and Messaging backend.
// See Also 
//      GPGetProfileBuddyListArg
GPResult gpGetProfileBuddyList(GPConnection* connection,
	GPProfile profile,
	int maxBuddies,
	GPEnum blocking,
	GPCallback callback,
	void* param);

///////////////////////////////////////////////////////////////////////////////
// gpGetReverseBuddies
// Summary
//		Get profiles that have you on their buddy list.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		blocking	: [in] GP_BLOCKING or GP_NON_BLOCKING 
//		callback	: [in] A GP callback that will be passed a 
//							GPGetReverseBuddiesResponseArg. 
//		param		: [in] Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      Get profiles that have you on their buddy list.
// See Also 
//      GPGetReverseBuddiesResponseArg
COMMON_API GPResult gpGetReverseBuddies(
	GPConnection * connection,
	GPEnum blocking,
	GPCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpRevokeBuddyAuthorization
// Summary
//		Remove the local client from a remote users buddy list.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		profile		: [in] The profile ID of the remote player.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      Use this function when the local client no longer wants the remote 
//		player to be able to send buddy messages or view status info.
COMMON_API GPResult gpRevokeBuddyAuthorization(
	GPConnection * connection,
	GPProfile profile
);

// gpSetCdKey
GPResult gpSetCdKey(
  GPConnection * connection,
  const gsi_char cdkeyhash,
  GPCallback callback
);

///////////////////////////////////////////////////////////////////////////////
// gpGetLoginTicket
// Summary
//		Retrieves a connection "token" that may be used by HTTP requests to 
//		uniquely identify the player.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		loginTicket	: [out] The login ticket.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		Retrieves a connection "token" that may be used by HTTP requests to 
//		uniquely identify the player.
COMMON_API GPResult gpGetLoginTicket(
	GPConnection * connection,
	char loginTicket[GP_LOGIN_TICKET_LEN]
);

#ifndef NOFILE

///////////////////////////////////////////////////////////////////////////////
// gpSetInfoCacheFilename
// Summary
//		Sets the file name for the internal profile cache.
// Parameters
//		filename : [in] The filename to use for the profile cache.  
// Remarks
//		This function should be called before gpInitialize.
// Note
//		gpSetInfoCacheFilenameW and gpSetInfoCacheFilenameA are UNICODE and 
//		ANSI mapped versions of gpSetInfoCacheFilename. The arguments of 
//		gpSetInfoCacheFilenameA are ANSI strings; those of 
//		gpSetInfoCacheFilenameW are wide-character strings.
COMMON_API void gpSetInfoCacheFilename(
	const gsi_char * filename
);

///////////////////////////////////////////////////////////////////////////////
// gpSendFilesCallback
// Summary
//		This is a callback used by gpSendFiles() to get the list of files to 
//		send.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		index		: [in] This starts at 0 and is incremented by 1 each time 
//							the callback gets called. 
//		path		: [in] Point this to the path to the file to send, or NULL 
//							for a directory. 
//		name		: [in] Point this to the name to send the file under, or 
//							NULL to use the file's local name. 
//		param		: [in] User-data that was passed into gpSendFiles. 
// Remarks
//		This function will be called repeatedly until neither path nor name are 
//		set (or both set to NULL). 
//		The callback can be used to specify either 
//		a file or a directory. To specify a file, set the path to point to a 
//		string containing the path to the file. If the name is also set, then 
//		it contains the name to send the file under. The name can be a simple 
//		filename (i.e., "file.ext"), or it can contain a path 
//		("files/file.ext"). 
//		If a path is specified, and name is not set (or set to NULL), then the 
//		filename part of the path will be used. For example, if path points to
//		"c:\files\file.ext" and name is not set, then the name will be 
//		"file.ext". 
//		To specify a directory, don't set the path (or set it to NULL), then 
//		set a name. The name will be treated as a directory to create. For 
//		example, if path is not set, and name is "files", this instructs the 
//		receiver to create a directory named "files". 
//		The name for a file or folder cannot contain any directory-level 
//		references (e.g., "../file.exe"), cannot start with a slash or 
//		backslash, cannot contain any empty directory names in the path, 
//		and cannot contain any invalid characters ( : * ? " < > | \n ).
// Note
//		gpSendFilesCallbackW and gpSendFilesCallbackA are UNICODE and ANSI 
//		mapped versions of gpSendFilesCallback. The arguments of 
//		gpSendFilesCallbackA are ANSI strings; those of gpSendFilesCallbackW
//		are wide-character strings.
typedef void (* gpSendFilesCallback)(
	GPConnection * connection,
	int index,
	const gsi_char ** path,
	const gsi_char ** name,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpSendFiles
// Summary
//		This function attempts to send one or more files (and/or sub-directory 
//		names) to another profile.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [out] A pointer to a GPTransfer object. 
//		profile		: [in]  The profile to send to. Must be a buddy, or we must 
//							be his buddy. 
//		message		: [in]  An optional message to send alone with the request. 
//		callback	: [in]  This callback will get called repeatedly to get the 
//							list of files to send. See below. 
//		param		: [in]  Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function attempts to send files to a remote profile. The profile 
//		must be either on the local profile's buddy list, or the local profile 
//		must be on the remote profile's buddy list. To send the files, a direct 
//		connection must be established between the two profiles. If both are 
//		behind firewalls, or a direct connection cannot be established for any 
//		other reason, the transfer will fail.
//		<p />
//		A successful call to this function will create a transfer object (which 
//		is identified by transfer). This object will not be freed until either 
//		the connection is destroyed with gpDestroy(), or the object is 
//		explicitly freed with gpFreeTransfer(). The object is not automatically 
//		freed when the transfer completes. Updates on this transfer will be 
//		passed back to the application through the GP_TRANSFER_CALLBACK 
//		callback.
// Note
//		gpSendFilesW and gpSendFilesA are UNICODE and ANSI mapped versions of 
//		gpSendFiles. The arguments of gpSendFilesA are ANSI strings; those of 
//		gpSendFilesW are wide-character strings.
// See Also
//		gpFreeTranser, GPTransfer, gpGetTransferProgress, gpGetTransferData
COMMON_API GPResult gpSendFiles(
	GPConnection * connection,
	GPTransfer * transfer,
	GPProfile profile,
	const gsi_char * message,
	gpSendFilesCallback callback,
	void * param
);

///////////////////////////////////////////////////////////////////////////////
// gpAcceptTransfer
// Summary
//		This function is used to accept a file transfer request.
// Parameters
//      connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//      transfer	: [in] The transfer passed along with the 
//							GP_TRANSFER_SEND_REQUEST.
//      message		: [in] An optional message to send along with the accept.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to accept an incoming files request. This will 
//		initiate the transfer from the remote profile to the local profile. 
//		When done with the transfer, the transfer should be freed with a call 
//		to gpFreeTransfer.
// Note
//		gpAcceptTransferW and gpAcceptTransferA are UNICODE and ANSI mapped 
//		versions of gpAcceptTransfer. The arguments of gpAcceptTransferA are 
//		ANSI strings; those of gpAcceptTransferW are wide-character strings. 
// See Also 
//      gpRejectTransfer, gpSendFiles
COMMON_API GPResult gpAcceptTransfer(
	GPConnection * connection,
	GPTransfer transfer,
	const gsi_char * message
);

///////////////////////////////////////////////////////////////////////////////
// gpRejectTransfer
// Summary
//		This function is used to reject a file transfer request.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize.
//		transfer	: [in] A pointer to a GPTransfer object. 
//		message		: [in] An optional message to send along with the 
//							rejection.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to reject an incoming files request. This will 
//		also free the transfer, so it should not be referenced again once 
//		rejected.
// Note
//		gpRejectTransferW and gpRejectTransferA are UNICODE and ANSI mapped 
//		versions of gpRejectTransfer. The arguments of gpRejectTransferA are 
//		ANSI strings; those of gpRejectTransferW are wide-character strings.
// See Also 
//      gpSendFiles, gpAcceptTransfer
COMMON_API GPResult gpRejectTransfer(
	GPConnection * connection,
	GPTransfer transfer,
	const gsi_char * message
);

///////////////////////////////////////////////////////////////////////////////
// gpFreeTransfer
// Summary
//		This function is used to free a file transfer.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to free a transfer object. If the transfer has 
//		completed, then this will simply free the object's resources. If the 
//		transfer has not yet completed, this will also cancel the transfer, 
//		causing the remote profile to get a GP_TRANSFER_CANCELLED callback.
// See Also 
//      gpSendFiles, gpAcceptTransfer, gpRejectTransfer
COMMON_API GPResult gpFreeTransfer(
	GPConnection * connection,
	GPTransfer transfer
);

///////////////////////////////////////////////////////////////////////////////
// gpSetTransferData
// Summary
//		This function is used to store arbitrary user-data with a transfer.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		userData	: [in] Arbitrary user data to associate with the transfer. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function allows an application to associate arbitrary user-data 
//		with a transfer. 
COMMON_API GPResult gpSetTransferData(
	GPConnection * connection,
	GPTransfer transfer,
	void * userData
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferData
// Summary
//		This function is used to retrieve arbitrary user-data stored with a 
//		transfer.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
// Remarks
//      This function allows an application to retrieve arbitrary user-data 
//		stored with a transfer.
COMMON_API void * gpGetTransferData(
	GPConnection * connection,
	GPTransfer transfer
);

///////////////////////////////////////////////////////////////////////////////
// gpSetTransferDirectory
// Summary
//		This function can be used to set the directory that files are received 
//		into.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		directory	: [in] The directory to store received files in. This must 
//							be the path to a directory, and it must end in a 
//							slash or backslash. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This allows the application to set what directory in which received 
//		files are stored. They will all be stored in this directory, with names 
//		randomly generated by GP. It is then up to the application to place the 
//		files in the appropriate directories with the appropriate names. If no 
//		directory is set, the application will pick one. On win32, the 
//		GetTempPath function is used to pick a directory. If the application 
//		wants to set a directory explicitly, it must call this function before 
//		accepting the transfer. The function will fail if it is called after 
//		the transfer has started. This function only sets the directory for 
//		the specified transfer.
// Note
//		gpSetTransferDirectoryW and gpSetTransferDirectoryA are UNICODE and 
//		ANSI mapped versions of gpSetTransferDirectory. The arguments of 
//		gpSetTransferDirectoryA are ANSI strings; those of 
//		gpSetTransferDirectoryW are wide-character strings.
COMMON_API GPResult gpSetTransferDirectory(
	GPConnection * connection,
	GPTransfer transfer,
	const gsi_char * directory
);

//DOM-IGNORE-BEGIN
///////////////////////////////////////////////////////////////////////////////
// gpSetTransferThrottle
// Summary
//		This function can be used to set a throttle on a transfer. 
//		<p />
//		NOTE: Throttling is not currently implemented. Throttle information is 
//		transmitted between the local profile and remote profile, but no 
//		throttling actually occurs.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		throttle	: [in] The throttle setting. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		NOTE: Throttling is not currently implemented. 
//		<p />
//		This function can be used to either pause a transfer or limit a 
//		transfer to a maximum rate. It can be called by either the sender or 
//		the receiver. After a call to this function, both the local profile and 
//		remote profile will receive a gpTransferCallback of type 
//		GP_TRANSFER_THROTTLE.
GPResult gpSetTransferThrottle(
	GPConnection * connection,
	GPTransfer transfer,
	int throttle
);
//DOM-IGNORE-END

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferThrottle
// Summary
//		This function can be used to get a transfer's throttle setting. 
//		<p />
//		NOTE: Throttling is not currently implemented. Throttle information is 
//		transmitted between the local profile and remote profile, but no 
//		throttling actually occurs.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		throttle	: [out] The throttle setting is stored here.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      NOTE: Throttling is not currently implemented. 
//		<p />
//		This function is used to get the throttle setting for a transfer. If 
//		throttle is positive, it is the throttle setting in bytes-per-second. 
//		If zero, the transfer is paused. If -1, then there is no throttling.
COMMON_API GPResult gpGetTransferThrottle(
	GPConnection * connection,
	GPTransfer transfer,
	int * throttle
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferProfile
// Summary
//		This function is used to get the remote profile for a transfer.
// Parameters
//		connection	: [in]	A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]	A pointer to a GPTransfer object. 
//		profile		: [out] The remote profile is stored here. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the remote profile for a transfer.
COMMON_API GPResult gpGetTransferProfile(
	GPConnection * connection,
	GPTransfer transfer,
	GPProfile * profile
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferSide
// Summary
//		This function is used to get which side of the transfer the local 
//		profile is on (sending or receiving).
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		side		: [out] The side is stored here. This will be either 
//							GP_TRANSFER_SENDER or GP_TRANSFER_RECEIVER 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to determine if the local profile is the sender 
//		or receiver for this transfer. This is often useful inside of the 
//		gpTransferCallback when dealing with a message that both the sender 
//		and receiver may get, such as GP_FILE_END.
COMMON_API GPResult gpGetTransferSide(
	GPConnection * connection,
	GPTransfer transfer,
	GPEnum * side
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferSize
// Summary
//		This function is used to get the total size of the transfer, in bytes.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		size		: [out] The size of the transfer, in bytes, will be stored 
//							here. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to determine the total size of a file transfer. 
//		This is the sum of the sizes of all the files being transferred. When a 
//		file is transferred, its size may be different than the size originally 
//		reported for the file. This can cause the total size of the transfer to 
//		change during the course of the transfer.
COMMON_API GPResult gpGetTransferSize(
	GPConnection * connection,
	GPTransfer transfer,
	int * size
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransferProgress
// Summary
//		This function is used to get the total progress of the transfer, in 
//		bytes.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		progress	: [out] The progress of the transfer, in bytes, is stored 
//							here.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to determine the total progress of a file 
//		transfer. This is the total number of bytes of file data that have been 
//		transferred so far.
COMMON_API GPResult gpGetTransferProgress(
	GPConnection * connection,
	GPTransfer transfer,
	int * progress
);

///////////////////////////////////////////////////////////////////////////////
// gpGetNumFiles
// Summary
//		This function is used to get the number of files (including 
//		directories) being transferred.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A GPTransfer object. 
//		num			: [out] The number of files within the GPTransfer object.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the number of files being transferred. 
//		This total includes any directory names that are being sent.
COMMON_API GPResult gpGetNumFiles(
	GPConnection * connection,
	GPTransfer transfer,
	int * num
);

///////////////////////////////////////////////////////////////////////////////
// gpGetCurrentFile
// Summary
//		This function is used to get the current file being transferred.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A GP transfer object 
//		index		: [out] Returns the index of the current transferring file. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the index of the current file being 
//		transferred. This will be 0 until the first file is finished, then 1 
//		until the second file finishes, etc. When the transfer is complete, it 
//		will be set to the number of files in the transfer.
COMMON_API GPResult gpGetCurrentFile(
	GPConnection * connection,
	GPTransfer transfer,
	int * index
);

///////////////////////////////////////////////////////////////////////////////
// gpSkipFile
// Summary
//		This function is used to skip transferring a certain file.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		index		: [in] Index of the file within the GPTransfer object.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//		This function is used to skip a file in the transfer. It can be called 
//		either before a file is transferred or while a file is being 
//		transferred. If it is called before the file starts transferring, then 
//		the a GP_FILE_SKIP callback will be received when the file becomes the 
//		current file. If it is called while a file is being transferred, then 
//		the GP_FILE_SKIP will be called as soon as possible, and the file will 
//		stop transferring.
COMMON_API GPResult gpSkipFile(
	GPConnection * connection,
	GPTransfer transfer,
	int index
);

///////////////////////////////////////////////////////////////////////////////
// gpGetFileName
// Summary
//		This function is used to get the name of a file.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		index		: [in]  The index of the file within the GPTransfer object. 
//		name		: [out] The name of the file.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the name of a file in the transfer. The 
//		receiver should use this name to determine where to put the file after
//		it is received. It may be a simple name ("file.ext"), or it may contain 
//		a directory path ("files/file.ext"). Any slashes in the name will be 
//		UNIX-style slashes ("files/file.ext") as opposed to Windows style 
//		slashes ("files\file.ext").
// Note
//		gpGetFileNameW and gpGetFileNameA are UNICODE and ANSI mapped versions 
//		of gpGetFileName. The arguments of gpGetFileNameA are ANSI strings; 
//		those of gpGetFileNameW are wide-character strings.
COMMON_API GPResult gpGetFileName(
	GPConnection * connection,
	GPTransfer transfer,
	int index,
	gsi_char ** name
);

///////////////////////////////////////////////////////////////////////////////
// gpGetFilePath
// Summary
//		This function is used to get the local path to a file.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		index		: [in] The index of the file within the GPTransfer object. 
//		path		: [in] The path of the file.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the local path to a file. For the sender, 
//		this will be the same path specified in the gpSendFilesCallback. For 
//		the receiver, this will be NULL for directories and for files that 
//		haven't started transferring yet. For files that have are either 
//		transferring or have finished transferring, this is the local path 
//		where the file is being stored. It is the application's responsibility 
//		to move the file to an appropriate location (likely using the file's 
//		name) after the file has finished transferring.
// Note
//		gpGetFilePathW and gpGetFilePathA are UNICODE and ANSI mapped versions 
//		of gpGetFilePath. The arguments of gpGetFilePathA are ANSI strings; 
//		those of gpGetFilePathW are wide-character strings. 
COMMON_API GPResult gpGetFilePath(
	GPConnection * connection,
	GPTransfer transfer,
	int index,
	gsi_char ** path
);

///////////////////////////////////////////////////////////////////////////////
// gpGetFileSize
// Summary
//		This function is used to get the size of a file being transferred.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		index		: [in] The index of the file within the GPTransfer object. 
//		size		: [in] The size of the file.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the size of a file being transferred. The 
//		size of each file is checked when the transfer is initialized, which 
//		is the size that will be reported before the file is actually 
//		transferred. The size of the file is checked again when the file 
//		actually begins transferring, which the size that will be reported from 
//		that moment on (the two sizes will only be different if the file has 
//		changed during that time).
COMMON_API GPResult gpGetFileSize(
	GPConnection * connection,
	GPTransfer transfer,
	int index,
	int * size
);

///////////////////////////////////////////////////////////////////////////////
// gpGetFileProgress
// Summary
//		This function is used to get the progress of a file being transferred.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in] A pointer to a GPTransfer object. 
//		index		: [in] The index of the file within the GPTransfer object. 
//		progress	: [in] The transfer progress. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the progress of a file being transferred, 
//		or in other words, the number of bytes of the file either sent or 
//		received so far. If the file hasn't started transferring yet, the 
//		progress will be 0. The progress will be continually updated while the 
//		file is being transferred. If the file finishes transferring 
//		successfully, the progress should be the same as the file's size.
COMMON_API GPResult gpGetFileProgress(
	GPConnection * connection,
	GPTransfer transfer,
	int index,
	int * progress
);

///////////////////////////////////////////////////////////////////////////////
// gpGetFileModificationTime
// Summary
//		This function is used to get a file's timestamp.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		transfer	: [in]  A pointer to a GPTransfer object. 
//		index		: [in]  Index of the file within the GPTransfer object. 
//		modTime		: [out] The modification time.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the timestamp for a file being 
//		transferred. This is typically used by the receiver to set the file's 
//		timestamp correctly after a file has been received.
COMMON_API GPResult gpGetFileModificationTime(
	GPConnection * connection,
	GPTransfer transfer,
	int index,
	gsi_time * modTime
);

///////////////////////////////////////////////////////////////////////////////
// gpGetNumTransfers
// Summary
//		Returns the number of pending file transfers.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		num			: [out] The number of pending transfers.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      Returns the number of pending file transfers. 
COMMON_API GPResult gpGetNumTransfers(
	GPConnection * connection,
	int * num
);

///////////////////////////////////////////////////////////////////////////////
// gpGetTransfer
// Summary
//		Returns the GPTransfer object at the specified index.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		index		: [in]  Index of the GPTransfer object. 
//		transfer	: [out] A pointer to a GPTransfer object.
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      Returns the GPTransfer object at the specified index.
COMMON_API GPResult gpGetTransfer(
	GPConnection * connection,
	int index,
	GPTransfer * transfer
);
#endif

#ifdef _DEBUG

///////////////////////////////////////////////////////////////////////////////
// gpProfilesReport
// Summary
//		Debug function to dump information on known profiles to the console.
// Parameters
//		connection	: [in] A GP connection object initialized with 
//							gpInitialize. 
//		report		: [in] A user-supplied function to be triggered with each 
//							line of info. See remarks. 
// Remarks
//		This is a debug-only function that will dump the contents of the 
//		internal profile map to the user-supplied function.
//
//		The user-supplied function is most commonly printf.
COMMON_API void gpProfilesReport(
	GPConnection * connection,
	void (* report)(const char * output)
);
#endif

///////////////////////////////////////////////////////////////////////////////
// gpGetReverseBuddiesList
// Summary
//		Get a list of profiles that have the specified profiles as buddies.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		targets		: [out] A list of profiles for which to find reverse 
//							buddies. 
//		numOfTargets: [out] The length of the list of profiles in targets.
//		blocking	: [in]  GP_BLOCKING or GP_NON_BLOCKING.
//		callback	: [in]  A GP callback that will be passed a 
//							GPGetReverseBuddiesListResponseArg. 
//		param		: [in]  Pointer to user-defined data. This value will be 
//							passed unmodified to the callback function. 
// Returns 
//      GPResult is described in the GP enums section.
// Remarks
//		This function is used to find the reverse buddies of a list of 
//		profiles. A "reverse buddy" is a someone who has you on their buddy 
//		list. This function returns a valid GPResult. Common return values are: 
//		<p />
//		GP_NO_ERROR on success. <p />
//		GP_PARAMETER_ERROR is returned if connection is NULL, profile is 0, 
//		callback is NULL, or the connection is not connected. GP_MEMORY_ERROR 
//		is returned when an allocation fails. <p /> 
//		GP_NETWORK_ERROR is returned when there is a problem connecting to the 
//		Presence and Messaging backend.
// See Also 
//      GPGetProfileBuddyListArg
COMMON_API GPResult gpGetReverseBuddiesList(
	GPConnection * connection,
	GPProfile *targets, 
	int numOfTargets, 
	GPEnum blocking,
	GPCallback callback,
	void * param
);
#ifndef GP_NEW_STATUS_INFO

///////////////////////////////////////////////////////////////////////////////
// gpGetBuddyStatus
// Summary
//		This function gets the status for a particular buddy on the buddy list.
// Parameters
//		connection	: [in]  A GP connection object initialized with 
//							gpInitialize. 
//		index		: [in]  The array index of the buddy. 
//		status		: [out] The status of this buddy. 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid 
//		GPResult is returned.
// Remarks
//      This function is used to get the status of a particular buddy. index is 
//		a number greater than or equal to 0 and less than the total number of 
//		buddies. This function will typically be called in response to the 
//		gpRecvBuddyStatus callback being called.
COMMON_API GPResult gpGetBuddyStatus(
	GPConnection * connection,
	int index,
	GPBuddyStatus * status
);

#else
//DOM-IGNORE-BEGIN
///////////////////////////////////////////////////////////////////////////////
// gpGetBuddyStatusInfo
// Summary
//		Gets the buddy's status using the alternate buddy info system.
COMMON_API GPResult gpGetBuddyStatusInfo(
	GPConnection * connection,
	int index, 
	GPBuddyStatusInfo * statusInfo
);

///////////////////////////////////////////////////////////////////////////////
// gpSetBuddyAddr
// Summary
//		Sets the IP address using the alternate buddy info system.
COMMON_API GPResult gpSetBuddyAddr(
	GPConnection *connection, 
	int index,
	unsigned int buddyIp,
	unsigned short buddyPort
);

///////////////////////////////////////////////////////////////////////////////
// gpSetStatusInfo
// Summary
//		Sets buddy status info using the alternate buddy info system.
COMMON_API GPResult gpSetStatusInfo(
	GPConnection *connection, 
	GPEnum statusState,
	unsigned int hostIp, 
	unsigned int hostPrivateIp,
	unsigned short queryPort,
	unsigned short hostPort,
	unsigned int sessionFlags,
	const gsi_char *richStatus,
	int richStatusLen,
	const gsi_char *gameType,
	int gameTypeLen,
	const gsi_char *gameVariant,
	int gameVariantLen,
	const gsi_char *gameMapName,
	int gameMapNameLen
);

///////////////////////////////////////////////////////////////////////////////
// gpAddStatusInfoKey
// Summary
//		Adds a buddy key using the alternate buddy info system.
COMMON_API GPResult gpAddStatusInfoKey(
	GPConnection *connection, 
	const gsi_char *keyName, 
	const gsi_char *keyValue
);

///////////////////////////////////////////////////////////////////////////////
// gpSetStatusInfoKey
// Summary
//		Sets the value of a buddy key using the alternate buddy info system.
COMMON_API GPResult gpSetStatusInfoKey(
	GPConnection *connection, 
	const gsi_char *keyName, 
	const gsi_char *keyValue
);

///////////////////////////////////////////////////////////////////////////////
// gpGetStatusInfoKeyVal
// Summary
//		Gets the value of a buddy key using the alternate buddy info system.
COMMON_API GPResult gpGetStatusInfoKeyVal(
	GPConnection *connection,
	const gsi_char *keyName, 
	gsi_char **keyValue
);

///////////////////////////////////////////////////////////////////////////////
// gpDelStatusInfoKey
// Summary
//		Deletes a buddy key using the alternate buddy info system.
COMMON_API GPResult gpDelStatusInfoKey(
	GPConnection *connection, 
	const gsi_char *keyName
);

///////////////////////////////////////////////////////////////////////////////
// gpGetBuddyStatusInfoKeys
// Summary
//		Gets a list of the buddy keys using the alternate buddy info system.
COMMON_API GPResult gpGetBuddyStatusInfoKeys(
	GPConnection *connection, 
	int index, 
	GPCallback callback, 
	void *userData
);
//DOM-IGNORE-END
#endif

#ifdef _PS3
#include <np.h>
#include <np\common.h>
#include <sysutil/sysutil_common.h>
///////////////////////////////////////////////////////////////////////////////
// GPNpBasicCallback
// Summary
//		A generic NP Basic callback function type used to specify the callback supplied to 
//		GP SDK with gpRegisterNpBasicEventCallback.
// Parameters
//		incomeEvent	: [in] Event type (SCE_NP_BASIC_EVENT_XXX)
//		retCode		: [in] Error code
//		reqId       : [in] Request ID 
//		arg 		: [in] The user-data, if any, that was passed into the function
//						   that triggered this callback event.
// Remarks
//		This callback notifies the developer to take action on an incoming NP Basic 
//		event. Do not call sceNpBasicGetEvent within the function registered using
//		gpRegisterNpBasicEventCallback - instead flag this event, and call sceNpBasicGetEvent
//      from a separate event handling thread.  This restriction is in accordance with
//		Sony Documentation.
//		English Docs: https://ps3.scedev.net/docs/ps3-en/1
// See Also
//		gpRegisterNpBasicEventCallback
typedef int (*GPNpBasicEventCallback)(int incomeEvent,
									  int retCode, 
									  uint32_t reqId, 
									  void *arg);

///////////////////////////////////////////////////////////////////////////////
// gpRegisterNpBasicEventCallback
// Summary
//		Registers a developer specified callback to handle events for PS3 NP Basic
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		callback	: [in] The developer-supplied Np Basic event handler. 
//		param		: [in] Pointer to user-defined data. This value will be passed unmodified 
//							to the callback function.  
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid GPResult is returned.
// Remarks
//      This function sets the callback to call when there is a pending PS3 NP Basic event. 
//		If no callback is set, then no alert will be given when a Basic event occurs. 
//		<p>
//		If this callback is registered, make a call to sceNpBasicGetEvent after receiving the 
//      callback (but not within the callback) to clear the event from the queue. 
//		Please refer to Sony PS3 NP Basic documentation for this function and SceNpBasicEventHandler.
//      English Docs: https://ps3.scedev.net/docs/ps3-en/1
GPResult gpRegisterNpBasicEventCallback(GPConnection *connection,
										GPNpBasicEventCallback callback,
										void *param);

///////////////////////////////////////////////////////////////////////////////
// gpSetNpCommunicationId
// Summary
//		Set the NP Communication ID in order to perform GP/NP Buddy Sync
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		communicationId	: [in] The Sony-provided Communication ID (4 alphabetic characters and 5 
//                             numerical characters). 
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid GPResult is returned.
// Remarks
//      This must be called after initializing GP to set the Communication ID as per Sony requirement.
//		Please refer to Sony PS3 NP documentation for the SceNpCommunicationId structure description.
//      English Docs: https://ps3.scedev.net/docs/ps3-en/1
GPResult gpSetNpCommunicationId(GPConnection *connection, 
								const SceNpCommunicationId *communicationId);

///////////////////////////////////////////////////////////////////////////////
// gpRegisterNpBasicEventCallback
// Summary
//		Sets the slot number for GP to use when registering the cellSysutil callback
// Parameters
//		connection	: [in] A GP connection object initialized with gpInitialize. 
//		slotNum 	: [in] Slot number for GPs cellSysutil callback (0-3); make sure this 
//                      does not overlap with your own slot number passed to cellSysutilRegisterCallback.  
// Returns 
//      This function returns GP_NO_ERROR on success. Otherwise a valid GPResult is returned.
// Remarks
//      This function must be called before you connect GP (eg. gpConnect).
//		Please refer to Sony PS3 documentation for info on the cellSysutilRegisterCallback
//      function. English Docs: https://ps3.scedev.net/docs/ps3-en/1
GPResult gpRegisterCellSysUtilCallbackSlot(GPConnection *connection, int slotNum);

//DOM-IGNORE-BEGIN
// just for testing!
void gpiNpCellSysutilCallback(uint64_t status,
							  uint64_t param,
							  void * userdata);
//DOM-IGNORE-END
#endif

#ifdef __cplusplus
}
#endif

#endif
