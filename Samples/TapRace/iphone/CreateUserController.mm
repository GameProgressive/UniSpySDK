// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "CreateUserController.h"
#import "LoginController.h"
#import "AppDelegate.h"
#import "Utility.h"


static void ConnectNewUserCallback(GPConnection* connection, GPConnectResponseArg* connectResponse, CreateUserController* controller);
static void ConnectCallback(GPConnection* connection, GPConnectResponseArg* connectResponse, CreateUserController* controller);
static void RegisterUniqueNickCallback(GPConnection* connection, GPRegisterUniqueNickResponseArg* registerResponse, CreateUserController* controller);


@interface CreateUserController(Private) <UITextFieldDelegate>

- (void)doneClicked: (id)sender;
- (void)onLoginTimer: (NSTimer*)timer;
- (void)connectNewUserCallback: (GPResult)result profile: (GPProfile)profile uniquenick: (const char[])uniquenick;
- (void)connectCallback: (GPResult)result profile: (GPProfile)profile uniquenick: (const char[])uniquenick;
- (void)registerUniqueNickCallback: (GPResult)result;

@end


@implementation CreateUserController

@synthesize loginController;
@synthesize connection;
@synthesize profile;
@synthesize autoLoginSwitch;

- (void)dealloc
{
	[autoLoginSwitch release];
	[emailField release];
	[userNameField release];
	[passwordField release];
    [super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}

- (NSString*)uniquenick
{
	return userNameField.text;
}

- (NSString*)password
{
	return passwordField.text;
}

- (IBAction)createButtonClicked: (id)sender
{
	// Check for no account info
	////////////////////////////
	if(([userNameField.text length] == 0) || ([passwordField.text length] == 0))
	{
		MessageBox(@"Please fill in all the account information.");
		return;
	}

	if (gpInitialize(&connection, SCRACE_PRODUCTID, WSLogin_NAMESPACE_SHARED_UNIQUE, GP_PARTNERID_GAMESPY) != GP_NO_ERROR)
	{
		MessageBox(@"Error initializing the login system.");
		return;
	}

	const char* nick = [userNameField.text cStringUsingEncoding: NSASCIIStringEncoding];
	const char* email = [emailField.text cStringUsingEncoding: NSASCIIStringEncoding];
	const char* password = [passwordField.text cStringUsingEncoding: NSASCIIStringEncoding];
	
	GPResult result = gpConnectNewUserA(&connection, nick, nick, email, password, NULL, GP_FIREWALL, GP_NON_BLOCKING, (GPCallback)ConnectNewUserCallback, self);

	if (result != GP_NO_ERROR) {
		char errorString[GP_ERROR_STRING_LEN];

		switch (result) {
			case GP_PARAMETER_ERROR:
				gpGetErrorStringA(&connection, errorString);
				MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding], @"Invalid Data");
				//MessageBox(@"One of the input fields contains invalid data.");
				break;

			default:
				gpGetErrorStringA(&connection, errorString);
				MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding]);
		}

		gpDisconnect(&connection);
		gpDestroy(&connection);
		return;
	}

	loginTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onLoginTimer:) userInfo: nil repeats: YES];
}

- (IBAction)returnClicked: (id)sender
{
	UITextField* textFields[] = { emailField, userNameField, passwordField };
	size_t textFieldCount = sizeof(textFields) / sizeof(textFields[0]);
	size_t textFieldIndex = 0;
	
	// Find the text field that generated the event.
	while (textFields[textFieldIndex] != sender)
		textFieldIndex++;
	
	// Cycle focus to the next text field.
	if (++textFieldIndex < textFieldCount) {
		[textFields[textFieldIndex] becomeFirstResponder];
	}
	else {
		[(UIResponder*)sender resignFirstResponder];
	}
}

@end


@implementation CreateUserController(Private)

- (void)textFieldDidBeginEditing: (UITextField*)textField
{
	self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemDone target: self action: @selector(doneClicked:)];
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	self.navigationItem.rightBarButtonItem = nil;
}

