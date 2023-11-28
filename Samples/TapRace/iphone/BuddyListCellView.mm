//
//  BuddyListCellView.mm
//
//  Created by CW Hicks on 10/21/08.
//  Copyright 2008 IGN. All rights reserved.
//

#import "BuddyListCellView.h"
#import "Utility.h"
#import "AppDelegate.h"
#import "sake/sake.h"

void SakeSearchForBuddyRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
GHTTPBool DownloadBuddyImageCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param);

@implementation BuddyListCellView

@synthesize cellView;
@synthesize profileId;
@synthesize pictureFileId;
@synthesize index;

- (id) initCellView
{
	[[NSBundle mainBundle] loadNibNamed: @"buddyListCell" owner: self options: nil];
	return self;
}


- (id)initWithFrame:(CGRect)frame 
{
	if ( (self = [super initWithFrame:frame]) ) 
	{
		// Initialization code
	}
	return self;
}

- (void)awakeFromNib
{
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
	// Configure the view for the selected state
}


- (void)dealloc
{
	if (cannotDie > 0) 
	{
		// this must be preserved until the last callback returns
		mustDie = YES;
		[self retain]; // bump the ref count so won't be released
		return;
	}
	
	[cellView release];
	[thumbnail release];
	[nameLabel release];
	[indexLabel release];
	[timeLabel release];
	[busy release];
	
	[super dealloc];
}

- (void)setName: (NSString*)name
{
	nameLabel.text = [name copy];
}

- (void)setIndex: (int)newIndex
{
	index = newIndex-1;
	indexLabel.text = [NSString stringWithFormat: @"%d", newIndex];
}

- (void)setImage: (UIImage*)image
{
	[thumbnail setImage:image];
}

- (void)setTime: (int)newTime 
{
	SetTimeLabel( timeLabel, newTime );
}

- (void) drawThumbnailFromImage: (UIImage*)image
{
	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: index];
	
	// resize the image to represent the profile size and get the current colorspace
	int imgh = image.size.height;
	int imgw = image.size.width;
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB(  );
	void * data = calloc(imgh, 4*imgw);
	CGContextRef thumbnailContextRef = CGBitmapContextCreate ( data, // void *data,
														imgw, // size_t width,
														imgh, // size_t height,
														8, // size_t bitsPerComponent,
														4*imgw, // size_t bytesPerRow,
														colorspace, // CGColorSpaceRef colorspace,
														kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big // CGBitmapInfo bitmapInfo
														);
	CGRect tnrect = CGRectMake( 0, 0, imgw, imgh );
	CGContextDrawImage ( thumbnailContextRef, tnrect, image.CGImage );	// draw (and resize) image 
	
	
	if (leader.thumbNail != nil) 
		[leader.thumbNail release];
	
	NSLog(@"setting thumbnail for player[%d] = %d\n", index, leader.profileId);
	
	// set thumbnail for leader
	leader.thumbNail = [[UIImage imageWithCGImage: CGBitmapContextCreateImage ( thumbnailContextRef )] retain];

	CGContextRelease(thumbnailContextRef);
	CGColorSpaceRelease(colorspace);
	free(data);
	
	[busy stopAnimating];
	busy.hidden=YES;
	[thumbnail setImage: leader.thumbNail];
	[thumbnail setNeedsDisplay];
}

- (GHTTPBool) downloadCompletedCallbackRequest:  (GHTTPRequest) request result: (GHTTPResult) result buffer: (char *) buffer bufferLen: (GHTTPByteCount) bufferLen 
{
	cannotDie--;
	if (mustDie && (cannotDie == 0)) {
		[self dealloc];
		return GHTTPTrue;
	}
	
	if(result != GHTTPSuccess)
	{
		NSLog(@"File Download: GHTTP Error: %d\n", result);
		ghttpCleanup();
		return GHTTPTrue;
	}
	// create a new thumbnail using the image in the buffer, then release the buffer
	NSData * jpegImage = [NSData dataWithBytes:(const void *)buffer length: bufferLen];
	if( bufferLen )
	{
		[self drawThumbnailFromImage: [UIImage imageWithData: jpegImage]];
	}
	
	ghttpCleanup();
	[busy stopAnimating];
	busy.hidden=YES;
	return GHTTPTrue; // frees the buffer
}

