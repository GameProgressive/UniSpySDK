// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __RACINGSERVICE_H__
#define __RACINGSERVICE_H__


// ***** Wii Racing web services.
//
// ***** PUBLIC INTERFACE AT THE BOTTOM OF THE FILE

#include "../common/gsSoap.h"

#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// URL for racing services:
#define WS_RACING_MAX_URL_LEN		  (128)
extern char wsRacingServiceURL[WS_RACING_MAX_URL_LEN];

#define WS_RACING_USER_DATA_SIZE      (80)    // Size of developer-specified data on rankings.

#define WS_RACING_MAX_REGION_NAME_LEN (32)    // Allocated on stack.
#define WS_RACING_MAX_GHOST_DATA_LEN  (16000) // Allocated on heap.

#define WS_RACING_USE_HEAP_FOR_CONTEST_MSG  1 // When 1, the string below will be allocated on heap.
#define WS_RACING_MAX_CONTEST_MSG_LEN (1024)  // Allocated on stack or heap.
#define WS_RACING_MAX_SPECIAL_MSG_LEN (1024)  // Allocated on stack or heap.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef enum WSRacingValue
{
	// Login response code (mResponseCode):
	//   -- GameSpy Devs: Must match server.
	WSRacing_Success = 0,

	WSRacing_DBError = 6,
	WSRacing_ServerError = 7,
	WSRacing_FailureMax,

	// Racing mode types:
	WSRacing_ModeAllTime = 0,
	WSRacing_ModeContest = 1,
	WSRacing_ModeMax = 2,

	// Service Result (mResult):
	WSRacing_HttpError = 100,
	WSRacing_ParseError,
	WSRacing_OutOfMemory = 104, // This is so we can merge with WSLogin results later.
	WSRacing_InvalidParameters,
	WSRacing_NoAvailabilityCheck,
	WSRacing_Cancelled,
	WSRacing_UnknownError

} WSRacingValue;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct GSRacingData
{
	int mCourseId;
	/*
	gsi_bool mIsValid;
*/
} GSRacingData;

//
//
typedef struct WSRacingGhostData{
	int		mProfileId;
	int		mCourseId;
	int		mLevel; // Is this necessary?
	int		mDataLen;
	gsi_u8	* mData;
} WSRacingGhostData;

typedef struct WSRacingRegionalData{
	int			mRegionId;
	gsi_char	mRegionName[WS_RACING_MAX_REGION_NAME_LEN];
} WSRacingRegionalData;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Racing submit score callback data:
typedef struct WSRacingSubmitScoresResponse{
	WSRacingValue	mResult;            // SDK high level result, (e.g., submission failed).
	WSRacingValue	mResponseCode;      // The server's result code, (e.g., invalid course id).
	void			* mUserData;
} WSRacingSubmitScoresResponse;

