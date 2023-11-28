// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import "gp/gp.h"

@class LoginController;

@interface CreateUserController : UIViewController
{
	IBOutlet UITextField* emailField;
	IBOutlet UITextField* userNameField;
	IBOutlet UITextField* passwordField;
	IBOutlet UISwitch* autoLoginSwitch;

	LoginController* loginController;

	NSTimer* loginTimer;

	GPConnection connection;
	GPProfile profile;
}

@property(nonatomic, assign) LoginController* loginController;
@property(nonatomic, readonly) GPConnection connection;
@property(nonatomic, readonly) GPProfile profile;
@property(nonatomic, readonly) NSString* uniquenick;
@property(nonatomic, readonly) NSString* password;
@property(nonatomic, retain) UISwitch * autoLoginSwitch;

- (IBAction)createButtonClicked: (id)sender;
- (IBAction)returnClicked: (id)sender;

@end
