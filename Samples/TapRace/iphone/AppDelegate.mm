// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "AppDelegate.h"
#import "Utility.h"
#include "atlas_taprace_v2.h"

#import "LoginController.h"
#import "MenuController.h"
#import "MatchmakingController.h"
#import "GameController.h"
#import "GameResultsController.h"
#import "LeaderboardsController.h"
#import "LeadersController.h"
#import "LeadersByLocationController.h"
#import "UserStatsController.h"

#import "qr2/qr2.h"
#import "natneg/natneg.h"
#import "gt2/gt2Encode.h"

#import <algorithm>
#import <mach/mach_time.h>
#import <arpa/inet.h>


// Pre-game messaging
#define MSG_CONNECT						"is" // client-pid, nick
#define MSG_TO_HOST_CERT				"ir" // host-pid, certificate
#define MSG_TO_HOST_CERT_TYPE			1
#define MSG_TO_CLIENT_CERT				"ir" // join-pid, certificate
#define MSG_TO_CLIENT_CERT_TYPE			2
#define MSG_TO_HOST_CERTVERIFIED		""
#define MSG_TO_HOST_CERTVERIFIED_TYPE	3	// ready for keys
#define MSG_TO_CLIENT_CERTVERIFIED		""
#define MSG_TO_CLIENT_CERTVERIFIED_TYPE	4	// ready for keys
#define MSG_TO_HOST_KEYS				"r"  // encryption keys
#define MSG_TO_HOST_KEYS_TYPE			5
#define MSG_TO_CLIENT_KEYS				"r"  // encrpytion keys
#define MSG_TO_CLIENT_KEYS_TYPE			6
#define MSG_REPLAY						""
#define MSG_REPLAY_TYPE					7	// replay game
#define MSG_REPLAY_CANCELED				""
#define MSG_REPLAY_CANCELED_TYPE		8	// ...or not

// Game messaging
#define MSG_COUNTDOWN            "i"  // count
#define MSG_COUNTDOWN_TYPE       20
#define MSG_SESSION_ID           "rr" // Host session ID, connID exchange
#define MSG_SESSION_ID_TYPE      21
#define MSG_CONNECTION_ID        "r"  // connection ID exhcnage
#define MSG_CONNECTION_ID_TYPE   22
#define MSG_START_RACE           ""	  // race start
#define MSG_START_RACE_TYPE      23
#define MSG_PROGRESS             "i"  // progress
#define MSG_PROGRESS_TYPE        24
#define MSG_END_RACE             "i"  // time
#define MSG_END_RACE_TYPE        25
#define MSG_CHAT                 "s"  // message
#define MSG_CHAT_TYPE            26

#undef HOST_PORT	// previously defined in mach headers
#define HOST_PORT                38466
#define HOST_PORT2               38468
#define CLIENT_PORT              38467
#define HOST_PORT_STRING         ":" STRINGIZE(HOST_PORT)
#define CLIENT_PORT_STRING       ":" STRINGIZE(CLIENT_PORT) // so you can run both
#define COUNTDOWN_START          5

// Stats & SDK constants
#define SCRACE_TIMEOUT_MS		 0
#define SCRACE_SLEEP_MS			 100
#define SCRACE_AUTHORITATIVE	 gsi_true
#define SCRACE_COLLABORATIVE	 gsi_false
#define SCRACE_NUM_PLAYERS		 2
#define SCRACE_NUM_TEAMS		 2

#define SCRACE_HOST_TEAM		 7564	// fake team ids
#define SCRACE_CLIENT_TEAM		 7565	// fake team ids

// NAT negotiation
#define NN_SERVER				0
#define NN_CLIENT				1

#define buttonExists( viewButtonIndex ) viewButtonIndex >= 0

enum AppState
{
	LOGGED_OUT,
	SETTING_UP,
	RACING,
	FINISHED_RACE,

	HOST_LISTENING,
	HOST_CONNECTED,
	HOST_ERROR,
	HOST_EXCHANGE_CERT,
	HOST_VERIFY_CERT,
	HOST_EXCHANGE_KEYS,
	HOST_WAITING,
	HOST_SEND_SESSID,

	JOIN_CHOOSESERVER,
	JOIN_CONNECTING,
	JOIN_CONNECTED,
	JOIN_ERROR,
	JOIN_EXCHANGE_CERT,
	JOIN_VERIFY_CERT,
	JOIN_EXCHANGE_KEYS,
	JOIN_WAITING,
	JOIN_SEND_CONNID
};

enum AppAlert
{
	ALERT_NONE,
	ALERT_LOADINGPLAYERDATA,
	ALERT_LISTENING,
	ALERT_NATNEG,
	ALERT_CONNECTING,
	ALERT_REPLAY,
	ALERT_REPLAY_CONFIRM,
	ALERT_WAITING_FOR_LEADERS,
	ALERT_NO_NETWORK_CONNECTION
};


///////////////////////////////////////////////////////////////////////////////
// Query and Reporting Callbacks
static void ServerKeyCallback(int keyid, qr2_buffer_t outbuf, void* userdata);
static void PlayerKeyCallback(int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static void TeamKeyCallback(int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static int CountCallback(qr2_key_type keytype, void* userdata);
static void KeyListCallback(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void* userdata);
static void AddErrorCallback(qr2_error_t error, char* errmsg, void* userdata);
static void NatNegCallback(int cookie, void* userData);

///////////////////////////////////////////////////////////////////////////////
// Competition Callbacks
static void CreateSessionCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData);
static void SetReportIntentionCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData);
static void SubmitReportCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData);

///////////////////////////////////////////////////////////////////////////////
// Sake Callbacks
static void GetMyRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, SAKEGetMyRecordsInput* inputData, SAKEGetMyRecordsOutput* outputData, void* userData);

///////////////////////////////////////////////////////////////////////////////
// GT2 Callbacks
static void ConnectedCallback(GT2Connection connection, GT2Result result, GT2Byte* message, int len);
static void ReceivedCallback(GT2Connection connection, GT2Byte* message, int len, GT2Bool reliable);
static void ClosedCallback(GT2Connection connection, GT2CloseReason reason);
static void ConnectAttemptCallback(GT2Socket listener, GT2Connection connection, unsigned int ip, unsigned short port, int latency, GT2Byte* message, int len);
static void SocketErrorCallback(GT2Socket socket);
static GT2Bool UnrecognizedMessageCallback(GT2Socket socket, unsigned int ip, unsigned short port, GT2Byte* message, int len);
	
///////////////////////////////////////////////////////////////////////////////
// Leaderboard Callbacks
//static void cbGetBuddyList( GPConnection * connection, void * arg, void * param );
static void GetMyLeaderPositionCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData);
static void SearchForLeadersTimeRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData);
static void SearchForTop5TimeRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData);
//static BOOL setTimeRecords(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData);

///////////////////////////////////////////////////////////////////////////////
// NAT Negotiation Callbacks
static void NNDetectionCallback(gsi_bool success, NAT nat);
static void NNProgressCallback(NegotiateState state, void* userdata);
static void NNCompletedCallback(NegotiateResult result, SOCKET gamesocket, sockaddr_in* remoteaddr, void* userDdata);

static const char PLAYER_STATS_TABLE[] = "PlayerStats_v" STRINGIZE(ATLAS_RULE_SET_VERSION);

@implementation LeaderboardObject
@synthesize name;
@synthesize profileId;
@synthesize pictureFileId;
@synthesize rank;
@synthesize careerWins;
@synthesize careerLosses;
@synthesize bestRaceTime;
@synthesize totalMatches;
@synthesize currentWinStreak;
@synthesize playerLocation;
@synthesize thumbNail;

- (CLLocationCoordinate2D)coordinate {
	return playerLocation.coordinate;
}

- (NSString *)title {
	return name;
}

@end

// GLOBAL VARIABLES
//=================
char			raceDateTimeString[256];
PlayerData		gPlayerData;
PlayerData		gOpponentData;
//LeaderboardObject gOpponentData;	// current opponent
int				gLeaderboardType;	// type of most recently downloaded leaderboard data
int				gLeaderboardProfileIndex;	// index of currently displayed profile
NSMutableArray*	gLeaderboardData;	// array of PlayerData for leaderboard
AppDelegate* appDelegate;
bool sdkInitialized;	// set true at end of applicationDidFinishLaunching
bool userKickedOut;		// set to true if same user logs into another device
bool lostConnection;  // true when we lose network connectivity
 
@interface AppDelegate(Private) <UIAlertViewDelegate>

//- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName;
- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName animated: (BOOL)animated;

- (bool)setupHosting;
- (bool)setupJoining: (unsigned int)ip port: (unsigned short)port useNatNeg: (bool)useNatNeg;
- (void)setupMatch: (bool)hostMatch;

- (void)onLoadRemoteDataTimer: (NSTimer*)timer;
- (void)onThinkTimer: (NSTimer*)timer;
- (void)onCountdownTimer: (NSTimer*)timer;
- (void)onHostingCountdownTimer: (NSTimer*)timer;

- (void)saveTwoPlayerGameStatsToFile;

- (void)reportTwoPlayerStats;
- (void)reportSinglePlayerStats;
- (void)countdown;
- (void)startRace;

- (void)addErrorCallback: (qr2_error_t)error errorMessage: (const char*)errmsg;

- (void)createSessionCallback: (const SCInterfacePtr)interface httpResult: (GHTTPResult)httpResult result: (SCResult)result;
- (void)setReportIntentionCallback: (const SCInterfacePtr)interface httpResult: (GHTTPResult)httpResult result: (SCResult)result;
- (void)submitReportCallback: (const SCInterfacePtr)interface httpResult: (GHTTPResult)httpResult result: (SCResult)result;

- (void)getMyRecordsCallback: (SAKERequestResult) result inputData: (SAKEGetMyRecordsInput*)inputData outputData: (SAKEGetMyRecordsOutput*)outputData;

- (void)connectedCallback: (GT2Connection)connection result: (GT2Result)result message: (GT2Byte*)message length: (int)length;
- (void)receivedCallback: (GT2Connection)connection message: (GT2Byte*)message length: (int)length reliable: (GT2Bool)reliable;
- (void)closedCallback: (GT2Connection)connection reason: (GT2CloseReason)reason;
- (void)connectAttemptCallback: (GT2Socket)listener connection: (GT2Connection)connection ip: (unsigned int)ip port: (unsigned short)port latency: (int)latency message: (GT2Byte*)message length: (int)length;

- (void)natnegCallback: (int)cookie;
- (void)natnegCompletedCallback: (NegotiateResult)result socket: (SOCKET)gamesocket remoteaddr: (sockaddr_in*)remoteaddr;

@end


@implementation AppDelegate

@synthesize serverBrowser;
@synthesize sake;

@synthesize connected;
@synthesize state;
@synthesize fromMenu;

@synthesize start;
@synthesize localTime;
@synthesize remoteTime;


// Called when leaving TapRace
// ===========================
- (void)dealloc
{
	[self deleteLeaderboards];
	[self deleteBlockedImagesData];
	[window release];
	[navController release];
	[super dealloc];
}

#ifdef GSI_COMMON_DEBUG

// Called to display SDK debug messages 
// ====================================
static void DebugCallback(GSIDebugCategory theCat, GSIDebugType theType, GSIDebugLevel theLevel, const char * theTokenStr, va_list theParamList)
{
	GSI_UNUSED(theLevel);
	
	printf("[%s][%s] ", gGSIDebugCatStrings[theCat], gGSIDebugTypeStrings[theType]);
	
	vprintf(theTokenStr, theParamList);
}
#endif


// Called when race is finished or terminated
- (void)stopHostingServer
{
	qr2_shutdown(NULL);

/*					if (gt2Connection)
				{
					gt2CloseConnection(gt2Connection);
					gt2Connection = NULL;
				}
				
				GT2Socket tempsocket = gt2Socket;
				gt2Socket = NULL; // prevent recursion
				gt2CloseSocket(tempsocket);
*/
	connected = false;
	hosting = false;
		NSLog(@"stopHostingServer qr2_shutdown");
}


// Called when click login or into login page 
// ==========================================
- (void)initGame
{
	sessionCreated  = false;
	NSLog(@"initGame:   null gt2 socket");
	gt2Socket		= NULL;
	state			= LOGGED_OUT;
	countdown		= 0;
	racing			= false;
	connIdSet		= false;	
	gt2Connection	= NULL;
	gLeaderboardData = nil;
	gLeaderboardType = LEADERBOARD_INVALID;
}


// Called when click login or into login page
// Initialize Sake - used to read stored data from the backend
// ===========================================================
- (void)initSake
{
	GSIACResult aResult = GSIACWaiting; 

	if(sdkInitialized)  return;

	// Do Availability Check - Make sure backend is available
	GSIStartAvailableCheck(SCRACE_GAMENAME);
	while( aResult == GSIACWaiting )
	{
		aResult = GSIAvailableCheckThink();
		msleep(5);
	}
	
	if ( aResult == GSIACUnavailable )
	{
		MessageBox( @"Online service for TapRace is no longer available." );
		return;
		//exit(EXIT_FAILURE);
	}
	
	if ( aResult == GSIACTemporarilyUnavailable )
	{
		MessageBox(@"Online service for TapRace is temporarily down for maintenance.");
		return;
		//exit(EXIT_FAILURE);
	}
	
	// Initialize SDK core object - used for both the AuthService and the Competition SDK
	gsCoreInitialize();
	
	
	// Initialize the Competition SDK - all users submit a snapshot
	SCResult result = scInitialize(SCRACE_GAMEID, &interface);
	
	if (result != SCResult_NO_ERROR)
	{
		NSLog(@"SCResult result = %d", result );
		
		switch (result)
		{
			case SCResult_OUT_OF_MEMORY:
				MessageBox(@"Application failed to initialize.", @"Out of memory");
				break;
				
			case SCResult_NOT_INITIALIZED:
				MessageBox(@"Application failed to initialize.", @"Competition");
				break;

			case SCResult_NO_AVAILABILITY_CHECK:
				MessageBox(@"Application failed to initialize.", @"Availability");
				break;
				
			default:
				MessageBox(@"Application failed to initialize.", @"Unknown Error");
				break;
		}
		return;
		//exit(EXIT_FAILURE);
	}
	
	gPlayerData.statsInterface = interface;
	gPlayerData.thumbNail = nil;
	gPlayerData.fullsizeImage = nil;
	
	sakeStartup(&sake);
	sakeSetGame(sake, SCRACE_GAMENAME, SCRACE_GAMEID, SCRACE_SECRETKEY);
	sdkInitialized = true;
}


// called after a delay to upload a new image from the image picker...
//  gives the mainloop a chance to switch views and render the leaders menu again before the upload starts
// =======================================================================================================
- (void)delayedNetworkConnectionErrorMessage
{
	MessageBox(@"Unable to establish connection. Please connect to a different WiFi network or switch to 3G.", 
			   @"Network Error");
}


//  called after TapRace finishes loading
// =======================================================================================================
- (void)applicationDidFinishLaunching: (UIApplication*)application
{
#ifdef GSI_COMMON_DEBUG
	// gsSetDebugFile(stdout);				// output to console
	// gsSetDebugCallback(DebugCallback);	// or output to a function for logging
	
	// Set output debug levels
	gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All, GSIDebugLevel_Verbose);  // all SDKs
#endif
	
	appDelegate = self;
	
	[window addSubview: navController.view];
	[window makeKeyAndVisible];
}

// Perform a reachability query for the address 0.0.0.0. If that address is reachable without
// requiring a connection, a network interface is available. We'll have to do more work to
// determine which network interface is available.
// =======================================================================================================
- (BOOL)isNetworkAvailableFlags:(SCNetworkReachabilityFlags *)outFlags
{
	SCNetworkReachabilityRef defaultRouteReachability;
	struct sockaddr_in zeroAddress;
	bzero(&zeroAddress, sizeof(zeroAddress));
	zeroAddress.sin_len = sizeof(zeroAddress);
	zeroAddress.sin_family = AF_INET;
	
	defaultRouteReachability = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&zeroAddress);
	
	SCNetworkReachabilityFlags flags;
	BOOL gotFlags = SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags);
	if (!gotFlags) 
		return NO;
	
	BOOL isReachable = flags & kSCNetworkReachabilityFlagsReachable;
	
	// This flag indicates that the specified nodename or address can
	// be reached using the current network configuration, but a
	// connection must first be established.
	//
	// If the flag is false, we don't have a connection. But because CFNetwork
	// automatically attempts to bring up a WWAN connection, if the WWAN reachability
	// flag is present, a connection is not required.
	BOOL noConnectionRequired = !(flags & kSCNetworkReachabilityFlagsConnectionRequired);
	if ((flags & kSCNetworkReachabilityFlagsIsWWAN)) {
		noConnectionRequired = YES;
	}
	
	isReachable = (isReachable && noConnectionRequired) ? YES : NO;
	
	// Callers of this method might want to use the reachability flags, so if an 'out' parameter
	// was passed in, assign the reachability flags to it.
	if (outFlags) {
		*outFlags = flags;
	}
	
	return isReachable;
}


- (void)applicationWillTerminate: (UIApplication*)application
{
	if( hosting )
	{
		qr2_shutdown(NULL);
	}
	
	if (gPlayerData.thumbNail != nil) {
		[gPlayerData.thumbNail release];
		gPlayerData.thumbNail = nil;
	}
	if (gPlayerData.fullsizeImage != nil) {
		[gPlayerData.fullsizeImage release];
		gPlayerData.fullsizeImage = nil;
	}
	
	// Note: gOpponentData does not use fullsizeImage
	if (gOpponentData.thumbNail != nil) {
		[gOpponentData.thumbNail release];
		gOpponentData.thumbNail = nil;
	}

	[appDelegate deleteLeaderboards];
	
	[self logout];
}

