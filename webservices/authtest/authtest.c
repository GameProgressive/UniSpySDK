///////////////////////////////////////////////////////////////////////////////
// File:	authtest.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../../common/gsCommon.h"
#include "../../common/gsCore.h"
#include "../../common/gsAvailable.h"
#include "../../GP/gp.h"
#include "../AuthService.h"

#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
#include <crtdbg.h>
#elif defined(_PS3)
    #include <np.h>
#elif defined(_PSP)
	#include <np/np.h>
	#include <np/np_auth.h>
	#include <np/np_service.h>
	#include <utility/utility_np_signin.h>
	#include <displaysvc.h>
	#include <libgu.h>
#endif

#ifdef UNDER_CE
void RetailOutputA(CHAR *tszErr, ...);
#define printf RetailOutputA
#elif defined(_NITRO)
#include "../../common/nitro/screen.h"
#define printf Printf
#define vprintf VPrintf
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Hard-coded info for login credentials
#define AUTHTEST_NICK               _T("sctest01")
#define AUTHTEST_EMAIL              _T("sctest@gamespy.com")
#define AUTHTEST_PASSWORD           _T("gspy")
#define AUTHTEST_TOKEN		        _T("GMTy13lsJmiY7L19ojyN3XTM08ll0C4EWWijwmJyq3ttiZmoDUQJ0OSnar9nQCu5MpOGvi4Z0EcC2uNaS4yKrUA+h+tTDDoJHF7ZjoWKOTj00yNOEdzWyG08cKdVQwFRkF+h8oG/Jd+Ik3sWviXq/+5bhZQ7iXxTbbDwNL6Lagp/pLZ9czLnYPhY7VEcoQlx9oO")
#define AUTHTEST_CHALLENGE  	    _T("LH8c.DLe")

// Type of login tests
#define AUTHTEST_LOGIN_PROFILE		0
#define AUTHTEST_LOGIN_UNIQUE		1
#define AUTHTEST_LOGIN_REMOTEAUTH	2

// GAME IDENTIFIERS - assigned by GameSpy
#define AUTHTEST_GAMENAME           _T("gmtest")
#define AUTHTEST_GAME_ID            0
#define SECRET_KEY	"HA6zkS"
#define ACCESS_KEY     "39cf9714e24f421c8ca07b9bcb36c0f5"

#define AUTHTEST_TIMEOUT            10000
#define BUDDYSYNC_DELAY             5000

#define GAMESPY_DEFAULT_SERVICE_ID	"WM0002-NPXM00002_00"
#define CHECK_GP_RESULT(func, errString) if(func != GP_NO_ERROR) { printf("%s\n", errString); exit(0); }

#ifdef _PSP
#define NP_AUTH_POOLSIZE (16 * 1024)
#define NP_SERVICE_POOLSIZE (16 * 1024)
#define NP_AUTH_STACKSIZE (15 * 1024)
#define NP_AUTH_TPL 40
#define NP_MANAGER_STACKSIZE (16 * 1024)
#define NP_MANAGER_TPL 40
#define LANG_SETTING    SCE_UTILITY_LANG_ENGLISH
#define BUTTON_SETTING  SCE_UTILITY_CTRL_ASSIGN_CROSS_IS_ENTER
#endif

