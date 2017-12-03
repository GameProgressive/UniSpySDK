///////////////////////////////////////////////////////////////////////////////
// File:	sampleAuth.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../common/gsCommon.h"
#include "../common/gsAvailable.h"
#include "../webservices/AuthService.h"
#include "sampleCommon.h"

#ifdef SAKE_SAMPLE
#include "../sake/sake.h"
#endif

#ifdef ATLAS_SAMPLE
#include "../sc/sc.h"
#endif

// The availability check needs to be called prior to starting up any of the SDKs - this
// allows the game to gracefully handle planned outages
gsi_bool AvailabilityCheck()
{
    GSIACResult aResult = GSIACWaiting; 
	printf("  Checking availability...\n");
    GSIStartAvailableCheck(SAMPLE_GAMENAME);
    while(aResult == GSIACWaiting)
    {
        aResult = GSIAvailableCheckThink();
        msleep(5);
    }

	// This result will only get set if GameSpy services are no longer supported by your title
    if (aResult == GSIACUnavailable)
    {
        printf("    Online Services for <insert your game name here> are no longer available\n");
        return gsi_false;
    }

	// GameSpy will set this result during planned outages so that users can be notified immediately
	// (prior to actual connection attempts to different services where you would have to wait for a timeout)
    if (aResult == GSIACTemporarilyUnavailable)
    {
        printf("    Online Services for <insert your game name here> are temporarily down for maintenance\n");
        return gsi_false;
    }

	printf("    Success - online services are available\n");

    return gsi_true;
}

// triggered when we receive a response from AuthService after our wsLogin call 
void SampleAuthenticationCallback(GHTTPResult httpResult, WSLoginResponse * theResponse, void * theUserData)
{
	// we use theUserData to track login completion
    int * loginComplete = (gsi_bool *)theUserData;

	// an HTTP specific error was encountered (prior to actual request processing on the backend)
	if (httpResult != GHTTPSuccess)
	{
        printf("    Authentication failed, HTTP error: %s (%d)\n", ghttpResultString(httpResult), httpResult);
		*loginComplete = -1;
	}
    // authentication failed for this user, check the mLoginResult in the WSLoginResponse for details
	else if (theResponse->mLoginResult != WSLogin_Success)
	{
		printf("    Authentication failed, Login result: %s (%d)\n", wsLoginValueString(theResponse->mLoginResult), theResponse->mLoginResult);
		*loginComplete = -1;
	}
	else // login success
	{
        printf("    Authentication success - we now have a certificate for player %s\n", theResponse->mCertificate.mUniqueNick);	
		*loginComplete = 1;
		// save off the profileId, mCertificate, and mPrivateData, to be used for Atlas/Sake/D2G/Brigades 
		// authentication
		memcpy(&certificate, &theResponse->mCertificate, sizeof(GSLoginCertificate));
		memcpy(&privateData, &theResponse->mPrivateData, sizeof(GSLoginPrivateData));
	}
}

// Authenticates the player (based on UniqueNick and password) and 'thinks' until the callback is 
// triggered to verify successful authentication
gsi_bool SampleAuthenticatePlayer(const char * uniqueNick, const char * password)
{
	gsi_u32 result;   // for the (local) return from the wsLogin functions

	int loginComplete = 0; // this will be set to 1 once our login callback is triggered

	// Authenticate the player profile via his uniquenick and password, using the default GameSpy 
	// namespace and partnercode (meaning he can use the same GameSpy account as that used for IGN 
	// and most other GameSpy-Powered games)
	// *Note: the cdkeyhash parameter is currently unused, so you can just pass NULL
    printf("  Attempting to authenticate our GameSpy profile...\n");
	result = wsLoginUnique(SAMPLE_GAME_ID, WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, 
    uniqueNick, password, NULL, SampleAuthenticationCallback, &loginComplete);

	if (result != WSLogin_Success) // local issue prior to request actually going out
    {
        printf("  Failed setting up login call. Result: %d\n", result);
        return gsi_false;
    }

	// keep 'thinking' while we wait for the wsLogin callback
    while (loginComplete == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
        gsCoreThink(0);
    }

	if (loginComplete < 0) // we set loginComplete to -1 if the callback returns an error
		return gsi_false;

	return loginComplete;
}

