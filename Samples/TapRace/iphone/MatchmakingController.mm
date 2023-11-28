// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

// Matchmaking is for selecting/looking for a server (aka player) to play a game with 

#import "MatchmakingController.h"
#import "LoginController.h"
#import "AppDelegate.h"
#import "Utility.h"

#import "qr2/qr2.h"

// Values for detecting shake
#define kAccelerometerFrequency		25	//Hz
#define kFilteringFactor			0.1
#define kMinShakeInterval			0.5
#define kShakeAccelerationThreshold	3.0

// Structure to hold server connection info
struct ServerData
{
	NSString*		name;		// Server name
	unsigned int	address;	// IP address
	unsigned short	port;		// port to connect on
	bool			useNatNeg;	// true if IP is not local and NAT negotiation is required
};


// Server Browsing Callback
static void SBCallback(ServerBrowser sb, SBCallbackReason reason, SBServer server, void* instance);

static bool sbeError = false;


@interface MatchmakingController(Private) <UIPickerViewDelegate, UIPickerViewDataSource>

@end


@implementation MatchmakingController

static gsi_bool sbUpdateFinished = gsi_false; // true when query for list of servers is complete 

// Release objects in page & server list
// =====================================
- (void)dealloc
{
	for (std::vector<ServerData>::iterator i = servers.begin(); i != servers.end(); ++i) {
		[i->name release];
	}
	
	[gamePicker release];
	[joinButton release];
	[hostButton release];
	[shakeLabel release];

    [super dealloc];
}


// Do not support auto-rotate
// =====================================
- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
    // Support portrait orientation only
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}


// When page will appear
//   - setup GUI logic
//   - start query for list of servers
// =====================================
- (void)viewWillAppear: (BOOL)animated
{
	// tell host to quit game if I just came from gameController
	[appDelegate stopHostingServer];
			
	// setup GUI
	self.title = @"MatchMaking"; 
	joinButton.enabled = NO;
	//[RefreshButton setTitle: @"Refresh" forState: UIControlStateNormal ];
	//[RefreshButton setTitle: @"Refreshing" forState: UIControlStateHighlighted  ];
	
	// Query server list
	ServerBrowser serverBrowser = ServerBrowserNewA(SCRACE_GAMENAME, SCRACE_GAMENAME, SCRACE_SECRETKEY, 0, MAX_SERVERS, QVERSION_QR2, SBFalse, SBCallback, self);
	appDelegate.serverBrowser = serverBrowser;
	
	unsigned char basicFields[] = { HOSTNAME_KEY, HOSTPORT_KEY };
	size_t basicFieldCount = sizeof(basicFields) / sizeof(basicFields[0]);
	sbUpdateFinished = gsi_false;
	
	// Get available servers 
	const char freeServersFilter[] = {"numplayers < maxplayers" };
	sbeError = false;
	SBError result = ServerBrowserLimitUpdate(serverBrowser, SBTrue, SBTrue, basicFields, basicFieldCount, freeServersFilter, MAX_SERVERS);
	
	if (result != sbe_noerror) 
	{
		sbeError = true;
		
	//	MessageBox(@"Error communicating with GameSpy master server.");
	//	ServerBrowserFree(serverBrowser);
		return;
	}
	
	//Configure and enable the accelerometer
	[UIAccelerometer sharedAccelerometer].updateInterval = (1.0 / kAccelerometerFrequency);
	[UIAccelerometer sharedAccelerometer].delegate = self;
}

- (void)viewDidAppear: (BOOL)animated
{
	if ( sbeError ) 
	{
		//MessageBox(@"Error retrieving Games.", @"Connection Error");
		//ServerBrowserFree(appDelegate.serverBrowser);
		return;
	}

}



// When the page will disappear
//    - release servers
//    - stop hosting timer if any
//    - stop check for shake
// =====================================
- (void)viewWillDisappear: (BOOL)animated
{
	// Clear server list in GUI
	for (std::vector<ServerData>::iterator i = servers.begin(); i != servers.end(); ++i) 
	{
		[i->name release];
	}
	servers.clear();

	// Stop hosting if it was
	[appDelegate stopHostingCountdownTimer];
	
	// Clear server list in structure
	ServerBrowser serverBrowser = appDelegate.serverBrowser;
	if (serverBrowser != NULL) 
	{
		ServerBrowserFree(serverBrowser);
		appDelegate.serverBrowser = NULL;
	}

	// Stop checking for shake when close matchmaking page 
	UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
	if (accelerometer.delegate == self) 
		accelerometer.delegate = nil;
}


