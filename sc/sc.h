///////////////////////////////////////////////////////////////////////////////
// File:	sc.h
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SC_H__
#define __SC_H__


//Includes
///////////////////////////////////////////////////////////////////////////////
#include "../common/gsCommon.h"
#include "../common/gsRC4.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttp.h"
#include "../webservices/AuthService.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// optional to explicitly use __stdcall, __cdecl, __fastcall
#define SC_CALL


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Set this to define memory settings for the SDK
#define SC_STATIC_MEM

// The initial (or fixed, for static memory) report buffer size
#define SC_REPORT_BUFFER_BYTES 65536

// URL for sc services.
#define SC_SERVICE_MAX_URL_LEN 128
extern char scServiceURL[SC_SERVICE_MAX_URL_LEN];
extern char scGameConfigDataServiceURL[SC_SERVICE_MAX_URL_LEN];

// Session GUID size - must match backend
//#define SC_SESSION_GUID_SIZE 16
#define SC_AUTHDATA_SIZE 16
#define SC_SESSION_GUID_SIZE 40
#define SC_CONNECTION_GUID_SIZE 40

#define SC_GUID_BINARY_SIZE 16 // convert the 40 byte string guid into an int, 2 shorts and 8 bytes

// Limit to the number of teams
#define SC_MAX_NUM_TEAMS	64

// OPTIONS flags - first two bits reserved for authoritative / final flags
#define SC_OPTIONS_NONE					0

// These length values come from the corresponding ATLAS DB column definitions.
#define SC_CATEGORY_MAX_LENGTH 64
#define SC_STAT_NAME_MAX_LENGTH 128



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Result codes


//////////////////////////////////////////////////////////////
// SCResult
// Summary
//		used for checking errors and failures.
// Remarks
//		Results of a call to an interface function or operation.  
//		It can be used to see if the initial call to a function completed without error.  
//		The callback that is passed to interface functions will also have a value that is of this type.  
//		The application can check this value for failures.<p>
typedef enum
{
	SCResult_NO_ERROR = 0,			// No error has occurred.
	SCResult_NO_AVAILABILITY_CHECK,	// The standard GameSpy Availability Check was not performed prior to initialization.
	SCResult_INVALID_PARAMETERS,	// Parameters passed to interface function were invalid.
	SCResult_NOT_INITIALIZED,		// The SDK was not initialized.
	SCResult_CORE_NOT_INITIALIZED,	// The core was initialized by the application.
	SCResult_OUT_OF_MEMORY,			// The SDK could not allocate memory for its resources.
	SCResult_CALLBACK_PENDING,		// Result tell the application, that the operation is still pending.

	SCResult_HTTP_ERROR,			// Error occurs if the backend fails to respond with correct HTTP.
	SCResult_UNKNOWN_RESPONSE,		// Error occurs if the SDK cannot understand the result.
	SCResult_RESPONSE_INVALID,		// Error occurs if the SDK cannot read the response from the backend.
	SCResult_INVALID_DATATYPE,		// 

	SCResult_REPORT_INCOMPLETE,		// The report was incomplete.
	SCResult_REPORT_INVALID,		// Part or all of report is invalid.
	SCResult_SUBMISSION_FAILED,		// Submission of report failed.

	SCResult_UNKNOWN_ERROR,			// Error unknown to sdk.

	SCResultMax						// Total number of result codes that can be returned.
} SCResult;


//////////////////////////////////////////////////////////////
// SCGameResult
// Summary
//		Used when submitting a report for a game session to reflect the player's result.
// Remarks
//		Can be used for both player and a team.<p>
typedef enum
{
	SCGameResult_WIN,
	SCGameResult_LOSS,
	SCGameResult_DRAW,
	SCGameResult_DISCONNECT,
	SCGameResult_DESYNC,
	SCGameResult_NONE,

	SCGameResultMax  // Total number of game result codes.
} SCGameResult;


