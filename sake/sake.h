///////////////////////////////////////////////////////////////////////////////
// File:	sake.h
// SDK: 	GameSpy Sake Persistent Storage SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SAKE_H__
#define __SAKE_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../common/gsCommon.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef SAKE_CALL
#define SAKE_CALL
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Storage for the Sake web-service URL
#define SAKE_MAX_URL_LENGTH 128
extern char sakeiSoapUrl[SAKE_MAX_URL_LENGTH];

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// General
typedef struct SAKEInternal *SAKE;
//////////////////////////////////////////////////////////////
// SAKEStartupResult
// Summary
//		Value returned from the call to sakeStartup.
typedef enum
{
	SAKEStartupResult_SUCCESS,			// Startup succeeded.
	SAKEStartupResult_NOT_AVAILABLE,	// The Sake service is unavailable.
	SAKEStartupResult_CORE_SHUTDOWN,	// Error in the gsCore.
	SAKEStartupResult_OUT_OF_MEMORY		// Not enough memory to initialize Sake.
} SAKEStartupResult;

//////////////////////////////////////////////////////////////
// sakeStartup
// Summary
//		Initializes the Sake SDK for use.
// Parameters
//		sakePtr	: [in] Pointer to a Sake object, which is initialized by startup. 
//					This will be used in nearly all subsequent Sake calls.
// Returns
//		An enumeration of possible results.  If the result is
//		 SAKEStartupResult_SUCCESS, 
//		then the startup has succeeded.  Any other value indicates a
//		 failure, and the 
//		game should not continue calling other Sake functions.
// Remarks
//		Before using Sake, the GameSpy Availability Check must have been
//		 performed and indicated 
//		that the game's backend is available, and the Core object must
//		 have been initialized 
//		by calling gsCoreInitialize. Sample code for this is available
//		 in the Sake test app.<p>
//		The SAKE object initialized by this startup call is valid until the game 
//		shutdowns the Sake SDK with sakeShutdown.
// See Also
//		SAKEStartupResult
SAKEStartupResult SAKE_CALL sakeStartup(SAKE *sakePtr);

//////////////////////////////////////////////////////////////
// sakeShutdown
// Summary
//		Shuts down the SDK and frees any memory that was allocated for
//		 the Sake object.
// Parameters
//		sake	: [in] The sake object.
// Remarks
//		After this function returns, the reference to the Sake object is
//		 no longer valid and should not be used. 
//		The game should also shutdown the GameSpy Core object by calling
//		 gsCoreShutdown.
//		Sample code for this is available in the Sake test app.<p>
void              SAKE_CALL sakeShutdown(SAKE sake);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Authentication

//////////////////////////////////////////////////////////////
// sakeSetGame
// Summary
//		Authenticates the game to use Sake.
// Parameters
//		sake		: [in] The Sake object.
//		gameName	: [in] Your title's gamename, assigned by GameSpy.
//		gameId		: [in] Your title's gameid, assigned by GameSpy.
//		secretKey	: [in] Your title's secret key , assigned by GameSpy.
// Remarks
//		The function provides no indication of whether or not the
//		 gamename and gameid are correct. 
//		The backend will simply check them and use the information to
//		 figure out which game's 
//		database is being used.<p>
//		The game should also call sakeSetProfile to authenticate the
//		 current player before continuing 
//		with any other Sake usage.
// See Also
//		sakeSetProfile
void SAKE_CALL sakeSetGame(SAKE sake, const gsi_char *gameName, int gameId, const gsi_char *secretKey);

//////////////////////////////////////////////////////////////
// sakeSetProfile
// Summary
//		Provides Sake authentication information for the current player.
// Parameters
//		sake		: [in] The sake object.
//		profileId	: [in] Current player's profile ID.
//		loginTicket	: [in] Current player's login ticket, which
//		 allows the backend to verify the player 
//							is correctly identifying himself.
// Remarks
//		The profile ID and login ticket are both obtained from the
//		 GameSpy Presence and Messaging SDK (GP).<p>
//		The profile ID can be obtained in the callback that is called as
//		 a result of logging into GP. 
//		A GPConnectResponseArg struct is passed to the callback, and the
//		 struct has a member variable 
//		"profile" that stores the player's profile ID. While the player
//		 is logged on, the game should call 
//		the GP function gpGetLoginTicket.<p>
//		As with sakeSetGame, sakeSetProfile provides no indication of
//		 whether or not the information provided 
//		is correct. The backend checks these values and uses them to
//		 authenticate the player and, 
//		for certain requests, identify which player's data is being
//		 access or updated.
// See Also
//		Presence\GPConnectResponseArg, Presence\gpGetLoginTicket
void SAKE_CALL sakeSetProfile(SAKE sake, int profileId, const char *loginTicket);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Fields