// When Join button is pressed
//    - update GUI logic
//    - update server list to see is selected server is still available
//    - if it is, initiate join to remote server 
// =====================================
- (IBAction)joinButtonClicked: (id)sender
{
	// disable join & accelarometer
	[self enableShake:NO];
	joinButton.enabled = NO;
	
	// get selected player name from list
	NSInteger index = [gamePicker selectedRowInComponent: 0];
	NSString *selectedServerName = servers.at(index).name; ;
	NSString *serverName = [[NSString alloc] initWithString: selectedServerName ] ;
	
	// renew server list
	[self refreshButtonClicked: sender ];
	
	// wait for server list to update
	while ((ServerBrowserThink(appDelegate.serverBrowser) == sbe_noerror) && (sbUpdateFinished == gsi_false))
		msleep(10);
	
	index = -1;
	BOOL foundServer = NO;
	
	// get selected server name from renewed list & highlight selected server
	for( std::vector<ServerData>::iterator i = servers.begin(); i != servers.end(); ++i) 
	{
		index++;
		if( [serverName compare: i->name] == NSOrderedSame )
		{	
			foundServer = YES;
			[gamePicker selectRow: index inComponent: 0 animated: YES ];
			break;
		}
	}	
			
	// if server name is not found, popup error
	if( foundServer == NO )
	{
		MessageBox(@"is not hosting anymore. Select another player.", serverName);
		[self enableShake:YES];
		joinButton.enabled = NO;
		return;
	}
			
	// else join match

	// get selected player from list
	index = [gamePicker selectedRowInComponent: 0];
	ServerData* data = &servers[index];

	// connect to host player
	// if not connected or join canceled, send NN cancel to host server
	if( [appDelegate joinGame: data->address port: data->port useNatNeg: data->useNatNeg] == false )
	{
		[self enableShake:YES];
		joinButton.enabled = NO;
		//SBSendMessageToServer(sbServer, data->address, data->port, "CANCEL_CONNECTION", <#int len#>);
	}
}


// Called when Host button clicked
//    - setup GUI
//    - initiate registration of host with GameSpy server
// =====================================
- (IBAction)hostButtonClicked: (id)sender
{
	// turn off the join button and accelerometer
    joinButton.enabled = NO;
	[ self enableShake: NO];
	
	if ( ! [appDelegate hostGame] )
	{
		[self hostingCanceled ];
	}
}


// Called when user cancels Hosting or timeout
//     - release host countdwon timer
//	   - setup GUI logic
// =====================================
- (void)hostingCanceled
{
	[appDelegate stopHostingCountdownTimer];
	
	if( servers.size() > 0 )
		joinButton.enabled = YES;
	
	[ self enableShake: YES];
}


// Enables/disables shake detection
// =====================================
- (void)enableShake: (bool) enable
{
	if( enable )
	{
		shakeLabel.hidden = NO;
		[UIAccelerometer sharedAccelerometer].delegate = self;
	}
	else
	{
		shakeLabel.hidden = YES;
		
		UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
		if (accelerometer.delegate == self) 
			accelerometer.delegate = nil;
	}
}


// Called when user cancels Hosting or timeout
//     - reload server list in GUI
//	   - setup GUI logic
// =====================================
- (void)reset
{
	[appDelegate stopHostingCountdownTimer];
	
	// get new server list
	[gamePicker reloadAllComponents];
	
	[ self enableShake: YES];

	// enable join if there are servers
	joinButton.enabled = ( servers.size() > 0 );
}

//
// =====================================
- (void)addServer: (NSString*)name address: (unsigned int)address port: (unsigned short)port useNatNeg: (bool)useNatNeg
{
	servers.push_back((ServerData){ [name copy], address, port, useNatNeg });
}


// =====================================
- (void)updateServer: (NSString*)name address: (unsigned int)address port: (unsigned short)port
{
	for (std::vector<ServerData>::iterator i = servers.begin(); i != servers.end(); ++i) {
		if ((i->address == address) && (i->port == port)) {
			[i->name release];
			i->name = [name copy];
			break;
		}
	}
}


// =====================================
- (void)removeServer: (unsigned int)address port: (unsigned short)port
{
	for (std::vector<ServerData>::iterator i = servers.begin(); i != servers.end(); ++i) {
		if ((i->address == address) && (i->port == port)) {
			[i->name release];
			servers.erase(i);
			break;
		}
	}
}


// =====================================
- (void)updatePicker
{
	[gamePicker reloadAllComponents];

	listCountLabel.text = [NSString stringWithFormat: @"Games available: %d", servers.size() ];
	
	[self enableShake:YES];
	
	// enable join & shake if there are servers
	if( servers.size() > 0 )
		joinButton.enabled = YES;
}


