//
//  ForgotPassword.mm
//  Gamespy
//
//  Created by Puap Puap on 10/27/08.
//  Copyright 2008 Pick Up And Play. All rights reserved.
//

#import "ForgotPassword.h"


@implementation ForgotPassword

// Override initWithNibName:bundle: to load the view using a nib file then perform additional customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ( (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) ) 
	{
		NSString* stringurl = @"https://login.gamespyid.com/lostpassword.aspx";
		NSURL * url = [NSURL URLWithString: stringurl];
//		NSURLRequest * urlrequest = [NSURLRequest requestWithURL: url];
//		[webView loadRequest: urlrequest];
		
		[[UIApplication sharedApplication] openURL:url];	
    }
    return self;
}

/*
// Implement loadView to create a view hierarchy programmatically.
- (void)loadView {
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc {
    [super dealloc];
}


@end
