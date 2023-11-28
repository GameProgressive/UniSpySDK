// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "LeadersByLocationController.h"
#import "AppDelegate.h"
#import "Utility.h"
#include "atlas_taprace_v2.h"
#import "UserStatsController.h"

#define LOCATION_DEGREES_RADIUS 1.1f
#define METER_RADIUS_FOR_SELECT (6000.0f*LOCATION_DEGREES_RADIUS) // approximate meters radius of map/10
#define MAP_ZOOM 7

// structure to hold the buddy info
@interface buddy : NSObject
{
	NSString * name;
	CLLocationDegrees latitude;
	CLLocationDegrees longitude;
}

@property (nonatomic, copy) NSString * name;
@property CLLocationDegrees latitude;
@property CLLocationDegrees longitude;

@end

//**************************************
@implementation buddy	
@synthesize name, latitude, longitude;
@end


void SearchForLocationRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData ); 
static const char PLAYER_STATS_TABLE[] = "PlayerStats_v" STRINGIZE(ATLAS_RULE_SET_VERSION);


//**************************************
@implementation LeadersByLocationController

@synthesize spinner, myName;

- (void)dealloc
{
	[webView release];
	[spinner release]; 
	[updateTimer invalidate];
	[updateTimer release];
	[super dealloc];
}

//**************************************
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
	if ( (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) ) {
		self->isCurrentlyUpdating = NO;
		self.myName = [gPlayerData.uniquenick copy];
	}
	return self;
}

//**************************************
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

//**************************************
- (void)updateTime: (NSTimer*)timer;
{
	// try to issue javascript to grab the latest lat/long
	NSString * touched = [webView stringByEvaluatingJavaScriptFromString: @"touched"];
	if ([touched intValue]) {
		[webView stringByEvaluatingJavaScriptFromString: @"touched = 0;"];
		NSString * lastLat = [webView stringByEvaluatingJavaScriptFromString: @"lastLat"];
		NSString * lastLon = [webView stringByEvaluatingJavaScriptFromString: @"lastLon"];
		
		CLLocation * loc = [[CLLocation alloc] initWithLatitude: [lastLat floatValue] longitude: [lastLon floatValue]];
		int budindex = -1;
		
		// locate the closest individual
		CLLocationDistance distance = 99999.99f;
		for (unsigned int i = 0; i < [gLeaderboardData count]; i++) 
		{
			LeaderboardObject * nextbuddy = (LeaderboardObject*)[gLeaderboardData objectAtIndex: i];
			CLLocationDistance buddyDist = [loc getDistanceFrom: nextbuddy.playerLocation];
			if (buddyDist < distance) {
				budindex = i;
				distance = buddyDist;
			}
		}
		// if not within 1000 meters, ignore the touch
		if ((distance < METER_RADIUS_FOR_SELECT) && (budindex >= 0)) {
			[appDelegate showUser: budindex];	// show the user profile
		}
	}
}

