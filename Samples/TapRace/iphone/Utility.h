// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#ifndef UTILITY_INCLUDED

// Allows expanding macros into string constants.
#define STRINGIZE1(x) #x
#define STRINGIZE(x) STRINGIZE1(x)

#define getLocalIntegerStat(key) [(NSNumber*)[gPlayerData.playerStatsData objectForKey: key ] intValue]
#define getLocalFloatStat(key) [(NSNumber*)[gPlayerData.playerStatsData objectForKey: key ] floatValue]
#define getLocalStringStat(key) [(NSString*)[gPlayerData.playerStatsData objectForKey: key ]]


// Multipliers for conversion between device time units and real time units.
extern double absolute_to_millis;
extern double millis_to_absolute;
extern double absolute_to_seconds;
extern double seconds_to_absolute;

// printf-style format string for displaying game time.
extern NSString* timeFormatString;

// Generic function for setting the text of a time label.
void SetTimeLabel(UILabel* label, int timeInMilliseconds);

// File name for storing single-player data.
extern NSString* playerStatsDataFile;
extern NSString* blockedImagesDataFile;

// Keys for accessing single-player data.
extern NSString* singlePlayerBestTimeKey;
extern NSString* singlePlayerWorstTimeKey;
extern NSString* singlePlayerAverageTimeKey;
extern NSString* singlePlayerGamesPlayedKey;
extern NSString* singlePlayerTotalPlaysKey;
extern NSString* singlePlayerTotalRaceTimeKey;



// Keys for accessing two-player game stats data.
extern NSString* matchBestTimeKey;
extern NSString* matchWorstTimeKey;
extern NSString* matchAverageTimeKey;
extern NSString* matchGamesPlayedKey;
extern NSString* matchWonKey;
extern NSString* matchLossKey;
extern NSString* matchWinStreakKey;
extern NSString* matchLossStreakKey;	
extern NSString* matchTotalRaceTimeKey;		
extern NSString* matchCareerDisconnectsKey;	
extern NSString* matchDisconnectRateKey;		
extern NSString* matchDrawsKey;		
extern NSString* matchDrawStreakKey;	

extern NSString* matchCareerLongestWinStreakKey;	
extern NSString* matchCareerLongestLossStreakKey;
extern NSString* matchCareerLongestDrawStreakKey;
extern NSString* matchTotalCompleteMatchesKey;

// Key for timestamp of stored stats data
extern NSString* lastTimePlayedKey;

// Debugging and user notification convenience functions.
void MessageBox(NSString* text, NSString* caption = nil);
void MessageBoxNoBlock(NSString* text, NSString* caption = nil, NSString* cancelButtonString = @"OK");
void OutputDebugString(NSString* text);

// Get the bold version of an existing font.  Returns the existing font if there is no bold verson.
UIFont* GetBoldVersion(UIFont* font);

#endif