- (void)navigationController: (UINavigationController*)navigationController willShowViewController: (UIViewController*)viewController animated: (BOOL)animated
{
	if ([viewController isMemberOfClass: [LoginController class]]) {
		if (state != LOGGED_OUT) 
		{
			state = LOGGED_OUT;
			NSLog(@"navigationController state=LOGGED_OUT");
			
			LoginController* loginController = (LoginController*)viewController;
			assert(loginController != nil);
			[loginController reset];
		}
	}
	else if ([viewController isMemberOfClass: [MenuController class]]) {
		if (state != SETTING_UP) 
		{
			racing = false;
			if (gt2Connection != NULL) 
			{
				gt2CloseConnection(gt2Connection);
				gt2Connection = NULL;
			}
			
			[self stopThinkTimer];
			
			state = SETTING_UP;
			NSLog(@"navigationController state=SETTING_UP");
			
			MenuController* menuController = (MenuController*)viewController;
			assert(menuController != nil);
			[menuController reset];
		}
	}
	else if ([viewController isMemberOfClass: [MatchmakingController class]]) {
		if (state != JOIN_CHOOSESERVER) {
			if (playerCount > 1) {
				if (countdownTimer != nil) 
				{
					[countdownTimer invalidate];
					countdownTimer = nil;
				}	
				if (hostingCountdownTimer != nil) 
				{
					[hostingCountdownTimer invalidate];
					hostingCountdownTimer = nil;
				}
				racing = false;
				if (gt2Connection != NULL) 
				{
					gt2CloseConnection(gt2Connection);
					gt2Connection = NULL;
				}
				
				state = JOIN_CHOOSESERVER;
				NSLog(@"navigationController state=JOIN_CHOOSESERVER");
				
				MatchmakingController* matchmakingController = (MatchmakingController*)viewController;
				assert(matchmakingController != nil);
				[matchmakingController reset];
			}
		}
	}
	else if ([viewController isMemberOfClass: [GameController class]]) {
		if (state != RACING) {
			state = RACING;
			NSLog(@"navigationController state=RACING");
			[gameController reset];
		}
	}
}

- (void)startThinkTimer
{
	if (thinkTimer != nil) 
		return; // already running
	
	thinkTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onThinkTimer:) userInfo: nil repeats: YES];

	[UIApplication sharedApplication].idleTimerDisabled = NO;
}

- (void)stopThinkTimer
{
	[thinkTimer invalidate];
	thinkTimer = nil;

	[UIApplication sharedApplication].idleTimerDisabled = NO;
}


/**
 * Once logged in, load the local file stats and initiate loading of server stats
 */
- (void)loggedIn
{
	NSString* dataPath;
	
	// Load locally saved player stats data.
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

	if ([paths count] == 0) {
		// Something's very wrong; this should never happen.
		MessageBox(@"Unable to access private data path in the file system.", @"Error");
		exit(EXIT_FAILURE);
	}

	NSString* documentsPath = (NSString*)[paths objectAtIndex: 0];
	gPlayerData.playerDataPath = [[documentsPath stringByAppendingPathComponent: [NSString stringWithFormat: @"%d", gPlayerData.profileId]] retain];

	// Check if diretory exists
	BOOL isDir = NO;
	NSFileManager* fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath: gPlayerData.playerDataPath isDirectory: &isDir] || !isDir) 
	{
		if (!isDir) 
		{
			// Don't know how this got here, but we don't want it:
			[fileManager removeItemAtPath: gPlayerData.playerDataPath error: NULL];
		}
	}
	
	//*** Get the player's block image list from file
	dataPath = [gPlayerData.playerDataPath stringByAppendingPathComponent: blockedImagesDataFile];
	gPlayerData.blockedImagesData = [[NSMutableArray alloc] initWithContentsOfFile: dataPath];	// will be nil if the file doesn't exist
	if( gPlayerData.blockedImagesData == nil )
		gPlayerData.blockedImagesData = [[NSMutableArray alloc] init];

	//*** Get the player stats from file
	dataPath = [gPlayerData.playerDataPath stringByAppendingPathComponent: playerStatsDataFile];		
	gPlayerData.playerStatsData = [[NSMutableDictionary alloc] initWithContentsOfFile: dataPath];	// will be nil if the file doesn't exist

	// Load single & multi-player data from server
	static char* fields[] =
	{
		ATLAS_GET_STAT_NAME(CAREER_WINS),
		ATLAS_GET_STAT_NAME(CAREER_LOSSES),
		ATLAS_GET_STAT_NAME(BEST_RACE_TIME),
		ATLAS_GET_STAT_NAME(WORST_RACE_TIME),
		ATLAS_GET_STAT_NAME(TOTAL_MATCHES),
		ATLAS_GET_STAT_NAME(AVERAGE_RACE_TIME),
		ATLAS_GET_STAT_NAME(CURRENT_WIN_STREAK),
		ATLAS_GET_STAT_NAME(CURRENT_LOSS_STREAK),
		ATLAS_GET_STAT_NAME(TOTAL_RACE_TIME),
		ATLAS_GET_STAT_NAME(CAREER_DISCONNECTS),
		ATLAS_GET_STAT_NAME(DISCONNECT_RATE),
		ATLAS_GET_STAT_NAME(CAREER_DRAWS),
		ATLAS_GET_STAT_NAME(CURRENT_DRAW_STREAK),
		ATLAS_GET_STAT_NAME(CAREER_LONGEST_WIN_STREAK),
		ATLAS_GET_STAT_NAME(CAREER_LONGEST_LOSS_STREAK),
		ATLAS_GET_STAT_NAME(CAREER_LONGEST_DRAW_STREAK),
		ATLAS_GET_STAT_NAME(TOTAL_COMPLETE_MATCHES),
		
		ATLAS_GET_STAT_NAME(SP_BEST_RACE_TIME),
		ATLAS_GET_STAT_NAME(SP_WORST_RACE_TIME),
		ATLAS_GET_STAT_NAME(SP_TOTAL_PLAYS),
		ATLAS_GET_STAT_NAME(SP_AVERAGE_RACE_TIME),
		ATLAS_GET_STAT_NAME(SP_TOTAL_RACE_TIME),
		ATLAS_GET_STAT_NAME(LAST_TIME_PLAYED)
	};
	static SAKEGetMyRecordsInput input = { (char*)PLAYER_STATS_TABLE, fields, sizeof(fields) / sizeof(fields[0]) };

	alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Loading player data..." delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
	[alertView show];
	
	currentAlert = ALERT_LOADINGPLAYERDATA;
	sakeSetProfile(sake, gPlayerData.profileId, gPlayerData.loginTicket);
	sakeGetMyRecords(sake, &input, (SAKERequestCallback)GetMyRecordsCallback, self);
	state = SETTING_UP;
	NSLog(@"loggedIn state=SETTING_UP");
	
	// display Menu page
	[self pushNewControllerOfType: [MenuController class] nib: @"menu"];
	loadRemoteDataTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onLoadRemoteDataTimer:) userInfo: nil repeats: YES];
}


// Called when user receives message from server that 
// - the login has been is disconnected on the server (due to same user login on other device)
// - the connection closed 
- (void)kickedOut
{
	userKickedOut = true;
}

- (bool)isUserKickedOut
{
	return userKickedOut;
}


// Called when logging user out
- (void)logout
{
	if (gt2Connection)
	{
		gt2CloseConnection(gt2Connection);
		gt2Connection = NULL;
	}
	if (gt2Socket != NULL) 
	{
		GT2Socket tempsocket = gt2Socket;
		gt2Socket = NULL; // prevent recursion
		NSLog(@"logout:   null & close gt2 socket");
		gt2CloseSocket(tempsocket);
	}

	NSLog(@"logout: qr2 shutdown");
	qr2_shutdown(NULL);
	
	sakeShutdown(sake);
	scShutdown(interface);
	gsCoreShutdown();
	
	[gPlayerData.playerDataPath release];
	gPlayerData.playerDataPath = nil;

	LoginController* loginController = (LoginController*)[self findExistingControllerOfType: [LoginController class]];
	
	[loginController reset];

	[navController popToViewController: loginController animated: YES];
}

- (bool)joinGame: (unsigned int)ip port: (unsigned short)port useNatNeg: (bool)useNatNeg;
{
	if (![self setupJoining: ip port: port useNatNeg: useNatNeg])
	{
		//MessageBox(@"Error setting up the match");
		MessageBox(@"Connection Error", @"Error setting up the match. Login when network connection is up");
// was commented *****************		
		if (gt2Connection)
		{
			gt2CloseConnection(gt2Connection);
			gt2Connection = NULL;
		}
		if (gt2Socket != NULL) 
		{
			GT2Socket tempsocket = gt2Socket;
			gt2Socket = NULL; // prevent recursion
			NSLog(@"joinGame:   null & close gt2 socket");
			gt2CloseSocket(tempsocket);
		}
// up to here *******************		
		//[self logout];  // from old code
		return false;
	}
	
	return true;
}

- (bool)hostGame
{
	if (![self setupHosting])
	{
		// If timed out waiting for player
		if (hostingCountdownTimer == nil || hostingCountdown != 0 ) 
		{
			MessageBox(@"Error setting up the match.");
		}
		
		return false;
	}
	return true;
}

- (void)stopHostingCountdownTimer
{
	NSLog(@"stopHostingCountdownTimer");
	
	if (hostingCountdownTimer != nil) 
	{
		[hostingCountdownTimer invalidate];
		hostingCountdownTimer = nil;
	}
	
}

- (void)tellHostToQuitGame
{
	// 2 player match - Let opponent know we finished.
	/////////////////////////////
	char buffer[32];
	
	int rcode = gtEncode(MSG_END_RACE_TYPE, MSG_END_RACE, buffer, sizeof(buffer), (uint32_t)(localTime * absolute_to_millis));
	assert(rcode != -1);
	
	NSLog(@"Protocol send MSG_END_RACE_TYPE\n");
	gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
}

- (void)findGame
{
	state = JOIN_CHOOSESERVER;
	NSLog(@"findGame state=JOIN_CHOOSESERVER");
	playerCount = 2;
	
	[self pushNewControllerOfType: [MatchmakingController class] nib: @"matchmaking"];
	
	[self startThinkTimer];
}

- (void)startSinglePlayerGame
{
	fromMenu = true;
	state = RACING;
	NSLog(@"startSinglePlayerGame state=RACING");
	reportSent = gsi_false;
	playerCount = 1;
	
	gameController = (GameController*)[self pushNewControllerOfType: [GameController class] nib: @"singleplayergame"];
}

- (void)startMultiPlayerGame: (const char*)remoteAddress
{
	const int buflen = 2 + 4 + 2 + GP_UNIQUENICK_LEN;	// 2 bytes for each message type, 4 bytes for int, GP_UNIQUENICK_LEN for max uniquenick length
	char buf[buflen];
	gtEncodeNoType(MSG_CONNECT, buf, buflen, gPlayerData.profileId, gPlayerData.uniquenick);
	
	GT2ConnectionCallbacks callbacks = {
		ConnectedCallback,
		ReceivedCallback,
		ClosedCallback,
		NULL
	};
	
	NSLog(@"startMultiPlayerGame:  connect gt2 socket");
	GT2Result result = gt2Connect(gt2Socket, &gt2Connection, remoteAddress, (GT2Byte*)buf, buflen, 0, &callbacks, GT2False);
	NSLog(@"startMultiPlayerGame remoteAddress=%d", remoteAddress);
	NSLog(@"startMultiPlayerGame gt2Connect:gt2Socket=%d result=%d\n", (int)gt2Socket, (int)result);
	if (result != GT2Success) 
	{
		MessageBox(@"Unable to connect to remote player.", @"Connection Error");
		return;
	}

	gt2SetConnectionData(gt2Connection, self);
	state = JOIN_CONNECTING;
	NSLog(@"startMultiPlayerGame state=JOIN_CONNECTING");
	
	// show that we are connecting
	alertView = [[UIAlertView alloc] initWithTitle: @"Connecting to remote player" message: @"30 seconds" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
	[alertView show];
	currentAlert = ALERT_CONNECTING;
	
	[appDelegate startJoiningCountdownTimer];
}

- (bool)isTwoPlayerGame
{
	return (playerCount == 2); 
}

- (void)startCountdown
{
	if (playerCount == 1) 
	{
		reportSent = false;
		scCreateMatchlessSession(interface, &gPlayerData.certificate, &gPlayerData.privateData, CreateSessionCallback, SCRACE_TIMEOUT_MS, self);

		// Single-player game, no countdown.
		[self startRace];
		return;
	}

	// release joining game countdown
	[self stopJoiningCountdownTimer];

	if (hosting) 
	{
		// The countdown here could be thought of as a loading screen.
		// During this loading phase - the Host will create the game session and notify
		// the other players of the session ID. All players will set their report intentions.
		scCreateSession(interface, &gPlayerData.certificate, &gPlayerData.privateData, CreateSessionCallback, SCRACE_TIMEOUT_MS, self);
		state = HOST_SEND_SESSID;
		NSLog(@"startCountdown state=HOST_SEND_SESSID");
		
		// Start the countdown.
		countdown = COUNTDOWN_START;
		countdownTimer = [NSTimer scheduledTimerWithTimeInterval: 1.0 target: self selector: @selector(onCountdownTimer:) userInfo: nil repeats: YES];
		[self countdown];
	}
}

- (void)startHostingCountdownTimer
{
	// release hosting countdown
	[self stopHostingCountdownTimer];  //if any
		
	// Start the countdown.
	NSLog(@"startHostingCountdownTimer starts hostingCountdownTimer");
	hostingCountdown = 30;
	hostingCountdownTimer = [NSTimer scheduledTimerWithTimeInterval: 1.0 target: self selector: @selector(onHostingCountdownTimer:) userInfo: self repeats: YES];

}

- (void)stopJoiningCountdownTimer
{
	NSLog(@"stopJoiningCountdownTimer");
	if (joiningCountdownTimer != nil) 
	{
		[joiningCountdownTimer invalidate];
		joiningCountdownTimer = nil;
	}

}

- (void)startJoiningCountdownTimer
{
	[self stopJoiningCountdownTimer];	// if running

	NSLog(@"startJoiningCountdownTimer");
	
	// Start the countdown.
	joiningCountdown = 30;
	joiningCountdownTimer = [NSTimer scheduledTimerWithTimeInterval: 1.0 target: self selector: @selector(onJoiningCountdownTimer:) userInfo: nil repeats: YES];

}


- (void)restartGame
{
	if (playerCount > 1) 
	{
		char buffer[32];
		int rcode = gtEncode(MSG_REPLAY_TYPE, MSG_REPLAY, buffer, sizeof(buffer));
		
		assert(rcode != -1);
		NSLog(@"Protocol send MSG_REPLAY_TYPE\n");
		gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
		
		alertView = [[UIAlertView alloc] initWithTitle: @"Waiting for reply" message: @"30 seconds" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
		[alertView show];
		currentAlert = ALERT_REPLAY;
		
		[self startHostingCountdownTimer];
	}
	else 
	{
		assert(gameController != nil);
		
		[navController popToViewController: gameController animated: YES];
	}
}

- (void)finishedGame
{
	fromMenu = false;
	
	BOOL newBestTime = NO;
	localTime = mach_absolute_time() - start;


	// set last game date/time
	NSString *raceTimeString = [[NSDate date] description];
	strcpy( raceDateTimeString, [raceTimeString cStringUsingEncoding: NSASCIIStringEncoding] );
	gPlayerData.lastGameTime = raceDateTimeString;

	if (playerCount == 1) 
	{
		int oldBestTime;
		GameResultsController* resultsController = (GameResultsController*)[self pushNewControllerOfType: [GameResultsController class] nib: @"singleplayerresults"];
		state = FINISHED_RACE;
		NSLog(@"finishedGame state=FINISHED_RACE");
		
		// Update single player data.
		int localTimeMillis = localTime * absolute_to_millis;
		float avgTapsPerSec = 60.0 / (localTimeMillis / 1000.0);
		
		if (gPlayerData.playerStatsData != nil) 
		{
			// data from file
			int bestTimeMillis = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerBestTimeKey] intValue];
			int worstTimeMillis = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerWorstTimeKey] intValue];
			int averageTimeMillis = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerAverageTimeKey] intValue];
			int gamesPlayed = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerGamesPlayedKey] intValue];

			averageTimeMillis = ((averageTimeMillis * gamesPlayed) + localTimeMillis) / (gamesPlayed + 1);
			gamesPlayed++;

			
			if (bestTimeMillis > localTimeMillis) 
			{
				newBestTime = YES;
				oldBestTime = bestTimeMillis;
				
				[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: localTimeMillis] forKey: singlePlayerBestTimeKey];
			} 
			else 
			{
				[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: bestTimeMillis] forKey: singlePlayerBestTimeKey];
			}

			if (worstTimeMillis < localTimeMillis)
				worstTimeMillis = localTimeMillis;

			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: worstTimeMillis] forKey: singlePlayerWorstTimeKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: averageTimeMillis] forKey: singlePlayerAverageTimeKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gamesPlayed] forKey: singlePlayerGamesPlayedKey];
 		}
		else 
		{
			gPlayerData.playerStatsData = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
											[NSNumber numberWithInt: localTimeMillis], singlePlayerBestTimeKey,
											[NSNumber numberWithInt: localTimeMillis], singlePlayerWorstTimeKey,
											[NSNumber numberWithInt: localTimeMillis], singlePlayerAverageTimeKey,
											[NSNumber numberWithInt: 1], singlePlayerGamesPlayedKey,
											nil];
		}

		[gPlayerData.playerStatsData setObject: raceTimeString forKey: lastTimePlayedKey];

		[gPlayerData.playerStatsData writeToFile: [gPlayerData.playerDataPath stringByAppendingPathComponent: playerStatsDataFile] atomically: YES];

		// Report the stats.
		////////////////////
		if (!reportSent)
		{
			disconnected = gsi_false;
			[self reportSinglePlayerStats];
		}		
		
		[resultsController setGameTime: localTimeMillis];
		[resultsController setGamesPlayed: [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerGamesPlayedKey] intValue]];
		[resultsController setBestTime: [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerBestTimeKey] intValue]];
		[resultsController setAverageTime: [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerAverageTimeKey] intValue]];
		[resultsController setAverageTaps: avgTapsPerSec ];
		if (newBestTime) 
		{
			[resultsController setNewBestTime: oldBestTime];
		}
	}
	else 
	{
		// 2 player match - Let opponent know we finished.
		/////////////////////////////
		char buffer[32];
		
		int rcode = gtEncode(MSG_END_RACE_TYPE, MSG_END_RACE, buffer, sizeof(buffer), (uint32_t)(localTime * absolute_to_millis));
		assert(rcode != -1);
		
		NSLog(@"Protocol send MSG_END_RACE_TYPE\n");
		GT2Result result = gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
		if( result == GT2NetworkError )
		{
			MessageBox(@"Lost connection to remote player. Game canceled",	@"Connection Error");
			[appDelegate returnToMatchMaking];
		}
	}
 
}