// when this is complete we'll have a 'certificate' to pass to other SDK calls to identify the player
// and prove he has successfully authenticated
gsi_bool AuthSetup()
{
	// Initialize SDK core/common objects for the auth service and task management  
	gsCoreInitialize();

	// Before auth service login you must first set your game identifiers
	// (this is used to isolate the title for metric tracking, and the metrics are
	// visualized on the developer portal Dashboard)
	wsSetGameCredentials(SAMPLE_ACCESS_KEY, SAMPLE_GAME_ID, SAMPLE_SECRET_KEY);

	// authenticate the player
	if (!SampleAuthenticatePlayer(SAMPLE_UNIQUENICK[0], SAMPLE_PASSWORD))
		return gsi_false;

	return gsi_true;
}

// shutdown the task management system used by autheservice and various other SDKs
void CoreCleanup()
{
	// shutdown core (the task management system), which cancels any tasks in progress
    gsCoreShutdown();

    // Wait for core shutdown (should be instantaneous unless you have multiple cores)
    while(gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        gsCoreThink(0);
        msleep(5);
    }
}

#ifdef SAKE_SAMPLE
// initialize the 'SAKE' object and set the game and profile credentials
gsi_bool SakeSetup()
{
	SAKEStartupResult startupResult;

	// initialize the SAKE object, which will then be passed to subsequent SDK calls 
	startupResult = sakeStartup(&sake);
	if(startupResult != SAKEStartupResult_SUCCESS)
	{
		printf("  Failed starting up Sake: %d\n", startupResult);
		return gsi_false;
	}

	// identify our game along with the secret key to verify authenticity 
	sakeSetGame(sake, SAMPLE_GAMENAME, SAMPLE_GAME_ID, SAMPLE_SECRET_KEY);
	
	// identify the user with the profileid, certificate, and privatedata from the AuthService
	// login callback
	sakeSetProfile(sake, certificate.mProfileId, &certificate, &privateData);

	return gsi_true;
}

const char * SakeFieldValueToString(SAKEField * field)
{
	static char buffer[64];
	SAKEFieldType type = field->mType;
	SAKEValue * value = &field->mValue;

	if(type == SAKEFieldType_BYTE)
		sprintf(buffer, "%d", (int)value->mByte);
	else if(type == SAKEFieldType_SHORT)
		sprintf(buffer, "%d", (int)value->mShort);
	else if(type == SAKEFieldType_INT)
		sprintf(buffer, "%d", value->mInt);
	else if(type == SAKEFieldType_FLOAT)
		sprintf(buffer, "%0.3f", value->mFloat);
	else if(type == SAKEFieldType_ASCII_STRING)
		return value->mAsciiString;
	else if(type == SAKEFieldType_UNICODE_STRING)
		UCS2ToAsciiString(value->mUnicodeString, buffer);
	else if(type == SAKEFieldType_BOOLEAN)
		return (value->mBoolean)?"true":"false";
	else if(type == SAKEFieldType_DATE_AND_TIME)
	{
		char * str = gsiSecondsToString(&value->mDateAndTime);
		str[strlen(str) - 1] = '\0';
		return str;
	}
	else if(type == SAKEFieldType_BINARY_DATA)
	{
		int i;
		int len = GS_MIN(value->mBinaryData.mLength, 8);
		for(i = 0 ; i < len ; i++)
			sprintf(buffer + (len*2), "%0X", value->mBinaryData.mValue[i]);
		buffer[len*2] = '\0';
	}
	else if(type == SAKEFieldType_INT64)
	{
		gsiInt64ToString(buffer, value->mInt64);
	}
	else
		return "      ERROR!!  Invalid value type set";

	return buffer;
}

// iterate through the fields of each Sake record and print out the values
void DisplaySakeRecords(SAKEField ** records, int numRecords, int numFields)
{
	SAKEField * field;
	int recordIndex;
	int fieldIndex;

	// loop through the records
	for(recordIndex = 0 ; recordIndex < numRecords ; recordIndex++)
	{
		// loop through the fields
		for(fieldIndex = 0 ; fieldIndex < numFields ; fieldIndex++)
		{
			field = &records[recordIndex][fieldIndex];

			if (field->mType == SAKEFieldType_NULL) // this shouldn't happen unless it's an Atlas stat with no default
				printf("      %s is null\n", field->mName);
			else // print the name and value of the field
				printf("      %s: %s\n", field->mName, SakeFieldValueToString(field));
		}
	}
}

