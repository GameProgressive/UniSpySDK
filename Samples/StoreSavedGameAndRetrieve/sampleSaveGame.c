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
#include "../../sake/sake.h"
#include "../sampleCommon.h"

// Dummy data for populating bit mask when creating fake game data to save
const gsi_u8 hasShoes = 1;                   // 2^0  (00000001)
const gsi_u8 hasAxe = 2;                     // 2^1  (00000010)
const gsi_u8 hasShield = 4;                  // 2^2  (00000100)
const gsi_u8 hasLeatherArmor = 8;            // 2^3  (00001000)
const gsi_u8 hasDragonScaleArmor = 16;       // 2^4  (00010000)
const gsi_u8 hasMount = 32;                  // 2^5  (00100000)
const gsi_u8 hasMap = 64;                    // 2^6  (01000000)
const gsi_u8 hasSwordOfThousandTruths = 128; // 2^7  (10000000)

int newRecordId = 0; // Sake recordid for our new record (once we create it)
char * SavedGameTable = "SavedGameData"; // name of the Sake table we're using to store saved games

// populates the fields passed with arbitrary dummy data
// (pretending we're saving our status in an RPG game)
static void CreateGameData(SAKEField * fields)
{
	int fieldIndex = 0;
	
	// byte-sized bit mask to store which pieces of equipment our character has
	gsi_u8 equipment = hasShoes + hasAxe + hasLeatherArmor + hasMap;

	// populate the name (as it is named on the Sake admin site), type, and value for each
	// of the fields for the record that we'll be creating
	fields[fieldIndex].mName = "Level";
	fields[fieldIndex].mType = SAKEFieldType_INT;
	fields[fieldIndex].mValue.mInt = 28;
	fieldIndex++;
	fields[fieldIndex].mName = "Class";
	fields[fieldIndex].mType = SAKEFieldType_ASCII_STRING;
	fields[fieldIndex].mValue.mAsciiString = "Rogue";
	fieldIndex++;
	fields[fieldIndex].mName = "Location";
	fields[fieldIndex].mType = SAKEFieldType_ASCII_STRING;
	fields[fieldIndex].mValue.mAsciiString = "Shadow Rift";
	fieldIndex++;
	fields[fieldIndex].mName = "Equipment";
	fields[fieldIndex].mType = SAKEFieldType_BYTE;
	fields[fieldIndex].mValue.mByte = equipment;
}

static gsi_bool Setup()
{
	// follow these helper functions into sampleCommon.c for details on what we need to do to setup 
	// for the services needed
	if (!AvailabilityCheck())
		return gsi_false;

	if (!AuthSetup())
		return gsi_false;

	if (!SakeSetup())
		return gsi_false;

	return gsi_true;
}

static gsi_bool Cleanup()
{
	// for the sake of keeping the sample table clean we'll delete the saved game record 
	// that we created
	printf("  Deleting our saved game record...\n");
	if (!DeleteSakeRecord(SavedGameTable, newRecordId))
		return gsi_false;

	// shutdown Sake and Core (which ensures internal objects get freed)
	SakeCleanup();

	// keep the command line prompt open so you can view the output
	WaitForUserInput();

	return gsi_true;
}

// triggered when we receive a response from Sake after our CreateRecord call
static void SampleStoreSavedGameCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void *inputData, void *outputData, void *userData)
{
	// convert the void pointer outputData into the appropriate structure
	SAKECreateRecordOutput * output = (SAKECreateRecordOutput *)outputData;
	
	// we passed this int pointer for userData so we can set it accordingly
	int * recordId = (int *)userData; 

	if (result != SAKERequestResult_SUCCESS) // something went wrong, record wasn't created
	{
		printf("    There was an error creating the Sake record: %d", result);
		*recordId = -1; // we'll know in our think loop that there was an error if it's -1
		return;
	}
	
	*recordId = output->mRecordId;  // set the recordid of our newly created record
	printf("    Success - our new record has recordid %d\n", output->mRecordId);

	GSI_UNUSED(sake);
	GSI_UNUSED(request);
	GSI_UNUSED(inputData);
	GSI_UNUSED(userData);
}