// save single player and two player game stats for logged in player
- (void)savePlayerStatsToFile
{	
	if (gPlayerData.playerStatsData == nil) 
	{
		// Copy stats from server to new file
		gPlayerData.playerStatsData = [[NSMutableDictionary alloc] initWithObjectsAndKeys:											
					// single player game stats
					[NSNumber numberWithInt: gPlayerData.spBestRaceTime], singlePlayerBestTimeKey,
					[NSNumber numberWithInt: gPlayerData.spWorstRaceTime], singlePlayerWorstTimeKey,
					[NSNumber numberWithInt: gPlayerData.spTotalPlays], singlePlayerTotalPlaysKey,
					[NSNumber numberWithInt: gPlayerData.spTotalRaceTime], singlePlayerTotalRaceTimeKey,
					[NSNumber numberWithInt:(int) (gPlayerData.spAverageRaceTime *1000)], singlePlayerAverageTimeKey,
						
					// two player game stats
					[NSNumber numberWithInt: gPlayerData.bestRaceTime], matchBestTimeKey,
					[NSNumber numberWithInt: gPlayerData.worstRaceTime], matchWorstTimeKey,
					[NSNumber numberWithInt: (int)(gPlayerData.averageRaceTime * 1000)], matchAverageTimeKey,
					[NSNumber numberWithInt: gPlayerData.careerWins], matchWonKey,
					[NSNumber numberWithInt: gPlayerData.careerLosses], matchLossKey,
					[NSNumber numberWithInt: gPlayerData.matchWinStreak], matchWinStreakKey,
					[NSNumber numberWithInt: gPlayerData.totalMatches], matchGamesPlayedKey,
					[NSNumber numberWithInt: gPlayerData.matchLossStreak], matchLossStreakKey,
					[NSNumber numberWithInt: gPlayerData.totalRaceTime], matchTotalRaceTimeKey,
					[NSNumber numberWithInt: gPlayerData.careerDisconnects], matchCareerDisconnectsKey,
					[NSNumber numberWithInt: gPlayerData.careerDraws], matchDrawsKey,
					[NSNumber numberWithInt: gPlayerData.matchDrawStreak], matchDrawStreakKey,
					[NSNumber numberWithInt: gPlayerData.careerLongestWinStreak], matchCareerLongestWinStreakKey,
					[NSNumber numberWithInt: gPlayerData.careerLongestLossStreak], matchCareerLongestLossStreakKey,
					[NSNumber numberWithInt: gPlayerData.careerLongestDrawStreak], matchCareerLongestDrawStreakKey,
					[NSNumber numberWithInt: gPlayerData.totalCompleteMatches], matchTotalCompleteMatchesKey,
					[NSNumber numberWithFloat: gPlayerData.disconnectRate], matchDisconnectRateKey,
					[NSString stringWithCString: gPlayerData.lastGameTime encoding: NSASCIIStringEncoding], lastTimePlayedKey,
					nil];
	}
	else
	{
		// single player game stats
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.spBestRaceTime] forKey: singlePlayerBestTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.spWorstRaceTime] forKey: singlePlayerWorstTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.spTotalPlays] forKey: singlePlayerTotalPlaysKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.spTotalRaceTime] forKey: singlePlayerTotalRaceTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: gPlayerData.spAverageRaceTime] forKey: singlePlayerAverageTimeKey];
		
		// two player game stats
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.bestRaceTime] forKey: matchBestTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.worstRaceTime] forKey: matchWorstTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: gPlayerData.averageRaceTime] forKey: matchAverageTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerWins] forKey: matchWonKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLosses] forKey: matchLossKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchWinStreak] forKey: matchWinStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.totalMatches] forKey: matchGamesPlayedKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchLossStreak] forKey: matchLossStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.totalRaceTime] forKey: matchTotalRaceTimeKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerDisconnects] forKey: matchCareerDisconnectsKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerDraws] forKey: matchDrawsKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchDrawStreak] forKey: matchDrawStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestWinStreak] forKey: matchCareerLongestWinStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestLossStreak] forKey: matchCareerLongestLossStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestDrawStreak] forKey: matchCareerLongestDrawStreakKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.totalCompleteMatches] forKey: matchTotalCompleteMatchesKey];
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: gPlayerData.disconnectRate] forKey: matchDisconnectRateKey];
		[gPlayerData.playerStatsData setObject: [NSString stringWithCString: gPlayerData.lastGameTime encoding: NSASCIIStringEncoding] forKey: lastTimePlayedKey];

	}
	
	[gPlayerData.playerStatsData writeToFile: [gPlayerData.playerDataPath stringByAppendingPathComponent: playerStatsDataFile] atomically: YES];
}


- (void)returnToMenu
{
	MenuController* controller = (MenuController*)[self findExistingControllerOfType: [MenuController class]];
	assert(controller != nil);
	[navController popToViewController: controller animated: YES];
}

 
- (void)returnToMatchMaking
{
	MatchmakingController* controller = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
	assert(controller != nil);
	[navController popToViewController: controller animated: YES];
}



 - (void)showAboutInfo: (id) sender 
 {    
	FlipsideViewController *controller = [[FlipsideViewController alloc] initWithNibName:@"FlipsideView" bundle:nil];
	controller.delegate = sender;
 
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[sender presentModalViewController:controller animated:YES];
 
	[controller release];
 }
 
  - (void)flipsideViewControllerDidFinish:(id) sender 
 {
 	[sender dismissModalViewControllerAnimated:YES];
 }
 
  
- (void)showLeaderboards
{
	[self deleteLeaderboards];	// delete leaderboards unconditionally, as data may have changed since last time
	[self pushNewControllerOfType: [LeaderboardsController class] nib: @"leaderboards"];
}


	
- (void)downloadTop5Leaders
{	
	static SAKESearchForRecordsInput input;
	static SAKERequest request;
	static char *fieldNames[] = { 
		(char *) "ownerid" , 
		(char *) "row", 
		ATLAS_GET_STAT_NAME( NICK ), 
		ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), 
		ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), 
		ATLAS_GET_STAT_NAME( CAREER_WINS ), 
		ATLAS_GET_STAT_NAME( CAREER_LOSSES ), 
		ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ),
		ATLAS_GET_STAT_NAME( LATITUDE ),
		ATLAS_GET_STAT_NAME( LONGITUDE)
	};
	SAKEStartRequestResult startRequestResult;
	
	// empty leader board
	[self deleteLeaderboards];
	gLeaderboardType = LEADERBOARD_BY_TIME;	
	
	// get Top 5 leaders
	input.mTableId = (char *) PLAYER_STATS_TABLE;
	input.mFieldNames = fieldNames;
	input.mNumFields = (sizeof(fieldNames) / sizeof(fieldNames[0]));
	input.mOffset = 0;
	input.mMaxRecords = 5;
	input.mCacheFlag = gsi_true; // caches the TOP 5 query
	input.mFilter = (char *) "BEST_RACE_TIME > 0 and BEST_RACE_TIME != 99999";
	input.mSort = (char *) "BEST_RACE_TIME asc";
	
	request = sakeSearchForRecords(sake, &input, SearchForTop5TimeRecordsCallback, self);
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(sake);
		NSLog(@"Failed to start request: %d\n", startRequestResult);
	}
	
	currentAlert = ALERT_WAITING_FOR_LEADERS;
}

- (void)getMyLeaderPosition
{	
	static char aFilter[128];
	static SAKEGetRecordCountInput input;
	static SAKERequest request;
	
	input.mTableId = (char *) PLAYER_STATS_TABLE;
	input.mCacheFlag = gsi_true; // caches the TOP 10 query
	sprintf( aFilter, "BEST_RACE_TIME < %d", gPlayerData.bestRaceTime );
	input.mFilter = (char *) aFilter;
	
	request = sakeGetRecordCount(sake, &input, GetMyLeaderPositionCallback, self);
	if(!request)
	{
		gPlayerData.rank = -1;
		
		SAKEStartRequestResult startRequestResult;
		startRequestResult = sakeGetStartRequestResult(sake);
		NSLog( @"Failed to start request: %d\n", startRequestResult );
	}
}

	
- (void)downloadLeaders
{	
	static SAKESearchForRecordsInput input;
	static SAKERequest request;
	static char *fieldNames[] = { 
		(char *) "ownerid" , 
		(char *) "row", 
		ATLAS_GET_STAT_NAME( NICK ), 
		ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), 
		ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), 
		ATLAS_GET_STAT_NAME( CAREER_WINS ), 
		ATLAS_GET_STAT_NAME( CAREER_LOSSES ), 
		ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ),
		ATLAS_GET_STAT_NAME( LATITUDE ),
		ATLAS_GET_STAT_NAME( LONGITUDE)
	};
	SAKEStartRequestResult startRequestResult;
	input.mTableId = (char *) PLAYER_STATS_TABLE;
	input.mFieldNames = fieldNames;
	input.mNumFields = (sizeof(fieldNames) / sizeof(fieldNames[0]));
	input.mOffset = 0;
	input.mMaxRecords = MAX_LEADERS;
	input.mCacheFlag = gsi_true; // caches the TOP 10 query
	
	switch (gLeaderboardType) {
		case LEADERBOARD_BY_TIME:
			input.mFilter = (char *) "BEST_RACE_TIME > 0 and BEST_RACE_TIME != 99999";
			input.mSort = (char *) "BEST_RACE_TIME asc";
	
			request = sakeSearchForRecords(sake, &input, SearchForLeadersTimeRecordsCallback, self);
			if(!request)
			{
				startRequestResult = sakeGetStartRequestResult(sake);
				NSLog(@"Failed to start request: %d\n", startRequestResult);
			}
			break;
	}
	alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Waiting for leaderboards..." delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
	[alertView show];
	currentAlert = ALERT_WAITING_FOR_LEADERS;
}

- (BOOL)setTimeRecords: (SAKE)sake request: (SAKERequest)request result: (SAKERequestResult)result inputData: (void*)inputData outputData: (void*)outputData
{
	float lat,lng;
	
	SAKESearchForRecordsOutput * records = (SAKESearchForRecordsOutput*)outputData;
		
	if (result == SAKERequestResult_SUCCESS) 
	{
		if (gLeaderboardData != nil) 
		{ 
			int lbtype = gLeaderboardType;
			[self deleteLeaderboards];
			gLeaderboardType = lbtype;
		}
		
		if (records->mNumRecords > 0) 
		{
			gLeaderboardData = [[NSMutableArray arrayWithCapacity: records->mNumRecords] retain];
			for (int i=0; i<records->mNumRecords; i++) 
			{
				LeaderboardObject * pData = [[LeaderboardObject alloc] init];
				
				pData.pictureFileId = 0;	// init picture file id
				
				SAKEField * field = sakeGetFieldByName( "ownerid", records->mRecords[i], 10 );
				if (field != NULL) 
					pData.profileId = field->mValue.mInt;
				else 
					pData.profileId = 0;
				
				field = sakeGetFieldByName( "row", records->mRecords[i], 10);
				if (field != NULL) 
					pData.rank = field->mValue.mInt;
				else
					pData.rank = -1;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( NICK ), records->mRecords[i], 10 );
				if (field != NULL)
				{
					pData.name = [NSString stringWithFormat: @"%s", (char*)( field->mValue.mAsciiString ) ];
				} 
				else 
					pData.name = nil;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.bestRaceTime = (gsi_u32)( field->mValue.mInt );
				else 
					pData.bestRaceTime = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.totalMatches = (gsi_u32)( field->mValue.mInt );
				else
					pData.totalMatches = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_WINS ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.careerWins = (gsi_u32)( field->mValue.mInt );
				else
					pData.careerWins = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_LOSSES ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.careerLosses = (gsi_u32)( field->mValue.mInt );
				else 
					pData.careerLosses = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.currentWinStreak = (gsi_u32)( field->mValue.mInt );
				else
					pData.currentWinStreak = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LATITUDE ), records->mRecords[i], 10 );
				if (field != NULL) 
					lat = field->mValue.mFloat;
				else 
					lat = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LONGITUDE ), records->mRecords[i], 10 );
				if (field != NULL)
					lng = field->mValue.mFloat;
				else 
					lng = 0;
				
				pData.playerLocation = [[CLLocation alloc] initWithLatitude: lat longitude: lng];
				[gLeaderboardData insertObject: pData atIndex: i];
				NSLog(@"leader[%d] = %d\n",i, pData.profileId);
			}
		}
		
		return YES;
	} 
	else 
	{
		// error
		return NO;
	}
}

- (void)searchForTop5TimeRecordsCallback: (SAKE)sake request: (SAKERequest)request result: (SAKERequestResult)result inputData: (void*)inputData outputData: (void*)outputData
{	
	float lat,lng;
	
	SAKESearchForRecordsOutput * records = (SAKESearchForRecordsOutput*)outputData;
		
	if (result == SAKERequestResult_SUCCESS) {
		if (gLeaderboardData != nil) 
		{ 
			int lbtype = gLeaderboardType;
			[self deleteLeaderboards];
			gLeaderboardType = lbtype;
		}
		
		if (records->mNumRecords <= 0) 
		{
			MenuController* menuController = (MenuController*)[self findExistingControllerOfType: [MenuController class]];
			[ menuController noTop5Leaders];
		}
		else
		{
			gLeaderboardData = [[NSMutableArray arrayWithCapacity: records->mNumRecords] retain];
			for (int i=0; i<records->mNumRecords; i++) 
			{
				LeaderboardObject * pData = [[LeaderboardObject alloc] init];
				
				SAKEField * field = sakeGetFieldByName( "ownerid", records->mRecords[i], 10 );
				if (field != NULL) 
					pData.profileId = field->mValue.mInt;
				else 
					pData.profileId = 0;
				
				field = sakeGetFieldByName( "row", records->mRecords[i], 10);
				if (field != NULL) 
					pData.rank = field->mValue.mInt;
				else
					pData.rank = -1;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( NICK ), records->mRecords[i], 10 );
				if (field != NULL)
				{
					pData.name = [NSString stringWithFormat: @"%s", (char*)( field->mValue.mAsciiString ) ];
				} 
				else 
					pData.name = nil;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.bestRaceTime = (gsi_u32)( field->mValue.mInt );
				else 
					pData.bestRaceTime = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.totalMatches = (gsi_u32)( field->mValue.mInt );
				else
					pData.totalMatches = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_WINS ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.careerWins = (gsi_u32)( field->mValue.mInt );
				else
					pData.careerWins = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_LOSSES ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.careerLosses = (gsi_u32)( field->mValue.mInt );
				else 
					pData.careerLosses = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), records->mRecords[i], 10 );
				if (field != NULL) 
					pData.currentWinStreak = (gsi_u32)( field->mValue.mInt );
				else
					pData.currentWinStreak = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LATITUDE ), records->mRecords[i], 10 );
				if (field != NULL) 
					lat = field->mValue.mFloat;
				else 
					lat = 0;
				
				field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LONGITUDE ), records->mRecords[i], 10 );
				if (field != NULL)
					lng = field->mValue.mFloat;
				else 
					lng = 0;
				
				pData.playerLocation = [[CLLocation alloc] initWithLatitude: lat longitude: lng];
				[gLeaderboardData insertObject: pData atIndex: i];
			}
		}
	} 
	else 
	{
		// error
	}
	// get data
	//[self setTimeRecords: sake request: request result: result inputData: inputData outputData: outputData ];
}


- (void) getMyLeaderPositionCallback: (SAKE) aSake request: (SAKERequest)request result: (SAKERequestResult)result inputData: (void*)inputData outputData: (void*)outputData
{
	SAKEGetRecordCountOutput * records = (SAKEGetRecordCountOutput*)outputData;
	
	// need to pass in pointer to rank variable (int *)rankVar
	if( result != SAKERequestResult_SUCCESS ) 
		gPlayerData.rank = 0;
	else
		gPlayerData.rank = records->mCount + 1;   // 1 + number of records before my best time
}

- (void)searchForLeadersTimeRecordsCallback: (SAKE) aSake request: (SAKERequest)request result: (SAKERequestResult)result inputData: (void*)inputData outputData: (void*)outputData
{
	// Got data - hide popup
	if (alertView != nil) 
	{
		//[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
		[alertView dismissWithClickedButtonIndex: alertView.firstOtherButtonIndex  animated: YES];
		[alertView release];
		alertView = nil;
	}
	currentAlert = ALERT_NONE;
	
	if( [self setTimeRecords: aSake request: request result: result inputData: inputData outputData: outputData ] )
	{	
		// display leaderboard page
		[self pushNewControllerOfType: [LeadersController class] nib: @"leadersbytime"];
	} 
	else 
	{
		// error
	}
}


- (void)showTop5LeadersByTime
{
	[self downloadTop5Leaders];
}

- (void)showLeadersByTime
{
	[self deleteLeaderboards];
	gLeaderboardType = LEADERBOARD_BY_TIME;
	[self downloadLeaders];
}


