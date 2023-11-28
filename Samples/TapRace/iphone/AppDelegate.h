// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import <MapKit/MapKit.h>
#import <SystemConfiguration/SCNetworkReachability.h>

#import "gt2/gt2.h"
#import "sc/sc.h"
#import "gp/gp.h"
#import "serverbrowsing/sb_serverbrowsing.h"
#import "sake/sake.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// global defines used in the sample

#define SCRACE_GAMENAME		"taprace"
#define SCRACE_SECRETKEY	"xJRENu"
#define SCRACE_GAMEID		2431
#define SCRACE_PRODUCTID	11675
#define SCRACE_VERSION		1.3
#define SCRACE_VERSION_STR	"1.3"


#define LEADERBOARD_INVALID		0
#define LEADERBOARD_BY_TIME		1
#define LEADERBOARD_BY_BUDDIES	2
#define LEADERBOARD_BY_LOCATION 3

#define MAX_BUDDIES		100
#define MAX_LEADERS		25
#define MAX_MAPPOINTS	100
#define MAX_SERVERS		20
#define TAP_POLL_TIME	0.01 // seconds 
#define INITIAL_BEST_TIME 99999  // milliseconds

#define THUMBNAIL_WIDTH			96
#define THUMBNAIL_HEIGHT		96
#define FULLSIZE_IMAGE_WIDTH	320
#define FULLSIZE_IMAGE_HEIGHT	320

#define PLAYER1		0
#define PLAYER2		1

// define for debugging....
#undef USE_DUMMY_LEADERBOARD_DATA

@class AppDelegate;
@class GameController;
@class LeaderboardObject;

typedef struct PlayerData PlayerData;

extern char				raceDateTimeString[256];
extern PlayerData		gPlayerData;
extern PlayerData		gOpponentData;
//extern LeaderboardObject gOpponentData;		// current opponent
extern int				gLeaderboardType;		// type of leaderboard data
extern int				gLeaderboardProfileIndex;	// index of currently displayed profile
extern NSMutableArray*	gLeaderboardData;		// most recent leaderboard download (array of LeaderboardObject's)
extern AppDelegate*	appDelegate;
extern bool sdkInitialized;
extern bool lostConnection;  // true when we lose network connectivity
extern bool userKickedOut;		// set to true if same user logs into another device


@interface AppDelegate : NSObject <UIApplicationDelegate, UINavigationControllerDelegate, UIAlertViewDelegate>
{
	IBOutlet UIWindow* window;
	IBOutlet UINavigationController* navController;

	NSTimer* loadRemoteDataTimer;
	NSTimer* thinkTimer;
	NSTimer* countdownTimer;
	NSTimer* hostingCountdownTimer;
	NSTimer* joiningCountdownTimer;
	UIAlertView* alertView;

	GameController* gameController;

	GT2Socket gt2Socket;
	GT2Connection gt2Connection;
	SCInterfacePtr interface;
	ServerBrowser serverBrowser;
	SAKE sake;

	GSLoginCertificate remoteCertificate;
	gsi_u8 remoteConnId[SC_CONNECTION_GUID_SIZE];
	SCReportPtr statsReport;

	uint64_t remoteTime;
	uint64_t localTime;
	uint64_t start;

	int state;
	int currentAlert;
	int playerCount;
	int countdown;
	int hostingCountdown;
	int joiningCountdown;
	int natnegCookie;
	bool connected;		// host connected to client 
	bool fromMenu;
	
	bool hosting;
	bool racing;
	bool win;
	bool tie;
	bool disconnected;
	bool reportSent;
	bool sessionCreated;
	bool connIdSet;
	
}

@property(nonatomic) ServerBrowser serverBrowser;
@property(nonatomic, readonly) SAKE sake;
@property bool connected;
@property(nonatomic, readonly) int state;	
@property bool fromMenu;
@property(nonatomic, readonly) uint64_t start;
@property(nonatomic, readonly) uint64_t localTime;
@property(nonatomic, readonly) uint64_t remoteTime;

- (void)startThinkTimer;
- (void)stopThinkTimer;
- (void)delayedNetworkConnectionErrorMessage;

- (void)loggedIn;
- (void)kickedOut;
- (bool)isUserKickedOut;
- (void)logout;
- (BOOL)isNetworkAvailableFlags:(SCNetworkReachabilityFlags *)outFlags;

- (bool)joinGame: (unsigned int)ip port: (unsigned short)port useNatNeg: (bool)useNatNeg;
- (bool)hostGame;


- (void)stopHostingServer;
- (void)initGame;
- (void)initSake;
- (void)findGame;
- (void)startSinglePlayerGame;
- (void)startMultiPlayerGame: (const char*)remoteAddress;
- (void)startCountdown;
- (void)startHostingCountdownTimer;
- (void)stopHostingCountdownTimer;
- (void)startJoiningCountdownTimer;
- (void)stopJoiningCountdownTimer;
- (void)restartGame;
- (void)finishedGame;
- (void)returnToMenu;
- (void)returnToMatchMaking;
- (bool)isTwoPlayerGame;
- (void)savePlayerStatsToFile;

