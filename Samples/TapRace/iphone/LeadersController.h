// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>

@interface LeadersController : UIViewController <UITableViewDelegate, UITableViewDataSource, UIAlertViewDelegate>
{
	UITableView*			leadersList;	// list of leader info
	IBOutlet UILabel*		tableContainer;	// table GUI
	NSMutableDictionary*		buddyList;	// preconstructed list of cell views

	UIAlertView* alertView;
}

@property (nonatomic, retain) NSMutableDictionary* buddyList;

- (IBAction)showInfo;

- (void)updatePictureInTable;

@end