// Globals
gsi_u32 gWaitCount = 0;
gsi_u32 gNumPassed = 0;
gsi_u32 gNumTests = 0;
char gRemoteAuthToken[GP_AUTHTOKEN_LEN];         
char gPartnerChallenge[GP_PARTNERCHALLENGE_LEN];


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/********
DEBUG OUTPUT
********/
#ifdef GSI_COMMON_DEBUG
    #if !defined(_MACOSX) && !defined(_IPHONE)
        static void DebugCallback(GSIDebugCategory theCat, GSIDebugType theType,
                                  GSIDebugLevel theLevel, const char * theTokenStr,
                                  va_list theParamList)
        {
            GSI_UNUSED(theLevel);
            printf("[%s][%s] ", 
                gGSIDebugCatStrings[theCat], 
                gGSIDebugTypeStrings[theType]);

            vprintf(theTokenStr, theParamList);
        }
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static gsi_bool isBackendAvailable()
{
    // Do Availability Check - Make sure backend is available
    /////////////////////////////////////////////////////////
    GSIACResult aResult = GSIACWaiting; 
    GSIStartAvailableCheck(AUTHTEST_GAMENAME);
    while(aResult == GSIACWaiting)
    {
        aResult = GSIAvailableCheckThink();
        msleep(5);
    }

    if (aResult == GSIACUnavailable)
    {
        printf("Online Services for AuthTest are no longer available\n");

        return gsi_false;
    }

    if (aResult == GSIACTemporarilyUnavailable)
    {
        printf("Online Services for AuthTest are temporarily down for maintenance\n");
        return gsi_false;
    }

    return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void myLoginCallback(GHTTPResult httpResult, WSLoginResponse * theResponse, void * theUserData)
{
    if (httpResult != GHTTPSuccess)
        printf("* %s * Failed on player login, HTTP error: %d\n", (char*)theUserData, httpResult);
    else if (theResponse->mLoginResult != WSLogin_Success)
        printf("* %s * Failed on player login, Login result: %d\n", (char*)theUserData, theResponse->mLoginResult);  
    else //Login worked!
    {
        _tprintf(_T("* %s * Player '%s' logged in.\n"), (char*)theUserData, theResponse->mCertificate.mUniqueNick);
        gNumPassed++;

        // validate cert
        if (wsLoginCertIsValid(&theResponse->mCertificate))
            printf("GSLoginCertificate is valid!\n");
        else
            printf("GSLoginCertificate is *INVALID*!\n");
    }

    gWaitCount--;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if !defined(_PS3) && !defined(_PSP)

static void myPlayerLogin(gsi_u8 logintype, const gsi_char * nick, const gsi_char * password)
{
    gsi_u32 result;

    switch(logintype)
    {
        case AUTHTEST_LOGIN_PROFILE:
            // login using default partnercode, namespaceid
            result = wsLoginProfile(AUTHTEST_GAME_ID, WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, 
                nick, AUTHTEST_EMAIL, password, _T(""), myLoginCallback, _T("wsLoginProfile"));
            break;

        case AUTHTEST_LOGIN_UNIQUE:
            // login using default partnercode, namespaceid
            result = wsLoginUnique(AUTHTEST_GAME_ID, WSLogin_PARTNERCODE_GAMESPY, WSLogin_NAMESPACE_SHARED_UNIQUE, 
                nick, password, _T(""), myLoginCallback, _T("wsLoginUnique"));
            break;

        case AUTHTEST_LOGIN_REMOTEAUTH:
            // -- *NOTE* the values passed in here for namespaceid/partnerid are ignored by
            // -- the AuthService for RemoteAuth logins. These values are obtained from the 
            // -- AuthToken itself.
			result = wsLoginRemoteAuth(AUTHTEST_GAME_ID, 0, 0, nick, password, myLoginCallback, _T("wsLoginRemoteAuth"));
            break;

        default:
            printf("Invalid login type\n");
            return;
    }

    gNumTests++;

    if (result != WSLogin_Success)
    {
        printf("Failed on login. Result: %d\n", result);
        return;
    }

    // wait for it to complete
    gWaitCount++;
    while (gWaitCount > 0)
    {
        msleep(10);
        gsCoreThink(0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Login tests for non-PS3. Executes each one in turn with pre-defined identifiers.
void RunLoginTests(int argc, char *argv[])
{
	wsSetGameCredentials(ACCESS_KEY, AUTHTEST_GAME_ID, SECRET_KEY);

    myPlayerLogin(AUTHTEST_LOGIN_REMOTEAUTH, AUTHTEST_TOKEN, AUTHTEST_CHALLENGE);
    myPlayerLogin(AUTHTEST_LOGIN_UNIQUE, AUTHTEST_NICK, AUTHTEST_PASSWORD);
    myPlayerLogin(AUTHTEST_LOGIN_PROFILE, AUTHTEST_NICK, AUTHTEST_PASSWORD);

    GSI_UNUSED(argc);
    GSI_UNUSED(argv);
}

#else //#if _PS3 || _PSP

// PS3/PSP specific globals
#if defined(_PS3)
uint8_t	np_pool[SCE_NP_MIN_POOL_SIZE];
#endif
gsi_bool gGotTicket;
gsi_bool gNpInitialized;

#if defined(_PSP)
static SceUtilityNetconfParam netconf_param;
static unsigned char *ticket=NULL;	// Buffer for getting ticket
static int ticket_len = 0;		// Ticket size

// used for PSP graphics library initialization
static char disp_list[0x10000] __attribute((aligned(64)));


// terminate PSP graphics library
gsiPspGuTerm()
{
	sceGuTerm();
}

// load PSP graphics library stuff for NP sign-in GUI
gsiPspGuInit()
{
	sceGuInit();

	sceGuStart(SCEGU_IMMEDIATE, disp_list, sizeof(disp_list));
	sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
	sceGuDispBuffer(SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP32_1,
	    SCEGU_VRAM_WIDTH);
	sceGuDepthBuffer(SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);

	sceGuOffset(0, 0);

	sceGuScissor(0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
	sceGuEnable(SCEGU_SCISSOR_TEST);

	sceGuBlendFunc(SCEGU_ADD, SCEGU_SRC_ALPHA, SCEGU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuEnable(SCEGU_BLEND);

	sceGuTexFunc(SCEGU_TEX_MODULATE, SCEGU_RGBA);
	sceGuTexFilter(SCEGU_LINEAR_MIPMAP_LINEAR, SCEGU_NEAREST);
	sceGuTexWrap(SCEGU_REPEAT, SCEGU_REPEAT);

	sceGuClearColor(0);
	sceGuClearDepth(0);
	sceGuClearStencil(0);

	sceGuFinish();
	sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);
	sceDisplayWaitVblankStart();

	sceGuDisplay(SCEGU_DISPLAY_ON);
}
#endif

///////////////////////////////////////////////////////////////////////////////
gsi_bool initializeNp()
{
    int ret = 0;
    int status = 0;
    int oldStatus = 0;
    gsi_time startTime = current_time();

#if defined(_PSP)
	// Initialize NP
	ret = sceNpInit();
	if(ret < 0){
		printf("sceNpInit() failed. ret = 0x%x\n", ret);
		return gsi_false;
		//normally terminate NP modules here
	}

	ret = sceNpAuthInit(NP_AUTH_POOLSIZE, NP_AUTH_STACKSIZE, NP_AUTH_TPL);
	if (ret < 0) {
		printf("sceNpAuthInit() failed. ret = 0x%x\n", ret);
		return gsi_false;
		//normally terminate NP modules here
	}

	ret = sceNpServiceInit(NP_SERVICE_POOLSIZE, NP_MANAGER_STACKSIZE,
	    NP_MANAGER_TPL);
	if(ret < 0){
		printf("sceNpServiceInit() failed. ret = 0x%x\n", ret);
		return gsi_false;
		//normally terminate NP modules here
	}

	gNpInitialized = gsi_true;

#elif defined(_PS3) 
    // Initial NP init
    ret = sceNpInit(SCE_NP_MIN_POOL_SIZE, np_pool);

    if (ret < 0) 
    {
        printf("sceNpInit() failed. ret = 0x%x\n", ret);
        return gsi_false;
    }

    gNpInitialized = gsi_true;

    // wait for online status before continuing
    while (1) 
    {
        ret = sceNpManagerGetStatus(&status);

        if (ret < 0)
        {
            printf("sceNpGetStatus() failed. ret = 0x%x\n", ret);
            return gsi_false;
        }
        
        // break once we have gotten online status
        if (status == SCE_NP_MANAGER_STATUS_ONLINE)
            break;

        // let this loop timeout if it takes too long to obtain status
        if ((current_time()-startTime) > AUTHTEST_TIMEOUT)
        {
            printf("sceNpGetStatus() timed out.\n");
            return gsi_false;
        }

        // update user of change in status updates
        if (status > oldStatus)
            printf("sceNpGetStatus - status = %d\n", status);
        
        oldStatus = status;
        msleep(10);
    }
#endif
    return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
void cleanupNP()
{
    sceNpTerm();
}

#if defined(_PS3)
///////////////////////////////////////////////////////////////////////////////
static void npManagerCallback(int event, int result, void *arg)
{
    switch (event) 
    {
        case SCE_NP_MANAGER_EVENT_GOT_TICKET:           
            printf("Got Ticket event!\n");

            if (result < 0) 
                printf("ticket error 0x%x\n", result);
            else
                gGotTicket = gsi_true;

            break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
gsi_bool retrieveNpTicket(const char * serviceId, gsi_u8 ** ticketData, size_t * ticketSize)
{
    int ret = 0; 
    gsi_time startTime;
    SceNpId npId;
    SceNpTicketVersion ticketVersion;

    // get the current user's NpId
    ret = sceNpManagerGetNpId(&npId);
    if (ret < 0) 
    {
        printf("sceNpManagerGetNpId() failed. ret = 0x%x\n", ret);
        return gsi_false;
    }

    // register the NP Manager callback
    ret = sceNpManagerRegisterCallback(npManagerCallback, NULL);
    if (ret < 0) 
    {
        printf("sceNpManagerRegisterCallback() failed. ret = 0x%x\n", ret);
        return gsi_false;
    }

    // start the ticket request
    ticketVersion.major = 3;
    ticketVersion.minor = 0;
    ret = sceNpManagerRequestTicket2(&npId, &ticketVersion, serviceId, NULL, 0, NULL, 0);
    if (ret < 0) 
    {
        printf("sceNpManagerRequestTicket() failed. ret = 0x%x\n", ret);
        return gsi_false;
    }

    // wait until ticket is retrieved
    startTime = current_time();
    while (!gGotTicket)
    {
        // you have to Poll for callbacks
        cellSysutilCheckCallback();

        // let this loop timeout if it takes too long
        if ((current_time()-startTime) > AUTHTEST_TIMEOUT)
        {
            printf("NP Ticket Request timed out.\n");
            return gsi_false;
        }
        msleep(10);
    }

    // retrieve the ticket size
    ret = sceNpManagerGetTicket(NULL, ticketSize);
    if (ret < 0) {
        printf("sceNpManagerGetTicket() error 0x%x\n", ret);
        return gsi_false;
    }

    // allocate space for ticket
    *ticketData = (gsi_u8 *)gsimalloc(*ticketSize);
    if (*ticketData == NULL) 
    {
        printf("failed to allocate space for ticket - out of memory!\n");
        return gsi_false;
    }

    // retrieve the ticket data
    ret = sceNpManagerGetTicket(*ticketData, ticketSize);
    if (ret < 0) 
        printf("sceNpManagerGetTicket() failed. ret = 0x%x\n", ret);
    else 
    {
        // store ticket to file?
        /*FILE *fd = fopen(SYS_APP_HOME "/ticket.dat", "w");
        ret = fwrite(ticketData, *ticketSize, 1, fd);
        fclose(fd);
        printf("Wrote " SYS_APP_HOME "/ticket.dat (%d)\n", ret);*/
    }

    // free up callback
    sceNpManagerUnregisterCallback();

    return gsi_true;
}
#elif defined(_PSP)
static int auth_callback(SceNpAuthRequestId id, int result, void *arg)
{
	int ret = 0;

	if(result < 0){
		printf("auth_callback called with error result: %d\n", result);
		return 0;
	}

	ticket_len = result;
	ticket = (SceUChar8 *)malloc(ticket_len);
	if(ticket == NULL){
		printf("%s: could not malloc ticket space.\n", __FUNCTION__);
		return 0;
	}

	ret = sceNpAuthGetTicket(id, ticket, ticket_len);
	if(ret < 0){
		printf("sceNpAuthGetTicket() failed. ret = 0x%x\n", ret);
		return 0;
	}

	gGotTicket = gsi_true;
	return 0;
}

gsi_bool retrieveNpTicket(const char * serviceId, gsi_u8 ** ticketData, size_t * ticketSize)
{
    int ret, status = 0; 
    gsi_time startTime;
    SceNpId npId;
    SceNpTicketVersion ticketVersion;

	gsi_bool getTicketDone;
	SceNpAuthRequestParameter param;


	// need to sign in to np cause stupid PSP won't do it automatically
	SceBool npSigninDone = SCE_FALSE;
	SceUtilityNpSigninParam npsignin_param;

	memset(&npsignin_param, 0x00, sizeof(SceUtilityNpSigninParam));
	npsignin_param.base.size = sizeof(SceUtilityNpSigninParam);
	npsignin_param.base.message_lang = LANG_SETTING;
	npsignin_param.base.ctrl_assign = BUTTON_SETTING;
	npsignin_param.base.sound_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY;
	npsignin_param.base.main_thread_priority  = SCE_KERNEL_USER_HIGHEST_PRIORITY + 1;
	npsignin_param.base.font_thread_priority  = SCE_KERNEL_USER_HIGHEST_PRIORITY + 2;
	npsignin_param.base.sub_thread_priority   = SCE_KERNEL_USER_HIGHEST_PRIORITY + 3;

	ret = sceUtilityNpSigninInitStart(&npsignin_param);
	if(ret < 0){
		printf( "sceUtilityNpSigninInitStart() failed. ret = 0x%x\n", ret);
		//goto error;
	}

	while (!npSigninDone)
	{
		char buf[256];

		sceGuStart(SCEGU_IMMEDIATE, disp_list, sizeof(disp_list));
		sceGuClear(SCEGU_CLEAR_ALL);

		sceGuFinish();
		sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

		status = sceUtilityNpSigninGetStatus();
		switch(status) {
			case SCE_UTILITY_COMMON_STATUS_INITIALIZE:
				break;
			case SCE_UTILITY_COMMON_STATUS_RUNNING:
				ret = sceUtilityNpSigninUpdate(1);
				if(ret < 0)
					printf("sceUtilityNpSigninUpdate() failed. ret = 0x%x\n", ret);
				break;
			case SCE_UTILITY_COMMON_STATUS_FINISHED:
				ret = sceUtilityNpSigninShutdownStart();
				if(ret < 0)
					printf("sceUtilityNpSigninShutdownStart() failed. ret = 0x%x\n", ret);
				break;
			case SCE_UTILITY_COMMON_STATUS_SHUTDOWN:
				break;
			case SCE_UTILITY_COMMON_STATUS_NONE:
				npSigninDone = SCE_TRUE;
				break;
			default:
				break;
		}
	}
	
	SceNpOnlineId onlineId;

	ret = sceNpGetOnlineId(&onlineId);
	if(ret < 0){
		// Error handling
	}
	printf("OnlineId: %s\n", onlineId.data);


	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.version.major = SCE_NP_AUTH_LATEST_TICKET_VERSION_MAJOR;
	param.version.minor = SCE_NP_AUTH_LATEST_TICKET_VERSION_MINOR;
	param.serviceId = serviceId;
	param.ticketCb = auth_callback;

	getTicketDone = SCE_FALSE;

	ret = sceNpAuthCreateStartRequest(&param);
	if(ret < 0){
		printf("sceNpAuthCreateStartRequest() failed. ret = 0x%x\n", ret);
		return gsi_false;
	}

	memset(&netconf_param, 0x00, sizeof(SceUtilityNetconfParam));
	netconf_param.base.size = sizeof(SceUtilityNetconfParam);
	netconf_param.base.message_lang = LANG_SETTING;
	netconf_param.base.ctrl_assign = BUTTON_SETTING;
	netconf_param.base.sound_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY;
	netconf_param.base.main_thread_priority  = SCE_KERNEL_USER_HIGHEST_PRIORITY + 1;
	netconf_param.base.font_thread_priority  = SCE_KERNEL_USER_HIGHEST_PRIORITY + 2;
	netconf_param.base.sub_thread_priority   = SCE_KERNEL_USER_HIGHEST_PRIORITY + 3;
	netconf_param.type = SCE_UTILITY_NETCONF_TYPE_RECONNECT_APNET;
	netconf_param.browser_available = SCE_UTILITY_NETCONF_BROWSER_UNAVAILABLE;

	ret = sceUtilityNetconfInitStart(&netconf_param);
	if(ret < 0){
		printf( "sceUtilityNetconfInitStart() failed. ret = 0x%x\n", ret);
	}

	startTime = current_time();
	while(!gGotTicket)
	{
		status = sceUtilityNetconfGetStatus();
		if (status == SCE_UTILITY_COMMON_STATUS_RUNNING) 
		{
			ret = sceUtilityNetconfUpdate(1);
			if(ret < 0){
				printf("sceUtilityNetconfUpdate() failed. ret = 0x%x\n", ret);
				return gsi_false;
			}
		}

		// let this loop timeout if it takes too long
        if ((current_time()-startTime) > AUTHTEST_TIMEOUT)
        {
            printf("NP Ticket Request timed out.\n");
            return gsi_false;
        }
        msleep(10);
	}

	if (!gGotTicket) 
	{
		printf("AuthCallback was not called successfully\n");
		return gsi_false;
	}

	*ticketData = ticket;
	*ticketSize = (size_t) ticket_len;

	// cleanup here?
    return gsi_true;
}
#endif 

// Login tests for PS3 only
///////////////////////////////////////////////////////////////////////////////
static void mySonyLoginCallback(GHTTPResult httpResult, WSLoginSonyCertResponse * theResponse, void * theUserData)
{
    gsi_u32 result;
    
    if (httpResult != GHTTPSuccess)
        printf("* %s * Failed on player login, HTTP error: %d\r\n", (char*)theUserData, httpResult);
    else if (theResponse->mLoginResult != WSLogin_Success)
        printf("* %s * Failed on player login, Login result: %d\r\n", (char*)theUserData, theResponse->mLoginResult);
    else //Login worked!
    {
        printf("* %s * succeeded, now attempting login...\n", (char*)theUserData);
        gNumPassed++;
        gNumTests++;

        // copy login creditentials for GP buddy sync test
        memcpy(&gRemoteAuthToken, &theResponse->mRemoteAuthToken, sizeof(theResponse->mRemoteAuthToken));
        memcpy(&gPartnerChallenge, &theResponse->mPartnerChallenge, sizeof(theResponse->mPartnerChallenge));

         // Now that we have the AuthToken/Challenge, login with these creditentials.
        // -- *NOTE* the values passed in here for namespaceid/partnerid are ignored by
        // -- the AuthService for RemoteAuth logins. These values are obtained from the 
        // -- AuthToken itself.
        
		// shutdown the core object then reinitialize 
		/*gsCoreShutdown();
		while (gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
		{
			gsCoreThink(0);
			msleep(10);
		}
		gsCoreInitialize();

		wsSetGameCredentials(ACCESS_KEY, AUTHTEST_GAME_ID, SECRET_KEY);*/

		result = wsLoginRemoteAuth(0, 0, 0, theResponse->mRemoteAuthToken, 
            theResponse->mPartnerChallenge, myLoginCallback, _T("wsLoginRemoteAuth"));

        if (result != WSLogin_Success)
            printf("Failed on wsLoginRemoteAuth. Result: %d\n", result);        
    }
}

///////////////////////////////////////////////////////////////////////////////
static void printHelp()
{
    printf("Standard AuthTest usage for PS3 (to use your own Service ID):\n");
    printf("> authtest <Sony Service ID>\n\n");
    printf("Defaulting to use GameSpy Test Service ID: %s...\n\n", GAMESPY_DEFAULT_SERVICE_ID);
}


///////////////////////////////////////////////////////////////////////////////
static void ConnectResponse(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
    if(arg->result == GP_NO_ERROR)
        printf("Connected to GP\n");
    else
        printf("GP Connection Attempt Failed\n");

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

//GP callbacks, everything is a noop except for the error callback
static void Error(GPConnection * pconnection, GPErrorArg * arg, void * param)
{
    gsi_char * errorCodeString;
    gsi_char * resultString;

#define RESULT(x) case x: resultString = _T(#x); break;
    switch(arg->result)
    {
        RESULT(GP_NO_ERROR)
            RESULT(GP_MEMORY_ERROR)
            RESULT(GP_PARAMETER_ERROR)
            RESULT(GP_NETWORK_ERROR)
            RESULT(GP_SERVER_ERROR)
    default:
        resultString = _T("Unknown result!\n");
    }

#define ERRORCODE(x) case x: errorCodeString = _T(#x); break;
    switch(arg->errorCode)
    {
        ERRORCODE(GP_GENERAL)
            ERRORCODE(GP_PARSE)
            ERRORCODE(GP_NOT_LOGGED_IN)
            ERRORCODE(GP_BAD_SESSKEY)
            ERRORCODE(GP_DATABASE)
            ERRORCODE(GP_NETWORK)
            ERRORCODE(GP_FORCED_DISCONNECT)
            ERRORCODE(GP_CONNECTION_CLOSED)
            ERRORCODE(GP_LOGIN)
            ERRORCODE(GP_LOGIN_TIMEOUT)
            ERRORCODE(GP_LOGIN_BAD_NICK)
            ERRORCODE(GP_LOGIN_BAD_EMAIL)
            ERRORCODE(GP_LOGIN_BAD_PASSWORD)
            ERRORCODE(GP_LOGIN_BAD_PROFILE)
            ERRORCODE(GP_LOGIN_PROFILE_DELETED)
            ERRORCODE(GP_LOGIN_CONNECTION_FAILED)
            ERRORCODE(GP_LOGIN_SERVER_AUTH_FAILED)
            ERRORCODE(GP_NEWUSER)
            ERRORCODE(GP_NEWUSER_BAD_NICK)
            ERRORCODE(GP_NEWUSER_BAD_PASSWORD)
            ERRORCODE(GP_UPDATEUI)
            ERRORCODE(GP_UPDATEUI_BAD_EMAIL)
            ERRORCODE(GP_NEWPROFILE)
            ERRORCODE(GP_NEWPROFILE_BAD_NICK)
            ERRORCODE(GP_NEWPROFILE_BAD_OLD_NICK)
            ERRORCODE(GP_UPDATEPRO)
            ERRORCODE(GP_UPDATEPRO_BAD_NICK)
            ERRORCODE(GP_ADDBUDDY)
            ERRORCODE(GP_ADDBUDDY_BAD_FROM)
            ERRORCODE(GP_ADDBUDDY_BAD_NEW)
            ERRORCODE(GP_ADDBUDDY_ALREADY_BUDDY)
            ERRORCODE(GP_AUTHADD)
            ERRORCODE(GP_AUTHADD_BAD_FROM)
            ERRORCODE(GP_AUTHADD_BAD_SIG)
            ERRORCODE(GP_STATUS)
            ERRORCODE(GP_BM)
            ERRORCODE(GP_BM_NOT_BUDDY)
            ERRORCODE(GP_GETPROFILE)
            ERRORCODE(GP_GETPROFILE_BAD_PROFILE)
            ERRORCODE(GP_DELBUDDY)
            ERRORCODE(GP_DELBUDDY_NOT_BUDDY)
            ERRORCODE(GP_DELPROFILE)
            ERRORCODE(GP_DELPROFILE_LAST_PROFILE)
            ERRORCODE(GP_SEARCH)
            ERRORCODE(GP_SEARCH_CONNECTION_FAILED)
    default:
        errorCodeString = _T("Unknown error code!\n");
    }

    if(arg->fatal)
    {
        printf( "-----------\n");
        printf( "GP FATAL ERROR\n");
        printf( "-----------\n");
    }
    else
    {
        printf( "-----\n");
        printf( "GP ERROR\n");
        printf( "-----\n");
    }
    _tprintf( _T("RESULT: %s (%d)\n"), resultString, arg->result);
    _tprintf( _T("ERROR CODE: %s (0x%X)\n"), errorCodeString, arg->errorCode);
    _tprintf( _T("ERROR STRING: %s\n"), arg->errorString);

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void RecvBuddyRequest(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}


static void RecvBuddyStatus(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void RecvBuddyMessage(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void RecvGameInvite(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}


static void TransferCallback(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void RecvBuddyAuth(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void RecvBuddyRevoke(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    GSI_UNUSED(arg);
    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

///////////////////////////////////////////////////////////////////////////////
static void RunBuddySyncTest()
{
    GPConnection conn;
    gsi_time timer;

#ifdef _PS3
	// set communication id to string provided by Sony
	SceNpCommunicationId communication_id = 
	{
		{'N','P','X','S','0','0','0','0','5'},
		'\0',
		0,
		0
	};
#endif

    // initialize GP
    printf("Initializing GP to test PS3 Buddy Sync...\n");
    CHECK_GP_RESULT(gpInitialize(&conn, 0, 0, 0), "gpInitialize failed");

    // setup callbacks as No-ops for testing purposes
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_ERROR, (GPCallback)Error, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_BUDDY_REQUEST, (GPCallback)RecvBuddyRequest, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_BUDDY_STATUS, (GPCallback)RecvBuddyStatus, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_BUDDY_MESSAGE, (GPCallback)RecvBuddyMessage, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_GAME_INVITE,	(GPCallback)RecvGameInvite, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_TRANSFER_CALLBACK, (GPCallback)TransferCallback, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_BUDDY_AUTH, (GPCallback)RecvBuddyAuth, NULL), "gpSetCallback failed");
    CHECK_GP_RESULT(gpSetCallback(&conn, GP_RECV_BUDDY_REVOKE, (GPCallback)RecvBuddyRevoke, NULL), "gpSetCallback failed");

#ifdef _PS3
	// set the NP communication ID (provided by Sony)
	CHECK_GP_RESULT(gpSetNpCommunicationId(&conn, &communication_id), "gpSetNpCommunicationId failed");  

	// register the slot for GameSpy to use for the CellSysUtilCallback to prevent overlap
	// (0 is fine if you do not use this callback)
	CHECK_GP_RESULT(gpRegisterCellSysUtilCallbackSlot(&conn, 0), "gpRegisterCellSysUtilCallbackSlot failed");
#endif

    // connect to GP
    printf("Connecting to GP...\n");
    CHECK_GP_RESULT(gpConnectPreAuthenticated(&conn, gRemoteAuthToken, gPartnerChallenge, GP_NO_FIREWALL, GP_BLOCKING, (GPCallback)ConnectResponse, NULL), "gpConnect failed");

    // process while we wait for sync
    printf("Connected, now wait %d seconds for sync to complete (on PS3)...\n", (BUDDYSYNC_DELAY/1000));
    timer = current_time();
    while ((current_time() - timer) < BUDDYSYNC_DELAY)
    {
        CHECK_GP_RESULT(gpProcess(&conn), "gpProcess failed");
        msleep(20);
    }

    // Disconnect from GP
    gpDisconnect(&conn);
    printf("Disconnected from GP\n");

    // Destroy GP
    gpDestroy(&conn);
    printf("Destroyed GP\n");
}

///////////////////////////////////////////////////////////////////////////////
void RunLoginTests(int argc, char *argv[])
{
    gsi_u32 result;
    size_t ticketSize;
    gsi_u8 * npTicket;

	wsSetGameCredentials(ACCESS_KEY, AUTHTEST_GAME_ID, SECRET_KEY);

#ifdef _PSP
	// load PSP graphics library stuff for NP sign-in GUI
	gsiPspGuInit();
#endif

    // init NP
    result = initializeNp();
                
    if (gNpInitialized)
    {
		if (argc > 1)
		{
            // grab the Sony Service ID from input args, then grab NP ticket
			result = retrieveNpTicket(argv[1], &npTicket, &ticketSize);
		}
		else
		{
            // else default to GameSpy Test Service ID
			printHelp();
			result = retrieveNpTicket(GAMESPY_DEFAULT_SERVICE_ID, &npTicket, &ticketSize);
		}
        
        // cleanup the NP stuff as it is no longer needed
        cleanupNP();

#ifdef _PSP
		// terminate PSP graphics library
		gsiPspGuTerm();
#endif
    }

    if (!result)
        return;
    

    // Now that we have the NP Ticket, validate it to get remote auth creditentials.
    // -- *NOTE* the values passed in here for namespaceid/partnercode are ignored by
    // -- the AuthService for PS3 Cert logins. These values are obtained from the 
    // -- NP Ticket itself.
    result = wsLoginSonyCert(AUTHTEST_GAME_ID, 0, 0, npTicket, ticketSize, mySonyLoginCallback, "wsLoginPs3Cert");

    if (result != WSLogin_Success)
    {
        printf("Failed on PS3 Cert. Result: %d\r\n", result);
        return;
    }

    gNumTests++;

    // wait for it to complete
    gWaitCount++;
    while (gWaitCount > 0)
    {
        msleep(10);
        gsCoreThink(0);
    }

    // free up ticket mem
    if (npTicket)
        gsifree(npTicket);

    // Login to GP to test Buddy Sync
    RunBuddySyncTest();
}

#endif //!_PS3

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PrintResults()
{
    printf("> --------------------------------\n");
    printf("> FINISHED: %d/%d PASSED\n", gNumPassed, gNumTests);
    printf("> --------------------------------\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int test_main(int argc, char *argv[])
{	
    // for debug output on these platforms
#if defined (_PS3) || defined (_PS2) || defined (_PSP) || defined(_NITRO)
    #ifdef GSI_COMMON_DEBUG
        // Define GSI_COMMON_DEBUG if you want to view the SDK debug output
        // Set the SDK debug log file, or set your own handler using gsSetDebugCallback
        //gsSetDebugFile(stdout); // output to console
        gsSetDebugCallback(DebugCallback);

        // Set debug levels
        gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All, GSIDebugLevel_Normal);
    #endif
#endif

    // enable Win32 C Runtime debugging 
#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG)
    {
        int tempFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        _CrtSetDbgFlag(tempFlag | _CRTDBG_LEAK_CHECK_DF);
    }
#endif

    // Check backend availability
    if (!isBackendAvailable())
        return -1;

    // Initialize SDK core/common objects for the auth service and 
    gsCoreInitialize();

    // Run the tests
    RunLoginTests(argc, argv);  

    // Print results of the test
    PrintResults();

    // shutdown core before quitting
    gsCoreShutdown();

    // Wait for core shutdown 
    //   (should be instantaneous unless you have multiple cores)
    while(gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
    {
        gsCoreThink(0);
        msleep(5);
    }

#if defined(_WIN32) && !defined(_XBOX)
    fflush(stderr);
    printf("Done - Press Enter\r\n"); 
    fflush(stdout);
    getc(stdin);
#else
    printf("Done.\r\n"); 
#endif

    return 0;
}