// helper function to copy the records received from SAKE for later use.
SAKEGetMyRecordsOutput * CopySakeRecordsFromResponseData(SAKEGetMyRecordsInput  *input, SAKEGetMyRecordsOutput *output)
{
	SAKEGetMyRecordsOutput *myRecord = (SAKEGetMyRecordsOutput *) gsimalloc (sizeof(SAKEGetMyRecordsOutput));
	int i, j;

	if (output->mNumRecords >0)
	{
		// Now we can traverse the records
		myRecord->mNumRecords = output->mNumRecords ;

		myRecord->mRecords = (SAKEField **) gsimalloc( sizeof(SAKEField *) * myRecord->mNumRecords);

		for (j=0; j<output->mNumRecords; j++)
		{
			myRecord->mRecords[j] = (SAKEField *) gsimalloc( sizeof(SAKEField) * input->mNumFields );
			for( i=0 ; i<input->mNumFields; i++)
			{
				myRecord->mRecords[j][i].mName = (char *) gsimalloc(strlen(output->mRecords[j][i].mName)+1);
				strcpy(myRecord->mRecords[j][i].mName , output->mRecords[j][i].mName);
				switch (output->mRecords[j][i].mType)
				{
					case SAKEFieldType_ASCII_STRING:
						
						myRecord->mRecords[j][i].mValue.mAsciiString = (char *) gsimalloc(strlen(output->mRecords[j][i].mValue.mAsciiString)+1);
						strcpy(myRecord->mRecords[j][i].mValue.mAsciiString , output->mRecords[j][i].mValue.mAsciiString);	
						break;
					case SAKEFieldType_INT:
						myRecord->mRecords[j][i].mValue.mInt = output->mRecords[j][i].mValue.mInt;
						break;
					default:
						printf("Unexpected type\n");
				}
			}
		}	
	}
	return  myRecord;
}

// helper function to free record memory allocated in a Sake search callback (eg. sakeGetMyRecords callback)
void FreeSakeRecords(SAKEGetMyRecordsOutput *records, int numberOfFields)
{
	int i,j;

	for (i=0; i< records->mNumRecords; i++)
	{
		for (j= 0 ; j< numberOfFields; j++)
		{
			gsifree(records->mRecords[i][j].mName);
			// we only use ascci for this sample
			if (records->mRecords[i][j].mType == SAKEFieldType_ASCII_STRING)
				gsifree(records->mRecords[i][j].mValue.mAsciiString);
		}
		gsifree(records->mRecords[i]);
	}
	gsifree(records->mRecords);
}

// this callback verifies whether the record was successfully deleted
static void DeleteRecordCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void *inputData, void *outputData, void *userData)
{
	// pointer to track the status of the delete
	int * recordDeleted = (int *)userData;

	if (result != SAKERequestResult_SUCCESS) // something went wrong, record wasn't deleted
	{
		printf("    There was an error deleting our Sake record. Result : %d", result);
		*recordDeleted = -1;
		return;
	}
	
	// record was deleted successfully
	*recordDeleted = 1;
	printf("    Success - our Sake record was deleted\n");

	// we don't really need these since we just care about the result
	GSI_UNUSED(sake);
	GSI_UNUSED(request);
	GSI_UNUSED(inputData);
	GSI_UNUSED(outputData);
}

// This function will delete a specific record from a specific table using sakeDeleteRecord
gsi_bool DeleteSakeRecord(char * tableName, int recordId)
{
	// this input object will just need the table name and record ID
	SAKEDeleteRecordInput input; 
	SAKERequest request;  // will get set to an object that tracks the Sake request (this will also get 
	                      // passed back to us in the callback)
	SAKEStartRequestResult startRequestResult;

	// this will be set to 1 once our callback is triggered after sakeDeleteRecord (if successful)
	int recordDeleted = 0;

	// set the input variables
	input.mTableId = tableName;
	input.mRecordId = recordId;

	// send out the DeleteRecord request (then we'll need to 'think' while we wait for the response)
	request = sakeDeleteRecord(sake, &input, DeleteRecordCallback, &recordDeleted);
	
	if(!request) // local issue prior to the request actually going out
	{
		// gets the result of the most recent Sake call attempted
		startRequestResult = sakeGetStartRequestResult(sake);
	    printf("  Failed to start Sake request: %d", startRequestResult);
		return gsi_false;
	}
	
	// keep 'thinking' while we wait for the delete record callback
    while (recordDeleted == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
		sakeThink();
    }

	if (recordDeleted < 0) // we set recordDeleted to -1 if the callback returns an error
		return gsi_false;
	
	return gsi_true;
}

