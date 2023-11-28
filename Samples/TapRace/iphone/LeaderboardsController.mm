// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "LeaderboardsController.h"
#import "LoginController.h"
#import "MyCLController.h"
#import "AppDelegate.h"
#import "Utility.h"
#import "sake/sakeRequest.h"
#import <UIKit/UIDevice.h>

//#define getLocalIntegerStat(key) [(NSNumber*)[gPlayerData.playerStatsData objectForKey: key ] intValue]
//#define getLocalStringStat(key) [(NSString*)[gPlayerData.playerStatsData objectForKey: key ]]

// callback prototypes
// http stuff to upload and download image
void postCallback( GHTTPRequest request, int bytesPosted, int totalBytes, int objectsPosted, int totalObjects, void * param );
GHTTPBool UploadCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param);
GHTTPBool DownloadCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param);
// sake stuff to access and create persistent server storage
void SakeCreateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
void SakeUpdateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
void SakeTestRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
void SakeSearchForRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 

static SAKEField fields[2];

static bool discardCallbacks = true;

// Exists for the simulator, not for the device: #import <ApplicationServices/ApplicationServices.h>
@interface LeaderboardsController(Private) <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

@end


@implementation LeaderboardsController

@synthesize  myTotalRaces;
@synthesize  myRank;
@synthesize  myTotalWins;
@synthesize  myTotalLosses;
@synthesize  myRatio;
@synthesize  myBestTime;
@synthesize  myStreak;
@synthesize  myStreakLabel;
@synthesize	 mySPTotalRaces;
@synthesize  mySPBestTime;
@synthesize  mySPAvgTime;
@synthesize thumbNail;


- (void)dealloc
{
	discardCallbacks = true;
	if (imagePickerController != nil) 
	{
		[imagePickerController release];
		imagePickerController = nil;
	}
	
	[thumbNail release];
	[setPhotoButton release];
	[top100Button release];
	[mySpotButton release];
	[byLocationButton release];
	[myTotalRaces release];
	[myTotalWins release];
	[myTotalLosses release];
	[myRatio release];
	[myBestTime release];
	[myStreak release];
	[mySPTotalRaces release];
	[mySPBestTime release];
	[mySPAvgTime release];
	[busy release];
    [super dealloc];
}

- (BOOL)shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}

