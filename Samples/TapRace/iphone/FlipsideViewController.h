//
//  FlipsideViewController.h
//  flipview
//
//  Created by Paul Berlinguette on 6/11/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//
#import <MessageUI/MessageUI.h>	

@protocol FlipsideViewControllerDelegate;

@interface FlipsideViewController : UIViewController <MFMailComposeViewControllerDelegate> {
	id <FlipsideViewControllerDelegate> delegate;
	IBOutlet UIButton* clearBlockedImagesButton;
}

@property (nonatomic, assign) id <FlipsideViewControllerDelegate> delegate;
- (IBAction)done;
- (IBAction)contactUs:(id)sender;
- (IBAction)clearBlockedImagesFile;

@end
@protocol FlipsideViewControllerDelegate

- (void)flipsideViewControllerDidFinish:(FlipsideViewController *)controller;
@end

