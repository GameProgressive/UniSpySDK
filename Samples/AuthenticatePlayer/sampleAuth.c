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
#include "../sampleCommon.h"

static gsi_bool Setup()
{
	// first ensure the availability check passes before using any GameSpy service
	if (!AvailabilityCheck())
		return gsi_false;

	// Initialize SDK core/common objects for the auth service and task management  
	gsCoreInitialize();

	// Before auth service login you must first set your game identifiers
	// (this is used to isolate the title for metric tracking, and the metrics are
	// visualized on the developer portal Dashboard)
	wsSetGameCredentials(SAMPLE_ACCESS_KEY, SAMPLE_GAME_ID, SAMPLE_SECRET_KEY);

	return gsi_true;
}

static gsi_bool Cleanup()
{
	// shutdown core (which ensures internal objects get freed)
    CoreCleanup();

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
	printf("Authenticating the player\n");
	if (!SampleAuthenticatePlayer(SAMPLE_UNIQUENICK[0], SAMPLE_PASSWORD))
		return -1;

	// shutdown the Core object, then we'll wait for user input before killing the app
	printf("Cleaning up after ourselves\n");
	if (!Cleanup())
		return -1;
    
	return 0;
}