//////////////////////////////////////////////////////////////
// SCGameStatus
// Summary
//		The Game Status Indicates how the session ended and is declared when ending a report.
// Remarks
//		For <strong>SCGameStatus</strong> reporting, the game should do the following. 
//		As long as the game finished properly, and no one disconnected during the course of play, 
//		then all players in the match should submit <strong>SCGameStatus</strong>_COMPLETE reports. 
//		If any members disconnected during play, but the game was finished completely, then all players 
//		in the match should submit <strong>SCGameStatus</strong>_PARTIAL reports indicating that disconnects occured. 
//		For any players who do not complete the match, a <strong>SCGameStatus</strong>_BROKEN report should be submitted. 
//		Thus if the game did not completely finish, all players will submit broken reports. 
//		The only case that will trigger an invalid report is if reports for the same game describe status as both 
//		<strong>SCGameStatus</strong>_COMPLETE and <strong>SCGameStatus</strong>_PARTIAL. 
//		Since COMPLETE indicates that all players finished the game w/o a disconnect and PARTIAL indicates that 
//		disconnects occured, at no time should a game report both complete and partial - 
//		this will be seen as an exploit and invalidate the report.<p>
// Members/Constants
//		SCGameStatus_COMPLETE	
//		SCGameStatus_PARTIAL	
//		SCGameStatus_BROKEN	
//		SCGameStatusMax	
typedef enum
{
	SCGameStatus_COMPLETE,	// The game session came to the expected end without interruption (disconnects, desyncs).
	//							This status indicates that game results are available for all players.
	SCGameStatus_PARTIAL,	// Although the game session came to the expected end, one or more players 
	//							unexpectedly quit or were disconnected. Game results should explicitly 
	//							report which players were disconnected to be used during normalization 
	//							for possible penalty metrics.
	SCGameStatus_BROKEN,	// The game session did not reach the expected end point and is incomplete. 
	//							This should be reported when there has been an event detected that makes 
	//							the end result indeterminate.
	SCGameStatusMax			// Total number of game status codes.
} SCGameStatus;

typedef enum SCStatDataType
{
	SCDataType_INT32,
	SCDataType_INT16,
	SCDataType_BYTE,
	SCDataType_STRING,
	SCDataType_FLOAT,
	SCDataType_INT64,
	SCDataTypeMax
} SCDataType;

//////////////////////////////////////////////////////////////
// SCPlatform
// Summary
//		The platforms supported by the SDK.
typedef enum SCPlatform
{
	SCPlatform_Unknown,
	SCPlatform_PC,
	SCPlatform_PS3,
	SCPlatform_PSP,
	SCPlatform_XBox360,
	SCPlatform_DS,
	SCPlatform_Wii,
	SCPlatform_iPhone,
	SCPlatform_Unix,

	SCPlatformMax
} SCPlatform;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Data types
typedef void*   SCInterfacePtr;
typedef void*   SCReportPtr;


//typedef gsi_u32 SCTeamCount;
//typedef gsi_u32 SCTeamIndex;

//typedef gsi_u32 SCPlayerCount;
//typedef gsi_u32 SCPlayerIndex;

//typedef gsi_u16 SCKey;

typedef char    SCHiddenData[64];

//typedef enum 
//{
//	SCReportKeyType_SERVER,
//	SCReportKeyType_TEAM,
//	SCReportKeyType_PLAYER
//} SCReportKeyType;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Callbacks
typedef void (*SCCheckBanListCallback)(const SCInterfacePtr theInterface,
								GHTTPResult          httpResult,
								SCResult             result,
								void *               userData,
								int					 resultProfileId,
								int					 resultPlatformId,
								gsi_bool             resultProfileBannedHost);

//////////////////////////////////////////////////////////////
// SCCreateSessionCallback
// Summary
//		Called when scCreateSession has completed.
// Parameters
//		theInterface	: [in] the pointer to the SC Interface object.  The game usually has copy of this also.
//		theHttpResult	: [in] Http result from creating a session
//		theResult		: [in] SC Result telling the application what happened when creating a session
//		theUserData		: [in] constant pointer to user data
// Remarks
//		Called when a game session is created.
//		The results will determine if the session was sucessfully created.
//		If there were any errors, theResult will be set to the specific error code.
//		Otherwise theResult will be set to SCResult_NO_ERROR.
//		Please see SCResult for error codes.<p>
// See Also
//		scCreateSession, SCResult
typedef void (*SCCreateSessionCallback)(const SCInterfacePtr theInterface,
								GHTTPResult          httpResult,
								SCResult             result,
								void *               userData);