- (void) downloadImageToThumbnail: (int) fileId forProfile: (int) aProfileId
{
	LeaderboardObject* leader = (LeaderboardObject*)[gLeaderboardData objectAtIndex: index]; 
	leader.pictureFileId = fileId;
	NSLog(@"setting fileid[%d] for player[%d]=%d\n", fileId, index, leader.profileId);

	// if image is blocked, do not load it
	if( [ appDelegate imageIsBlocked: fileId] )
	{
		//leader.thumbNail = nil;
		[busy stopAnimating];
		busy.hidden=YES;
		return;
	}
	
	GHTTPRequest request;
	gsi_char url[SAKE_MAX_URL_LENGTH];
	
	ghttpStartup();
	if (sakeGetFileDownloadURL(appDelegate.sake, fileId, url)) 
	{
		NSLog(@"Download image for player[%d] URL: %s\n", leader.profileId, url);
		
		// download the file
		request = ghttpGetEx(url, NULL, NULL, 0, NULL, GHTTPFalse, GHTTPFalse, NULL, DownloadBuddyImageCompletedCallback, self);
		if (request == -1) 
		{
			NSLog(@"Error starting image file download\n");
		} 
		else 
		{
			cannotDie++;
			[appDelegate startThinkTimer];
			return;
		}
	} 
	else 
	{
		NSLog(@"Failed to get download url!\n");
	}
	
	ghttpCleanup();
	[busy stopAnimating];
	busy.hidden=YES;
}

- (void) sakeSearchForRecordsRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	cannotDie--;
	if (mustDie && (cannotDie == 0)) 
	{
		[self dealloc];
		return;
	}
	
	SAKESearchForRecordsInput* inData = (SAKESearchForRecordsInput*)inputData;
	if(result == SAKERequestResult_SUCCESS) 
	{
		if ( ((SAKESearchForRecordsOutput*)outputData)->mNumRecords > 0) 
		{
			[self downloadImageToThumbnail: ((SAKEGetMyRecordsOutput*)outputData)->mRecords[0]->mValue.mInt
					forProfile: (inData)->mOwnerIds[0] ];
			return;
		} 
		else 
		{
			//XNSLog(@"No existing profile for player[%d]\n", *((SAKESearchForRecordsInput*)(request)->mInput)->mOwnerIds[0]);
			NSLog(@"No existing profile for player[%d]\n", (inData)->mOwnerIds[0]);
		}
	} 
	else 
	{
		NSLog(@"Failed to search for player[%d] record, error: %d\n", *((inData)->mOwnerIds) ,result);
	}
	[busy stopAnimating];
	busy.hidden=YES;
}

- (void)getImageFromNetwork
{
	SAKERequest request;
	SAKEStartRequestResult startRequestResult;
	static SAKESearchForRecordsInput sfr_input;
	static char * fieldNames[] = { (char *) "picture" };
	int ownerids[] = { profileId };
	
	sfr_input.mTableId = (char *) "Profiles";
	sfr_input.mFieldNames = fieldNames;
	sfr_input.mNumFields = sizeof(fieldNames) / sizeof(fieldNames[0]);
	sfr_input.mSort = (char *) "recordid asc";
	sfr_input.mOffset = 0;
	sfr_input.mMaxRecords = 1;
	sfr_input.mOwnerIds = ownerids;
	sfr_input.mNumOwnerIds = 1;
	
	NSLog(@"get fileid for [%d]\n", profileId);
	request = sakeSearchForRecords(appDelegate.sake, &sfr_input, SakeSearchForBuddyRecordsCallback, self );
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start search for records request: %d\n", startRequestResult);
		[busy stopAnimating];

	} 
	else 
	{
		NSLog(@"Searching for picture fileid record for player [%d]\n", profileId);
		cannotDie++;
	}
}
	
	
@end

void SakeSearchForBuddyRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	[(BuddyListCellView *)userData sakeSearchForRecordsRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

GHTTPBool DownloadBuddyImageCompletedCallback(GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param)
{
	return [(BuddyListCellView *)param downloadCompletedCallbackRequest: request result: result buffer: buffer bufferLen: bufferLen];
}