- (void)viewWillAppear: (BOOL)animated
{
	[super viewWillAppear: animated];
	
	[appDelegate getMyLeaderPosition ];
	
	NSString* aTitle = [NSString stringWithFormat: @"%@'s Profile", gPlayerData.uniquenick ];
	self.title = aTitle;  //[leader.name copy];	// index of currently displayed profile
	
	// Hide Buddies leaderboard button if no buddies
	int numBuddies;
	LoginController* loginController = (LoginController*)[appDelegate findExistingControllerOfType: [LoginController class]];
	GPResult res = gpGetNumBuddies( [loginController getGPConnectionPtr], &numBuddies );

	if ( (res == GP_NO_ERROR) && (numBuddies == 0)) 
		mySpotButton.hidden= YES;
	else 
		mySpotButton.hidden = NO;
		
	// disable Location button if GPS is not available
	NSString *modelType = [UIDevice currentDevice].model;
	if( ! [modelType isEqualToString: (NSString *) @"iPod touch" ] )
	{
		// disable Location button on iPhone if GPS is not available
		if( [MyCLController sharedInstance].locationManager.locationServicesEnabled )
			byLocationButton.hidden = NO;
		else
			byLocationButton.hidden = YES;
	}

	setPhotoButton.hidden = ![UIImagePickerController isSourceTypeAvailable: UIImagePickerControllerSourceTypePhotoLibrary];

	// hide buddy list button for v1
	mySpotButton.hidden = YES;
	
	int matchesPlayed = getLocalIntegerStat(matchGamesPlayedKey);
	myTotalRaces.text = [NSString stringWithFormat: @"%d", matchesPlayed ]; 
	mySPTotalRaces.text = [NSString stringWithFormat: @"%d", getLocalIntegerStat(singlePlayerGamesPlayedKey)];

	int wins = getLocalIntegerStat(matchWonKey);
	int loss = getLocalIntegerStat(matchLossKey);
	
	myTotalWins.text = [NSString stringWithFormat: @"%d", wins]; 
	myTotalLosses.text = [NSString stringWithFormat: @"%d", loss ];
	
	if( matchesPlayed > 0 )
	{
		myRatio.text = [NSString stringWithFormat: @"%d\%%", (wins*100)/matchesPlayed ];
	} 
	else 
	{
		myRatio.text = [NSString stringWithString: @"0"];
	}
	
	int bestRaceTime = getLocalIntegerStat(matchBestTimeKey);
	if( (bestRaceTime <= 0) || (bestRaceTime >= INITIAL_BEST_TIME) ) 
	{
		myBestTime.text = [NSString stringWithString: @"none"];
	} 
	else 
	{
		SetTimeLabel( myBestTime, bestRaceTime);
	}
	
    //int spBestTime = [(NSNumber*)[gPlayerData.playerStatsData objectForKey: singlePlayerBestTimeKey] intValue];
	int spBestRaceTime = getLocalIntegerStat(singlePlayerBestTimeKey);
	int spAvgPlayTime = getLocalIntegerStat(singlePlayerAverageTimeKey);
	
	if( (spBestRaceTime <= 0) || (spBestRaceTime >= INITIAL_BEST_TIME) ) 
	{
		mySPBestTime.text = [NSString stringWithString: @"none"];
		mySPAvgTime.text = [NSString stringWithString: @"none"];
	} 
	else 
	{
		SetTimeLabel( mySPBestTime, spBestRaceTime );
		SetTimeLabel(mySPAvgTime, spAvgPlayTime );
	}

	// determine whether to display win, loss or draw streak
	int tmpStreak = getLocalIntegerStat(matchDrawStreakKey);
	NSMutableString* tmpStreakLabel = @"Draw Streak:"; 
	
	if( tmpStreak == 0 )
	{
		tmpStreak = getLocalIntegerStat(matchLossStreakKey);
		tmpStreakLabel = @"Loss Streak:";

		if( tmpStreak == 0 )
		{
			tmpStreak = getLocalIntegerStat(matchWinStreakKey);
			tmpStreakLabel = @"Win Streak:";
		}
	}
	
	myStreak.text = [NSString stringWithFormat: @"%d", tmpStreak ];
	myStreakLabel.text = tmpStreakLabel;

	discardCallbacks = false;
	
	// display current rank
	if( gPlayerData.rank > 0 )
		myRank.text = [NSString stringWithFormat: @"#%d", gPlayerData.rank ];
	else
	{
		// go get rank retrieved from server
		[self performSelector:@selector(delayedRank) withObject:nil afterDelay:1.0f];
	}
	
	// if coming back from picking a new image...
	if (newImage != nil ) 
	{
		// do a delayed request to allow this view to draw, then upload the image
		[busy startAnimating];
		[self performSelector:@selector(delayedDraw) withObject:nil afterDelay:0.02f];
	} 
	else if (gPlayerData.thumbNail != nil) // if no thumbnail, just leave as the default 'No Photo'
	{
		[busy stopAnimating];
		[self.thumbNail setImage: gPlayerData.thumbNail];
	} 
	else
	{
		// check to see if there is a player record
		[busy startAnimating];
		[self getImageFromProfile];
	}
}

// Display the About page
 - (IBAction)showInfo 
 {    
	[ appDelegate showAboutInfo: self ];
 } 

// Display the iPhone Photo Album to select a picture
- (IBAction)setPhotoButtonClicked: (id)sender
{
	imagePickerController = [[UIImagePickerController alloc] init];
	imagePickerController.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
	imagePickerController.delegate = self;
//	self.navigationController.navigationBarHidden = YES;
//	[self.navigationController pushViewController: imagePickerController animated: NO];
	[self presentModalViewController:imagePickerController animated:YES];	
	//[imagePickerController release];

}