- (void)showLeadersByBuddies
{
	GPBuddyStatus status;
	int numBuddies;
	
	[self deleteLeaderboards];
	gLeaderboardType = LEADERBOARD_BY_BUDDIES;
	
	GPResult res;
	LoginController* loginController = (LoginController*)[self findExistingControllerOfType: [LoginController class]];

	res = gpGetNumBuddies( [loginController getGPConnectionPtr], &numBuddies );
	if ( (res == GP_NO_ERROR) && (numBuddies == 0)) {
		// just go to other pager where a no-buddies message will pop up
		[self pushNewControllerOfType: [LeadersController class] nib: @"leadersbytime"];

	} else {
		if (res == GP_NO_ERROR) {
		// create an array of buddies...since we only want the 1st MAX_BUDDIES, use a static array
		static GPProfile buddyProfileIds[MAX_BUDDIES];
		for (int i=0; (i < MAX_BUDDIES) && (i < numBuddies); i++) 
		{
			if (GP_NO_ERROR == gpGetBuddyStatus( [loginController getGPConnectionPtr], i, &status )) 
			{
				buddyProfileIds[i] = status.profile;
			} 
			else 
				break;
		}
			
		static SAKESearchForRecordsInput input;
		SAKERequest request;		
		static char *fieldNames[] = { 
			(char *) "ownerid" , 
			(char *) "row" , 
			 ATLAS_GET_STAT_NAME( NICK ) , 
			 ATLAS_GET_STAT_NAME( BEST_RACE_TIME ) , 
			 ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ) , 
			 ATLAS_GET_STAT_NAME( CAREER_WINS ) , 
			 ATLAS_GET_STAT_NAME( CAREER_LOSSES ) , 
			 ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ) ,
			 ATLAS_GET_STAT_NAME( LATITUDE ) ,
			 ATLAS_GET_STAT_NAME( LONGITUDE)
		};		

		SAKEStartRequestResult startRequestResult;
			
			input.mTableId = (char *) PLAYER_STATS_TABLE;
			input.mFieldNames = fieldNames;		
			input.mNumFields = (sizeof(fieldNames) / sizeof(fieldNames[0]));		
			input.mFilter = (char *) "BEST_RACE_TIME > 0 and BEST_RACE_TIME != 99999";
			input.mSort = (char *) "BEST_RACE_TIME asc";
			//input.mSort = (char *) "recordid asc";
			input.mOffset = 0;		
			input.mMaxRecords = MAX_BUDDIES;		
			input.mOwnerIds = buddyProfileIds;		
			input.mNumOwnerIds = numBuddies;		
			//input.mCacheFlag = gsi_true;
			
			request = sakeSearchForRecords(sake, &input, SearchForLeadersTimeRecordsCallback, self);		
			if(!request)			
			{			
				startRequestResult = sakeGetStartRequestResult(sake);			
				NSLog(@"Failed to start buddylist request: %d\n", startRequestResult);			
			}
		}
	}
}

- (void)showLeadersByLocation
{
	[self deleteLeaderboards];
	gLeaderboardType = LEADERBOARD_BY_LOCATION;
	LeadersByLocationController * newview = (LeadersByLocationController *)[self pushNewControllerOfType: [LeadersByLocationController class] nib: @"leadersbylocation"];
	[newview searchLeaderboardsByLocation];
}

// show the profile of a user in the leaderboard array, indexed by index
- (void)showUser: (int) index
{
	gLeaderboardProfileIndex = index;
	[self pushNewControllerOfType: [UserStatsController class] nib: @"UserStats"];
}

- (UIViewController*)findExistingControllerOfType: (Class)type
{
	NSArray* viewControllers = navController.viewControllers;
	NSUInteger index = [viewControllers count];
	
	while (index > 0) 
	{
		UIViewController* viewController = [viewControllers objectAtIndex: --index];
		
		if ([viewController isKindOfClass: type]) 
			return viewController;
	}
	
	return nil;
}

- (void)deleteLeaderboards
{
	if (gLeaderboardData != nil) {
		for (int i=[gLeaderboardData count]-1; i>=0; i--) 
		{
			LeaderboardObject * pData = (LeaderboardObject*)[gLeaderboardData objectAtIndex: i];
			if (pData.playerLocation != nil) 
			{
				pData.playerLocation = nil; // this is a property, auto-releases
			}
			if (pData.name != nil) 
			{
				pData.name = nil; // this is a property, auto-releases
			}
			if (pData.thumbNail != nil) 
			{
				pData.thumbNail = nil; // this is a property, auto-releases
			}
			[gLeaderboardData removeObjectAtIndex: i];
		}
		
		[gLeaderboardData release];
		gLeaderboardData = nil;
	}
	gLeaderboardType = LEADERBOARD_INVALID;
}

- (void)deleteBlockedImagesData
{
	if( gPlayerData.blockedImagesData != nil ) 
	{
		for( int i = 0; i < (int)[gPlayerData.blockedImagesData count]; i++ ) 
		{
			[gPlayerData.blockedImagesData removeObjectAtIndex: i];
		}
		
		[gPlayerData.blockedImagesData release];
		gPlayerData.blockedImagesData = nil;
		
		MessageBox(@"Cleared all blocked images.");
	}
}


// Remove leaders that have a default time of 99.999
- (void)removeFalseLeaders
{
	if (gLeaderboardData != nil) 
	{
		for (int i=[gLeaderboardData count]-1; i>=0; i--) 
		{
			LeaderboardObject * pData = (LeaderboardObject*)[gLeaderboardData objectAtIndex: i];
			
			// Remove entry if player has default best time
			if (pData.bestRaceTime == INITIAL_BEST_TIME ) 
			{
				if (pData.playerLocation != nil) 
					pData.playerLocation = nil; // this is a property, auto-releases
				
				if (pData.name != nil) 
					pData.name = nil; // this is a property, auto-releases

				if (pData.thumbNail != nil) 
					pData.thumbNail = nil; // this is a property, auto-releases

				[gLeaderboardData removeObjectAtIndex: i];
			}
		
			if( [gLeaderboardData count] == 0 )
			{
				[gLeaderboardData release];
				gLeaderboardData = nil;
				gLeaderboardType = LEADERBOARD_INVALID;
			}
		}
	}
}

- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName
{
	return [self pushNewControllerOfType: type nib: nibName animated: YES];
}

- (bool)imageIsBlocked: (int) pictureId
{
	int numBlockedImages = [gPlayerData.blockedImagesData count];
	if( pictureId == 0 || numBlockedImages == 0 )
		return false;
	
	NSNumber* fileIdNum = [[NSNumber alloc] initWithUnsignedInt: pictureId ];
	
	for (int i = 0; i < numBlockedImages; i++ )
	{
		if( [[gPlayerData.blockedImagesData objectAtIndex: i ] isEqualToNumber:fileIdNum ] )
			return true;
	}
	
	return false;
}


@end


@implementation AppDelegate(Private)

// Called when an alertView button is tapped.
/*- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex 
{
	// 0 index is "Cancel" button
	if(alertView.cancelButtonIndex >= 0 && buttonIndex == alertView.cancelButtonIndex) 
	{
		[self stopHostingCountdownTimer]; 
	}
}
*/

// Called by system mainloop when animation of alert view is finished
- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
	switch (currentAlert) 
	{
		case ALERT_LOADINGPLAYERDATA:
			if( buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				[loadRemoteDataTimer invalidate];
				[self logout];
			}
			break;

		case ALERT_LISTENING:
			NSLog(@"ALERT_LISTENING");
			if( buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				NSLog(@"ALERT_LISTENING: qr2 shutdown");
				qr2_shutdown(NULL);	// I would like this to de-list the host from the master server, but it doesn't seem to do that.

				if (gt2Connection)
				{
					gt2CloseConnection(gt2Connection);
					gt2Connection = NULL;
				}
				
				GT2Socket tempsocket = gt2Socket;
				gt2Socket = NULL; // prevent recursion
				NSLog(@"ALERT_LISTENING:   null & close gt2 socket");
				gt2CloseSocket(tempsocket);

				MatchmakingController* matchmakingcontroller = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
				[matchmakingcontroller hostingCanceled];
			}
			break;

		case ALERT_NATNEG:
			NSLog(@"ALERT_NATNEG");
			if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				NNCancel(natnegCookie);
			}
			break;

		case ALERT_CONNECTING:
			NSLog(@"ALERT_CONNECTING");
			if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				if (gt2Connection)
				{
					gt2CloseConnection(gt2Connection);
					gt2Connection = NULL;
				}
				
				if (gt2Socket != NULL) 
				{
					GT2Socket tempsocket = gt2Socket;
					gt2Socket = NULL; // prevent recursion
					NSLog(@"ALERT_CONNECTING:   null & close gt2 socket");
					gt2CloseSocket(tempsocket);
				}
			}
			break;

		case ALERT_REPLAY:
			NSLog(@"ALERT_REPLAY");
			if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				char buffer[32];
				int rcode = gtEncode(MSG_REPLAY_CANCELED_TYPE, MSG_REPLAY_CANCELED, buffer, sizeof(buffer));
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_REPLAY_CANCELED_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
			}
			break;

		case ALERT_REPLAY_CONFIRM:
			NSLog(@"ALERT_REPLAY_CONFIRM");
			if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				char buffer[32];
				int rcode = gtEncode(MSG_REPLAY_CANCELED_TYPE, MSG_REPLAY_CANCELED, buffer, sizeof(buffer));
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_REPLAY_CANCELED_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
			}
			else if (buttonExists(alertView.firstOtherButtonIndex) >= 0 && buttonIndex == alertView.firstOtherButtonIndex) 
			{
				char buffer[32];
				int rcode = gtEncode(MSG_REPLAY_TYPE, MSG_REPLAY, buffer, sizeof(buffer));
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_REPLAY_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
				[self setupMatch: hosting];
				[navController popToViewController: gameController animated: YES];
			}
			break;
			
		case ALERT_WAITING_FOR_LEADERS:
			NSLog(@"ALERT_WAITING_FOR_LEADERS");
			/*if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				// what can we do to cancel a sake search?
				// nothing for now.
			}*/
			break;
			

		case ALERT_NO_NETWORK_CONNECTION:
			NSLog(@"ALERT_NO_NETWORK_CONNECTION");
			if (buttonExists(alertView.cancelButtonIndex) && buttonIndex == alertView.cancelButtonIndex) 
			{
				NSLog(@"No network connection");
				//exit(0);
				[self logout];
			}
			break;

		default:
			if( currentAlert == ALERT_NONE )
				NSLog(@"ALERT_NONE");
			else
				NSLog(@"ERROR: UNKNOWN ALERT");
			break;
	}

	currentAlert = ALERT_NONE;
}

//- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName
//{
//	return [self pushNewControllerOfType: type nib: nibName animated: YES];
//}

- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName animated: (BOOL)animated
{
	UIViewController* viewController = [type alloc];
	[viewController initWithNibName: nibName bundle: nil];
	[navController pushViewController: viewController animated: animated];
	[viewController release];
	return viewController;
}


- (void)setupMatch: (bool)hostMatch
{
	start = 0;
	countdown = 0;
	localTime = 0;
	remoteTime = 0;
	racing = false;
	reportSent = gsi_false;
	hosting = hostMatch;
}