// triggered when we receive a response from Sake after our GetMyRecords call
static void SampleRetrieveSavedGameCallback(SAKE sake, SAKERequest request, SAKERequestResult result, void *inputData, void *outputData, void *userData)
{
	// convert the void pointers into the appropriate structures
	SAKEGetMyRecordsInput * input = (SAKEGetMyRecordsInput *)inputData;
	SAKEGetMyRecordsOutput * output = (SAKEGetMyRecordsOutput *)outputData;

	// we need to set this to 1 upon success so that we'll know we can stop thinking
	int * dataRetrieved = (int *)userData; 

	if (result != SAKERequestResult_SUCCESS) // something went wrong, no record retrieved
	{
		printf("    There was an error retrieving our Sake record: %d", result);
		*dataRetrieved = -1;
		return;
	}
	
	// we got our record, now let's print it out
	*dataRetrieved = 1;
	printf("    Success - our Sake record was retrieved: \n");
	DisplaySakeRecords(output->mRecords, output->mNumRecords, input->mNumFields);

	GSI_UNUSED(sake);
	GSI_UNUSED(request);
	GSI_UNUSED(userData);
}

// Use sakeCreateRecord to store data in the Sake 'cloud'. 
// Note that if the player already had a record (which you can determine with sakeGetMyRecords) 
// you would instead use sakeUpdateRecord
static gsi_bool SampleStoreSavedGame()
{
	SAKECreateRecordInput recordData;  // will store our fields for the record we want to create
	SAKERequest request;  // will get set to an object that tracks the Sake request (this will also get 
	                      // passed back to us in the callback)
	SAKEField fields[4];  // to identify the fields in the Sake table and set values for each

	// create some dummy data to act as our 'saved game', and populate the fields with this data
	CreateGameData(fields);
	
	// now that the fields have data we can setup the recordData
	recordData.mFields = fields;
	recordData.mNumFields = 4;

	// the table (along with the fields) must first be created on the Sake admin site
	recordData.mTableId = SavedGameTable;

	// send off the CreateRecord request and then wait for a response via the callback
	// we pass 'newRecordId' for the userData so that we can set this in the callback  
	printf("  Creating Sake record...\n");
	request = sakeCreateRecord(sake, &recordData, SampleStoreSavedGameCallback, &newRecordId);
	
	if(!request) // local issue prior to the request actually going out
	{
		// gets the result of the most recent Sake call attempted
		SAKEStartRequestResult startRequestResult = sakeGetStartRequestResult(sake);
	    printf("  Failed to start Sake request: %d", startRequestResult);
		return gsi_false;
	}

	// keep 'thinking' while we wait for the create record callback (SampleStoreSavedGameCallback)
    while (newRecordId == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
		sakeThink();
    }

	if (newRecordId < 0) // we set newRecordId to -1 if the callback returns an error
		return gsi_false;
	
	return gsi_true;
}

static gsi_bool SampleRetrieveSavedGame()
{
	// this input object will store the fields that we want to retrieve values for
	SAKEGetMyRecordsInput myRecordToRetrieve;
	SAKERequest request;  // will get set to an object that tracks the Sake request (this will also get 
	                      // passed back to us in the callback)
	SAKEStartRequestResult startRequestResult;
	
	// this will be set to 1 once our callback is triggered after sakeGetMyRecords (if successful)
	int savedGameDataRetrieved = 0;

	// we'll query all fields except 'recordid'
	char *fieldNames[] = {"ownerid", "Level", "Class", "Location", "Equipment"};
	myRecordToRetrieve.mTableId = SavedGameTable; // same table from the admin site
	myRecordToRetrieve.mFieldNames = fieldNames;
	myRecordToRetrieve.mNumFields = 5; 

	printf("  Getting our Sake record...\n");
	// send off the GetMyRecords request and then wait for a response via the callback
	request = sakeGetMyRecords(sake, &myRecordToRetrieve, SampleRetrieveSavedGameCallback, &savedGameDataRetrieved);
	
	if(!request) // local issue prior to the request actually going out
	{
		// gets the result of the most recent Sake call attempted
		startRequestResult = sakeGetStartRequestResult(sake);
	    printf("Failed to start Sake request: %d", startRequestResult);
		return gsi_false;
	}

	// keep 'thinking' while we wait for the retrieve saved game callback
    while (savedGameDataRetrieved == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
		sakeThink();
    }

	if (savedGameDataRetrieved < 0) // we set savedGameDataRetrieved to -1 if the callback returns an error
		return gsi_false;
	
	return gsi_true;
}

int test_main()
{	
	// first do the prerequisite initialization and such so we'll be ready to authenticate a player
    printf("Doing the prerequisite setup and initialization\n");
	if (!Setup())
		return -1;

	// the beef of the sample app - you can also reference this sample code on the developer portal 'Docs'
	printf("Storing our saved game data\n");
	if (!SampleStoreSavedGame())
		return -1;

	printf("Retrieving our saved game data\n");
	if (!SampleRetrieveSavedGame())
		return -1;

	// shutdown the Core object, then we'll wait for user input before killing the app
	printf("Cleaning up after ourselves\n");
	if (!Cleanup())
		return -1;
    
	return 0;
}
