// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>


@interface GameResultsController : UIViewController
{
	// Single-player controls:
	IBOutlet UILabel* gameTimeLabel;
	IBOutlet UILabel* gamesPlayedLabel;
	IBOutlet UILabel* bestTimeLabel;
	IBOutlet UILabel* averageTimeLabel;
	IBOutlet UILabel* congratsLabel;
	IBOutlet UILabel* newbestLabel;
	IBOutlet UILabel* newbestTimeLabel;
	IBOutlet UILabel* oldbestLabel;
	IBOutlet UILabel* avgTapsPerSecLabel;
	
	// Multi-player controls:
	IBOutlet UILabel* tieLabel;

	IBOutlet UILabel* localWinnerLabel;
	IBOutlet UILabel* localPlayerLabel;
	IBOutlet UILabel* localTimeLabel;
	IBOutlet UIButton* localThumbnailButton;  // so that we can grey out loser
	//IBOutlet UIImageView* localThumbnail;
	
	IBOutlet UILabel* remoteWinnerLabel;
	IBOutlet UILabel* remotePlayerLabel;
	IBOutlet UILabel* remoteTimeLabel;
	IBOutlet UIButton* remoteThumbnailButton;
	//IBOutlet UIImageView* remoteThumbnail;

	// Common controls:
	IBOutlet UIButton* leaderboardsButton;
	IBOutlet UIButton* playAgainButton;
	IBOutlet UIButton* menuButton;
	
	IBOutlet UIActivityIndicatorView* busyLocal;
	IBOutlet UIActivityIndicatorView* busyRemote;

	int localTime;
	int remoteTime;
}

- (IBAction)leaderboardsButtonClicked: (id)sender;
- (IBAction)playAgainButtonClicked: (id)sender;
- (IBAction)menuButtonClicked: (id)sender;
- (IBAction)viewOpponentClicked: (id)sender;
- (IBAction)viewMyStatsClicked: (id)sender;

- (IBAction)showInfo;
- (IBAction)gotoMenu;
- (IBAction)gotoMatchMaking;

- (void)setGamesPlayed: (int)gamesPlayed;
- (void)setBestTime: (int)bestTime;
- (void)setAverageTime: (int)averageTime;
- (void)setAverageTaps: (float) avgTaps;

- (void)setLocalPlayer: (NSString*)playerName time: (int)time;
- (void)setRemotePlayer: (NSString*)playerName time: (int)time;

- (void)setGameTime: (int)gameTime;
- (void)setWinner;
- (void)setNewBestTime: (int)oldTime;
- (void) getImagesForResultsPage;
- (void) drawThumbnailImage: (UIImage*)image forPlayer: (int) profileId;

- (void)connectionDropped;
@end