//////////////////////////////////////////////////////////////
// SAKEFieldType
// Summary
//		Indicates the type of data stored in a field.
// Remarks
//		The value for a field is stored in a SAKEValue union.<p>
// See Also
//		SAKEField, SAKEBinaryData
typedef enum
{
	SAKEFieldType_BYTE,
	SAKEFieldType_SHORT,
	SAKEFieldType_INT,
	SAKEFieldType_FLOAT,
	SAKEFieldType_ASCII_STRING,
	SAKEFieldType_UNICODE_STRING,
	SAKEFieldType_BOOLEAN,
	SAKEFieldType_DATE_AND_TIME,
	SAKEFieldType_BINARY_DATA,
	SAKEFieldType_INT64,
	SAKEFieldType_NUM_FIELD_TYPES
} SAKEFieldType;

//////////////////////////////////////////////////////////////
// SAKEBinaryData
// Summary
//		Data struct used to store arbitrary binary data in a Sake field.
// Remarks
//		mValue may be NULL if mLength is 0.<p>
// See Also
//		SAKEFieldType
typedef struct
{
	gsi_u8 *mValue;		// pointer to the data.
	int     mLength;	// the number of bytes of data.
} SAKEBinaryData;

typedef union
{
	gsi_u8          mByte;
	gsi_i16         mShort;
	gsi_i32         mInt;
	float           mFloat;
	char           *mAsciiString;
	gsi_u16        *mUnicodeString;
	gsi_bool        mBoolean;
	time_t          mDateAndTime;
	SAKEBinaryData  mBinaryData;
	gsi_i64         mInt64;
} SAKEValue;

//////////////////////////////////////////////////////////////
// SAKEField
// Summary
//		object used to represent the field of a record.
// See Also
//		SAKEFieldType, SAKEBinaryData
typedef struct
{
	char         *mName;	// the name used to identify the field.
	SAKEFieldType mType;	// The type of data stored in the field.
	SAKEValue     mValue;	// The value that will be stored in the field.
} SAKEField;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Requests
typedef struct SAKERequestInternal *SAKERequest;

//////////////////////////////////////////////////////////////
// SAKEStartRequestResult
// Summary
//		The status result of the most recent request.
// See Also
//		sakeGetStartRequestResult
typedef enum
{
	SAKEStartRequestResult_SUCCESS,
	SAKEStartRequestResult_NOT_AUTHENTICATED,
	SAKEStartRequestResult_OUT_OF_MEMORY,
	SAKEStartRequestResult_BAD_INPUT,
	SAKEStartRequestResult_BAD_TABLEID,
	SAKEStartRequestResult_BAD_FIELDS,
	SAKEStartRequestResult_BAD_NUM_FIELDS,
	SAKEStartRequestResult_BAD_FIELD_NAME,
	SAKEStartRequestResult_BAD_FIELD_TYPE,
	SAKEStartRequestResult_BAD_FIELD_VALUE,
	SAKEStartRequestResult_BAD_OFFSET,
	SAKEStartRequestResult_BAD_MAX,
	SAKEStartRequestResult_BAD_RECORDIDS,
	SAKEStartRequestResult_BAD_NUM_RECORDIDS,
	SAKEStartRequestResult_UNKNOWN_ERROR
} SAKEStartRequestResult;

