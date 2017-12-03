///////////////////////////////////////////////////////////////////////////////
// File:	sampleAuth.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../../common/gsCommon.h"
#include "../../common/gsCore.h"
#include "../../common/gsAvailable.h"
#include "../../webservices/AuthService.h"
#include "../../sc/sc.h"
#include "../sampleCommon.h"
#include "atlas_atlasSamples_v2.h"

// this will make our lives easier since every atlas call returns a local result
#define CHECK_SCRESULT(result) if(result != SCResult_NO_ERROR) { return gsi_false; }

#define NUM_PLAYERS 3

// cars
const int CORVETTE = 0;
const int VIPER = 1;

// tracks
const int DAYTONA_BEACH = 0;
const int MONACO = 1; 

typedef struct SampleMatchData
{
	char sessionId[SC_SESSION_GUID_SIZE];
	int track;
	gsi_u8 corvetteUses;
	gsi_u8 viperUses;

	SCGameStatus status;
} SampleMatchData;
static SampleMatchData matchData;

typedef struct SamplePlayerData
{
	GSLoginCertificate * certificate;
	GSLoginPrivateData * privateData;

	SCInterfacePtr statsInterface;
	
	SCReportPtr report;

	gsi_u8 connectionId[SC_CONNECTION_GUID_SIZE];
	
	SCGameResult result;
	int vehicleUsed;
	gsi_u64 raceTime;

	char uniquenick[20]; // 20 is the max length for a GameSpy uniquenick

} SamplePlayerData;
static SamplePlayerData playerData[NUM_PLAYERS];

// A utility to get a random integer
static int myGetRandomInt(int theMax)
{
	return (int)((float)rand()/RAND_MAX*theMax);
}

// generates match data such as race time and win/loss results for each player in the match so that we can
// then populate and submit our match report for ATLAS
static void PlayFakeMatch()
{
	int i, j; // for looping through players
	gsi_u64 bestRaceTime = 0;

	srand(current_time()); // random seed for the random number generator

	printf("  Playing our fake match so that we'll have some match data to report to ATLAS\n");
	// match (global) data - set the track and initialize the vehicle uses (which will increment for each 
	// player that uses that vehicle)
	matchData.track = myGetRandomInt(2);
	matchData.corvetteUses = 0;
	matchData.viperUses = 0;

	// player data - set the nick and racetime and vehicle used by each player
	for (i = 0; i < NUM_PLAYERS; i++)
	{
		strcpy(playerData[i].uniquenick, SAMPLE_UNIQUENICK[i]);
		playerData[i].vehicleUsed = myGetRandomInt(2);
		if (playerData[i].vehicleUsed == CORVETTE)
			matchData.corvetteUses++;
		else if (playerData[i].vehicleUsed == VIPER)
			matchData.viperUses++;
		playerData[i].raceTime = myGetRandomInt(300000);; // between 0 and 5 minutes (we'll assume unit is milliseconds)
		if (bestRaceTime == 0 || playerData[i].raceTime < bestRaceTime)
			// keep track of the fastest race time so we can determine the winner
			bestRaceTime = playerData[i].raceTime; 
	}

	// loop through players once more to set results (best race time wins, all others lose)
	for (j = 0; j < NUM_PLAYERS; j++)
	{
		if (playerData[j].raceTime == bestRaceTime)
			playerData[j].result = SCGameResult_WIN;
		else
			playerData[j].result = SCGameResult_LOSS;
	}

	// status will be needed for the match report, to show that the match is finished (so that the backend
	// knows to close the session and commence normalization and processing)
	matchData.status = SCGameStatus_COMPLETE;
}

