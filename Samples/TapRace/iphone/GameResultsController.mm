// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "GameResultsController.h"
#import "AppDelegate.h"
#import "Utility.h"

#import <algorithm>

typedef struct serverParam_struct 
{
	GameResultsController * controller;
	int						profileId;
} ServerParam;

// callback prototypes
// http stuff to upload and download image
//void postCallback( GHTTPRequest request, int bytesPosted, int totalBytes, int objectsPosted, int totalObjects, void * param );
//GHTTPBool UploadCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param);
GHTTPBool DownloadImageCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param);

// sake stuff to access and create persistent server storage
//void SakeCreateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
//void SakeUpdateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
//void SakeTestRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
void SakeSearchForImageFileIdRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 

//static SAKEField fields[2];

static bool discardCallbacks = true;




@interface GameResultsController(Private)

- (void)determineWinner;

@end


@implementation GameResultsController

- (void)dealloc
{
	discardCallbacks = true;
	[oldbestLabel release];
	[avgTapsPerSecLabel release];
	[congratsLabel release];
	[newbestLabel release];
	[newbestTimeLabel release];
	[gamesPlayedLabel release];
	[bestTimeLabel release];
	[averageTimeLabel release];
	[localWinnerLabel release];
	[remoteWinnerLabel release];
	[tieLabel release];
	[localPlayerLabel release];
	[localTimeLabel release];
	[localThumbnailButton release];
	[remotePlayerLabel release];
	[remoteTimeLabel release];
	[remoteThumbnailButton release];
	[gameTimeLabel release];
	[leaderboardsButton release];
	[playAgainButton release];
	[menuButton release];
    [super dealloc];
}


// Common controls:
IBOutlet UIButton* leaderboardsButton;
IBOutlet UIButton* playAgainButton;
IBOutlet UIButton* menuButton;

- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}

- (void)viewWillAppear: (BOOL)animated
{
	[super viewWillAppear: animated];
	self.title = @"Race Result";
	//self.backBarButtonItem.title = @"Menu";
	
	//[localThumbnail setImage: gPlayerData.thumbNail ]; 
	//[remoteThumbnail setImage: gPlayerData.thumbNail ];
	self.navigationItem.hidesBackButton = YES;
	discardCallbacks = false;
}


- (void)viewDidLoad
{	
	// set right button to go to menu page
	UIBarButtonItem* aBarButton = [ [UIBarButtonItem alloc] initWithTitle:@"Menu" 
																	style:UIBarButtonItemStyleBordered 
																   target:self 
																   action:@selector(gotoMenu)];
	
	// if Two-player game, set left button back to matchmaking
	if( [ appDelegate isTwoPlayerGame ] ) 
	{
		self.navigationItem.rightBarButtonItem = aBarButton;
		[aBarButton release];

		aBarButton = [ [UIBarButtonItem alloc] initWithTitle:@"MatchMaking" 
													   style:UIBarButtonItemStyleBordered 
													  target:self 
													  action:@selector(gotoMatchMaking)];
		self.navigationItem.leftBarButtonItem = aBarButton;
		[aBarButton release];
	}
	else
	{
		self.navigationItem.leftBarButtonItem = aBarButton;
		[aBarButton release];
	}	
}


- (IBAction)viewOpponentClicked: (id)sender
{
	//[appDelegate showLeaderboards];
}

- (IBAction)viewMyStatsClicked: (id)sender
{
	[appDelegate showLeaderboards];
}


- (IBAction)leaderboardsButtonClicked: (id)sender
{
	[appDelegate showLeaderboards];
}

- (IBAction)playAgainButtonClicked: (id)sender
{
	[appDelegate restartGame];
}

- (IBAction)menuButtonClicked: (id)sender
{
	[appDelegate returnToMenu];
}


- (IBAction)gotoMatchMaking
{
	[appDelegate returnToMatchMaking];
}

- (IBAction)gotoMenu
{
	[appDelegate returnToMenu];
}

// Display the About page
- (IBAction)showInfo 
{    
	[ appDelegate showAboutInfo: self ];
} 

