// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "LoginController.h"
#import "AppDelegate.h"
#import "Utility.h"
#import "CreateUserController.h"
#import "ForgotPassword.h"
#import "InfoController.h"

#import "common/gsAvailable.h"
#import "webservices/AuthService.h"

static NSString* animationId = @"UserNamePickerAnimation";

//static NSString* loginFileName = @"login.plist";
//static NSString* lastLoginKey = @"lastLogin";
//static NSString* passwordsByLoginKey = @"passwordsByLogin";

static NSString* useridKey = @"useridKey";
static NSString* passwordKey = @"passwordKey";
static NSString* autologinKey = @"autologinKey";




@interface LoginController(Private) <UIAlertViewDelegate, UITextFieldDelegate>

- (void)doneClicked: (id)sender;
- (void)onLoginTimer: (NSTimer*)timer;
- (void)loginCallback: (GPResult)result profile: (GPProfile)profile uniquenick: (const char[])uniquenick;
- (void)loginCallback: (GHTTPResult)result certificate: (GSLoginCertificate*)certificate privateData: (GSLoginPrivateData*)privateData;
- (void)animationDidStop: (NSString*)animId finished: (BOOL)finished context: (id)context;
- (void)closeUserNamePicker;

@end

static void gpLoginCallback(GPConnection* connection, GPConnectResponseArg* arg, void* param);
static void gpUserKickedOutCallback(GPConnection* connection, void* arg, void* param);
static void wsLoginCallback(GHTTPResult httpResult, WSLoginResponse* response, void* userData);

@implementation LoginController

