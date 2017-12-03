///////////////////////////////////////////////////////////////////////////////
// File:	sampleAuth.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

//
// This sample is to show how to upload and download content using SAKE.
// Any type of file can be uploaded( e.g. txt, bmp, jpg etc.). 
// In this case, we assume they are screen shots.
// Also, we show how meta data concerning the uploaded data is created
// and retrieved using SAKE.
// The APIs used in this sample:
//	- sakeUploadContent()
//	- sakeCreateRecord()
//	- sakeDownloadContent()
//  - sakeGetMyRecords()
//

#include "../../common/gsCommon.h"
#include "../../common/gsCore.h"
#include "../../common/gsAvailable.h"
#include "../../webservices/AuthService.h"
#include "../../sake/sake.h"
#include "../sampleCommon.h"

int newFileId	= 0;   // This is the file ID returned from the backend after upload
int newRecordId = 0;	// Sake recordid for our new record (once we create it).
						// This sample uploads 1 file and creates only 1 record per profile.

char *ScreenShotTable					= "ScreenShots"; // name of the Sake table we're using to store content (screen shots in this sample)
char *ScreenShotFilenameWithPath = "./gamespylogo.bmp";
char *ScreenShotFileName				= "gamespylogo.bmp";
char *ScreenShotFileType				= "bmp";

// This is a global variable which is used track 
// whether response was received from the backend
int responseReceived = 0;

// Initialization for the SAKE API
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

// Callback function which is called to indicate content upload completed
// FileID is returned as a handle to the uploaded content
static void UploadCompletedCallback(SAKE sake, 
									SAKERequestResult result, 
									gsi_i32 fileid, 
									SAKEFileResult fileResult, 
									void *userData)
{
	// error uploading our file
	if	(result != SAKERequestResult_SUCCESS)
    {
        printf("    Upload Content failed. Result = %d \n", result);
		responseReceived = -1;
    }
    else
    {
        if(fileid != 0) // upload completed successfully (and we have a valid file id)
        {
			printf("    Success - uploaded File ID %d \n", fileid);
			
			// We received a file id from the backend - we use the userData for saving the fileId
			*(int *)userData = fileid;

			// Indicate to the sample's think function that response has been received and completed
			responseReceived = 1;
        }
        else // something is wrong with our file
        {
            printf("    Update of file has failed! file id is 0, file result %d\n", fileResult);
			responseReceived = -1;
        }
    }

    GSI_UNUSED(sake); // (alleviates compile warnings)
}

// upload a local file using Sake, and return the file id upon success (0 otherwise)
static int SampleUploadScreenShot()
{
	// initialize UploadContent variables
	SAKEStartRequestResult	result			= SAKEStartRequestResult_SUCCESS;
	int						uploadedFileId	= 0;
	SAKEUploadContentInput uploadContentInfo;
	SAKEContentInfo info;
    
	// initialize the content info, specifying the file path and that it's from disk instead of from memory
	info.mFileid		= 0;
	info.mStorage.mFile = ScreenShotFilenameWithPath;
	info.mType			= SAKEContentStorageType_DISK;

	// initialize the request input, specifying the name to store the file as, and non-blocking
	uploadContentInfo.content = &info;
	uploadContentInfo.progressCallback = NULL;
	uploadContentInfo.remoteFileName = ScreenShotFileName;
	uploadContentInfo.transferBlocking = gsi_false;

	// use sakeUploadContent to save the file on the Sake Storage Server
	printf("  Uploading the screenshot file...\n");
	result = sakeUploadContent( sake, &uploadContentInfo, UploadCompletedCallback, &uploadedFileId);

	if(result != SAKERequestResult_SUCCESS) // local issue prior to the request actually going out
	{
	    printf("  Failed to start Sake request\n");
		return gsi_false;
	}

	// keep 'thinking' while we wait for the upload uploadCompleted callback
	responseReceived = 0;
	while (responseReceived == 0)
    {
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
        msleep(10); 
		sakeThink();
    }

	if (responseReceived < 0) // we set responseReceived to -1 if the callback returns an error
		return 0; 

	// return the fileId which is created by SAKE. 
	return uploadedFileId;
}