static gsi_bool Setup()
{
	int i;

	// follow these helper functions into sampleCommon.c for details on what we need to do to setup 
	// for the services needed
	if (!AvailabilityCheck())
		return gsi_false;

	if (!AuthSetup())
		return gsi_false;

	// Atlas setup for host (player index 0)
	if (!AtlasReportingSetup(&playerData[0].statsInterface, 0, NULL))
		return gsi_false;

	// host needs to hand-off the session ID to the clients - since we're 'cheating' by having all 
	// players report in the same app, we use the matchData and playerData[] structs to store the data
	// that would normally need be shared across players
	strcpy(matchData.sessionId, scGetSessionId(playerData[0].statsInterface));

	// set our player-specific authentication credentials and identifier data for the host
	strcpy((char *)playerData[0].connectionId, scGetConnectionId(playerData[0].statsInterface));
	playerData[0].certificate = &certificates[0];
	playerData[0].privateData = &privateDatas[0];

	// iterate through the rest of the players to set them up for Atlas reporting (again - this would
	// normally take place seperately in each respective client application) 
	for (i = 1; i < NUM_PLAYERS; i++)
	{
		if (!AtlasReportingSetup(&playerData[i].statsInterface, i, matchData.sessionId))
			return gsi_false;
		strcpy((char *)playerData[i].connectionId, scGetConnectionId(playerData[i].statsInterface));
		playerData[i].certificate = &certificates[i];
		playerData[i].privateData = &privateDatas[i];
	}

	// now we would proceed with the match, tracking the stats that we'll include in our match report
	PlayFakeMatch();

	return gsi_true; // now we're all ready to build and submit our reports
}

// callback to verify whether our report was submitted successfully to the ATLAS backend. once the backend
// receives each report it will begin the normalization process to consolidate the reported data into 1
// report, then processing will ensue, where the rules are run with the reported keys in order to update 
// the stats
static void sampleSubmitReportCallback(SCInterfacePtr theInterface,
						   GHTTPResult httpResult,
                           SCResult result,
						   void * userData)
{
	// pointer to track the status of the report submission
	int * reportSubmitted = (int *)userData;

	// an HTTP specific error was encountered (prior to actual request processing on the backend)
	if (httpResult != GHTTPSuccess)
	{
        printf("    Create Session failed, HTTP error: %s (%d)\n", ghttpResultString(httpResult), httpResult);
		*reportSubmitted = -1;
	}
    // report submission failed
	else if (result != SCResult_NO_ERROR)
	{
		printf("    Create Session failed, Result: %d\n", result);
		*reportSubmitted = -1;
	}
	else // success - report submitted
	{
		printf("    Success - report has been submitted\n");
		*reportSubmitted = 1;
	}

	GSI_UNUSED(theInterface);
}