- (bool)setupHosting
{
	[self setupMatch: true];


	if( gt2Socket == NULL )
	{
		NSLog(@"setupHosting:  create gt2 socket");
		GT2Result gt2Result = gt2CreateSocket(&gt2Socket, HOST_PORT_STRING, 0, 0, SocketErrorCallback);
		NSLog(@"gt2socket = %d", gt2Socket );
		if (gt2Result != GT2Success)
		{
			NSLog(@"gt2socket creation failed");
			hosting = false;
			return false;
		}
	}
	gt2SetSocketData(gt2Socket, self);
	gt2Listen(gt2Socket, ConnectAttemptCallback);
	gt2SetUnrecognizedMessageCallback(gt2Socket, UnrecognizedMessageCallback);
	

	state = HOST_LISTENING;
	NSLog(@"setupHosting state=HOST_LISTENING");
	
	//qr2_error_t qr2Result = qr2_init( NULL, NULL, HOST_PORT /*2*/, SCRACE_GAMENAME, SCRACE_SECRETKEY, TRUE, TRUE,
	//									ServerKeyCallback, PlayerKeyCallback, TeamKeyCallback, KeyListCallback, CountCallback, AddErrorCallback, self); 
										
	qr2_error_t qr2Result = qr2_init_socketA(NULL, gt2GetSocketSOCKET(gt2Socket), HOST_PORT, SCRACE_GAMENAME, SCRACE_SECRETKEY, TRUE, TRUE, ServerKeyCallback, PlayerKeyCallback, TeamKeyCallback, KeyListCallback, CountCallback, AddErrorCallback, self);

	if (qr2Result != e_qrnoerror) 
	{
		if (gt2Connection)
		{
			gt2CloseConnection(gt2Connection);
			gt2Connection = NULL;
		}
		if( gt2Socket )
		{
			GT2Socket tempsocket = gt2Socket;
			gt2Socket = NULL; // prevent recursion
			NSLog(@"setupHosting:  null & close gt2 socket");
			gt2CloseSocket(tempsocket);
		}
		
		return false;
	}

	qr2_register_natneg_callback(NULL, NatNegCallback);
	
	alertView = [[UIAlertView alloc] initWithTitle: @"Waiting for player" message: @"30 seconds" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
	[alertView show];
	currentAlert = ALERT_LISTENING;

	NSLog(@"setupHosting starts hostingCountdownTimer");
	[self startHostingCountdownTimer];

	return true;
}

- (bool)setupJoining: (unsigned int)ip port: (unsigned short)port useNatNeg: (bool)useNatNeg;
{
	[self setupMatch: false];

	// create socket if new match
	if( gt2Socket == NULL )
	{
		NSLog(@"setupJoining:  create gt2 socket");
		GT2Result result = gt2CreateSocket(&gt2Socket, CLIENT_PORT_STRING, 0, 0, SocketErrorCallback);
		if (result != GT2Success)
			return false;
	}

	gt2SetSocketData(gt2Socket, self);
	
	// Do NAT negotiation if needed
	if (useNatNeg) 
	{
		const char* ip_addr = inet_ntoa((in_addr){ ip });

		while (natnegCookie == 0) 
		{
			natnegCookie = rand();
		}

		SBError sbret = ServerBrowserSendNatNegotiateCookieToServerA(serverBrowser, ip_addr, port, natnegCookie);

		if (sbret != sbe_noerror) 
		{
			// What if somebody else is already doing a transaction with this cookie value?  Astronomically improbable, but does the server return an error value?
			NSLog(@"NN transaction cancelled due to cookie in use.");
			return false;
		}

		NegotiateError error = NNStartNatDetection(NNDetectionCallback);
		
		
		gt2SetUnrecognizedMessageCallback(gt2Socket, UnrecognizedMessageCallback);
		NegotiateError nnret = NNBeginNegotiationWithSocket(gt2GetSocketSOCKET(gt2Socket), natnegCookie, NN_CLIENT, NNProgressCallback, NNCompletedCallback, self);

		if (nnret != ne_noerror) 
		{
			// ne_noerror, ne_allocerror, ne_socketerror, ne_dnserror
			/*if( nnret == ne_socketerror )
				NSLog(@"NN socket error.");
			else if( nnret == ne_dnserror ) 
				NSLog(@"NN DNS error.");
			else
				NSLog(@"NN allocation error.");
			*/
			return false;
		}

		alertView = [[UIAlertView alloc] initWithTitle: @"Connection" message: @"Negotiating..." delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
		[alertView show];
		currentAlert = ALERT_NATNEG;
	}

	else // if not using NatNeg
	{
		NSString* remoteHost = [NSString stringWithFormat: @"%s:%d", inet_ntoa((in_addr){ ip }), port];
		[self startMultiPlayerGame: [remoteHost cStringUsingEncoding: NSASCIIStringEncoding]];
	}
	
	return true;
}

- (void)onLoadRemoteDataTimer: (NSTimer*)timer
{
	gsCoreThink(0);
}

- (void)onThinkTimer: (NSTimer*)timer
{
	static bool thinking;
	
	if (!thinking)
	{
		thinking = true;
		
		// Think so SDKs can process
		/////////////////////////////////////////////
		if (gt2Socket != NULL)
			gt2Think(gt2Socket);
		
		ghttpThink();
		scThink(interface);
		
		if (hosting)
			qr2_think(NULL);
		
		if (serverBrowser != NULL)
			ServerBrowserThink(serverBrowser);
		
		NNThink();

		switch (state)
		{
			// ********************************************************** //
			// ************************* HOST LOGIC ********************* //
			// ********************************************************** //

			case HOST_LISTENING:
				// Wait for client to join
				/////////////////////////////////////////////
				break;

			case HOST_EXCHANGE_CERT:
			{
				char buffer[520];
				char cert[512];
				int rcode;
				gsi_u32 certLen;
				
				// Store cert in a binary buffer for easy exchange
				/////////////////////////////////////////////
				wsLoginCertWriteBinary(&gPlayerData.certificate, cert, sizeof(cert), &certLen);
				
				// Exchange certificates with the other player to validate (step 1)
				/////////////////////////////////////////////
				rcode = gtEncode(MSG_TO_CLIENT_CERT_TYPE, MSG_TO_CLIENT_CERT, buffer, sizeof(buffer), gPlayerData.profileId, cert, certLen);
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_TO_CLIENT_CERT_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
				
				// Wait for a reply
				/////////////////////////////////////////////
				state = HOST_WAITING;
				NSLog(@"onThinkTimer state=HOST_WAITING from HOST_EXCHANGE_CERT");
				break;
			}

			case HOST_VERIFY_CERT:
				// Validate authentication certificates (step 2)
				/////////////////////////////////////////////
				remoteCertificate.mIsValid = wsLoginCertIsValid(&remoteCertificate);
				if (gsi_is_false(remoteCertificate.mIsValid))
				{
					[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
					[alertView release];
					alertView = nil;
					MessageBox(@"Remote player has an invalid certificate, cancelling game.");
				}
				else
				{
					char buffer[32];
					int rcode = gtEncode(MSG_TO_CLIENT_CERTVERIFIED_TYPE, MSG_TO_CLIENT_CERTVERIFIED, buffer, sizeof(buffer));
					assert(rcode != -1);
					NSLog(@"Protocol send MSG_TO_CLIENT_CERTVERIFIED_TYPE\n");
					gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
					
					// Wait for a reply
					/////////////////////////////////////////////
					state = HOST_WAITING;
					NSLog(@"onThinkTimer state=HOST_WAITING from HOST_VERIFY_CERT");
				}
				break;

			case HOST_EXCHANGE_KEYS:
			{
				char buffer[512];
				int rcode;
				SCPeerKeyExchangeMsg exchangeMsg;
				
				// P2P encryption exchange keys (step 3)
				/////////////////////////////////////////////
				
				// Each player should create a key for receiving data from the remote player
				//     For extra security, we use a different encryption key for each channel
				/////////////////////////////////////////////
				scPeerCipherInit(&gPlayerData.certificate, &gPlayerData.peerRecvCipher); 
				
				// Create a key exchange message for transmitting the key to the other player
				// using the remote player's certificate to encrypt the cipher
				/////////////////////////////////////////////
				scPeerCipherCreateKeyExchangeMsg(&remoteCertificate, &gPlayerData.peerRecvCipher, exchangeMsg);
				
				// Now send the key to the other player
				/////////////////////////////////////////////
				rcode = gtEncode(MSG_TO_CLIENT_KEYS_TYPE, MSG_TO_CLIENT_KEYS, buffer, sizeof(buffer), exchangeMsg, GS_CRYPT_RSA_BYTE_SIZE);
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_TO_CLIENT_KEYS_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
				
				// Wait for a reply
				/////////////////////////////////////////////
				state = HOST_WAITING;
				NSLog(@"onThinkTimer state=HOST_WAITING from HOST_EXCHANGE_KEYS");
				break;
			}

			case HOST_CONNECTED:
				if (alertView != nil) 
				{
					//[alertView dismissWithClickedButtonIndex: -1 animated: YES];
					[alertView dismissWithClickedButtonIndex: alertView.firstOtherButtonIndex  animated: YES];
					[alertView release];
					alertView = nil;
				}
				
				state = RACING;
				NSLog(@"onThinkTimer state=RACING on HOST_CONNECTED");
				
				gameController = (GameController*)[self pushNewControllerOfType: [GameController class] nib: @"twoplayergame"];
				[gameController setRemoteNick: [NSString stringWithCString: remoteCertificate.mUniqueNick encoding: NSASCIIStringEncoding]];
				gOpponentData.profileId = remoteCertificate.mProfileId;
				localTime = 0;
				remoteTime = 0;
				break;

			case HOST_SEND_SESSID:
				if(sessionCreated)
				{
					int rcode;
					char buffer[256];
					char sessionCrypt[SC_SESSION_GUID_SIZE];
					char connCrypt[SC_CONNECTION_GUID_SIZE];
					
					// Encrypt the connID/session ID to send using P2P encryption
					/////////////////////////////////////////////////////////
					memcpy(sessionCrypt, gPlayerData.sessionId, SC_SESSION_GUID_SIZE);
					scPeerCipherEncryptBufferIV(&gPlayerData.peerSendCipher, 1, (gsi_u8*)sessionCrypt, SC_SESSION_GUID_SIZE);
					
					memcpy(connCrypt, gPlayerData.connectionId, SC_CONNECTION_GUID_SIZE);
					scPeerCipherEncryptBufferIV(&gPlayerData.peerSendCipher, 2, (gsi_u8*)connCrypt, SC_CONNECTION_GUID_SIZE);
					
					OutputDebugString([NSString stringWithFormat: @"[HOST_SEND_SESSID] sessionCrypt: %.40s\n", sessionCrypt]);
					OutputDebugString([NSString stringWithFormat: @"[HOST_SEND_SESSID] connCrypt: %.40s\n", connCrypt]);
					
					
					// Now the host sends the session ID & his conn ID to the client
					/////////////////////////////////////////////
					rcode = gtEncode(MSG_SESSION_ID_TYPE, MSG_SESSION_ID, buffer, sizeof(buffer), 
									 sessionCrypt, SC_SESSION_GUID_SIZE, connCrypt, SC_CONNECTION_GUID_SIZE);
					assert(rcode != -1);
					NSLog(@"Protocol send MSG_SESSION_ID_TYPE\n");
					gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
					
					
					// Once session is created, set the session ID and report intention
					/////////////////////////////////////////////
					scSetSessionId(interface, gPlayerData.sessionId);
					scSetReportIntention(interface, gPlayerData.connectionId, SCRACE_AUTHORITATIVE, &gPlayerData.certificate, &gPlayerData.privateData,
										 SetReportIntentionCallback, SCRACE_TIMEOUT_MS, self);
					
					sessionCreated = gsi_false;
					
					// Go back to racing state
					/////////////////////////////////////////////
					state = RACING;
					NSLog(@"onThinkTimer state=RACING on HOST_SEND_SESSID");
				}
				break;

			case HOST_ERROR:
				if (alertView != nil) 
				{
					[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
					[alertView release];
					alertView = nil;
				}
				
				MessageBox(@"Error setting up hosting");
				state = JOIN_CHOOSESERVER;
				NSLog(@"onThinkTimer state=JOIN_CHOOSESERVER on HOST_ERROR");
				break;


		// ********************************************************** //
		// **************** JOIN (CLIENT) LOGIC ********************* //
		// ********************************************************** //
		
			case JOIN_EXCHANGE_CERT:
				// Wait for host to send cert first
				/////////////////////////////////////////////
				break;

			case JOIN_VERIFY_CERT:
				// Validate authentication certificates (step 2)
				/////////////////////////////////////////////
				remoteCertificate.mIsValid = wsLoginCertIsValid(&remoteCertificate);
				if (gsi_is_false(remoteCertificate.mIsValid))
				{
					[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
					[alertView release];
					alertView = nil;
					MessageBox(@"Remote player has an invalid certificate, cancelling game.");
					[self logout];
				}
				else
				{
					state = JOIN_EXCHANGE_KEYS;
					NSLog(@"onThinkTimer state=JOIN_EXCHANGE_KEYS on JOIN_VERIFY_CERT");
				}
				break;

			case JOIN_EXCHANGE_KEYS:
				// Wait for host to send keys first
				/////////////////////////////////////////////
				break;

			case JOIN_CONNECTED:
				if (alertView != nil) 
				{
					[alertView dismissWithClickedButtonIndex: -1 animated: YES];
					//[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
					[alertView release];
					alertView = nil;
				}
				state = RACING;
				NSLog(@"onThinkTimer state=RACING on JOIN_CONNECTED");
				
				gameController = (GameController*)[self pushNewControllerOfType: [GameController class] nib: @"twoplayergame"];
				[gameController setRemoteNick: [NSString stringWithCString: remoteCertificate.mUniqueNick encoding: NSASCIIStringEncoding]];
				localTime = 0;
				remoteTime = 0;
				break;

			case JOIN_SEND_CONNID:
				// Once connection ID has been set, relay it to the host
				/////////////////////////////////////////////
				if (connIdSet)
				{
					int rcode;
					char buffer[128];		
					char connCrypt[SC_CONNECTION_GUID_SIZE];
					
					// Encrypt the connection ID to send using P2P encryption
					/////////////////////////////////////////////////////////
					memcpy(connCrypt, gPlayerData.connectionId, SC_CONNECTION_GUID_SIZE);
					scPeerCipherEncryptBufferIV(&gPlayerData.peerSendCipher, 1, (gsi_u8*)connCrypt, SC_CONNECTION_GUID_SIZE);
					
					OutputDebugString([NSString stringWithFormat: @"[JOIN_SEND_CONNID] connCrypt: %.40s\n", connCrypt]);
					
					
					// Client needs to send the host his/her connection ID
					/////////////////////////////////////////////
					rcode = gtEncode(MSG_CONNECTION_ID_TYPE, MSG_CONNECTION_ID, buffer, sizeof(buffer), connCrypt, SC_CONNECTION_GUID_SIZE);
					assert(rcode != -1);
					NSLog(@"Protocol send MSG_CONNECTION_ID_TYPE\n");
					gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
					
					connIdSet = gsi_false;
					
					// set profile id of opponent
					gOpponentData.profileId = remoteCertificate.mProfileId;
					
					// Go back to the racing state
					/////////////////////////////////////////////
					state = RACING;
					NSLog(@"onThinkTimer state=RACING on JOIN_SEND_CONNID");
				}
				break;

			case JOIN_ERROR:
				if (alertView != nil) 
				{
					[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
					[alertView release];
					alertView = nil;
				}
				
				MessageBox(@"Error joining a game");
				state = JOIN_CHOOSESERVER;
				NSLog(@"onThinkTimer state=JOIN_CHOOSESERVER on JOIN_ERROR");
				break;
		}

		thinking = false;
	}
	
	// Are we racing?
	/////////////////
	if(racing)
	{
		// Did we finish?
		/////////////////
		if(localTime)
		{			
			// Let our opponent know we are done.
			////////////////////////////////////////
			char buffer[64];
			int rcode;				
			int steps = gameController.numSteps;
			if ( playerCount > 1 && steps != gameController.prevNumSteps ) 
			{
				gameController.prevNumSteps = steps;
				rcode = gtEncode(MSG_PROGRESS_TYPE, MSG_PROGRESS, buffer, sizeof(buffer), steps);
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_PROGRESS_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2False);
			}			
			
			// Did we both finish?
			//////////////////////
			if(remoteTime)
			{
				// Done racing.
				///////////////
				racing = false;
				state = FINISHED_RACE;
				NSLog(@"onThinkTimer state=FINISHED_RACE");
				
				[gameController doneRacing];
				
				// Show the times.
				//////////////////
				NSString* message;
				if(localTime < remoteTime)
				{
					message = [NSString stringWithFormat: @"You won!\n%0.2fs to %0.2fs", localTime * absolute_to_seconds, remoteTime * absolute_to_seconds];
					win = gsi_true;
				}
				else if(remoteTime < localTime)
				{
					message = [NSString stringWithFormat: @"You lost!\n%0.2fs to %0.2fs", localTime * absolute_to_seconds, remoteTime * absolute_to_seconds];
				}
				else
				{
					message = [NSString stringWithFormat: @"You tied!\n%0.2fs", localTime * absolute_to_seconds];
					tie = gsi_true;
				}
				
				MessageBox(message);
				
				// Report the stats.
				////////////////////
				if (!reportSent)
				{
					disconnected = gsi_false;
					[self reportTwoPlayerStats];
					[self saveTwoPlayerGameStatsToFile];
				}

				GameResultsController* controller = (GameResultsController*)[self pushNewControllerOfType: [GameResultsController class] nib: @"twoplayerresults" animated: YES];
				int localTimeMillis = localTime * absolute_to_millis;
				int remoteTimeMillis = remoteTime * absolute_to_millis;
				//[controller setGameTime: std::max(localTimeMillis, remoteTimeMillis)];
				
				[controller setLocalPlayer: gPlayerData.uniquenick time: localTimeMillis];
				[controller setRemotePlayer: [NSString stringWithCString: remoteCertificate.mUniqueNick encoding: NSASCIIStringEncoding] time: remoteTimeMillis];
				[controller setWinner ];  // call only after setting local & remote
				[controller getImagesForResultsPage];
			}
		}
		else
		{
			char buffer[64];
			int rcode;				
			int steps = gameController.numSteps;
			
			// Let our opponent know how far we are if we tapped.
			////////////////////////////////////////
			if ( playerCount > 1 && steps != gameController.prevNumSteps ) 
			{
				gameController.prevNumSteps = steps;
				rcode = gtEncode(MSG_PROGRESS_TYPE, MSG_PROGRESS, buffer, sizeof(buffer), steps);
				assert(rcode != -1);
				NSLog(@"Protocol send MSG_PROGRESS_TYPE\n");
				gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2False);
			}
		}
	}
}

- (void)onCountdownTimer: (NSTimer*)timer
{
	--countdown;
	[self countdown];

	if (countdown == 0)
	{
		[countdownTimer invalidate];
		countdownTimer = nil;
		[self startRace];
	}
}

// Countdown popup when 2 player game is connecting
- (void)onJoiningCountdownTimer: (NSTimer*)timer
{
	--joiningCountdown;
	alertView.message = [NSString stringWithFormat: @"%d seconds", joiningCountdown];
	
	if (joiningCountdown == 0)
	{
		[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
		[alertView release];
		alertView = nil;
		
		NSLog(@"onJoiningCountdownTimer stop timer");
		
		[joiningCountdownTimer invalidate];
		joiningCountdownTimer = nil;
		
		if( currentAlert == ALERT_REPLAY )
		{
			// tell opponent rematch is cancelled due to taking too long to reply
			char buffer[32];
			int rcode = gtEncode(MSG_REPLAY_CANCELED_TYPE, MSG_REPLAY_CANCELED, buffer, sizeof(buffer));
			assert(rcode != -1);
			NSLog(@"Protocol send MSG_REPLAY_CANCELED_TYPE\n");
			gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
			
			MessageBox(@"Failed to connect with remote player. Match canceled.");
		}
		else
		{
			NSLog( @"onJoiningCountdownTimer timed out with currentAlert=%d", currentAlert );
			MessageBox(@"Error establishing connection.", @"Connection Error");
		}
		
		// cancel hosting
		MatchmakingController* matchmakingController = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
		[ matchmakingController hostingCanceled ];
		
		[self stopHostingServer];
		hosting = false;
		racing = false;
		state = JOIN_CHOOSESERVER;
		NSLog(@"onJoiningCountdownTimer state=JOIN_CHOOSESERVER");

	}
}

- (void)onHostingCountdownTimer: (NSTimer*)timer
{
	--hostingCountdown;
	alertView.message = [NSString stringWithFormat: @"%d seconds", hostingCountdown];
	
	if (hostingCountdown == 0)
	{
		[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: YES];
		[alertView release];
		alertView = nil;
		
		[hostingCountdownTimer invalidate];
		hostingCountdownTimer = nil;

		// remove hosting from server
		[self stopHostingServer];
		
		//NSLog(@"onHostingCountdownTimer: qr2 shutdown");
		//qr2_shutdown(NULL);
		
		if( currentAlert == ALERT_REPLAY )
		{
			// tell opponent rematch is cancelled due to taking too long to reply
			char buffer[32];
			int rcode = gtEncode(MSG_REPLAY_CANCELED_TYPE, MSG_REPLAY_CANCELED, buffer, sizeof(buffer));
			assert(rcode != -1);
			NSLog(@"Protocol send MSG_REPLAY_CANCELED_TYPE\n");
			gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
			
			MessageBox(@"Player has not responded. Rematch canceled.");
		}
		
		// cancel hosting
		MatchmakingController* matchmakingController = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
		[ matchmakingController hostingCanceled ];
		
		hosting = false;
		racing = false;
		state = JOIN_CHOOSESERVER;
		NSLog(@"onHostingCountdownTimer state=JOIN_CHOOSESERVER");
		
		[self returnToMatchMaking];
	}
}

// Called after each 2 player matches
- (void)saveTwoPlayerGameStatsToFile
{
	// get my race time
	int myRaceTimeMillis = (uint32_t)(localTime * absolute_to_millis);

	
	if (gPlayerData.playerStatsData == nil) // no local stats file
	{
		// get here if no server stats also so init first ever stats		
		
		// set win, loss, draw and disconnect
		if (!disconnected)
		{
			gPlayerData.totalCompleteMatches = 1;
			
			gPlayerData.bestRaceTime = gPlayerData.worstRaceTime = 
			gPlayerData.totalRaceTime = myRaceTimeMillis;
			gPlayerData.averageRaceTime = localTime;
			
			if( win )
			{
				gPlayerData.careerWins = gPlayerData.matchWinStreak = 
				gPlayerData.careerLongestWinStreak = 1;
			}
			else if( !tie )
			{
				gPlayerData.careerLosses = gPlayerData.matchLossStreak =
				gPlayerData.careerLongestLossStreak = 1;
			}
			else
			{
				gPlayerData.careerDraws = gPlayerData.matchDrawStreak =
				gPlayerData.careerLongestDrawStreak = 1;
			}
			
			gPlayerData.disconnectRate = 0.0;
			gPlayerData.careerDisconnects = 0;
		}
		else
		{
			gPlayerData.disconnectRate = 1.0;
			gPlayerData.careerDisconnects = 1;
		}
		
		gPlayerData.totalMatches = 1;
		
		// Save first 2 player game stats to new file
		gPlayerData.playerStatsData = [[NSMutableDictionary alloc] initWithObjectsAndKeys:	
					[NSNumber numberWithInt: gPlayerData.bestRaceTime], matchBestTimeKey,
					[NSNumber numberWithInt: gPlayerData.worstRaceTime], matchWorstTimeKey,
					[NSNumber numberWithInt: (int)(gPlayerData.averageRaceTime * 1000)], matchAverageTimeKey,
					
					[NSNumber numberWithInt: gPlayerData.careerWins], matchWonKey,
					[NSNumber numberWithInt: gPlayerData.careerLosses], matchLossKey,
					[NSNumber numberWithInt: gPlayerData.careerDraws], matchDrawsKey,
					
					[NSNumber numberWithInt: gPlayerData.matchWinStreak], matchWinStreakKey,
					[NSNumber numberWithInt: gPlayerData.matchLossStreak], matchLossStreakKey,
					[NSNumber numberWithInt: gPlayerData.matchDrawStreak], matchDrawStreakKey,
					
					[NSNumber numberWithInt: gPlayerData.totalMatches], matchGamesPlayedKey,
					[NSNumber numberWithInt: gPlayerData.totalRaceTime], matchTotalRaceTimeKey,
					[NSNumber numberWithInt: gPlayerData.careerDisconnects], matchCareerDisconnectsKey,
					[NSNumber numberWithInt: gPlayerData.totalCompleteMatches], matchTotalCompleteMatchesKey,
					[NSNumber numberWithFloat: gPlayerData.disconnectRate], matchDisconnectRateKey,
					
					
					[NSNumber numberWithInt: gPlayerData.careerLongestWinStreak], matchCareerLongestWinStreakKey,
					[NSNumber numberWithInt: gPlayerData.careerLongestLossStreak], matchCareerLongestLossStreakKey,
					[NSNumber numberWithInt: gPlayerData.careerLongestDrawStreak], matchCareerLongestDrawStreakKey,
					
					[NSString stringWithCString: gPlayerData.lastGameTime encoding: NSASCIIStringEncoding], lastTimePlayedKey,
					nil];
	}
	else if (disconnected)
	{
		int discon = getLocalIntegerStat(matchCareerDisconnectsKey);
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: discon+1] forKey: matchCareerDisconnectsKey];

		int totalMatches = getLocalIntegerStat(matchGamesPlayedKey);
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: totalMatches+1] forKey: matchGamesPlayedKey];

		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: 1.0*discon/totalMatches] forKey: matchDisconnectRateKey];

	}
	else
	{	
		int bestRaceTime = getLocalIntegerStat(matchBestTimeKey);
		if( myRaceTimeMillis < bestRaceTime ) 
		{
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: myRaceTimeMillis] forKey: matchBestTimeKey];
		}
		
		int worstRaceTime = getLocalIntegerStat(matchWorstTimeKey);
		if( myRaceTimeMillis > worstRaceTime )
		{
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: myRaceTimeMillis] forKey: matchWorstTimeKey];
		} 

		int totalRaceTime = getLocalIntegerStat(matchTotalRaceTimeKey) + myRaceTimeMillis;
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: totalRaceTime] forKey: matchTotalRaceTimeKey];

		int totalMatches = getLocalIntegerStat(matchGamesPlayedKey) + 1;
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: totalMatches] forKey: matchGamesPlayedKey];

		int discon = getLocalIntegerStat(matchCareerDisconnectsKey);
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: 1.0*discon/totalMatches] forKey: matchDisconnectRateKey];

		int newAvgRaceTime = totalRaceTime / totalMatches;
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithFloat: newAvgRaceTime] forKey: matchAverageTimeKey];
		
		int totalCompletedMatches = getLocalIntegerStat(matchTotalCompleteMatchesKey) + 1;
		[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: totalCompletedMatches] forKey: matchTotalCompleteMatchesKey];
		
		//int wins = getLocalIntegerStat(matchWonKey) ;
		//int loss = getLocalIntegerStat(matchLossKey);
		//int draws = getLocalIntegerStat(matchDrawsKey);

		if( win )
		{
			gPlayerData.careerWins++;
			gPlayerData.matchWinStreak++;
			gPlayerData.matchLossStreak = gPlayerData.matchDrawStreak = 0;	

			if( gPlayerData.matchWinStreak > gPlayerData.careerLongestWinStreak ) 
			{
				gPlayerData.careerLongestWinStreak = gPlayerData.matchWinStreak;
				[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestWinStreak] forKey: matchCareerLongestWinStreakKey];
			}
			
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerWins] forKey: matchWonKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchWinStreak] forKey: matchWinStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchLossStreak] forKey: matchLossStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchDrawStreak] forKey: matchDrawStreakKey];
		}
		else if( !tie )
		{
			gPlayerData.careerLosses++;
			gPlayerData.matchLossStreak++;
			gPlayerData.matchWinStreak = gPlayerData.matchDrawStreak = 0;	

			if( gPlayerData.matchLossStreak > gPlayerData.careerLongestLossStreak ) 
			{
				gPlayerData.careerLongestLossStreak = gPlayerData.matchLossStreak;
				[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestLossStreak ] forKey: matchCareerLongestLossStreakKey];
			}
			
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLosses] forKey: matchLossKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchWinStreak ] forKey: matchWinStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchLossStreak ] forKey: matchLossStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchDrawStreak ] forKey: matchDrawStreakKey];
		}
		else
		{
			gPlayerData.careerDraws++;
			gPlayerData.matchDrawStreak++;
			gPlayerData.matchWinStreak = gPlayerData.matchLossStreak = 0;	

			if( gPlayerData.matchDrawStreak > gPlayerData.careerLongestDrawStreak ) 
			{
				gPlayerData.careerLongestDrawStreak = gPlayerData.matchDrawStreak;
				[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerLongestDrawStreak ] forKey: matchCareerLongestDrawStreakKey];
			}
			
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.careerDraws ] forKey: matchDrawsKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchWinStreak ] forKey: matchWinStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchLossStreak ] forKey: matchLossStreakKey];
			[gPlayerData.playerStatsData setObject: [NSNumber numberWithInt: gPlayerData.matchDrawStreak ] forKey: matchDrawStreakKey];
		}
		
		[gPlayerData.playerStatsData setObject: [NSString stringWithCString: gPlayerData.lastGameTime encoding: NSASCIIStringEncoding] forKey: lastTimePlayedKey];		
	}
	
	[gPlayerData.playerStatsData writeToFile: [gPlayerData.playerDataPath stringByAppendingPathComponent: playerStatsDataFile] atomically: YES];

	//	scReportAddFloatValue(statsReport, KEY_LONGITUDE, gPlayerData.playerLocation.coordinate.longitude);
	//	scReportAddFloatValue(statsReport, KEY_LATITUDE, gPlayerData.playerLocation.coordinate.latitude);

}