//////////////////////////////////////////////////////////////
// SCSetReportIntentionCallback
// Summary
//		Called when scReportIntention has completed.
// Parameters
//		theInterface	: [ref] the pointer to the SC Interface object.  The game usually has copy of this also.
//		theHttpResult	: [in] Http result from creating a session
//		theResult		: [in] SC Result telling the application what happened when creating a session
//		theUserData		: [ref] constant pointer to user data
// Remarks
//		Called when a host or client reporting its intention is complete.
//		The results will determine if the session was sucessfully created.
//		If there were any errors, theResult will be set to the specific error code.
//		Otherwise theResult will be set to SCResult_NO_ERROR.
//		Please see SCResult for error codes.<p>
// See Also
//		scSetReportIntention, SCResult
typedef void (*SCSetReportIntentionCallback)(const SCInterfacePtr theInterface,
									         GHTTPResult          httpResult,
                                             SCResult             result,
											 void *	              userData);

//////////////////////////////////////////////////////////////
// SCSubmitReportCallback
// Summary
//		Called when scSubmitReport completes.
// Parameters
//		theInterface	: [in] the pointer to the SC Interface object.  The game usually has copy of this also.
//		theHttpResult	: [in] Http result from creating a session
//		theResult		: [in] SC Result telling the application what happened when creating a session
//		theUserData		: [in] constant pointer to user data
// Remarks
//		After the SDK submits the report, the backend will send back results that will be available in this callback.
//		If there were any errors, theResult will be set to the specific error code.
//		Otherwise theResult will be set to SCResult_NO_ERROR.
//		Please see SCResult for error codes.<p>
// See Also
//		scSubmitReport, SCResult
typedef void (*SCSubmitReportCallback)(const SCInterfacePtr theInterface,
									   GHTTPResult          httpResult,
									   SCResult             result,
									   void *               userData);

/*
typedef void (*SCKeyCallback) (SCReportPtr theReport,
							   SCReportKeyType theKeyType,
							   gsi_u32 theIndex, // how many times this has been called
							   void* theUserParam);
*/



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Main interface functions
//////////////////////////////////////////////////////////////
// scInitialize
// Summary
//		Initializes the competition SDK.
// Parameters
//		theGameId	: [in] The Game ID issued to identify a game.
//		theInterfaceOut	: [out] The pointer to the SC Interface Object instance
// Returns
//		Enum value used to indicate the specific result of the request. This will return SCResult_NO_ERROR
//		if the request completed successfully.
// Remarks
//		The function must be called in order to get a valid SC Interface object.
//		Most other interface functions depend on this interface function when being called. 
//		Note that if the standard GameSpy Availability Check was not performed prior to this call, 
//		the SDK will return SCResult_NO_AVAILABILITY_CHECK.<p>
SCResult SC_CALL scInitialize(int gameId,
							  SCInterfacePtr * theInterfaceOut);

//////////////////////////////////////////////////////////////
// scShutdown
// Summary
//		Shuts down the Competition SDK.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		In order to clean up all resources used by the SDK, this interface function must be called.
//		Do not call this function if you plan to continue reporting stats.<p>
// See Also
//		scInitialize
SCResult SC_CALL scShutdown  (SCInterfacePtr theInterface);

//////////////////////////////////////////////////////////////
// scThink
// Summary
//		Called to complete pending operations for functions with callbacks.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		This function should be called with a valid interface object.
//		It will take care of pending requests that have been made by the interface functions.<p>
// See Also
//		scInitialize, scCreateSession, scSetReportIntention, scSubmitReport
SCResult SC_CALL scThink     (SCInterfacePtr theInterface);


SCResult SC_CALL scCheckBanList(SCInterfacePtr              theInterface,
                                const GSLoginCertificate *	certificate,
								const GSLoginPrivateData *	privateData,
								gsi_u32                     hostProfileId,
								SCPlatform                  hostPlatform,
								SCCheckBanListCallback		callback,
								gsi_time					timeoutMs,
								void *						userData);

