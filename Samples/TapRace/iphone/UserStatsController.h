// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>


@interface UserStatsController : UIViewController <UIAlertViewDelegate>
{
	IBOutlet UIImageView* thumbNail;
	IBOutlet UILabel* userName;
	IBOutlet UILabel* userTotalRaces;
	IBOutlet UILabel* userTotalWins;
	IBOutlet UILabel* userTotalLosses;
	IBOutlet UILabel* userRatio;
	IBOutlet UILabel* userBestTime;
	IBOutlet UILabel* userWinStreak;
	IBOutlet UILabel* myRank;
	
	IBOutlet UILabel* myTotalRaces;
	IBOutlet UILabel* myTotalWins;
	IBOutlet UILabel* myTotalLosses;
	IBOutlet UILabel* myRatio;
	IBOutlet UILabel* myBestTime;
	IBOutlet UILabel* myWinStreak;
	
	IBOutlet UIButton* makeBuddyButton;
	IBOutlet UIButton* reportImageButton;
	
	UIAlertView* alertView;
	int currentAlert;
	NSTimer* updateTimer;
}
@property (nonatomic, retain) UIImageView* thumbNail;
@property (nonatomic, retain) UILabel* userName;
@property (nonatomic, retain)  UILabel* userTotalRaces;
@property (nonatomic, retain)  UILabel* userTotalWins;
@property (nonatomic, retain)  UILabel* userTotalLosses;
@property (nonatomic, retain)  UILabel* userRatio;
@property (nonatomic, retain)  UILabel* userBestTime;
@property (nonatomic, retain)  UILabel* userWinStreak;
@property (nonatomic, retain)  UILabel* myRank;

@property (nonatomic, retain)  UILabel* myTotalRaces;
@property (nonatomic, retain)  UILabel* myTotalWins;
@property (nonatomic, retain)  UILabel* myTotalLosses;
@property (nonatomic, retain)  UILabel* myRatio;
@property (nonatomic, retain)  UILabel* myBestTime;
@property (nonatomic, retain)  UILabel* myWinStreak;

- (IBAction)makeBuddyButtonClicked: (id)sender;
- (IBAction)reportImageButtonClicked: (id)sender;
- (IBAction)showInfo;


@end