// shutdown Sake and Core
void SakeCleanup()
{
	// shutdown sake (freeing the 'SAKE' object)
	sakeShutdown(sake);

	// shutdown core
	CoreCleanup();
}
#endif // above only needed for Sake samples

#ifdef ATLAS_SAMPLE

// callback to verify whether the session was created without issue; once this is triggered (assuming
// success) you can obtain the session ID with scGetSessionId, in order to then share it with the other
// players in the match
static void createSessionCallback(SCInterfacePtr theInterface,
						   GHTTPResult httpResult,
                           SCResult result,
						   void * userData)
{
	// pointer to track the status of the session create
	int * sessionCreated = (int *)userData;

	// an HTTP specific error was encountered (prior to actual request processing on the backend)
	if (httpResult != GHTTPSuccess)
	{
        printf("    Create Session failed, HTTP error: %s (%d)\n", ghttpResultString(httpResult), httpResult);
		*sessionCreated = -1;
	}
    // session creation failed
	else if (result != SCResult_NO_ERROR)
	{
		printf("    Create Session failed, Result: %d\n", result);
		*sessionCreated = -1;
	}
	else // success - session created
	{
		//const char * sessionId = scGetSessionId(theInterface)
		printf("    Success - the session has been created\n");
		*sessionCreated = 1;
	}

	GSI_UNUSED(theInterface);
}

// callback to verify whether the report intention was set without issue; after this has been triggered 
// (assuming success) you can obtain the player's connection ID with scGetConnectionId, in order to then 
// share this with the other players in the match (since they'll need to pass it to scReportSetPlayerData 
// when building their match report)
static void setReportIntentionCallback(SCInterfacePtr theInterface,
						   GHTTPResult httpResult,
                           SCResult result,
						   void * userData)
{
	// pointer to track the status of the report intention setting
	int * reportIntentionSet = (int *)userData;

	// an HTTP specific error was encountered (prior to actual request processing on the backend)
	if (httpResult != GHTTPSuccess)
	{
        printf("    Failed setting report intention, HTTP error: %s (%d)\n", ghttpResultString(httpResult), httpResult);
		*reportIntentionSet = -1;
	}
    // set report intention failed
	else if (result != SCResult_NO_ERROR)
	{
		printf("    Failed setting report intention, Result: %d\n", result);
		*reportIntentionSet = -1;
	}
	else // success - we now have a connection ID to identify us in this match
	{
		//const char * connectionId = scGetConnectionId(theInterface);
		printf("    Success - report intention has been set\n");
		*reportIntentionSet = 1;
	}

	GSI_UNUSED(theInterface);
}