- (void)setGamesPlayed: (int)gamesPlayed
{
	gamesPlayedLabel.text = [NSString stringWithFormat: @"%d", gamesPlayed];
}


- (void)setBestTime: (int)bestTime
{
	SetTimeLabel(bestTimeLabel, bestTime);
}

- (void)setNewBestTime: (int)oldTime
{
	SetTimeLabel(newbestTimeLabel, oldTime);
	congratsLabel.hidden = NO;
	newbestLabel.hidden = NO;
	newbestTimeLabel.hidden = NO;
	oldbestLabel.hidden = NO;
}


- (void) getImagesForResultsPage 
{
	SAKERequest request;
	SAKEStartRequestResult startRequestResult;
	static SAKESearchForRecordsInput sfr_input;
	static char * fieldNames[] = { (char *) "ownerid", (char *) "picture" };
	//int ownerids[3];
	static int ownerids[2] = { gPlayerData.profileId, gOpponentData.profileId };
	
	if( gPlayerData.thumbNail == nil && gOpponentData.thumbNail == nil )
	{
		ownerids[0] = gPlayerData.profileId;
		ownerids[1] = gOpponentData.profileId;
//		ownerids[2] = '\0';
		
		[busyLocal startAnimating ];
		[busyRemote startAnimating ];
		busyLocal.hidden=NO;
		busyRemote.hidden=NO;

		sfr_input.mNumOwnerIds = 2;
		sfr_input.mMaxRecords = 2;
		NSLog(@"getImagesForResultsPage for player[%d] and opponent[%d]",gPlayerData.profileId,gOpponentData.profileId);
	}
	else if( gPlayerData.thumbNail != nil )
	{
		// display player picture
		[localThumbnailButton setBackgroundImage:gPlayerData.thumbNail forState: UIControlStateNormal ];
		busyLocal.hidden=YES;
		[busyLocal stopAnimating];
		
		// get opponent picture
		ownerids[0] = gOpponentData.profileId;
		ownerids[1] = '\0';
		sfr_input.mNumOwnerIds = 1;
		sfr_input.mMaxRecords = 1;
		[busyRemote startAnimating ];
		busyLocal.hidden=NO;
		NSLog(@"getImagesForResultsPage for opponent");
	}
	else if( gOpponentData.thumbNail != nil )
	{
		// display opponent picture
		[remoteThumbnailButton  setBackgroundImage:gOpponentData.thumbNail forState: UIControlStateNormal ];
		busyRemote.hidden=YES;
		[busyRemote stopAnimating];

		// get player picture
		ownerids[0] = gPlayerData.profileId;
		ownerids[1] = '\0';
		sfr_input.mNumOwnerIds = 1;
		sfr_input.mMaxRecords = 1;
		busyLocal.hidden=NO;
		[busyLocal startAnimating];
		NSLog(@"getImagesForResultsPage for player");
	}
	else
	{
		// get no pictures
		busyLocal.hidden=YES;
		busyRemote.hidden=YES;
		[busyLocal stopAnimating];
		[busyRemote stopAnimating];
		
		[localThumbnailButton setBackgroundImage:gPlayerData.thumbNail forState: UIControlStateNormal ];
		[remoteThumbnailButton  setBackgroundImage:gOpponentData.thumbNail forState: UIControlStateNormal ];
		NSLog(@"getImagesForResultsPage for no one");
		return;
	}
		
	sfr_input.mTableId = (char *) "Profiles";
	sfr_input.mFieldNames = fieldNames;
	sfr_input.mNumFields = 2;
	sfr_input.mSort = (char *) "recordid asc";
	sfr_input.mOffset = 0;
	sfr_input.mOwnerIds = ownerids;
	
	request = sakeSearchForRecords(appDelegate.sake, &sfr_input, SakeSearchForImageFileIdRecordsCallback, self );
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start search for records request: %d\n", startRequestResult);
	} 
	else 
	{
		NSLog(@"Searching for player record\n");
	}
}