// create the ATLAS report and populate it with the dummy match data we generated in PlayFakeMatch
static gsi_bool SampleReportStats()
{
	int i, j; // for looping through players
	
	// this loop will *not* be needed in your game code, since only the local player will submit his 
	// report. we need it since we're having all players report from the same app for ease of demonstrating
	// a sample session
	for (i = 0; i < NUM_PLAYERS; i++)
	{
		int reportSubmitted = 0; // will be used to track status of the report submission
		
		// initialize the report object which will then be passed to the scReport functions as we populate
		// it. the ruleset version is that under Ruleset Management on the ATLAS admin site, and for this 
		// sample teams do not apply so we pass 0 for numTeams
		CHECK_SCRESULT(scCreateReport(&playerData[i].statsInterface, ATLAS_RULE_SET_VERSION, NUM_PLAYERS, 0, &playerData[i].report));

		// now we need to start with the global (match-wide) data, starting with the begin call to start 
		// the section
		CHECK_SCRESULT(scReportBeginGlobalData(playerData[i].report));

		// now we use the scReportAdd functions to write all of our game keys that have non-zero values. 
		// the key IDs are those from the ATLAS admin site, and are included in the header file 
		// (atlas_atlasSamples_v2.h) which you can download from the admin site under Ruleset Management
		CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_VEHICLE_1_G, matchData.corvetteUses));
		CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_VEHICLE_2_G, matchData.viperUses));
		
		// check which track was played and set the corredsponding USED_TRACK_<x> key
		if (matchData.track == DAYTONA_BEACH)
		{
			CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_TRACK_1, 1));
		}
		else if (matchData.track == MONACO)
		{
			CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_TRACK_2, 1));
		}

		// now that we've finished the global section, we're on to the the player section. again, we start
		// with the begin call to start this section
		CHECK_SCRESULT(scReportBeginPlayerData(playerData[i].report));

		// now we'll iterate through each player (including ourself) to set his/her data from the match
		for (j = 0; j < NUM_PLAYERS; j++)
		{
			// we need another begin call to start each player's section
			CHECK_SCRESULT(scReportBeginNewPlayer(playerData[i].report));

			// you must set player data for all players in the match - this is why players will need to 
			// share their connection ID and certificate. we pass 0 for the team ID since our match 
			// doesn't involve teams, and the AuthData is unused currently so you can just pass NULL. for
			// the result, we pass the win/loss result as determined in PlayFakeMatch
			CHECK_SCRESULT(scReportSetPlayerData(playerData[i].report, j, playerData[j].connectionId, 0, playerData[j].result, 
								playerData[j].certificate->mProfileId, playerData[j].certificate, NULL));

			CHECK_SCRESULT(scReportAddStringValue(playerData[i].report, ATLAS_KEY_PLAYER_NAME_P, playerData[j].uniquenick));

			// check which vehicle this player drove and set the corredsponding USED_VEHICLE_<x> key
			if (playerData[j].vehicleUsed == CORVETTE)
			{
				CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_VEHICLE_1_P, 1));
			}
			else if (playerData[j].vehicleUsed == VIPER)
			{
				CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_VEHICLE_2_P, 1));
			}
			
			// we pass the track as a player key as well as game key since it is used as a qualifier in 
			// Player rules (rules that output Player stats) as well as Game rules (in the Ruleset on the
			// admin site)
			if (matchData.track == DAYTONA_BEACH)
			{
				CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_TRACK_1, 1));
			}
			else if (matchData.track == MONACO)
			{
				CHECK_SCRESULT(scReportAddByteValue(playerData[i].report, ATLAS_KEY_USED_TRACK_2, 1));
			}

			// set the player's race time - this will end up hitting a 'Minimum' rule that we setup on the
			// admin site to compare this to the player's existing BEST_RACE_TIME stat, replacing the best
			// race time value if it's been beat by this new time
			CHECK_SCRESULT(scReportAddInt64Value(playerData[i].report, ATLAS_KEY_RACE_TIME_P, playerData[j].raceTime));
		}

		// now that we've looped through all the player's and added all their data, we begin the Team
		// section (this begin needs to be called even when you have no actual data to report)
		CHECK_SCRESULT(scReportBeginTeamData(playerData[i].report));

		// now that we've added all of our match data we can end the report
		CHECK_SCRESULT(scReportEnd(playerData[i].report, gsi_true, matchData.status)); 

		// having completed the report, it's now time to submit it so that it can be processed, updating 
		// the game and player-wide stats accordingly. again we pass 0 for the timeout parameter to wait 
		// indefinitely for the callback, and we reiterate here that our report will be Authoritative (this 
		// must match what we passed in scSetReportIntention)
		printf("  Submitting the ATLAS report for player %d...\n", i+1);
		CHECK_SCRESULT(scSubmitReport(playerData[i].statsInterface, playerData[i].report, gsi_true, playerData[i].certificate,
								playerData[i].privateData, sampleSubmitReportCallback, 0, &reportSubmitted)); 

		// keep 'thinking' while we wait for the submit report callback
		while (reportSubmitted == 0)
		{
			// normally you would be doing other game stuff here; we just sleep because we have nothing 
			// better to do
			msleep(10); 
			scThink(playerData[i].statsInterface);
		}

		if (reportSubmitted < 0) // we set reportSubmitted to -1 if the callback returns an error
			return gsi_false;
	}
	return gsi_true; 
	// our reports for the match session have been submitted successfully, now we can go to the Debug Viewer 
	// on the ATLAS admin site to check on it as it goes through normalization and processing
}

static gsi_bool Cleanup()
{
	// because we cheated and reported all players from the same sample app, we need to iterate through
	// each player to shutdown his specific ATLAS interface
	int i;
	for (i = 0; i < NUM_PLAYERS; i++)
	{
		// shutdown ATLAS and Core (which ensures internal objects get freed)
		AtlasCleanup(playerData[i].statsInterface);
	}

	// keep the command line prompt open so you can view the output
	WaitForUserInput();

	return gsi_true;
}

int test_main()
{	
	// first do the prerequisite initialization and such so we'll be ready to authenticate a player
    printf("Doing the prerequisite setup and initialization\n");
	if (!Setup())
		return -1;

	// the beef of the sample app - you can also reference this sample code on the developer portal 'Docs'
	printf("Reporting stats for our sample match\n");
	if (!SampleReportStats())
		return -1;

	// shutdown the Core object, then we'll wait for user input before killing the app
	printf("Cleaning up after ourselves\n");
	if (!Cleanup())
		return -1;
    
	return 0;
}