// Initialize Atlas and do all the prerequisite tasks necessary before creating/submitting a stats
// report. The host (playerIndex 0) will need to create the session, while the other players will need
// to set their session ID to match that created by the host. Then all players will set their report
// intention, specifying 'Authoritative' so that all reports have equal weight in the arbitration process.
//
// Note that players would normally go through this setup process separately in their respective 
// applications, but for the sake of having a full roster we're having all players report from the same
// sample app.
gsi_bool AtlasReportingSetup(SCInterfacePtr * statsInterface, int playerIndex, char * sessionId)
{
	SCResult result = SCResult_NO_ERROR;
	
	// we'll use these to track status on our asynchronous calls
	int sessionCreated = 0;
	int reportIntentionSet = 0;

	// this will initialize our interface object, which we'll need to pass to the rest of the SC calls
	result = scInitialize(SAMPLE_GAME_ID, statsInterface);
	if (result != SCResult_NO_ERROR)
	{
		printf("  Failed initializing Atlas : %d\n", result);
		return gsi_false;
	}

	// only the host needs to create a session; the clients will just need to set their session IDs
	if (playerIndex == 0)
	{
		// we'll use the certificates array since we're authenticating multiple players
		// (but seriously, don't try this at home!)
		memcpy(&certificates[playerIndex], &certificate, sizeof(GSLoginCertificate));
		memcpy(&privateDatas[playerIndex], &privateData, sizeof(GSLoginPrivateData));

		// create the session, then we can grab the sessionID (scGetSessionId) to pass to the other players
		// in the match. we pass 0 for the timeout parameter to wait indefinitely for the callback 
		printf("  The host is creating a session...\n");
		result = scCreateSession(*statsInterface, &certificates[playerIndex], &privateDatas[playerIndex], createSessionCallback, 0, &sessionCreated);  
		if (result != SCResult_NO_ERROR)
		{
			printf("  Failed creating session: %d\n", result);
			return gsi_false;
		}

		// keep 'thinking' while we wait for the create session callback
		while (sessionCreated == 0)
		{
			// normally you would be doing other game stuff here; we just sleep because we have nothing 
			// better to do
			msleep(10); 
			scThink(*statsInterface);
		}
		if (sessionCreated < 0) // we set sessionCreated to -1 if the callback returns an error
			return gsi_false;
	}
	else // non-host players need to set their sessionId (after authenticating)
	{
		// authenticate the player (we must kill and re-init the core first since it doesn't handle
		// authenticating multiple players)
		CoreCleanup();
		gsCoreInitialize();
		wsSetGameCredentials(SAMPLE_ACCESS_KEY, SAMPLE_GAME_ID, SAMPLE_SECRET_KEY);
		if (!SampleAuthenticatePlayer(SAMPLE_UNIQUENICK[playerIndex], SAMPLE_PASSWORD))
			return gsi_false;

		// set the respective certificate data, which we'll need to pass to subsequent ATLAS calls
		memcpy(&certificates[playerIndex], &certificate, sizeof(GSLoginCertificate));
		memcpy(&privateDatas[playerIndex], &privateData, sizeof(GSLoginPrivateData));

		// set the session ID, which the client will have received from the host; this way the ATLAS 
		// system will know they are in the same match. note that this must be called prior to setting
		// report intention (scSetReportIntention)
		scSetSessionId(*statsInterface, (gsi_u8 *)sessionId);
	}

	// all players need to set a report intention - we will have all reports be 'Authoritative' so that 
	// each player's reported data has equal weight in arbitration. note that the ConnectionID parameter 
	// should be NULL unless the game supports late entry and this player was in this same match previously,
	// and we pass 0 for the timeout parameter to wait indefinitely for the callback 
	printf("  Setting report intention for player %d...\n", playerIndex+1);
	result = scSetReportIntention(*statsInterface, NULL, gsi_true, &certificates[playerIndex], &privateDatas[playerIndex], setReportIntentionCallback, 0, &reportIntentionSet);
	if (result != SCResult_NO_ERROR)
	{
		printf("  Failed setting report intention: %d\n", result);
		return gsi_false;
	}

	// keep 'thinking' while we wait for the set report intention callback
    while (reportIntentionSet == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
		scThink(*statsInterface);
    }
	if (reportIntentionSet < 0) // we set reportIntentionSet to -1 if the callback returns an error
		return gsi_false;

	return gsi_true;
}

// all we need to do before running Atlas Queries is initialize the SDK (we do not create a session or
// set report intention or any of the stuff specific to Atlas Reporting
gsi_bool AtlasQuerySetup(SCInterfacePtr * statsInterface)
{
	// this will initialize our interface object, which we'll need to pass to the rest of the SC calls
	SCResult result;
	result = scInitialize(SAMPLE_GAME_ID, statsInterface);
	if (result != SCResult_NO_ERROR)
	{
		printf("  Failed initializing Atlas : %d\n", result);
		return gsi_false;
	}
	return gsi_true;
}

// iterate through the each stat for each player in the response (from the player stats query callback),
// printing out the names and values
void DisplayAtlasPlayerRecords(SCPlayerStatsQueryResponse * response)
{
	unsigned int i,j; // for looping through players/stats

	// loop through each player
	for(i= 0; i < response->mCategories->mPlayersCount; i++) 
	{ 
		// loop through each stat for this player
		for(j= 0; j < response->mCategories->mPlayers[i].mStatsCount; j++)
		{  
			// "<stat name>:<statvalue>" 
			printf("      %s:%s\n", response->mCategories->mPlayers[i].mStats[j].mName,
			       response->mCategories->mPlayers[i].mStats[j].mValue);  
		} 
		printf("      profileid:%d\n\n",response->mCategories->mPlayers[i].mProfileId);
	}
}

// shutdown ATLAS and Core, taking in the stats interface pointer since that's what needs to be freed
void AtlasCleanup(SCInterfacePtr statsInterface)
{
	// shutdown ATLAS (freeing the interface object)
	scShutdown(statsInterface);

	// shutdown core
	CoreCleanup();
}
#endif // above only needed for ATLAS samples

// This function will keep the command line prompt open until the user responds
void WaitForUserInput()
{
	fflush(stderr);
    printf("  Cleanup complete - press Enter to exit\r\n"); 
    fflush(stdout);
    getc(stdin);
}