- (void)dealloc
{
	[userNameField release];
	[passwordField release];
	[autoLoginSwitch release];
	[activityIndicator release];
	[userNamePicker release];

	[super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
	// Return YES for supported orientations
	return interfaceOrientation == UIInterfaceOrientationPortrait;
}

/*- (void)viewDidLoad
{
/*	if (![self isNetworkAvailableFlags:nil]) 
	{
		MessageBox( @"A network connection is not available.  Application will close.", @"OK" );
		exit(0);
	}
*/	

//static NSMutableString* errorMsg = @"error";
// called after a delay to upload a new image from the image picker...
//  gives the mainloop a chance to switch views and render the leaders menu again before the upload starts
- (void)delayedNetworkConnectionErrorMessage
{
	lostConnection = true;
	//MessageBox( errMsg, @"Network Error" );
	MessageBox(@"Unable to establish connection. Please connect to a different WiFi network or switch to 3G.", @"Network Error");
}
	
	
- (void)viewDidLoad
{
	// Basic initialization.
	alertView		= nil;
	sdkInitialized  = false;
	lostConnection  = false;
	
	[appDelegate initGame];
	
	// load login info from user defaults
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	userNameField.text = [ defaults stringForKey: useridKey ];
	passwordField.text = [ defaults stringForKey: passwordKey ];
	autoLoginSwitch.on = [ defaults boolForKey: autologinKey ];
		
	// Seed the random number generator (for generating cookie values for nat negotiation).
	time_t currentTime;
	time(&currentTime);
	srand(currentTime);

	// Check Network connectivity
	if (![self isNetworkAvailableFlags:nil]) 
	{
		// display error message in a second to give the system time to display the login page
		[self performSelector:@selector(delayedNetworkConnectionErrorMessage) withObject:nil afterDelay:1.0f];
		return;
	}
	
	// Init gsCore, Competition & Sake SDKs
	[appDelegate initSake ];
	
	// Login if auto login is true
	if( autoLoginSwitch.on )
	{
		[ self loginClicked: nil ];
	}
}


/*- (void)viewWillAppear: (BOOL)animated
{
	if (![self isNetworkAvailableFlags:nil]) 
	{
		MessageBox( @"A network connection is not available.  Application will close.", @"OK" );
		exit(0);
	}	
}
*/

/*	
- (void)viewDidLoad
{
	if (![self isNetworkAvailableFlags:nil]) {
		MessageBox( @"A network connection is not available.  Application will close.", @"No Network" );
		exit(0);
	}
	// load login info
	//////////////////
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	if ([paths count] > 0) {
		NSString* documentsPath = (NSString*)[paths objectAtIndex: 0];
		NSString* loginFilePath = [documentsPath stringByAppendingPathComponent: loginFileName];
		NSDictionary* loginDictionary = [NSDictionary dictionaryWithContentsOfFile: loginFilePath];

		if (loginDictionary != nil) {
			NSString* lastLogin = (NSString*)[loginDictionary objectForKey: lastLoginKey];

			if (lastLogin != nil) {
				autoLoginSwitch.on = YES;
				NSDictionary* passwordsByLogin = (NSDictionary*)[loginDictionary objectForKey: passwordsByLoginKey];

				if (passwordsByLogin != nil) {
					NSString* password = [passwordsByLogin objectForKey: lastLogin];

					if (password != nil) {
						userNameField.text = lastLogin;
						passwordField.text = password;
					}
				}
			}
		}
	}
}
*/
	
- (NSInteger)numberOfComponentsInPickerView: (UIPickerView*)pickerView
{
	return 1;
}

- (NSInteger)pickerView: (UIPickerView*)pickerView numberOfRowsInComponent: (NSInteger)component
{
	return [rememberedLogins count] + 1;
}

- (NSString*)pickerView: (UIPickerView*)pickerView titleForRow: (NSInteger)row forComponent: (NSInteger)component
{
	if (row < (int) ([rememberedLogins count]) )
	
	{
		return (NSString*)[rememberedLogins objectAtIndex: row];
	}

	return @"<Enter Name>";
}

- (void)pickerView: (UIPickerView*)pickerView didSelectRow: (NSInteger)row inComponent: (NSInteger)component
{
	if (row < (int) ([rememberedLogins count]) ) 
	{
		NSString* login = (NSString*)[rememberedLogins objectAtIndex: row];
		userNameField.text = login;
		passwordField.text = (NSString*)[rememberedPasswordsByLogin objectForKey: login];
		[autoLoginSwitch setOn: YES animated: YES];
	}
	else {
		userNameField.text = nil;
		passwordField.text = nil;
		[autoLoginSwitch setOn: NO animated: YES];
	}

}

// Perform a reachability query for the address 0.0.0.0. If that address is reachable without
// requiring a connection, a network interface is available. We'll have to do more work to
// determine which network interface is available.
- (BOOL)isNetworkAvailableFlags:(SCNetworkReachabilityFlags *)outFlags
{
	SCNetworkReachabilityRef defaultRouteReachability;
	struct sockaddr_in zeroAddress;
	bzero(&zeroAddress, sizeof(zeroAddress));
	zeroAddress.sin_len = sizeof(zeroAddress);
	zeroAddress.sin_family = AF_INET;
		       
        defaultRouteReachability = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&zeroAddress);

	SCNetworkReachabilityFlags flags;
	BOOL gotFlags = SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags);
	if (!gotFlags) 
		return NO;
	
	BOOL isReachable = flags & kSCNetworkReachabilityFlagsReachable;
	
	// This flag indicates that the specified nodename or address can
	// be reached using the current network configuration, but a
	// connection must first be established.
	//
	// If the flag is false, we don't have a connection. But because CFNetwork
	// automatically attempts to bring up a WWAN connection, if the WWAN reachability
	// flag is present, a connection is not required.
	BOOL noConnectionRequired = !(flags & kSCNetworkReachabilityFlagsConnectionRequired);
	if ((flags & kSCNetworkReachabilityFlagsIsWWAN)) {
		noConnectionRequired = YES;
	}
	
	isReachable = (isReachable && noConnectionRequired) ? YES : NO;
	
	// Callers of this method might want to use the reachability flags, so if an 'out' parameter
	// was passed in, assign the reachability flags to it.
	if (outFlags) {
		*outFlags = flags;
	}
	
	return isReachable;
}