// Display the table to Top 30 leaders
- (IBAction)top100ButtonClicked: (id)sender
{
	[appDelegate showLeadersByTime];
}

- (IBAction)myBuddiesButtonClicked: (id)sender
{
	[appDelegate showLeadersByBuddies];
}

- (IBAction)byLocationButtonClicked: (id)sender
{
	[appDelegate showLeadersByLocation];
}

- (void) getImageFromProfile 
{
	SAKERequest request;
	SAKEStartRequestResult startRequestResult;
	static SAKESearchForRecordsInput sfr_input;
	static char * fieldNames[] = { (char *) "picture" };
	int ownerids[] = { gPlayerData.profileId };
	
	sfr_input.mTableId = (char *) "Profiles";
	sfr_input.mFieldNames = fieldNames;
	sfr_input.mNumFields = 1;
	sfr_input.mSort = (char *) "recordid asc";
	sfr_input.mOffset = 0;
	sfr_input.mMaxRecords = 1;
	sfr_input.mOwnerIds = ownerids;
	sfr_input.mNumOwnerIds = 1;
	
	request = sakeSearchForRecords(appDelegate.sake, &sfr_input, SakeSearchForRecordsCallback, self );
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


- (void) testForExistingImageRecord
{
	SAKERequest request;
	SAKEStartRequestResult testRequestResult;
	static SAKEGetMyRecordsInput gmr_input;
	static char *fieldnames[] = { (char *) "recordId" };
	gmr_input.mTableId = (char *) "Profiles";
	gmr_input.mFieldNames = fieldnames;
	gmr_input.mNumFields = 1;
	
	request = sakeGetMyRecords(appDelegate.sake, &gmr_input, SakeTestRecordCallback, self);
	if(!request)
	{
		testRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start test for record request: %d\n", testRequestResult);
		[busy stopAnimating];
	} 
	else 
	{
		NSLog(@"Testing for existing image record\n");
	}
}

- (void) drawAndUploadImage: (UIImage*)image
{
	GHTTPPost post;
	GHTTPRequest request;
	gsi_char url[SAKE_MAX_URL_LENGTH];

	int imgh = image.size.height;
	int imgw = image.size.width;
	
	NSLog(@"drawAndUploadImage full image height=%d  width=%d", imgh, imgw);

	[busy startAnimating];
	[self drawThumbnailFromImage: image];
	
	ghttpStartup();
	
	// get an upload url
	if(sakeGetFileUploadURL(appDelegate.sake, url)) 
	{
		NSLog(@"Upload URL: %s\n", url);
		
		// upload a file
		post = ghttpNewPost();
		ghttpPostSetCallback(post, postCallback, self);
		
		// get a jpeg version of the image
		NSLog(@"drawAndUploadImage full image height=%d  width=%d", gPlayerData.thumbNail.size.height, gPlayerData.thumbNail.size.width);
		NSData * jpeg = UIImageJPEGRepresentation ( gPlayerData.thumbNail, 1.0f );
		ghttpPostAddFileFromMemory(post, "picture.image", (const char *)[jpeg bytes], [jpeg length], "picture.image", NULL);
		
		request = ghttpPostEx(url, NULL, post, GHTTPFalse, GHTTPTrue, NULL, UploadCompletedCallback, self);
		
		if(request == -1)
		{
			NSLog(@"Error starting file upload\n");
		} 
		else 
		{
			[appDelegate startThinkTimer];
			return; // wait for callback before cleaning up ghttp
		}
	} 
	else 
	{
		NSLog(@"Failed to get upload url!\n");
	}
	
	ghttpCleanup();
	[busy stopAnimating];
}

// Resize the downloaded image and save in gPlayerData.fullsizeImage to represent the profile size
// Resize the fullsizeimage to and save in gPlayerData.thumbnail to represent the leaderboard thumbnail size
// Then draw thumbnail
- (void) drawThumbnailFromImage: (UIImage*)image
{

	// get the current colorspace
	int imgh = image.size.height;
	int imgw = image.size.width;
	
	NSLog(@"drawThumbnailFromImage h=%d  w=%d", imgh, imgw);
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
	if (gPlayerData.fullsizeImage != nil) [gPlayerData.fullsizeImage release];	
	gPlayerData.fullsizeImage = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( bmpContextRef )] retain];
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
	if (gPlayerData.thumbNail != nil) [gPlayerData.thumbNail release];
	gPlayerData.thumbNail = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( thumbnailContextRef )] retain];

	imgh = gPlayerData.thumbNail.size.height;
	imgw = gPlayerData.thumbNail.size.width;
	NSLog(@"drawThumbnailFromImage thumbnail h=%d  w=%d", imgh, imgw);

	CGContextRelease(thumbnailContextRef);
	CGColorSpaceRelease(colorspace);
	free(data);
	
	[busy stopAnimating];
	[self.thumbNail setImage: gPlayerData.thumbNail];
	[thumbNail setNeedsDisplay];
}