// Callback to indicate the MetaData create record request has been completed.
static void StoreMetaDataCallback(	SAKE				sake, 
									SAKERequest			request, 
									SAKERequestResult	result, 
									void				*inputData, 
									void				*outputData, 
									void				*userData
								)
{
    newRecordId = 0; // Initialize the new record id
	if (result != SAKERequestResult_SUCCESS)
	{
		printf("    Store Content Meta Data failure. Result = %d \n", result);
		responseReceived = -1;
	}
	else
	{
		SAKECreateRecordOutput *output = (SAKECreateRecordOutput *) outputData;
		newRecordId = output->mRecordId; // remember the meta data record id. 
		printf("    Stored Content Meta Data successfully. Record Id %d\n", output->mRecordId);
		responseReceived = 1;
	}

	GSI_UNUSED(userData);
	GSI_UNUSED(inputData);
	GSI_UNUSED(request);
	GSI_UNUSED(sake);
}

// with each uploaded file it is necessary create metadata via Sake to keep track
static gsi_bool SampleStoreScreenshotMetaData()
{
	// setup parameters for sakeCreateRecord
	SAKERequest				request;
	SAKECreateRecordInput metaDataRecordInput;
	int			fieldIndex = 0;
	// initalize the fields with field name and type; note that we use int type for FileIDs, but 
	// you use the FileID type when you create the field for your Sake table
	SAKEField	fields[4] = {	"userName", SAKEFieldType_ASCII_STRING, (char)"",	
								"myfile",	SAKEFieldType_INT,			0,
								"fileName", SAKEFieldType_ASCII_STRING,	(char)"",
								"fileType", SAKEFieldType_ASCII_STRING,	 (char)""};

	// Initialize the field values
	fields[fieldIndex++].mValue.mAsciiString = SAMPLE_UNIQUENICK[0];	// unique nick
	fields[fieldIndex++].mValue.mInt		 = newFileId;				// uploaded file id
	fields[fieldIndex++].mValue.mAsciiString = ScreenShotFileName;		// uploaded file name
	fields[fieldIndex++].mValue.mAsciiString = ScreenShotFileType;		// uploaded file type

	// set the input data for the record creation
	metaDataRecordInput.mTableId	= ScreenShotTable;					// Table name
	metaDataRecordInput.mNumFields	= fieldIndex;						// Number of fields per record
	metaDataRecordInput.mFields		= fields;							// The fields in the record.

	// now create a Sake record to store our new file data
	printf("  Create a Sake record with metadata for our screenshot...\n");
	request = sakeCreateRecord(sake, &metaDataRecordInput, StoreMetaDataCallback, NULL);

	if(!request) // local issue prior to the request actually going out
	{
		// gets the result of the most recent Sake call attempted
		SAKEStartRequestResult startRequestResult = sakeGetStartRequestResult(sake);
	    printf("  Failed to start Sake request: %d", startRequestResult);
		return gsi_false;
	}

	// keep 'thinking' while we wait for the createRecord callback 
	responseReceived = 0;
	while (responseReceived == 0)
	{
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
		msleep(10); 
		sakeThink();
	}

	if (responseReceived < 0) // we set responseReceived to -1 if the callback returns an error
		return gsi_false; 

	return gsi_true;
}