- (IBAction)loginClicked: (id)sender
{	
	[appDelegate initGame ];
	[appDelegate initSake ];

	// Check for no account info
	if(([userNameField.text length] == 0) || ([passwordField.text length] == 0))
	{
		MessageBox(@"Please fill in all the account information.");
		return;
	}
		
	// Initialize GP
	GPResult gpresult = gpInitialize(&connection, SCRACE_PRODUCTID, WSLogin_NAMESPACE_SHARED_UNIQUE, GP_PARTNERID_GAMESPY);
	if( gpresult == GP_MEMORY_ERROR )
	{
		MessageBox(@"Could not initialize the login system", @"Memory Error");
		return;
	}
	else if( gpresult == GP_PARAMETER_ERROR)
	{
		MessageBox(@"Could not initialize the login system", @"Parameter Error");
		return;
	}
	else if( gpresult == GP_SERVER_ERROR)
	{
		MessageBox(@"Could not initialize the login system", @"Server Error");
		return;
	}
	else if( gpresult == GP_NETWORK_ERROR)
	{
		MessageBox(@"Could not initialize the login system", @"Network Error");		
		return;
	}
	else if( gpresult != GP_NO_ERROR)
	{
		MessageBox(@"Error initializing the login system");
		return;
	}

	// set callback to be notified if same user logs into another device
	gpSetCallback(&connection, GP_ERROR , (GPCallback)gpUserKickedOutCallback, self);
	
	// Connect to the account specified
	const char* userName = [userNameField.text cStringUsingEncoding: NSASCIIStringEncoding];
	const char* password = [passwordField.text cStringUsingEncoding: NSASCIIStringEncoding];

	GPResult result = gpConnectUniqueNickA(&connection, userName, password, GP_FIREWALL, GP_NON_BLOCKING, (GPCallback)gpLoginCallback, self);
	if (result != GP_NO_ERROR)
	{
		switch (result)
		{
			case GP_PARAMETER_ERROR:
				MessageBox(@"Please enter a valid user name and password.");
				return;

			default:
				MessageBox(@"Login failed. Try again when network connection is up", @"Connection Error");
				return;
		}
	}

	alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Logging in..." delegate: self cancelButtonTitle: @"Cancel" otherButtonTitles: nil];
	[alertView show];
	loginTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onLoginTimer:) userInfo: nil repeats: YES];
	[activityIndicator startAnimating];
}

- (IBAction)returnClicked: (id)sender
{
	UITextField* textFields[] = { userNameField, passwordField };
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
	//	[(UIResponder*)sender resignFirstResponder];
	}
}


- (IBAction)createClicked: (id)sender
{
	CreateUserController* createUserController = [[CreateUserController alloc] initWithNibName: @"createuser" bundle: nil];
	[self.navigationController pushViewController: createUserController animated: YES];
	createUserController.loginController = self;
	createUserController.autoLoginSwitch.on = autoLoginSwitch.on; // use same setting as on login
}

- (IBAction)forgotPasswordClicked: (id)sender
{
	ForgotPassword* forgotPasswordController = [[ForgotPassword alloc] initWithNibName: @"forgotPassword" bundle: nil];
	[self.navigationController pushViewController: forgotPasswordController animated: YES];
}
	
- (IBAction)autoLoginChanged: (id)sender
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[ defaults setBool: autoLoginSwitch.on forKey: autologinKey ];
}

