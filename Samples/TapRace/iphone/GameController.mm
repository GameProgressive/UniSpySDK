// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "GameController.h"
#import "AppDelegate.h"
#import "Utility.h"

#import <mach/mach_time.h>

#if TARGET_IPHONE_SIMULATOR
static const int TOTAL_STEPS = 3;
#else
static const int TOTAL_STEPS = 60;
#endif



@interface GameController(Private)

- (void)updateTime: (NSTimer*)timer;

@end


@implementation GameController

@synthesize numSteps;
@synthesize prevNumSteps;

- (id)initWithCoder: (NSCoder*)decoder
{
	self = [super initWithCoder: decoder];

	if (self != nil) 
	{
		numSteps = prevNumSteps	= 0;
		opponentFinished = false;
	}

	return self;
}

- (void)dealloc
{
	[tapButton1 release];
	[tapButton2 release];
	[localProgress release];
	[remoteProgress release];
	[localPlayerLabel release];
	[remotePlayerLabel release];
	[gameTimeLabel release];
	[previousBestLabel release];
	[previousBestTimeLabel release];
	[messageLabel release];
	[activityIndicator release];
	[gameControls release];
	[initialMessageText release];
    [super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}

- (void)viewDidLoad
{
	gameControls = [[NSArray alloc] initWithObjects: tapButton1, tapButton2, nil];

	if (messageLabel != nil) 
	{
		initialMessageText = [messageLabel.text copy];
	}

	// if Two-player game, set button back to matchmaking
	if ( [appDelegate isTwoPlayerGame] ) 
	{
		// set right button to go to menu page
		UIBarButtonItem* aBarButton = [ [UIBarButtonItem alloc] initWithTitle:@"Menu" 
																		style:UIBarButtonItemStyleBordered 
																	   target:self 
																	   action:@selector(gotoMenu)];
		self.navigationItem.rightBarButtonItem = aBarButton;
		[aBarButton release];

		aBarButton = [ [UIBarButtonItem alloc] initWithTitle:@"MatchMaking" 
											 style:UIBarButtonItemStyleBordered 
											target:self 
											action:@selector(gotoMatchMaking)];
		self.navigationItem.leftBarButtonItem = aBarButton;
		[aBarButton release];
	}
}

- (void)viewWillAppear: (BOOL)animated
{
	// if Single-player game
	if (! [appDelegate isTwoPlayerGame] ) 
	{
		self.title = @"Single Player Game";
		
		// show previous best time 
		if (gPlayerData.playerStatsData != nil) 
		{
			int previousBestTimeMillis = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerBestTimeKey] intValue];

			previousBestTimeLabel.text = [NSString stringWithFormat: timeFormatString, previousBestTimeMillis / (60 * 1000), (previousBestTimeMillis % (60 * 1000)) / 1000.0f];
			previousBestLabel.hidden = NO;
			previousBestTimeLabel.hidden = NO;

			messageLabel.text= @"Start tapping the keys to begin!";
			messageLabel.hidden = NO;
		}
		else 
		{
			previousBestLabel.hidden = YES;
			previousBestTimeLabel.hidden = YES;
		}
	}
}

- (void)viewWillDisappear: (BOOL)animated
{
	if (updateTimer != nil) 
	{
		[updateTimer invalidate];
		updateTimer = nil;
	}
}

- (void)viewDidAppear: (BOOL)animated
{
	// Two-player game 
	if ( [appDelegate isTwoPlayerGame] ) 
	{
		// start immediately
		[appDelegate startCountdown];
	}
	else if( appDelegate.fromMenu )	// Single player game
	{
		MessageBoxNoBlock(@"Alternate tapping the keys. The faster you tap, the better your time will be! ", @"How To Play", @"Continue" );
	}

}

// Display the About page
- (IBAction)showInfo 
{    
	[ appDelegate showAboutInfo: self ];
} 