typedef void (*WSRacingSubmitScoresCallback)(
	GHTTPResult						httpResult, 
	WSRacingSubmitScoresResponse	* response, 
	void							* userData
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Racing submit score callback data:
typedef struct WSRacingSubmitGhostResponse
{
	WSRacingValue mResult;           // SDK high level result, (e.g., submission failed).
	WSRacingValue mResponseCode;     // The server's result code, (e.g., invalid course id).
	void * mUserData;
} WSRacingSubmitGhostResponse;

typedef void (*WSRacingSubmitGhostCallback)(
	GHTTPResult						httpResult, 
	WSRacingSubmitGhostResponse		* response, 
	void							* userData
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Racing get ranks callback data
typedef struct WSRacingRankingData{
	int		mOwnerId;
	int		mRank;
	int		mTime;
	int		mGhostFile;
	int		mUserDataLen;
	gsi_u8	mUserData[WS_RACING_USER_DATA_SIZE];
} WSRacingRankingData;

typedef struct WSRacingGetRanksResponse{
	WSRacingValue			mResult;		// SDK high level result, (e.g., submission failed).
	WSRacingValue			mResponseCode;	// The server's result code, (e.g., invalid course id).
	// Ranking data.
	int						mNumRecords;

	WSRacingRankingData		* mRecords;

	void					* mUserData;
} WSRacingGetRanksResponse;

typedef void (*WSRacingGetRanksCallback)(
	GHTTPResult					httpResult, 
	WSRacingGetRanksResponse	* response, 
	void						* userData
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Racing get regional callback data:
typedef struct WSRacingGetRegionalDataResponse
{
	WSRacingValue			mResult;		// SDK high level result, (e.g., submission failed).
	WSRacingValue			mResponseCode;	// The server's result code, (e.g., invalid course id).
	// Regional data.
	int						mRegionalDataCount;
	WSRacingRegionalData	* mRegionalData;
	void					* mUserData;
} WSRacingGetRegionalDataResponse;

typedef void (*WSRacingGetRegionalDataCallback)(
	GHTTPResult							httpResult, 
	WSRacingGetRegionalDataResponse		* response, 
	void								* userData
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Racing get contest callback data:
typedef struct WSRacingGetContestDataResponse
{
	WSRacingValue mResult;			// SDK high level result, (e.g., submission failed).
	WSRacingValue mResponseCode;	// The server's result code, (e.g., invalid course id).
	
	// Contest data.
	gsi_char * mContestMessage;
	gsi_char * mSpecialMessage;
	
	int mNumGhosts;
	WSRacingGhostData * mGhostData;
	
	void * mUserData;
} WSRacingGetContestDataResponse;

typedef void (*WSRacingGetContestDataCallback)(
	GHTTPResult						httpResult, 
	WSRacingGetContestDataResponse	* response, 
	void							* userData
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Set the service URL:
void wsRacingSetServiceURL(const char url[WS_RACING_MAX_URL_LEN]);

// Get contest message and download current contest ghosts.
gsi_u32 wsRacingGetContestData(
	int								gameId, 
	int								region, 
	int								courseid, 
	WSRacingGetContestDataCallback	callback, 
	void							* userData
);

// Submit a score to either WSRacing_ModeAllTime or WSRacing_ModeContest. Use 
// this to update the high score list.
// NOTE: Each playerData must be of size: WS_RACING_USER_DATA_SIZE, so your 
// array will need be dimensions of [count][WS_RACING_USER_DATA_SIZE].
gsi_u32 wsRacingSubmitScores(
	int								gameid, 
	int								region, 
	int								profileId,
	const gsi_u8					**playerData,
	int								* courseid, 
	int								* score, 
	int								count, 
	WSRacingValue					scoremode, 
	WSRacingSubmitScoresCallback	callback, 
	void							* userData
);

// Use this to update the public ghost table, which does not require a high 
// score.
// * Region is for optional separation. Use region 0 for "world".
gsi_u32 wsRacingSubmitGhost(
	int								gameid, 
	int								region, 
	int								courseid, 
	int								profileid, 
	int								score, 
	int								sakefileid, 
	WSRacingSubmitGhostCallback		callback, 
	void							* userData
);

// Retrieve Ranking Data:
gsi_u32 wsRacingGetTop10Rankings(
	int							gameid, 
	/* ticker?,*/ 
	int							region, 
	int							courseid, 
	WSRacingValue				scoremode, 
	WSRacingGetRanksCallback	callback, 
	void						* userData
);
gsi_u32 wsRacingGetRanksAboveAndBelow(
	int							gameid, 
	/* ticket?,*/ 
	int							region, 
	int							courseid, 
	int							profileid, 
	WSRacingValue				scoremode, 
	WSRacingGetRanksCallback	callback, 
	void						* userData
);
gsi_u32 wsRacingGetFriendRankings(
	int							gameid, 
	/* ticket?,*/ 
	int							region, 
	int							courseid, 
	int							profileid, 
	WSRacingValue				scoremode, 
	WSRacingGetRanksCallback	callback, 
	void						* userData
);

gsi_u32 wsRacingGetRegionalData(
	int									gameid, 
	/*ticket?,*/ 
	WSRacingGetRegionalDataCallback		callback, 
	void								* userData
);




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__RACINGSERVICE_H__
