///////////////////////////////////////////////////////////////////////////////
// File:	brigadesTest.c
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// These files need to be added to application includes
#include "../../common/gsCommon.h"
#include "../../common/gsCore.h"
#include "../../common/gsAvailable.h"
#include "../../webservices/AuthService.h"
#include "../brigades.h"

#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#if defined(_NITRO)
	#include "../../common/nitro/screen.h"
	#define printf Printf
	#define vprintf VPrintf
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Globals
//#define GAMEID          101
#define GAMEID          0
#define SECRET_KEY      "HA6zkS"
#define API_KEY         "39cf9714e24f421c8ca07b9bcb36c0f5"
#define LOGIN_NICK      _T("sctest01")
#define LOGIN_PASSWORD  _T("gspy")
#define MAX_NUM_TESTS   100

typedef struct TestResults
{
    gsi_bool	pass;
    char		errString[512];
    char		requestType[512];
} TestResults;

typedef struct LoginData
{
	GSLoginCertificate *mLoginCert;
	GSLoginPrivateData *mLoginPrivDat;
} LoginData;

gsi_bool                gLoggedIn;          
gsi_bool				gMemInBrigade;
gsi_u32                 gNumOperations;             // state control
TestResults             gResults[MAX_NUM_TESTS];
int                     gIndex = 0;
GSBInstancePtr			gInstance = NULL;
GSBBrigadeMemberList	*gMemberList = NULL;
GSBBrigade *testBrigade;
GSBBrigadeMember *testUpdateBrigadeMember;
gsi_u32					gRoleId = 0;
GSBBrigade              *testBrigade = NULL;
GSBEntitlementList      *gDefaultEntitlementList = NULL;
GSBEntitlementIdList	*gDefaultEntitlementIdList = NULL;
gsi_u32                 gBrigadeToDisband ;
gsi_u32                 gTestPlayerProfileId = 191134465;
GSLoginCertificate		myLoginCertificate;
GSLoginPrivateData		myLoginPrivData;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void printResults()
{
    int i;
    int passed = 0;
    int failed = 0;

    printf("\n=======================================================\n");
    printf("==================== FINAL RESULTS ====================\n");
    printf("=======================================================\n");


    for (i=0; i<gIndex; i++)
    {
        printf("#%3d - %s: ", i+1, gResults[i].requestType);
        if (gResults[i].pass)
        {
            printf("PASS\n");
            passed++;
        }
        else
        {
            printf("FAIL\t[%s]\n", gResults[i].errString);
            failed++;
        }
    }
    printf("TOTAL PASSED = %d\n", passed);
    printf("TOTAL FAILED = %d\n\n", failed);

    if (failed == 0)
    {
        printf("         ALL TEST PASSED - WOOHOO!!!!\n\n");
        printf("                          __\n");
        printf("                  _ww   _a+'D\n");
        printf("           y#,  _r^ # _*^  y`\n");
        printf("          q0 0 a'   W*`    F   ____\n");
        printf("       ;  #^ Mw`  __`. .  4-~~^^`\n");
        printf("      _  _P   ` /'^           `www=.\n");
        printf("    , $  +F    `                q\n");
        printf("    K ]                         ^K`\n");
        printf("  , #_                . ___ r    ],\n");
        printf("  _*.^            '.__dP^^~#,  ,_ *,\n");
        printf("  ^b    / _         ``     _F   ]  ]_\n");
        printf("   '___  '               ~~^    ]   [\n");
        printf("   :` ]b_    ~k_               ,`  yl\n");
        printf("     #P        `*a__       __a~   z~`\n");
        printf("     #L     _      ^------~^`   ,/\n");
        printf("      ~-vww*'v_               _/`\n");
        printf("              ^'q_         _x'\n");
        printf("              __#my..___p/`mma____\n");
        printf("           _awP',`,^'-_'^`._ L L  #\n");
        printf("         _#0w_^_^,^r___...._ t [],'w\n");
        printf("        e^   ]b_x^_~^` __,  .]Wy7` x`\n");
        printf("         '=w__^9*$P-*MF`      ^[_.=\n");
        printf("             ^'y   qw/'^_____^~9 t\n");
        printf("               ]_l  ,'^_`..===  x'\n");
        printf("                '>.ak__awwwwWW###r\n");
        printf("                  ##WWWWWWWWWWWWWW__\n");
        printf("                 _WWWWWWMM#WWWW_JP^'~-=w_\n");
        printf("       .____awwmp_wNw#[w/`     ^#,      ~b___.\n");
        printf("        ` ^^^~^'W___            ]Raaaamw~`^``^^~\n");
        printf("                  ^~'~---~~~~~~`\n");
    }
}