// called after a delay to upload a new image from the image picker...
//  gives the mainloop a chance to switch views and render the leaders menu again before the upload starts
- (void)delayedDraw
{
	int imgh = newImage.size.height;
	int imgw = newImage.size.width;
	
	NSLog(@"delayedDraw full image height=%d  width=%d", imgh, imgw);

	[self drawAndUploadImage: newImage];
	[newImage release];
	newImage = nil;
}

// called after a delay to show rank
//  gives the mainloop a chance for callback to set server value for rank
- (void)delayedRank
{
	if( gPlayerData.rank != 0 )
		myRank.text = [NSString stringWithFormat: @"#%d", gPlayerData.rank ];
}

@end


@implementation  LeaderboardsController(Private)

//
//- (void) drawAndUploadImage: (UIImage*)image
//{
//	GHTTPPost post;
//	GHTTPRequest request;
//	gsi_char url[SAKE_MAX_URL_LENGTH];
//	
//	[busy startAnimating];
//	[self drawThumbnailFromImage: image];
//	
//	ghttpStartup();
//	
//	// get an upload url
//	if(sakeGetFileUploadURL(appDelegate.sake, url)) {
//		printf("Upload URL: %s\n", url);
//		// upload a file
//		post = ghttpNewPost();
//		ghttpPostSetCallback(post, postCallback, self);
//		// get a jpeg version of the image
//		NSData * jpeg = UIImageJPEGRepresentation ( gPlayerData.thumbnail, 1.0f );
//		ghttpPostAddFileFromMemory(post, "picture.image", (const char *)[jpeg bytes], [jpeg length], "picture.image", NULL);
//		
//		request = ghttpPostEx(url, NULL, post, GHTTPFalse, GHTTPTrue, NULL, UploadCompletedCallback, self);
//		
//		if(request == -1)
//		{
//			printf("Error starting file upload\n");
//		} else {
//			[appDelegate startThinkTimer];
//			return; // wait for callback before cleaning up ghttp
//		}
//	} else {
//		printf("Failed to get upload url!\n");
//	}
//	
//	ghttpCleanup();
//	[busy stopAnimating];
//}

- (void)imagePickerController: (UIImagePickerController*)picker didFinishPickingImage: (UIImage*)image editingInfo: (NSDictionary*)editingInfo
{
//	[self.navigationController popToViewController: self animated: NO];
//	self.navigationController.navigationBarHidden = NO;
	newImage = [image retain]; // set the new image field
	[self dismissModalViewControllerAnimated:YES];
	[imagePickerController release];
	imagePickerController = nil;
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
	[self dismissModalViewControllerAnimated:YES];
	[imagePickerController release];
	imagePickerController = nil;
//	[self.navigationController popToViewController: self animated: YES];
//	self.navigationController.navigationBarHidden = NO;
}

- (void) postCallBackRequest: (GHTTPRequest) request bytesPosted: (int) bytesPosted totalBytes: (int) totalBytes objectsPosted: (int) objectsPosted totalObjects: (int) totalObjects {
	// progress of post
}

