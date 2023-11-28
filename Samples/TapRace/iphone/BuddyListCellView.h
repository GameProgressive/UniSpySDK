//
//  BuddyListCellView.h
//
//  Created by CW Hicks 10/21/08.
//  Copyright 2008 IGN. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface BuddyListCellView : UIView
{
	IBOutlet UIView*	cellView;
	IBOutlet UIImageView* thumbnail;
	IBOutlet UILabel* nameLabel;
	IBOutlet UILabel* indexLabel;
	IBOutlet UILabel* timeLabel;
	
	int profileId;
	unsigned int pictureFileId;
	int index;
	
	int cannotDie; // ref count for number of outstanding callbacks
	bool mustDie;
	
	IBOutlet UIActivityIndicatorView* busy;
}

@property (nonatomic, retain) UIView * cellView;
@property int profileId;
@property unsigned int pictureFileId;
@property int index;

- (void)setName: (NSString*)name;
- (void)setIndex: (int)newIndex;
- (void)setImage: (UIImage*)image;
- (void)setTime: (int)newTime; 

- (id)initCellView; // call to load from xib
- (void)getImageFromNetwork;

@end

