// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "LeadersController.h"
#import "AppDelegate.h"
#import "BuddyListCellView.h"


static NSString * kCellIdentifier1 = @"gobbletygoop";


@implementation LeadersController

@synthesize buddyList;


- (void)dealloc
{
	[tableContainer release];
	[leadersList release];
	[buddyList release];
	[super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)constructView
{
	// Create table.
	CGRect rect = tableContainer.frame;
	
	// add the table view to it
	leadersList = [[UITableView alloc] initWithFrame:tableContainer.frame style: UITableViewStylePlain];
	leadersList.backgroundColor = self.view.backgroundColor;
	leadersList.delegate = self;
	leadersList.dataSource = self;
	leadersList.separatorStyle = UITableViewCellSeparatorStyleSingleLine;
	leadersList.separatorColor = [UIColor blackColor];
	[self.view addSubview: leadersList];	
	leadersList.frame = rect;
	
	if (gLeaderboardType == LEADERBOARD_BY_TIME) 
	{
		self.title = [NSString stringWithString: @"Leaderboard"];
	} 
	else 
	{
		self.title = [NSString stringWithString: @"Leaders by Buddies"];
	}
	
	if ([gLeaderboardData count] == 0) 
	{
		if (gLeaderboardType == LEADERBOARD_BY_TIME) 
		{
			alertView = [[UIAlertView alloc] initWithTitle: nil message: @"No leaders!" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
		} 
		else 
		{
			alertView = [[UIAlertView alloc] initWithTitle: nil message: @"No buddies have played!" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
		}
		
		[alertView show];
	}
}

- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
	// same as exit
	[self.navigationController popViewControllerAnimated: YES];
}

// Set each thumbnail that is blocked to default picture
-(void)updatePictureInTable
{
	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: gLeaderboardProfileIndex];
	if( [appDelegate imageIsBlocked:leader.pictureFileId ] )
	{
		// get default image
		NSString* defaultImage = [[NSBundle mainBundle] pathForResource:@"singlePlayer" ofType:@"png"];
		UIImage* imageObj = [[UIImage alloc] initWithContentsOfFile:defaultImage];

		// set cell picture to default & refresh
		NSNumber * key = [[NSNumber alloc] initWithInt:gLeaderboardProfileIndex];
		BuddyListCellView * view = [buddyList objectForKey: key];
		[view setImage: imageObj];   
		[view setNeedsDisplay];
	}
}

- (void)viewWillAppear: (BOOL)animated
{
	if (leadersList == nil) 
	{
		[self constructView];
	}
	else
	{
		[self updatePictureInTable ];
	}
}


// Display the About page
- (IBAction)showInfo 
{    
	[ appDelegate showAboutInfo: self ];
} 


- (UITableViewCell *)tableView:(UITableView *)tblView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	UITableViewCell *cell = nil;
	NSNumber * key = [NSNumber numberWithInteger: indexPath.row];
	unsigned int keyint = [key intValue];
	
	if (keyint > [buddyList count]) 
		return nil;
	
	BuddyListCellView * view = [buddyList objectForKey: key];
	CGRect rect;
	if (view == nil)
		return nil;
	
	rect = view.cellView.frame;
	cell = [tblView dequeueReusableCellWithIdentifier: kCellIdentifier1];
	if (cell == nil) 
	{
		cell = [[[UITableViewCell alloc] initWithFrame: rect reuseIdentifier:kCellIdentifier1] autorelease];
	}
	
	// ??? need to change cell = initWithStyle:reuseIdentifier
	//UITableViewCellAccessoryDisclosureIndicator ];
	
	// modify the data for this cellview
	UIView * uiview = nil;
	for (uiview in cell.contentView.subviews) 
		break; // just get the first one (only one)
	
	if (uiview != nil) 
	{
		// remove old one
		[uiview removeFromSuperview]; // releases it and its contents
	}
	
	// go ahead and add the current view
	[cell.contentView addSubview: view.cellView]; // only add the 1st time it is visible
	
	return cell;
}


- (NSInteger)tableView:(UITableView *)tblView numberOfRowsInSection:(NSInteger)section
{
	if ((buddyList == nil) && (gLeaderboardData != nil) && ([gLeaderboardData count] > 0)) 
	{
		// make the list before the table actually starts needing it
		buddyList = [[NSMutableDictionary dictionaryWithCapacity: [gLeaderboardData count]] retain]; 
	}
	return [gLeaderboardData count];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tblView
{
	return 1;
}

- (CGFloat)tableView:(UITableView *)tblView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	NSNumber * key = [NSNumber numberWithInteger: indexPath.row];
	int keyint = [key intValue];
	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: keyint];
	
	// now construct the cell for this entry in the table
	BuddyListCellView * newcell = [[BuddyListCellView alloc] initCellView];
	
	// fill in the fields
	[newcell setName: leader.name];
	newcell.profileId = leader.profileId;
	[newcell setIndex: keyint+1];
	UIImage * img = leader.thumbNail;
	
	// if no thumbnail, leave as default 'No Photo' image
	if (img != nil && ![appDelegate imageIsBlocked: leader.pictureFileId]) 
	{
		[newcell setImage: img];
	} 
	else 
	{
		// kick off the process of getting the image
		[newcell getImageFromNetwork];
	}
	
	[newcell setTime: leader.bestRaceTime];

	// add to array
	[buddyList setObject: newcell forKey: key];
	
	CGRect rect = newcell.cellView.frame;	
	[newcell release];
	return rect.size.height+1;
}

//Managing Accessory Views
- (UITableViewCellAccessoryType)tableView:(UITableView *)tblView accessoryTypeForRowWithIndexPath:(NSIndexPath *)indexPath
{
	return UITableViewCellAccessoryDisclosureIndicator;
}

- (void)tableView:(UITableView *)tblView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{	
	NSNumber * key = [NSNumber numberWithInteger: indexPath.row];
	int keyint = [key intValue];

	[tblView deselectRowAtIndexPath:indexPath	animated:YES];
	[appDelegate showUser: keyint];
}


@end