- (void) setupRecordRequest: (char**) tableId fields: (SAKEField**) fieldsPtr fieldCount: (int*)numFields {
	*tableId = (char *) "Profiles";
	fields[0].mName = (char *) "nick";
	fields[0].mType = SAKEFieldType_ASCII_STRING;
	fields[0].mValue.mAsciiString = (char*) [gPlayerData.uniquenick cStringUsingEncoding: NSASCIIStringEncoding]; // player nick goes in here
	fields[1].mName = (char *) "picture";
	fields[1].mType = SAKEFieldType_INT;
	fields[1].mValue.mInt = gUploadedFileId; // fileid of image
	*fieldsPtr = fields;
	*numFields = 2;	
}

- (void) updateImageToProfile: (int)recordId {
	SAKERequest request;
	SAKEStartRequestResult startRequestResult;
	static SAKEUpdateRecordInput ur_input;
	
	[self setupRecordRequest: &ur_input.mTableId fields: &ur_input.mFields fieldCount: &ur_input.mNumFields];
	ur_input.mRecordId = recordId;
	request = sakeUpdateRecord(appDelegate.sake, &ur_input, SakeUpdateRecordCallback, self);
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start request: %d\n", startRequestResult);
		[busy stopAnimating];
	} else {
		NSLog(@"Updating image to profile\n");
	}
}

- (void) addImageToProfile {
	SAKERequest request;
	SAKEStartRequestResult startRequestResult;
	static SAKECreateRecordInput cr_input;
	
	[self setupRecordRequest: &cr_input.mTableId fields: &cr_input.mFields fieldCount: &cr_input.mNumFields];
	request = sakeCreateRecord(appDelegate.sake, &cr_input, SakeCreateRecordCallback, self);
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start request: %d\n", startRequestResult);
	} else {
		NSLog(@"Adding image to profile\n");
	}
}

- (void) downloadImageToThumbnail: (int) fileId 
{
	GHTTPRequest request;
	gsi_char url[SAKE_MAX_URL_LENGTH];
	
	ghttpStartup();
	if (sakeGetFileDownloadURL(appDelegate.sake, fileId, url)) 
	{
		NSLog(@"Download image URL: %s\n", url);
		
		// download the file
		request = ghttpGetEx(url, NULL, NULL, 0, NULL, GHTTPFalse, GHTTPFalse, NULL, DownloadCompletedCallback, self);
		if (request == -1) 
		{
			NSLog(@"Error starting image file download\n");
			[busy stopAnimating];
		} else {
			[appDelegate startThinkTimer];
			return;
		}
	} else {
		NSLog(@"Failed to get download url!\n");
		[busy stopAnimating];
	}
	ghttpCleanup();
}

- (void) sakeCreateRecordRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	if (discardCallbacks) return;
	if(result != SAKERequestResult_SUCCESS)
	{
		NSLog(@"Failed to add image to profile, error: %d\n", result);
	} else {
		NSLog(@"Image added to profile\n");
	}
	[busy stopAnimating];
}

- (void) sakeUpdateRecordRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	if (discardCallbacks) return;
	if(result != SAKERequestResult_SUCCESS)
	{
		NSLog(@"Failed to update image to profile, error: %d\n", result);
	} else {
		NSLog(@"Image updated to profile\n");
	}
	[busy stopAnimating];
}

- (void) sakeTestRecordRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	if (discardCallbacks) return;
	if(result == SAKERequestResult_SUCCESS) {
		if ( ((SAKEGetMyRecordsOutput*)outputData)->mNumRecords > 0) 
		{
			
			[self updateImageToProfile: ((SAKEGetMyRecordsOutput*)outputData)->mRecords[0]->mValue.mInt];
		} else {
			[self addImageToProfile ];
		}
	} else {
		NSLog(@"Failed to test for image record, error: %d\n", result);
	}
	[busy stopAnimating];
}

- (void) sakeSearchForRecordsRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	if (discardCallbacks) return;
	
	if(result == SAKERequestResult_SUCCESS) 
	{
		if ( ((SAKESearchForRecordsOutput*)outputData)->mNumRecords > 0) 
		{
			gPlayerData.pictureFileId = ((SAKEGetMyRecordsOutput*)outputData)->mRecords[0]->mValue.mInt;
			[self downloadImageToThumbnail: gPlayerData.pictureFileId ];
		} 
		else 
		{
			[busy stopAnimating];
			NSLog(@"No existing profile for this player\n");
		}
	} 
	else 
	{
		[busy stopAnimating];
		NSLog(@"Failed to search for this player record, error: %d\n", result);
	}
}

- (GHTTPBool) uploadCompletedCallbackRequest: (GHTTPRequest) request result: (GHTTPResult) result buffer: (char *) buffer bufferLen: (GHTTPByteCount) bufferLen 
{
	SAKEFileResult fileResult;
	gsi_bool gUploadResult;
	gUploadResult = gsi_false;
	
	if (discardCallbacks) return GHTTPFalse;
	[busy stopAnimating];
	if(result != GHTTPSuccess)
	{
		NSLog(@"File Upload: GHTTP Error: %d\n", result);
		ghttpCleanup();
		return GHTTPTrue;
	}
	
	if(!sakeGetFileResultFromHeaders(ghttpGetHeaders(request), &fileResult))
	{
		NSLog(@"File Upload: Failed to find Sake-File-Result header\n");
		ghttpCleanup();
		return GHTTPTrue;
	}
	
	if(fileResult != SAKEFileResult_SUCCESS)
	{
		NSLog(@"File Upload: SakeFileResult != success: %d\n", fileResult);
		ghttpCleanup();
		return GHTTPTrue;
	}
	
	if(!sakeGetFileIdFromHeaders(ghttpGetHeaders(request), &gUploadedFileId))
	{
		NSLog(@"File Upload: Unable to parse Sake-File-Id header\n");
		ghttpCleanup();
		return GHTTPTrue;
	}
	
	gPlayerData.pictureFileId = gUploadedFileId;	// save new picture file id
	
	NSLog(@"File Upload: Uploaded fileId: %d\n", gUploadedFileId);
	gUploadResult = gsi_true;
	ghttpCleanup();
	
	// see if record already existed
	[busy startAnimating];
	[self testForExistingImageRecord ];
	
	return GHTTPTrue;
}

- (GHTTPBool) downloadCompletedCallbackRequest:  (GHTTPRequest) request result: (GHTTPResult) result buffer: (char *) buffer bufferLen: (GHTTPByteCount) bufferLen 
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
		[self drawThumbnailFromImage: [UIImage imageWithData: jpegImage]];
		NSLog(@"Thumbnail downloaded\n");
	}
	
	
	ghttpCleanup();
	[busy stopAnimating];
	return GHTTPTrue; // frees the buffer
}


@end

// callbacks
void postCallback( GHTTPRequest request, int bytesPosted, int totalBytes, int objectsPosted, int totalObjects, void * param ) 
{
	if (discardCallbacks) return;
	[(LeaderboardsController *)param postCallBackRequest: request bytesPosted: bytesPosted totalBytes: totalBytes objectsPosted: objectsPosted totalObjects: totalObjects ];
}

GHTTPBool UploadCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param)
{
	if (discardCallbacks) return GHTTPFalse;
	return [(LeaderboardsController *)param uploadCompletedCallbackRequest: request result: result buffer: buffer bufferLen: bufferLen];
}

GHTTPBool DownloadCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param)
{
	if (discardCallbacks) return GHTTPFalse;
	return [(LeaderboardsController *)param downloadCompletedCallbackRequest: request result: result buffer: buffer bufferLen: bufferLen];
}

void SakeCreateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	if (discardCallbacks) return;
	[(LeaderboardsController *)userData sakeCreateRecordRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

void SakeUpdateRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	if (discardCallbacks) return;
	[(LeaderboardsController *)userData sakeUpdateRecordRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

void SakeTestRecordCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	if (discardCallbacks) return;
	[(LeaderboardsController *)userData sakeTestRecordRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

void SakeSearchForRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	if (discardCallbacks) return;
	[(LeaderboardsController *)userData sakeSearchForRecordsRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