- (void) downloadThumbnail: (int) fileId forPlayer: (int) profileId
{
	GHTTPRequest request;
	gsi_char url[SAKE_MAX_URL_LENGTH];
	
	ghttpStartup();
	if (sakeGetFileDownloadURL(appDelegate.sake, fileId, url)) 
	{
		NSLog(@"Download image URL: %s\n", url);
		
		ServerParam *param = (ServerParam*) gsimalloc(sizeof(ServerParam));
		param->controller = self;
		param->profileId = profileId;
			
		// download the file
		request = ghttpGetEx(url, NULL, NULL, 0, NULL, GHTTPFalse, GHTTPFalse, NULL, DownloadImageCompletedCallback, (void *) param);
		if (request == -1) 
		{
			gsifree(param);
			NSLog(@"Error starting image file download\n");
			if( (unsigned int)fileId == gPlayerData.pictureFileId )
				[busyLocal stopAnimating];
			else
				[busyRemote stopAnimating];
		} 
		else 
		{
			[appDelegate startThinkTimer];
			return;
		}
	} 
	else 
	{
		NSLog(@"Failed to get download url!\n");
		
		if( (unsigned int)fileId == gPlayerData.pictureFileId )
			[busyLocal stopAnimating];
		else
			[busyRemote stopAnimating];
	}
	ghttpCleanup();
}


- (void)setAverageTime: (int)averageTime
{
	SetTimeLabel(averageTimeLabel, averageTime);
}

- (void)setAverageTaps: (float) avgTaps
{
	avgTapsPerSecLabel.text = [NSString stringWithFormat: @"%5.3f", avgTaps];
}
- (void)setLocalPlayer: (NSString*)playerName time: (int)time
{
	localPlayerLabel.text = playerName;
	SetTimeLabel(localTimeLabel, time);
	localTime = time;
}

- (void)setRemotePlayer: (NSString*)playerName time: (int)time
{
	remotePlayerLabel.text = playerName;
	SetTimeLabel(remoteTimeLabel, time);
	remoteTime = time;
}

- (void)setGameTime: (int)gameTime
{
	SetTimeLabel(gameTimeLabel, gameTime);
}

- (void)setWinner
{
	[self determineWinner];
}

- (void)connectionDropped
{
	playAgainButton.enabled = NO;
}

