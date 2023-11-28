// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import "MyCLController.h"
#import "FlipsideViewController.h"

@interface MenuController : UIViewController <MyCLControllerDelegate,FlipsideViewControllerDelegate>
{
	IBOutlet UIButton* singlePlayerButton;
	IBOutlet UIButton* multiPlayerButton;
	IBOutlet UIButton* leaderboardsButton;
	IBOutlet UILabel*  leadersLabel;
	IBOutlet UILabel*  leadersPositionLabel;
	IBOutlet UILabel*  leadersTimeLabel;
	
	NSTimer* displayLeaderTimer;

	BOOL isCurrentlyUpdating;
	UIAlertView* alertView;
}

@property (nonatomic, retain)  UILabel* leadersLabel;
@property (nonatomic, retain)  UILabel* leadersPositionLabel;
@property (nonatomic, retain)  UILabel* leadersTimeLabel;

- (IBAction)onSinglePlayerClicked: (id)sender;
- (IBAction)onMultiPlayerClicked: (id)sender;
- (IBAction)onLeaderboardsClicked: (id)sender;
//- (IBAction)showAbout: (id) sender;
- (IBAction)showInfo;

- (void)reset;
-(void)noTop5Leaders;


@end
