// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import "Utility.h"
#import <mach/mach_time.h>

static mach_timebase_info_data_t InitTimebaseInfo();
static mach_timebase_info_data_t timebase_info = InitTimebaseInfo();

double absolute_to_millis;
double millis_to_absolute;
double absolute_to_seconds;
double seconds_to_absolute;

NSString* timeFormatString = @"%02u:%05.2f";
NSString* playerStatsDataFile = @"playerStats.plist";
NSString* blockedImagesDataFile = @"blockedImages.plist";

NSString* singlePlayerBestTimeKey = @"spBestTime";
NSString* singlePlayerWorstTimeKey = @"spWorstTime";
NSString* singlePlayerAverageTimeKey = @"spAverageTime";
NSString* singlePlayerGamesPlayedKey = @"spGamesPlayed";
NSString* singlePlayerTotalPlaysKey = @"spTotalPlays";
NSString* singlePlayerTotalRaceTimeKey = @"spTotalRaceTime";

NSString* matchBestTimeKey = @"BestTime";
NSString* matchWorstTimeKey = @"WorstTime";
NSString* matchAverageTimeKey = @"AverageTime";
NSString* matchGamesPlayedKey = @"GamesPlayed";

NSString* matchWonKey = @"Won";
NSString* matchLossKey = @"Loss";
NSString* matchDrawsKey = @"careerDraws";	

NSString* matchWinStreakKey = @"WinStreak";
NSString* matchLossStreakKey = @"currentLossStreak";
NSString* matchDrawStreakKey = @"currentDrawStreak";
	
NSString* matchCareerLongestWinStreakKey = @"careerLongestWinStreak";	
NSString* matchCareerLongestLossStreakKey = @"careerLongestLossStreak";
NSString* matchCareerLongestDrawStreakKey = @"careerLongestDrawStreak";

NSString* matchTotalRaceTimeKey = @"totalRaceTimeKeyotalRaceTime";		
NSString* matchCareerDisconnectsKey = @"careerDisconnects";	
NSString* matchDisconnectRateKey = @"disconnectRate";		
NSString* matchTotalCompleteMatchesKey = @"totalCompleteMatches";

NSString* lastTimePlayedKey = @"LastTimePlayed";


void SetTimeLabel(UILabel* label, int timeInMilliseconds)
{
	label.text = [NSString stringWithFormat: timeFormatString, timeInMilliseconds / (60 * 1000), (timeInMilliseconds % (60 * 1000)) / 1000.0f];
}


@interface MessageBoxDelegate : NSObject <UIAlertViewDelegate>
{
	NSInteger clickedButtonIndex;
}

@property(nonatomic, readonly) NSInteger clickedButtonIndex;

@end


@implementation MessageBoxDelegate

@synthesize clickedButtonIndex;

- (id)init
{
	self = [super init];

	if (self != nil) {
		clickedButtonIndex = -1;
	}

	return self;
}

- (void)alertView: (UIAlertView*)alertView willDismissWithButtonIndex: (NSInteger)buttonIndex
{
	clickedButtonIndex = buttonIndex;
}

@end



void MessageBox(NSString* text, NSString* title)
{
	MessageBoxDelegate* delegate = [[MessageBoxDelegate alloc] init];
	UIAlertView* alertView = [[UIAlertView alloc] initWithTitle: title message: text delegate: delegate cancelButtonTitle: @"OK" otherButtonTitles: nil];
	[alertView show];

	while (delegate.clickedButtonIndex == -1) {
		[[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.10]];
	}

	[alertView release];
	[delegate release];
}


void MessageBoxNoBlock(NSString* text, NSString* caption, NSString* cancelButtonString )
{
	UIAlertView* alertView = [[UIAlertView alloc] initWithTitle: caption message: text delegate: nil cancelButtonTitle: cancelButtonString otherButtonTitles: nil];
	[alertView show];	
	[alertView release];
}

void OutputDebugString(NSString* text)
{
#ifdef GSI_COMMON_DEBUG
	NSLog(@"%s\n", [text cStringUsingEncoding: NSASCIIStringEncoding]);
#endif
}

UIFont* GetBoldVersion(UIFont* font)
{
	// Apple doesn't make this easy.  There's no programmatic way to request the bold version of a given font,
	// so we have to use the font names as a guide.
	NSString* fontFamilyName = font.familyName;
	NSArray* fontNames = [UIFont fontNamesForFamilyName: fontFamilyName];
	NSString* boldFontName = font.fontName;
	NSUInteger shortestAttributeStringLength = NSUIntegerMax;

	// For each font name in the font's family, search for the string "bold."  After this loop, boldFontName
	// holds the best candidate.
	for (NSString* fontName in fontNames) {
		// Font names tend to follow the format "FamilyName-Attributes".  We don't want to search the family
		// name part (if there's a font family named "Kobold", don't match the "bold" part of that), so try
		// to isolate just the attributes...
		NSRange familyNameRange = [fontName rangeOfString: fontFamilyName options: NSCaseInsensitiveSearch];
		NSString* fontAttributes = [NSString stringWithFormat: @"%@ %@", [fontName substringToIndex: familyNameRange.location], [fontName substringFromIndex: familyNameRange.location + familyNameRange.length]];
		fontAttributes = [fontAttributes stringByTrimmingCharactersInSet: [NSCharacterSet characterSetWithCharactersInString: @" -"]];
		bool bold = [fontAttributes rangeOfString: @"bold" options: NSCaseInsensitiveSearch].length > 0;

		// A font can have more than one attribute, such as "ObliqueBold".  Try to match the shortest
		// attribute string -- this should be closest to pure "bold".
		if (bold && (fontAttributes.length < shortestAttributeStringLength)) {
			shortestAttributeStringLength = fontAttributes.length;
			boldFontName = fontName;
		}
	}

	if ([boldFontName isEqualToString: font.fontName]) {
		return font;
	}

	return [UIFont fontWithName: boldFontName size: font.pointSize];
}

mach_timebase_info_data_t InitTimebaseInfo()
{
	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);
	absolute_to_millis = timebase_info.numer / (1000000.0 * timebase_info.denom);
	millis_to_absolute = 1.0 / absolute_to_millis;
	absolute_to_seconds = timebase_info.numer / (1000000000.0 * timebase_info.denom);
	seconds_to_absolute = 1.0 / absolute_to_seconds;
	
	return timebase_info;
}
