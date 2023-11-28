// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "UserStatsController.h"
#import "LeadersController.h"
#import "BuddyListCellView.h"
#import "AppDelegate.h"
#import "Utility.h"
#import "LoginController.h"

enum AppAlert
{
	ALERT_NONE,
	ALERT_MAKEBUDDY,
	ALERT_REPORTIMAGE
};

@implementation UserStatsController

@synthesize thumbNail;
@synthesize userName;
@synthesize  userTotalRaces;
@synthesize  userTotalWins;
@synthesize  userTotalLosses;
@synthesize  userRatio;
@synthesize  userBestTime;
@synthesize  userWinStreak;

@synthesize  myRank;
@synthesize  myTotalRaces;
@synthesize  myTotalWins;
@synthesize  myTotalLosses;
@synthesize  myRatio;
@synthesize  myBestTime;
@synthesize  myWinStreak;

- (void)dealloc
{
	if(thumbNail != nil) [thumbNail release];
	[userName release];
	[userTotalRaces release];
	[userTotalWins release];
	[userTotalLosses release];
	[userRatio release];
	[userBestTime release];
	[userWinStreak release];
	[myRank release];
	[myTotalRaces release];
	[myTotalWins release];
	[myTotalLosses release];
	[myRatio release];
	[myBestTime release];
	[myWinStreak release];
	//if(updateTimer != nil) [updateTimer invalidate];
	//[updateTimer release];
	[super dealloc];
}

// display image if one is received
//*************************************
- (void)updateTime: (NSTimer*)timer;
{
	LeaderboardObject* leader = [gLeaderboardData objectAtIndex: gLeaderboardProfileIndex];
	UIImage * img = leader.thumbNail;

	// Hide "Report image" button if no image or it is blocked or it is myself
	if (img == nil || [appDelegate imageIsBlocked: leader.pictureFileId] || gPlayerData.profileId == leader.profileId )
	{
		reportImageButton.hidden = YES;
		[updateTimer invalidate];
	}
	else
	// if image found and it is not blocked, display it
	// don't change if no image, will display the default No Photo
	{
		reportImageButton.hidden = NO;
		[self.thumbNail setImage: img];
		[updateTimer invalidate];
		[self.thumbNail setNeedsDisplay];
	}

}


// called after a delay to show rank
//  gives the mainloop a chance for callback to set server value for rank
- (void)delayedRank
{
	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: gLeaderboardProfileIndex];

	if( leader.rank != 0 )
		myRank.text = [NSString stringWithFormat: @"#%d", leader.rank ];
}


//*************************************
- (void)viewWillAppear: (BOOL) animated
{
	int totalRaces;
	int totalWins;
	int bestTime;
	
	// V.1 hide buddy button
	makeBuddyButton.hidden = YES;
	
	
	
	//[super viewWillAppear: animated];

	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: gLeaderboardProfileIndex];

	// if this is a buddy list, then hide the 'make buddy' button since he's already a buddy
	if (gLeaderboardType == LEADERBOARD_BY_BUDDIES) 
	{
		makeBuddyButton.hidden = YES;
	}
	
	myRank.text = [NSString stringWithFormat: @"#%d", gLeaderboardProfileIndex + 1 ];
