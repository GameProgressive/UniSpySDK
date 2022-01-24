///////////////////////////////////////////////////////////////////////////////
// File:	gptestc.c
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include <wchar.h>
#include "../gp.h"
#include "../../common/gsAvailable.h"

#if defined(_WIN32) && !defined(UNDER_CE)
	#include <conio.h>
#endif

#ifdef UNDER_CE
	void RetailOutputA(CHAR *tszErr, ...);
	#define printf RetailOutputA
#elif defined(_NITRO)
	#include "../../common/nitro/screen.h"
	#define printf Printf
	#define vprintf VPrintf
#endif

#if defined(_WIN32)
// Disable the warning about our while(1) statement.
#pragma warning(disable:4127)
#endif

#ifdef __MWERKS__	// CodeWarrior will warn if function is not prototyped.
int test_main(int argc, char **argv);
#endif 

#define GPTC_PRODUCTID 0
#define GPTC_GAMENAME   _T("gmtest")
#define GPTC_NICK1      _T("gptestc1")
#define GPTC_NICK2      _T("gptestc2")
#define GPTC_NICK3      _T("gptestc3")
#define GPTC_EMAIL1     _T("gptestc1@gptestc.com")
#define GPTC_EMAIL2     _T("gptestc2@gptestc.com")
#define GPTC_EMAIL3     _T("gptestc3@gptestc.com")
#define GPTC_PASSWORD   _T("gptestc")
#define GPTC_PID1       2957553 
#define GPTC_PID2       3052160

#define GPTC_PID3       118305038
#define GPTC_FIREWALL_OPTION   GP_FIREWALL

#define CHECK_GP_RESULT(func, errString) if(func != GP_NO_ERROR) { printf("%s\n", errString); totalErrors++; /*return 1;*/ }