/*
- (IBAction)autoLoginChanged: (id)sender
{
	if (!autoLoginSwitch.on) {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		
		if ([paths count] > 0) {
			NSString* documentsPath = (NSString*)[paths objectAtIndex: 0];
			NSString* loginFilePath = [documentsPath stringByAppendingPathComponent: loginFileName];
			NSMutableDictionary* loginDictionary = [NSMutableDictionary dictionaryWithContentsOfFile: loginFilePath];
			
			if (loginDictionary != nil) {
				[loginDictionary removeObjectForKey: lastLoginKey];

				NSMutableDictionary* passwordsByLogin = [NSMutableDictionary dictionaryWithDictionary: (NSDictionary*)[loginDictionary objectForKey: passwordsByLoginKey]];

				if (passwordsByLogin != nil) {
					[passwordsByLogin removeObjectForKey: userNameField.text];
					[loginDictionary setObject: passwordsByLogin forKey: passwordsByLoginKey];
				}

				[loginDictionary writeToFile: loginFilePath atomically: YES];
			}
		}
	}
}
*/
/*- (IBAction)autoLoginChanged: (id)sender
{
	if (!rememberMeSwitch.on) {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		
		if ([paths count] > 0) {
			NSString* documentsPath = (NSString*)[paths objectAtIndex: 0];
			NSString* loginFilePath = [documentsPath stringByAppendingPathComponent: loginFileName];
			NSMutableDictionary* loginDictionary = [NSMutableDictionary dictionaryWithContentsOfFile: loginFilePath];
			
			if (loginDictionary != nil) {
				[loginDictionary removeObjectForKey: lastLoginKey];
				
				NSMutableDictionary* passwordsByLogin = [NSMutableDictionary dictionaryWithDictionary: (NSDictionary*)[loginDictionary objectForKey: passwordsByLoginKey]];
				
				if (passwordsByLogin != nil) {
					[passwordsByLogin removeObjectForKey: userNameField.text];
					[loginDictionary setObject: passwordsByLogin forKey: passwordsByLoginKey];
				}
				
				[loginDictionary writeToFile: loginFilePath atomically: YES];
			}
		}
	}
}
*/


- (void)userProfileCreated: (CreateUserController*)createUserController autoLogin: (BOOL) autoLoginValue
{
	// if the autoLogin flag is true, turn on the switch first
	autoLoginSwitch.on = autoLoginValue;
		
	// These objects disappear as soon as we pop the view controller.
	// Popping the view conroller will trigger the viewWillAppear message, which calls reset, which
	// destroys the login controller's connection object.  Since we want to preserve the connection
	// object created by the createUserController, assign it to a temporary variable rather than to
	// self->connection.
	GPConnection tempConnection = createUserController.connection;
	userNameField.text = createUserController.uniquenick;
	passwordField.text = createUserController.password;

	// This line destroys createUserController and triggers the viewWillAppear message.
	[self.navigationController popToViewController: self animated: YES];

	// By this point, viewWillAppear has been called, so it should now be safe to assign the connection
	// object.
	connection = tempConnection;

	// Continue the login process as usual.
	loginTimer = [NSTimer scheduledTimerWithTimeInterval: 0.050 target: self selector: @selector(onLoginTimer:) userInfo: nil repeats: YES];
	[self loginCallback: GP_NO_ERROR profile: createUserController.profile uniquenick: [userNameField.text cStringUsingEncoding: NSASCIIStringEncoding]];
}

- (void)reset
{
	gpDisconnect(&connection);
	gpDestroy(&connection);
	
	// Basic initialization.
	alertView		= nil;
	sdkInitialized  = false;
	lostConnection  = false;
	
	// clean up gPlayerData ... PlayerData should be promoted to a full object!
	[gPlayerData.uniquenick release];
	gPlayerData.uniquenick = nil;
	[gPlayerData.thumbNail release];
	gPlayerData.thumbNail = nil;
	[gPlayerData.fullsizeImage release];
	gPlayerData.fullsizeImage = nil;
	[gPlayerData.playerLocation release];
	gPlayerData.playerLocation = nil;
	[gPlayerData.playerDataPath release];
	gPlayerData.playerDataPath = nil;
	[gPlayerData.playerStatsData release];
	gPlayerData.playerStatsData = nil;
	
	// clean up gOpponentData 
	[gOpponentData.uniquenick release];
	gOpponentData.uniquenick = nil;
	[gOpponentData.thumbNail release];
	gOpponentData.thumbNail = nil;
	[gOpponentData.fullsizeImage release];
	gOpponentData.fullsizeImage = nil;
	[gOpponentData.playerLocation release];
	gOpponentData.playerLocation = nil;
	[gOpponentData.playerDataPath release];
	gOpponentData.playerDataPath = nil;
	[gOpponentData.playerStatsData release];
	gOpponentData.playerStatsData = nil;
	
	// clean up leaderboards
	[appDelegate deleteLeaderboards];
}