//////////////////////////////////////////////////////////////
// scCreateSession
// Summary
//		Requests the Competition service to create a session ID and keep track of the session that is about to start.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
//		theCertificate	: [in] Certificate obtained from the auth service.
//		thePrivateData	: [in] Private Data obtained from the auth service.
//		theCallback		: [in] The callback called when create session completes.
//		theTimeoutMs	: [in] Timeout in case the create session operation takes too long
//		theUserData		: [in] User data for use in callbacks.  Note that it is a constant pointer in the callback
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The certificate and private data may be NULL if the local client is an unauthenticated dedicated server.
//		The function should be called by the host after initializing the SDK, 
//		and obtaining a certificate and private data from the authentication service.
//		The competition service creates and sends a session ID to the host.
//		The callback passed in will get called even if the request failed.<p>
// See Also
//		SCCreateSessionCallback, scInitialize
SCResult SC_CALL scCreateSession(SCInterfacePtr             theInterface,
                                 const GSLoginCertificate * certificate,
								 const GSLoginPrivateData * privateData,
								 SCCreateSessionCallback    callback,
								 gsi_time                   timeoutMs,
								 void *                     userData);

//////////////////////////////////////////////////////////////
// scCreateMatchlessSession
// Summary
//		This is a variation of scCreateSession that creates a "matchless" session; 
//		"matchless" means incoming data will be scrutinized less, and applied to stats immediately 
//		instead of when the match is over.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
//		theCertificate	: [in] Certificate obtained from the auth service.
//		thePrivateData	: [in] Private Data obtained from the auth service.
//		theCallback		: [in] The callback called when create session completes.
//		theTimeoutMs	: [in] Timeout in case the create session operation takes too long
//		theUserData		: [in] User data for use in callbacks.  Note that it is a constant pointer in the callback
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		Reports sent for matchless sessions should be marked as such using "scReportSetAsMatchless".<p>
// See Also
//		scReportSetAsMatchless, SCCreateSessionCallback, scInitialize
SCResult SC_CALL scCreateMatchlessSession(SCInterfacePtr    theInterface,
								 const GSLoginCertificate * certificate,
								 const GSLoginPrivateData * privateData,
								 SCCreateSessionCallback    callback,
								 gsi_time                   timeoutMs,
								 void *                     userData);

//////////////////////////////////////////////////////////////
// scSetReportIntention
// Summary
//		Called to tell the ATLAS the type of report that the player or 
//		host will send.
// Parameters
//		theInterface	: [ref] A valid SC Interface Object.
//		theConnectionId	: [in]  The player's connection id. Set to NULL unless the player is
//								rejoining a match he previously left.
//		isAuthoritative	: [in]  flag set if the snapshot being reported will be an authoritative.
//		certificate		: [ref] Certificate obtained from the authentication web service.
//		privateData		: [ref] Private data obtained from the authentication web service.
//		callback		: [ref] The callback called when set report intention completes.
//		timeoutMs		: [in]  The amount of time to spend on the operation before a timeout occurs.
//		userData		: [ref] Application data that may be used in the callback.
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The should be called by both the host and client before sending a report.
//		<p />
//		The host should have created a session before calling this. It allows the 
//		server to know ahead of time what type of report will be sent. Reports 
//		submitted without an intention will be discarded.
//		<p />
//		Note: The theConnectionId argument should be set to NULL unless the player
//		is rejoining a match he previously left.
// See Also
//		scCreateSession, SCSetReportIntentionCallback, scSubmitReport
SCResult SC_CALL scSetReportIntention(const SCInterfacePtr         theInterface,
									  const gsi_u8                 theConnectionId[SC_CONNECTION_GUID_SIZE],
									  gsi_bool                     isAuthoritative,
  							          const GSLoginCertificate *   certificate,
							          const GSLoginPrivateData *   privateData,
							          SCSetReportIntentionCallback callback,
							          gsi_time                     timeoutMs,
									  void *					   userData);