//**************************************
- (MKAnnotationView *)mapView:(MKMapView *)aMapView viewForAnnotation:(id <MKAnnotation>)annotation
{
	MKPinAnnotationView *av = (MKPinAnnotationView *)[aMapView dequeueReusableAnnotationViewWithIdentifier:@"myID"];
	if ( nil == av )
		av = [[MKPinAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:@"myId"];

	UIButton *button = [UIButton buttonWithType:UIButtonTypeDetailDisclosure];
	av.rightCalloutAccessoryView = button;
	av.canShowCallout = YES;
	return [av autorelease];
}


//**************************************
- (void)mapView:(MKMapView *)mapView annotationView:(MKAnnotationView *)view calloutAccessoryControlTapped:(UIControl *)control
{
	gLeaderboardProfileIndex = 0;
	
	// get the selected uniquenick and set user to display from list
	NSString* selectedLeader = [[ view annotation] title ]; 
	
	for( unsigned int index = 0; index < [gLeaderboardData count]; index++ )
	{
		if( [ selectedLeader isEqualToString: ((LeaderboardObject*)[gLeaderboardData objectAtIndex: index ]).name ] )
		{
			gLeaderboardProfileIndex = index;
			break;
		}
	}
	
	[appDelegate pushNewControllerOfType: [UserStatsController class] nib: @"UserStats"];
}


//**************************************
- (void)viewDidLoad {
	unsigned int i;
	CLLocationDegrees  myLat, buddyLat, myLon, buddyLon;
	
	myLat = gPlayerData.playerLocation.coordinate.latitude;
	myLon = gPlayerData.playerLocation.coordinate.longitude;
	NSLog(@"myLat %f  myLon %f", myLat, myLon );

	// calculate area to display
	// All leaders with no position are set to my position
	for( i = 0; i < [gLeaderboardData count]; i++ ) 
	{
		LeaderboardObject * nextbuddy = (LeaderboardObject*)[gLeaderboardData objectAtIndex: i];
		
		buddyLat = nextbuddy.playerLocation.coordinate.latitude;
		buddyLon = nextbuddy.playerLocation.coordinate.longitude;
		if( buddyLat == 0.0 && buddyLon == 0.0 )
		{
			NSLog(@"Unknown location for %s\n", nextbuddy.name );
			nextbuddy.playerLocation.coordinate.latitude = myLat;
			nextbuddy.playerLocation.coordinate.longitude = myLon;
			continue;
		}
		
		NSLog(@"bddy Lat %lf  Lon %lf", buddyLat, buddyLon );
	}

	MKCoordinateSpan span = MKCoordinateSpanMake( LOCATION_DEGREES_RADIUS, LOCATION_DEGREES_RADIUS);
	MKCoordinateRegion location = { gPlayerData.playerLocation.coordinate, span };
	[mapView setRegion:location];
	
	// set pins for each player
	[mapView addAnnotations:gLeaderboardData];
}

// Display the About page
- (IBAction)showInfo 
{    
	[ appDelegate showAboutInfo: self ];
} 


/*
-(void)viewDidLoadOld {
	
	[spinner startAnimating];
	// TODO:  Parse and save lat/long
	NSMutableString *update = [[[NSMutableString alloc] init] autorelease];
	[update appendString:@"<html>\n<head>\n<script type='text/javascript'\n"];
	[update appendString:@"src='http://api.maps.yahoo.com/ajaxymap?v=3.8&appid=7DHW5ibV34H2zuiFMFWdpe.1.uDnWWJ50QtSkcP7_lwPLkIgnH1DHOQ52QGEiPo-'"];
	[update appendString:@"></script>\n<style type='text/css'>\n#map{ height: 100%; width: 100%; }\n</style>\n</head><body><div id='map'></div>\n"];
	[update appendString:@"<script type='text/javascript'>\n"];
	[update appendString:@"var touched = 0; var lastLat; var lastLon;\n"];
	[update appendString:@"var map = new YMap(document.getElementById('map'));\n"];
	[update appendString:@"function startMap() {\n"];
	[update appendString:@"map.setMapType(YAHOO_MAP_REG);\n"];
	// now set the current user's lat/long for the center of the map
	[update appendString:@"var latlong = new YGeoPoint("];
	[update appendString:[NSString stringWithFormat: @"%f", gPlayerData.playerLocation.coordinate.latitude]];
	[update appendString:@","];
	[update appendString:[NSString stringWithFormat: @"%f", gPlayerData.playerLocation.coordinate.longitude]];
	[update appendString:@");\n"];
	[update appendString:@"map.drawZoomAndCenter( latlong, "];
	[update appendString:[NSString stringWithFormat: @"%d", MAP_ZOOM]];
	[update appendString: @" );\n"];
	// now insert all the buddies in the list
#if defined(USE_DUMMY_LEADERBOARD_DATA)
	// for testing on emulator, force buddies to lie on .01 degree radius from user
	float flat = gPlayerData.playerLocation.coordinate.latitude;
	float flon = gPlayerData.playerLocation.coordinate.longitude;
	float ulat, ulon; // FOR DEBUG
#endif
	for (unsigned int i = 0; i < [gLeaderboardData count]; i++) {
		LeaderboardObject * nextbuddy = (LeaderboardObject*)[gLeaderboardData objectAtIndex: i];
#if defined(USE_DUMMY_LEADERBOARD_DATA)
		float angle = float(i)*6.28f/float([gLeaderboardData count]); // FOR DEBUG 
		ulat = flat + 0.008*(sin(angle) - cos(angle));  // FOR DEBUG
		ulon = flon + 0.009*(cos(angle) + sin(angle));  // FOR DEBUG...place opponents around player
		[nextbuddy.playerLocation initWithLatitude: ulat longitude: ulon];  // FOR DEBUG...force near player so works in simulator
#endif
		
		[update appendString:@"newmarker = new YMarker( new YGeoPoint("];
		[update appendString:[NSString stringWithFormat: @"%f", nextbuddy.playerLocation.coordinate.latitude]];
		[update appendString:@","];
		[update appendString:[NSString stringWithFormat: @"%f", nextbuddy.playerLocation.coordinate.longitude]];
		[update appendString:@") );\n"];
		if (nextbuddy.rank < 0) {
			[update appendString:[NSString stringWithFormat: @"newmarker.addLabel('?');\n"]];
		} else {
			[update appendString:[NSString stringWithFormat: @"newmarker.addLabel('%d');\n", nextbuddy.rank]];
		}
		if (nextbuddy.name == nil) {
			[update appendString:[NSString stringWithFormat: @"newmarker.addAutoExpand('?');\n"]];
		} else {
			[update appendString:[NSString stringWithFormat: @"newmarker.addAutoExpand('%@');\n", nextbuddy.name]];
		}
		[update appendString:@"map.addOverlay( newmarker );\n"];		
	}
	// mark me
	[update appendString:@"var newmarker = new YMarker(latlong);\n"];
	if (gPlayerData.rank <= 0) {
		[update appendString:[NSString stringWithFormat: @"newmarker.addLabel('%@');\n", myName]];
	} else {
		[update appendString:[NSString stringWithFormat: @"newmarker.addLabel('%d');\n", gPlayerData.rank]];
	}
	[update appendString:[NSString stringWithFormat: @"newmarker.addAutoExpand('%@');\n", myName]];
	[update appendString:@"map.addOverlay( newmarker );\n"];		
	[update appendString:@"newmarker.openAutoExpand();\n"];
	
	// add event handler
	[update appendString:@"YEvent.Capture(map, EventsList.MouseClick, tagTouched);\n"];
	[update appendString:@"function tagTouched(_e, _c) {\n"];
	[update appendString:@"touched = 1; lastLat = _c.Lat; lastLon = _c.Lon;}}\n"];
	[update appendString:@"window.onload = startMap;\n"];
	[update appendString:@"</script></body></html>\n"];
	
	[webView loadHTMLString:update baseURL: nil];

	if (alertView != nil) {
		// remove the previous alert
		[alertView dismissWithClickedButtonIndex: alertView.cancelButtonIndex+1 animated: NO];
		[alertView release];
		alertView = nil;
	}
	
	if ([gLeaderboardData count] == 0) {
		alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Searching for nearby players" delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
		[alertView show];
		return; // don't look for tagTouch events
	}
	// kick off a timer to test for tagTouch events
	if (updateTimer != nil) {
		[updateTimer invalidate];
	}
	updateTimer = [NSTimer scheduledTimerWithTimeInterval: 0.01 target: self selector: @selector(updateTime:) userInfo: nil repeats: YES];

	[spinner stopAnimating];
}
*/

- (void)alertView: (UIAlertView*)alert didDismissWithButtonIndex: (NSInteger)buttonIndex
{
	if ([gLeaderboardData count] == 0) 
	{
		// same as exit
		[self.navigationController popViewControllerAnimated: YES];
	}
}

-(void)newError:(NSString *)text {
	alertView = [[UIAlertView alloc] initWithTitle: nil message: @"Error finding location" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
	[alertView show];
}

- (void) searchLeaderboardsByLocation
{
	static SAKESearchForRecordsInput input;
	SAKERequest request;
	static char *fieldNames[] = { 
		(char *) "ownerid" , 
		(char *) "row", 
		ATLAS_GET_STAT_NAME( NICK ), 
		ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), 
		ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), 
		ATLAS_GET_STAT_NAME( CAREER_WINS ), 
		ATLAS_GET_STAT_NAME( CAREER_LOSSES ), 
		ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ),
		ATLAS_GET_STAT_NAME( LATITUDE ),
		ATLAS_GET_STAT_NAME( LONGITUDE)
	};		
	
	SAKEStartRequestResult startRequestResult;
	
	char filter[512];
	float myLat = gPlayerData.playerLocation.coordinate.latitude;
	float myLong = gPlayerData.playerLocation.coordinate.longitude;
	float offset = LOCATION_DEGREES_RADIUS;
	
	sprintf(filter, "BEST_RACE_TIME != 99999 and LATITUDE > %f and LATITUDE < %f and LONGITUDE > %f and LONGITUDE < %f",
			myLat-offset, myLat+offset, myLong-offset, myLong+offset);
	
	input.mTableId = (char *) PLAYER_STATS_TABLE;
	input.mFieldNames = fieldNames;
	input.mNumFields = (sizeof(fieldNames) / sizeof(fieldNames[0]));
	input.mFilter = filter;
	input.mSort = (char *) "BEST_RACE_TIME asc";
	input.mOffset = 0;
	input.mMaxRecords = 30;
	
	request = sakeSearchForRecords(appDelegate.sake, &input, SearchForLocationRecordsCallback, self);
	if(!request)
	{
		startRequestResult = sakeGetStartRequestResult(appDelegate.sake);
		NSLog(@"Failed to start location search request: %d\n", startRequestResult);
	}
}