//////////////////////////////////////////////////////////////
// SAKERequestResult
// Summary
//		The result of Sake calls used to modify or read from records
//		 (returned to the SAKERequestCallback).
// See Also
//		SAKERequestCallback
typedef enum
{
	SAKERequestResult_SUCCESS,					// Sake request completed successfully.
	SAKERequestResult_SECRET_KEY_INVALID,		// The secretKey passed to sakeSetGame is invalid (sakeSetGame does 
	//												not actually authenticate the game info).
	SAKERequestResult_SERVICE_DISABLED,			// GameSpy services have been disabled for this title.
	SAKERequestResult_CONNECTION_TIMEOUT,		// A connection to the Sake backend could not be established before 
	//												the timeout was reached.
	SAKERequestResult_CONNECTION_ERROR,			// An error occurred while connecting to the Sake backend.
	SAKERequestResult_MALFORMED_RESPONSE,		// The SOAP response sent from the backend has been corrupted.
	SAKERequestResult_OUT_OF_MEMORY,			// A memory allocation failed.
	SAKERequestResult_DATABASE_UNAVAILABLE,		// The Sake database is temporarily down.
	SAKERequestResult_LOGIN_TICKET_INVALID,		// The profile's loginTicket, obtained via gpGetLoginTicket and 
	//												passed to sakeSetProfile, is invalid (sakeSetProfile does not 
	//												actually authenticate the player info).
	SAKERequestResult_LOGIN_TICKET_EXPIRED,		// The profile's loginTicket, obtained via gpGetLoginTicket and 
	//												passed to sakeSetProfile, has expired. The profile must obtain 
	//												an updated login ticket (via gpGetLoginTicket) and call sakeSetProfile 
	//												once again with the updated ticket.
	SAKERequestResult_TABLE_NOT_FOUND,			// No Sake table matches the mTableId passed in the Sake query.
	SAKERequestResult_RECORD_NOT_FOUND,			// No records in the specified table match the mRecordId passed in 
	//												the Sake query.
	SAKERequestResult_FIELD_NOT_FOUND,			// mFieldNames or mFields contains a field which 
	//												is not in the specified table.
	SAKERequestResult_FIELD_TYPE_INVALID,		// The mType set in a SAKEField struct is not a valid SAKEFieldType.
	SAKERequestResult_NO_PERMISSION,			// The profile is not the owner of the record and the record does not 
	//												have public access (permissions are set on the Sake admin site).
	SAKERequestResult_RECORD_LIMIT_REACHED,		// The profile cannot create any more records in the table specified 
	//												(Limit per owner is set on the Sake admin site).
	SAKERequestResult_ALREADY_RATED,			// No profile can rate a single record more than once.
	SAKERequestResult_NOT_RATEABLE,				// The record is in a table that is not rateable (The ‘Rateable' flag
	//												for a table is set on the Sake admin site).
	SAKERequestResult_NOT_OWNED,				// Only the record owner profile can perform the attempted function 
	//												on the specified record.
	SAKERequestResult_FILTER_INVALID,			// The mFilter string has invalid SQL grammar.
	SAKERequestResult_SORT_INVALID,				// The mSort string has invalid SQL grammar.
	SAKERequestResult_TARGET_FILTER_INVALID,	// Error is unknown used if none of the above.
	SAKERequestResult_UNKNOWN_ERROR
} SAKERequestResult;

//////////////////////////////////////////////////////////////
// SAKEFileResult
// Summary
//		Used to determine the status of a file uploaded to Sake.	
// See Also
//		sakeGetFileResultFromHeaders
typedef enum
{
	SAKEFileResult_SUCCESS           = 0,	// Upload succeeded.
	SAKEFileResult_BAD_HTTP_METHOD   = 1,	// Incorrect ghttp call used to upload file.
	SAKEFileResult_BAD_FILE_COUNT    = 2,	// Number of files uploaded is incorrect.
	SAKEFileResult_MISSING_PARAMETER = 3,	// Missing parameter in the ghttp upload call.
	SAKEFileResult_FILE_NOT_FOUND    = 4,	// No file was found.
	SAKEFileResult_FILE_TOO_LARGE    = 5,	// File uploaded larger than the specified size.
	SAKEFileResult_SERVER_ERROR      = 6,	// Unknown error occurred on the server when processing this request.
	SAKEFileResult_UNKNOWN_ERROR			// Error is unknown - used if none of the above.
} SAKEFileResult;


////////////////////////////////////////////////
// SAKERequestCallback
// Summary
//		This typedef defines the general callback function.
// Parameters
//      sake        : [in] The sake instance the callback is running on.
//		request	    : [in] The request data containing everything related.
//		inputData	: [in] Pointer for the input data to API call.
//		outputData  : [in] Pointer for the input data from API callback function.
//		userData	: [in/out] The developer specified void pointer data shared 
//                         between the API and its callback. 
// Remarks
//		Not all request types have output objects. If a request type
//		 does not have an output object, 
//		then outputData will be always be NULL when the callback is called.<p>
// See Also
//		SAKERequestResult
typedef void (*SAKERequestCallback)(SAKE sake, 
	SAKERequest       request, 
	SAKERequestResult result, 
	void *inputData, 
	void *outputData, 
	void *userData);

//////////////////////////////////////////////////////////////
// sakeGetStartRequestResult
// Summary
//		Called to retrieve the result of a request - normally used to
//		 determine the reason for a failed request.
// Parameters
//		sake	: [in] The sake object
// Returns
//		Enum value used to indicate the specific result of the request.
// Remarks
//		This function will always return the most recent request that
//		 was attempted, so it must 
//		be called immediately after a failure to get the reason for that
//		 failure.<p>
// See Also
//		SAKEStartRequestResult
SAKEStartRequestResult SAKE_CALL sakeGetStartRequestResult(SAKE sake);

//////////////////////////////////////////////////////////////
// SAKECreateRecordInput
// Summary
//		Input object passed to sakeCreateRecord.
// Remarks
//		Any fields which are not initialized will be set to their default value.
//		If mNumFields is 0, indicating that no initial values will be
//		 set, then mFields can be NULL.<p>
// See Also
//		sakeCreateRecord
typedef struct
{
	char      *mTableId;	// Points to the tableid of the table in which the record will be created.
	SAKEField *mFields;		// Points to an array of fields which has the initial values for the new record's fields.
	int        mNumFields;	// Stores the number of fields in the mFields array.
} SAKECreateRecordInput;