// Called when the accelerometer detects motion; if shaken with enough force, makes a random selection from the choices.
// =====================================
- (void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
	UIAccelerationValue	length, x, y, z;
	
	//Use a basic high-pass filter to remove the influence of the gravity
	myAccelerometer[0] = acceleration.x * kFilteringFactor + myAccelerometer[0] * (1.0 - kFilteringFactor);
	myAccelerometer[1] = acceleration.y * kFilteringFactor + myAccelerometer[1] * (1.0 - kFilteringFactor);
	myAccelerometer[2] = acceleration.z * kFilteringFactor + myAccelerometer[2] * (1.0 - kFilteringFactor);

	// Compute values for the three axes of the acceleromater
	x = acceleration.x - myAccelerometer[0];
	y = acceleration.y - myAccelerometer[0];
	z = acceleration.z - myAccelerometer[0];
	
	//Compute the intensity of the current acceleration 
	length = sqrt(x * x + y * y + z * z);

	// If above a given threshold, pick a server at random and join
	if( length >= kShakeAccelerationThreshold ) 
	{
		//NSInteger n = [gamePicker numberOfRowsInComponent: 0];
		
		// Update server list if none in it
		if( servers.size() < 1 ) 
		{
			// renew server list
			[self refreshButtonClicked: self ];
			
			// wait for server list to update
			while ((ServerBrowserThink(appDelegate.serverBrowser) == sbe_noerror) && (sbUpdateFinished == gsi_false))
				msleep(10);
		}
		
		// Host if no servers available
		if( servers.size() < 1 )
		{
			[self hostButtonClicked: self ];
		}
		else
		{
			// select random player to join
			int index = (rand() & INT_MAX) % servers.size();
			ServerData* data = &servers[index];
			
			// turn off the join button and accelerometer
			joinButton.enabled = NO;
			[ self enableShake: NO];
			
			// join 
			[appDelegate joinGame: data->address port: data->port useNatNeg: data->useNatNeg];
		}
		
	}
}


// =====================================
- (IBAction)refreshButtonClicked: (id)sender
{
	joinButton.enabled = NO;
	[self viewWillDisappear: NO];	// use the shutdown function to kill servers and picker list
	[self viewWillAppear: NO];		// and pretend we're starting up again
	[self updatePicker];
	
}


/* =====================================
- (void)hostCountdown: (int) count
{
		if (hosting)
	 {
	 alertView.message = @"qwe qwe";
	 }
	 
	 //[gameController countdown: countdown];
	 
}
*/
@end


@implementation MatchmakingController(Private)

- (NSString*)pickerView: (UIPickerView*)pickerView titleForRow: (NSInteger)row forComponent: (NSInteger)component
{
	return servers[row].name;
}

- (void)pickerView: (UIPickerView*)pickerView didSelectRow: (NSInteger)row inComponent: (NSInteger)component
{
}

- (NSInteger)numberOfComponentsInPickerView: (UIPickerView*)pickerView
{
	return 1;
}

- (NSInteger)pickerView: (UIPickerView*)pickerView numberOfRowsInComponent: (NSInteger)component
{
	return servers.size();
}

@end


void SBCallback(ServerBrowser sb, SBCallbackReason reason, SBServer server, void* instance)
{
	MatchmakingController* controller = (MatchmakingController*)instance;
	
	switch (reason) {
		case sbc_serveradded:
		case sbc_serverupdated:
		case sbc_serverupdatefailed:
		case sbc_serverdeleted:
		{
			NSLog(@"SBCallback added/updated/deleted");
			unsigned int address = SBServerGetPublicInetAddress(server);
			unsigned short port = SBServerGetPublicQueryPort(server);
			bool useNatNeg = false;

			// Server is behind a NAT device; check and see if it's local.
			if (SBServerHasPrivateAddress(server) && (address == ServerBrowserGetMyPublicIPAddr(sb))) {
				// Use the private address.
				address = SBServerGetPrivateInetAddress(server);
				port = SBServerGetPrivateQueryPort(server);
			}
			else {
				// We'll have to use NAT negotiation to connect.
				useNatNeg = true;
			}

			
			NSString* name = [NSString stringWithCString: SBServerGetStringValueA(server, qr2_registered_key_list[HOSTNAME_KEY], "Unnamed Game") encoding: NSASCIIStringEncoding];
			
			switch (reason) {
				case sbc_serveradded:
					if( [name compare: gPlayerData.uniquenick ] !=  NSOrderedSame )
					{
						[controller addServer: name address: address port: port useNatNeg: useNatNeg];
						NSLog(@"SB adding");
					}
					break;
					
				case sbc_serverupdated:
					[controller updateServer: name address: address port: port];
					NSLog(@"SB updating");
					break;
					
				case sbc_serverupdatefailed:
				case sbc_serverdeleted:
					[controller removeServer: address port: port];
					NSLog(@"SB deleting");
					break;
					
				case sbc_updatecomplete:
				case sbc_queryerror:
				case sbc_serverchallengereceived:
					NSLog(@"SB nothing to do code=%d", reason);
					break;
			}
			break;
		}
			
		case sbc_updatecomplete:
		NSLog(@"SBCallback sbc_updatecomplete");
			sbUpdateFinished = gsi_true;
			[controller updatePicker];
			break;
			
		case sbc_queryerror:
		NSLog(@"SBCallback sbc_queryerror");
			sbUpdateFinished = gsi_true;
			MessageBox([NSString stringWithCString: ServerBrowserListQueryErrorA(sb) encoding: NSASCIIStringEncoding]);
			break;
			
		case sbc_serverchallengereceived:
		NSLog(@"SBCallback sbc_serverchallengereceived");
			break;
	}
}