- (void) sakeSearchForLocationRecordsRequestCallback:(SAKE) sake request:(SAKERequest) request result:(SAKERequestResult) result inputData:(void *) inputData outputData:(void *) outputData
{
	float lat;
	float lng;
	
	if(result == SAKERequestResult_SUCCESS) {
		SAKESearchForRecordsOutput* records = (SAKESearchForRecordsOutput*)outputData;
		if ( records->mNumRecords > 0) {
			// extract the info and put it into the buddy list and display it
			if (gLeaderboardData != nil) 
			{ 
				int lbtype = gLeaderboardType;
				[appDelegate deleteLeaderboards];
				gLeaderboardType = lbtype;
			}
			
			if (records->mNumRecords > 0) 
			{
				gLeaderboardData = [[NSMutableArray arrayWithCapacity: records->mNumRecords] retain];
				
				for (int i=0; i<records->mNumRecords; i++) 
				{
					SAKEField * field = sakeGetFieldByName( "ownerid", records->mRecords[i], 10 );
					
					// if this guy is me, skip it
					if ((field != NULL) && ( (int)(gPlayerData.profileId) == field->mValue.mInt)) 
					{
						/* // but extract my rank, just in case
						field = sakeGetFieldByName( "row", records->mRecords[i], 10);
						if (field != NULL) 
						{
							gPlayerData.rank = field->mValue.mInt;
						}
						*/
						continue;
					}
							
					// create a new leaderboard object
					LeaderboardObject * pData = [[LeaderboardObject alloc] init];
					
					if (field != NULL) {
						pData.profileId = field->mValue.mInt;
					} else {
						pData.profileId = 0;
					}
					field = sakeGetFieldByName( "row", records->mRecords[i], 10);
					if (field != NULL) {
						pData.rank = field->mValue.mInt;
					}  else {
						pData.rank = -1;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( NICK ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.name = [NSString stringWithFormat: @"%s", (char*)( field->mValue.mAsciiString ) ];
					} else {
						pData.name = nil;
					}
					NSLog(@"Found player[%d]: %s ", pData.rank, field->mValue.mAsciiString);
					
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( BEST_RACE_TIME ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.bestRaceTime = (gsi_u32)( field->mValue.mInt );
					}  else {
						pData.bestRaceTime = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( TOTAL_COMPLETE_MATCHES ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.totalMatches = (gsi_u32)( field->mValue.mInt );
					}  else {
						pData.totalMatches = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_WINS ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.careerWins = (gsi_u32)( field->mValue.mInt );
					}  else {
						pData.careerWins = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CAREER_LOSSES ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.careerLosses = (gsi_u32)( field->mValue.mInt );
					}  else {
						pData.careerLosses = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( CURRENT_WIN_STREAK ), records->mRecords[i], 10 );
					if (field != NULL) {
						pData.currentWinStreak = (gsi_u32)( field->mValue.mInt );
					}  else {
						pData.currentWinStreak = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LATITUDE ), records->mRecords[i], 10 );
					if (field != NULL) {
						lat = field->mValue.mFloat;
					}  else {
						NSLog(@"- no lat  ");
						lat = 0;
					}
					field = sakeGetFieldByName( ATLAS_GET_STAT_NAME( LONGITUDE ), records->mRecords[i], 10 );
					if (field != NULL) {
						lng = field->mValue.mFloat;
					}  else {
						NSLog(@" - no long - ");
						lng = 0;
					}
					NSLog(@" at lat=%f, long=%f\n", lat, lng);
					
					pData.playerLocation = [[CLLocation alloc] initWithLatitude: lat longitude: lng];
					[gLeaderboardData addObject: pData];
				}
				
				// redraw the map
				[self viewDidLoad];
			} else {
				if (alertView != nil) {
					// remove the previous alert
					[alertView dismissWithClickedButtonIndex: alertView.firstOtherButtonIndex animated: NO];
					[alertView release];
					alertView = nil;
				}
				alertView = [[UIAlertView alloc] initWithTitle: nil message: @"No nearby leaders" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
				[alertView show];
			}
		}
	} else {
		NSLog(@"Failed to search by location, error: %d\n", result);
	}
}

@end

void SearchForLocationRecordsCallback( SAKE sake, SAKERequest request, SAKERequestResult result, void * inputData, void * outputData, void * userData )
{
	[(LeadersByLocationController *)userData sakeSearchForLocationRecordsRequestCallback: sake request: request result: result inputData: inputData outputData: outputData ];
}

