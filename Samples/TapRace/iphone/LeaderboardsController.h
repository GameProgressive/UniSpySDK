// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>


@interface LeaderboardsController : UIViewController
{
	IBOutlet UIButton* setPhotoButton;
	IBOutlet UIButton* top100Button;
	IBOutlet UIButton* mySpotButton;
	IBOutlet UIButton* byLocationButton;
	
	IBOutlet UILabel* myTotalRaces;
	IBOutlet UILabel* myRank;
	IBOutlet UILabel* myTotalWins;
	IBOutlet UILabel* myTotalLosses;
	IBOutlet UILabel* myRatio;
	IBOutlet UILabel* myBestTime;
	IBOutlet UILabel* myStreak;
	IBOutlet UILabel* myStreakLabel;
	
	
	IBOutlet UILabel* mySPTotalRaces;
	IBOutlet UILabel* mySPBestTime;
	IBOutlet UILabel* mySPAvgTime;

	IBOutlet UIImageView* thumbNail;	
	
	IBOutlet UIActivityIndicatorView* busy;
	
	UIImagePickerController* imagePickerController;
	
	int gUploadedFileId;
	UIImage * newImage;
}

@property (nonatomic, retain)  UILabel* myTotalRaces;
@property (nonatomic, retain)  UILabel* myRank;
@property (nonatomic, retain)  UILabel* myTotalWins;
@property (nonatomic, retain)  UILabel* myTotalLosses;
@property (nonatomic, retain)  UILabel* myRatio;
@property (nonatomic, retain)  UILabel* myBestTime;
@property (nonatomic, retain)  UILabel* myStreak;
@property (nonatomic, retain)  UILabel* myStreakLabel;
@property (nonatomic, retain)  UILabel* mySPTotalRaces;
@property (nonatomic, retain)  UILabel* mySPBestTime;
@property (nonatomic, retain)  UILabel* mySPAvgTime;

@property (nonatomic, retain)  UIImageView* thumbNail;


- (IBAction)setPhotoButtonClicked: (id)sender;
- (IBAction)top100ButtonClicked: (id)sender;
- (IBAction)myBuddiesButtonClicked: (id)sender;
- (IBAction)byLocationButtonClicked: (id)sender;
- (IBAction)showInfo;

- (void) testForExistingImageRecord;
- (void) getImageFromProfile;
- (void) drawAndUploadImage: (UIImage*)image;
- (void) drawThumbnailFromImage: (UIImage*)image;
- (void) delayedDraw;
- (void) delayedRank;

@end