//////////////////////////////////////////////////////////////
// scSubmitReport
// Summary
//		Initiates the submission of a report.
// Parameters
//		theInterface	: [in] A valid SC Interface Object.
//		theReport		: [in] A valid SC Report object
//		isAuthoritative	: [in] Flag to tell if the snapshot is authoritative
//		theCertificate	: [in] Certificate Obtained from the auth service.
//		thePrivateData	: [in] Private Data Obtained from the auth service.
//		theCallback		: [in] Callback to be called when submit report completes.
//		theTimeoutMs	: [in] The amount of time before a timeout occurs
//		theUserData		: [in] Application data that may be used in the callback
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		Once the report has been completed with a call to scReportEnd, the player or host 
//		can call this function to submit a report.
//		The certificate and private data are both required to submit a report.
//		Incomplete reports will be discarded.
//		The callback passed in will tell the game the result of the operation.<p>
// See Also
//		scInitialize, scCreateSession, scSetReportIntention, scReportEnd, SCSubmitReportCallback
SCResult SC_CALL scSubmitReport      (const SCInterfacePtr       theInterface,
									  const SCReportPtr          theReport,
									  gsi_bool                   isAuthoritative,
									  const GSLoginCertificate * certificate,
									  const GSLoginPrivateData * privateData,
                                      SCSubmitReportCallback     callback,
                                      gsi_time                   timeoutMs,
									  void *                     userData);

//////////////////////////////////////////////////////////////
// scSetSessionId
// Summary
//		Used to set the session ID for the current game session.
// Parameters
//		theInterface	: [in] A valid SC Interface Object
//		theSessionId	: [in] The session ID - this has a constant length of SC_SESSION_GUID_SIZE
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The session ID identifies a single game session happening between players. 
//		Players should use the scGetSessionId function in order to obtain the session ID. 
//		This should not be called if a session has not yet been created.<p>
// See Also
//		scGetSessionId, scCreateSession
SCResult SC_CALL scSetSessionId(const SCInterfacePtr theInterface, const gsi_u8 theSessionId[SC_SESSION_GUID_SIZE]);

//////////////////////////////////////////////////////////////
// scGetSessionId
// Summary
//		Used to obtain the session ID for the current game session.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
// Returns
//		pointer to session id string
// Remarks
//		The session ID indentifies a single game session happening between players.
//		After the host creates a session, this function can be called to obtain the session ID.
//		The host can then send the session ID to all other players participating in the game session.<p>
// See Also
//		scSetSessionId, scCreateSession
const char * scGetSessionId   (const SCInterfacePtr theInterface);

//////////////////////////////////////////////////////////////
// scGetConnectionId
// Summary
//		Used to obtain a Connection ID when setting player data in the report.
// Parameters
//		theInterface	: [in] A valid SC Inteface Object
// Returns
//		pointer to conneciton id string
// Remarks
//		The connection id identifies a single player in a game session.
//		It may be possible to have different connection ids during the same session since players 
//		can come and leave sessions.<p>
const char * scGetConnectionId(const SCInterfacePtr theInterface);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Report generation functions

//////////////////////////////////////////////////////////////
// scCreateReport
// Summary
//		Creates a new report for the game session.
// Parameters
//		theInterface		: [in] A valid SC Inteface Object
//		theHeaderVersion	: [in] Header version of the report
//		thePlayerCount		: [in] Player count for allocating enough resources and verification purposes
//		theTeamCount		: [in] Team count for allocating enough resources and verification purposes
//		theReportOut		: [ref] The pointer to created SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		There should have been a call to CreateSession and SetReportIntention before calling this function. 
//		This function should be called after a game session has ended.
//		The player count and team count are more accurate at that point for dedicated server games.
//		This function should also be called before calling any scReport* function.
//		The header version can be obtained from the adminstration site where the the keys are created.
//		See the overview on obtaining access or send a request devsupport@gamespy.com.<p>
// See Also
//		scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, scReportBeginNewPlayer, 
//		scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scCreateReport(const SCInterfacePtr theInterface, 
								gsi_u32              theHeaderVersion, 
								gsi_u32              thePlayerCount,
								gsi_u32              theTeamCount,
								SCReportPtr *  theReportOut);

//    - Write global data key/values
//////////////////////////////////////////////////////////////
// scReportBeginGlobalData
// Summary
//		Tells the competition SDK to start writing global data to the report.
// Parameters
//		theReportData	: [ref] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		After creating a report, this function should be called prior to writing global game data.
//		Global data comes before player and team data.
//		Note that keys and values can be recorded via the key/value utility functions.<p>
// See Also
//		scCreateReport, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportBeginGlobalData(SCReportPtr theReportData);

