// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>


@interface GameController : UIViewController
{
	IBOutlet UIButton* tapButton1;
	IBOutlet UIButton* tapButton2;
	IBOutlet UIProgressView* localProgress;
	IBOutlet UIProgressView* remoteProgress;
	IBOutlet UILabel* localPlayerLabel;
	IBOutlet UILabel* remotePlayerLabel;
	IBOutlet UILabel* gameTimeLabel;
	IBOutlet UILabel* previousBestLabel;
	IBOutlet UILabel* previousBestTimeLabel;
	IBOutlet UILabel* messageLabel;
	IBOutlet UIActivityIndicatorView* activityIndicator;

	NSTimer* updateTimer;
	NSArray* gameControls;

	NSString* initialMessageText;

	int numSteps, prevNumSteps;

	bool opponentFinished;
}

@property(nonatomic, readonly) int numSteps;
@property(nonatomic, readwrite ) int prevNumSteps;

- (IBAction)startButtonClicked: (id)sender;
- (IBAction)tapButtonPressed: (id)sender;
- (IBAction)showInfo;
- (IBAction)gotoMenu;

//- (void)gotoMenu: (id)sender;
- (void)countdown: (int)count;
- (void)stopRaceClock;
- (void)raceStarted;
- (void)doneRacing;
- (void)remoteProgress: (int)progress;
- (void)setRemoteNick: (NSString*)nick;

- (void)reset;

@end