- (GPConnection *)getGPConnectionPtr
{
	return &connection;
}


- (void)stopLoginTimer
{
	if (loginTimer != nil) 
	{
		[loginTimer invalidate];
		loginTimer = nil;
	}
}

@end


@implementation LoginController(Private)

- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
	if (buttonIndex == alertView.cancelButtonIndex) 
	{
		[loginTimer invalidate];
		gpDisconnect(&connection);
		gpDestroy(&connection);
		
		[activityIndicator stopAnimating];
	}
}

- (BOOL)textFieldShouldBeginEditing: (UITextField*)textField
{
/*	if (textField == userNameField) {
		if (userNamePickerView.hidden) {
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

			if ([paths count] > 0) {
				NSString* documentsPath = (NSString*)[paths objectAtIndex: 0];
				NSString* loginFilePath = [documentsPath stringByAppendingPathComponent: loginFileName];
				NSDictionary* loginDictionary = [NSDictionary dictionaryWithContentsOfFile: loginFilePath];
				
				if (loginDictionary != nil) {
					// Released when the user disimisses the picker view.
					rememberedPasswordsByLogin = [(NSDictionary*)[loginDictionary objectForKey: passwordsByLoginKey] retain];

					if (rememberedPasswordsByLogin != nil) {
						// Released when the user disimisses the picker view.
						rememberedLogins = [[rememberedPasswordsByLogin allKeys] retain];

						if ([rememberedLogins count] > 0) {
							[userNamePicker reloadAllComponents];
							NSUInteger currentIndex = [rememberedLogins indexOfObject: userNameField.text];

							if (currentIndex == NSNotFound) {
								currentIndex = [rememberedLogins count];
							}

							[userNamePicker selectRow: currentIndex inComponent: 0 animated: NO];

							[passwordField resignFirstResponder];
							self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemDone target: self action: @selector(closeUserNamePicker)] autorelease];
							userNamePickerView.alpha = 0.0f;
							userNamePickerView.hidden = NO;
							
							[UIView beginAnimations: animationId context: nil];
							userNamePickerView.alpha = 1.0f;
							[UIView commitAnimations];
							return NO;
						}
					}
				}
			}
		}
	}
*/
	return YES;
}

- (void)textFieldDidBeginEditing: (UITextField*)textField
{
	//self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemDone target: self action: @selector(doneClicked:)] autorelease];
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	//self.navigationItem.rightBarButtonItem = nil;
}

- (void)doneClicked: (id)sender
{
	UITextField* textFields[] = { userNameField, passwordField };
	size_t textFieldCount = sizeof(textFields) / sizeof(textFields[0]);

	for (size_t n = 0; n < textFieldCount; n++) {
		[textFields[n] resignFirstResponder];
	}
}

- (void)animationDidStop: (NSString*)animId finished: (BOOL)finished context: (id)context
{
	if ([userNamePicker selectedRowInComponent: 0] == [userNamePicker numberOfRowsInComponent: 0] - 1) {
		[userNameField becomeFirstResponder];
	}

	userNamePickerView.hidden = YES;
}

- (void)closeUserNamePicker
{
	[rememberedLogins release];
	[rememberedPasswordsByLogin release];

	//self.navigationItem.rightBarButtonItem = nil;

	[UIView beginAnimations: animationId context: nil];
	[UIView setAnimationDelegate: self];
	[UIView setAnimationDidStopSelector: @selector(animationDidStop:finished:context:)];
	userNamePickerView.alpha = 0.0f;
	[UIView commitAnimations];
}

- (void)onLoginTimer: (NSTimer*)timer
{
	//GPEnum connected;
	
	// We are still using core!
	// The logic here is to think when the core is valid
	// the core is valid
	//		1) GSCore_IN_USE
	//		2) GSCore_SHUTTING_DOWN
	
	// We may not have started using core also
	if (gsCoreIsShutdown() != GSCore_SHUTDOWN_COMPLETE)
		gsCoreThink(0);

	if (connection)
		gpProcess(&connection);	
}