//////////////////////////////////////////////////////////////
// SAKECreateRecordOutput
// Summary
//		Returned output object that specifies the recordid for the newly
//		 created record.
// See Also
//		sakeCreateRecord, SAKERequestCallback
typedef struct
{
	int mRecordId;	// The recordid for the newly created record.
} SAKECreateRecordOutput;

//////////////////////////////////////////////////////////////
// sakeCreateRecord
// Summary
//		Creates a new Record in a backend table.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the data for the record you wish to create.
//		callback	: [in] The request callback function.
//		userdata	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request 
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the output object
//		 contains the recordid of the newly created record.<p>
// See Also
//		SAKECreateRecordInput, SAKECreateRecordOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeCreateRecord( SAKE sake, SAKECreateRecordInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEUpdateRecordInput
// Summary
//		Input object passed to sakeUpdateRecord.
// Remarks
//		Unlike with a CreateRecord request, mNumFields cannot be 0; at
//		 least one field must be updated.<p>
//		UpdateRecord does not have an output object, because the backend
//		 does not send any response other 
//		than the success or failure indicated by the result parameter
//		 passed to the callback. When the callback 
//		is called, the outputData parameter will always be set to NULL.
// See Also
//		sakeUpdateRecord
typedef struct
{
	char      *mTableId;	// Points to the tableid of the table in which the record to be updated exists.
	int        mRecordId;	// Identifies the record to be updated.
	SAKEField *mFields;		// Points to an array of fields which has the new values for the record's fields.
	int        mNumFields;	// Stores the number of fields in the mFields array.
} SAKEUpdateRecordInput;