GPConnection * pconn;
GPProfile other;
GPProfile otherBlock;
int otherIndex = -1;
int appState = -1;
gsi_bool receivedLastMessage = gsi_false;
gsi_bool gotStoodUp = gsi_false;
gsi_bool blockTesting = gsi_false;
int noComLineArgs;
int namespaceIds[GP_MAX_NAMESPACEIDS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
int totalErrors = 0;


#ifdef GSI_COMMON_DEBUG
	static void DebugCallback(GSIDebugCategory theCat, GSIDebugType theType,
	                          GSIDebugLevel theLevel, const char * theTokenStr,
	                          va_list theParamList)
	{
		GSI_UNUSED(theLevel);

		printf("[%s][%s] ", gGSIDebugCatStrings[theCat], gGSIDebugTypeStrings[theType]);

		vprintf(theTokenStr, theParamList);
	}
#endif

static void Error(GPConnection * pconnection, GPErrorArg * arg, void * param)
{
	gsi_char * errorCodeString;
	gsi_char * resultString;

#define RESULT(x) case x: resultString = (gsi_char *) _T(#x); break;
	switch(arg->result)
	{
	RESULT(GP_NO_ERROR)
	RESULT(GP_MEMORY_ERROR)
	RESULT(GP_PARAMETER_ERROR)
	RESULT(GP_NETWORK_ERROR)
	RESULT(GP_SERVER_ERROR)
	default:
		resultString = (gsi_char *) _T("Unknown result!\n");
	}

#define ERRORCODE(x) case x: errorCodeString = (gsi_char *) _T(#x); break;
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
		errorCodeString = (gsi_char *) _T("Unknown error code!\n");
	}

	if(arg->fatal)
	{
		printf( "-----------\n");
		printf( "FATAL ERROR\n");
		printf( "-----------\n");
	}
	else
	{
		printf( "-----\n");
		printf( "ERROR\n");
		printf( "-----\n");
	}

	_tprintf( _T("RESULT: %s (%d)\n"), resultString, arg->result);
	_tprintf( _T("ERROR CODE: %s (0x%X)\n"), errorCodeString, arg->errorCode);
	_tprintf( _T("ERROR STRING: %s\n"), arg->errorString);

	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

gsi_char whois[GP_NICK_LEN];
static void Whois(GPConnection * pconnection, GPGetInfoResponseArg * arg, void * param)
{
	if(arg->result == GP_NO_ERROR)
	{
		_tcscpy( GSI_WCHAR whois, GSI_WCHAR arg->nick );
	}
	else
		printf( "WHOIS FAILED\n");
	
	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

static void ConnectResponse(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
	if(arg->result == GP_NO_ERROR)
		printf("Connected\n");
	else
		printf( "CONNECT FAILED\n");

	gpSetStatus(pconnection, (GPEnum)GP_ONLINE, (gsi_char *) _T("Not Ready"), (gsi_char *) _T("gptestc"));

	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

static void ProfileSearchResponse(GPConnection * pconnection, GPProfileSearchResponseArg * arg, void * param)
{
	GPResult result;
	int i;
	if(arg->result == GP_NO_ERROR)
	{
		if(arg->numMatches > 0)
		{
			printf( "found %d match(es)\n", arg->numMatches );
			for(i = 0 ; i < arg->numMatches ; i++)
			{
				result = gpGetInfo(pconn, arg->matches[i].profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL);
				if(result != GP_NO_ERROR)
					printf("  gpGetInfo failed\n");
				else
					_tprintf(_T("  Found: %s\n"), GSI_WCHAR whois);

			}
		}
		else
			printf( "  NO MATCHES\n");
	}
	else
		printf( "  SEARCH FAILED\n");

	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

int msgCount = 0;
static void RecvBuddyMessage(GPConnection * pconnection, void * theArg, void * param)
{
	GPRecvBuddyMessageArg *arg;
	GPResult result;

	arg = (GPRecvBuddyMessageArg *)theArg;

	result = gpGetInfo(pconn, arg->profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL);
	if(result != GP_NO_ERROR)
		printf(" gpGetInfo failed\n");
	else 
		_tprintf( _T(" Received buddy message: %s : [%s]\n"), GSI_WCHAR whois, GSI_WCHAR arg->message);

	if (!(_tcscmp(arg->message, _T("5_Hello!")))) {
		receivedLastMessage = gsi_true;
	}
		
	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}


static void GetInfoResponse(GPConnection * pconnection, GPGetInfoResponseArg * arg, void * param)
{
	_tprintf(_T("First Name: %s \nLast Name: %s \nEmail: (%s)\n"), 
				arg->firstname, arg->lastname, arg->email);
	
	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

static void RecvBuddyStatus(GPConnection * connection, void * arg_, void * param)
{
	GPRecvBuddyStatusArg * arg = (GPRecvBuddyStatusArg *)arg_;
	GPBuddyStatus status;
	static char* statusToString[6] =
	{
		"GP_OFFLINE",
		"GP_ONLINE",
		"GP_PLAYING",
		"GP_STAGING",
		"GP_CHATTING",
		"GP_AWAY"
	};

	printf(" Buddy index: %d\n", arg->index);

	if (arg->profile == other) 
	{
		if (appState < 0)
			appState = 0;
		otherIndex = arg->index;
		gpGetBuddyStatus(connection, arg->index, &status);
		if (status.status == 1 && _tcscmp(status.statusString, _T("Ready")) && _tcscmp(status.statusString, _T("BlockTime"))) {
			if (appState < 1) {
				appState = 1;
				//printf("Buddy is online!\n");
				CHECK_GP_RESULT(gpSetStatus(pconn, (GPEnum) GP_ONLINE, (gsi_char *)_T("Ready"), (gsi_char *)_T("gptestc")), "gpSetStatus failed");
				//printf("Now wait until buddy is ready to message...\n");
			}
		}
		else if (status.status == 1 && !(_tcscmp(status.statusString, "Ready"))) {
			if (appState < 2) {
				appState = 2;
				//printf("Buddy is ready to message!\n");
				CHECK_GP_RESULT(gpSetStatus(pconn, (GPEnum) GP_ONLINE, (gsi_char *)_T("Ready"), (gsi_char *)_T("gptestc")), "gpSetStatus failed");
			}
		}
		else if (status.status == 1 && !(_tcscmp(status.statusString, "BlockTime"))) {
			if (appState < 3)
				appState = 3;
			//printf("Buddy is ready to block test!\n");
		}
	}
	printf(" ");
	gpGetInfo(connection, arg->profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)GetInfoResponse, NULL);

	gpGetBuddyStatus(connection, arg->index, &status);

	_tprintf(_T("  Status: ") GS_STR _T(", Status String: %s, Location String: %s, IP: %d, Port: %d\n"), 
			statusToString[status.status], status.statusString, status.locationString, status.ip, status.port);

	GSI_UNUSED(param);
}

static void RecvBuddyRequest(GPConnection * connection, void * arg_, void * param)
{
	GPRecvBuddyRequestArg * arg = (GPRecvBuddyRequestArg *)arg_;
	
	gsi_char buddy1[50];
	gsi_char buddy2[50];

	gpGetInfo(connection, arg->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL);
	
	_tprintf(_T("\nBuddy Request from %s\n"), whois);
#ifdef GSI_UNICODE
	_stprintf((wchar_t *)buddy1, sizeof(buddy1), _T("%s"), GPTC_NICK1);
	_stprintf((wchar_t *)buddy2, sizeof(buddy2), _T("%s"), GPTC_NICK2);
#else
	_stprintf(buddy1, "%s", GPTC_NICK1);
	_stprintf(buddy2, "%s", GPTC_NICK2);

#endif
	
	if (!_tcscmp(whois, buddy1) || !_tcscmp(whois, buddy2))
	{
		printf("Authorizing buddy request\n");
		gpAuthBuddyRequest(connection, arg->profile);
		CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
		if (blockTesting) {
			appState = 4;
			if (!noComLineArgs) {
				gpSendBuddyRequest(pconn, other, (gsi_char *) _T("testing"));
				CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
			}
		}
	}
	else
	{
		printf("Denying buddy request\n");
		gpDenyBuddyRequest(connection, arg->profile);
	}
	GSI_UNUSED(connection);
	GSI_UNUSED(param);
}

static void RecvBuddyRevoke(GPConnection * connection, void * arg_, void * param)
{
	printf("Buddy Revoke received...\n");

	if (appState == 2) {
		appState = 3;
	}
	
	GSI_UNUSED(connection);
	GSI_UNUSED(param);
	GSI_UNUSED(arg_);
}

#ifndef _PS3
static void printBlockedList()
{    
	int i=0;
	int numBlocked;
	int pid;

	gpGetNumBlocked(pconn, &numBlocked);
	printf("Get blocked list: num = %d\n", numBlocked);

	for (i=0; i<numBlocked; i++)
	{
		gpGetBlockedProfile(pconn, i, &pid);
		printf("[%d]: %d\n", i+1, pid);
		gpGetInfo(pconn, pid, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL);
	}
}
#endif

int test_main(int argc, char **argv)
{
#ifdef GSI_UNICODE
	//unsigned char ucMsg[] ={ 0xF0, 0x90, 0x90, 0xA4, 0}; //deseret char
   //unsigned char ucMsg[] = {0xD1, 0x81, 0xD0, 0xBB, 0xD1, 0x83, 0xD1, 0x88, 0xD0, 0xB0, 0xD1, 0x82, 0xD1, 0x8C, 0 }; //russian
	gsi_char ucMsg[] = { 0x0441, 0x043B, 0x0443, 0x0448,0x0430, 0x0442, 0x044C, 0 };
#else
	gsi_char ucMsg[] = "Hello";
#endif

	gsi_char * nick;
	gsi_char * email;
	gsi_char * password;
	gsi_char messageToSend[90];
	//wchar_t messageToSend[90];
	GPConnection connection;
	GSIACResult acResult = GSIACWaiting;
	int totalTime;
	int messagesSent;

#ifdef _PS3
	// Set communication id to string provided by Sony.
	SceNpCommunicationId communication_id = 
	{
		{'N','P','X','S','0','0','0','0','5'},
		'\0',
		0,
		0
	};
#endif

#ifdef GSI_COMMON_DEBUG
	// Define GSI_COMMON_DEBUG if you want to view the SDK debug output.
	// Set the SDK debug log file, or set your own handler using gsSetDebugCallback.
	//gsSetDebugFile(stdout); // output to console
	gsSetDebugCallback(DebugCallback);

	// Set some debug levels
	gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All, GSIDebugLevel_Debug);
	//gsSetDebugLevel(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Verbose);   // Show Detailed data on network traffic
	//gsSetDebugLevel(GSIDebugCat_App, GSIDebugType_All, GSIDebugLevel_Hardcore);  // Show All app comment
#endif

	GSI_UNUSED(argv);

	// check that the game's backend is available
	GSIStartAvailableCheck( (gsi_char *) GPTC_GAMENAME);
	while((acResult = GSIAvailableCheckThink()) == GSIACWaiting)
		msleep(5);
	if(acResult != GSIACAvailable)
	{
		printf("The backend is not available\n");
		return 1;
	}

	pconn = &connection;

	//argc = 2;

	if(argc == 1)
	{
		noComLineArgs = 1;
		nick = (gsi_char *) GPTC_NICK1;
		email = (gsi_char *) GPTC_EMAIL1;
	}
	else
	{
		noComLineArgs = 0;
		nick = (gsi_char *) GPTC_NICK2;
		email = (gsi_char *) GPTC_EMAIL2;
	}
	_tprintf(_T("Using email=%s, nick= %s\n"), email, nick);

	password = (gsi_char *) GPTC_PASSWORD;

	//INITIALIZE
	////////////
	CHECK_GP_RESULT(gpInitialize(pconn, GPTC_PRODUCTID, 0, GP_PARTNERID_GAMESPY), "gpInitialize failed");
	gpEnable(pconn, GP_INFO_CACHING_BUDDY_AND_BLOCK_ONLY);
	printf("\nInitialized\n");
	CHECK_GP_RESULT(gpSetCallback(pconn, GP_ERROR, (GPCallback)Error, NULL), "gpSetCallback failed");
	CHECK_GP_RESULT(gpSetCallback(pconn, GP_RECV_BUDDY_MESSAGE, &RecvBuddyMessage, NULL), "gpSetCallback failed");
	CHECK_GP_RESULT(gpSetCallback(pconn, GP_RECV_BUDDY_STATUS, &RecvBuddyStatus, NULL), "gpSetCallback failed");
	CHECK_GP_RESULT(gpSetCallback(pconn, GP_RECV_BUDDY_REQUEST, &RecvBuddyRequest, NULL), "gpSetCallback failed");
	CHECK_GP_RESULT(gpSetCallback(pconn, GP_RECV_BUDDY_REVOKE, &RecvBuddyRevoke, NULL), "gpSetCallback failed");

#ifdef _PS3
	// set the NP communication ID (provided by Sony)
	CHECK_GP_RESULT(gpSetNpCommunicationId(pconn, &communication_id), "gpSetNpCommunicationId failed");  

	// register the slot for GameSpy to use for the CellSysUtilCallback to prevent overlap
	// (0 is fine if you do not use this callback)
	CHECK_GP_RESULT(gpRegisterCellSysUtilCallbackSlot(pconn, 0), "gpRegisterCellSysUtilCallbackSlot failed");
#endif

	//CONNECT
	/////////
	printf("\nConnecting...\n");
	CHECK_GP_RESULT(gpConnect(pconn, nick, email, password, GPTC_FIREWALL_OPTION, GP_BLOCKING, (GPCallback)ConnectResponse, NULL), "gpConnect failed");
	
	//SEARCH (blocking)
	///////////////////
	printf("\nSearching (blocking)\n");

	printf(" Search on email (dan@gamespy.com)\n");
	CHECK_GP_RESULT(gpProfileSearch(pconn, (gsi_char *) _T(""), (gsi_char *) _T(""), 
									(gsi_char *) _T("dan@gamespy.com"), (gsi_char *) _T(""), 
									(gsi_char *) _T(""), 0,	GP_BLOCKING, 
									(GPCallback)ProfileSearchResponse, NULL), "gpProfileSearch failed");

	printf(" Search on nick ('crt')\n");
	CHECK_GP_RESULT(gpProfileSearch(pconn, (gsi_char *) _T("crt"), (gsi_char *) _T(""), (gsi_char *) _T(""), (gsi_char *) _T(""), 
									(gsi_char *) _T(""), 0, GP_BLOCKING, 
									(GPCallback)ProfileSearchResponse, NULL), "gpProfileSearch failed");

	printf(" Search on nick and email ('gptestc1', gptestc@gptestc.com)\n");
	CHECK_GP_RESULT(gpProfileSearch(pconn, (gsi_char *) GPTC_NICK1, (gsi_char *) _T(""), (gsi_char *) GPTC_EMAIL1, (gsi_char *) _T(""), 
									(gsi_char *) _T(""), 0, GP_BLOCKING, 
									(GPCallback)ProfileSearchResponse, NULL), "gpProfileSearch failed");

	printf(" Search on unique nick for different namespaces...\n");
	CHECK_GP_RESULT(gpProfileSearchUniquenick(pconn, (gsi_char *) GPTC_NICK1, namespaceIds, GP_MAX_NAMESPACEIDS, GP_BLOCKING, 
											  (GPCallback)ProfileSearchResponse, NULL),	"gpProfileSearchUniquenick failed" );

	if(noComLineArgs)
	{
		CHECK_GP_RESULT(gpProfileFromID(pconn, &other, GPTC_PID2), "gpProfileFromID failed");
	}
	else
	{
		CHECK_GP_RESULT(gpProfileFromID(pconn, &other, GPTC_PID1), "gpProfileFromID failed");
	}
	
	//GET INFO
	//////////
	printf("\nGetting Info\n");
	CHECK_GP_RESULT(gpGetInfo(pconn, GPTC_PID1, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)GetInfoResponse, NULL), "gpGetInfo failed");

//-----------------------------
#ifndef _PS3 // since buddy/block requests are not useable on PS3
	//BUDDY STUFF
	/////////

	printf("\nRetrieving buddy info (and processing any old buddy messages...)\n");
	totalTime = 0;
	while(appState < 2)
	{
		CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
		msleep(50);
		totalTime += 50;
		if (totalTime > 10000) 
		{
			if (appState == -1) // buddy is not actually on our buddy list 
			{ 
				if (noComLineArgs) 
					_tprintf( _T("%s"), GPTC_NICK2);
				else 
					_tprintf( _T("%s"), GPTC_NICK1);

				printf(" is not on our buddy list! Sending him a buddy request and waiting for a response...\n");

				gpSendBuddyRequest(pconn, other, (gsi_char *) _T("testing"));

				totalTime = 0;
				while (totalTime < 10000 && appState != 2) {
					CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
					msleep(50);
					totalTime += 50;
				}
				if (appState != 2)
				{
					if (noComLineArgs) 
						_tprintf(_T("\n%s never showed up =(\n"), (gsi_char *)GPTC_NICK2);
					else 
						_tprintf(_T("\n%s never showed up =(\n"), (gsi_char *)GPTC_NICK1);

					gotStoodUp = gsi_true;
					break;
				}
			}
			else // buddy is on our buddy list but did not come online and set status to "Ready"
			{
				if (noComLineArgs) 
					_tprintf(_T("\n%s never showed up =(\n"), (gsi_char *)GPTC_NICK2);
				else 
					_tprintf(_T("\n%s never showed up =(\n"), (gsi_char *)GPTC_NICK1);

				gotStoodUp = gsi_true;
				break;
			}
		}
	}

	if (!gotStoodUp) 
	{
		printf("\nSending messages to buddy (and receiving messages from him)\n");
		totalTime = 0;
		messagesSent = 0;
		while(messagesSent < 5)
		{
			CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
			msleep(50);
			totalTime += 50;
			if (totalTime % 1000 == 0) 
			{
#ifdef GSI_UNICODE
				_stprintf((wchar_t *)messageToSend, sizeof(messageToSend), _T("%d_%s!"), (int)(totalTime/1000), ucMsg);
#else
				_stprintf(messageToSend, _T("%d_%s!"), (int)(totalTime/1000), ucMsg );
#endif
				CHECK_GP_RESULT(gpSendBuddyMessage(pconn, other, (gsi_char *)messageToSend), "gpSendBuddyMessage failed");
				messagesSent++;
			}
		}

		while(!receivedLastMessage)
		{
			CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
			msleep(50);
		}

		CHECK_GP_RESULT(gpSetStatus(pconn, (GPEnum) GP_ONLINE, (gsi_char *)_T("BlockTime"), (gsi_char *)_T("gptestc")), "gpSetStatus failed");
	
		while(appState != 3)
		{
			CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
			msleep(50);
		}
	}
	else
		appState = 3;

	blockTesting = gsi_true;
	if (noComLineArgs) {
		// block buddy 
		if (gotStoodUp) {
			CHECK_GP_RESULT(gpProfileFromID(pconn, &other, GPTC_PID3), "gpProfileFromID failed");
			_tprintf(_T("\nBlocking %s\n"), (gsi_char *)GPTC_NICK3);
		}
		else
			_tprintf(_T("\nBlocking %s\n"), (gsi_char *)GPTC_NICK2);

		CHECK_GP_RESULT(gpAddToBlockedList(pconn, other), "gpAddToBlockedList failed");
		CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
		printBlockedList();

		if (!gotStoodUp) {
			_tprintf(_T("Checking if %s is on our buddy list...\n"), (gsi_char *)GPTC_NICK2);

			if (gpIsBuddy(pconn, other))
				printf("Yup.\n");
			else
				printf("Nope.\n");

			_tprintf(_T("Wait while %s tries to message us...\n"), (gsi_char *)GPTC_NICK2);

			totalTime = 0;
			while (totalTime < 5000) {
				CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
				msleep(50);
				totalTime += 50;
			}
		}

		_tprintf(_T("\nUnblocking %s\n"), (!gotStoodUp ? (gsi_char *)GPTC_NICK2 : (gsi_char *)GPTC_NICK3 ) );

		CHECK_GP_RESULT(gpRemoveFromBlockedList(pconn, other), "gpRemoveFromBlockedList");
		CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
		printBlockedList();

		CHECK_GP_RESULT(gpSetStatus(pconn, (GPEnum) GP_ONLINE, (gsi_char *)_T("DoneBlockTesting"), (gsi_char *)_T("gptestc")), "gpSetStatus failed");

		if (!gotStoodUp) {
			_tprintf(_T("Sending a buddy request to %s\n"), (gsi_char *)GPTC_NICK2);
			gpSendBuddyRequest(pconn, other, (gsi_char *)_T("testing"));

			printf("Waiting to get buddy requested back...\n");
			while (appState != 4) {
				CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
				msleep(50);
			}
		}
	}
	else {
		if (!gotStoodUp) {
			_tprintf(_T("Waiting for %s to block us...\n"), (gsi_char *)GPTC_NICK1);

			while (gpIsBuddy(pconn, other)) {
				CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
				msleep(50);
			}

			_tprintf(_T("We have been blocked. Trying to send buddy message to %s\n"), (gsi_char *)GPTC_NICK1);
			CHECK_GP_RESULT(gpSendBuddyMessage(pconn, other, (gsi_char *)_T("Why did you block me?")), "gpSendBuddyMessage failed");
			
			printf("Waiting to get unblocked and buddy requested...\n");
			while (appState != 4) {
				CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
				msleep(50);
			}
			CHECK_GP_RESULT(gpSetStatus(pconn, (GPEnum) GP_ONLINE, (gsi_char *)_T("DoneBlockTesting"), (gsi_char *)_T("gptestc")), "gpSetStatus failed");
		}
	} 
#else
// think a little while to make sure PS3 buddy sync finishes
_tprintf(_T("Buddy syncing...\n"));
while (totalTime < 5000 && appState != 2) {
	CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
	msleep(50);
	totalTime += 50;
}
_tprintf(_T("Done\n"));
GSI_UNUSED(messagesSent);
GSI_UNUSED(messageToSend);
GSI_UNUSED(ucMsg);
#endif

#if defined(_WIN32) && !defined(UNDER_CE)
	/*printf("\nDONE - Press any key to exit.\n\n");
	while (1)
	{
		CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
		msleep(10);
		if (_kbhit()) {
			break;
		}
	}
	*/
	msleep(2500); // Make sure everything finishes.
	CHECK_GP_RESULT(gpProcess(pconn), "gpProcess failed");
#endif

	printf("\nTotal Errors: %d\n\n", totalErrors);
	
	//DISCONNECT
	////////////
	gpDisconnect(pconn);
	printf("Disconnected\n");

	//DESTROY
	/////////
	gpDestroy(pconn);
	printf("Destroyed\n");
	return totalErrors;
//#endif
}