- (void)reportTwoPlayerStats
{
	SCResult        aResult = SCResult_NO_ERROR;
	SCGameResult	myGameResult, opponentGameResult;
	int				myTeam, opponentTeam;
	int				numTeams = SCRACE_NUM_TEAMS;
	
	
	// Determine winners and losers
	/////////////////////////////////////////////
	if (!disconnected)
	{
		if (win)
		{
			myGameResult = SCGameResult_WIN;
			opponentGameResult = SCGameResult_LOSS;
		}
		else if (!tie)
		{
			myGameResult = SCGameResult_LOSS;
			opponentGameResult = SCGameResult_WIN;
		} 
		else
		{
			myGameResult = SCGameResult_DRAW;
			opponentGameResult = SCGameResult_DRAW;
		}
	}
	else
	{
		//report disconnected game - don't report any keys
		myGameResult = SCGameResult_DISCONNECT;
		opponentGameResult = SCGameResult_DISCONNECT;
	}
	
	
	// Determine teams, and who is on which
	/////////////////////////////////////////////
	if (hosting)
	{
		myTeam = SCRACE_HOST_TEAM;
		opponentTeam = SCRACE_CLIENT_TEAM;
	}
	else
	{
		myTeam = SCRACE_CLIENT_TEAM;
		opponentTeam = SCRACE_HOST_TEAM;
	}
	
	
	// Create the report and begin building it
	/////////////////////////////////////////////
	aResult = scCreateReport(interface, ATLAS_RULE_SET_VERSION, playerCount, numTeams, &statsReport);
	if (aResult != SCResult_NO_ERROR)
	{
		MessageBox(@"Failed to Create Report - Out of memory");
		return;
	}
	
	// Non-player data
	/////////////////////////////////////////////
	scReportBeginGlobalData(statsReport);
	// no global data reported
	
	// Player data
	/////////////////////////////////////////////
	scReportBeginPlayerData(statsReport);
	
	// Report your data
	////////////////////
	scReportBeginNewPlayer(statsReport);
	scReportSetPlayerData(statsReport, PLAYER1, gPlayerData.connectionId, myTeam, 
						  myGameResult, gPlayerData.profileId, &gPlayerData.certificate, gPlayerData.statsAuthdata);
	if (!disconnected) 
	{
		scReportAddIntValue(statsReport, KEY_MULTI_PLAY,  (uint32_t)(TRUE));
		scReportAddIntValue(statsReport, RACE_TIME,  (uint32_t)(localTime * absolute_to_millis));
		scReportAddFloatValue(statsReport, KEY_LONGITUDE, gPlayerData.playerLocation.coordinate.longitude);
		scReportAddFloatValue(statsReport, KEY_LATITUDE, gPlayerData.playerLocation.coordinate.latitude);
		
		static char nickString[256];
		strcpy( nickString,  [gPlayerData.uniquenick cStringUsingEncoding: NSASCIIStringEncoding]);
		scReportAddStringValue(statsReport, KEY_NICK, nickString);
		
		// get current time and save as last game date/time
		//gPlayerData.lastGameTime = [[NSDate date] description];
		//char raceDateTimeString[256];
		NSString *raceTimeString = [[NSDate date] description];
		strcpy( raceDateTimeString, [raceTimeString cStringUsingEncoding: NSASCIIStringEncoding] );
		gPlayerData.lastGameTime = raceDateTimeString;
		scReportAddStringValue(statsReport, KEY_RACE_DATE_TIME, gPlayerData.lastGameTime );
	}
	
	// Report opponent data
	////////////////////
	scReportBeginNewPlayer(statsReport);
	scReportSetPlayerData(statsReport, PLAYER2, remoteConnId, opponentTeam, 
						  opponentGameResult, remoteCertificate.mProfileId, &remoteCertificate, gPlayerData.statsAuthdata);
	if (!disconnected)
		scReportAddIntValue(statsReport, RACE_TIME, (uint32_t)(remoteTime * absolute_to_millis));	
	
	
	// Team data
	/////////////////////////////////////////////
	scReportBeginTeamData(statsReport);
	
	// Report your team data
	////////////////////////
	scReportBeginNewTeam(statsReport); 
	scReportSetTeamData(statsReport, myTeam, myGameResult);
	
	// Report opponent team data
	////////////////////////
	scReportBeginNewTeam(statsReport);
	scReportSetTeamData(statsReport, opponentTeam, opponentGameResult);	
	
	
	// End the report and set GameStatus
	if (!disconnected)
		scReportEnd(statsReport, SCRACE_AUTHORITATIVE, SCGameStatus_COMPLETE);
	else
		scReportEnd(statsReport, SCRACE_AUTHORITATIVE, SCGameStatus_BROKEN);
	
	// Submit the Report
	/////////////////////////////////////////////
	if (SCResult_NO_ERROR != scSubmitReport(interface, statsReport, SCRACE_AUTHORITATIVE, &gPlayerData.certificate, 
											&gPlayerData.privateData, SubmitReportCallback, SCRACE_TIMEOUT_MS, self))
	{
		MessageBox(@"Failed to submit Stats Report.");
		return;
	}
}


- (void)reportSinglePlayerStats
{
	SCResult        aResult = SCResult_NO_ERROR;
//	SCGameResult	myGameResult, opponentGameResult;
//	int				myTeam;
//	int				numTeams = SCRACE_NUM_TEAMS;
			
	
	// Create the report and begin building it
	/////////////////////////////////////////////
	aResult = scCreateReport(interface, ATLAS_RULE_SET_VERSION, 1, 0, &statsReport);
	if (aResult != SCResult_NO_ERROR)
	{
		MessageBox(@"Failed to create Single Player Game stats report - Out of memory");
		return;
	}
	
	scReportSetAsMatchless(statsReport);
	
	// Non-player data
	/////////////////////////////////////////////
	scReportBeginGlobalData(statsReport);
	// no global data reported
	
	// Player data
	/////////////////////////////////////////////
	scReportBeginPlayerData(statsReport);
	
	// Report your data
	////////////////////
	scReportBeginNewPlayer(statsReport);
	scReportSetPlayerData(statsReport, PLAYER1, gPlayerData.connectionId, SCRACE_HOST_TEAM, SCGameResult_WIN, 
						  gPlayerData.profileId, &gPlayerData.certificate, gPlayerData.statsAuthdata);
	if (!disconnected) 
	{
		scReportAddIntValue(statsReport, KEY_SINGLE_PLAY,  (uint32_t)(TRUE));
		scReportAddIntValue(statsReport, SP_RACE_TIME,  (uint32_t)(localTime * absolute_to_millis));
		scReportAddFloatValue(statsReport, KEY_LONGITUDE, gPlayerData.playerLocation.coordinate.longitude);
		scReportAddFloatValue(statsReport, KEY_LATITUDE, gPlayerData.playerLocation.coordinate.latitude);
		
		static char nickString[256];
		strcpy( nickString,  [gPlayerData.uniquenick cStringUsingEncoding: NSASCIIStringEncoding]);
		scReportAddStringValue(statsReport, KEY_NICK, nickString);
		
		// get current time and save as last game date/time
		//gPlayerData.lastGameTime = [[NSDate date] description];
		//char raceDateTimeString[256];
		//NSString *raceTimeString = [[NSDate date] description];
		//strcpy( raceDateTimeString, [raceTimeString cStringUsingEncoding: NSASCIIStringEncoding] );
		//gPlayerData.lastGameTime = raceDateTimeString;
		scReportAddStringValue(statsReport, KEY_RACE_DATE_TIME, gPlayerData.lastGameTime );
	}
	
	// Team data
	/////////////////////////////////////////////
	scReportBeginTeamData(statsReport);
		
	// End the report and set GameStatus
	if (!disconnected)
		scReportEnd(statsReport, gsi_false, SCGameStatus_COMPLETE);
	else
		scReportEnd(statsReport, gsi_false, SCGameStatus_BROKEN);
	
	// Submit the Report
	/////////////////////////////////////////////
	if (SCResult_NO_ERROR != scSubmitReport(interface, statsReport, gsi_false, &gPlayerData.certificate, 
											&gPlayerData.privateData, SubmitReportCallback, SCRACE_TIMEOUT_MS, self))
	{
		MessageBox(@"Failed to submit Single Player Game stats report.");
		return;
	}
}


- (void)countdown
{
	// If report was just recently submitted, reset the submission flag
	///////////////////////////////////////////////////////////////////
	if (reportSent)
		reportSent = gsi_false;
	
	if (hosting)
	{
		int rcode;
		char message[32];
		
		rcode = gtEncode(MSG_COUNTDOWN_TYPE, MSG_COUNTDOWN, message, sizeof(message), countdown);
		assert(rcode != -1);
		NSLog(@"Protocol send MSG_COUNTDOWN_TYPE\n");
		gt2Send(gt2Connection, (const unsigned char*)message, rcode, GT2True);
	}
	
	[gameController countdown: countdown];
}

- (void)hostingCountdown
{
/*	if (hosting)
	{
		alertView.message = @"qwe qwe";
	}
*/
	//[alertView countdown: countdown];
}

- (void)startRace
{
	if(hosting)
	{
		int rcode;
		char buffer[32];
		rcode = gtEncode(MSG_START_RACE_TYPE, MSG_START_RACE, buffer, sizeof(buffer));
		assert(rcode != -1);
		NSLog(@"Protocol send MSG_START_RACE_TYPE\n");
		gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
	}

	// initialize stats markers
	/////////////////////////////////////////////
	win = gsi_false;
	tie = gsi_false;
	disconnected = gsi_false;
	
	localTime = 0;
	remoteTime = 0;
	[gameController raceStarted];
	racing = true;
	start = mach_absolute_time();
}

- (void)addErrorCallback: (qr2_error_t)error errorMessage: (const char*)errmsg
{
	if (alertView != nil) 
	{
		[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex animated: TRUE];
		[alertView release];
		alertView = nil;
	}

	MessageBox([NSString stringWithCString: errmsg encoding: NSASCIIStringEncoding]);
}

- (void)createSessionCallback: (const SCInterfacePtr)interfacePtr httpResult: (GHTTPResult)httpResult result: (SCResult)result
{
	if (httpResult == GHTTPSuccess && result == SCResult_NO_ERROR)
	{
		// Retrieve the Session/Connection ID to be used later
		memcpy(gPlayerData.sessionId, scGetSessionId(interfacePtr), SC_SESSION_GUID_SIZE);
		memcpy(gPlayerData.connectionId, scGetConnectionId(interfacePtr), SC_CONNECTION_GUID_SIZE);
		
		sessionCreated = gsi_true;
		
		OutputDebugString([NSString stringWithFormat: @"[createSessionCB] Session ID: %s\n", gPlayerData.sessionId]);
		OutputDebugString([NSString stringWithFormat: @"[createSessionCB] Connection ID: [%s]\n", gPlayerData.connectionId]);
		
		if( playerCount == 1 )
			scSetReportIntention(interface, gPlayerData.connectionId, gsi_false, &gPlayerData.certificate, &gPlayerData.privateData,
								 SetReportIntentionCallback, SCRACE_TIMEOUT_MS, self);
	}
	else
	{
		OutputDebugString([NSString stringWithFormat: @"[createSessionCB] Error. HTTPResult: %d, SCResult: %d\n", httpResult, result]);
		
		[gameController stopRaceClock];
		MessageBox(@"Error Creating Stats Session");
		[self logout];
	}
}

- (void)setReportIntentionCallback: (const SCInterfacePtr)interfacePtr httpResult: (GHTTPResult)httpResult result: (SCResult)result
{
	if (httpResult == GHTTPSuccess && result == SCResult_NO_ERROR)
	{
		// Retrieve the connection ID to be used later
		/////////////////////////////////////////////////////////
		const char * connectionId = scGetConnectionId(interfacePtr);
		memcpy(gPlayerData.connectionId, connectionId, SC_CONNECTION_GUID_SIZE);
		connIdSet = gsi_true;
		
		OutputDebugString([NSString stringWithFormat: @"[setIntentionCB] Connection ID: [%s]\n", gPlayerData.connectionId]);
	}
	else
	{
		OutputDebugString([NSString stringWithFormat: @"[setIntentionCB] Error. HTTPResult: %d, SCResult: %d\n", httpResult, result]);
		
		MessageBox(@"Error initializing stats system. Login when network connection is up", @"Connection Error");
		[self logout];
		return;
	}
}

- (void)submitReportCallback: (const SCInterfacePtr)interface httpResult: (GHTTPResult)httpResult result: (SCResult)result
{
	if (httpResult != GHTTPSuccess || result != SCResult_NO_ERROR)
	{
		OutputDebugString([NSString stringWithFormat: @"[submitReportCB] Error. HTTPResult: %d, SCResult: %d\n", httpResult, result]);
		
		//MessageBox(@"Error submitting stats report.", @"Connection Error");
		//return;
	}

	reportSent = gsi_true; //mark that we've submitted a report for this session
	
	// Cleanup
	/////////////////////////////////////////////
	scDestroyReport(statsReport);
	statsReport = NULL;
}

