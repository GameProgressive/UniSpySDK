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
#include "../../sc/sci.h"
#include "../../sc/sc.h"
#include "../sampleCommon.h"
#include "atlas_atlasSamples_v2.h"

// this will make our lives easier since every atlas call returns a local result
#define CHECK_SCRESULT(result) if(result != SCResult_NO_ERROR) { return gsi_false; }

SCInterfacePtr statsInterface;

static gsi_bool Setup()
{
	// follow these helper functions into sampleCommon.c for details on what we need to do to setup 
	// for the services needed
	if (!AvailabilityCheck())
		return gsi_false;

	if (!AuthSetup())
		return gsi_false;

	if (!AtlasQuerySetup(&statsInterface))
		return gsi_false;

	return gsi_true; // now we're all ready to build and submit our reports
}

static void OnPlayerStatsQueryComplete(const SCInterfacePtr theInterface,
										GHTTPResult          httpResult,
										SCResult             result,
										gsi_char *           msg,
										int                  msgLen,
										SCPlayerStatsQueryResponse * response,
										void *               userData)
{
	int * queryComplete = (int *)userData; // pointer to track the status of the report submission

	// an HTTP specific error was encountered (prior to actual request processing on the backend)
	if (httpResult != GHTTPSuccess)
	{
        printf("    Stats query failed, HTTP error: %s (%d)\n", ghttpResultString(httpResult), httpResult);
		*queryComplete = -1;
	}

    // stats query failed
	else if (result != SCResult_NO_ERROR)
	{
		printf("    Stats query failed, Result: %d\n", result);
		if (msgLen > 0)
			printf("      %s\n", msg);
		*queryComplete = -1;
	}
	else // success - report submitted
	{
		printf("    Success - here are the results:\n\n");
		*queryComplete = 1;

		// print out the records/stats in the response
		DisplayAtlasPlayerRecords(response);

		//cleanup - make sure you you've copied the response object first if you'll need it for further paging
		scDestroyPlayerStatsQueryResponse(&response);
	}
	GSI_UNUSED(theInterface);
} 

// For this sample we just search the top 5 players sorted by Win/Loss ratio. This is accomplished by
// invoking the 'PLAYER_LB' Ranked query (which we created on the ATLAS admin site), passing 5 for pagesize
// and 1 for pageindex. If we wanted the 2nd page (again assuming a 5 records per page display) the code 
// would be identical except pageindex would be 2. 

// Note that we normally recommend you query 50 or so records at once so that when the user pages up (and down)
// you don't need to make a new query until they navigate passed the 50th record. 
static gsi_bool sampleQueryLeaderboard()
{
	int rulesetVer = ATLAS_RULE_SET_VERSION;  // the ruleset version in use (as chosen on the admin site)
	int queryComplete = 0; // will be used to track status of the report submission
	SCQueryParameterListPtr queryParamsList = NULL; // we need a NULL parameter list that will then get 'created' by the SDK 
	char queryID[GS_GUID_SIZE]; // this will hold the query guid which identifies the query we wish to invoke  

	int queryParamCount = 2;  // this Player Query has 2 parameters - 'pageindex' and 'pagesize'
	
	// we want the first page of the leaderboard - eg. the top <x> records where x is the pagesize below
	gsi_char * queryParam1Name = "pageindex"; 
	gsi_char * queryParam1Value = "1";  
	
	// the number of records per page, so we will query the top 5 records 
	gsi_char * queryParam2Name = "pagesize";
	gsi_char * queryParam2Value = "5";  
	
	// if we wanted to query the 2nd page of this leaderboard (records 6-10) we would simply change the 
	// queryParam1Value above to "2"
	
	// set the query ID to the appropriate query label, as defined in the header file exported from the admin site
	sprintf(queryID, ATLAS_Query_Player_Stats_PLAYER_LB);

	// setup the query parameter list
	CHECK_SCRESULT(scCreateQueryParameterList(&queryParamsList, queryParamCount));

	// add each parameter to the list as a name/value pair
	CHECK_SCRESULT(scAddQueryParameterToList(queryParamsList, queryParam1Name, queryParam1Value));
	CHECK_SCRESULT(scAddQueryParameterToList(queryParamsList, queryParam2Name, queryParam2Value));

	// run the query (then wait for the callback); the atlas interface and login data are from previous 
	// initialization/authentication
	printf("  Invoking the player stats query...\n");
	CHECK_SCRESULT(scRunPlayerStatsQuery(statsInterface, &certificate, &privateData, rulesetVer, queryID, queryParamsList, OnPlayerStatsQueryComplete, &queryComplete));

	// cleanup - we no longer need the queryParamsList
	CHECK_SCRESULT(scDestroyQueryParameterList(&queryParamsList));

	// keep 'thinking' while we wait for the query complete callback
	while (queryComplete == 0)
	{
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
		msleep(10); 
		scThink(statsInterface);
	}

	if (queryComplete < 0) // we set queryComplete to -1 if the callback returns an error
		return gsi_false;

	return gsi_true;
}

static gsi_bool Cleanup()
{
	// shutdown ATLAS and Core (which ensures internal objects get freed)
	AtlasCleanup(statsInterface);

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
	printf("Retrieve the first page of the player stats leaderboard\n");
	if (!sampleQueryLeaderboard())
		return -1;

	// shutdown the Core object, then we'll wait for user input before killing the app
	printf("Cleaning up after ourselves\n");
	if (!Cleanup())
		return -1;
    
	return 0;
}