// Callback to indicate the response for get my records has been received
static void GetMyRecordsCallback(	SAKE				sake, 
									SAKERequest			request, 
									SAKERequestResult	result, 
									void				*inputData, 
									void				*outputData, 
									void				*userData
								)
{
	// error retrieving our record(s)
	if (result != SAKERequestResult_SUCCESS) 
	{
		printf("    Get metadata failure. Result = %d \n", result);
		responseReceived = -1;
	}
	else // success
	{
		// copy the record data into the userData
		SAKEGetMyRecordsInput  *input  = (SAKEGetMyRecordsInput  *) inputData;
		SAKEGetMyRecordsOutput *output = (SAKEGetMyRecordsOutput *) outputData;
		*(SAKEGetMyRecordsOutput *) userData = *(SAKEGetMyRecordsOutput *) CopySakeRecordsFromResponseData(input,output);
		
		// print the metadata record info
		printf("    Metadata retrieved:\n");
		DisplaySakeRecords(output->mRecords , output->mNumRecords, input->mNumFields);

		responseReceived = 1;
	}

	GSI_UNUSED(sake);
	GSI_UNUSED(request);
}

// Progress callback for downloading content from SAKE
static void DownloadProgressCallback(SAKE sake, 
                                    gsi_u32 bytesTransfered, 
                                    gsi_u32 totalSize, 
                                    void *userData)
{
    printf("    Downloaded %d out of %d bytes....\n", bytesTransfered, totalSize);
    GSI_UNUSED(sake);
    GSI_UNUSED(userData);
}

// This callback is invoked when download completes 
static void DownloadCompletedCallback(	SAKE     sake, 
										SAKERequestResult result,
										SAKEFileResult fileResult, 
										char	 *buffer, 
										gsi_i32  bufferLength, 
										void     *userData)
{


    if(result != SAKERequestResult_SUCCESS)
	{
        printf("    Download failed, request result = %d, file result = %d\n", result, fileResult);
		responseReceived = -1;
	}
    else
	{
        printf("    Successfully downloaded file!!\n");
		responseReceived = 1;
	}
    
	GSI_UNUSED(userData);
    GSI_UNUSED(sake);
}

// use sakeGetMyRecords to return our screenshot metadata record (in order to get the fileID so we can
// then download the screenshot file)
static gsi_bool SampleRetrieveScreenshotMetaData(SAKEGetMyRecordsOutput * myRecords)
{
	// setup parameters for getMyRecords call
	SAKERequest request;
	char *fields[3] = {"recordid", "myfile", "fileName"};
	SAKEGetMyRecordsInput myRecordsInput;
	myRecordsInput.mTableId = ScreenShotTable;
	myRecordsInput.mFieldNames = fields;
	myRecordsInput.mNumFields  = 3;
	
	// retrieve our screenshot metadata, which includes the FileID; we'll copy the record(s) into the 
	// userData so that we can use this metadata info for downloading the file(s)
	printf("  Getting our Sake record which contains screenshot metadata...\n");
    request = sakeGetMyRecords(sake, &myRecordsInput, GetMyRecordsCallback, myRecords);
	
	if(!request) // local issue prior to the request actually going out
	{
		// gets the result of the most recent Sake call attempted
		SAKEStartRequestResult startRequestResult = sakeGetStartRequestResult(sake);
	    printf("  Failed to start Sake request: %d", startRequestResult);
		return gsi_false;
	}

	// keep 'thinking' while we wait for the getMyRecords callback
	responseReceived = 0;
	while (responseReceived == 0)
	{
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
		msleep(10); 
		sakeThink();
	}

	if (responseReceived < 0) // we set responseReceived to -1 if the callback returns an error
		return gsi_false; 

	return gsi_true;
}

