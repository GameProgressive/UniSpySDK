///////////////////////////////////////////////////////////////////////////////
// File:	gamespyBase.cs
// SDK:		GameSpy Base C# Wrapper
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Gamespy
{
    /// <summary>
    /// Declare the Enumerated Types used in the SDK
    /// </summary>

    // GS Common declarations
    public enum GSCoreState
    {
        GSCoreState_IN_USE,
        GSCoreState_SHUTDOWN_PENDING,
        GSCoreState_SHUTDOWN_COMPLETE
    };

    
    // Nat Negoation SDK declarations
    
    // NatNegotiateState - Possible states for the SDK. The two you will be notified for are
    public enum NatNegotiateState
    {	
	    ns_preinitsent, 
	    ns_preinitack,		// When the NAT Negotiation server acknowledges your connection
	    ns_initsent,		// Initial connection request has been sent to the server (internal).
	    ns_initack,			// NAT Negotiation server has acknowledged your connection request.
	    ns_connectping,		// Direct negotiation with the other client has started.
	    ns_finished,		// The negotiation process has completed (internal).
	    ns_canceled,		// The negotiation process has been canceled (internal).
	    ns_reportsent,		// Negotiation result report has been sent to the server (internal).
	    ns_reportack		// NAT Negotiation server has acknowledged your result report (internal).
    } ;

    // NatNegotiateResult
    public enum NatNegotiateResult
    {
	    nr_success,			// Successful negotiation, other parameters can be used to continue 
	    //							communications with the client.
	    nr_deadbeatpartner,	// Partner did not register with the NAT Negotiation Server.
	    nr_inittimeout,		// Unable to communicate with NAT Negotiation Server.
	    nr_pingtimeout,		// Unable to communicate with partner.
	    nr_unknownerror,	// NAT Negotiation server indicated an unknown error condition.
	    nr_noresult			// Initial negotiation status before a result is determined.
    } ;


    // NatNegotiateError - Possible error values that can be returned when starting a negotiation.
    public enum NatNegotiateError
    {
	    ne_noerror,			// No error.
	    ne_allocerror,		// Memory allocation failed.
	    ne_socketerror,		// Socket allocation failed.
	    ne_dnserror			// DNS lookup failed.
    } ;
    
    
    // qr2_error_t
    //		Constants returned from qr2_init and the error callback to signal an
    //		 error condition.
    public enum QueryReportResult
    {
        qr_noerror,			// No error occurred
        qr_wsockerror,			// A standard socket call failed, e.g. exhausted resources
        qr_binderror,			// The SDK was unable to find an available port to bind on
        qr_dnserror,			// A DNS lookup (for the master server) failed
        qr_connerror,			// The server is behind a NAT and does not support negotiation
        qr_nochallengeerror,	// No challenge was received from the master. The common reasons for this error are: <br>
        //					1. Not enabling the NatNegotiate flag in the peerSetTitle or qr2_init calls - this should be  
        //					   PEERTrue (or 1 for QR2) for games that support NATs. Otherwise the master server will assume  
        //					   this server is not behind a NAT and can directly connect to it <br> 
        //					2. Calling qr2_buffer_add more than once on a particular key value <br>
        //					3. Firewall or NAT configuration is blocking incoming traffic. You may need to open ports to allow
        //					   communication (see related) <br> 
        //					4. Using the same port for socket communications - shared socket implementations with qr2/peer 
        //					   together <br> 
        //					5. The heartbeat packet has exceeded the max buffer size of 1400 bytes. Try abbreviating some of 
        //					   the custom keys to fit within the 1400 byte buffer. The reason for this restriction is to 
        //					   support as many routers as possible, as UDP packets beyond this data range are more inclined to
        //					   be dropped. <br> 
        //					6. "numplayers" or "maxplayers" being set to negative values <br> 
        //					7. Having 2 network adapters connected to an internal and external network, and the internal one 
        //					   is set as primary <br>

        qr2_error_count
    };

    public enum qr2DefaultKeys 
    {
        HOSTNAME_KEY = 1,
        GAMENAME_KEY,
        GAMEVER_KEY,			
        HOSTPORT_KEY,		
        MAPNAME_KEY,			
        GAMETYPE_KEY,		
        GAMEVARIANT_KEY,		
        NUMPLAYERS_KEY,		
        NUMTEAMS_KEY,		
        MAXPLAYERS_KEY,		
        GAMEMODE_KEY,		
        TEAMPLAY_KEY,		
        FRAGLIMIT_KEY,		
        TEAMFRAGLIMIT_KEY,	
        TIMEELAPSED_KEY,		
        TIMELIMIT_KEY,		
        ROUNDTIME_KEY,		
        ROUNDELAPSED_KEY,	
        PASSWORD_KEY,		
        GROUPID_KEY,			
        PLAYER__KEY,			
        SCORE__KEY,			
        SKILL__KEY,			
        PING__KEY,			
        TEAM__KEY,			
        DEATHS__KEY,			
        PID__KEY,			
        TEAM_T_KEY,			
        SCORE_T_KEY,			
        NN_GROUP_ID_KEY,		

        // Query-From-Master-Only keys
        // - these two values are retrieved only from the master server so we need to make
        //   sure not to overwrite them when querying servers directly
        COUNTRY_KEY, 			
        REGION_KEY,
        NUM_RESERVED_KEYS =50       // Custom keys should be greater than this value 	
    };
    
    // qr2_key_type
    // Summary
    //		Keytype indicates the type of keys being referenced -- server,
    //		 player, or team.
    public enum QueryReportKeyType
    {
	    key_server,		// General information about the game in progress.
	    key_player,		// Information about a specific player.
	    key_team,		// Information about a specific team.
	    key_type_count
    };
    
    // Server Browser SDK Declarations

    //  ServerBrowserError - Error codes that can be returned from Server Browsing functions.
    public enum ServerBrowserError
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
    };	

    //  ServerBrowserState - States the ServerBrowser object can be in.
    public enum ServerBrowserState
    {
        sb_disconnected,	// Idle and not connected to the master server.
        sb_listxfer,		// Downloading list of servers from the master server.
        sb_querying,		// Querying servers.
        sb_connected		// Idle, but still connected to the master server.
    };

    //  ServerBrowserCallbackReason - Callbacks that can occur during server browsing operations.
    public enum ServerBrowserCallbackReason
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
    };	

    //  ServerBrowserConnectRequestResult -  Results passed to the SBConnectToServerCallback.
    public enum ServerBrowserConnectRequestResult
    {
        sbcs_succeeded,		// Connected to server successfully.
        sbcs_failed			// Failed to connect to server.
    };

    // Atlas SDK Declarations
 
	// Result codes
	public enum SCResult
	{
		SCResult_NO_ERROR = 0,			// No error has occurred.
		SCResult_NO_AVAILABILITY_CHECK,	// The standard GameSpy Availability Check was not performed 
		// prior to initialization.
		SCResult_INVALID_PARAMETERS,	// Parameters passed to interface function were invalid.
		SCResult_NOT_INITIALIZED,		// The SDK was not initialized.
		SCResult_CORE_NOT_INITIALIZED,	// The core was initialized by the application.
		SCResult_OUT_OF_MEMORY,			// The SDK could not allocate memory for its resources.
		SCResult_CALLBACK_PENDING,		// Result tell the application, that the operation is still pending.

		SCResult_HTTP_ERROR,			// Error if the backend fails to respond with correct HTTP.
		SCResult_UNKNOWN_RESPONSE,		// Error if the SDK cannot understand the result.
		SCResult_RESPONSE_INVALID,		// Error if the SDK cannot read the response from the backend.
		SCResult_INVALID_DATATYPE,		// Error if an invalid datatype is received.

		SCResult_REPORT_INCOMPLETE,		// The report was incomplete.
		SCResult_REPORT_INVALID,		// Part or all of report is invalid.
		SCResult_SUBMISSION_FAILED,		// Submission of report failed.

		SCResult_QUERY_DISABLED,		// Error occurs if the query id is disabled on the backend administration site
		SCResult_QUERY_TYPE_MISMATCH,	// Error occurs if the query id passed is used for the wrong query type
		// e.g. query id is passed into a game stats query instead of player stats query
		SCResult_QUERY_INVALID,			// Error occurs if the query id is invalid or not found.  The text message will provide details.
		SCResult_QUERY_PARAMS_MISSING,	// Error occurs if a parameter or parameters for the specified query are missing.

		SCResult_QUERY_PARAMS_TOO_MANY,	// Error occurs if the number of params exceeds the expected number
		SCResult_QUERY_PARAM_TYPE_INVALID,	// Error occurs if a parameter value that is passed does not match what the query expects
		// e.g. if the query expects a number string and a non-numeric string is passed in.

		SCResult_UNKNOWN_ERROR,			// Error unknown to SDK

		// response codes dealing with errors in response headers
		SCResult_INVALID_GAMEID,			// make sure GameID is properly set with wsSetCredentials
		SCResult_INVALID_SESSIONTOKEN,     // make sure wsSetCredentials was called with valid credentials and you have logged in via AuthService
		SCResult_SESSIONTOKEN_EXPIRED,      // re-login via AuthService to refresh your 'session'

		SCResultMax						// Total number of result codes that can be returned.
	} ;

	public enum SCGameResult
	{
		SCGameResult_WIN,			// The game session resulted in a win for the current player or team.
		SCGameResult_LOSS,			// The game session resulted in a loss for the current player or team.
		SCGameResult_DRAW,			// The game session resulted in a draw for the current player or team.
		SCGameResult_DISCONNECT,	// The current player or team disconnected during the game session.
		SCGameResult_DESYNC,		// The current player or team lost sync during the game session.
		SCGameResult_NONE,			// There was no result from the game session for the current player or team.
		SCGameResultMax				// The upper bound of game result codes.
	} ;

	public enum SCGameStatus
	{
		SCGameStatus_COMPLETE,	// The game session came to the expected end without 
		// interruption (disconnects, desyncs).
		// This status indicates that game results are available 
		// for all players.
		SCGameStatus_PARTIAL,	// Although the game session came to the expected end, one 
		// or more players unexpectedly quit or were disconnected. 
		// Game results should explicitly report which players were 
		// disconnected to be used during normalization for possible 
		// penalty metrics.
		SCGameStatus_BROKEN,	// The game session did not reach the expected end point and 
		// is incomplete. This should be reported when there has been 
		// an event detected that makes the end result indeterminate.
		SCGameStatusMax			// The upper bound of game status codes.
	} ;

	//////////////////////////////////////////////////////////////
	// SCDataType
	// Summary
	//		The datatypes supported by ATLAS.
	public enum SCStatDataType
	{
		SCDataType_INT32,
		SCDataType_INT16,
		SCDataType_BYTE,
		SCDataType_STRING,
		SCDataType_FLOAT,
		SCDataType_INT64,
		SCDataTypeMax
	} ;

	//////////////////////////////////////////////////////////////
	// SCPlatform
	// Summary
	//		The platforms supported by the SDK.
	public enum SCPlatform
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
	} ;


    // Common 
    public enum GSIACResult
    {
        GSIACWaiting = 0,             // still waiting for a response from the backend
        GSIACAvailable,               // the game's backend services are available
        GSIACUnavailable,             // the game's backend services are unavailable
        GSIACTemporarilyUnavailable   // the game's backend services are temporarily unavailable
    };

    // Sake SDK Declarations 
 
    public enum SAKEStartupResult
    {
        SAKEStartupResult_SUCCESS = 0,		// Startup succeeded.
        SAKEStartupResult_NOT_AVAILABLE,	// The Sake service is unavailable.
        SAKEStartupResult_CORE_SHUTDOWN,	// Error in the gsCore.
        SAKEStartupResult_OUT_OF_MEMORY		// Not enough memory to initialize Sake.
    };

    public enum SAKEFieldType
    {
	    SAKEFieldType_BYTE,
	    SAKEFieldType_SHORT,
	    SAKEFieldType_INT,
	    SAKEFieldType_FLOAT,
	    SAKEFieldType_ASCII_STRING,
	    SAKEFieldType_UNICODE_STRING,
	    SAKEFieldType_BOOLEAN,
	    SAKEFieldType_DATE_AND_TIME,
	    SAKEFieldType_BINARY_DATA,
	    SAKEFieldType_INT64,
	    SAKEFieldType_NUM_FIELD_TYPES
    } ;

    public enum SAKEContentStorageType
    {
	    SAKEContentStorageType_DISK,   // content is stored as a local file
	    SAKEContentStorageType_MEMORY  // content is stored in memory
    } ;

    public enum SAKEStartRequestResult
    {
	    SAKEStartRequestResult_SUCCESS,
	    SAKEStartRequestResult_NOT_AUTHENTICATED,
	    SAKEStartRequestResult_OUT_OF_MEMORY,
	    SAKEStartRequestResult_BAD_INPUT,
	    SAKEStartRequestResult_BAD_TABLEID,
	    SAKEStartRequestResult_BAD_FIELDS,
	    SAKEStartRequestResult_BAD_NUM_FIELDS,
	    SAKEStartRequestResult_BAD_FIELD_NAME,
	    SAKEStartRequestResult_BAD_FIELD_TYPE,
	    SAKEStartRequestResult_BAD_FIELD_VALUE,
	    SAKEStartRequestResult_BAD_OFFSET,
	    SAKEStartRequestResult_BAD_MAX,
	    SAKEStartRequestResult_BAD_RECORDIDS,
	    SAKEStartRequestResult_BAD_NUM_RECORDIDS,
	    SAKEStartRequestResult_BAD_REASON_CODE,
	    SAKEStartRequestResult_BAD_REASON,
        SAKEStartRequestResult_INVALID_DATA,
        SAKEStartRequestResult_HTTP_ERROR,
	    SAKEStartRequestResult_UNKNOWN_ERROR,
        SAKEStartRequestResult_FILE_NOT_FOUND,
        SAKEStartRequestResult_HTTP_INVALID_POST,
        SAKEStartRequestResult_HTTP_INVALID_BUFFERSIZE,
        SAKEStartRequestResult_HTTP_INVALID_URL
    };

    public enum SAKERequestResult
    {
	    SAKERequestResult_SUCCESS,					// Sake request completed successfully.
	    SAKERequestResult_SECRET_KEY_INVALID,		// The secretKey passed to sakeSetGame is invalid (sakeSetGame does 
	    //												not actually authenticate the game info).
	    SAKERequestResult_SERVICE_DISABLED,			// GameSpy services have been disabled for this title.
	    SAKERequestResult_CONNECTION_TIMEOUT,		// A connection to the Sake service could not be established before 
												    // the timeout was reached.
	    SAKERequestResult_CONNECTION_ERROR,			// An error occurred while connecting to the Sake service.
	    SAKERequestResult_MALFORMED_RESPONSE,		// The SOAP response sent from the Sake service was corrupt.
	    SAKERequestResult_OUT_OF_MEMORY,			// A memory allocation failed.
	    SAKERequestResult_DATABASE_UNAVAILABLE,		// The Sake database is temporarily down.
	    SAKERequestResult_LOGIN_TICKET_INVALID,		// The profile's loginTicket, obtained via gpGetLoginTicket and 
												    // passed to sakeSetProfile, is invalid (sakeSetProfile does not 
												    // actually authenticate the player info).
	    SAKERequestResult_LOGIN_TICKET_EXPIRED,		// The profile's loginTicket, obtained via gpGetLoginTicket and 
												    // passed to sakeSetProfile, has expired. The profile must obtain 
												    // an updated login ticket (via gpGetLoginTicket) and call sakeSetProfile 
												    // once again with the updated ticket.
	    SAKERequestResult_TABLE_NOT_FOUND,			// No Sake table matches the mTableId passed in the Sake query.
	    SAKERequestResult_RECORD_NOT_FOUND,			// No records in the specified table match the mRecordId passed in 
												    // the Sake query.
	    SAKERequestResult_FIELD_NOT_FOUND,			// mFieldNames or mFields contains a field which 
												    // is not in the specified table.
	    SAKERequestResult_FIELD_TYPE_INVALID,		// The mType set in a SAKEField struct is not a valid SAKEFieldType.
	    SAKERequestResult_NO_PERMISSION,			// The profile is not the owner of the record and the record does not 
												    // have public access (permissions are set on the Sake admin site).
	    SAKERequestResult_RECORD_LIMIT_REACHED,		// The profile cannot create any more records in the table specified 
												    // (Limit per owner is set on the Sake admin site).
	    SAKERequestResult_ALREADY_RATED,			// No profile can rate a single record more than once.
	    SAKERequestResult_NOT_RATEABLE,				// The record is in a table that is not rateable (The ‘Rateable' flag
												    // for a table is set on the Sake admin site).
	    SAKERequestResult_NOT_OWNED,				// Only the record owner profile can perform the attempted function 
												    // on the specified record.
	    SAKERequestResult_FILTER_INVALID,			// The mFilter string has invalid SQL grammar.
	    SAKERequestResult_SORT_INVALID,				// The mSort string has invalid SQL grammar.
	    SAKERequestResult_TARGET_FILTER_INVALID,	// Error is unknown used if none of the above.
	    SAKERequestResult_CERTIFICATE_INVALID,      // AuthService certificate/proof cannot be authenticated
	    SAKERequestResult_UNKNOWN_ERROR,
	    SAKERequestResult_REQUEST_CANCELLED,
	    SAKERequestResult_CONTENTSERVER_FAILURE,

	    SAKERequestResult_ALREADY_REPORTED,         // No profile can report a single record more than once (while it's in queue for moderation)
    	
	    // response codes dealing with errors in response headers
	    SAKERequestResult_INVALID_GAMEID,			// make sure GameID is properly set with wsSetGameCredentials
	    SAKERequestResult_INVALID_SESSIONTOKEN,     // make sure wsSetGameCredentials was called with valid credentials and you have logged in via AuthService
	    SAKERequestResult_SESSIONTOKEN_EXPIRED      // re-login via AuthService to refresh your 'session'
    };

    public enum SAKEFileResult
    {
	    SAKEFileResult_SUCCESS           = 0,	// Upload succeeded.
	    SAKEFileResult_BAD_HTTP_METHOD   = 1,	// Incorrect ghttp call used to upload file.
	    SAKEFileResult_BAD_FILE_COUNT    = 2,	// Number of files uploaded is incorrect.
	    SAKEFileResult_MISSING_PARAMETER = 3,	// Missing parameter in the ghttp upload call.
	    SAKEFileResult_FILE_NOT_FOUND    = 4,	// No file was found.
	    SAKEFileResult_FILE_TOO_LARGE    = 5,	// File uploaded larger than the specified size.
	    SAKEFileResult_SERVER_ERROR      = 6,	// Unknown error occurred on the server when processing this request.
	    SAKEFileResult_UNKNOWN_ERROR			// Error is unknown - used if none of the above.
    };

    // Http SDK Declarations 

    public enum GHTTPResult
    {
        GHTTPSuccess,               // 0:  Successfully retrieved file.
        GHTTPOutOfMemory,           // 1:  A memory allocation failed.
        GHTTPBufferOverflow,        // 2:  The user-supplied buffer was too small to hold the file.
        GHTTPParseURLFailed,        // 3:  There was an error parsing the URL.
        GHTTPHostLookupFailed,      // 4:  Failed looking up the hostname.
        GHTTPSocketFailed,          // 5:  Failed to create/initialize/read/write a socket.
        GHTTPConnectFailed,         // 6:  Failed connecting to the http server.
        GHTTPBadResponse,           // 7:  Error understanding a response from the server.
        GHTTPRequestRejected,       // 8:  The request has been rejected by the server.
        GHTTPUnauthorized,          // 9:  Not authorized to get the file.
        GHTTPForbidden,             // 10: The server has refused to send the file.
        GHTTPFileNotFound,          // 11: Failed to find the file on the server.
        GHTTPServerError,           // 12: The server has encountered an internal error.
        GHTTPFileWriteFailed,       // 13: An error occurred writing to the local file 
        //		(for ghttpSaveFile[Ex]).
        GHTTPFileReadFailed,        // 14: There was an error reading from a local file 
        //		(for posting files from disk).
        GHTTPFileIncomplete,        // 15: Download started but was interrupted. Only reported 
        //		if file size is known.
        GHTTPFileToBig,             // 16: The file is to big to be downloaded (size exceeds 
        //		range of internal data types)
        GHTTPEncryptionError,       // 17: Error with encryption engine.
        GHTTPRequestCancelled,      // 18: User requested cancel and/or graceful close.
        GHTTPRecvFileTimeout        // 19: Stuck in the Receive File state
    };

    // Auth Service SDK Declarations 

    public enum WSLoginValue
    {
        // Login response code (mResponseCode)
        //   -- GameSpy Devs: Must match server
        WSLogin_Success = 0,
        WSLogin_ServerInitFailed,

        WSLogin_UserNotFound,
        WSLogin_InvalidPassword,
        WSLogin_InvalidProfile,
        WSLogin_UniqueNickExpired,

        WSLogin_DBError,
        WSLogin_ServerError,
        WSLogin_FailureMax,         // must be the last failure

        // Login result (mLoginResult)
        WSLogin_HttpError = 100,    // ghttp reported an error, response ignored
        WSLogin_ParseError,         // couldn't parse http response
        WSLogin_InvalidCertificate, // login success but certificate was invalid!
        WSLogin_LoginFailed,        // failed login or other error condition
        WSLogin_OutOfMemory,        // could not process due to insufficient memory
        WSLogin_InvalidParameters,  // check the function arguments
        WSLogin_NoAvailabilityCheck,// No availability check was performed
        WSLogin_Cancelled,          // login request was cancelled
        WSLogin_UnknownError,       // error occurred, but detailed information not available

        // response codes dealing with errors in response headers
        WSLogin_InvalidGameID = 200, // make sure GameID is properly set with wsSetGameCredentials
        WSLogin_InvalidAccessKey,       // make sure Access Key is properly set with wsSetGameCredentials

        // login results dealing with errors in response headers
        WSLogin_InvalidGameCredentials // check the parameters passed to wsSetGameCredentials

    };
    
    // Debug log declarations
    
    public enum GSIDebugCategory
    {
	    GSIDebugCat_App,
	    GSIDebugCat_GP,
	    GSIDebugCat_Peer,
	    GSIDebugCat_QR2,
	    GSIDebugCat_SB,
	    GSIDebugCat_Voice,
	    GSIDebugCat_AD,
	    GSIDebugCat_NatNeg,
	    GSIDebugCat_HTTP,
	    GSIDebugCat_CDKey,
	    GSIDebugCat_Direct2Game,
        GSIDebugCat_Brigades,
        GSIDebugCat_AuthService,
        GSIDebugCat_Sake,
	    GSIDebugCat_Atlas,
	    // Add new ones here (update string table in gsiDebug.c!)


	    GSIDebugCat_Common, // Common should be last to prevent display weirdness
	                        // resulting from initialization order
	    GSIDebugCat_Count,
	    GSIDebugCat_All = GSIDebugCat_Count
    } ;
    
    public enum GSIDebugType
    {
	    GSIDebugType_Network,  // Network activity
	    GSIDebugType_File,     // File output
	    GSIDebugType_Memory,   // Memory allocations
	    GSIDebugType_State,    // State update
	    GSIDebugType_Misc,     // None of the above
	    // add new ones here (update string table in gsiDebug.c!)

	    GSIDebugType_Count,
	    GSIDebugType_All = GSIDebugType_Count
    } ; 

    // Output levels (a mask for the levels you want to receive)
    // (update string table in gsiDebug.c!)
    public enum GSIDebugLevel
    {
        None    =   0,  //    No output
        HotError =  1,
        WarmError = 2,
        Warning =   4,
        Normal  =   7,  //    Warnings and above
        Notice  =   8,
        Debug   =   15, //    Notice and above
        Comment =   16,
        Verbose =   31, //    Comment and above
        RawDump =   32,
        StackTrace = 64,
        Hardcore=   127 //    Recv all*/
    };
}