///////////////////////////////////////////////////////////////////////////////
// Process task function
static void processTasks()
{
	if (gNumOperations > 1)
		printf("WHY ARE TWO TASKS PROCESSING HERE!?");

	// Think to process tasks
	while(gNumOperations > 0)
	{
		msleep(5);
		gsCoreThink(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Standard cleanup function
static int cleanupAndQuit(int ret)
{
	// Cleanup SDK
	//////////////////////
	if (gInstance)
		gsbShutdown(gInstance);

	//////////////////////
	// Shutdown the core
	gsCoreShutdown();
	while(gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
	{
		gsCoreThink(0);
		msleep(10);
	}

#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
	fflush(stderr);
	printf("Done - Press Enter\r\n"); 
	fflush(stdout);
	getc(stdin);
#else
	printf("Done.\r\n"); 
#endif
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Brigades requires authentication against the GameSpy Auth Service
///////////////////////////////////////////////////////////////////////////////
void authCallback(GHTTPResult result, WSLoginResponse * response, void * userData)
{
	gNumOperations--;

	// Check for HTTP errors
	if (result != GHTTPSuccess)
	{
		printf("HTTP error when logging in: %d\r\n", result);
		gLoggedIn = gsi_false;
	}

	// Check server result....invalid password etc
	else if (response->mLoginResult != WSLogin_Success)
	{
		printf("Login failed, server reported: %d\r\n", response->mLoginResult);
		gLoggedIn = gsi_false;
	}
	else // Success!
	{
		LoginData *data = (LoginData *)userData;
		GS_ASSERT(data);
		printf("Logged in as %d (%s)\r\n", response->mCertificate.mProfileId, response->mCertificate.mUniqueNick);
		memcpy(data->mLoginCert, &response->mCertificate, sizeof(GSLoginCertificate));
		memcpy(data->mLoginPrivDat, &response->mPrivateData, sizeof(GSLoginPrivateData));
		gLoggedIn = gsi_true;
	}

	GSI_UNUSED(userData);
}

static gsi_bool authenticateLeaderPlayer(GSLoginCertificate *loginCert, GSLoginPrivateData *loginPrivData)
{
	LoginData *test;
	int result;
	test = (LoginData *)gsimalloc(sizeof(LoginData));
	test->mLoginCert = loginCert;
	test->mLoginPrivDat = loginPrivData;

	// need to call this prior to Auth Service login
	wsSetGameCredentials(API_KEY, GAMEID, SECRET_KEY);

	result = wsLoginUnique(GAMEID, WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, 
		LOGIN_NICK, LOGIN_PASSWORD, "", authCallback, (void *)test);

	if (result != WSLogin_Success)
	{
		printf("Failed to start wsLoginUnique: %d\r\n", result);
		return gsi_false;
	}

	gNumOperations++;
	processTasks();

	gsifree(test);
	return gLoggedIn;
}

static gsi_bool authenticateMemberPlayer(GSLoginCertificate *loginCert, GSLoginPrivateData *loginPrivData)
{
	LoginData *test;
	int result;
	test = (LoginData *)gsimalloc(sizeof(LoginData));
	test->mLoginCert = loginCert;
	test->mLoginPrivDat = loginPrivData;

	// need to call this prior to Auth Service login
	wsSetGameCredentials(API_KEY, GAMEID, SECRET_KEY);

	result = wsLoginUnique(GAMEID, WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, 
		"brigadetest01", "GameSpy", "", authCallback, (void *)test);

	if (result != WSLogin_Success)
	{
		printf("Failed to start wsLoginUnique: %d\r\n", result);
		return gsi_false;
	}

	gNumOperations++;
	processTasks();

	gsifree(test);
	return gLoggedIn;
}

//////////////////////////////////////////////////////////////////////////
// Login functions
//////////////////////////////////////////////////////////////////////////
int loginAsLeader()
{
	if(gInstance != NULL) gsbShutdown(gInstance);

	gInstance = NULL;

	// shutdown the core object then reinitialize 
	gsCoreShutdown();
	while (gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
	{
		gsCoreThink(0);
		msleep(10);
	}
	gsCoreInitialize();

	// Authenticate player via Auth Service to get Login Certificate
	if (!authenticateLeaderPlayer(&myLoginCertificate, &myLoginPrivData))
		exit(cleanupAndQuit(-1));

	// PERFORM TESTS
	//////////////////////
	gsbInitialize(&gInstance, GAMEID, &myLoginCertificate, &myLoginPrivData);

	// NOTE: This is for testing purposes only! There is no need to set this after testing is complete
	//gsbSetBaseServiceUrl(gInstance, "kgilmore-dt/BrigadesService/");

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int loginAsMember()
{
	if(gInstance != NULL) gsbShutdown(gInstance);

	gInstance = NULL;

	// shutdown the core object then reinitialize 
	gsCoreShutdown();
	while (gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
	{
		gsCoreThink(0);
		msleep(10);
	}
	gsCoreInitialize();

	// Authenticate player via Auth Service to get Login Certificate
	if (!authenticateMemberPlayer(&myLoginCertificate, &myLoginPrivData))
		exit(cleanupAndQuit(-1));

	// PERFORM TESTS
	//////////////////////
	gsbInitialize(&gInstance, GAMEID, &myLoginCertificate, &myLoginPrivData);

	// NOTE: This is for testing purposes only! There is no need to set this after testing is complete
	// gsbSetBaseServiceUrl(gInstance, "snader-server/BrigadesService/");

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// Display functions
//////////////////////////////////////////////////////////////////////////
//
void displayBrigadeMember(GSBBrigadeMember *brigadeMember)
{
    wprintf(L"\tTitle               : %s \n",brigadeMember->mTitle);
    wprintf(L"\tProfile Id          : %d \n", brigadeMember->mProfileId);
    wprintf(L"\tMember Id           : %d \n", brigadeMember->mBrigadeMemberId);
    wprintf(L"\tBrigade Id          : %d \n", brigadeMember->mBrigadeId);

    wprintf(L"\tRole Id             : %d \n",brigadeMember->mRoleId);
    wprintf(L"\tEmail Opt In        : %s \n", gsi_is_true(brigadeMember->mEmailOptIn)? L"True": L"False");
    wprintf(L"\tDescription         : %s\n", brigadeMember->mDescription);
    printf("\tDate joined           :%s", gsiSecondsToString(&brigadeMember->mDateAdded));
    wprintf(L"\tMembership Status   : ");
    switch (brigadeMember->mStatus)
    {
    case   GSBBrigadeMemberStatus_Active :
        wprintf (L"Active \n");
        break;
    case    GSBBrigadeMemberStatus_Inactive :   
        wprintf (L"Inactive \n");
        break;
    case    GSBBrigadeMemberStatus_Invited :
        wprintf (L"Invited \n");
        break;
    case    GSBBrigadeMemberStatus_RequestJoin :
        wprintf (L"Pending Join \n");
        break;
    case    GSBBrigadeMemberStatus_Leader :	
        wprintf (L"Leader \n");
        break;
    case    GSBBrigadeMemberStatus_Kicked :
        wprintf (L"Kicked Out \n");
        break;
    case    GSBBrigadeMemberStatus_Blocked :
        wprintf (L"Blocked \n");
        break;
    default :
        wprintf (L"%d(Unknown Status) \n", brigadeMember->mStatus);
        break;
    }
}
//////////////////////////////////////////////////////////////////////////

void displayBrigade(GSBBrigade *brigade)
{
    gsi_u32 i = 0;
    wprintf(L"\tBrigade Name        : %s \n",brigade->mName);
    wprintf(L"\tBrigade Id          : %d \n", brigade->mBrigadeId);
    wprintf(L"\tGame Id             : %d \n",brigade->mGameId);
    wprintf(L"\tCreator Profile Id  : %d \n", brigade->mCreatorProfileId);
    wprintf(L"\tDisbanded           : %s \n", gsi_is_true(brigade->mDisbanded)? L"True": L"False");

    if (gsi_is_true(brigade->mDisbanded))
    {
        printf("\tDisband Date        : %s\n", gsiSecondsToString(&brigade->mDisbandDate));
    }
    wprintf(L"\tLeader Profile Id   : %d\n", brigade->mLeaderProfileId);
    wprintf(L"\tMOTD                : %s\n", brigade->mMessageOfTheDay);
    wprintf(L"\tBrigade URL         : %s\n",brigade->mUrl);
    wprintf(L"\tBrigade Tag         : %s\n",brigade->mTag);
    wprintf(L"\tRecruiting Type     : ");
    switch (brigade->mRecruitingType)
    {
    case    GSBRecruitingStatus_Open :
        wprintf (L"Open \n");
        break;
    case    GSBRecruitingStatus_Moderated :   
        wprintf (L"Moderated \n");
        break;
    case    GSBRecruitingStatus_InviteOnly :
        wprintf (L"Invite Only \n");
        break;
    case    GSBRecruitingStatus_NoNewMembers :
        wprintf (L"No New Members \n");
        break;
    default :
        wprintf (L"%d(Unknown Recruiting Type) \n", brigade->mRecruitingType);
        break;
    }
    if (brigade->mLogoList.mCount> 0 )
    {
        for (i = 0; i<(brigade->mLogoList.mCount); i++)
        {

            wprintf(L"\tLOGO[%d] \n", i+1);
            wprintf(L"\t\tDefault Logo  : ");
            brigade->mLogoList.mLogos->mDefaultLogo? wprintf(L"True \n"): wprintf(L"False \n");

            wprintf(L"\t\tLogo File Id  : %d\n",brigade->mLogoList.mLogos->mFileId);
            wprintf(L"\t\tLogo Path     : %s\n",brigade->mLogoList.mLogos->mPath);
            wprintf(L"\t\tLogo Size     : %d\n",brigade->mLogoList.mLogos->mSizeId);
            wprintf(L"\t\tLogo URL      : %s\n",brigade->mLogoList.mLogos->mUrl);

        }
    }
}
//////////////////////////////////////////////////////////////////////////
void displayEntitlement(GSBEntitlement *entitlement)
{
    wprintf(L"\tEntitlement Id   : %d \n", entitlement->mEntitlementId);
    wprintf(L"\tEntitlement Name : %s \n", entitlement->mEntitlementName);
}

//////////////////////////////////////////////////////////////////////////
// void displayBrigadeMember(GSBBrigadeMember *player)
// {
//     wprintf(L"\tBrigade Member Id   : %d \n", player->mBrigadeMemberId);
//     wprintf(L"\tBrigade Id          : %d \n", player->mBrigadeId);
//     wprintf(L"\tProfile Id          : %d \n", player->mProfileId);
//     wprintf(L"\tDescription         : %s \n", player->mDescription);
//     wprintf(L"\tRole Id             : %d \n", player->mRoleId);
//     wprintf(L"\tStatus              : %d \n", player->mStatus);
//     wprintf(L"\tTitle               : %s \n", player->mTitle);
// }

//////////////////////////////////////////////////////////////////////////
void displayBrigadeHistory(GSBBrigadeHistoryEntry *historyEntry)
{
    //wprintf(L"\tGame Id             : %d \n", historyEntry->mGameID);
    wprintf(L"\tHistory Entry Type      : %s \n", historyEntry->mAccessLevel);
    wprintf(L"\tBrigade Id              : %d \n", historyEntry->mBrigadeId);
    printf("\tDate                    : %s \n", gsiSecondsToString(&historyEntry->mDateCreated));
    wprintf(L"\tHistory Entry Id        : %d \n", historyEntry->mHistoryEntryId); 
    wprintf(L"\tAction                  : %s \n", historyEntry->mHistoryAction);
    wprintf(L"\tInstigating Profile Id  : %d \n", historyEntry->mInstigatingProfileId);
    wprintf(L"\tTarget Profile Id       : %d \n", historyEntry->mTargetProfileId);
    wprintf(L"\tSource Profile Nick     : %s \n", historyEntry->mSourceProfileNickname);
    wprintf(L"\tTarget Profile Nick     : %s \n", historyEntry->mTargetProfileNickname);
    wprintf(L"\tNotes                   : %s \n", historyEntry->mNotes);
    wprintf(L"\tReference Id            : %d \n", historyEntry->mReferenceId);
}

///////////////////////////////////////////////////////////////////////////////
// prints request result, returns gsi_false for errors
static gsi_bool handleRequestResult(GSResult operationResult, GSResultSet *resultSet, const char * requestType)
{
    gNumOperations--; // decrement operation count
	
	sprintf(gResults[gIndex].requestType, "%s", requestType);

	if (GS_FAILED(operationResult))
	//if (operationResult != GSResultCode_Success)
	{
		// the operation failed so we need to record it as a failure and just print an error code
		// Developers are free to print better errors by extrapolating from the result
		sprintf(gResults[gIndex].errString, "Error: Operation failed: error code: %08X", operationResult);
		gResults[gIndex].pass = gsi_false;
		gIndex++;
		return gsi_false;
	}
	else if (resultSet && resultSet->mNumResults > 0)
	{
		int i;
		// The operation succeeded but there was a problem performing the operation 
		// e.g. the name of a brigade
		int len; 
        
        if (resultSet->mNumResults > 0)
        {
            len = snprintf(gResults[gIndex].errString, sizeof(gResults[gIndex].errString), "Error: Operation succeeded, but problems occurred when performing it: error codes:");
		
		    for (i = 0; i < resultSet->mNumResults; i++)
		    {
			    len += snprintf(gResults[gIndex].errString + len, sizeof(gResults[gIndex].errString), "% 08X", resultSet->mResults[i]);
                wprintf(L"FAILURE:  %s\n", &resultSet->mErrorMessage[i]);
		    }
            gResults[gIndex].pass = gsi_false;
            gIndex++;
            return gsi_false;
        }
	}

	gResults[gIndex].pass = gsi_true;
	gIndex++;
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
// Run the required availability check (all apps must do this)
static gsi_bool isBackendAvailable()
{
	GSIACResult result;

	GSIStartAvailableCheck("gmtest"); // use "gmtest" here for sample
	do 
	{
		result = GSIAvailableCheckThink();
		msleep(10);
	} while(result == GSIACWaiting);

	// check result
	if (result == GSIACUnavailable)
	{
		printf("Availability check returned GSIACUnavailable -- Backend services have been disabled\r\n");
		return gsi_false;
	}
	else if (result == GSIACTemporarilyUnavailable)
	{
		printf("Availability check returned GSIACTemporarilyUnavailable -- Backend services are temporarily down\r\n");
		return gsi_false;
	}

	// GSIACAvailable
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeByIdCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigade *theBrigade, void *userData)
{    
    
    if(!handleRequestResult(operationResult, resultSet, "gsbGetBrigadeById"))
        return;

    // YAY!
    printf("************************[ GetBrigadeById SUCCESS!!! ]************************\n");
    displayBrigade(theBrigade);
    
    // Now keep a copy of the brigade for our testing later
    if (testBrigade!=NULL)
    {
        gsbFreeBrigade(testBrigade);
//        gsifree(testBrigade);
    }
	gsbCloneBrigade (&testBrigade, theBrigade);
	
    GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeById(GSBInstancePtr gsbInstance, gsi_u32 brigadeId)
{

    gsbGetBrigadeById(gsbInstance, brigadeId, sampleTestGetBrigadeByIdCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestDisbandBrigadeCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbDisbandBrigade"))
    {
        wprintf(L"FAILED with return code 0x%x\n", operationResult);
        return;
    }

    // YAY!
    printf("************************[ DisbandBrigade SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestDisbandBrigade(GSBInstancePtr instance, gsi_u32 brigadeId)
{
    wprintf(L"Testing Start: Disband Brigade %d\n", brigadeId);

    gsbDisbandBrigade(instance, brigadeId, sampleTestDisbandBrigadeCallback, NULL);
    gNumOperations++;
    processTasks();

    wprintf(L"Testing End Disband Brigade\n"); 
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestInviteBrigadeMemberCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbInviteBrigadeMember"))
        return;

    // YAY!
    printf("************************[ InviteBrigadeMember SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}

void sampleTestInviteToBrigade(GSBInstancePtr gsbInstance,gsi_u32 brigadeId, gsi_u32 invitedProfileId)
{
	wprintf(L"Testing: Invite to brigade\n");
	gsbInviteToBrigade(gsbInstance,brigadeId, invitedProfileId, sampleTestInviteBrigadeMemberCallback, NULL);
	gNumOperations++;
	processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestAcceptJoinCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbAcceptJoin"))
		return;

	// YAY!
	printf("************************[ AcceptJoin SUCCESS!!! ]***********************\n");

	GSI_UNUSED(userData);
}

void sampleTestAcceptJoin(GSBInstancePtr gsbInstance,gsi_u32 brigadeId, gsi_u32 profileId)
{
	wprintf(L"Testing: Accept Join\n");
	gsbAnswerJoin(gsbInstance,brigadeId, profileId, gsi_true, sampleTestAcceptJoinCallback, NULL);
	gNumOperations++;
	processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestBanMemberCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbBanMember"))
		return;

	// YAY!
	printf("************************[ BanMember SUCCESS!!! ]***********************\n");

	GSI_UNUSED(userData);
}

void sampleTestBanMember(GSBInstancePtr gsbInstance,gsi_u32 brigadeId, gsi_u32 profileId)
{
	wprintf(L"Testing: Ban Member\n");
	gsbBanMember(gsbInstance, brigadeId, profileId, sampleTestBanMemberCallback, NULL);
	gNumOperations++;
	processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestRescindInviteCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbRescindInvite"))
		return;

	printf("************************[ gsbRescindInvite SUCCESS!!! ]***********************\n");

	GSI_UNUSED(userData);
}

void sampleTestRescindInvite(GSBInstancePtr gsbInstance,gsi_u32 brigadeId, gsi_u32 profileId)
{
    wprintf(L"Testing: Rescind Invite\n");
    gsbRescindInvite(gsbInstance, brigadeId, profileId, sampleTestRescindInviteCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestLeaveBrigadeCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbLeaveBrigade"))
        return;

    // YAY!
    printf("************************[ LeaveBrigade SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}

void sampleTestLeaveBrigade(GSBInstancePtr gsbInstance,gsi_u32 brigadeId)
{
    gsbLeaveBrigade(gsbInstance,brigadeId, sampleTestLeaveBrigadeCallback, NULL);
    gNumOperations++;
    processTasks();

}
///////////////////////////////////////////////////////////////////////////////
void sampleTestAnswerInviteCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbAnswerInvite"))
    {
        wprintf(L"Failed Result code : 0x%x\n", operationResult);
        return;
    }

    // YAY!
    printf("************************[ AnswerInvite SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestAnswerInvite(GSBInstancePtr instance, gsi_u32 brigadeId, gsi_bool answer)
{

    if (gsi_is_false(answer))
        printf("*** Testing Decline Invite\n");
    else
        printf("*** Testing Accept Invite\n");
    gsbAnswerInvite(instance ,brigadeId, answer, sampleTestAnswerInviteCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestAnswerJoinCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbAnswerJoin"))
    {
        wprintf(L"Failed Result code : 0x%x\n", operationResult);
        return;
    }

    // YAY!
    printf("************************[ AnswerJoin SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestAnswerJoin(GSBInstancePtr instance, gsi_u32 brigadeId, gsi_u32 profileId, gsi_bool answer)
{

    if (gsi_is_false(answer))
        printf("*** Testing Reject Join\n");
    else
        printf("*** Testing Accept Join\n");
    gsbAnswerJoin(instance ,brigadeId, profileId, answer, sampleTestAnswerJoinCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestJoinBrigadeCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbJoinBrigade"))
        return;

    // YAY!
    printf("************************[ JoinBrigade SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestJoinBrigade(GSBInstancePtr instance, gsi_u32 brigadeId)
{
    gsbJoinBrigade(instance, brigadeId, sampleTestJoinBrigadeCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestRemoveBrigadeMemberCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbRemoveMember"))
        return;

    // YAY!
    printf("************************[ RemoveMember SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}

void sampleTestRemoveBrigadeMember(GSBInstancePtr instance, gsi_u32 profileIdToRemove, gsi_u32 brigadeId)
{
    gsbRemoveBrigadeMember(instance,brigadeId, profileIdToRemove, sampleTestRemoveBrigadeMemberCallback, NULL);
    gNumOperations++;
    processTasks();
}
///////////////////////////////////////////////////////////////////////////////
void displayRole( GSBRole *role)
{
    wprintf(L"\tBrigade Id      : %d\n", role->mBrigadeId);
    wprintf(L"\tRole Name       : %s\n",role->mRoleName);
    wprintf(L"\tRole Id         : %d \n",role->mRoleId);
    wprintf(L"\tIs Default      : %s\n", role->mIsDefault? L"True":L"False");
    wprintf(L"\tIs Game Role    : %s\n",role->mIsGameRole?L"True":L"False");
}

///////////////////////////////////////////////////////////////////////////////
void displayPendingAction( GSBBrigadePendingActions *action)
{
    wprintf(L"\tBrigade Id          : %d\n", action->mBrigadeId);
    wprintf(L"\tRole Name           : %s\n", action->mBrigadeName);

#ifdef _USE_32BIT_TIME_T // Todo: does this work on non windows platforms?
    wprintf(L"\tRole Id             : %d \n", action->mDateAdded);
#else
    wprintf(L"\tRole Id             : %lldd \n",action->mDateAdded);
#endif


    printf("\tDate joined           : %s", gsiSecondsToString(&action->mDateAdded));
    wprintf(L"\tMembership Status   : ");
    switch (action->mStatus)
    {
    case   GSBBrigadeMemberStatus_Active :
        wprintf (L"Active \n");
        break;
    case    GSBBrigadeMemberStatus_Inactive :   
        wprintf (L"Inactive \n");
        break;
    case    GSBBrigadeMemberStatus_Invited :
        wprintf (L"Invited \n");
        break;
    case    GSBBrigadeMemberStatus_RequestJoin :
        wprintf (L"Pending Join \n");
        break;
    case    GSBBrigadeMemberStatus_Leader :	
        wprintf (L"Leader \n");
        break;
    case    GSBBrigadeMemberStatus_Kicked :
        wprintf (L"Kicked Out \n");
        break;
    case    GSBBrigadeMemberStatus_Blocked :
        wprintf (L"Blocked \n");
        break;
    default :
        wprintf (L"%d(Unknown Status) \n", action->mStatus);
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestGetRoleListByBrigadeIdCallback(GSResult operationResult, GSResultSet *resultSet, GSBRoleList *roleList, void *userData)
{
    gsi_u32 i = 0;
    //GSBRole *myRole = (GSBRole*)userData;
    wprintf(L"=============[ Get Role List By Brigade Id Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbGetRoleList"))
        return;

    // YAY!
    printf("************************[ SUCCESS!!! ]***********************\n");
    
    for (i = 0; i<roleList->mCount; i++)
    {  
        // Just for now. Use the last role in the list for testing
        if (i == (roleList->mCount -1) )
        {
            gRoleId = roleList->mRoles[i].mRoleId;
        }
        wprintf(L"============  Role [%d] ============\n", i+1);
        displayRole(&roleList->mRoles[i]);
    }
    // use the brigade I just created
    //myRole->mRoleId = roleId;
    wprintf(L"=============[ Get Role List By Brigade Id Response END ]================\n");

    GSI_UNUSED(userData);
}

void sampleTestGetRoleListByBrigadeId(GSBInstancePtr gsbInstance, gsi_u32 brigadeId)
{
    wprintf(L"============== Testing : Get Role List By Brigade Id %d ===========\n", brigadeId);
    gsbGetRoleList(gsbInstance, brigadeId, sampleTestGetRoleListByBrigadeIdCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestGetPendingInvitesAndJoinsCallback(GSResult operationResult, 
                                                 GSResultSet *resultSet, 
                                                 GSBBrigadePendingActionsList *actionsList, 
                                                 void *userData)
{
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Pending Invites And Joins Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbGetMyPendingInvitesAndJoins"))
        return;

    // YAY!
    printf("************************[ SUCCESS!!! ] Found %d ***********************\n", actionsList->mCount);

    for (i = 0; i< actionsList->mCount; i++)
    {  
        wprintf(L"============  Pending Action [%d] ============\n", i+1);
        displayPendingAction(&actionsList->mPendingActions[i]);
    }

    wprintf(L"=============[ Get Pending Invites And Joins Response END ]================\n");

    GSI_UNUSED(userData);
}


///////////////////////////////////////////////////////////////////////////////
void sampleTestCreateRoleCallback(GSResult operationResult, GSResultSet *resultSet, gsi_u32 roleId, void *userData)
{    
    //GSBRole *myRole = (GSBRole*)userData;
    wprintf(L"=============[ Create Role Response START ]================\n");
    if(!handleRequestResult(operationResult, resultSet, "gsbCreateRole"))
    {
        wprintf(L"FAILED\n");
        return;
    }
    // YAY!
    printf("************************[ SUCCESS!!! ]***********************\n");
    printf("Role ID = %d\n", roleId);
    gRoleId = roleId;
    wprintf(L"=============[ Create Role Response END ]================\n");

    // use the brigade I just created
    //myRole->mRoleId = roleId;
    gRoleId = roleId;
    GSI_UNUSED(userData);
}

void sampleTestCreateRole(GSBInstancePtr gsbInstance)
{

    GSBRole myRole;
    GSBEntitlementIdList *myEntitlements = NULL;

    myRole.mBrigadeId = testBrigade->mBrigadeId;
    myRole.mRoleId = 0;
    myRole.mIsDefault = gsi_false;
    myRole.mRoleName = goawstrdup(L"The Queen3");
    myRole.mIsGameRole = gsi_false;

    myEntitlements = (GSBEntitlementIdList *) gsimalloc (sizeof(GSBEntitlementIdList));
	myEntitlements->mCount = 2;
    myEntitlements->mEntitlementIds = (gsi_u32 *) gsimalloc(myEntitlements->mCount * sizeof(gsi_u32));
    myEntitlements->mEntitlementIds[0] = gDefaultEntitlementList->mEntitlements[0].mEntitlementId;
	myEntitlements->mEntitlementIds[1] = gDefaultEntitlementList->mEntitlements[1].mEntitlementId;

    gsbCreateRole(gsbInstance, &myRole, myEntitlements, sampleTestCreateRoleCallback, NULL);
    gNumOperations++;
    processTasks();

	gsbFreeEntitlementIdList(myEntitlements);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestUpdateRoleCallback(GSResult operationResult, GSResultSet *resultSet, gsi_u32 roleId, void *userData)
{    
    //GSBRole *myRole = (GSBRole*)userData;
    wprintf(L"=============[ Update Role Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbUpdateRole"))
        return;

    // YAY!
    printf("*****************[ UpdateRole SUCCESS!!! ]*****************\n");
    printf("Role ID = %d\n", roleId);

    wprintf(L"=============[ Update Role Response END ]================\n");

    //gRoleId = roleId;
    GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestUpdateRole(GSBInstancePtr gsbInstance, gsi_u32 roleId)
{
    GSBRole myRole;
    GSBEntitlementIdList *myEntitlementIds = NULL;

    myRole.mBrigadeId   = testBrigade->mBrigadeId;
    myRole.mRoleId      = roleId;
    myRole.mIsDefault   = gsi_false;
    myRole.mRoleName    = goawstrdup(L"Nemo");
    myRole.mIsGameRole  = gsi_false;

    gsbCloneEntitlementIdList(&myEntitlementIds, gDefaultEntitlementIdList);
    gsbUpdateRole(gsbInstance, &myRole, myEntitlementIds, sampleTestUpdateRoleCallback, NULL);
    gNumOperations++;
    processTasks();
    
    // release the memory allocated
    gsbFreeEntitlementIdList(myEntitlementIds);
}

///////////////////////////////////////////////////////////////////////////////
void AddRoleEntitlementCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    if(!handleRequestResult(operationResult, resultSet, "gsbAddRoleEntitlementCallback"))
        return;

    // YAY!
    printf("************************[ AddRoleEntitlement SUCCESS!!! ]***********************\n");

    GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestRemoveRoleCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    wprintf(L"=============[ Remove Role  Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbRemoveRoleEntitlementCallback"))
    {
        wprintf (L"FAILED with 0x%x\n", operationResult);
        return;
    }

    // YAY!
    printf("*****************[ RemoveRole SUCCESS!!! ]*****************\n");
    wprintf(L"=============[ Remove Role Response END ]================\n");

    GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestRemoveRole(GSBInstancePtr gsbInstance, gsi_u32 brigadeId, gsi_u32 roleId)
{
    wprintf(L"\n======== Testing Start: Remove Role  ===========\n");
    gsbRemoveRole(gsbInstance, brigadeId, roleId, sampleTestRemoveRoleCallback, NULL);
    gNumOperations++;
    processTasks();
    wprintf(L"======== Testing End: Remove Role  ===========\n");
}

///////////////////////////////////////////////////////////////////////////////
void GetRoleEntitlementListCallback(GSResult operationResult, GSResultSet *resultSet, GSBRoleEntitlementList *roleEntitlementList, void *userData)
{    
    gsi_u32 i; 

    if(!handleRequestResult(operationResult, resultSet, "gsbGetRoleEntitlementList"))
    {
        wprintf (L"FAILED with 0x%x\n", operationResult);
        return;
    }

    // YAY!
    printf("*****************[ GetRoleEntitlement SUCCESS!!! ]*****************\n");
    for (i = 0; i < roleEntitlementList->mCount; i++) 
    {
	   // printf("Role Entitlement ID = %d\n retrieved", roleEntitlementList->mRoleEntitlements[i].mEntitlementId);
	   // printf("Entitlement Name = %d\n retrieved", entitlementList->mEntitlements[i].mEntitlementId);

    }
   
    GSI_UNUSED(userData);
}


///////////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeMemberListCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigadeMemberList *memberList, void *userData)
{    
    gsi_u32 i;
	GSResult rcode;
	gMemInBrigade = gsi_false;
    wprintf(L"=============[ Get Brigade Member List Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbGetBrigadeMemberList"))
    {
        wprintf (L"FAILED with 0x%x\n", operationResult);
        return;
    }

    // YAY!
    wprintf(L"*****************[ GetBrigadeMember SUCCESS!!! ]*****************\n");
	
    for (i=0;i<memberList->mCount;i++) 
    {
        wprintf(L"Member ID: %d = %s retrieved\n", memberList->mBrigadeMembers[i].mBrigadeMemberId,memberList->mBrigadeMembers[i].mDescription);
		
		if(memberList->mBrigadeMembers[i].mProfileId == gTestPlayerProfileId)
				  gMemInBrigade = gsi_true;
		
		if (i == 0)
		{			
			rcode = gsbCloneBrigadeMember(&testUpdateBrigadeMember, &memberList->mBrigadeMembers[i]);
			if (GS_FAILED(rcode))
			{
				wprintf(L"Failed to duplicate member.\n");
			}
		}
	}
    // Refresh the game member list
    if (gMemberList != NULL)
    {
        gsbFreeBrigadeMemberList(gMemberList);
        gMemberList = NULL;
    }
    gsbCloneBrigadeMemberList(&gMemberList, memberList);

    for (i=0;i<memberList->mCount;i++) 
    {
        wprintf(L"=============================[Brigade Member[%d] ]==========================\n", (i+1));
        displayBrigadeMember(&memberList->mBrigadeMembers[i]);	
	}
    wprintf(L"=============[ Get Brigade Member List Response END ]================\n");
    GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeMemberList(GSBInstancePtr gsbInstance, gsi_u32 brigadeId, gsi_u32 status)
{
    gsbGetBrigadeMemberList(gsbInstance, brigadeId, status, sampleTestGetBrigadeMemberListCallback, NULL);
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestUpdateBrigadeMemberCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    wprintf(L"=============[ Update Brigade Member Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbUpdateBrigadeMember"))
        return;

    // YAY!
    printf("************************[ UpdateBrigadeMember SUCCESS!!! ]***********************\n");
    wprintf(L"=============[ Update Brigade Member Response END ]================\n");

    GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestUpdateBrigadeMember(GSBInstancePtr gsbInstance, GSBBrigadeMemberList *memberList)
{

	if ((memberList != NULL) && (memberList->mCount>0)) 
	{
        // Just use the first one on the list for this test
		gsifree(memberList->mBrigadeMembers[0].mTitle);
		memberList->mBrigadeMembers[0].mTitle  = goawstrdup(L"Sergeant");
		memberList->mBrigadeMembers[0].mRoleId = 83;
	    gsbUpdateBrigadeMember(gsbInstance,&memberList->mBrigadeMembers[0],sampleTestUpdateBrigadeMemberCallback, NULL);
	    gNumOperations++;
	    processTasks();
	}
    else
        wprintf(L"Member List empty");
}


//////////////////////////////////////////////////////////////////////////
void sampleTestSearchBrigadesCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigadeList *brigadeList, void * userData)
{
    gsi_u32 i = 0;
    wprintf(L"=============[ Search Brigade Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbSearchBrigades"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=====================[Brigade List ]==========================\n");
            for (i = 0; i < brigadeList->mCount; i++)
            {
                wprintf(L"=============================[Brigade %d ]==========================\n", (i+1));

                displayBrigade(&brigadeList->mBrigades[i]);
            }
        }
    }
    wprintf(L"=============[ Search Brigade Response END ]================\n");
    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestSearchBrigade(GSBInstancePtr gsbInstance, UCS2String searchStr)
{
    GSBSearchFilterList * filterList;
    filterList = (GSBSearchFilterList *) gsimalloc(sizeof(GSBSearchFilterList));
    filterList->mCount = 1;
    filterList->mFilters = (GSBSearchFilter *) gsimalloc (sizeof(GSBSearchFilter) * filterList->mCount);
    memset (filterList->mFilters , 0, (sizeof(GSBSearchFilter) * filterList->mCount));
    filterList->mFilters[0].mKey = GSBSearchFilterKey_Name;
    filterList->mFilters[0].mValue.mValueStr = goawstrdup(searchStr);
    filterList->mFilters[0].mType = GSBSearchFilterValueType_Unicode;

    gsbSearchBrigades(gsbInstance, filterList, sampleTestSearchBrigadesCallback, NULL);
    gNumOperations++;
    processTasks();
}

//////////////////////////////////////////////////////////////////////////
void sampleTestSearchPlayersCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigadeMemberList *playerList, void * userData)
{
    gsi_u32 i = 0;
    wprintf(L"=============[ Search Player Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbSearchPlayers"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=============================[Player List ]==========================\n");
            for (i = 0; i< playerList->mCount; i++)
            {
                wprintf(L"=============================[Player %d ]==========================\n", (i+1));

                displayBrigadeMember(&playerList->mBrigadeMembers[i]);
            }

        }

    }

    wprintf(L"=============[ Search Player Response END ]================\n");
    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestSearchPlayers(GSBInstancePtr gsbInstance)
{
    GSBSearchFilterList * filterList;

    filterList = (GSBSearchFilterList *) gsimalloc(sizeof(GSBSearchFilterList));
    filterList->mCount = 2;
    filterList->mFilters = (GSBSearchFilter *) gsimalloc (sizeof(GSBSearchFilter) * filterList->mCount);
    memset (filterList->mFilters , 0, (sizeof(GSBSearchFilter) * filterList->mCount));
	filterList->mFilters[0].mKey = GSBSearchFilterKey_BrigadeId;
	filterList->mFilters[0].mValue.mValueUint = 165842;
	filterList->mFilters[0].mType = GSBSearchFilterValueType_Uint;
	filterList->mFilters[1].mKey = GSBSearchFilterKey_Name;
    filterList->mFilters[1].mValue.mValueStr = goawstrdup(L"sctest01");
    filterList->mFilters[1].mType = GSBSearchFilterValueType_Unicode;

    gsbSearchPlayers(gsbInstance, filterList, sampleTestSearchPlayersCallback, NULL);
    gNumOperations++;
    processTasks();

}

void sampleTestUploadCompleteCallback(GSResult operationResult, GSResultSet *resultSet, void *userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbUploadLogo"))
		return;

	printf("************************[ UploadLogo SUCCESS!!! ]***********************\n");
	GSI_UNUSED(userData);
     
}

//////////////////////////////////////////////////////////////////////////
void sampleTestUploadBrigadeLogo(GSBInstancePtr gsbInstance, gsi_char *fileName, gsi_u32 brigadeId)
{
	gsbUploadLogo(gsbInstance, fileName, brigadeId,  sampleTestUploadCompleteCallback, NULL);
	gNumOperations++;
	processTasks();

}

void sampleTestDownloadCompleteCallback(GSResult operationResult, GSResultSet *resultSet, void *userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbDownloadLogo"))
		return;

	printf("************************[ DownloadLogo SUCCESS!!! ]***********************\n");
	GSI_UNUSED(userData);
     
}

void sampleTestDownloadBrigadeLogo(GSBInstancePtr gsbInstance, gsi_char *fileName,GSBBrigadeLogo *blogo)
{
	gsbDownloadLogo(gsbInstance, fileName, blogo ,  sampleTestDownloadCompleteCallback, NULL);
	gNumOperations++;
	processTasks();

}

void GetRoleListCallback(GSResult operationResult, GSResultSet *resultSet, GSBRoleList *roleList, void *userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbGetRoleList"))
		return;

	printf("************************[ GetRoleList SUCCESS!!! ]***********************\n");
	if (roleList)
	{
		gsi_u32 i;

		for (i = 0; i < roleList->mCount; i++)
		{
			wprintf(L"Role %d\n", i );
			wprintf(L"Brigade ID: %d\n", roleList->mRoles[i].mBrigadeId);
			wprintf(L"Role ID: %d\n", roleList->mRoles[i].mRoleId);
			wprintf(L"Role ID: %s\n", roleList->mRoles[i].mRoleName);
		}
		gsbFreeRoleList(roleList);
	}
	GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestSaveBrigadeCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigade *brigade, void *userData)
{
	if(!handleRequestResult(operationResult, resultSet, "gsbSaveBrigade"))
    {
		return;
    }
	if (brigade!= NULL)
	{
		 displayBrigade(brigade);		
         gBrigadeToDisband = brigade->mBrigadeId;
	}
	GSI_UNUSED(userData);
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestSaveBrigade( GSBInstancePtr gsbInstance, GSBBrigade *brigade, gsi_bool update)
{
    if (update)
    {
        wprintf(L"======[ Update Brigade ]===========\n");
    }
    else
    {
        wprintf(L"======[ Create Brigade ]===========\n");
    }

    if (brigade != NULL)
    {
        gsbSaveBrigade(gsbInstance, brigade, sampleTestSaveBrigadeCallback, NULL);
        gNumOperations++;
        processTasks();

    }
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestCreateBrigade(GSBInstancePtr instance, 
                             UCS2String brigadeName, 
                             UCS2String brigadeTag, 
                             UCS2String brigadeUrl, 
                             UCS2String brigadeMOTD, 
                             GSBRecruitingType recruitingType)
{
    GSBBrigade *newBrigade = NULL ;

    newBrigade = (GSBBrigade *) gsimalloc(sizeof(GSBBrigade));

    memset(newBrigade, 0, sizeof(GSBBrigade));

    newBrigade->mName    = goawstrdup(brigadeName);
    newBrigade->mTag     = goawstrdup(brigadeTag);
    newBrigade->mUrl     = goawstrdup(brigadeUrl);
    newBrigade->mMessageOfTheDay = goawstrdup(brigadeMOTD);
    newBrigade->mRecruitingType  = recruitingType;
    sampleTestSaveBrigade(instance, newBrigade, gsi_false);
    gsbFreeBrigade(newBrigade);
    
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestGetEntitlementListCallback(GSResult operationResult, GSResultSet *resultSet, GSBEntitlementList *entitlementList, void *userData)
{    
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Entitlement List By Game Id Response START ]================\n");
    if(!handleRequestResult(operationResult, resultSet, "gsbGetEntitlementList"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=======================[Entitlement List ]======================\n");
            for (i=0; i< entitlementList->mCount; i++)
            {
                wprintf(L"=======================[Entitlement %d ]======================\n", (i+1));

                displayEntitlement(&entitlementList->mEntitlements[i]);
            }
        }
    }
    wprintf(L"=============[ Get Entitlement List By Game Id END ]================\n");

    // Keep a deep copy of the entitlement list retrieved from the backend
    gsbCloneEntitlementList(&gDefaultEntitlementList,entitlementList);

    if (gDefaultEntitlementIdList == NULL)
    {

        //Initialize the default entitlementIdList here since it is needed for test data
        gDefaultEntitlementIdList = (GSBEntitlementIdList *) gsimalloc(sizeof(GSBEntitlementIdList));
        gDefaultEntitlementIdList->mCount = entitlementList->mCount ;
        gDefaultEntitlementIdList->mEntitlementIds = (gsi_u32 *) gsimalloc(entitlementList->mCount * sizeof(gsi_u32));
        for (i = 0; i< entitlementList->mCount; i++ )
        {
            gDefaultEntitlementIdList->mEntitlementIds[i] = entitlementList->mEntitlements[i].mEntitlementId; 
        }
    }
    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestGetEntitlementListByGameId(GSBInstancePtr gsbInstance)
{
	gsbGetEntitlementList(gsbInstance,sampleTestGetEntitlementListCallback, NULL);
    gNumOperations++;
    processTasks();
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestDisplayRoleEntitlement(GSBRoleEntitlement *roleEntitlement)
{
    wprintf(L"\tRole Id             : %d\n", roleEntitlement->mRoleId);
    wprintf(L"\tEnttilement Id      : %d\n",roleEntitlement->mEntitlementId);
    wprintf(L"\tRole-Entitlement Id : %d\n",roleEntitlement->mRoleEntitlementId);
};
///////////////////////////////////////////////////////////////////////////////
void sampleTestGetRoleEntitlementListByEntitlementCallback(GSResult operationResult, GSResultSet *resultSet, GSBRoleIdList *roleIdList, void *userData)
{    
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Role Id List By Entitlement Id Response START ]================\n");
    if(!handleRequestResult(operationResult, resultSet, "gsbGetRoleEntitlementListByEntitlement"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=======================[Role Id List ]======================\n");
            for (i=0; i< roleIdList->mCount; i++)
            {
                wprintf(L"\tRoleId[%d] = %d\n",i, roleIdList->mRoleIds[i]);
            }
        }
    }
    wprintf(L"=============[ Get Role Id List By Entitlement Id END ]================\n");

    // Keep a deep copy of the entitlement list retrieved from the backend

    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestGetRoleEntitlementListByEntitlementId(GSBInstancePtr instance, gsi_u32 brigadeId, gsi_u32 entitlementId)
{
    wprintf(L"\n============== Testing Start: Get Role Id List By Entitlement Id ===========\n");
    wprintf(L"\tEntitlement Id    : %d\n", entitlementId);
    gsbGetRoleIdListByEntitlementId(instance, brigadeId, entitlementId, sampleTestGetRoleEntitlementListByEntitlementCallback, NULL);
    gNumOperations++;
    processTasks();
    wprintf(L"============== Testing End: Get Role Id List By Entitlement Id ===========\n");
}
///////////////////////////////////////////////////////////////////////////////
void sampleTestGetRoleEntitlementListByRoleIdCallback(GSResult operationResult, GSResultSet *resultSet, GSBEntitlementIdList *entitlementIdList, void *userData)
{    
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Entitlement Id List By Role Id Response START ]================\n");
    if(!handleRequestResult(operationResult, resultSet, "gsbGetRoleEntitlementListByRoleId"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=======================[Entitlement Id List ]======================\n");
            for (i=0; i< entitlementIdList->mCount; i++)
            {
                wprintf(L"\tEntitlementId[%d ] = %d\n",i, entitlementIdList->mEntitlementIds[i]);
            }
        }
    }
    wprintf(L"=============[ Get Entitlement Id List By Role Id END ]================\n");

    // Keep a deep copy of the entitlement list retrieved from the backend

    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestGetRoleEntitlementListByRoleId(GSBInstancePtr instance, gsi_u32 brigadeId, gsi_u32 roleId)
{
    wprintf(L"\n============== Testing Start: Get Entitlement Id List By Role Id ===========\n");
    wprintf(L"\tRole Id    : %d\n", roleId);
    gsbGetEntitlementIdListByRoleId(instance,brigadeId, roleId, sampleTestGetRoleEntitlementListByRoleIdCallback, NULL);
    gNumOperations++;
    processTasks();
    wprintf(L"=============[ Get Entitlement Id List By Role Id END ]================\n");
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestSendMessageToBrigadeCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    wprintf(L"=============[ Send Brigade Message Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbSendMessageToBrigade"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"Message sent to the Brigade successfully\n");
        }
    }

    wprintf(L"=============[ Send Brigade Message Response END ]================\n");
    GSI_UNUSED(userData);
     
}
//////////////////////////////////////////////////////////////////////////
void sampleTestSendMessageToBrigade(GSBInstancePtr gsbInstance, gsi_u32 brigadeId, UCS2String msg)
{
    gsbSendMessageToBrigade(gsbInstance, brigadeId, msg, sampleTestSendMessageToBrigadeCallback, NULL );
    gNumOperations++;
    processTasks();
}

///////////////////////////////////////////////////////////////////////////////
void sampleTestSendMessageToMemberCallback(GSResult operationResult, GSResultSet *resultSet, void * userData)
{
    wprintf(L"=============[ Send Member Message Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbSendMessageToMember"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"Message sent to the Brigade successfully\n");
        }
    }

    wprintf(L"=============[ Send Member Message Response END ]================\n");
    GSI_UNUSED(userData);
     
}
//////////////////////////////////////////////////////////////////////////
void sampleTestSendMessageToMember(GSBInstancePtr gsbInstance, gsi_u32 to, UCS2String msg)
{
    //TODO Use valid profile ids
    gsbSendMessageToMember(gsbInstance, to , msg, sampleTestSendMessageToMemberCallback, NULL );
    gNumOperations++;
    processTasks();
}

//////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadesByProfileIdCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigadeList *brigadeList, void * userData)
{
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Brigades By ProfileId Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbGetBrigadesByProfileId"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=====================[Brigade List ]==========================\n");
            for (i = 0; i < brigadeList->mCount; i++)
            {
                wprintf(L"=============================[Brigade %d ]==========================\n", (i+1));

                displayBrigade(&brigadeList->mBrigades[i]);
            }
        }
    }
    wprintf(L"=============[ Get Brigades By ProfileId Response END ]================\n");
    GSI_UNUSED(userData);
}
//////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadesByProfileId(GSBInstancePtr instance, gsi_u32 profileId)
{
    gsbGetBrigadesByProfileId(instance, profileId, sampleTestGetBrigadesByProfileIdCallback, NULL);
    gNumOperations++;
    processTasks();
}

//////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeHistoryCallback(GSResult operationResult, GSResultSet *resultSet, GSBBrigadeHistoryList *historyList, void * userData)
{
    gsi_u32 i = 0;
    wprintf(L"=============[ Get Brigades History Response START ]================\n");

    if(!handleRequestResult(operationResult, resultSet, "gsbGetBrigadeHistory"))
    {
        wprintf (L"FAILED \n");
        return;
    }
    else
    {   
        if (GS_SUCCEEDED(operationResult))
        {
            wprintf(L"=====================[History List ]==========================\n");
            for (i = 0; i < historyList->mCount; i++)
            {
                wprintf(L"=============================[History Entry %d ]==========================\n", (i+1));

                displayBrigadeHistory(&historyList->mBrigadeHistory[i]);
            }
        }
    }
    wprintf(L"=============[ Get Brigades History Response END ]================\n");
    GSI_UNUSED(userData);
     
}
//////////////////////////////////////////////////////////////////////////
void sampleTestGetBrigadeHistory(GSBInstancePtr gsbInstance, gsi_u32 brigadeId, gsi_u32 profileId, GSBBrigadeHistoryAccessLevel entryType)
{

    gsbGetBrigadeHistory(gsbInstance, brigadeId, profileId, entryType,  sampleTestGetBrigadeHistoryCallback, NULL);
    gNumOperations++;
    processTasks();
   
}

//////////////////////////////////////////////////////////////////////////
void sampleTestUpdateBrigade( GSBInstancePtr instance, GSBBrigade *brigade, UCS2String motd)
{
    wprintf(L"============== Testing Start: Update a Brigade ===========\n");
    //gsifree(brigade->mMessageOfTheDay);
    brigade->mMessageOfTheDay = goawstrdup(motd);
    sampleTestSaveBrigade(instance, brigade, gsi_true);
    wprintf(L"============== Testing End: Update a Brigade ===========\n");
}

//////////////////////////////////////////////////////////////////////////
void sampleTestGetMyPendingInvitesAndJoins(GSBInstancePtr instance)
{
	gsbGetMyPendingInvitesAndJoins(instance, sampleTestGetPendingInvitesAndJoinsCallback, NULL);
	gNumOperations++;
	processTasks();
}

//////////////////////////////////////////////////////////////////////////
// Assumes brigade 37 and 51 exists
int testScenario_rescindInvite()
{
	loginAsLeader();

	// Init test
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	if(gMemInBrigade == gsi_true)
		sampleTestRemoveBrigadeMember(gInstance, gTestPlayerProfileId, 165842);

	// +++ Start test +++

	// invite player
	sampleTestInviteToBrigade( gInstance, 165842, gTestPlayerProfileId );
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Invited);

	// leader invite other team
	sampleTestRescindInvite(gInstance, 165842, gTestPlayerProfileId);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Max - 1);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int testScenario_answerInvite(gsi_bool acceptInvite)
{
	loginAsLeader();

	// Init test
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	if(gMemInBrigade == gsi_true)
		sampleTestRemoveBrigadeMember(gInstance, gTestPlayerProfileId, 165842);

	// +++ Start test +++

	// invite player
	sampleTestInviteToBrigade( gInstance, 165842, gTestPlayerProfileId );
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Invited);

	// player either accepts or rejects invite
	loginAsMember();
	sampleTestAnswerInvite(gInstance, 165842, acceptInvite);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Max - 1);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int testScenario_AnswerJoin(gsi_bool acceptJoin)
{
	loginAsLeader();

	// Init test
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	if(gMemInBrigade == gsi_true)
		sampleTestRemoveBrigadeMember(gInstance, gTestPlayerProfileId, 165842);

	// +++ Start test +++
	// player makes request to join team
	loginAsMember();
	sampleTestJoinBrigade(gInstance, 165842);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_RequestJoin);

	// get pending list
	sampleTestGetMyPendingInvitesAndJoins(gInstance);

	// Leader either accepts/rejects Invite via acceptJoin bool
	loginAsLeader();

	sampleTestAnswerJoin(gInstance, 165842, gTestPlayerProfileId, acceptJoin);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Max - 1);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// Leader removes member, then reinvites member to Brigade. Member gets pending list and joins Brigade.
int testScenario_getPendingInvitesAndJoins()
{
	loginAsLeader();

	// Init test
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	if(gMemInBrigade == gsi_true)
		sampleTestRemoveBrigadeMember(gInstance, gTestPlayerProfileId, 165842);

	// +++ Start test +++

	// invite player
	sampleTestInviteToBrigade( gInstance, 165842, gTestPlayerProfileId );
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Invited);

	// player requests to join Brigade
	loginAsMember();
	sampleTestJoinBrigade(gInstance, 165842);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_RequestJoin);

	// get pending list
	sampleTestGetMyPendingInvitesAndJoins(gInstance);

	// Leader needs to confirm Invite
	loginAsLeader();

	sampleTestAnswerJoin(gInstance, 165842, gTestPlayerProfileId, gsi_true);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Max - 1);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// Assumes brigade 37
int testScenario_banMember()
{
	// Init test
	loginAsLeader();
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	if(gMemInBrigade == gsi_true)
		sampleTestRemoveBrigadeMember(gInstance, gTestPlayerProfileId, 165842);

	// player joins team 37
	loginAsMember();
	sampleTestJoinBrigade(gInstance, 165842);
	sampleTestGetBrigadeMemberList(gInstance, 37, GSBBrigadeMemberStatus_RequestJoin);

	// leader accepts join
	loginAsLeader();
	sampleTestAcceptJoin(gInstance, 165842, gTestPlayerProfileId);
	sampleTestGetBrigadeMemberList(gInstance, 37, GSBBrigadeMemberStatus_Active);

	// +++ Start test +++
	sampleTestBanMember(gInstance, 165842, gTestPlayerProfileId);
	sampleTestGetBrigadeMemberList(gInstance, 37, GSBBrigadeMemberStatus_Blocked);

	// see how the member like it
	loginAsMember();
	sampleTestGetBrigadeMemberList(gInstance, 37, GSBBrigadeMemberStatus_Max -1);

	return 1;
}


//////////////////////////////////////////////////////////////////////////
int test_main(int argc, char *argv[])
{
    
//	GSBBrigadeLogo *blogo;
    GSI_UNUSED(argc);
    GSI_UNUSED(argv);

    // Set debug output options
	gsSetDebugFile(stdout);
	gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All, GSIDebugLevel_Debug);
	
	// enable Win32 C Runtime debugging 
#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
{
	int tempFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(tempFlag | _CRTDBG_LEAK_CHECK_DF);
}
#endif

    // Perform the required availability check
	if (!isBackendAvailable())
        return cleanupAndQuit(-1);

	// TEST - Scenario for getPendingInvitesAndJoins (Same as testScenario_AnswerJoin)
	testScenario_getPendingInvitesAndJoins();

	// TEST - Get Entitlement Ids for this GameID
	sampleTestGetEntitlementListByGameId(gInstance);

	// TEST - Get Role ID that have the Remove Ban Entitlements (Should only be leader)
	sampleTestGetRoleEntitlementListByEntitlementId(gInstance, 165842, gDefaultEntitlementList->mEntitlements[0].mEntitlementId);

	// TEST - Get Role ID that have Accept/Decline Entitlements (Should only be leader and owner)
	sampleTestGetRoleEntitlementListByEntitlementId(gInstance, 165842, gDefaultEntitlementList->mEntitlements[4].mEntitlementId);

	// TEST - Get Entitlements By RoleId
	sampleTestGetRoleEntitlementListByRoleId(gInstance, 165842,  637195);

	// TEST - Leader rejects a request to join and accepts a request to join
	testScenario_AnswerJoin(gsi_false);
	testScenario_AnswerJoin(gsi_true);

	// TEST - Leader invites member to brigade, and member either accepts or rejects invitiation
 	testScenario_answerInvite(gsi_false);
 	testScenario_answerInvite(gsi_true);

	// TEST - Leader invites a member, but then decides to cancel that invite
	testScenario_rescindInvite();
	
	// TEST - Leader bans (need to figure out how to unban!)
//	testScenario_banMember();
    
    //TEST - Get History of a player in a brigade
    sampleTestGetBrigadeHistory( gInstance, 165842, 0, GSBBrigadeHistoryAccessLevel_Member);
    sampleTestGetBrigadeHistory( gInstance, 165842, 0, GSBBrigadeHistoryAccessLevel_Match);
    sampleTestGetBrigadeHistory( gInstance, 165842, 0, GSBBrigadeHistoryAccessLevel_Admin);
    sampleTestGetBrigadeHistory(gInstance, 165842, 0, GSBBrigadeHistoryAccessLevel_Leader);
    sampleTestGetBrigadeHistory(gInstance, 165842, 0, GSBBrigadeHistoryAccessLevel_Member | GSBBrigadeHistoryAccessLevel_Leader | GSBBrigadeHistoryAccessLevel_Admin | GSBBrigadeHistoryAccessLevel_Public);

	// TEST - Leaving a Brigade
	testScenario_answerInvite(gsi_true);

	loginAsMember();
	sampleTestLeaveBrigade(gInstance, 165842);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Max - 1);

    //
    // TEST - Get a Brigade give a brigade id
    sampleTestGetBrigadeById(gInstance, 165842);

    // TEST - Now get all the roles for the brigade
    sampleTestSearchBrigade(gInstance, L"Game"); 
    sampleTestGetRoleListByBrigadeId(gInstance, 165842);

    // TEST - Send Message to all the team members Test
    loginAsLeader();
	sampleTestSendMessageToBrigade(gInstance, 165842 , L"Hi Team!");

    // TEST - Search Message to a brigade member 
	loginAsMember();
    sampleTestSendMessageToMember(gInstance, 64880026 , L"Hey You!"); 

    // TEST - Search Brigade Test
    sampleTestSearchBrigade(gInstance, L"MWENG");

    // TEST - Search for Players
    sampleTestSearchPlayers(gInstance);

    // TEST - Update a Brigade with a new MOTD
    //        just change the MOTD for this test
	loginAsLeader();
    sampleTestGetBrigadeById(gInstance, 165842);
    sampleTestUpdateBrigade(gInstance, testBrigade, L"It's a wonderful world!");
	sampleTestGetBrigadeById(gInstance, 165842);

    // change it back to original state
	sampleTestGetBrigadesByProfileId(gInstance, 64880026);
	sampleTestGetBrigadeById(gInstance, 165842);
	sampleTestGetBrigadeMemberList(gInstance, 165842, GSBBrigadeMemberStatus_Active);
	sampleTestUpdateBrigade(gInstance, testBrigade, L"This is a test brigade");
	sampleTestGetBrigadesByProfileId(gInstance, 64880026);

    // TEST - Uploading a Brigade Logo (disabled in E2E test as well...)
// 	loginAsLeader();
// 	sampleTestGetBrigadeById(gInstance, 165842);
// 	sampleTestUploadBrigadeLogo(gInstance, _T("./Avatar.jpg") , 165842);
// 	sampleTestGetBrigadeById(gInstance, 165842);
// 	blogo = &testBrigade->mLogoList.mLogos[0];
// 	sampleTestDownloadBrigadeLogo(gInstance, "./Avatar2.jpg" ,&testBrigade->mLogoList.mLogos[0]);

    //TEST - Get History of a player in a brigade
	testScenario_AnswerJoin(gsi_true);
	loginAsMember();
    sampleTestGetBrigadeHistory( gInstance, 165842, 191134465, GSBBrigadeHistoryAccessLevel_Member);

    // TEST - Create a New Custom Role
	loginAsLeader();
	sampleTestCreateRole(gInstance);
    sampleTestGetRoleListByBrigadeId(gInstance, testBrigade->mBrigadeId);

    // TEST - Update Role Entitlements to new Role
    sampleTestUpdateRole( gInstance, gRoleId);
	sampleTestGetRoleListByBrigadeId(gInstance, testBrigade->mBrigadeId);

    // TEST - Now Remove a Role
    sampleTestRemoveRole(gInstance, testBrigade->mBrigadeId, gRoleId);
    sampleTestGetRoleListByBrigadeId(gInstance, testBrigade->mBrigadeId);

	// TEST - Create a new Brigade
	//     sampleTestCreateBrigade(gInstance, 
	//         L"GameDudes", 
	//         L"[PA]", 
	//         L"www.poweredbygamespy.com", 
	//         L"We rule!",
	//         GSBRecruitingStatus_Open);
	//     sampleTestGetBrigadeById(gInstance, gBrigadeToDisband);
	//     sampleTestCreateBrigade(gInstance, 
	//         L"mweng-3", 
	//         L"", 
	//         L"www.poweredbygamespy.com", 
	//         L"We rule!",
	//         GSBRecruitingStatus_Open);
	//     sampleTestDisbandBrigade(gInstance, gBrigadeToDisband );
	//     sampleTestSearchBrigade(gInstance, L"GameSpy");

 //   // TEST - Disband a Brigade
 //   sampleTestDisbandBrigade(gInstance, 165842);

 //   // TEST - Create a new Brigade
 //   sampleTestCreateBrigade(gInstance, 
 //                           L"GameSpy", 
 //                           L"[GSI]", 
 //                           L"www.poweredbygamespy.com", 
 //                           L"We rule!",
 //                           GSBRecruitingStatus_Open);

 
    // TEST #4 - Update a Brigade
    //myBrigade.mMOTD = "Eat a bowl of dicks!";
    //myBrigade.mRecruitingStatus = GSBRecruitingStatus_MODERATED;
    //myBrigade.mRecruitingType = GSBRecruitingType_CLOSED;
    //gsbUpdateBrigade(gInstance, &myBrigade, UpdateBrigadeCallback, NULL);
    //gNumOperations++;
    //processTasks();
	   
    // keep thinking
    printf("Finalizing any incomplete tasks\n");
    processTasks();

    // print out final results
    printResults();

    // Cleanup SDK
    return cleanupAndQuit(0);
}