/*	else
	{
		// request rank from server
		[appDelegate getMyLeaderPosition: &(leader.rank) ];
		
		// go get it in 1 second 
		[self performSelector:@selector(delayedRank) withObject:nil afterDelay:1.0f];
	}
*/	
	NSString* aTitle = [NSString stringWithFormat: @"%@'s Profile", leader.name];
	self.title = aTitle;  //[leader.name copy];	// index of currently displayed profile
	//self.title = [leader.name copy];
	// display image if not blocked and a picture exists
	// if no image exists, will display the default No Photo

	UIImage * img = leader.thumbNail;
	if( img == nil && leader.pictureFileId == 0 )
	{
		// kick off a timer to poll for the image...just in case the leaderboard fills it in for us while we're sitting here
		if (updateTimer != nil) {
			[updateTimer invalidate];
		}
		updateTimer = [NSTimer scheduledTimerWithTimeInterval: 0.1 target: self selector: @selector(updateTime:) userInfo: nil repeats: YES];
	}
	else if ( ! [appDelegate imageIsBlocked: leader.pictureFileId] ) 
	{
		reportImageButton.hidden = NO;	
		[self.thumbNail setImage: img];
	}
	else	// picture is blocked 
	{
		reportImageButton.hidden = YES;		
	}		

	if( gPlayerData.profileId == leader.profileId )
	{
		reportImageButton.hidden = YES;		
	}		

	self.userName.text = [leader.name copy];
	
	totalRaces = leader.totalMatches;
	self.userTotalRaces.text = [NSString stringWithFormat: @"%d", totalRaces];
	
	totalWins = leader.careerWins;
	self.userTotalWins.text = [NSString stringWithFormat: @"%d", totalWins];
	
	self.userTotalLosses.text = [NSString stringWithFormat: @"%d", leader.careerLosses];
	if (totalWins > 0) {
		self.userRatio.text = [NSString stringWithFormat: @"%d\%%", (totalWins*100)/totalRaces];
	} else {
		self.userRatio.text = [NSString stringWithString: @"0%"];
	}
	
	bestTime = leader.bestRaceTime;
	if ((bestTime <= 0) || (bestTime >= INITIAL_BEST_TIME)) 
	{
		self.userBestTime.text = [NSString stringWithString: @"none"];
	} 
	else 
	{
		SetTimeLabel(self.userBestTime, bestTime);
	}
	
	self.userWinStreak.text = [NSString stringWithFormat: @"%d", leader.currentWinStreak];

	self.myTotalRaces.text = [NSString stringWithFormat: @"%d", getLocalIntegerStat(matchTotalCompleteMatchesKey)];
	self.myTotalWins.text = [NSString stringWithFormat: @"%d", getLocalIntegerStat(matchWonKey)];
	self.myTotalLosses.text = [NSString stringWithFormat: @"%d", getLocalIntegerStat(matchLossKey)];
	
	if (gPlayerData.totalMatches > 0) 
	{
		self.myRatio.text = [NSString stringWithFormat: @"%d\%%", (getLocalIntegerStat(matchWonKey)*100) / getLocalIntegerStat(matchTotalCompleteMatchesKey)];
	} 
	else 
	{
		self.myRatio.text = [NSString stringWithString: @"0%"];
	}
	
	int myBestRaceTime = getLocalIntegerStat(matchBestTimeKey);
	if( (myBestRaceTime <= 0) || (myBestRaceTime >= INITIAL_BEST_TIME) )  
	{
		self.myBestTime.text = [NSString stringWithString: @"none"];
	} 
	else 
	{
		SetTimeLabel(self.myBestTime, myBestRaceTime);
	}
	
	self.myWinStreak.text = [NSString stringWithFormat: @"%d", gPlayerData.matchWinStreak];
	
}

//*************************************
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}


// Display the About page
- (IBAction)showInfo 
{    
	[ appDelegate showAboutInfo: self ];
} 


//*************************************
- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
	switch (currentAlert) {
		case ALERT_MAKEBUDDY:
			if (buttonIndex != alertView.cancelButtonIndex) {
				gsi_char reason[GP_REASON_LEN];
				reason[0] = 0; // null in any language
				
				LoginController* loginController = (LoginController*)[appDelegate findExistingControllerOfType: [LoginController class]];
				gpSendBuddyRequest(  [loginController getGPConnectionPtr], 
								   ((LeaderboardObject*)[gLeaderboardData objectAtIndex: gLeaderboardProfileIndex]).profileId, 
								   reason ); 
				// this could cause a gpRecvBuddyRequest callback indicating that it was accepted or rejected.
				//   for this simple app, we don't care
			}
			break;
		case ALERT_REPORTIMAGE:
			// Block image if cancel button ("No") was not pressed 
			if (buttonIndex != alertView.cancelButtonIndex) 
			{
				// set & display default image in user stat page
				NSString* defaultImage = [[NSBundle mainBundle] pathForResource:@"singlePlayer" ofType:@"png"];
				UIImage* imageObj = [[UIImage alloc] initWithContentsOfFile:defaultImage];
				[self.thumbNail setImage: imageObj];

				// add to block images list
				LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: gLeaderboardProfileIndex];
				NSNumber* aFileId = [[ NSNumber alloc] initWithUnsignedInteger: leader.pictureFileId ];
				[ gPlayerData.blockedImagesData addObject:aFileId ];
				
				// save to file
				[ gPlayerData.blockedImagesData writeToFile: [gPlayerData.playerDataPath stringByAppendingPathComponent: blockedImagesDataFile] atomically: YES];
			}
			break;
	}
	currentAlert = ALERT_NONE;
}
			
			
//*************************************
- (IBAction)makeBuddyButtonClicked: (id)sender
{
	alertView = [[UIAlertView alloc] initWithTitle: @"Make Buddy" message: @"Are you sure?" delegate: self cancelButtonTitle: @"No" otherButtonTitles: @"Yes", nil];
	[alertView show];
	currentAlert = ALERT_MAKEBUDDY;
}


//*************************************
- (IBAction)reportImageButtonClicked: (id)sender
{
	alertView = [[UIAlertView alloc] initWithTitle: @"Report Image" message: @"Are you sure?" delegate: self cancelButtonTitle: @"No" otherButtonTitles: @"Yes", nil];
	[alertView show];
	currentAlert = ALERT_REPORTIMAGE;
}

@end