// sakeUpdateRecord
// Summary
//		Updates the values stored in an existing record.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the updated data for the record. 
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//      Reference to internal object that tracks the request. If this is NULL, then the request has failed 
//      to initialize. You can call sakeGetStartRequestResult to obtain the reason for the failure.
// Remarks
//      UpdateRecord does not have an output object, because the backend does not send any response other 
//      than the success or failure indicated by the result parameter passed to the callback. When the callback 
//      is called, the outputData parameter will always be set to NULL.
// See Also
//		SAKEUpdateRecordInput, SAKERequestCallback
SAKERequest SAKE_CALL sakeUpdateRecord(SAKE sake, SAKEUpdateRecordInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEDeleteRecordInput
// Summary
//		Input object passed to sakeDeleteRecord.
// Remarks
//		DeleteRecord does not have an output object, because the backend
//		 does not send any response other than the success or failure
//		 indicated by the result parameter passed to the callback.
//		When the callback is called, the outputData parameter will
//		 always be set to NULL.<p>
// See Also
//		sakeDeleteRecord
typedef struct
{
	char *mTableId;		// Points to the tableid of the table in which the record to be deleted exists.
	int   mRecordId;	// Identifies the record to be deleted.
} SAKEDeleteRecordInput;

//////////////////////////////////////////////////////////////
// sakeDeleteRecord
// Summary
//		Deletes the specified record.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the info about the record you wish to delete. 
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request 
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//      DeleteRecord does not have an output object, because the backend does not send any 
//      response other than the success or failure indicated by the result parameter passed 
//      to the callback. When the callback is called, the outputData parameter will always be set to NULL.
// See Also
//		SAKEDeleteRecordInput, SAKERequestCallback
SAKERequest SAKE_CALL sakeDeleteRecord(SAKE sake, SAKEDeleteRecordInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKESearchForRecordsInput
// Summary
//		Input object passed to sakeSearchForRecords.
// Members/Constants
//		mTableId	
//		mFieldNames	
//		mNumFields	
//		mFilter	
//		mSort	
//		mOffset	
//		mMaxRecords	
//		mTargetRecordFilter	
//		mSurroundingRecordsCount	
//		mOwnerIds	
//		mNumOwnerIds	
//		mCacheFlag	
// Remarks
//		In addition to the fields which the developer defines, you can
//		 also request values for the "recordid" field, "ownerid" field (if
//		 the table has an owner type of profile), and "num_ratings" and
//		 "average_rating" fields (if the table has its ratings option set to
//		 true).<p>
// See Also
//		sakeSearchForRecords
typedef struct
{
	char      *mTableId;					// Points to the tableid of the table to be searched.
	char     **mFieldNames;					// Points to an array of strings, each of which contains the name of 
	//											a field for which to return values. This list controls the values 
	//											which will be returned as part of the response.  The array can 
	//											contain just one field name, the names of all the fields in the table, 
	//											or any subset of the field names.
	int        mNumFields;					// Stores the number of strings in the mFieldNames array.
	gsi_char  *mFilter;						// SQL-like filter string which is used to search for records based on the 
	//											values in their fields. For example, to find everyone who has a score of 
	//											more than 50 use "score > 50", or to find everyone who has a name that 
	//											starts with an A use "name like A%".  Note that a field can be used 
	//											in the filter string even if it is not listed in the mFieldNames array, 
	//											and that file metadata fields can be used in a filter string.
	char      *mSort;						// SQL-like sort string which is used to sort the records which are found by 
	//											the search. To sort the results on a particular field, just pass in the 
	//											name of that field, and the results will be sorted from lowest to highest 
	//											based on that field. To make the sort descending instead of ascending 
	//											add " desc" after the name of the field. Note that a field can be used 
	//											in the sort string even if it is not listed in the mFieldNames array, 
	//											and that file metadata fields can be used in a sort.
	int        mOffset;						// If not set to 0, then the backend will return records starting from the 
	//											given offset into the result set.
	int        mMaxRecords;					// Used to specify the maximum number of records to return for 
	//											a particular search.
	gsi_char  *mTargetRecordFilter;			// Used to specify a single record to return - when done in conjunction 
	//											with mSurroundingRecordsCount, this will return the "target" record plus 
	//											the surrounding records above and below this target record. Can also 
	//											be used to specify a "set" of target records to return, but when used 
	//											in this context the surrounding records count does not apply.
	int        mSurroundingRecordsCount;	// Used in conjunction with mTargetRecordFilter - specifies the number 
	//											of records to return above and below the target record. (e.g. if = 5, 
	//											you will receive a maximum of 11 possible records, the target record + 5 
	//											above and 5 below).
	int       *mOwnerIds;					// Specifies an array of ownerIds (profileid of record owner) to return 
	//											from the search.
	int		   mNumOwnerIds;				// Specifies the number of ids contained in the mOwnerIds array.
	gsi_bool   mCacheFlag;					// Enables caching if set to gsi_true. Defaults to no caching if none 
	//											is specified.
} SAKESearchForRecordsInput;

//////////////////////////////////////////////////////////////
// SAKESearchForRecordsOutput
// Summary
//		Returned output object that contains the records founds by the search.
// See Also
//		sakeSearchForRecords, SAKERequestCallback, SAKEField
typedef struct
{
	int         mNumRecords;	// The number of records found.
	SAKEField **mRecords;		// Points an array of records, each of  which is represented as an array of fields.
} SAKESearchForRecordsOutput;

//////////////////////////////////////////////////////////////
// sakeSearchForRecords
// Summary
//		Searches a table for records that match certain specified criteria.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the info about the records you wish to search for.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request 
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the output object
//		 contains contains records founds 
//		by the search. See definitions of the Input & Output structs for
//		 more information about how to 
//		limit what is retrieved in the request and certain metadeta
//		 fields that can be retrieved.<p>
// See Also
//		SAKESearchForRecordsInput, SAKESearchForRecordsOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeSearchForRecords(SAKE sake, SAKESearchForRecordsInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEGetMyRecordsInput
// Summary
//		Input object passed to sakeGetMyRecords.
// Remarks
//		In addition to the fields which the developer defines, you can
//		 also request values for the "recordid" field, "ownerid" field (if
//		 the table has an owner type of profile), and "num_ratings" and
//		 "average_rating" fields (if the table has its ratings option set to
//		 true).<p>
// See Also
//		sakeGetMyRecords
typedef struct
{
	char      *mTableId;	// Points to the tableid of the table from which to return records.
	char     **mFieldNames;	// Points to an array of strings, each of which contains the name of a
	//							field for which to return values.
	int        mNumFields;	// Stores the number of strings in the mFieldNames array.  This list controls 
	//							the values which will be returned as part of the response.  The array can contain 
	//							just one field name, the names of all the fields in the table, or any subset 
	//							of the field names.
} SAKEGetMyRecordsInput;

//////////////////////////////////////////////////////////////
// SAKEGetMyRecordsOutput
// Summary
//		Returned output object that specifies all of the records which
//		 the local player owns in the table.	
// See Also
//		sakeGetMyRecords, SAKERequestCallback, SAKEField
typedef struct
{
	int         mNumRecords;	// The number of records found.
	SAKEField **mRecords;		// Points an array of records, each of  which is represented as an array of fields.
} SAKEGetMyRecordsOutput;

//////////////////////////////////////////////////////////////
// sakeGetMyRecords
// Summary
//		Gets all of the records owned by the local player from a table.
// Parameters
//		sake		: [in] The Sake object.
//		input		: [in] Stores info about the records you wish to retrieve.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the 
//		request has failed to initialize. You can call
//		 sakeGetStartRequestResult to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the output object
//		 contains all of the records which 
//		the local player owns in the table. See definitions of the Input
//		 & Output structs for more information 
//		about how to limit what is retrieved in the request and certain
//		 metadeta fields that can be retrieved.<p>
// See Also
//		SAKEGetMyRecordsInput, SAKEGetMyRecordsOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeGetMyRecords(SAKE sake, SAKEGetMyRecordsInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEGetSpecificRecordsInput
// Summary
//		Input object passed to sakeGetSpecificRecords.
// Remarks
//		In addition to the fields which the developer defines, you can
//		 also request values for the "recordid" field, "ownerid" field (if
//		 the table has an owner type of profile), and "num_ratings" and
//		 "average_rating" fields (if the table has its ratings option set to
//		 true).<p>
// See Also
//		sakeGetSpecificRecords
typedef struct
{
	char      *mTableId;		// Points to the tableid of the table from which to get the records.
	int       *mRecordIds;		// An array of recordids, each one identifying a record which is to be returned.
	int        mNumRecordIds;	// The number of recordids in the mRecordIds array.
	char     **mFieldNames;		// Points to an array of strings, each of which contains the name of a field for which 
	//								to return values.
	int        mNumFields;		// stores the number of strings in the mFieldNames array.  This list controls 
	//								the values which will be returned as part of the response.  The array can 
	//								contain just one field name, the names of all the fields in the table, or 
	//								any subset of the field names.
} SAKEGetSpecificRecordsInput;

//////////////////////////////////////////////////////////////
// SAKEGetSpecificRecordsOutput
// Summary
//		Returned output object that contains all of the records which
//		 were specified in the request.
// See Also
//		sakeGetSpecificRecords, SAKERequestCallback, SAKEField
typedef struct
{
	int         mNumRecords;	// The number of records found.
	SAKEField **mRecords;		// Points an array of records, each of  which is represented as an array of fields.
} SAKEGetSpecificRecordsOutput;

//////////////////////////////////////////////////////////////
// sakeGetSpecificRecords
// Summary
//		Gets a list of specific records from a table.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the info about the specific records
//		 you wish to retrieve.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request 
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the output object
//		 contains all of the records 
//		which were specified in the request. See definitions of the
//		 Input & Output structs for more 
//		information about how to limit what is retrieved in the request
//		 and certain metadeta fields that can be retrieved.<p>
// See Also
//		SAKEGetSpecificRecordsInput, SAKEGetSpecificRecordsOutput,
//		 SAKERequestCallback
SAKERequest SAKE_CALL sakeGetSpecificRecords(SAKE sake, SAKEGetSpecificRecordsInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEGetRandomRecordInput
// Summary
//		Input object passed to sakeGetRandomRecord.
// Remarks
//		In addition to the fields which the developer defines, you can
//		 also request values for the "recordid" field, "ownerid" field (if
//		 the table has an owner type of profile), and "num_ratings" and
//		 "average_rating" fields (if the table has its ratings option set to
//		 true).<p>
// See Also
//		sakeGetRandomRecord
typedef struct
{
	char      *mTableId;	// Points to the tableid of the table to be searched.
	char     **mFieldNames;	// Points to an array of strings, each of which contains the name of a field for 
	//							which to return values. This list controls the values which will be returned 
	//							as part of the response.  The array can contain just one field name, the names 
	//							of all the fields in the table, or any subset of the field names.
	int        mNumFields;	// Stores the number of strings in the mFieldNames array.
	gsi_char  *mFilter;		// SQL-like filter string which is used to filter which records are to be looked at 
	//							when choosing a random record. Note that if the search criteria is too specific 
	//							and no records are found, then the output will return no random record. Note that 
	//							a field can be used in the filter string even if it is not listed in the mFieldNames 
	//							array, and that file metadata fields can be used in a filter string.
} SAKEGetRandomRecordInput;

//////////////////////////////////////////////////////////////
// SAKEGetRandomRecordOutput
// Summary
//		Returned output object that contains a random record.
// Remarks
//		If no record was found due to constrained search criteria, the
//		 returned record will be set to NULL.<p>
typedef struct
{
	SAKEField *mRecord;	// An array of fields representing the random record.
} SAKEGetRandomRecordOutput;

//////////////////////////////////////////////////////////////
// sakeGetRandomRecord
// Summary
//		Retrieves a random record from the provided search criteria.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the info for the random record search.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the 
//		request has failed to initialize. You can call
//		 sakeGetStartRequestResult to obtain the reason for the failure.
// Remarks
//		The output will always be a single record (unless no records
//		 pass the filter, in which 
//		case the output will contain NULL data for the returned record).
//		 Note that this function works best 
//		in a table in which records are not deleted or are deleted in
//		 order of oldest first (in other words, 
//		tables where recordids are contiguous).<p>
// See Also
//		SAKEGetRandomRecordInput, SAKEGetRandomRecordOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeGetRandomRecord(SAKE sake, SAKEGetRandomRecordInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKERateRecordInput
// Summary
//		Input object passed to sakeRateRecord.
// Remarks
//		The range of ratings which Sake supports is 0 to 255.
//		However a game can restrict itself to a subset of that range if it wishes.
//		For example, a game may want to use a rating of 1 to 5 (a star
//		 rating), or it may want to use a 
//		range of 0 to 100.
//		Sake allows users to rate records which they own, however no
//		 profile can rate a single record more than once. 
//		The special field "my_rating" keeps track of what the current
//		 profile's rating is for a given record; 
//		if the record has not yet been rated, my_rating = -1 by default.<p>
//		Sake stores on the backend each individual rating that has been
//		 given, which allows it to compute accurate 
//		averages and prevent repeat ratings. The field name "my_rating"
//		 can be used to obtain the current 
//		profile's rating for a given record or when searching for
//		 records. By default, my_rating = -1 for records 
//		that have not yet been rated. When browsing for records, use the
//		 special search tags @rated or @unrated 
//		in the filter string to limit the searches to either rated or
//		 unrated records.
// See Also
//		sakeRateRecord
typedef struct
{
	char  *mTableId;	// Points to the tableid of the table in which the record to be rated exists.
	int    mRecordId;	// The recordid of the record to rate.
	gsi_u8 mRating;	// The rating the user wants to give the record.
} SAKERateRecordInput;

//////////////////////////////////////////////////////////////
// SAKERateRecordOutput
// Summary
//		Returned output object that lists the new number of ratings and
//		 the new average rating for the specified record.	
// See Also
//		sakeRateRecord, SAKERequestCallback
typedef struct
{
	int mNumRatings;		// The number of ratings associated with this record.
	float mAverageRating;	// The average rating of this record.
} SAKERateRecordOutput;

//////////////////////////////////////////////////////////////
// sakeRateRecord
// Summary
//		Rates a specified record.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores the information for the record you wish to rate.
//		callback	: [in] The request callback function.
//		userdata	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request has 
//		failed to initialize. You can call sakeGetStartRequestResult to
//		 obtain the reason for the failure.
// Remarks
//		The range of ratings which Sake supports is 0 to 255.
//		However a game can restrict itself to a subset of that range if it wishes.
//		For example, a game may want to use a rating of 1 to 5 (a star
//		 rating), or it may want to use a range of 0 to 100.
//		Sake allows users to rate records which they own, however no
//		 profile can rate a single record more than once.<p>
// See Also
//		SAKERateRecordInput, SAKERequestCallback
SAKERequest SAKE_CALL sakeRateRecord(SAKE sake, SAKERateRecordInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEGetRecordLimitInput
// Summary
//		Input object passed to sakeGetRecordLimit.
// See Also
//		sakeGetRecordLimit
typedef struct
{
	char *mTableId;	// Points to the tableid of the table for which to check the limit.
} SAKEGetRecordLimitInput;

//////////////////////////////////////////////////////////////
// SAKEGetRecordLimitOutput
// Summary
//		Returned output object that specifies the maximum number of
//		 records that a profile can own in the table.
// See Also
//		sakeGetRecordLimit, SAKERequestCallback
typedef struct
{
	int mLimitPerOwner;	// Contains the maximum number of records that a profile can own in the table; 
	//						corresponds to the limit per owner option that can be set using the Sake Administration website.
	int mNumOwned;		// Contains the number of records that the local profile currently owns in the table.
} SAKEGetRecordLimitOutput;

//////////////////////////////////////////////////////////////
// sakeGetRecordLimit
// Summary
//		Checks the maximum number of records that a profile can own for
//		 a particular table.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores info on the table whose record limit
//		 you wish to check.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the Output object
//		 contains info about the record 
//		limit for the specified table.<p>
// See Also
//		SAKEGetRecordLimitInput, SAKEGetRecordLimitOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeGetRecordLimit(SAKE sake, SAKEGetRecordLimitInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// SAKEGetRecordCountInput
// Summary
//		Input object passed to sakeGetRecordCount.	
// See Also
//		sakeGetRecordCount
typedef struct
{
	char	  *mTableId;	// Points to the tableid of the table to be searched.
	gsi_char  *mFilter;		// SQL-like filter string which is used to filter which records are to be looked 
	//							at when getting the record count.
	gsi_bool   mCacheFlag;	// Enables caching if set to gsi_true. Defaults to no caching if none is specified.
} SAKEGetRecordCountInput;

//////////////////////////////////////////////////////////////
// SAKEGetRecordCountOutput
// Summary
//		Returned record count based on the specified table and search filter used.
// See Also
//		sakeGetRecordCount, SAKERequestCallback
typedef struct
{
	int	       mCount;	// Contains the value of the record count. If no records exist or the search 
	//						criteria was too specific so that no records were found, this value will be 0.
} SAKEGetRecordCountOutput;

//////////////////////////////////////////////////////////////
// sakeGetRecordCount
// Summary
//		Gets a count for the number of records in a table based on the
//		 given filter criteria.
// Parameters
//		sake		: [in] The sake object.
//		input		: [in] Stores info on the table whose count you wish to check.
//		callback	: [in] The request callback function.
//		userData	: [in] pointer to user specified data sent to the
//		 request callback.
// Returns
//		Reference to internal object that tracks the request. If this is
//		 NULL, then the request 
//		has failed to initialize. You can call sakeGetStartRequestResult
//		 to obtain the reason for the failure.
// Remarks
//		If the request completed successfully, then the Output object
//		 contains info about the count 
//		for the specified table.<p>
// See Also
//		SAKEGetRecordCountInput, SAKEGetRecordCountOutput, SAKERequestCallback
SAKERequest SAKE_CALL sakeGetRecordCount(SAKE sake, SAKEGetRecordCountInput *input, SAKERequestCallback callback, void *userData);

//////////////////////////////////////////////////////////////
// sakeGetFieldByName
// Summary
//		This utility function retrieves the specified field from the
//		 record, via the name that identifies it.
// Parameters
//		name		: [in] The name of the field to retrieve.
//		fields		: [in] An array of fields, representing a record.
//		numFields	: [in] The number of fields in the array.
// Returns
//		Pointer to a SAKEField which represents the field that was
//		 identified by the given name.
// See Also
//		SAKEField
SAKEField * SAKE_CALL sakeGetFieldByName(const char *name, SAKEField *fields, int numFields);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Files

#define SAKE_FILE_RESULT_HEADER "Sake-File-Result:"
#define SAKE_FILE_ID_HEADER "Sake-File-Id:"
#define SAKE_FILE_TYPE_HEADER "Sake-File-Type"


gsi_bool SAKE_CALL sakeSetFileDownloadURL(SAKE sake, gsi_char url[SAKE_MAX_URL_LENGTH]);
gsi_bool SAKE_CALL sakeSetFileUploadURL(SAKE sake, gsi_char url[SAKE_MAX_URL_LENGTH]);

//////////////////////////////////////////////////////////////
// sakeGetFileDownloadURL
// Summary
//		Used to get a download URL for a particular fileid.
// Parameters
//		sake		: [in] The Sake object. 
//		fileid		: [in] The fileid returned by the headers of the uploaded file. Call sakeGetFileIdFromHeaders to obtain.
//		url     	: [out] The download url for the specified file. 
// Returns
//		gsi_true if download url was retrieved successfully, gsi_false otherwise.
// See Also
//		sakeGetFileIdFromHeaders
gsi_bool SAKE_CALL sakeGetFileDownloadURL(SAKE sake, int fileId, gsi_char url[SAKE_MAX_URL_LENGTH]);

//////////////////////////////////////////////////////////////
// sakeGetFileUploadURL
// Summary
//		Retrieves the URL which can be used to upload files.
// Parameters
//		sake		: [in] The Sake object. 
//		url     	: [out] The URL where the file can be uploaded. 
// Returns
//		gsi_true if upload url was retrieved successfully, gsi_false otherwise.
gsi_bool SAKE_CALL sakeGetFileUploadURL(SAKE sake, gsi_char url[SAKE_MAX_URL_LENGTH]);

//////////////////////////////////////////////////////////////
// sakeGetFileResultFromHeaders
// Summary
//		Checks the headers from the uploaded file to see the result.
// Parameters
//		headers		: [in] The headers to parse for the fileid. Can obtain these by calling ghttpGetHeaders. 
//		result		: [ref] Reference to the result as obtained in the headers. 
// Returns
//		gsi_true if it was able to parse the result successfully, gsi_false otherwise.
// Remarks
//      You can also check the headers manually for the “Sake-File-Result” header. The value stored in the 
//      header is an integer, the possible values of which are enumerated in SAKEFileResult. 
//      SAKEFileResult_SUCCESS means that the file was uploaded successfully, while any other value indicates 
//      that there was an error uploading the file.
// See Also
//		SAKEFileResult
gsi_bool SAKE_CALL sakeGetFileResultFromHeaders(const char *headers, SAKEFileResult *result);

//////////////////////////////////////////////////////////////
// sakeGetFileIdFromHeaders
// Summary
//		If the file was uploaded successfully, this function obtains the fileid that references the file.
// Parameters
//		headers		: [in] The headers to parse for the fileid. Can obtain these by calling ghttpGetHeaders. 
//		fileId		: [ref] reference to the uploaded fileid. 
// Returns
//		gsi_true if able to parse the fileid successfully, gsi_false otherwise.
// Remarks
//      To get the fileid from the headers manually, look for the “Sake-File-Id” header. Once obtained, the 
//      fileid can now be stored in a fileid field in the database.
//
//      *Note that If a file is uploaded, but the fileid is not stored in the database, then the file will 
//      be automatically deleted by the backend after approximately 24 hours.
// See Also
//		ghttpGetHeaders
gsi_bool SAKE_CALL sakeGetFileIdFromHeaders(const char *headers, int *fileId);


#if defined(__cplusplus)
} // extern "C"
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SAKE_H__
