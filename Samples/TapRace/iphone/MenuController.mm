// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "MenuController.h"
#import "AboutController.h"
#import "AppDelegate.h"
#import <UIKit/UIDevice.h>

#define ROTATE_LEADER_TIME	3  // seconds

@interface MenuController(Private)

- (void)updateLeaderLabel: (NSTimer*)timer;

@end



@implementation MenuController


@synthesize  leadersLabel;
@synthesize  leadersPositionLabel;
@synthesize  leadersTimeLabel;

- (void)dealloc
{
	[MyCLController sharedInstance].delegate = nil; // we won't handle this any more
	[singlePlayerButton release];
	[multiPlayerButton release];
	[leaderboardsButton release];
	[super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}



- (IBAction)onSinglePlayerClicked: (id)sender
{
	[appDelegate startSinglePlayerGame];
}

- (IBAction)onMultiPlayerClicked: (id)sender
{
	[appDelegate findGame];
}

- (IBAction)onLeaderboardsClicked: (id)sender
{
	[appDelegate showLeaderboards];
}

- (void)reset
{
	// Empty
}

- (void)updatePosition: (BOOL)start 
{
	if (!start) {
		[[MyCLController sharedInstance].locationManager stopUpdatingLocation];
		isCurrentlyUpdating = NO;
	} else {
		[[MyCLController sharedInstance].locationManager startUpdatingLocation];
		isCurrentlyUpdating = YES;
	}
}


- (void)viewWillDisappear: (BOOL)animated
{
	[self updatePosition: NO];

	if( displayLeaderTimer != nil )
	{
		[displayLeaderTimer invalidate];
	}
}


- (void)viewDidLoad
{
	isCurrentlyUpdating = NO;
	
	[MyCLController sharedInstance].delegate = self;
	
	// Check to see if the user has disabled location services
	// In that case, we just print a message
	NSString *modelType = [UIDevice currentDevice].model;
	if( [modelType isEqualToString: (NSString *) @"iPhone" ] )
	{
		isCurrentlyUpdating = YES;
		
		if ( ! [MyCLController sharedInstance].locationManager.locationServicesEnabled ) 
		{
			alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Location Services Disabled" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
			[alertView show];
			[self updatePosition: NO];
		} 
		else
		{
			[self updatePosition: YES];
		}	
	}
}

- (void)viewWillAppear: (BOOL) animated
{
	self.title = @"Menu";
	[appDelegate showTop5LeadersByTime];

	displayLeaderTimer = [NSTimer scheduledTimerWithTimeInterval: ROTATE_LEADER_TIME target: self selector: @selector(updateLeaderLabel:) userInfo: nil repeats: YES];
}


-(void)noTop5Leaders
{
	leadersLabel.text = @"No leaders yet!";
	[displayLeaderTimer invalidate];
}


- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
}

	
 - (void)flipsideViewControllerDidFinish:(FlipsideViewController *)controller 
 {
 	[self dismissModalViewControllerAnimated:YES];
 }
 
 
 - (IBAction)showInfo 
 {    
 	[ appDelegate showAboutInfo: self ];
 } 



#pragma mark ---- delegate methods for the MyCLController class ----
-(void)newLocationUpdateLat:(float)lat Long: (float)lng
{
	NSLog(@"updated player location to %.5f %.5f", lat, lng);
	isCurrentlyUpdating = NO;
	if (gPlayerData.playerLocation == nil) 
	{
		gPlayerData.playerLocation = [[CLLocation alloc] initWithLatitude: lat longitude: lng];
	} 
	else 
	{
		[gPlayerData.playerLocation initWithLatitude: lat longitude: lng];
	}
	
	[self updatePosition: NO];
}

-(void)newError:(NSString *)text
{
	isCurrentlyUpdating = NO;
	[self updatePosition: NO];
	alertView = [[UIAlertView alloc] initWithTitle: @"Location Services Failed" message: text delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
	[alertView show];
}


@end


@implementation MenuController(Private)

#define TOP_LEADERS_UPDATE_INTERVAL	10*60*1000
//#define TOP_LEADER_COUNT			5

// rotate display of Top 5 in Menu page every 3 seconds 
- (void)updateLeaderLabel: (NSTimer*)timer;
{
	static int displayedLeaderNumber = -1;
	int maxLeaders = [ gLeaderboardData count ];
	
	if( maxLeaders == 0 )
		return;
	
	++displayedLeaderNumber;
	if( displayedLeaderNumber >= maxLeaders )
		displayedLeaderNumber = 0;
	
	int secs = ((LeaderboardObject*)[ gLeaderboardData objectAtIndex:displayedLeaderNumber ]).bestRaceTime /1000;
	int millis = ((LeaderboardObject*)[ gLeaderboardData objectAtIndex:displayedLeaderNumber ]).bestRaceTime - (secs * 1000);
	NSString* leaderName = [((LeaderboardObject*)[gLeaderboardData objectAtIndex: displayedLeaderNumber]).name copy ];

	leadersPositionLabel.text = [NSString stringWithFormat: @"%d", displayedLeaderNumber+1 ];
	leadersLabel.text = leaderName; //[NSString stringWithFormat: @"%d %s", displayedLeaderNumber+1, leaderName ];
	leadersTimeLabel.text = [NSString stringWithFormat: @"%2d.%03d", secs, millis ];
}


/*- (void)updateLeaderLabel: (NSTimer*)timer;
{
/*	static int currentLeaderNumber = -1;
	static uint64_t	curTime, lastUpdateTime = nil;
	
	curTime = mach_absolute_time();
	
	if( lastUpdateTime == nil || lastUpdateTime - curTime > TOP_LEADERS_UPDATE_INTERVAL )
	{
		// update latest Top 5 leaders
		[appDelegate showLeadersByTime ];		
	}
	
	// set last update time
	lastUpdateTime = curTime;

	// decide next leader to display
	if( currentLeaderNumber == TOP_LEADER_COUNT )
		currentLeaderNumber = -1;
	
	++currentLeaderNumber;
	
	leadersLabel.text = [NSString stringWithFormat: @"  %d. %s", currentLeaderNumber, gLeaderboardData.];
}
*/	


@end