//////////////////////////////////////////////////////////////
// scReportBeginPlayerData
// Summary
//		Tells the competition SDK to start writing player data to the report.
// Parameters
//		theReportData	: [in] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		Use this function to mark the starting of player data.
//		Player data should come after global data, and before team data.
//		The game can start adding each player and its specific data after this is called.<p>
// See Also
//		scCreateReport, scReportBeginNewPlayer, scReportSetPlayerData, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportBeginPlayerData(SCReportPtr theReportData);

//////////////////////////////////////////////////////////////
// scReportBeginTeamData
// Summary
//		Tells the competition SDK to start writing player data to the report.
// Parameters
//		theReportData	: [in] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		Use this function to mark the starting of team data.
//		Team data should come after global data, an.
//		player data.
//		The game can start adding each team and its specific data after this is called.<p>
// See Also
//		scCreateReport, scReportBeginNewTeam, scReportSetTeamData, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportBeginTeamData  (SCReportPtr theReportData);


//    - Write player auth info and key/values
//////////////////////////////////////////////////////////////
// scReportBeginNewPlayer
// Summary
//		Add a new player to the report.
// Parameters
//		theReportData	: [in] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		This funciton is used to before adding new player data in the report.
//		It tells the SDK that a new player needs to be added to the report.<p>
// See Also
//		scCreateReport, scReportBeginPlayerData, scReportSetPlayerData, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportBeginNewPlayer(SCReportPtr  theReportData);

//////////////////////////////////////////////////////////////
// scReportSetPlayerData
// Summary
//		Sets initial player data in the report specified.
// Parameters
//		theReport				: [ref] A valid SC Report Object
//		thePlayerIndex			: [in] Index of the player (0 - Number of players)
//		thePlayerConnectionId	: [in] Connection ID that the player received from the competition backend
//		thePlayerTeamIndex		: [in] Team index of the player, if that player is on a team.
//		theResult				: [in] Standard SC Game result
//		theProfileId			: [in] Profile ID of the player
//		theCertificate			: [in] Certificate obtained from the auth service. 
//										Note: This parameter is unused currently.
//		theAuthData				: [in] Authentication data
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		A report must have been created prior to using this function.
//		Each player must have a valid login certificate from the authentication service also.
//		This function should be called after a new player is added to the report.
//		Any key/value pairs that need to be added should be done after calling this function.<p>
// See Also
//		scCreateReport, scReportBeginPlayerData, scReportBeginNewPlayer, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportSetPlayerData (SCReportPtr  theReport,
									    gsi_u32            thePlayerIndex,
									    const gsi_u8       thePlayerConnectionId[SC_CONNECTION_GUID_SIZE],
									    gsi_u32            thePlayerTeamId,
									    SCGameResult       result,
									    gsi_u32            theProfileId,
									    const GSLoginCertificate * certificate,
									    const gsi_u8       theAuthData[16]);

//     - Write team info and key/values
//////////////////////////////////////////////////////////////
// scReportBeginNewTeam
// Summary
//		Adds a new team to the report.
// Parameters
//		theReportData	: [in] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		After the beginning of any team data is set, this function can be called to start a new team.
//		After this function has been called, the game can start adding team data to the report.<p>
// See Also
//		scCreateReport, scReportBeginTeamData, scReportSetPlayerData, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportBeginNewTeam(SCReportPtr theReportData);

//////////////////////////////////////////////////////////////
// scReportSetTeamData
// Summary
//		Sets the initial team data in the report specified.
// Parameters
//		theReport		: [in] A valid SC Report Object
//		theTeamIndex	: [in] The index of the team being reported
//		theResult		: [in] The team's result (e.g. win, loss, draw)
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		A report must have been created prior to using this function.
//		This function should be called after a new team is added to the report.
//		Any key/value pairs that need to be added should be done after calling this function.<p>
// See Also
//		scCreateReport, scReportBeginTeamData, scReportBeginNewTeam, scReportAddIntValue, scReportAddStringValue
SCResult SC_CALL scReportSetTeamData (SCReportPtr theReport,
									  gsi_u32           theTeamId,
									  SCGameResult      result);