- (void)getMyRecordsCallback: (SAKERequestResult) result inputData: (SAKEGetMyRecordsInput*)inputData outputData: (SAKEGetMyRecordsOutput*)outputData
{
	[alertView dismissWithClickedButtonIndex: -1 animated: YES];
	[alertView release];
	alertView = nil;

	[loadRemoteDataTimer invalidate];
	loadRemoteDataTimer = nil;

	if (result != SAKERequestResult_SUCCESS) 
	{
		MessageBox(@"Attempt to retrieve data failed.");
		return;
	}

	if (outputData->mNumRecords == 0) 
	{
		// New user.
		gPlayerData.bestRaceTime = INITIAL_BEST_TIME;
		gPlayerData.spBestRaceTime = INITIAL_BEST_TIME;
		//gPlayerData.lastGameTime = '\0';
		return;
	}

	// Assign vars:
	SAKEField* fields = outputData->mRecords[0];
	
	for (int n = 0; n < inputData->mNumFields; n++) {
		int stat = ATLAS_GET_STAT(fields[n].mName);
		
		switch (stat) {
			case CAREER_WINS:
				gPlayerData.careerWins = fields[n].mValue.mInt;
				break;
				
			case CAREER_LOSSES:
				gPlayerData.careerLosses = fields[n].mValue.mInt;
				break;
				
			case BEST_RACE_TIME:
				gPlayerData.bestRaceTime = fields[n].mValue.mInt;
				break;
				
			case WORST_RACE_TIME:
				gPlayerData.worstRaceTime = fields[n].mValue.mInt;
				break;
				
			case TOTAL_MATCHES:
				gPlayerData.totalMatches = fields[n].mValue.mInt;
				break;
				
			case AVERAGE_RACE_TIME:
				gPlayerData.averageRaceTime = fields[n].mValue.mFloat;
				break;
				
			case CURRENT_WIN_STREAK:
				gPlayerData.matchWinStreak = fields[n].mValue.mInt;
				break;
				
			case CURRENT_LOSS_STREAK:
				gPlayerData.matchLossStreak = fields[n].mValue.mInt;
				break;
				
			case TOTAL_RACE_TIME:
				gPlayerData.totalRaceTime = fields[n].mValue.mInt;
				break;
				
			case CAREER_DISCONNECTS:
				gPlayerData.careerDisconnects = fields[n].mValue.mInt;
				break;
				
			case DISCONNECT_RATE:
				gPlayerData.disconnectRate = fields[n].mValue.mFloat;
				break;
				
			case CAREER_DRAWS:
				gPlayerData.careerDraws = fields[n].mValue.mInt;
				break;
				
			case CURRENT_DRAW_STREAK:
				gPlayerData.matchDrawStreak = fields[n].mValue.mInt;
				break;
				
			case CAREER_LONGEST_WIN_STREAK:
				gPlayerData.careerLongestWinStreak = fields[n].mValue.mInt;
				break;
				
			case CAREER_LONGEST_LOSS_STREAK:
				gPlayerData.careerLongestLossStreak = fields[n].mValue.mInt;
				break;
				
			case CAREER_LONGEST_DRAW_STREAK:
				gPlayerData.careerLongestDrawStreak = fields[n].mValue.mInt;
				break;
				
			case TOTAL_COMPLETE_MATCHES:
				gPlayerData.totalCompleteMatches = fields[n].mValue.mInt;
				break;
				
			case LAST_TIME_PLAYED:
				gPlayerData.lastGameTime = fields[n].mValue.mAsciiString;
				break;

			case SP_AVERAGE_RACE_TIME:
				gPlayerData.spAverageRaceTime = fields[n].mValue.mFloat;
				break;
			
			case SP_BEST_RACE_TIME:
				gPlayerData.spBestRaceTime = fields[n].mValue.mInt;
				break;
				
			case SP_WORST_RACE_TIME:
				gPlayerData.spWorstRaceTime = fields[n].mValue.mInt;
				break;
				
			case SP_TOTAL_PLAYS:
				gPlayerData.spTotalPlays = fields[n].mValue.mInt;
				break;

			case SP_TOTAL_RACE_TIME:
				gPlayerData.spTotalRaceTime = fields[n].mValue.mInt;
				break;				
		}
	}
	
	// Check if player stats from server are more up-to-date than the local stats saved on file
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss Z"];
	
	NSString* aTime = (NSString*)[gPlayerData.playerStatsData objectForKey: lastTimePlayedKey ];

	// if no stat timestamp is found in file, save server data to file 
	if( aTime == nil )
	{
		[self savePlayerStatsToFile ];
		return;
	}	
	
	NSDate* fileDate = [dateFormatter dateFromString: aTime ];
	
	aTime = [NSString stringWithCString: gPlayerData.lastGameTime encoding: NSASCIIStringEncoding];
	// if no stat timestamp is found from server, log and ignore 
	if( aTime == nil )
	{
		NSLog(@"Stats from server does not have a timestamp.");
		return;
	}	
	NSDate* serverDate = [dateFormatter dateFromString: aTime];
	
	NSComparisonResult dateResult = [ fileDate compare: serverDate ];
	if( dateResult == NSOrderedSame || dateResult == NSOrderedAscending )
	{
		// File stats are older or same time as server stats
		// Save server data to file
		NSLog(@"Save server data to file");
		[self savePlayerStatsToFile ];
	}
	else		
	{
		// File stats are more up to date than server stats
		// Use file data for display stats
		NSLog(@"Use file data for display stats");
	}
 
}

- (void)connectedCallback: (GT2Connection)connection result: (GT2Result)result message: (GT2Byte*)message length: (int)length
{
	if (result == GT2Success)
	{
		state = JOIN_EXCHANGE_CERT;
		NSLog(@"connectedCallback  state=JOIN_EXCHANGE_CERT -  (success)");
	}
	else
	{
		state = JOIN_ERROR;
		NSLog(@"connectedCallback state=JOIN_ERROR  (fail)");
		gt2Connection = NULL;
	}
}

- (void)receivedCallback: (GT2Connection)connection message: (GT2Byte*)message length: (int)length reliable: (GT2Bool)reliable
{
	if(!message || !length)
		return;
	
	switch (gtEncodedMessageType((char*)message))
	{
		case MSG_COUNTDOWN_TYPE:
		{
			assert(!hosting);
NSLog(@"receivedCallback(MSG_COUNTDOWN_TYPE)");	
			if(gtDecode(MSG_COUNTDOWN, (char*)message, length, &countdown) == -1)
			{
				state = JOIN_ERROR;
				NSLog(@"state=JOIN_ERROR - MSG_COUNTDOWN_TYPE in receivedCallback");
				return;
			}
			
			if( state == RACING ) 
				[self countdown];
				
			break;
		}
						
		case MSG_START_RACE_TYPE:
NSLog(@"receivedCallback(MSG_START_RACE_TYPE)");	
			assert(!hosting);
			[self startRace];
			break;
			
		case MSG_PROGRESS_TYPE:
NSLog(@"receivedCallback(MSG_PROGRESS_TYPE)");	
			if(racing)
			{
				int progress;
				
				if(gtDecode(MSG_PROGRESS, (char*)message, length, &progress) != -1)
				{
					[gameController remoteProgress: progress];
				}
			}
			break;
			
		case MSG_END_RACE_TYPE:
NSLog(@"receivedCallback(MSG_END_RACE_TYPE)");	
			if(racing)
			{
				uint32_t millis;
				gtDecode(MSG_END_RACE, (char*)message, length, &millis);
				remoteTime = millis * millis_to_absolute;
			}
			break;
			
		case MSG_TO_CLIENT_CERT_TYPE:
		{
NSLog(@"receivedCallback(MSG_TO_CLIENT_CERT_TYPE)");	
			char remoteCert[512];
			int remoteCertLen = 512;
			int pid;
			
			if(gtDecode(MSG_TO_CLIENT_CERT, (char*)message, length, &pid, remoteCert, &remoteCertLen) == -1)
			{
				state = JOIN_ERROR;
				NSLog(@"state=JOIN_ERROR - MSG_TO_CLIENT_CERT_TYPE in receivedCallback");
				return;
			}
			
			// Parse the certificate
			/////////////////////////////////////////////////////////
			wsLoginCertReadBinary(&remoteCertificate, remoteCert, remoteCertLen);
			
			// Build up and send the certificate response
			/////////////////////////////////////////////////////////
			char buffer[520];
			char cert[512];
			int rcode;
			gsi_u32 certLen;
			
			wsLoginCertWriteBinary(&gPlayerData.certificate, cert, sizeof(cert), &certLen);
			
			rcode = gtEncode(MSG_TO_HOST_CERT_TYPE, MSG_TO_HOST_CERT, buffer, sizeof(buffer), gPlayerData.profileId, cert, certLen);
			assert(rcode != -1);
			NSLog(@"Protocol send MSG_TO_HOST_CERT_TYPE\n");
			gt2Send(connection, (const unsigned char*)buffer, rcode, GT2True);
			
			state = JOIN_VERIFY_CERT;
			NSLog(@"receivedCallback state=JOIN_VERIFY_CERT");
			break;
		}

		case MSG_TO_HOST_CERT_TYPE:
		{
NSLog(@"receivedCallback(MSG_TO_HOST_CERT_TYPE)");	
			char remoteCert[512];
			int remoteCertLen = 512;
			int pid;
			
			if(gtDecode(MSG_TO_HOST_CERT, (char*)message, length, &pid, remoteCert, &remoteCertLen) == -1)
			{
				state = HOST_ERROR;
				NSLog(@"MSG_TO_HOST_CERT_TYPE - state=HOST_ERROR");
				return;
			}
			
			// Parse the certificate
			/////////////////////////////////////////////////////////
			wsLoginCertReadBinary(&remoteCertificate, remoteCert, remoteCertLen);
			
			state = HOST_VERIFY_CERT;
			NSLog(@"state=JOIN_VERIFY_CERT");
			break;
		}

		case MSG_TO_CLIENT_CERTVERIFIED_TYPE:
NSLog(@"receivedCallback(MSG_TO_CLIENT_CERTVERIFIED_TYPE)");	
			char buffer[32];
			int rcode;
			assert(!hosting);
			rcode = gtEncode(MSG_TO_HOST_CERTVERIFIED_TYPE, MSG_TO_HOST_CERTVERIFIED, buffer, sizeof(buffer));
			assert(rcode != -1);
			NSLog(@"Protocol send MSG_TO_HOST_CERTVERIFIED_TYPE\n");
			gt2Send(gt2Connection, (const unsigned char*)buffer, rcode, GT2True);
			break;

		case MSG_TO_HOST_CERTVERIFIED_TYPE:
NSLog(@"receivedCallback(MSG_TO_HOST_CERTVERIFIED_TYPE)");	
			assert(hosting);
			state = HOST_EXCHANGE_KEYS;
			NSLog(@"state=HOST_EXCHANGE_KEYS");
			break;

		case MSG_TO_CLIENT_KEYS_TYPE:
		{
NSLog(@"receivedCallback(MSG_TO_CLIENT_KEYS_TYPE)");	
			SCPeerKeyExchangeMsg recvMsg;
			int recvMsgLen = GS_CRYPT_RSA_BYTE_SIZE;
			
			if(gtDecode(MSG_TO_CLIENT_KEYS, (char*)message, length, recvMsg, &recvMsgLen) == -1)
			{
				state = JOIN_ERROR;
				NSLog(@"state=JOIN_ERROR - MSG_TO_CLIENT_KEYS_TYPE in receivedCallback");
				return;
			}
			
			// Stop Join countdown
			[self stopJoiningCountdownTimer];	// if running
			
			// Receiving player should parse the cipher key out of it.
			//   - decrypting the msg requires the local player's private data
			/////////////////////////////////////////////////////////
			scPeerCipherParseKeyExchangeMsg(&gPlayerData.certificate, &gPlayerData.privateData, 
											recvMsg, &gPlayerData.peerSendCipher);
			
			
			// Send response to host
			/////////////////////////////////////////////////////////
			char buffer[512];
			int rcode;
			SCPeerKeyExchangeMsg exchangeMsg;
			
			scPeerCipherInit(&gPlayerData.certificate, &gPlayerData.peerRecvCipher); 	
			scPeerCipherCreateKeyExchangeMsg(&remoteCertificate, &gPlayerData.peerRecvCipher, exchangeMsg);
			
			// Now send the key to the other player
			/////////////////////////////////////////////////////////
			rcode = gtEncode(MSG_TO_HOST_KEYS_TYPE, MSG_TO_HOST_KEYS, buffer, sizeof(buffer), exchangeMsg, GS_CRYPT_RSA_BYTE_SIZE);
			assert(rcode != -1);
			NSLog(@"Protocol send MSG_TO_HOST_KEYS_TYPE\n");
			gt2Send(connection, (const unsigned char*)buffer, rcode, GT2True);
			
			state = JOIN_CONNECTED;
			NSLog(@"state=JOIN_CONNECTED");
			break;
		}

		case MSG_TO_HOST_KEYS_TYPE:
		{
NSLog(@"receivedCallback(MSG_TO_HOST_KEYS_TYPE)");	
			SCPeerKeyExchangeMsg exchangeMsg;
			int exchangeMsgLen = GS_CRYPT_RSA_BYTE_SIZE;
			
			if(gtDecode(MSG_TO_HOST_KEYS, (char*)message, length, exchangeMsg, &exchangeMsgLen) == -1)
			{
				state = HOST_ERROR;
				NSLog(@"state=HOST_ERROR");
				return;
			}
			
			// release hosting countdown
			[self stopHostingCountdownTimer];	// if running

			// Receiving player should parse the cipher key out of it.
			//   - decrypting the msg requires the local player's private data
			/////////////////////////////////////////////////////////
			scPeerCipherParseKeyExchangeMsg(&gPlayerData.certificate, &gPlayerData.privateData, 
											exchangeMsg, &gPlayerData.peerSendCipher);
			
			
			state = HOST_CONNECTED;
			NSLog(@"receivedCallback(MSG_TO_HOST_KEYS_TYPE)  state=HOST_CONNECTED");
			
			break;
		}

		case MSG_SESSION_ID_TYPE:
		{
NSLog(@"receivedCallback(MSG_SESSION_ID_TYPE)");	
			assert(!hosting);
			char sessionCrypt[SC_SESSION_GUID_SIZE];
			char connCrypt[SC_CONNECTION_GUID_SIZE];
			int sidLen = SC_SESSION_GUID_SIZE;
			int ccidLen = SC_CONNECTION_GUID_SIZE;
			
			
			// Client decodes sessionID / remote connID
			/////////////////////////////////////////////////////////
			if(gtDecode(MSG_SESSION_ID, (char*)message, length, sessionCrypt, &sidLen, connCrypt, &ccidLen) == -1)
			{
				state = JOIN_ERROR;
				NSLog(@"state=JOIN_ERROR - MSG_SESSION_ID_TYPE in receivedCallback");
				return;
			}
			
			OutputDebugString([NSString stringWithFormat: @"[MSG_SESSION_ID_TYPE] sessionCrypt: %.40s\n", sessionCrypt]);
			OutputDebugString([NSString stringWithFormat: @"[MSG_SESSION_ID_TYPE] connCrypt: %.40s\n", connCrypt]);
			
			// Decrypt the sessionID / remote connID
			/////////////////////////////////////////////////////////
			scPeerCipherDecryptBufferIV(&gPlayerData.peerRecvCipher, 1, (gsi_u8*)sessionCrypt, SC_SESSION_GUID_SIZE);
			memcpy(gPlayerData.sessionId, sessionCrypt, SC_SESSION_GUID_SIZE);
			
			scPeerCipherDecryptBufferIV(&gPlayerData.peerRecvCipher, 2, (gsi_u8*)connCrypt, SC_CONNECTION_GUID_SIZE);
			memcpy(remoteConnId, connCrypt, SC_CONNECTION_GUID_SIZE);
			
			
			OutputDebugString([NSString stringWithFormat: @"[MSG_SESSION_ID_TYPE] mSessionId: %s\n", gPlayerData.sessionId]);
			OutputDebugString([NSString stringWithFormat: @"[MSG_SESSION_ID_TYPE] remoteConnId: %s\n", remoteConnId]);
			
			
			// Joining player sets the session ID and the report intention
			/////////////////////////////////////////////////////////
			scSetSessionId(gPlayerData.statsInterface, gPlayerData.sessionId);
			scSetReportIntention(gPlayerData.statsInterface, gPlayerData.connectionId, SCRACE_AUTHORITATIVE, &gPlayerData.certificate, &gPlayerData.privateData,
								 SetReportIntentionCallback, SCRACE_TIMEOUT_MS, self);
			
			state = JOIN_SEND_CONNID;
			NSLog(@"state=JOIN_SEND_CONNID");
			break;
		}

		case MSG_CONNECTION_ID_TYPE:
		{
NSLog(@"receivedCallback(MSG_CONNECTION_ID_TYPE)");	
			assert(hosting);
			char connCrypt[SC_CONNECTION_GUID_SIZE];
			int ccidLen = SC_CONNECTION_GUID_SIZE;
			
			// Hosting player decodes the remote conn ID for use in reporting
			/////////////////////////////////////////////////////////
			if(gtDecode(MSG_CONNECTION_ID, (char*)message, length, connCrypt, &ccidLen) == -1)
			{
				state = HOST_ERROR;
				NSLog(@"MSG_CONNECTION_ID_TYPE - state=HOST_ERROR");
				return;
			}
			
			OutputDebugString([NSString stringWithFormat: @"[MSG_CONNECTION_ID_TYPE] connCrypt: %.40s\n", connCrypt]);
			
			
			// Decrypt the remote conn ID
			/////////////////////////////////////////////////////////
			scPeerCipherDecryptBufferIV(&gPlayerData.peerRecvCipher, 1, (gsi_u8*)connCrypt, SC_CONNECTION_GUID_SIZE);
			memcpy(remoteConnId, connCrypt, SC_CONNECTION_GUID_SIZE);
			
			
			OutputDebugString([NSString stringWithFormat: @"[MSG_CONNECTION_ID_TYPE] remoteConnId: %s\n", remoteConnId]);
			
			// at this point all of our exchanges are complete - ready for game start
			/////////////////////////////////////////////////////////
			break;
		}

		case MSG_REPLAY_TYPE:	
NSLog(@"receivedCallback(MSG_REPLAY_TYPE) state=%d", state);
			assert(state == FINISHED_RACE);
			if (currentAlert == ALERT_REPLAY) 
			{
				// Remote player agreed to a replay.
				[alertView dismissWithClickedButtonIndex: -1 animated: YES];
				[alertView release];
				alertView = nil;
				[self stopHostingCountdownTimer];
				
				[self setupMatch: hosting];
				[navController popToViewController: gameController animated: YES];
			}
			else 
			{
				// Remote player asked for a replay.
				alertView = [[UIAlertView alloc] initWithTitle: @"You have been challenged to a rematch!" message: @"Accept?" delegate: self cancelButtonTitle: @"No" otherButtonTitles: @"Yes", nil];
				[alertView show];
				currentAlert = ALERT_REPLAY_CONFIRM;
			}
			break;

		case MSG_REPLAY_CANCELED_TYPE:
NSLog(@"receivedCallback(MSG_REPLAY_CANCELED_TYPE)");	
			[self stopHostingCountdownTimer];
			
			assert(state == FINISHED_RACE);
			if (currentAlert == ALERT_REPLAY) {
				[alertView dismissWithClickedButtonIndex: -1 animated: YES];
				MessageBox(@"Your replay request was declined.");
			}
			else if (currentAlert == ALERT_REPLAY_CONFIRM) {
				[alertView dismissWithClickedButtonIndex: -1 animated: YES];
				MessageBox(@"Your opponent canceled the replay request.");
			}
	}
}

