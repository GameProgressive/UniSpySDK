// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "RacingService.h"
#include "../common/gsXML.h"
#include "../common/gsAvailable.h"
#include "../md5.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define WS_RACINGSERVICE_GETREGIONALDATA     "GetRegionalData"
#define WS_RACINGSERVICE_GETCONTESTDATA      "GetContestData"
#define WS_RACINGSERVICE_SUBMITSCORES         "SubmitScores"
#define WS_RACINGSERVICE_SUBMITGHOST         "SubmitGhost"
#define WS_RACINGSERVICE_GETTOPTENRANKINGS   "GetTopTenRankings"
#define WS_RACINGSERVICE_GETTENABOVERANKINGS "GetTenAboveRankings"
#define WS_RACINGSERVICE_GETFRIENDRANKINGS   "GetFriendRankings"

#define WS_RACINGSERVICE_NAMESPACE             "ns1"
#define WS_RACINGSERVICE_GETREGIONALDATA_SOAP  "SOAPAction: \"http://gamespy.net/RaceService/GetRegionalData\""
#define WS_RACINGSERVICE_GETCONTESTDATA_SOAP   "SOAPAction: \"http://gamespy.net/RaceService/GetContestData\""
#define WS_RACINGSERVICE_SUBMITSCORES_SOAP      "SOAPAction: \"http://gamespy.net/RaceService/SubmitScores\""
#define WS_RACINGSERVICE_SUBMITGHOST_SOAP      "SOAPAction: \"http://gamespy.net/RaceService/SubmitGhost\""
#define WS_RACINGSERVICE_GETTOPTENRANKINGS_SOAP   "SOAPAction: \"http://gamespy.net/RaceService/GetTopTenRankings\""
#define WS_RACINGSERVICE_GETTENABOVERANKINGS_SOAP "SOAPAction: \"http://gamespy.net/RaceService/GetTenAboveRankings\""
#define WS_RACINGSERVICE_GETFRIENDRANKINGS_SOAP   "SOAPAction: \"http://gamespy.net/RaceService/GetFriendRankings\""

#define WS_RACINGSERVICE_NAMESPACE_COUNT  1
const char * WS_RACINGSERVICE_NAMESPACES[WS_RACINGSERVICE_NAMESPACE_COUNT] =
{
	WS_RACINGSERVICE_NAMESPACE "=\"http://gamespy.net/RaceService/\""
};

// This is declared as an extern so it can be overriden when testing.
#define WS_RACING_SERVICE_URL_FORMAT   "http://%s.race.pubsvs." GSI_DOMAIN_NAME "/RaceService/NintendoRacingService.asmx"
char wsRacingServiceURL[WS_RACING_MAX_URL_LEN] = "";