//     - Call this when you're finished writing the report
//////////////////////////////////////////////////////////////
// scReportEnd
// Summary
//		Denotes the end of a report for the report specified.
// Parameters
//		theReport	: [in] A valid SC Report Object
//		isAuth		: [in] Authoritative report
//		theStatus	: [in] Final Status of the reported game
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		Used to set the end of a report. The report must have been properly created and have some data.
//		Any report being submitted requires that function be called before the submission.
//		Incomplete reports will be discarded.<p>
// See Also
//		scCreateReport, scSubmitReport, SCGameStatus
SCResult SC_CALL scReportEnd(SCReportPtr theReport, 
							 gsi_bool    isAuth, 
							 SCGameStatus theStatus);

//////////////////////////////////////////////////////////////
// scReportSetAsMatchless
// Summary
//		Called after creating the report to set it as a matchless report - 
//		this is needed if the report is being submitted to a "matchless" game session.
// Parameters
//		theReport	: [ref] A valid SC Report Object
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		This should not be used for a non-matchless session report.<p>
// See Also
//		scCreateMatchlessSession, scCreateReport
SCResult SC_CALL scReportSetAsMatchless(SCReportPtr theReport);


// Utility to record key value pairs
//////////////////////////////////////////////////////////////
// scReportAddIntValue
// Summary
//		Adds an integer value to the report for a specific key.
// Parameters
//		theReportData	: [in] A valid SC Report object
//		theKeyId		: [in] Key Identifier for reporting data
//		theValue		: [in] 32 bit Integer value representation of the data
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The host or player can call this function to add either global, player-, or team-specific data.
//		A report needs to be created before calling this function.
//		For global keys, this function can only be called after starting global data. 
//		For player or teams, a new player or team needs to be added.<p>
// See Also
//		scCreateReport, scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, 
//		scReportBeginNewPlayer, scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData
SCResult SC_CALL scReportAddIntValue(SCReportPtr theReportData,
									 gsi_u16           theKeyId,
									 gsi_i32           theValue);


SCResult SC_CALL scReportAddInt64Value(SCReportPtr theReportData,
									   gsi_u16           theKeyId,
									   gsi_i64           theValue);

//////////////////////////////////////////////////////////////
// scReportAddShortValue
// Summary
//		Adds a short value to the report for a specific key.
// Parameters
//		theReportData	: [in] A valid SC Report object
//		theKeyId		: [in] Key Identifier for reporting data
//		theValue		: [in] 16 bit Short value representation of the data
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The host or player can call this function to add either global, player-, or team-specific data.
//		A report needs to be created before calling this function.
//		For global keys, this function can only be called after starting global data. 
//		For player or teams, a new player or team needs to be added.<p>
// See Also
//		scCreateReport, scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, 
//		scReportBeginNewPlayer, scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData
SCResult SC_CALL scReportAddShortValue(SCReportPtr theReportData,
									 gsi_u16           theKeyId,
									 gsi_i16           theValue);

//////////////////////////////////////////////////////////////
// scReportAddByteValue
// Summary
//		Adds a byte value to the report for a specific key.
// Parameters
//		theReportData	: [in] A valid SC Report object
//		theKeyId		: [in] Key Identifier for reporting data
//		theValue		: [in] 8 bit Byte value representation of the data
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The host or player can call this function to add either global, player-, or team-specific data.
//		A report needs to be created before calling this function.
//		For global keys, this function can only be called after starting global data. 
//		For player or teams, a new player or team needs to be added.<p>
// See Also
//		scCreateReport, scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, 
//		scReportBeginNewPlayer, scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData
SCResult SC_CALL scReportAddByteValue(SCReportPtr theReportData,
									 gsi_u16           theKeyId,
									 gsi_i8            theValue);