// Called when the connection to remote player is closed
- (void)closedCallback: (GT2Connection)connection reason: (GT2CloseReason)reason
{
	// Logout triggers this function when it's a local close
	// so we need to make sure we don't loop
	NSLog(@"closedCallback reason = %d  state=%d", reason, state );
	
	// Stop host countdown
	[self stopHostingCountdownTimer];
	
	if (reason != GT2LocalClose)
	{
		NSLog(@"closedCallback reason=%d - NOT GT2LocalClose", reason);

		if( state == RACING )
		{
			// stop the game-start countdown timer
			if (countdownTimer != nil) 
			{
				[countdownTimer invalidate];
				countdownTimer = nil;
			}

			// stop game race clock
			[gameController stopRaceClock];
			
			// If the connection was closed remotely, or connection errors occured
			// we should report stats anyways to report disconnects
			MessageBox(@"Opponent canceled match");
			
			[self stopHostingServer];
			disconnected = gsi_true;
			
			[self reportTwoPlayerStats];
			[self saveTwoPlayerGameStatsToFile];
			
			MatchmakingController* matchmakingController = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
			[navController popToViewController: matchmakingController animated: YES];
		}
		else if ( state == HOST_CONNECTED )
		{
			[alertView dismissWithClickedButtonIndex: -1 animated: YES];
			[alertView release];
			alertView = nil;
			
			[self stopJoiningCountdownTimer];
			[self stopHostingServer];
			disconnected = gsi_true;
			
			[self reportTwoPlayerStats];
			[self saveTwoPlayerGameStatsToFile];
			
			// set state so that onThinkTimer does not go to game page
			state = JOIN_CHOOSESERVER;
			
			//MatchmakingController* matchmakingController = (MatchmakingController*)[self findExistingControllerOfType: [MatchmakingController class]];
			//[navController popToViewController: matchmakingController animated: YES];
			
			MessageBox(@"Opponent canceled" );
		}

		else 
		{	
			NSLog(@"closedCallback currentAlert=%d - NOT GT2LocalClose", currentAlert);

			GameResultsController* resultsController = (GameResultsController*)[self findExistingControllerOfType: [GameResultsController class]];
			if (resultsController != nil) 
			{
				[resultsController connectionDropped];
			}

			if ( state != FINISHED_RACE )
			{
				switch (currentAlert) 
				{
					case ALERT_REPLAY:
					case ALERT_REPLAY_CONFIRM:
						[alertView dismissWithClickedButtonIndex: -1 animated: YES];
						[alertView release];
						alertView = nil;
						
						[self stopHostingServer];
						disconnected = gsi_true;

						MessageBox(@"The connection to the other player was terminated.");
						break;
						
					default:
						// stop game race clock
						[gameController stopRaceClock];

						[alertView dismissWithClickedButtonIndex: -1 animated: YES];
						[alertView release];
						alertView = nil;
						
						[self stopHostingServer];
						disconnected = gsi_true;
						
						MessageBox(@"Opponent quit race" );
						[self returnToMatchMaking];
						break;
				}
			}
		}
	}
	
	
	racing = false;
	if (gt2Connection)
	{
		gt2CloseConnection(gt2Connection);
		gt2Connection = NULL;
	}
	if (gt2Socket)
	{
		GT2Socket tempsocket = gt2Socket;
		gt2Socket = NULL; // prevent recursion
		NSLog(@"closeCallback:  null & close gt2 socket");
		gt2CloseSocket(tempsocket);
	}
}

- (void)connectAttemptCallback: (GT2Socket)listener connection: (GT2Connection)connection ip: (unsigned int)ip port: (unsigned short)port latency: (int)latency message: (GT2Byte*)message length: (int)length
{
	NSLog(@"start connectAttemptCallback");
	
	int pid = 0;
	char nick[128];
	
	// Only allow one connection.
	/////////////////////////////
	if(gt2Connection)
	{
		gt2Reject(connection, NULL, 0);
		return;
	}
	
	// Decode the pid.
	//////////////////
	if(message && length)
	{
		if(gtDecodeNoType(MSG_CONNECT, (char*)message, length, &pid, nick) == -1)
			pid = 0;
	}
	
	// If we didn't/couldn't get the pid, reject the attempt.
	/////////////////////////////////////////////////////////
	if(!pid)
	{
		gt2Reject(connection, NULL, 0);
		return;
	}
	
	// Accept the connection.
	/////////////////////////
	GT2ConnectionCallbacks callbacks = {
		NULL,
		ReceivedCallback,
		ClosedCallback,
		NULL
	};
	
	if(!gt2Accept(connection, &callbacks))
		return;
	
	// Set some states.
	///////////////////
	gt2Connection = connection;
	gt2SetConnectionData(gt2Connection, self);
	state = HOST_EXCHANGE_CERT; //once connected, exchange certifications
	NSLog(@"connectAttemptCallback  state=HOST_EXCHANGE_CERT");
	
	if( hosting )
	{		
		// remove countdown pop-up
		[alertView dismissWithClickedButtonIndex: 0 animated: YES];
		[alertView release];
		alertView = nil;
		
		// stop hosting countdown
		[self stopHostingCountdownTimer];
		
		// show that we are connecting
		alertView = [[UIAlertView alloc] initWithTitle: @"Initiating race with player" message: @"30 seconds" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
		[alertView show];
		currentAlert = ALERT_CONNECTING;  //NEW ???
		
		[self startJoiningCountdownTimer];
	}
	
	connected = true;
	qr2_send_statechanged(NULL);
}


// Called by NN SDK for host only
- (void)natnegCallback: (int)cookie
{
	if( hosting )
	{		
		// remove countdown pop-up
//		[alertView dismissWithClickedButtonIndex: 0 animated: YES];
		[alertView dismissWithClickedButtonIndex: -1 animated: YES];
		[alertView release];
		alertView = nil;
		
		// stop hosting countdown
		[self stopHostingCountdownTimer];
		
		// show that we are connecting
		alertView = [[UIAlertView alloc] initWithTitle: @"Connecting to player" message: @"30 seconds" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
		[alertView show];
		currentAlert = ALERT_CONNECTING;  //NEW ???
		
		[self startJoiningCountdownTimer];	
	}
	
	NNBeginNegotiationWithSocket(gt2GetSocketSOCKET(gt2Socket), cookie, NN_SERVER, NNProgressCallback, NNCompletedCallback, self);
}

- (void)natnegCompletedCallback: (NegotiateResult)result socket: (SOCKET)gamesocket remoteaddr: (sockaddr_in*)remoteaddr
{
	NSLog(@"natnegCompletedCallback  start  (state=%d) (result=%d)", state, result);

	if (state == HOST_LISTENING) 
	{
		NSLog(@"natnegCompletedCallback host waiting for client\n");
		// Server side doesn't have to do anything here.
		return;
	}
	
	// Close the Connection popup  didDismissWithButtonIndex
	[alertView dismissWithClickedButtonIndex: -1 animated: YES];
	[alertView release];
	alertView = nil;

	if (result != nr_success) 
	{
		switch (result) 
		{
			case nr_inittimeout:
				MessageBox(@"Unable to communicate with NAT Negotiation server.");
				break;

			case nr_pingtimeout:
				MessageBox(@"Couldn't establish a direct connection to the other player.");
				break;

			case nr_deadbeatpartner:
				MessageBox(@"The other player did not register with the NAT Negotiation Server.");
				break;

			default:
				MessageBox(@"An error occurred connecting to the remote player.");
				break;
		}

		return;
	}

	NSString* remoteAddress = [NSString stringWithFormat: @"%s:%d", inet_ntoa(remoteaddr->sin_addr), ntohs(remoteaddr->sin_port)];
	NSLog(@"remote Address = %s", remoteAddress);
	
	[self startMultiPlayerGame: [remoteAddress cStringUsingEncoding: NSASCIIStringEncoding]];
}


// System needs more memory
// Remove any data we can retrieve from server
- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	MessageBox(@"System is running low on memory. Logging out", @"Low memory");
	[self logout];
	NSLog( @"WARNING: applicationDidReceiveMemoryWarning" );
}

@end


void ServerKeyCallback(int keyid, qr2_buffer_t outbuf, void* userdata)
{
	switch (keyid) 
	{
		case HOSTNAME_KEY:
			qr2_buffer_addA(outbuf, [gPlayerData.uniquenick cStringUsingEncoding: NSASCIIStringEncoding]);
			break;

		case GAMEVER_KEY:
			qr2_buffer_addA(outbuf, SCRACE_VERSION_STR);
			break;

		case HOSTPORT_KEY:
			qr2_buffer_add_int(outbuf, HOST_PORT2);
			break;

		case GAMEMODE_KEY:
		{
			switch([ appDelegate state ])
			{
				RACING:
					qr2_buffer_addA(outbuf, "closedplaying");
					break;
					
				HOST_CONNECTED:
				JOIN_CONNECTED:
					qr2_buffer_addA(outbuf, "closedwaiting");  // accept no more players
					break;
				
				HOST_LISTENING:
				JOIN_ERROR:
					qr2_buffer_addA(outbuf, "openwaiting");
					break;
					
				default:
					qr2_buffer_addA(outbuf, "openwaiting"); // hosting and waiting
					break;
			}
			break;
		}

		case TIMEELAPSED_KEY:
			qr2_buffer_add_int(outbuf, 0);
			break;

		case NUMPLAYERS_KEY:
			qr2_buffer_add_int(outbuf, ( [appDelegate connected] ? 2 : 1) );
			break;

		case MAXPLAYERS_KEY:
			qr2_buffer_add_int(outbuf, 2);
			break;

		default:
			qr2_buffer_addA(outbuf, "");
			break;
	}
}

void PlayerKeyCallback(int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
	switch (keyid) 
	{
		case PLAYER__KEY:
			qr2_buffer_addA(outbuf, [gPlayerData.uniquenick cStringUsingEncoding: NSASCIIStringEncoding]);
			break;

		case PING__KEY:
			qr2_buffer_add_int(outbuf, 0);
			break;

		case PID__KEY:
			qr2_buffer_add_int(outbuf, gPlayerData.profileId);
			break;

		default:
			qr2_buffer_addA(outbuf, "");
			break;
	}
}

void TeamKeyCallback(int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
	qr2_buffer_addA(outbuf, "");
}

int CountCallback(qr2_key_type keytype, void* userdata)
{
	if( keytype == key_player )
		return 1;

	return 0;
}

void KeyListCallback(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void* userdata)
{
	switch (keytype) 
	{
		case key_server:
			qr2_keybuffer_add(keybuffer, HOSTNAME_KEY);
			qr2_keybuffer_add(keybuffer, GAMEVER_KEY);
			qr2_keybuffer_add(keybuffer, HOSTPORT_KEY);
			qr2_keybuffer_add(keybuffer, GAMEMODE_KEY);
			qr2_keybuffer_add(keybuffer, TIMEELAPSED_KEY);
			qr2_keybuffer_add(keybuffer, NUMPLAYERS_KEY);
			qr2_keybuffer_add(keybuffer, MAXPLAYERS_KEY);
			break;

		case key_player:
			qr2_keybuffer_add(keybuffer, PLAYER__KEY);
			qr2_keybuffer_add(keybuffer, PING__KEY);
			qr2_keybuffer_add(keybuffer, PID__KEY);
			break;

		case key_team:
		case key_type_count:
			break;
	}
}

void AddErrorCallback(qr2_error_t error, char* errmsg, void* userdata)
{
	[(AppDelegate*)userdata addErrorCallback: error errorMessage: errmsg];
}

void NatNegCallback(int cookie, void* userData)
{	
	[(AppDelegate*)userData natnegCallback: cookie];
}

void CreateSessionCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData)
{
	[(AppDelegate*)userData createSessionCallback: interface httpResult: httpResult result: result];
}

void SetReportIntentionCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData)
{
	[(AppDelegate*)userData setReportIntentionCallback: interface httpResult: httpResult result: result];
}

void SubmitReportCallback(const SCInterfacePtr interface, GHTTPResult httpResult, SCResult result, void* userData)
{
	[(AppDelegate*)userData submitReportCallback: interface httpResult: httpResult result: result];
}

void GetMyRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, SAKEGetMyRecordsInput* inputData, SAKEGetMyRecordsOutput* outputData, void* userData)
{
	[(AppDelegate*)userData getMyRecordsCallback: result  inputData: inputData  outputData: outputData];
}

void ConnectedCallback(GT2Connection connection, GT2Result result, GT2Byte* message, int len)
{
	AppDelegate* delegate = (AppDelegate*)gt2GetConnectionData(connection);
	[delegate connectedCallback: connection result: result message: message length: len];
}

void ReceivedCallback(GT2Connection connection, GT2Byte* message, int len, GT2Bool reliable)
{
	AppDelegate* delegate = (AppDelegate*)gt2GetConnectionData(connection);
	[delegate receivedCallback: connection message: message length: len reliable: reliable];
}

void ClosedCallback(GT2Connection connection, GT2CloseReason reason)
{
	AppDelegate* delegate = (AppDelegate*)gt2GetConnectionData(connection);
	[delegate closedCallback: connection reason: reason];
}

void ConnectAttemptCallback(GT2Socket listener, GT2Connection connection, unsigned int ip, unsigned short port, int latency, GT2Byte* message, int len)
{
	AppDelegate* delegate = (AppDelegate*)gt2GetSocketData(listener);
	[delegate connectAttemptCallback: listener connection: connection ip: ip port: port latency: latency message: message length: len];
}

void SocketErrorCallback(GT2Socket socket)
{
}

GT2Bool UnrecognizedMessageCallback(GT2Socket socket, unsigned int ip, unsigned short port, GT2Byte* message, int len)
{
	static GT2Byte qrHeader[] = { QR_MAGIC_1, QR_MAGIC_2 };
	static GT2Byte nnHeader[] = { NN_MAGIC_0, NN_MAGIC_1, NN_MAGIC_2, NN_MAGIC_3, NN_MAGIC_4, NN_MAGIC_5 };

	if (memcmp(message, qrHeader, sizeof(qrHeader)) == 0) 
	{
		sockaddr_in address = { sizeof(sockaddr_in), AF_INET, htons(port), { ip }, { 0 } };
		qr2_parse_queryA(NULL, (char*)message, len, (sockaddr*)&address);
		return GT2True;
	}
	else if (memcmp(message, nnHeader, sizeof(nnHeader)) == 0) 
	{
		sockaddr_in address = { sizeof(sockaddr_in), AF_INET, htons(port), { ip }, { 0 } };
		NNProcessData((char*)message, len, &address);
		return GT2True;
	}

	NSLog(@"UnrecognizedMessageCallback: unknown message type with header[%d, %d]", message[0], message[1]);
	return GT2False;
}

void NNDetectionCallback(gsi_bool success, NAT nat)
{
	int charsWritten;
	char aString[256];
	
	NSLog(@"****************************************************************************\n");
	if(success == gsi_false)
	{
		NSLog(@"NNDetectionCallback: Failed to fully detect the NAT.  Please try the detection again.\n");
		return;
	}
	
	memset(aString, 0, 255);
	charsWritten = sprintf(aString, "NNDetectionCallback: The detected NAT is a ");
	switch(nat.natType)
	{
		case no_nat:
			charsWritten = sprintf(aString, "NNDetectionCallback: No NAT detected.");
			break;
		case firewall_only:
			charsWritten = sprintf(aString, "NNDetectionCallback: No NAT detected, but firewall may be present.");
			break;
		case full_cone:
			charsWritten += sprintf(aString + charsWritten, "full cone ");
			break;
		case restricted_cone:
			charsWritten += sprintf(aString + charsWritten, "restricted cone ");
			break;
		case port_restricted_cone:
			charsWritten += sprintf(aString + charsWritten, "port restricted cone ");
			break;
		case symmetric:
			charsWritten += sprintf(aString + charsWritten, "symmetric ");
			break;
		case unknown:
		default:
			charsWritten = sprintf(aString, "NNDetectionCallback: Unknown NAT type detected ");
			break;
	}
	
	if(nat.natType != no_nat && nat.natType != firewall_only)
		switch(nat.mappingScheme)
	{
		case private_as_public:
			charsWritten += sprintf(aString + charsWritten, "and is using the private port as the public port.");
			break;
		case consistent_port:
			charsWritten += sprintf(aString + charsWritten, "and is using the same public port for all requests from the same private port.");
			break;
		case incremental:
			charsWritten += sprintf(aString + charsWritten, "and is using an incremental port mapping scheme.");
			break;
		case mixed:
			charsWritten += sprintf(aString + charsWritten, "and is using a mixed port mapping scheme.");
			break;
		case unrecognized:
		default:
			charsWritten += sprintf(aString + charsWritten, "and is using an unrecognized port mapping scheme.");
			break;
	}
	
	charsWritten += sprintf(aString + charsWritten, "\n");
	
	printf("NNDetectionCallback: [%s]", aString );
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Misc, GSIDebugLevel_Notice, aString);
}


void NNProgressCallback(NegotiateState state, void* userdata)
{
	NSLog(@"NN progress status: %d", (int) state );
}

void NNCompletedCallback(NegotiateResult result, SOCKET gamesocket, sockaddr_in* remoteaddr, void* userdata)
{
	AppDelegate* delegate = (AppDelegate*)userdata;
	[delegate natnegCompletedCallback: result socket: gamesocket remoteaddr: remoteaddr];
}

// C callback bridges to Objective C
static void SearchForLeadersTimeRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData)
{
	[(AppDelegate*)userData searchForLeadersTimeRecordsCallback: sake request: request result: result inputData: inputData outputData: outputData];
}

static void SearchForTop5TimeRecordsCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData)
{
	[(AppDelegate*)userData searchForTop5TimeRecordsCallback: sake request: request result: result inputData: inputData outputData: outputData];
}

static void GetMyLeaderPositionCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData)
{
	[(AppDelegate*)userData getMyLeaderPositionCallback: sake request: request result: result inputData: inputData outputData: outputData];
}
