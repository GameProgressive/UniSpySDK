//
//  FlipsideViewController.m
//  flipview
//
//  Created by Paul Berlinguette on 6/11/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//
#import "Utility.h"
#import "AppDelegate.h"
#import "FlipsideViewController.h"



@implementation FlipsideViewController

@synthesize delegate;


- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor viewFlipsideBackgroundColor];      
}


- (IBAction)done 
{
	[ appDelegate flipsideViewControllerDidFinish: self.delegate];	
}

#pragma mark User Interface Actions

- (IBAction)contactUs:(id)sender {

	// Warn if cannot send mail
	if(! [MFMailComposeViewController canSendMail] )
	{
		MessageBox( @"Device is unable to send an email at the moment.");
		return;
	}
	
	MFMailComposeViewController *composeVC = [[MFMailComposeViewController alloc] init];
    
    composeVC.mailComposeDelegate = self;
    
    [composeVC setSubject:@"GameSpy TapRace - Request info on iPhone tech"];
        
    NSString *messageBody = [NSString stringWithFormat:@"Hi,\n\n"
											"I am a developer and I have played TapRace on the iPhone and would like more information on GameSpy's social gaming and multi-player services\n\n"
											"Please contact me at xxx-xxx-xxxx or reply to my email.\n\n"
											"Thanks you,\n\n"
											"<My Name>\n<My Company>\n"
							 ];//"Thanks, - %@ by %@\n", title, artist];
    [composeVC setMessageBody:messageBody isHTML:NO];
    
	[composeVC setToRecipients: [NSArray arrayWithObject:@"devrelations@gamespy.com"]];

    [self presentModalViewController:composeVC animated:YES];
    [composeVC release];
}

- (IBAction)clearBlockedImagesFile
{
	NSString* dataPath;
	
	//*** Remove player's block image file
	dataPath = [gPlayerData.playerDataPath stringByAppendingPathComponent: blockedImagesDataFile];
	
	BOOL isDir = NO;
	NSFileManager* fileManager = [NSFileManager defaultManager];
	if ([fileManager fileExistsAtPath: dataPath isDirectory: &isDir]) 
	{
		[fileManager removeItemAtPath: dataPath error: NULL];
		
	}
	
	[appDelegate deleteBlockedImagesData ];
}


- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error {
    [self dismissModalViewControllerAnimated:YES];
}

/*
 // Override to allow orientations other than the default portrait orientation.
 - (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
 // Return YES for supported orientations
 return (interfaceOrientation == UIInterfaceOrientationPortrait);
 }
 */

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

@end