//////////////////////////////////////////////////////////////
// scReportAddFloatValue
// Summary
//		Adds a float value to the report for a specific key.
// Parameters
//		theReportData	: [in] A valid SC Report object
//		theKeyId		: [in] Key Identifier for reporting data
//		theValue		: [in] 32 bit Float value representation of the data
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The host or player can call this function to add either global, player-, or team-specific data.
//		A report needs to be created before calling this function.
//		For global keys, this function can only be called after starting global data. 
//		For player or teams, a new player or team needs to be added.<p>
// See Also
//		scCreateReport, scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, 
//		scReportBeginNewPlayer, scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData
SCResult SC_CALL scReportAddFloatValue(SCReportPtr theReportData,
									 gsi_u16           theKeyId,
									 float             theValue);

//////////////////////////////////////////////////////////////
// scReportAddStringValue
// Summary
//		Adds a string value to the report for a specific key.
// Parameters
//		theReportData	: [in] A valid SC Report object
//		theKeyId		: [in] The string key's identifier
//		theValue		: [in] The string value
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		The host or player can call this function to add either global, player-, or team-specific data.
//		A report needs to be created before calling this function.
//		For global keys, this function can only be called after starting global data. 
//		For player or teams, a new player or team needs to be added.<p>
// See Also
//		scCreateReport, scReportBeginGlobalData, scReportBeginPlayerData, scReportBeginTeamData, 
//		scReportBeginNewPlayer, scReportSetPlayerData, scReportBeginNewTeam, scReportSetTeamData
SCResult SC_CALL scReportAddStringValue(SCReportPtr theReportData,
										gsi_u16           theKeyId,
										const gsi_char *  theValue);

//////////////////////////////////////////////////////////////
// scDestroyReport
// Summary
//		Used to clean up and free the report object after it has been submitted.
// Parameters
//		theReport	: [in] The pointer to a created SC Report Object.
// Returns
//		Enum value used to indicate the specific result of the request. 
//		This will return SCResult_NO_ERROR if the request completed successfully.
// Remarks
//		This should be called regardless of whether or not the report was submitted successfully. 
//		It should only be used if the report object contains a valid pointer from a successful call to scCreateReport.<p>
// See Also
//		scCreateReport
SCResult SC_CALL scDestroyReport(SCReportPtr theReport);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Peer to peer encryption utilities (Will probably be moved to common code)

//     A symmetric cipher key for peer-to-peer communication
//     Will usually have one key for sending and a second key for receiving
typedef struct SCPeerCipher
{
	RC4Context mRC4;
	gsi_u8  mKey[GS_CRYPT_RSA_BYTE_SIZE];
	gsi_u32 mKeyLen;
	gsi_bool mInitialized;
} SCPeerCipher;

typedef char SCPeerKeyExchangeMsg[GS_CRYPT_RSA_BYTE_SIZE];

SCResult SC_CALL scPeerCipherInit(const GSLoginCertificate * theLocalCert, SCPeerCipher * theCipher);

SCResult SC_CALL scPeerCipherCreateKeyExchangeMsg(const GSLoginCertificate * theRemoteCert,
												  const SCPeerCipher *       theCipher, 
												  SCPeerKeyExchangeMsg       theMsgOut);

SCResult SC_CALL scPeerCipherParseKeyExchangeMsg (const GSLoginCertificate * theLocalCert,  
												  const GSLoginPrivateData * theCertPrivateData, 
												  const SCPeerKeyExchangeMsg theMsg, 
												  SCPeerCipher *             theCipherOut);

// Encrypt/Decrypt in place, also the RC4 context is modified every time encryption/decryption take place
SCResult SC_CALL scPeerCipherEncryptBuffer(SCPeerCipher * theCipher, gsi_u8 * theData, gsi_u32 theLen);
SCResult SC_CALL scPeerCipherDecryptBuffer(SCPeerCipher * theCipher, gsi_u8 * theData, gsi_u32 theLen);

// When using UDP (non-ordered) you must supply a message num
//    - This is less efficient then encrypting an ordered stream
SCResult SC_CALL scPeerCipherEncryptBufferIV(SCPeerCipher * theCipher, gsi_u32 theMessageNum, gsi_u8 * theData, gsi_u32 theLen);
SCResult SC_CALL scPeerCipherDecryptBufferIV(SCPeerCipher * theCipher, gsi_u32 theMessageNum, gsi_u8 * theData, gsi_u32 theLen);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SC_H__