- (IBAction)startButtonClicked: (id)sender
{
	// Single-player game; tap button
	[tapButton1 removeTarget: self action: @selector(startButtonClicked:) forControlEvents: UIControlEventAllEvents];
	[tapButton1 addTarget: self action: @selector(tapButtonPressed:) forControlEvents: UIControlEventTouchDown];

	tapButton1.enabled = NO;
	tapButton2.enabled = YES;
	
	messageLabel.hidden = YES;
	
	numSteps = 1;
	localProgress.progress = numSteps / (float)TOTAL_STEPS;

	[appDelegate startCountdown];
}

- (IBAction)tapButtonPressed: (id)sender
{
	tapButton1.enabled = !tapButton1.enabled;
	tapButton2.enabled = !tapButton2.enabled;

	++numSteps;
	localProgress.progress = numSteps / (float)TOTAL_STEPS;
	
	if(numSteps == TOTAL_STEPS)
	{
		if(!opponentFinished)
			messageLabel.text = @"Waiting for opponent to finish race";
		
		[appDelegate finishedGame];

		[updateTimer invalidate];
		updateTimer = nil;

		tapButton1.enabled = NO;
		tapButton2.enabled = NO;
	}
}

- (IBAction)gotoMenu
{
	[appDelegate returnToMenu];
}

- (IBAction)gotoMatchMaking
{
	[appDelegate returnToMatchMaking];
}

// stop race clock when disconnected or race canceled
- (void)stopRaceClock
{
	[updateTimer invalidate];
	updateTimer = nil;
}


// 5 second start of race countdown
- (void)countdown: (int)count
{
	if(count)
		messageLabel.text = [NSString stringWithFormat: @"Race starts in %ds", count];
}

- (void)raceStarted
{
	updateTimer = [NSTimer scheduledTimerWithTimeInterval: TAP_POLL_TIME target: self selector: @selector(updateTime:) userInfo: nil repeats: YES];
	if (remoteProgress == nil) {
		// If in single-player mode, the game is already active.
		return;
	}

	/*for (UIControl* control in gameControls) {
		control.enabled = YES;
	}*/
	tapButton1.enabled = YES;
	tapButton2.enabled = NO;

	messageLabel.text = @"Go!";
	[activityIndicator stopAnimating];
}

- (void)doneRacing
{
//	uint64_t localTime = mach_absolute_time() - appDelegate.start;	
//	gameTimeLabel.text = [NSString stringWithFormat: timeFormatString, localTime / (60 * 1000), (localTime % (60 * 1000)) / 1000.0f];
//	gameTimeLabel.text = [NSString stringWithFormat: @"%0.2f", localTime * absolute_to_seconds ];

	messageLabel.text = @"Race Complete";
	
	localProgress.progress = 0.0;
	remoteProgress.progress = 0.0;
}

- (void)remoteProgress: (int)progress
{
	remoteProgress.progress = progress / (float)TOTAL_STEPS;
}

- (void)setRemoteNick: (NSString*)nick
{
	remotePlayerLabel.text = nick;
}

- (void)reset
{
	gameTimeLabel.text = [NSString stringWithFormat: timeFormatString, 0, 0.0];

	if (remoteProgress == nil) 
	{
		tapButton1.enabled = YES;
		tapButton2.enabled = NO;

		// First touch starts the game.
		[tapButton1 removeTarget: self action: NULL forControlEvents: UIControlEventAllEvents];
		[tapButton1 addTarget: self action: @selector(startButtonClicked:) forControlEvents: UIControlEventTouchDown];

		numSteps = 0;
		localProgress.progress = 0.0;
	}
	else 
	{
		numSteps = 0;
		
		localProgress.progress = 0.0;
		remoteProgress.progress = 0.0;
		
		[activityIndicator startAnimating];
		messageLabel.text = initialMessageText;
	}
}

@end


@implementation GameController(Private)

- (void)updateTime: (NSTimer*)timer;
{
	unsigned int timeElapsed = (mach_absolute_time() - appDelegate.start) * absolute_to_millis;
	gameTimeLabel.text = [NSString stringWithFormat: timeFormatString, timeElapsed / (60 * 1000), (timeElapsed % (60 * 1000)) / 1000.0f];
}

@end
