///////////////////////////////////////////////////////////////////////////////
// File:	sb_serverbrowsing.h
// SDK:		GameSpy Server Browsing SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
// ------------------------------------
// Please see the GameSpy Server Browsing SDK documentation for more 
// information.

#ifndef _SB_SERVERBROWSING_H
#define _SB_SERVERBROWSING_H

#include "../common/gsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Type Definitions
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//ServerBrowser is an abstract data type used to represent the server list and
//		 query engine objects.
typedef struct _ServerBrowser *ServerBrowser;

//SBServer is an abstract data type representing a single server.
#ifndef SBServer 
typedef struct _SBServer *SBServer;
#endif	

///////////////////////////////////////////////////////////////////////////////
//  SBBool
//	Brief
//      Standard Boolean.
//
typedef enum {
	SBFalse,	// False.
	SBTrue		// True.
} SBBool;

///////////////////////////////////////////////////////////////////////////////
//  SBError
//	Brief
//      Error codes that can be returned from Server Browsing functions.
//
typedef enum 
{
	sbe_noerror,				// No error has occurred.
	sbe_socketerror,			// A socket function has returned an 
								// unexpected error.
	sbe_dnserror,				// DNS lookup of the master address failed.
	sbe_connecterror,			// Connection to the master server failed.
	sbe_dataerror,				// Invalid data was returned from the master 
								// server.
	sbe_allocerror,				// Memory allocation failed.
	sbe_paramerror,				// An invalid parameter was passed to a 
								// function.
	sbe_duplicateupdateerror	// Server update was requested on a server 
								// that was already being updated.
} SBError;	

///////////////////////////////////////////////////////////////////////////////
//  SBError
//	Brief
//      States the ServerBrowser object can be in.
//
typedef enum 
{
	sb_disconnected,	// Idle and not connected to the master server.
	sb_listxfer,		// Downloading list of servers from the master server.
	sb_querying,		// Querying servers.
	sb_connected		// Idle, but still connected to the master server.
} SBState;

///////////////////////////////////////////////////////////////////////////////
//  SBCallbackReason
//	Brief
//      Callbacks that can occur during server browsing operations.
//
typedef enum 
{
	sbc_serveradded,				// A server was added to the list, may 
									// just have an IP & port at this point.
	sbc_serverupdated,				// Server information has been updated - 
									// either basic or full information is now 
									// available about this server.
	sbc_serverupdatefailed,			// Either a direct or master server query 
									// to retrieve information about this 
									// server failed.
	sbc_serverdeleted,				// A server was removed from the list.
	sbc_updatecomplete,				// The server query engine is now idle.
	sbc_queryerror,					// The master returned an error string for 
									// the provided query.
	sbc_serverchallengereceived		// Prequery IP verification challenge was
									// received. (Informational, no action 
									// required.)
} SBCallbackReason;	

///////////////////////////////////////////////////////////////////////////////
//  SBConnectToServerState
//	Brief
//      States passed to the SBConnectToServerCallback.
//
typedef enum
{
	sbcs_succeeded,		// Connected to server successfully.
	sbcs_failed			// Failed to connect to server.
} SBConnectToServerState;

///////////////////////////////////////////////////////////////////////////////
//  ServerBrowserCallback
//	Brief
//      The callback provided to ServerBrowserNew. Gets called as the Server 
//		Browser updates the server list.
//  Parameters
//      sb				[in] The ServerBrowser object the callback is referring 
//							 to. 
//      reason			[in] The reason for being called. See SDK Doc for more 
//							 info. 
//      server			[in] The server that is being referred to. 
//      instance		[in] User provided data.
//  Remarks
//      "instance" is any game-specific data you want passed to the callback 
//		function. For example, you can pass a structure pointer or object 
//		pointer for use within the callback. If you can access any needed data 
//		within the callback already, then you can just pass NULL for "instance".
//	Example
//		Your callback function should look something like:
//		<code lang="c++">
//void SBCallback(ServerBrowser sb, SBCallbackReason reason, SBServer 
//						server, void *instance)
//		{
//			CMyGame *g = (CMyGame *)instance;
//		
//			switch (reason)
//			{
//				case sbc_serveradded :
//					g->ServerView->AddServerToList(server);
//					break;
//				case sbc_serverupdated : 
//					g->ServerView->UpdateServerInList(server);
//					break;
//				case sbc_updatecomplete:
//					g->ServerView->SetStatus("Update Complete");
//					break;
//				case sbc_queryerror:
//					g->ServerView->SetStatus("Query Error Occurred:", 
//					ServerBrowserListQueryError(sb));
//				break;
//			}
//		}
//		
//		Example use of the Callback:
//		int CMyGame::OnMultiplayerButtonClicked(...)
//		{
//		    m_ServerBrowser = ServerBrowserNew("mygame", "mygame", "123456", 
//												0, 10, QVERSION_QR2, 
//												SBCallBack, this);
//		}
//		</code>
//  See Also 
//      ServerBrowserNew
//
typedef void (*ServerBrowserCallback)(ServerBrowser sb, 
	SBCallbackReason reason, 
	SBServer server, 
	void *instance);

///////////////////////////////////////////////////////////////////////////////
//  SBConnectToServerCallback
//	Brief
//      The callback provided to ServerBrowserConnectToServer. Gets called 
//		when the state of the connect attempt changes.
//  Parameters
//      sb				[in] The ServerBrowser object the callback is referring 
//							 to. 
//      state			[in] The state of the connect attempt.
//      gameSocket		[in] A UDP socket, ready for use to communicate with 
//							 the server. 
//      instance		[in] User provided data.
//  Remarks
//      "instance" is any game-specific data you want passed to the callback 
//		function. For example, you can pass a structure pointer or object 
//		pointer for use within the callback. If you can access any needed data 
//		within the callback already, then you can just pass NULL for "instance".
//  See Also 
//      ServerBrowserConnectToServer
//
typedef void (*SBConnectToServerCallback)(ServerBrowser sb, SBConnectToServerState state, SOCKET gamesocket, struct sockaddr_in *remoteaddr, void *instance);

	//Maximum length for the SQL filter string
#define MAX_FILTER_LEN 511

	//Version defines for query protocol