// download our screenshot, using the fileID and metadata in myRecords
// note that we only maintain one record per owner id for this sample (otherwise we would loop 
// through all records)
static gsi_bool SampleDownloadScreenshot(SAKEGetMyRecordsOutput myRecords)
{
	// sakeDownloadContent parameters
	SAKERequestResult result;
	SAKEDownloadContentInput downloadContent;
	SAKEContentInfo content;
		
	// set the file specific data, using our metadata record
	content.mFileid = myRecords.mRecords[0][1].mValue.mInt;
	content.mStorage.mFile =  myRecords.mRecords[0][2].mValue.mAsciiString;
	content.mType = SAKEContentStorageType_DISK; // store file on disk rather than memory
    
	// we'll use the progress callback to show a progress percentage
	downloadContent.progressCallback = DownloadProgressCallback;
	downloadContent.transferBlocking = gsi_false; // non-blocking (asynchronous)
	downloadContent.content = &content;

	// alas, let's download our file
	printf("  Downloading screenshot file...\n");
	result = sakeDownloadContent(sake, &downloadContent, DownloadCompletedCallback, NULL);

	if(result != SAKERequestResult_SUCCESS) // local issue prior to the request actually going out
	{
	    printf("  Failed to start Sake request\n");
		return gsi_false;
	}

	// keep 'thinking' while we wait for the downloadCompleted callback
	responseReceived = 0;
	while (responseReceived == 0)
	{
		// normally you would be doing other game stuff here; we just sleep because we have nothing 
		// better to do
		msleep(10); 
		sakeThink();
	}

	if (responseReceived < 0) // we set responseReceived to -1 if the callback returns an error
		return gsi_false;

	return gsi_true;
}

// saving a screenshot (or any file) consists of 2 steps - uploading the file itself, then creating 
// a Sake record for associated data (most importantly the 'fileID' received upon uploading)
static gsi_bool SampleSaveScreenshot()
{
	// upload our screenshot, making sure to keep track of the file id
	newFileId = SampleUploadScreenShot();
	if (newFileId == 0) // error uploading file
		return gsi_false;

	// create a Sake record to store data associated with the uploaded file
	if (!SampleStoreScreenshotMetaData())
		return gsi_false;

	return gsi_true;
}

// assuming we don't already know the fileID we want, retrieving a user's screenshot (or any file) 
// also consists of 2 steps - retrieving the player's file metadata record, then downloading the file
// based on the fileID in the metadata
static gsi_bool SampleRetrieveScreenShot( )
{
	SAKEGetMyRecordsOutput myRecords;
	
	// get our screenshot metadata so we have the info needed to download our screenshot
	if (!SampleRetrieveScreenshotMetaData(&myRecords))
		return gsi_false;

	// check whether there are any meta data records obtained
	if (myRecords.mNumRecords > 0 && myRecords.mRecords[0] != NULL)
	{
		// we use this global variable to track the recordID so we can delete it later during cleanup
		newRecordId = myRecords.mRecords[0][0].mValue.mInt;

		// download the screenshot
		if (!SampleDownloadScreenshot(myRecords))
			return gsi_false;
	}

	// Free the allocated memory (from the getMyRecords callback) for the meta data 
	FreeSakeRecords(&myRecords, 3);

	return gsi_true;
}

static gsi_bool Cleanup()
{
	// for the sake of keeping the sample table clean we'll delete the saved record 
	// that we created.
	printf("  Deleting our file meta data record...\n");
	if (!DeleteSakeRecord(ScreenShotTable, newRecordId))
		return gsi_false;

	// shutdown Sake and Core (which ensures internal objects get freed)
	SakeCleanup();

	// keep the command line prompt open so you can view the output
	WaitForUserInput();

	return gsi_true;
}

int test_main()
{	
	// first do the prerequisite initialization and such so we'll be ready to use Sake to save/retrieve
	// a screenshot
    printf("Doing the prerequisite setup and initialization\n");
	if (!Setup())
		return -1;

	// the beef of the sample app - you can also reference this sample code on the developer portal 'Docs'
	printf("Saving our screenshot\n");
	if (!SampleSaveScreenshot())
		return -1;

	printf("Retrieving our screenshot\n");
	if (!SampleRetrieveScreenShot())
		return -1;

	// shutdown the Core object, then we'll wait for user input before killing the app
	printf("Cleaning up after ourselves\n");
	if (!Cleanup())
		return -1;
    
	return 0;
}
