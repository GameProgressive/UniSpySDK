// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import <vector>

@interface MatchmakingController : UIViewController <UIAccelerometerDelegate>
{
	IBOutlet UIPickerView	*gamePicker;
	IBOutlet UIButton	*joinButton;
	IBOutlet UIButton	*hostButton;
	IBOutlet UIButton	*RefreshButton;
	IBOutlet UILabel	*shakeLabel;
	IBOutlet UILabel	*listCountLabel;

	UIAccelerationValue	myAccelerometer[3];	// high pass shake detection

	std::vector<struct ServerData> servers;
}

- (IBAction)joinButtonClicked: (id)sender;
- (IBAction)hostButtonClicked: (id)sender;
- (IBAction)refreshButtonClicked: (id)sender;

//- (void)hostCountdown: (int)count;
- (void)hostingCanceled;
- (void)enableShake: (bool) enable;

- (void)reset;

- (void)addServer: (NSString*)name address: (unsigned int)address port: (unsigned short)port useNatNeg: (bool)useNatNeg;
- (void)updateServer: (NSString*)name address: (unsigned int)address port: (unsigned short)port;
- (void)removeServer: (unsigned int)address port: (unsigned short)port;

- (void)updatePicker;

@end
