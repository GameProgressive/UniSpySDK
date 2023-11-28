// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import <MapKit/MapKit.h>

@interface LeadersByLocationController : UIViewController <UIWebViewDelegate,  UIAlertViewDelegate, MKMapViewDelegate>
{
	NSString * myName;
	NSTimer* updateTimer;
	
	IBOutlet UIWebView* webView;
	IBOutlet MKMapView *mapView;
	IBOutlet UIActivityIndicatorView *spinner;
	
	BOOL isCurrentlyUpdating;
	UIAlertView* alertView;
}

@property (nonatomic, retain) UIActivityIndicatorView *spinner;
@property (nonatomic, retain) NSString * myName;

- (IBAction)showInfo;

- (void)searchLeaderboardsByLocation;

@end