typedef struct WSIRequestData
{
	union 
	{
		WSRacingGetRegionalDataCallback mRegionalDataCallback;
		WSRacingGetContestDataCallback  mContestDataCallback;
		WSRacingSubmitScoresCallback     mSubmitScoresCallback;
		WSRacingSubmitGhostCallback     mSubmitGhostCallback;
		WSRacingGetRanksCallback        mGetRanksCallback;
	} mUserCallback;
	void * mUserData;
} WSIRequestData;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Checks to make sure the Availability Check has been performed prior to using 
// any AuthService Login, and sets the service URL if it has.
static gsi_bool wsiServiceAvailable()
{
	if (__GSIACResult == GSIACAvailable)
	{
		if (wsRacingServiceURL[0] == '\0')
			snprintf(wsRacingServiceURL, WS_RACING_MAX_URL_LEN, WS_RACING_SERVICE_URL_FORMAT, __GSIACGamename);
		return gsi_true;
	}
	else
		return gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void wsRacingSetServiceURL(const char url[WS_RACING_MAX_URL_LEN])
{
	strcpy(wsRacingServiceURL, url);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsRacingGetRegionalDataCallback(GHTTPResult theResult, 
								   GSXmlStreamWriter theRequestXml,
								   GSXmlStreamReader theResponseXml,
								   void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

	WSRacingGetRegionalDataResponse response;
	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPSuccess)
	{
		// Try to parse the SOAP.
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "GetRegionalDataResult")))
		{
			response.mResult = WSRacing_ParseError;
		}
		else
		{
			// Prepare the response structure.
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				// Could not parse login response code.
				response.mResult = WSRacing_ParseError;
			}
			else if (response.mResponseCode != WSRacing_Success)
			{
				// Server reported an error into reponseCode.
				response.mResult = WSRacing_ServerError;
			}
			else 
			{
				// Read out the response data.
				int numRegions = 0;
				if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "regioncount", &numRegions)) ||
					gsi_is_false(gsXmlMoveToNext(theResponseXml, "regionDataArray")) )
				{
					// Parse the error., number of regions not supplied.
					// reponse.mResult = WSRacing_ParseError;
				}
				else
				{
					int i=0;

					// Allocate region block.
					response.mRegionalData = (WSRacingRegionalData*)gsimalloc(numRegions*sizeof(WSRacingRegionalData));
					if (response.mRegionalData == NULL)
					{
						//response.mResult = WSRacing_OUTOFMEMORY;
						//
					}

					// Read each region's data.
					for (i=0; i < numRegions; i++)
					{
						// Check the length of token+challenge, then read it 
						// into memory.
						if ( gsi_is_false(gsXmlMoveToNext(theResponseXml, "RegionData")) ||
							 gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "regionid", &response.mRegionalData[i].mRegionId)) ||
							 gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "regionname", response.mRegionalData[i].mRegionName, WS_RACING_MAX_REGION_NAME_LEN)))
						{
							// Region data is incomplete.
							gsifree(response.mRegionalData);
							response.mResult = WSRacing_ParseError;
						}
					}

					// Successfully parsed the data.
					response.mRegionalDataCount = numRegions;
					response.mResult = WSRacing_Success;
				}
			}
		}
	}
	else if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSRacing_Cancelled;
	}
	else
	{
		response.mResult = WSRacing_HttpError;
	}

	// Trigger the user callback.
	if (requestData->mUserCallback.mRegionalDataCallback != NULL)
	{
		WSRacingGetRegionalDataCallback userCallback = (WSRacingGetRegionalDataCallback)(requestData->mUserCallback.mRegionalDataCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	gsifree(response.mRegionalData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingGetRegionalData(int gameId,
					   WSRacingGetRegionalDataCallback userCallback, 
					   void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mRegionalDataCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETREGIONALDATA)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETREGIONALDATA)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_GETREGIONALDATA_SOAP,
			writer, wsRacingGetRegionalDataCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsRacingGetContestDataCallback(GHTTPResult theResult, 
										   GSXmlStreamWriter theRequestXml,
										   GSXmlStreamReader theResponseXml,
										   void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

#if (WS_RACING_USE_HEAP_FOR_CONTEST_MSG)
	gsi_char * contestTempBuf = (gsi_char*)gsimalloc(WS_RACING_MAX_CONTEST_MSG_LEN*sizeof(gsi_char)); 
	gsi_char * specialTempBuf = (gsi_char*)gsimalloc(WS_RACING_MAX_SPECIAL_MSG_LEN*sizeof(gsi_char));
#else
	gsi_char contestTempBuf[WS_RACING_MAX_CONTEST_MSG_LEN] = { '\0' };
	gsi_char specialTempBuf[WS_RACING_MAX_SPECIAL_MSG_LEN] = { '\0' };
#endif

	WSRacingGetContestDataResponse response;
	memset(&response, 0, sizeof(response));
	response.mResponseCode = WSRacing_ParseError;

	if (theResult == GHTTPSuccess)
	{
		// Try to parse the SOAP.
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "GetContestDataResult")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode))
			)
		{
			response.mResult = WSRacing_ParseError;
		}
		else if (response.mResponseCode != WSRacing_Success)
		{
			// Server reported an error into reponseCode.
			response.mResult = WSRacing_ServerError;
		}
		else if (contestTempBuf == NULL || specialTempBuf == NULL)
		{
			response.mResult = WSRacing_OutOfMemory;
		}
		else
		{
			// Server returned success. Parse out the message contents, then 
			// read out the response data
			if (gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "contestmessage", contestTempBuf, WS_RACING_MAX_CONTEST_MSG_LEN)) ||
				gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "specialmessage", specialTempBuf, WS_RACING_MAX_SPECIAL_MSG_LEN)) ||
				gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "ghostcount", &response.mNumGhosts)) ||
				gsi_is_false(gsXmlMoveToNext(theResponseXml, "contestGhostDataArray"))
				)
			{
				// Parse the error.
				response.mResult = WSRacing_ParseError;
			}
			else
			{
				int i=0;

				// TODO: How to handle case where num ghosts is < 1
				response.mGhostData = (WSRacingGhostData*)gsimalloc(response.mNumGhosts*sizeof(WSRacingGhostData));

				if (response.mGhostData == NULL)
				{
					response.mResult = WSRacing_OutOfMemory;
				}
				else
				{
					response.mContestMessage = contestTempBuf;
					response.mSpecialMessage = specialTempBuf;

					memset(response.mGhostData, 0, (response.mNumGhosts*sizeof(WSRacingGhostData)));

					// Read each ghost data..
					for (i=0; i < response.mNumGhosts; i++)
					{
						// Check the length of the token+challenge, then read 
						// it into memory.
						if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "ContestGhostData")) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "profileid", &response.mGhostData[i].mProfileId)) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "courseid", &response.mGhostData[i].mCourseId)) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "level", &response.mGhostData[i].mLevel)) ||
							gsi_is_false(gsXmlReadChildAsHexBinary(theResponseXml, "rawdata", NULL, WS_RACING_MAX_GHOST_DATA_LEN, &response.mGhostData[i].mDataLen))
							)
						{
							// Region data is incomplete.
							gsifree(response.mGhostData);
							response.mResult = WSRacing_ParseError;
							break; // Assumption: memory cleanup will happen below..
						}
						else
						{
							// Allocate ghost data.
							response.mGhostData[i].mData = (gsi_u8*)gsimalloc((size_t)response.mGhostData[i].mDataLen);
							if (response.mGhostData[i].mData == NULL)
							{
								response.mResult = WSRacing_OutOfMemory;
								break; // Assumption: memory cleanup will happen below.
							}
							memset(response.mGhostData[i].mData, 0, (size_t)response.mGhostData[i].mDataLen);
							if(gsi_is_false(gsXmlReadChildAsHexBinary(theResponseXml, "rawdata", response.mGhostData[i].mData, WS_RACING_MAX_GHOST_DATA_LEN, &response.mGhostData[i].mDataLen)))
							{
								response.mResult = WSRacing_ParseError;
								break; // Assumption: memory cleanup will happen below.
							}
						}
					}
					// Successfully parsed the data.
					response.mResult = WSRacing_Success;
				}
			}
		}
	}
	else if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSRacing_Cancelled;
	}
	else
	{
		response.mResult = WSRacing_HttpError;
	}

	// Trigger the user callback.
	if (requestData->mUserCallback.mContestDataCallback != NULL)
	{
		WSRacingGetContestDataCallback userCallback = (WSRacingGetContestDataCallback)(requestData->mUserCallback.mContestDataCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	if (response.mNumGhosts > 0 && response.mGhostData != NULL)
	{
		int i=0;
		for (i=0; i<response.mNumGhosts; i++)
			gsifree(response.mGhostData[i].mData);
	}
#if (WS_RACING_USE_HEAP_FOR_CONTEST_MSG)
	gsifree(contestTempBuf);
	gsifree(specialTempBuf);
#endif
	gsifree(response.mGhostData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingGetContestData(int gameId,
							   int regionId,
							   int courseId,
							   WSRacingGetContestDataCallback userCallback, 
							   void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mContestDataCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETCONTESTDATA)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courseId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETCONTESTDATA)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_GETCONTESTDATA_SOAP,
			writer, wsRacingGetContestDataCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsRacingGetRanksCallback(GHTTPResult theResult, 
									 GSXmlStreamWriter theRequestXml,
									 GSXmlStreamReader theResponseXml,
									 void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

	WSRacingGetRanksResponse response;
	memset(&response, 0, sizeof(response));
	response.mResponseCode = WSRacing_ParseError;

	if (theResult == GHTTPSuccess)
	{
		// Try to parse the SOAP.
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "responseCode")) ||
			gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode))
			)
		{
			response.mResult = WSRacing_ParseError;
		}
		else if (response.mResponseCode != WSRacing_Success)
		{
			// Server reported an error into reponseCode.
			response.mResult = WSRacing_ServerError;
		}
		else
		{
			// Server returned success. Parse out the message contents, then 
			// read out the response data.
			if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "dataArray")) ||
				gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "numrecords", &response.mNumRecords))
				//gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "contestmessage", contestTempBuf, WS_RACING_MAX_CONTEST_MSG_LEN)) ||
				//gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "specialmessage", specialTempBuf, WS_RACING_MAX_SPECIAL_MSG_LEN)) ||
				//gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "ghostcount", &response.mNumGhosts)) ||
				//gsi_is_false(gsXmlMoveToNext(theResponseXml, "contestGhostDataArray"))
				)
			{
				// Parse the error.
				response.mResult = WSRacing_ParseError;
			}
			else if (response.mNumRecords == 0)
			{
				// Success, but there is no data for this profile.
				response.mResult = WSRacing_Success;
			}
			else
			{
				int i=0;
				response.mRecords = (WSRacingRankingData*)gsimalloc(response.mNumRecords*sizeof(WSRacingRankingData));
				if (response.mRecords == NULL)
				{
					response.mResult = WSRacing_OutOfMemory;
				}
				else
				{
					gsi_bool gotError = gsi_false;

					memset(response.mRecords, 0, (response.mNumRecords*sizeof(WSRacingRankingData)));

					// Read each ghost data.
					for (i=0; i < response.mNumRecords; i++)
					{
						// Check length of token+challenge, then read into memory
						if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "RankingData")) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "ownerid", &response.mRecords[i].mOwnerId)) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "rank", &response.mRecords[i].mRank)) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "time", &response.mRecords[i].mTime)) ||
							gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "ghostfile", &response.mRecords[i].mGhostFile)) ||
							gsi_is_false(gsXmlReadChildAsBase64Binary(theResponseXml, "userdata", response.mRecords[i].mUserData, &response.mRecords[i].mUserDataLen))
							)
						{
							// data incomplete
							gsifree(response.mRecords);
							response.mRecords = NULL;
							response.mResult = WSRacing_ParseError;
							gotError=gsi_true;
							
							// Assumption: memory cleanup will happen below.
							break; 
							// FALLS THROUGH AFTER FOR LOOP.
						}
					}
					if (gsi_is_false(gotError))
					{
						// Successfully parsed the data.
						response.mResult = WSRacing_Success;
					}
				}
			}
		}
	}
	else if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSRacing_Cancelled;
	}
	else
	{
		response.mResult = WSRacing_HttpError;
	}

	// Trigger the user callback.
	if (requestData->mUserCallback.mGetRanksCallback != NULL)
	{
		WSRacingGetRanksCallback userCallback = (WSRacingGetRanksCallback)(requestData->mUserCallback.mGetRanksCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);

	// Free the data, then reset it to NULL so the developer is less likely to 
	// keep a reference around.
	if (response.mRecords != NULL)
	{
		gsifree(response.mRecords);
		response.mRecords = NULL;
	}
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingGetTop10Rankings(int gameId, int regionId, int courseId, WSRacingValue scoreMode, WSRacingGetRanksCallback userCallback, void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mGetRanksCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETTOPTENRANKINGS)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courseId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETTOPTENRANKINGS)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_GETTOPTENRANKINGS_SOAP,
			writer, wsRacingGetRanksCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	GSI_UNUSED(scoreMode);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingGetRanksAboveAndBelow(int gameId, int regionId, int courseId, int profileId, WSRacingValue scoreMode, WSRacingGetRanksCallback userCallback, void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mGetRanksCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETTENABOVERANKINGS)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courseId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "profileid", (gsi_u32)profileId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETTENABOVERANKINGS)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_GETTENABOVERANKINGS_SOAP,
			writer, wsRacingGetRanksCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	GSI_UNUSED(scoreMode);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingGetFriendRankings(int gameId, int regionId, int courseId, int profileId, WSRacingValue scoreMode, WSRacingGetRanksCallback userCallback, void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mGetRanksCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETFRIENDRANKINGS)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courseId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "profileid", (gsi_u32)profileId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_GETFRIENDRANKINGS)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_GETFRIENDRANKINGS_SOAP,
			writer, wsRacingGetRanksCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	GSI_UNUSED(scoreMode);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsRacingSubmitGhostCallback(GHTTPResult theResult, 
									 GSXmlStreamWriter theRequestXml,
									 GSXmlStreamReader theResponseXml,
									 void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

	WSRacingSubmitGhostResponse response;
	memset(&response, 0, sizeof(response));
	response.mResponseCode = WSRacing_ParseError;

	if (theResult == GHTTPSuccess)
	{
		// Try to parse the SOAP.
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "SubmitGhostResponse")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "SubmitGhostResult", (int*)&response.mResponseCode))
			)
		{
			response.mResult = WSRacing_ParseError;
		}
		else if (response.mResponseCode != WSRacing_Success)
		{
			// Server reported an error into reponseCode.
			response.mResult = WSRacing_ServerError;
		}
		else
		{

		}
	}
	else if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSRacing_Cancelled;
	}
	else
	{
		response.mResult = WSRacing_HttpError;
	}

	// Trigger the user callback.
	if (requestData->mUserCallback.mSubmitGhostCallback != NULL)
	{
		WSRacingSubmitGhostCallback userCallback = (WSRacingSubmitGhostCallback)(requestData->mUserCallback.mSubmitGhostCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingSubmitGhost(int gameId, int regionId, int courseId, int profileId, int score, int sakeFileId, WSRacingSubmitGhostCallback userCallback, void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	requestData->mUserCallback.mSubmitGhostCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_SUBMITGHOST)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courseId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "profileid", (gsi_u32)profileId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "score", (gsi_u32)score)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "fileid", (gsi_u32)sakeFileId)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_SUBMITGHOST)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_SUBMITGHOST_SOAP,
			writer, wsRacingSubmitGhostCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsRacingSubmitScoresCallback(GHTTPResult theResult, 
									 GSXmlStreamWriter theRequestXml,
									 GSXmlStreamReader theResponseXml,
									 void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

	WSRacingSubmitScoresResponse response;
	memset(&response, 0, sizeof(response));
	response.mResponseCode = WSRacing_ParseError;

	if (theResult == GHTTPSuccess)
	{
		// Try to parse the SOAP.
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "SubmitScoresResult")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode))
			)
		{
			response.mResult = WSRacing_ParseError;
		}
		else if (response.mResponseCode != WSRacing_Success)
		{
			// The server reported an error into the reponseCode.
			response.mResult = WSRacing_ServerError;
		}
		else
		{

		}
	}
	else if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSRacing_Cancelled;
	}
	else
	{
		response.mResult = WSRacing_HttpError;
	}

	// Trigger the user callback.
	if (requestData->mUserCallback.mSubmitScoresCallback != NULL)
	{
		WSRacingSubmitScoresCallback userCallback = (WSRacingSubmitScoresCallback)(requestData->mUserCallback.mSubmitScoresCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_u32 wsRacingSubmitScores(int gameId, int regionId, int profileId, const gsi_u8 **playerData, 
							 int * courses, int * scores, int count, WSRacingValue scoreMode, WSRacingSubmitScoresCallback userCallback, void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable())
		return WSRacing_NoAvailabilityCheck;

	// Allocate the request values.
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSRacing_OutOfMemory;

	// Check parameters.
	if (courses == NULL || scores == NULL || count < 1)
		return WSRacing_InvalidParameters;

	requestData->mUserCallback.mSubmitScoresCallback = userCallback;
	requestData->mUserData     = userData;

	// Create the xml request.
	writer = gsXmlCreateStreamWriter(WS_RACINGSERVICE_NAMESPACES, WS_RACINGSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		GSSoapTask * aTask = NULL;
		int i=0;

		// Open the envelope.
		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_SUBMITSCORES)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, "gameData")) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "regionid", (gsi_u32)regionId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "profileid", (gsi_u32)profileId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "scoremode", (gsi_u32)scoreMode)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, "ScoreDatas"))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		// Dump the request data.
		for (i=0; i < count; i++)
		{
			if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_RACINGSERVICE_NAMESPACE, "ScoreData")) ||
				gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "time", (gsi_u32)scores[i])) ||
				gsi_is_false(gsXmlWriteIntElement   (writer, WS_RACINGSERVICE_NAMESPACE, "courseid", (gsi_u32)courses[i])) ||
				gsi_is_false(gsXmlWriteBase64BinaryElement(writer, WS_RACINGSERVICE_NAMESPACE, "playerinfobase64", playerData[i], WS_RACING_USER_DATA_SIZE)) ||
				gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, "ScoreData"))
				)
			{
				gsXmlFreeWriter(writer);
				return WSRacing_OutOfMemory;
			}
		}
	
		// Close the envelope.
		if (gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, "ScoreDatas")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, "gameData")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_RACINGSERVICE_NAMESPACE, WS_RACINGSERVICE_SUBMITSCORES)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSRacing_OutOfMemory;
		}

		aTask = gsiExecuteSoap(wsRacingServiceURL, WS_RACINGSERVICE_SUBMITSCORES_SOAP,
			writer, wsRacingSubmitScoresCallback, (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSRacing_OutOfMemory;
		}
	}
	return 0;
}