- (void)loginCallback: (GPResult)result profile: (GPProfile)profile uniquenick: (const char[])uniquenick
{
	[alertView dismissWithClickedButtonIndex: alertView.firstOtherButtonIndex animated: YES];

	if (result != GP_NO_ERROR) {
		[loginTimer invalidate];
		loginTimer = nil;
		[activityIndicator stopAnimating];
		char errorString[GP_ERROR_STRING_LEN];
		gpGetErrorStringA(&connection, errorString);
		MessageBox([NSString stringWithCString: errorString encoding: NSASCIIStringEncoding], @"Login Failed");
		return;
	}

	// Save the login info and autosave flag for next time
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[ defaults setObject: userNameField.text forKey: useridKey ];
	[ defaults setObject: passwordField.text forKey: passwordKey ];
	[ defaults setBool: autoLoginSwitch.on forKey: autologinKey ];
	
	// Store the profile ID for the player
	//////////////////////////////////////
	gPlayerData.uniquenick = [[NSString alloc] initWithCString: uniquenick encoding: NSASCIIStringEncoding];
	gPlayerData.profileId = profile;
	gpGetLoginTicket(&connection, gPlayerData.loginTicket);

	NSLog(@"login ticket[%s]", gPlayerData.loginTicket);
	
	const char* password = [passwordField.text cStringUsingEncoding: NSASCIIStringEncoding];
	gsi_u32 wsLoginResult = wsLoginUnique(WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, uniquenick, password, NULL, wsLoginCallback, self);

	if (wsLoginResult != WSLogin_Success) 
	{
		switch (wsLoginResult) 
		{
			// Just output same error message for all login error type
			default:
				MessageBox(@"Failed to login to authentication service.", @"Login Failed");
				break;
		}

		[loginTimer invalidate];
		loginTimer = nil;
		[activityIndicator stopAnimating];
		gpDisconnect(&connection);
		gpDestroy(&connection);
	}
}

- (void)loginCallback: (GHTTPResult)result certificate: (GSLoginCertificate*)certificate privateData: (GSLoginPrivateData*)privateData
{

	if (result != GHTTPSuccess) 
	{
		[activityIndicator stopAnimating];
		switch (result) 
		{
			// Just output same error message for all login error type
			default:
				MessageBox(@"Failed to login to authentication service.", @"Login Failed");
				break;
		}

		[loginTimer invalidate];
		loginTimer = nil;
		
		gpDisconnect(&connection);
		gpDestroy(&connection);

		return;
	}

	// Store the Login Certificate & Private Data for later use w/ Competition
	//////////////////////////////////////////////////////////////////////////
	memcpy(&gPlayerData.certificate, certificate, sizeof(GSLoginCertificate));
	memcpy(&gPlayerData.privateData, privateData, sizeof(GSLoginPrivateData));

	[activityIndicator stopAnimating];
	[appDelegate loggedIn];
}

@end


void gpUserKickedOutCallback( GPConnection* connection, void* arg, void* param )
{
	GPErrorCode err = ((GPErrorArg*)arg)->errorCode;
	if( err == GP_FORCED_DISCONNECT || err == GP_CONNECTION_CLOSED )
	{
	//	LoginController* loginController = (LoginController*)[appDelegate findExistingControllerOfType: [LoginController class]];
	//	[loginController stopLoginTimer];
		
		[appDelegate kickedOut];
		NSLog(@"User logged into other device.\n");
		MessageBox( @"You have been disconnected because another player has logged in with your ID.", @"Logged Out" );
		[appDelegate logout];
	}
}


void gpLoginCallback(GPConnection* connection, GPConnectResponseArg* arg, void* param)
{
	LoginController* loginController = (LoginController*)param;
	[loginController loginCallback: arg->result profile: arg->profile uniquenick: arg->uniquenick];
}

void wsLoginCallback(GHTTPResult httpResult, WSLoginResponse* response, void* userData)
{
	LoginController* loginController = (LoginController*)userData;
	[loginController loginCallback: httpResult certificate: &response->mCertificate privateData: &response->mPrivateData];
}
