///////////////////////////////////////////////////////////////////////////////
// File:	sampleAuth.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../webservices/AuthService.h"
#ifdef SAKE_SAMPLE
#include "../sake/sake.h"
#endif
#ifdef ATLAS_SAMPLE
#include "../sc/sc.h"
#endif

#ifdef ATLAS_SAMPLE // use atlas specific sample game credentials
static const char * SAMPLE_GAMENAME = "atlasSamples";
static const int SAMPLE_GAME_ID = 1649;
static const char * SAMPLE_SECRET_KEY = "Zc0eM6";
static const char * SAMPLE_ACCESS_KEY = "39cf9714e24f421c8ca07b9bcb36c0f5";
#else
// game identifiers - these can be found on your Dashboard on the developer portal
static const char * SAMPLE_GAMENAME = "gmtest";
static const int SAMPLE_GAME_ID = 0;
static const char * SAMPLE_SECRET_KEY = "HA6zkS";
static const char * SAMPLE_ACCESS_KEY = "39cf9714e24f421c8ca07b9bcb36c0f5";
#endif

// user credentials
static char * SAMPLE_UNIQUENICK[3] = {"gsSampleName", "gsSampleName2", "gsSampleName3"};
static const char * SAMPLE_PASSWORD = "gspy";

GSLoginCertificate certificate;
GSLoginPrivateData privateData;
GSLoginCertificate certificates[3];
GSLoginPrivateData privateDatas[3];

gsi_bool AvailabilityCheck();

void SampleAuthenticationCallback(GHTTPResult httpResult, WSLoginResponse * theResponse, void * theUserData);

gsi_bool SampleAuthenticatePlayer(const char * uniqueNick, const char * password);

gsi_bool AuthSetup();

void CoreCleanup();

#ifdef SAKE_SAMPLE
SAKE sake;
gsi_bool SakeSetup();
const char * SakeFieldValueToString(SAKEField * field);
void DisplaySakeRecords(SAKEField ** records, int numRecords, int numFields);
SAKEGetMyRecordsOutput * CopySakeRecordsFromResponseData(SAKEGetMyRecordsInput  *input, SAKEGetMyRecordsOutput *output);
void FreeSakeRecords(SAKEGetMyRecordsOutput *records, int numberOfFields);
gsi_bool DeleteSakeRecord(char * tableName, int recordId);
void SakeCleanup();
#endif

#ifdef ATLAS_SAMPLE
gsi_bool AtlasReportingSetup(SCInterfacePtr * statsInterface, int playerIndex, char * sessionId);
gsi_bool AtlasQuerySetup(SCInterfacePtr * statsInterface);
void DisplayAtlasPlayerRecords(SCPlayerStatsQueryResponse * response);
void AtlasCleanup(SCInterfacePtr statsInterface);
#endif

void WaitForUserInput();