// Resize the downloaded image and save in gPlayerData.fullsizeImage to represent the profile size
// Resize the fullsizeimage to and save in gPlayerData.thumbnail to represent the leaderboard thumbnail size
// Then draw thumbnail
- (void) drawThumbnailImage: (UIImage*)image forPlayer: (int) profileId
{
	
	// get the current colorspace
	int imgh = image.size.height;
	int imgw = image.size.width;
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB(  );
	void * data = calloc(imgh, 4*imgw);
	CGContextRef bmpContextRef = CGBitmapContextCreate ( data, // void *data,
														imgw, // size_t width,
														imgh, // size_t height,
														8, // size_t bitsPerComponent,
														4*imgw, // size_t bytesPerRow,
														colorspace, // CGColorSpaceRef colorspace,
														kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big // CGBitmapInfo bitmapInfo
														);
	CGRect fsrect = CGRectMake( 0, 0, imgw, imgh );
	CGContextDrawImage ( bmpContextRef, fsrect, image.CGImage );	// draw (and resize) image
	//if (gPlayerData.fullsizeImage != nil) [gPlayerData.fullsizeImage release];	
	//gPlayerData.fullsizeImage = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( bmpContextRef )] retain];
	CGContextRelease(bmpContextRef);
	free(data);
	
	// now redraw into thumbnail
	int imageOrientation = image.imageOrientation;
	int th,tw;
	float hscale = ((float)imgh)/float(THUMBNAIL_HEIGHT);
	float wscale = ((float)imgw)/float(THUMBNAIL_WIDTH);
	if (hscale > wscale) {
		// scale to height
		th = THUMBNAIL_HEIGHT;
		tw = (int)((float)imgw/hscale);
	} else {
		tw = THUMBNAIL_WIDTH;
		th = (int)((float)imgh/wscale);
	}
	data = calloc(th, 4*tw);
	CGContextRef thumbnailContextRef = CGBitmapContextCreate ( NULL, // void *data,
															  tw, // size_t width,
															  th, // size_t height,
															  8, // size_t bitsPerComponent,
															  4*tw, // size_t bytesPerRow,
															  colorspace, // CGColorSpaceRef colorspace,
															  kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big // CGBitmapInfo bitmapInfo
															  );
	CGRect tnrect;
	// rotate the context
	switch (imageOrientation) {
		case 0: // no rotation 
			tnrect = CGRectMake( 0, 0, tw, th );
			break;
		case 3: // rotate -90
			tnrect = CGRectMake( 0, 0, th, tw );
			CGContextTranslateCTM (thumbnailContextRef, 0.0f, float(th) );
			CGContextRotateCTM (thumbnailContextRef, -3.1415926f/2.0f);
			break;
		case 2: // rotate 180
			tnrect = CGRectMake( 0, 0, tw, th );
			CGContextTranslateCTM (thumbnailContextRef, float(tw), float(th) );
			CGContextRotateCTM (thumbnailContextRef, -3.1415926f);
			break;
		case 1: // rotate 90
			tnrect = CGRectMake( 0, 0, th, tw );
			CGContextTranslateCTM (thumbnailContextRef, float(tw), 0.0f );
			CGContextRotateCTM (thumbnailContextRef, 3.1415926f/2.0f);
			break;
	}	
	CGContextDrawImage ( thumbnailContextRef, tnrect, image.CGImage );	// draw image

	if( (unsigned int)profileId == gPlayerData.profileId )
	{
		if (gPlayerData.thumbNail != nil) [gPlayerData.thumbNail release];
		gPlayerData.thumbNail = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( thumbnailContextRef )] retain];
		CGContextRelease(thumbnailContextRef);
		CGColorSpaceRelease(colorspace);

		[busyLocal stopAnimating];
		busyLocal.hidden=YES;
		[localThumbnailButton  setBackgroundImage:gPlayerData.thumbNail forState: UIControlStateNormal ];
		//[localThumbnailButton  setImage:gPlayerData.thumbNail forState: UIControlStateNormal ];

		[localThumbnailButton setNeedsDisplay];
	}
	else
	{
		if (gOpponentData.thumbNail != nil) [gOpponentData.thumbNail release];
		gOpponentData.thumbNail = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( thumbnailContextRef )] retain];
		CGContextRelease(thumbnailContextRef);
		CGColorSpaceRelease(colorspace);

		[busyRemote stopAnimating];
		busyRemote.hidden=YES;
		[remoteThumbnailButton  setBackgroundImage:gOpponentData.thumbNail forState: UIControlStateNormal ];

		[remoteThumbnailButton setNeedsDisplay];
	}
	
	free(data);
}

@end


@implementation GameResultsController(Private)

- (GHTTPBool) downloadImageCompletedCallbackRequest:  (GHTTPRequest) request 
											 result: (GHTTPResult) result 
											 buffer: (char *) buffer 
										  bufferLen: (GHTTPByteCount) bufferLen
										  profileId: (int) profileId
{
	if (discardCallbacks) return GHTTPFalse;
	if(result != GHTTPSuccess)
	{
		NSLog(@"File Download: GHTTP Error: %d\n", result);
		if( (unsigned int)profileId == gPlayerData.profileId )
		{
			busyLocal.hidden=YES;
			[busyLocal stopAnimating];	
		}
		else
		{
			busyRemote.hidden=YES;
			[busyRemote stopAnimating];
		}

		ghttpCleanup();
		return GHTTPTrue;
	}
	// if picture is found, create a new thumbnail using the image in the buffer, then release the buffer
	if( bufferLen )
	{
		NSData * jpegImage = [NSData dataWithBytes:(const void *)buffer length: bufferLen];
		[self drawThumbnailImage: [UIImage imageWithData: jpegImage] forPlayer: profileId ];
		NSLog(@"Thumbnail downloaded for %d \n", profileId);
	}
	
	
	ghttpCleanup();
	[busyLocal stopAnimating];	
	busyLocal.hidden=YES;
	[busyRemote stopAnimating];
	busyRemote.hidden=YES;
	
	return GHTTPTrue; // frees the buffer
}



- (void) sakeSearchForImageFileIdRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	if (discardCallbacks) return;
	
	if(result == SAKERequestResult_SUCCESS) 
	{
		if ( ((SAKESearchForRecordsOutput*)outputData)->mNumRecords > 0) 
		{
			int aProfileId, aFileId;
			SAKEField* aField;
			
			NSLog(@"sakeSearchForImageFileIdRequestCallback get %d ids", ((SAKESearchForRecordsOutput*)outputData)->mNumRecords );
			for( int i = 0; i < ((SAKESearchForRecordsOutput*)outputData)->mNumRecords; i++ )
			{
				aField = ( (SAKEField*) ((SAKESearchForRecordsOutput*)outputData)->mRecords[i]);
				aProfileId = aField[0].mValue.mInt;
				aFileId = aField[1].mValue.mInt;
				
				if ( (unsigned int)aProfileId == gPlayerData.profileId )
				{
					gPlayerData.pictureFileId = aFileId;
					[self downloadThumbnail: aFileId forPlayer: gPlayerData.profileId ];
				}
				else
				{
					gOpponentData.pictureFileId = aFileId;
					[self downloadThumbnail: aFileId forPlayer: gOpponentData.profileId ];
				}
				
			}
		} 
		else 
		{
			[busyLocal stopAnimating];
			[busyRemote stopAnimating];
			busyLocal.hidden=YES;
			busyRemote.hidden=YES;
			NSLog(@"No existing profiles\n");
		}
	} 
	else 
	{
		[busyLocal stopAnimating];
		[busyRemote stopAnimating];
		busyLocal.hidden=YES;
		busyRemote.hidden=YES;
		NSLog(@"Failed to search for this player record, error: %d\n", result);
	}
}


- (void)determineWinner
{
	if ((localTime > 0) && (remoteTime > 0)) {
		// Multi-player game.
		if (localTime == remoteTime) 
		{
			tieLabel.hidden = NO;
			localWinnerLabel.hidden = YES;
			remoteWinnerLabel.hidden = YES;
			//localThumbnailButton.enabled = YES;
			//remoteThumbnailButton.enabled = YES;
		}
		else if (localTime < remoteTime) 
		{
			tieLabel.hidden = YES;
			localWinnerLabel.hidden = NO;
			remoteWinnerLabel.hidden = YES;
			//localThumbnailButton.enabled = YES;
			//remoteThumbnailButton.enabled = NO;
		}
		else 
		{
			tieLabel.hidden = YES;
			localWinnerLabel.hidden = YES;
			remoteWinnerLabel.hidden = NO;
			//localThumbnailButton.enabled = NO;
			//remoteThumbnailButton.enabled = YES;
		}
	}
}

/*
- (GHTTPBool) downloadImageCompletedCallbackRequest:  (GHTTPRequest) request result: (GHTTPResult) result buffer: (char *) buffer bufferLen: (GHTTPByteCount) bufferLen 
{
	if (discardCallbacks) return GHTTPFalse;
	if(result != GHTTPSuccess)
	{
		NSLog(@"File Download: GHTTP Error: %d\n", result);
		ghttpCleanup();
		return GHTTPTrue;
	}
	// if picture is found, create a new thumbnail using the image in the buffer, then release the buffer
	if( bufferLen )
	{
		NSData * jpegImage = [NSData dataWithBytes:(const void *)buffer length: bufferLen];
		[self drawThumbnailImage: [UIImage imageWithData: jpegImage]  forProfileID:  ];
		NSLog(@"Thumbnail downloaded\n");
	}
	
	
	ghttpCleanup();
	[busy stopAnimating];
	return GHTTPTrue; // frees the buffer
}
*/
@end

void SakeSearchForImageFileIdRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	if (discardCallbacks) return;
	[(GameResultsController *)userData sakeSearchForImageFileIdRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];

}


GHTTPBool DownloadImageCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param)
{
	if (discardCallbacks) return GHTTPFalse;
	
	GHTTPBool res =  [(GameResultsController *)( ((ServerParam*)param)->controller) downloadImageCompletedCallbackRequest: request 
								result: result 
								buffer: buffer 
								bufferLen: bufferLen 
								profileId: (int)( ((ServerParam*)param)->profileId) ];
	gsifree(param);
	return res;
}