- (void)doneClicked: (id)sender
{
	UITextField* textFields[] = { emailField, userNameField, passwordField };
	size_t textFieldCount = sizeof(textFields) / sizeof(textFields[0]);
	
	for (size_t n = 0; n < textFieldCount; n++) {
		[textFields[n] resignFirstResponder];
	}
}

- (void)onLoginTimer: (NSTimer*)timer
{
	gpProcess(&connection);
}

- (void)connectNewUserCallback: (GPResult)result profile: (GPProfile)userProfile uniquenick: (const char[])uniquenick
{
	[loginTimer invalidate];
	loginTimer = nil;

	if (result != GP_NO_ERROR) {
		GPErrorCode errorCode;
		char errorString[GP_ERROR_STRING_LEN];
		gpGetErrorCode(&connection, &errorCode);

		switch (errorCode) {
			case GP_NEWUSER_BAD_PASSWORD:
				MessageBox(@"There is an existing account with the given e-mail address, but the password does not match.  To use your existing "
						   @"GameSpy account, provide the correct password for the account.", @"Invalid Password");
				break;

			case GP_NEWUSER_BAD_NICK:
			{
				// This may mean that the user already has a profile with this nick; try to connect to it, and then try
				// to register a corresponding uniquenick.
				const char* nick = [userNameField.text cStringUsingEncoding: NSASCIIStringEncoding];
				const char* email = [emailField.text cStringUsingEncoding: NSASCIIStringEncoding];
				const char* password = [passwordField.text cStringUsingEncoding: NSASCIIStringEncoding];

				gpDestroy(&connection);
				gpInitialize(&connection, SCRACE_PRODUCTID, WSLogin_NAMESPACE_SHARED_UNIQUE, GP_PARTNERID_GAMESPY);
				gpConnectA(&connection, nick, email, password, GP_FIREWALL, GP_NON_BLOCKING, (GPCallback)ConnectCallback, self);
				loginTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onLoginTimer:) userInfo: nil repeats: YES];
				return;
			}

			default:
				gpGetErrorStringA(&connection, errorString);
				MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding], @"Try Again" );
		}

		gpDisconnect(&connection);
		gpDestroy(&connection);
		return;
	}

	profile = userProfile;
	[loginController userProfileCreated: self autoLogin: autoLoginSwitch.on];
}

- (void)connectCallback: (GPResult)result profile: (GPProfile)userProfile uniquenick: (const char[])uniquenick
{
	if (result != GP_NO_ERROR) {
		char errorString[GP_ERROR_STRING_LEN];
		gpGetErrorStringA(&connection, errorString);
		MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding]);
		gpDisconnect(&connection);
		gpDestroy(&connection);
		[loginTimer invalidate];
		loginTimer = nil;
		return;
	}

	profile = userProfile;
	gpRegisterUniqueNickA(&connection, [userNameField.text cStringUsingEncoding: NSASCIIStringEncoding], NULL, GP_NON_BLOCKING, (GPCallback)RegisterUniqueNickCallback, self);
}

- (void)registerUniqueNickCallback: (GPResult)result
{
	[loginTimer invalidate];
	loginTimer = nil;

	if (result != GP_NO_ERROR) {
		char errorString[GP_ERROR_STRING_LEN];
		gpGetErrorStringA(&connection, errorString);
		MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding]);
		gpDisconnect(&connection);
		gpDestroy(&connection);
		return;
	}

	[loginController userProfileCreated: self autoLogin: autoLoginSwitch.on];
}

@end


void ConnectNewUserCallback(GPConnection* connection, GPConnectResponseArg* connectResponse, CreateUserController* controller)
{
	[controller connectNewUserCallback: connectResponse->result profile: connectResponse->profile uniquenick: connectResponse->uniquenick];
}

void ConnectCallback(GPConnection* connection, GPConnectResponseArg* connectResponse, CreateUserController* controller)
{
	[controller connectCallback: connectResponse->result profile: connectResponse->profile uniquenick: connectResponse->uniquenick];
}

void RegisterUniqueNickCallback(GPConnection* connection, GPRegisterUniqueNickResponseArg* registerResponse, CreateUserController* controller)
{
	[controller registerUniqueNickCallback: registerResponse->result];
}