- (void)showAboutInfo: (id) sender;
- (void)flipsideViewControllerDidFinish:(id) sender; 
- (void)showLeaderboards;
- (void)showLeadersByTime;
- (void)showTop5LeadersByTime;
- (void)showLeadersByBuddies;
- (void)showLeadersByLocation;
- (void)showUser: (int)index;
- (void)deleteLeaderboards;
- (void)deleteBlockedImagesData;
- (void)removeFalseLeaders;
- (bool)imageIsBlocked: (int) pictureId;
- (void)getMyLeaderPosition;

- (UIViewController*)findExistingControllerOfType: (Class)type;
- (UIViewController*)pushNewControllerOfType: (Class)type nib: (NSString*)nibName;

@end

//*** indicates is used on player stats screens
struct PlayerData
{
	// "Normal" game data
	NSString* uniquenick;			// player name displayed
	gsi_u32	profileId;				// player ID
	gsi_u32	pictureFileId;			// picture fileId
	int		rank;					// player rank
	UIImage *thumbNail;				// picture of player (not from comrade)
	UIImage *fullsizeImage;			// bigger picture of player (not from comrade)
	CLLocation *playerLocation;		// lat/long of player, used for map view

	// login/connection info
	char loginTicket[GP_LOGIN_TICKET_LEN];
	GSLoginCertificate certificate;
	GSLoginPrivateData privateData;
	SCPeerCipher peerSendCipher; // for fast encryption
	SCPeerCipher peerRecvCipher; // for fast decryption

	// local data
	NSMutableDictionary* playerStatsData;	// Player stats data (stored locally)
	NSString* playerDataPath;				// Player data directory path on device
	NSMutableArray* blockedImagesData;				// player's blocked image list data (stored locally)

	// Stats connection data (stored remotely)
	SCInterfacePtr statsInterface;
	gsi_u8  sessionId[SC_SESSION_GUID_SIZE];
	gsi_u8  connectionId[SC_CONNECTION_GUID_SIZE];
	gsi_u8  statsAuthdata[16];

	// 2 player game stats 
	int careerWins;					// # of 2 player won
	int careerLosses;				// # of 2 player matches lost
	int bestRaceTime;				// Player's best 2 player matches race time (milliseconds).
	int worstRaceTime;				// Player's 2 player matches worst race time (milliseconds).
	int totalMatches;				// # of 2 player matches played
	float averageRaceTime;			// Player's average race time per match (milliseconds/match).
	int matchWinStreak;				// # of consecutive 2 player games won
	int matchLossStreak;			// # of consecutive 2 player games lost
	int totalRaceTime;				// Player's total race time for all matches (milliseconds).
	int careerDisconnects;			// Player's total number of times disconnected.
	float disconnectRate;			// Player's disconnect rate (disconnects/matches).
	int careerDraws;				// Player's total number of draws (tied matches).
	int matchDrawStreak;			// # of consecutive 2 player games ending with draw
	int careerLongestWinStreak;		// longest consecutive 2 player games won
	int careerLongestLossStreak;	// longest consecutive 2 player games lost
	int careerLongestDrawStreak;	// longest consecutive 2 player games ending with draw
	int totalCompleteMatches;		// Total number of matches where the game went to completion (all win/loss/draw results).	

	// single player game stats
	int spBestRaceTime;				//*** Player's career best race time (milliseconds).
	int spWorstRaceTime;			// Player's career worst race time (milliseconds).
	int spTotalPlays;				// total single player games played
	float spAverageRaceTime;		// Player's average race time per play (milliseconds/match).
	int spTotalRaceTime;			// Player's total race time for all plays (milliseconds).

	// single and 2 player game stats
	char* lastGameTime;			// date/time string of last game stats retrieved from Sake

};


@interface LeaderboardObject : NSObject <MKAnnotation>
{
	NSString* name;
	gsi_u32 profileId;
	
	gsi_u32 pictureFileId;
	UIImage *thumbNail;	
	
	int rank;
	int careerWins;			//***
	int careerLosses;		//***
	int bestRaceTime;		//*** Player's career best race time (milliseconds).
	int totalMatches;		//***
	int currentWinStreak;	//***
							//***
	CLLocation *playerLocation;		// lat/long of player, used for map view
}
@property (nonatomic, copy) NSString * name;
@property gsi_u32 profileId;
@property gsi_u32 pictureFileId;
@property (nonatomic, retain) UIImage * thumbNail;
@property int rank;
@property int careerWins;
@property int careerLosses;
@property int bestRaceTime;
@property int totalMatches;
@property int currentWinStreak;
@property (nonatomic, retain) CLLocation * playerLocation;

@end
