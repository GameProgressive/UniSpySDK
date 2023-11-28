// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import <SystemConfiguration/SCNetworkReachability.h>

#import "GP/gp.h"

@class CreateUserController;

@interface LoginController : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource>
{
	IBOutlet UITextField* userNameField;
	IBOutlet UIPickerView* userNamePicker;
	IBOutlet UITextField* passwordField;
	IBOutlet UISwitch* autoLoginSwitch;
	IBOutlet UIActivityIndicatorView* activityIndicator;
	IBOutlet UIButton* forgotPassword;
	IBOutlet UIView* userNamePickerView;
	IBOutlet UIButton* info;
	
	UIAlertView* alertView;

	NSTimer* loginTimer;

	NSArray* rememberedLogins;

	NSDictionary* rememberedPasswordsByLogin;

	GPConnection connection;
}

- (IBAction)loginClicked: (id)sender;
- (IBAction)returnClicked: (id)sender;
- (IBAction)createClicked: (id)sender;
- (IBAction)forgotPasswordClicked: (id)sender;
- (IBAction)autoLoginChanged: (id)sender;

- (void)userProfileCreated: (CreateUserController*)createUserController autoLogin: (BOOL) autoLoginValue;
- (void)stopLoginTimer;
- (void)reset;
- (void)delayedNetworkConnectionErrorMessage;
- (GPConnection *)getGPConnectionPtr;
- (BOOL)isNetworkAvailableFlags:(SCNetworkReachabilityFlags *)outFlags;

@end