#define QVERSION_GOA 0
#define QVERSION_QR2 1

	/*******************
	ServerBrowser Object Functions
	********************/

#ifndef GSI_UNICODE
#define ServerBrowserNew			ServerBrowserNewA
#define ServerBrowserUpdate			ServerBrowserUpdateA
#define ServerBrowserLimitUpdate	ServerBrowserLimitUpdateA
#define ServerBrowserAuxUpdateIP	ServerBrowserAuxUpdateIPA
#define ServerBrowserRemoveIP		ServerBrowserRemoveIPA
#define ServerBrowserSendNatNegotiateCookieToServer	ServerBrowserSendNatNegotiateCookieToServerA
#define ServerBrowserSendMessageToServer	ServerBrowserSendMessageToServerA
#define ServerBrowserSort			ServerBrowserSortA
#define SBServerGetStringValue		SBServerGetStringValueA
#define SBServerGetIntValue			SBServerGetIntValueA
#define SBServerGetFloatValue		SBServerGetFloatValueA
#define SBServerGetBoolValue		SBServerGetBoolValueA
#define SBServerGetPlayerStringValue	SBServerGetPlayerStringValueA
#define SBServerGetPlayerIntValue	SBServerGetPlayerIntValueA
#define SBServerGetPlayerFloatValue	SBServerGetPlayerFloatValueA
#define SBServerGetTeamStringValue	SBServerGetTeamStringValueA
#define SBServerGetTeamIntValue		SBServerGetTeamIntValueA
#define SBServerGetTeamFloatValue	SBServerGetTeamFloatValueA
#define ServerBrowserListQueryError	ServerBrowserListQueryErrorA
#define ServerBrowserErrorDesc		ServerBrowserErrorDescA
#define ServerBrowserGetServerByIP	ServerBrowserGetServerByIPA
#else
#define ServerBrowserNew			ServerBrowserNewW
#define ServerBrowserUpdate			ServerBrowserUpdateW
#define ServerBrowserLimitUpdate	ServerBrowserLimitUpdateW
#define ServerBrowserAuxUpdateIP	ServerBrowserAuxUpdateIPW
#define ServerBrowserRemoveIP		ServerBrowserRemoveIPW
#define ServerBrowserSendNatNegotiateCookieToServer	ServerBrowserSendNatNegotiateCookieToServerW
#define ServerBrowserSendMessageToServer	ServerBrowserSendMessageToServerW
#define ServerBrowserSort			ServerBrowserSortW
#define SBServerGetStringValue		SBServerGetStringValueW
#define SBServerGetIntValue			SBServerGetIntValueW
#define SBServerGetFloatValue		SBServerGetFloatValueW
#define SBServerGetBoolValue		SBServerGetBoolValueW
#define SBServerGetPlayerStringValue	SBServerGetPlayerStringValueW
#define SBServerGetPlayerIntValue	SBServerGetPlayerIntValueW
#define SBServerGetPlayerFloatValue	SBServerGetPlayerFloatValueW
#define SBServerGetTeamStringValue	SBServerGetTeamStringValueW
#define SBServerGetTeamIntValue		SBServerGetTeamIntValueW
#define SBServerGetTeamFloatValue	SBServerGetTeamFloatValueW
#define ServerBrowserListQueryError	ServerBrowserListQueryErrorW
#define ServerBrowserErrorDesc		ServerBrowserErrorDescW
#define ServerBrowserGetServerByIP	ServerBrowserGetServerByIPW
#endif

//////////////////////////////////////////////////////////////
// ServerBrowserNew
// Summary
//		Initialize the ServerBrowser SDK.
// Parameters
//		queryForGamename	: [in] Servers returned will be for this Gamename.
//		queryFromGamename	: [in] Your assigned GameName.
//		queryFromKey		: [in] Secret key that corresponds to the queryFromGamename.
//		queryFromVersion	: [in] Set to zero unless directed otherwise by GameSpy.
//		maxConcUpdates		: [in] The maximum number of queries the
//									ServerBrowsing SDK will send out at one time.
//									Must be set to 20 or greater per GameSpy Certification 
//									Process.
//		queryVersion		: [in] The QueryReporting protocol used by the
//									server. Should be QVERSION_GOA or QVERSION_QR2. See remarks.
//		lanBrowse			: [in] The switch to turn on only LAN browsing
//		callback			: [in] Function to be called when the operation completes.
//		instance			: [in] Pointer to user data. This is optional and will
//									be passed unmodified to the callback function.
// Returns
//		This function returns the initialized ServerBrowser interface. No
//		 return value is reserved to indicate an error.
// Remarks
//		The ServerBrowserNew function initializes the ServerBrowsing SDK.
//		 Developers should then use ServerBrowserUpdate or
//		 ServerBrowserLANUpdate to begin retrieving the list of registered
//		 game servers.<p>
// Example
//		(In this particular file, we should refer them to the ServerBrowser
//		 sample as it is very simple already.)
//		/* SERVERBROWSERNEW.C: This program uses ServerBrowserNew * to
//		 initialize the ServerBrowsing SDK */ 
//		<code lang="c++">
//#include <sb_serverbrowsing.h> 
//		void main( void )
//		{ 
//			ServerBrowser aServerBrowser = SBServerBrowserNew("gmtest",
//		 "HA6zkS", 0, 10, QVERSION_QR2, SBCallback, NULL); 
//		}
//		</code>
// See Also
//		ServerBrowserFree, ServerBrowserUpdate, ServerBrowserLANUpdate
ServerBrowser ServerBrowserNew(const gsi_char *queryForGamename, const gsi_char *queryFromGamename, const gsi_char *queryFromKey, int queryFromVersion, int maxConcUpdates, int queryVersion, SBBool lanBrowse, ServerBrowserCallback callback, void *instance);

//////////////////////////////////////////////////////////////
// ServerBrowserFree
// Summary
//		Frees memory allocated by the ServerBrowser SDK. Terminates any
//		 pending queries.
// Parameters
//		sb	: [in] A ServerBrowser interface previously allocated with
//		 ServerBrowserNew.
// Remarks
//		The ServerBrowserFree function frees any allocated memory associated
//		 with the SDK as 
//		well as terminates any pending queries. This function must be called
//		 once for every call to 
//		ServerBrowserNew to ensure proper cleanup of the ServerBrowsing SDK.<p>
// Example
//		<code lang="c++">
//#include <sb_serverbrowsing.h>
//		
//		void main( void )
//		{
//		    ServerBrowser aServerBrowser = ServerBrowserNew("gmtest",
//		 "gmtest", "HA6zkS", 0, 10, QVERSION_QR2, SBFalse, SBCallback, NULL);
//		
//		    ServerBrowserFree(aServerBrowser);
//		}
//		</code>
// See Also
//		ServerBrowserNew
void ServerBrowserFree(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserUpdate
// Summary
//		Retrieves the current list of games from the GameSpy master server.
// Parameters
//		sb						: [in] ServerBrowser object initialized with ServerBrowserNew.
//		async					: [in] When set to SBTrue this function
//		 will run in non-blocking mode.
//		disconnectOnComplete	: [in] When set to SBTrue this function will
//		 terminate the connection with the GameSpy master after the download is complete.
//		basicFields				: [in] A byte array of basic field
//		 identifiers to retrieve for each server. See remarks.
//		numBasicFields			: [in] The number of valid fields in the basicFields array.
//		serverFilter			: [in] SQL like string used to remove
//		 unwanted servers from the downloaded list.
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserUpdate function retrieves the current list of
//		 servers registered with the GameSpy master server.
//		As each server entry is received, one corresponding call to the
//		 SBCallback function will be made with 
//		the status sbc_serveradded, then if basic keys are not yet available 
//      (check with SBServerHasBasickeys) another call will be made for this 
//      server with status sbc_serverupdated.<p>
// See Also
//		ServerBrowserNew, ServerBrowserLANUpdate
SBError ServerBrowserUpdate(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const gsi_char *serverFilter);

//////////////////////////////////////////////////////////////
// ServerBrowserLimitUpdate
// Summary
//		Retrieves the current limited list of games from the GameSpy master
//		 server. Useful for low-memory systems.
// Parameters
//		sb						: [in] ServerBrowser object intialized with ServerBrowserNew.
//		async					: [in] When set to SBTrue this function
//		 will run in non-blocking mode.
//		disconnectOnComplete	: [in] When set to SBTrue this function will
//		 terminate the connection with the GameSpy master after the download is complete.
//		basicFields				: [in] A byte array of basic field
//		 identifiers to retreive for each server. See remarks.
//		numBasicFields			: [in] The number of valid fields in the basicFields array.
//		serverFilter			: [in] SQL like string used to remove
//		 unwanted servers from the downloaded list.
//		maxServers				: [in] Maximum number of servers to be returned
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserLimitUpdate function retrieves a limited set of the
//		 servers registered with the 
//		GameSpy master server. This is most useful for low memory systems
//		 such as the DS which may not be 
//		capable of loading an entire server list.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
SBError ServerBrowserLimitUpdate(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const gsi_char *serverFilter, int maxServers);


//////////////////////////////////////////////////////////////
// ServerBrowserThink
// Summary
//		Allows ServerBrowsingSDK to continue internal processing including
//		 processing query replies.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserThink function is required for the SDK to process
//		 incoming data. 
//		Because of the single threaded design of the GameSpy SDKs, all data
//		 is processed during this call, 
//		and processing is paused when this call is complete. When updating
//		 server lists, this function 
//		should be called as frequently as possible to reduce the latency
//		 associated with server response times. 
//		If this function is not called often enough, server pings may be
//		 inflated due to processing delays; ~10ms is ideal
// See Also
//		ServerBrowserNew
SBError ServerBrowserThink(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserLANUpdate
// Summary
//		Retrieves the current list of games broadcasting on the local network.
// Parameters
//		sb				: [in] ServerBrowser object initialized with ServerBrowserNew.
//		async			: [in] When set to SBTrue this function will run in
//		 non-blocking mode.
//		startSearchPort	: [in] The lowest port the SDK will listen to
//		 broadcasts from, in network byte order.
//		endSearchPort	: [in] The highest port the SDK will listen to
//		 broadcasts from, in network byte order.
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserLANUpdate function listens for broadcast packets on
//		 the local network. 
//		Servers that are broadcasting within the specified port range will
//		 be detected. 
//		As each server broadcast is received, one corresponding call to the
//		 SBCallbackfunction will 
//		be made with the status sbc_serveradded.<p>
//		Generally this should start with your standard query port, and range
//		 above it, since the QR and QR2 SDKs will
//		automatically allocate higher port numbers when running multiple
//		 servers on the same machine. 
//		You should limit your searcht to 100 ports or less in most cases to limit 
//		flooding of the LAN with broadcast packets.
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
SBError ServerBrowserLANUpdate(ServerBrowser sb, SBBool async, unsigned short startSearchPort, unsigned short endSearchPort);

//////////////////////////////////////////////////////////////
// ServerBrowserAuxUpdateIP
// Summary
//		Queries key/values from a single server.
// Parameters
//		sb			: [in] ServerBrowser object returned from ServerBrowserNew.
//		ip			: [in] Address string of the game server.
//		port		: [in] Query port of the game server.
//		viaMaster	: [in] Set to SBTrue to retrieve cached values from the
//		 master server.
//		async		: [in] Set to SBTrue to run in non-blocking mode.
//		fullUpdate	: [in] Set to SBTrue to retrieve the full set of
//		 key/values from the server.
// Returns
//		This function returns sbe_noerror for success. On an error
//		 condition, this function will return 
//		an SBError code. If the async option is SBTrue, the status condition
//		 will be reported to the SBCallback.
// Remarks
//		The ServerBrowserAuxUpdateIP function is used to retrieve
//		 information about a specific server. 
//		Information returned is in the form of key/value pairs and may be
//		 accessed through the standard 
//		SBServer object accessors.<p>
// See Also
//		ServerBrowserUpdate, ServerBrowserLANUpdate, ServerBrowserAuxUpdateServer
SBError ServerBrowserAuxUpdateIP(ServerBrowser sb, const gsi_char *ip, unsigned short port, SBBool viaMaster, SBBool async, SBBool fullUpdate);

//////////////////////////////////////////////////////////////
// ServerBrowserAuxUpdateServer
// Summary
//		Query key/values from a single server that has already been added to
//		 the internal list.
// Parameters
//		sb			: [in] ServerBrowser object returned from ServerBrowserNew.
//		server		: [in] SBServer object for the server to update.
//		 (usually obtained from SBCallback)
//		async		: [in] Set to SBTrue to run in non-blocking mode.
//		fullUpdate	: [in] Set to SBTrue to retrieve the full set of
//		 key/values from the server.
// Returns
//		This function returns sbe_noerror for success. On an error
//		 condition, this function will return an SBError code. 
//		If the async option is SBTrue, the status condition will be reported
//		 to the SBCallback.
// Remarks
//		The ServerBrowserAuxUpdateServer function is used to retrieve
//		 information about a specific server. 
//		Information returned is in the form of key/value pairs and may be
//		 accessed through the standard SBServer 
//		object accessors.<p>
//		Generally used to get additional information about a server 
//		(for example, to get full rules and player information from a server
//		 that only has basic information so far), 
//		but can also be used to "refresh" the information about a given
//		 server. Data will automatically be retrieved 
//		from the master server directly or from the game server as
//		 appropriate. When called asynchronously, 
//		multiple server update requests can be queued and will be executed
//		 by the query engine in turn.
// See Also
//		ServerBrowserUpdate, ServerBrowserLANUpdate, ServerBrowserAuxUpdateIP
SBError ServerBrowserAuxUpdateServer(ServerBrowser sb, SBServer server, SBBool async, SBBool fullUpdate);


//////////////////////////////////////////////////////////////
// ServerBrowserDisconnect
// Summary
//		Disconnect from the GameSpy master server.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Remarks
//		The ServerBrowserDisconnect function disconnects a maintained
//		 connection to the GameSpy master server.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
void ServerBrowserDisconnect(ServerBrowser sb);


//////////////////////////////////////////////////////////////
// ServerBrowserState
// Summary
//		Gets current state of the Server Browser object.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Returns
//		Returns the current state.
// Remarks
//		Descriptions of the possible state values can be found in the main
//		 header file.<p>
SBState ServerBrowserState(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserRemoveIP
// Summary
//		Removes a server from the local list.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		ip		: [in] The address of the server to remove.
//		port	: [in] The port of the server to remove, in network byte order.
// Remarks
//		The ServerBrowserRemoveIP function removes a single SBServer from
//		 the local list. 
//		This does not affect the backend or remote users. Please refer to
//		 the QR2 SDK for removing 
//		a registered server from the backend list.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserRemoveServer
void ServerBrowserRemoveIP(ServerBrowser sb, const gsi_char *ip, unsigned short port);

//////////////////////////////////////////////////////////////
// ServerBrowserRemoveServer
// Summary
//		Removes a server from the local list.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		server	: [in] The server to remove.
// Remarks
//		The ServerBrowserRemoveServer function removes a single SBServer
//		 from the local list. This does not 
//		affect the backend or remote users. Please refer to the QR2 SDK for
//		 removing a registered 
//		server from the backend list.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserRemoveIP
void ServerBrowserRemoveServer(ServerBrowser sb, SBServer server);

//////////////////////////////////////////////////////////////
// ServerBrowserHalt
// Summary
//		Stop an update in progress. 
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Remarks
//		The ServerBrowserHalt function will stop an update in progress. 
//		This is often tied to a "cancel" button presented to the user on the
//		 server list screen.
//		Clears any servers queued to be queried, and disconnects from the
//		 master server.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
void ServerBrowserHalt(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserClear
// Summary
//		Clear the current server list.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Remarks
//		The ServerBrowserClear function empties the current list of servers
//		 in preparation for a 
//		ServerBrowserUpdate or other list populating call.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserFree
void ServerBrowserClear(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserErrorDesc
// Summary
//		Returns a human readable string for the specified SBError.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		error	: [in] A valid SBError code.
// Returns
//		For a valid SBError, this function will return a human readable
//		 description. 
//		Otherwise this function returns an empty string.
// Remarks
//		The ServerBrowserErrorDesc function is usefull for displaying error
//		 information to a user 
//		that might not understand SBError codes. These descriptions are in
//		 english. For localization purposes, 
//		you will need to provide your own translation functions.<p>
// See Also
//		ServerBrowserNew, ServerBrowserListQueryError
const gsi_char *ServerBrowserErrorDesc(ServerBrowser sb, SBError error);

//////////////////////////////////////////////////////////////
// ServerBrowserListQueryError
// Summary
//		Returns the ServerList error string, if any.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Returns
//		If a list error has occurred, a string description of the error is
//		 returned. Otherwise, an empty 
//		string "" is returned.
// Remarks
//		The ServerBrowserListQueryError function returns the last string
//		 error reported by the server. 
//		For localization purposes, you may safely assume that this string
//		 will not change, and 
//		test for it as you would a numeric error code.<p>
// See Also
//		ServerBrowserNew
const gsi_char *ServerBrowserListQueryError(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserGetServer
// Summary
//		Returns the SBServer object at the specified index.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		index	: [in] The array index.
// Returns
//		Returns the SBServer at the specified array index. If index is
//		 greater than the 
//		bounds of the array, NULL is returned.
// Remarks
//		Use ServerBrowserCount to retrieve the current number of servers in
//		 the array. 
//		This index is zero based, so a list containing 5 servers will have
//		 the valid indexes 0 through 4. 
//		This list is usually populated using one of the list retrieval
//		 methods such as ServerBrowserUpdate.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
SBServer ServerBrowserGetServer(ServerBrowser sb, int index);

//////////////////////////////////////////////////////////////
// ServerBrowserGetServerByIP
// Summary
//		Returns the SBServer with the specified IP.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		ip		: [in] The dotted IP address of the server e.g. "1.2.3.4"
//		port	: [in] The query port of the server, in network byte order.
// Returns
//		Returns the Server if found, otherwise NULL;
SBServer ServerBrowserGetServerByIP(ServerBrowser sb, const gsi_char* ip, unsigned short port);

//////////////////////////////////////////////////////////////
// ServerBrowserCount
// Summary
//		Retrieves the current list of games from the GameSpy master server.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Returns
//		Returns the number of servers in the current list. The index is zero
//		 based when referencing.
// Remarks
//		The ServerBrowserCount function returns the number of servers in the
//		 current list. 
//		This may be a combination of servers returned by
//		 ServerBrowserUpdate and servers added manually 
//		by ServerBrowserAuxUpdateIP. Please note that index functions such
//		 as ServerBrowserGetServer use 
//		a zero based index. The actual valid indexes are 0 to
//		 ServerBrowserCount()-1.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
int ServerBrowserCount(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserPendingQueryCount
// Summary
//		Retrieves the number of servers with outstanding queries. Use this to check 
//		progress while asynchronously updating the server list.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew.
// Returns
//		Returns the number of servers that have not yet been queried.
// Remarks
//		The ServerBrowserPendingQueryCount function is most useful when
//		 updating a large list of servers. 
//		Use this function to display progress information to the user. For
//		 example "1048/2063 servers updated", 
//		or as a progress bar graphic.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
int ServerBrowserPendingQueryCount(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserGetMyPublicIP
// Summary
//		Returns the local client's external (firewall) address.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew
// Returns
//		The local clients external (firewall) address.  This may be returned
//		 as a string or integer address.
// Remarks
//		The ServerBrowserGetMyPublicIP and ServerBrowserGetMyPublicIPAddr
//		 functions return the external address 
//		of the local client, as report by the GameSpy Master Server.
//		Because of this, the return value is only valid after a successful
//		 call to ServerBrowserUpdate. 
//		The reason for this is that a client cannot determine their external
//		 address without first sending 
//		an outgoing packet.
//		It is up to the receiver of that packet to report the public address
//		 back to the local client.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
char *ServerBrowserGetMyPublicIP(ServerBrowser sb);

//////////////////////////////////////////////////////////////
// ServerBrowserGetMyPublicIPAddr
// Summary
//		Returns the local client's external (firewall) address.
// Parameters
//		sb	: [in] ServerBrowser object initialized with ServerBrowserNew
// Returns
//		The local clients external (firewall) address.  This may be returned
//		 as a string or integer address.
// Remarks
//		The ServerBrowserGetMyPublicIP and ServerBrowserGetMyPublicIPAddr
//		 functions return the external 
//		address of the local client, as report by the GameSpy Master Server.
//		Because of this, the return value is only valid after a successful
//		 call to ServerBrowserUpdate. 
//		The reason for this is that a client cannot determine their external
//		 address without first 
//		sending an outgoing packet.
//		It is up to the receiver of that packet to report the public address
//		 back to the local client.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate
unsigned int ServerBrowserGetMyPublicIPAddr(ServerBrowser sb);


//////////////////////////////////////////////////////////////
// ServerBrowserSendNatNegotiateCookieToServer
// Summary
//		Sends a NAT negotiation cookie to the server. The cookie is sent via
//		 the master server.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		ip		: [in] Address of the server in string form. "xxx.xxx.xxx.xxx"
//		port	: [in] The query port of the server to relay the NatNeg
//		 cookie to, in network byte order.
//		cookie	: [in] An integer cookie value. See remarks.
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserSendNatNegotiateCookieToServer function can be used
//		 to relay a NatNegotiation cookie 
//		value to a server behind a firewall. This cookie is sent through the
//		 backend since direct communication 
//		with the server is not always possible. This cookie may then be used
//		 to initiate a nat negotiation attempt. 
//		Please refer to the NatNegotiation SDK documentation for more info.<p>
// See Also
//		ServerBrowserNew, ServerBrowserLANUpdate
SBError ServerBrowserSendNatNegotiateCookieToServer(ServerBrowser sb, const gsi_char *ip, unsigned short port, int cookie);


//////////////////////////////////////////////////////////////
// ServerBrowserSendMessageToServer
// Summary
//		Sends a game specific message to the specified IP/port. This message
//		 is routed through the master server.
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		ip		: [in] Address of the server in string form. "xxx.xxx.xxx.xxx"
//		port	: [in] The query port of the server to send the message to,
//		 in network byte order.
//		data	: [in] The raw data buffer.
//		len		: [in] The length of the data buffer.
// Returns
//		If an error occurs, a valid SBError error code is returned.
//		 Otherwise, sbe_noerror is returned.
// Remarks
//		The ServerBrowserSendMessageToServer function can be used to relay a
//		 raw data buffer to a server 
//		behind a firewall. The raw buffer is sent through the backend since
//		 direct communication with the 
//		server is not always possible. The buffer is sent in raw form to the
//		 server's query port and does 
//		not contain any header information. This message is most usefull in
//		 a shared socket QR2 implementation.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate,
//		 ServerBrowserSendNatNegotiateCookieToServer
SBError ServerBrowserSendMessageToServer(ServerBrowser sb, const gsi_char *ip, unsigned short port, const char *data, int len);


//////////////////////////////////////////////////////////////
// ServerBrowserConnectToServer
// Summary
//		Connects to a game server.
// Parameters
//		sb			: [in] ServerBrowser object returned from ServerBrowserNew.
//		server		: [in] SBServer object for the server to connect to.
//		callback	: [in] The callback to call when the attempt completes.
// Returns
//		This function returns sbe_noerror for success. On an error
//		 condition, this function will return an SBError code.  
//		If there is an error, the callback will not be called.
// Remarks
//		Connects to a game server, internally using Nat Negotiation if necessary.
//		The callback will be called when the connection attempt completes.
//		If the attempt is successful, the server will have its
//		 qr2_clientconnectedcallback_t called.<p>
// See Also
//		QR2\qr2_clientconnectedcallback_t, SBConnectToServerCallback
SBError ServerBrowserConnectToServer(ServerBrowser sb, SBServer server, SBConnectToServerCallback callback);


//////////////////////////////////////////////////////////////
// ServerBrowserConnectToServerWithSocket
// Summary
//		Connects to a game server with the gamesocket provided.
// Parameters
//		sb			: [in] ServerBrowser object returned from ServerBrowserNew.
//		server		: [in] SBServer object for the server to connect to.
//      gamesocket  : [in] Socket to be used in establishing connection with
//		 the specified server. 
//                         This socket must have already been initialized. 
//		callback	: [in] The callback to call when the attempt completes.
// Returns
//		This function returns sbe_noerror for success. On an error
//		 condition, this function will return an SBError code.  
//		If there is an error, the callback will not be called.
// Remarks
//		Connects to a game server, internally using Nat Negotiation if necessary.
//      The gamesocket passed in will be used in establishing connection with
//		 the specified server.   
//      The developer is responsible for receiving traffic on this socket and
//		 passing received 
//      NN messages to NNProcessData. NN packets can be identified by the 6
//		 magic bytes that are used 
//      at the beginning of every packet. These are defined in natneg.h
//		 starting with NN_MAGIC_0.
//		The callback will be called when the connection attempt completes.
//		If the attempt is successful, the server will have its
//		 qr2_clientconnectedcallback_t called.<p>
// See Also
//		QR2\qr2_clientconnectedcallback_t, SBConnectToServerCallback,
//		 natneg\NNProcessData
SBError ServerBrowserConnectToServerWithSocket(ServerBrowser sb, SBServer server, SOCKET gamesocket, SBConnectToServerCallback callback);


///////////////////////////////////////////////////////////////////////////////
//  SBCompareMode
//	Brief
//      Comparison types for the ServerBrowserSort function.
// See Also
//		ServerBrowserSort
typedef enum {
	sbcm_int,		// Assume the values are int, and do an integer comparison.
	sbcm_float,		// Assume the values are float, and do a float comparison.
	sbcm_strcase,	// Assume the values are strings, and do a case-sensitive 
	// string comparison.
	sbcm_stricase	// Assume the values are strings, and do a case-insensitive 
	// string comparison.
} SBCompareMode;


//////////////////////////////////////////////////////////////
// ServerBrowserSort
// Summary
//		Sort the current list of servers.
// Parameters
//		sb			: [in] ServerBrowser object initialized with ServerBrowserNew.
//		ascending	: [in] When set to SBTrue this function will sort in
//		 ascending order. (a-b-c order, not c-b-a)
//		sortkey		: [in] The "key" of the key/value pair to sort by.
//		comparemode	: [in] Specifies the data type of the sortkey. See remarks.
// Remarks
//		The ServerBrowserSort function will order the server list, sorting by the specified sortkey.
//		Sorting may be in ascending or descending order and various
//		 data-types are supported.
//		SBCompareMode may be one of the following values:<p>
//		sbcm_int: Uses integer comparison. "1,2,3,12,15,20"
//		sbcm_float: Similar to above but considers decimal values. "1.1,1.2,2.1,3.0"
//		sbcm_strcase: Uses case sensitive string comparison. Uses strcmp. 
//		sbcm_stricase: Case in-sensitive string comparision. Uses _stricmp
//		 or equivilent.
//		Please note that calling this function repeatedly for a large server
//		 list may impact performance. 
//		This is due to the standard qsort algorithm being ineffecient when
//		 sorting an already ordered list. 
//		This is rarely a cause for concern, but certain optimizations may be
//		 made if performance is noticeably impacted.
// See Also
//		ServerBrowserUpdate, ServerBrowserThink
void ServerBrowserSort(ServerBrowser sb, SBBool ascending, const gsi_char *sortkey, SBCompareMode comparemode);

//////////////////////////////////////////////////////////////
// ServerBrowserLANSetLocalAddr
// Summary
//		Sets the network adapter to use for LAN broadcasts (optional).
// Parameters
//		sb		: [in] ServerBrowser object initialized with ServerBrowserNew.
//		theAddr	: [in] The address to use.
void ServerBrowserLANSetLocalAddr(ServerBrowser sb, const char* theAddr);


/*******************
SBServer Object Functions
********************/

//////////////////////////////////////////////////////////////
// SBServerKeyEnumFn
// Summary
//		Callback function used for enumerating the keys/values for a server.
// Parameters
//		key			: [in] The enumerated key.
//		value		: [in] The enumerated value.
//		instance	: [in] User provided data.
// See Also
//		SBServerEnumKeys
typedef void (*SBServerKeyEnumFn)(gsi_char *key, gsi_char *value, void *instance);

//////////////////////////////////////////////////////////////
// SBServerGetConnectionInfo
// Summary
//		Checks if Nat Negotiation is required, based off whether it is a LAN game, 
//		a public IP is present and several other factors. Fills a supplied
//		 pointer with an IP 
//		string to use for Nat Negotiation, or a direct connection if possible.
// Parameters
//		gSB				: [in] ServerBrowser object returned from ServerBrowserNew.
//		server			: [in] A valid SBServer object.
//		portToConnectTo	: [in] The game port to connect to.
//		ipstring_out	: [out] An IP String you can use for a direct
//		 connection, or to attempt Nat Negotiation with.
// Returns
//		Returns SBTrue if Nat Negotiation is required, SBFalse if not.
// Remarks
//		The connection test will result in one of three scenarios, based
//		 upon the return value of the function.<p>
//		Returns SBFalse:
//		1) LAN game - connect using the IP string.
//		2) Internet game with a direct connection available - connect using
//		 the IP string.
//		
//		Returns SBTrue:
//		3) Nat traversal required, perform Nat Negotiation with the IP
//		 string before connecting.

SBBool SBServerGetConnectionInfo(ServerBrowser gSB, SBServer server, gsi_u16 PortToConnectTo, char *ipstring_out);


//////////////////////////////////////////////////////////////
// SBServerHasPrivateAddress
// Summary
//		Tests to see fi a private address is available for the server.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		Returns SBTrue if the server has a private address; otherwise SBFalse.
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
SBBool SBServerHasPrivateAddress(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerDirectConnect
// Summary
//		Indicates whether the server supports direct UDP connections.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		Returns SBTrue if a direct connection is possible, otherwise SBFalse.
// Remarks
//		A return of SBFalse usually means that NatNegotiation is required. Note: this should
//      only be used to check public servers (where SBServerHasPrivateAddress returns SBFalse)<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
SBBool SBServerDirectConnect(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPing
// Summary
//		Returns the stored ping time for the specified server.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The stored server response time.
// Remarks
//		The SBServerGetPing function will return the stored response time of
//		 the server. 
//		This response time is caculated from the last server update.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
int SBServerGetPing(SBServer server);


//////////////////////////////////////////////////////////////
// SBServerGetPublicAddress
// Summary
//		Returns the external address of the SBServer, if any. For users
//		 behind a NAT or firewall, 
//		this is the address of the outermost NAT or firewall layer.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The public address of the SBServer, in string or integer form.
// Remarks
//		When a client machine is behind a NAT or Firewall device,
//		 communication must go through the public address.
//		The public address of the SBServer is the address of the outermost
//		 Firewall or NAT device.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 be accessible for servers in the list. 
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
char *SBServerGetPublicAddress(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPrivateAddress
// Summary
//		Returns the internal address of the SBServer, if any.  For users
//		 behind a NAT or firewall, 
//		this is the local DHCP or assigned IP address of the machine.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The private address of the SBServer, in string or integer form.
// Remarks
//		When a client machine is behind a NAT or Firewall device,
//		 communication must go through the public address.
//		The private address may be used by clients behind the same NAT or
//		 Firewall, and may be used 
//		to specifically identify two clients with the same public address.
//		Often the private address is of the form "192.168.###.###" and is not
//		 usable for communication outside 
//		the local network.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 be accessible for servers in the list. 
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
char *SBServerGetPrivateAddress(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPublicInetAddress
// Summary
//		Returns the external address of the SBServer, if any. 
//		For users behind a NAT or firewall, this is the address of the
//		 outermost NAT or firewall layer.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The public address of the SBServer, in string or integer form.
// Remarks
//		When a client machine is behind a NAT or Firewall device,
//		 communication must go through the public address.
//		The public address of the SBServer is the address of the outermost
//		 Firewall or NAT device.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 be accessible for servers in the list. 
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
unsigned int SBServerGetPublicInetAddress(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPrivateInetAddress
// Summary
//		Returns the internal address of the SBServer, if any.  
//		For users behind a NAT or firewall, this is the local DHCP or
//		 assigned IP address of the machine.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The private address of the SBServer, in string or integer form.
// Remarks
//		When a client machine is behind a NAT or Firewall device,
//		 communication must go through the public address.
//		The private address may be used by clients behind the same NAT or
//		 Firewall, and may be used to 
//		specifically identify two clients with the same public address.
//		Often the private address is of the form "192.168.###.###" and is not
//		 usable for communication 
//		outside the local network.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 be accessible for servers in the list. 
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
unsigned int SBServerGetPrivateInetAddress(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPublicQueryPort
// Summary
//		Returns the public query port of the specified server. 
//		This is the external port on which the GameSpy backend communicates
//		 with the server.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The public query port.
// Remarks
//		The SBServerGetPublicQueryPort function will return the public query
//		 port of the server.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
unsigned short SBServerGetPublicQueryPort(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetPrivateQueryPort
// Summary
//		Returns the private query port of the specified server. 
//		This is the internal port on which the server communicates to the
//		 GameSpy backend.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		The private query port.
// Remarks
//		The SBServerGetPrivateQueryPort function will return the private
//		 query port of the server.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
unsigned short SBServerGetPrivateQueryPort(SBServer server);


//////////////////////////////////////////////////////////////
// SBServerHasBasicKeys
// Summary
//		Determine if basic information is available for the specified server.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		SBTrue if available; otherwise SBFalse.
// Remarks
//		The SBServerHasBasicKeys function is used to determine if the server object
//		has been populated with the 'basicFields' keys as passed to the ServerBrowserUpdate. 
//		Information may not be available if a server query is still pending.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
SBBool SBServerHasBasicKeys(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerHasFullKeys
// Summary
//		Determine if full information is available for the specified server.
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		SBTrue if available; otherwise SBFalse.
// Remarks
//		The SBServerHasFullKeys function is used to determine if the server object has been populated with 
//      all keys reported by the server. 'Full' server information is retrieved after a ServerBrowserAuxUpdate  
//		call. Information may not be available if a server query is still pending.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
SBBool SBServerHasFullKeys(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerHasValidPing
// Summary
//		Determines if a server has a valid ping value (otherwise the ping
//		 will be 0).
// Parameters
//		server	: [in] A valid SBServer object.
// Returns
//		SBTrue if the server has a valid ping value, otherwise SBFalse.
SBBool SBServerHasValidPing(SBServer server);

//////////////////////////////////////////////////////////////
// SBServerGetStringValue
// Summary
//		Returns the value associated with the specified key. 
//		This value is returned as the appropriate type. SBBool, float, int
//		 or string.
// Parameters
//		server	: [in] A valid SBServer object.
//		keyname	: [in] The value associated with this key will be returned.
//		def		: [in] The value to return if the key is not found.
//                     Note: this default string will be returned if the key has been reported as an empty string.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native
//		 data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found or is reported as an empty string, the
//		 supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
const gsi_char *SBServerGetStringValue(SBServer server, const gsi_char *keyname, const gsi_char *def);

//////////////////////////////////////////////////////////////
// SBServerGetIntValue
// Summary
//		Returns the value associated with the specified key. This value is
//		 returned as the 
//		appropriate type: SBBool, float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		key			: [in] The value associated with this key will be returned.
//		idefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.
//		  For an existing key, 
//		the value is converted from string form to the appropriate data
//		 type.  These functions do not 
//		perform any type checking.
// Remarks
//		These functions are usefull for converting custom keys to a native
//		 data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found,the supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
int SBServerGetIntValue(SBServer server, const gsi_char *key, int idefault);

//////////////////////////////////////////////////////////////
// SBServerGetFloatValue
// Summary
//		Returns the value associated with the specified key. This value is returned 
//		as the appropriate type: SBBool, float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		key			: [in] The value associated with this key will be returned.
//		fdefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native
//		 data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found,the supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
double SBServerGetFloatValue(SBServer server, const gsi_char *key, double fdefault);

//////////////////////////////////////////////////////////////
// SBServerGetBoolValue
// Summary
//		Returns the value associated with the specified key. This value is
//		 returned as the 
//		appropriate type: SBBool, float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		key			: [in] The value associated with this key will be returned.
//		bdefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native
//		 data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found,the supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
SBBool SBServerGetBoolValue(SBServer server, const gsi_char *key, SBBool bdefault);


//////////////////////////////////////////////////////////////
// SBServerGetPlayerStringValue
// Summary
//		Returns the value associated with the specified player's key. 
//		This value is returned as the appropriate type. Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		playernum	: [in] The zero based index for the desired player.
//		key			: [in] The value associated with this key will be returned.
//		sdefault	: [in] The value to return if the key is not found.
//                         Note: this default string will be returned if the key has been reported as an empty string.
// Returns
//		If the player or key is invalid or missing, the specified default is
//		 returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom player keys to a
//		 native data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found or is reported as an empty string, the
//		 supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
const gsi_char *SBServerGetPlayerStringValue(SBServer server, int playernum, const gsi_char *key, const gsi_char *sdefault);

//////////////////////////////////////////////////////////////
// SBServerGetPlayerIntValue
// Summary
//		Returns the value associated with the specified player's key. 
//		This value is returned as the appropriate type. Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		playernum	: [in] The zero based index for the desired player.
//		key			: [in] The value associated with this key will be returned.
//		idefault	: [in] The value to return if the key is not found.
// Returns
//		If the player or key is invalid or missing, the specified default is
//		 returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom player keys to a
//		 native data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found,the supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
int SBServerGetPlayerIntValue(SBServer server, int playernum, const gsi_char *key, int idefault);

//////////////////////////////////////////////////////////////
// SBServerGetPlayerFloatValue
// Summary
//		Returns the value associated with the specified player's key. 
//		This value is returned as the appropriate type. Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		playernum	: [in] The zero based index for the desired player.
//		key			: [in] The value associated with this key will be returned.
//		fdefault	: [in] The value to return if the key is not found.
// Returns
//		If the player or key is invalid or missing, the specified default is
//		 returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom player keys to a
//		 native data type. 
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type. 
//		If a key is not found,the supplied default is returned.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
double SBServerGetPlayerFloatValue(SBServer server, int playernum, const gsi_char *key, double fdefault);

//////////////////////////////////////////////////////////////
// SBServerGetTeamStringValue
// Summary
//		Returns the value associated with the specified teams' key. 
//		This value is returned as the appropriate type: Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		teamnum		: [in] The integer index of the team.
//		key			: [in] The value associated with this key will be returned.
//		sdefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native data type.
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type.
//		If a key is not found or is reported as an empty string, the
//		 supplied default is returned.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 exist for servers in the list. 
//		IP addresses removed from the server list will not have an SBServer
//		 object associated.<p>
//		Team indexes are determined on a per-game basis. The only
//		 requirement is that they match the 
//		server's reporting indexes.
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
const gsi_char *SBServerGetTeamStringValue(SBServer server, int teamnum, const gsi_char *key, const gsi_char *sdefault);

//////////////////////////////////////////////////////////////
// SBServerGetTeamIntValue
// Summary
//		Returns the value associated with the specified teams' key. 
//		This value is returned as the appropriate type: Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		teamnum		: [in] The integer index of the team.
//		key			: [in] The value associated with this key will be returned.
//		idefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native data type.
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type.
//		If a key is not found,the supplied default is returned.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 exist for servers in the list. 
//		IP addresses removed from the server list will not have an SBServer
//		 object associated.<p>
//		Team indexes are determined on a per-game basis. The only
//		 requirement is that they match the 
//		server's reporting indexes.
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
int SBServerGetTeamIntValue(SBServer server, int teamnum, const gsi_char *key, int idefault);

//////////////////////////////////////////////////////////////
// SBServerGetTeamFloatValue
// Summary
//		Returns the value associated with the specified teams' key. 
//		This value is returned as the appropriate type: Float, int or string.
// Parameters
//		server		: [in] A valid SBServer object.
//		teamnum		: [in] The integer index of the team.
//		key			: [in] The value associated with this key will be returned.
//		fdefault	: [in] The value to return if the key is not found.
// Returns
//		If the key is invalid or missing, the specified default is returned.  
//		For an existing key, the value is converted from string form to the
//		 appropriate data type.  
//		These functions do not perform any type checking.
// Remarks
//		These functions are useful for converting custom keys to a native data type.
//		No type checking is performed, the string value is simply cast to
//		 the appropriate data type.
//		If a key is not found,the supplied default is returned.<p>
//		The SBServer object may be obtained during the SBCallback from
//		 ServerBrowserUpdate, 
//		or by calling ServerBrowserGetServer. An SBServer object will only
//		 exist for servers in the list. 
//		IP addresses removed from the server list will not have an SBServer
//		 object associated.<p>
//		Team indexes are determined on a per-game basis. The only
//		 requirement is that they match the 
//		server's reporting indexes.
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
double SBServerGetTeamFloatValue(SBServer server, int teamnum, const gsi_char *key, double fdefault);


//////////////////////////////////////////////////////////////
// SBServerEnumKeys
// Summary
//		Enumerates the keys/values for a given server by calling KeyEnumFn
//		 with each key/value. 
//		The user-defined instance data will be passed to the KeyFn callback.
// Parameters
//		server		: [in] A valid SBServer object.
//		KeyFn		: [in] A callback that is called once for each key.
//		instance	: [in] A user-defined data value that will be passed
//		 into each call to KeyFn.
// Remarks
//		The SBServerEnumKeys function is used to list the available keys for
//		 a particular SBServer object. 
//		This is often useful when the number of keys or custom keys is
//		 unknown or variable. 
//		Most often, the number of keys is predefined and constant, making
//		 this function call unnecessary. 
//		No query is sent when enumerating keys, instead the keys are stored
//		 from the previous server update.<p>
// See Also
//		ServerBrowserNew, ServerBrowserUpdate, ServerBrowserGetServer
void SBServerEnumKeys(SBServer server, SBServerKeyEnumFn KeyFn, void *instance);


#ifdef __cplusplus
}
#endif

#endif 
