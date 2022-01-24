///////////////////////////////////////////////////////////////////////////////
// File:	gcdkeyservertest_qr2.c
// SDK:		GameSpy CD Key SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ------------------------------------
// This sample is similar to the gcdkeyservertest sample; it also
// demonstrates support for Query & Reporting 2 SDK integration, as detailed 
// in the documentation.
// 
// Please see the GameSpy CD Key SDK documentation for more information.

#include "../gcdkeys.h"
#include "../gcdkeyc.h"
#include "../../common/gsAvailable.h"
#include "../../common/gsStringUtil.h"
#include "../../common/gsAvailable.h"
#include "../../common/gsCommon.h"
#include "../../common/gsAvailable.h"
#include "../../common/gsCore.h"
#include "../../ghttp/ghttp.h"
#include "../../ghttp/ghttpSoap.h"
#include "../../webservices/AuthService.h"
#ifdef _WIN32
#include <conio.h>
#endif

// Defines for AuthService
#define GAME_NAME       _T("gmtest")
#define GAMEID	        0
#define SECRET_KEY_ASCII	"HA6zkS"
#define ACCESS_KEY     "39cf9714e24f421c8ca07b9bcb36c0f5"
#define PRODUCTID	0		
#define NAMESPACEID 1
#define NICKNAME    _T("saketest")
#define EMAIL       _T("saketest@saketest.com")
#define PASSWORD    _T("saketest")

#define DEFAULT_KEY     "2dd4-893a-ce85-6411"   // This is used to the have host register himself.
#define HOST_ID         100                     // This is set to 100 so that client ids don't overlap.
#define DEFAULT_PORT    5000

int errorOccured = 1; // Error by default, will set to zero if the host authenticates.

// Place for AuthService Data
int authenticated = 0;
GSLoginCertificate certificate;
GSLoginPrivateData privateData;

void serverkey_callback(int keyid, qr2_buffer_t outbuf, void *userdata)
{
	switch (keyid)
	{
	case HOSTNAME_KEY:
		qr2_buffer_add(outbuf, _T("Gamespy CDKEY TestServer"));
		break;
	case GAMEVER_KEY:
		qr2_buffer_add_int(outbuf, 10);
		break;
	default:
		qr2_buffer_add(outbuf, _T(""));
	}

	GSI_UNUSED(userdata);
}

void playerkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
	switch (keyid)
	{
	default:
		qr2_buffer_add(outbuf, _T(""));
		break;		
	}

	GSI_UNUSED(userdata);
	GSI_UNUSED(index);
}

void teamkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
	switch (keyid)
	{
	default:
		qr2_buffer_add(outbuf, _T(""));
		break;		
	}

	GSI_UNUSED(userdata);
	GSI_UNUSED(index);
}	

void keylist_callback(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata)
{
	switch (keytype)
	{
	case key_server:
		qr2_keybuffer_add(keybuffer, HOSTNAME_KEY);
		qr2_keybuffer_add(keybuffer, GAMEVER_KEY);
		break;
	case key_player:
		break;
	case key_team:
		break;
	}

	GSI_UNUSED(userdata);
}

int count_callback(qr2_key_type keytype, void *userdata)
{
	if (keytype == key_player)
		return 0;
	else if (keytype == key_team)
		return 0;
	else
		return 0;

	GSI_UNUSED(userdata);
}

void adderror_callback(qr2_error_t error, char *errmsg, void *userdata)
{
	printf("Error adding server: %d, %s\n", error, errmsg);

	GSI_UNUSED(userdata);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void myLoginCallback(GHTTPResult httpResult, WSLoginResponse * theResponse, void * userData)
{
	if (httpResult != GHTTPSuccess)
	{
		printf("Failed on player login, HTTP error: %d", httpResult);
		exit(0);
	}
	else if (theResponse->mLoginResult != WSLogin_Success)
	{
		printf("Failed on player login, Login result: %d", theResponse->mLoginResult);
		exit(0);
	}
	else
	{
		// Copy certificate and private key.
		memcpy(&certificate, &theResponse->mCertificate, sizeof(GSLoginCertificate));
		memcpy(&privateData, &theResponse->mPrivateData, sizeof(GSLoginPrivateData));

		authenticated = 1; // This is so we know that we can stop gsCore thinking.
		_tprintf(_T("Player '") GS_USTR _T("' logged in.\n"), theResponse->mCertificate.mUniqueNick);
	}
	GSI_UNUSED(userData);
}

// Use AuthService, authentication required to use QR2 service
static void LoginAndAuthenticate()
{
	wsSetGameCredentials(ACCESS_KEY, GAMEID, SECRET_KEY_ASCII);

	if (0 != wsLoginProfile(GAMEID, GP_PARTNERID_GAMESPY, NAMESPACEID, NICKNAME, EMAIL, PASSWORD, _T(""), myLoginCallback, NULL))
	{
		printf("Failed on wsLoginProfile");
		exit(0);
	}

	while(authenticated == 0)
	{
		msleep(100);
		gsCoreThink(0);
	}

}

#define MAXCLIENTS 64

// A structure for storing client information; you will probably have most or 
// all of these things in your client structure/object.
typedef struct
{
	SOCKET sock;
	char challenge[32];
	struct sockaddr_in saddr;
	int auth;
} client_t;

// Generate a random nchar challenge.
static char *randomchallenge(int nchars)
{
	static char s[33];
	if (nchars > 32) nchars = 32;
	s[nchars] = 0;
	while (nchars--)
	{
		s[nchars] = 'a' + (char)(rand() % 26);
	}
	return s;
}

// Callback function to indicate whether or not a client has been authorized.
// If the client has been authenticated, we send them a "welcome" string, 
// which allows them to "enter" the game. If they have not been authenticated, 
// we dump them after sending an error message.
static void ClientAuthorize(int gameid, int localid, int authenticated, char *errmsg, void *instance)
{
	client_t *clients = (client_t *)instance;
	char outbuf[512];

	// The client was not authenticated, so print an error and shutdown.
	if (!authenticated)
	{
		sprintf(outbuf,"E:%s\n",errmsg);
		send(clients[localid].sock, outbuf, strlen(outbuf),0);
		shutdown(clients[localid].sock, 2);
		closesocket(clients[localid].sock);
		clients[localid].sock = INVALID_SOCKET;
	} else
	{
		sprintf(outbuf,"M:Welcome to the game, have fun! (%s)\n",errmsg);
		send(clients[localid].sock, outbuf, strlen(outbuf),0);
	}

	GSI_UNUSED(gameid);
}

// Callback function to reauthorize players.
static void ClientRefreshAuthorize(int gameid, int localid, int hint, char *challenge, void *instance)
{
	client_t *clients = (client_t *)instance;
	char outbuf[512];

	// Somebody else is trying to login using this key, so we ask the client to 
	// re-validate. The hint is sent along as a pass through value. The client 
	// will include it with the response.
	sprintf(outbuf, "r:%08d%s", hint, challenge);
	send(clients[localid].sock, outbuf, strlen(outbuf),0);

	GSI_UNUSED(gameid);
}

// Host SELF authorization.
static void HostAuthorize(int gameid, int localid, int authenticated, char *errmsg, void *instance)
{
	if (!authenticated)
		printf("HOST was NOT authenticated (%s)\n",errmsg);
	else
	{
		printf("HOST was authenticated (%s)\n",errmsg);
		errorOccured = 0;
	}

	GSI_UNUSED(instance);
	GSI_UNUSED(localid);
	GSI_UNUSED(gameid);
}

// Host SELF reauthorization.
static void HostRefreshAuthorize(int gameid, int localid, int hint, char *challenge, void *instance)
{
	char response[73];
	printf("REAUTHENTICATING host\n");
	gcd_compute_response(DEFAULT_KEY, challenge, response, CDResponseMethod_REAUTH);
	gcd_process_reauth(GAMEID, HOST_ID, hint, response);

	GSI_UNUSED(instance);
	GSI_UNUSED(localid);
	GSI_UNUSED(gameid);
}

void PublicAddressCallback(unsigned int ip, unsigned short port, void * userdata)
{
	// Self validation using the public IP.
	char response[73];
	char challenge[32];

	strcpy(challenge, randomchallenge(8));
	printf("authenticating host\n");
	gcd_compute_response(DEFAULT_KEY, challenge, response, CDResponseMethod_NEWAUTH);
	gcd_authenticate_user(GAMEID, HOST_ID , ip, challenge, response, HostAuthorize, HostRefreshAuthorize, NULL);

	GSI_UNUSED(userdata);
	GSI_UNUSED(port);
}


// Primary "game" logic. Basically:
// 1. Set up a "server" listen socket
// 2. Initialize the client structures
// 3. Enter a main loop
//	a. Let the gcd code think / do callbacks
//	b. Check for a new connection on the server socket and create a new client
//	c. Check for data on the client sockets
//	d. Check for disconnects on the client sockets
// 4. Disconnect remaining players and exit

int test_main(int argc, char **argv)
{

	client_t clients[MAXCLIENTS];
	SOCKET ssock;
	struct sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	fd_set set; 
	struct timeval timeout = {0,0};
	int error;
	int i,len;
	int totalTime = 0;
	int quit = 0;
	char buf[512];
	gsi_char secret_key[9];
	GSIACResult result;

	// Perform the standard GameSpy Availability Check.
	GSIStartAvailableCheck(GAME_NAME);
	while((result = GSIAvailableCheckThink()) == GSIACWaiting)
		msleep(5);
	if(result != GSIACAvailable)
	{
		printf("The backend is not available\n");
		return 1;
	}

	// Initialize the GameSpy SDK Core (required by SOAP SDKs).
	printf("Initializing the GameSpy Core\n");
	gsCoreInitialize();

	// Authenticate
	LoginAndAuthenticate();

	// First we set up the Query & Reporting SDK - this is mostly taken from 
	// the "qrcsample" sample.
	// Set the secret key (in a semi-obfuscated manner).
	secret_key[0] = 'H';
	secret_key[1] = 'A';
	secret_key[2] = '6';
	secret_key[3] = 'z';
	secret_key[4] = 'k';
	secret_key[5] = 'S';
	secret_key[6] = '\0';

	// Call qr2_init with the query port number and gamename, default IP 
	// address, and no user data.
	if(qr2_init(NULL, NULL, 26900, GAME_NAME, secret_key, 1, 1, serverkey_callback, playerkey_callback,
		teamkey_callback, keylist_callback, count_callback, adderror_callback, NULL) != 0)
	{
		printf("Error starting query sockets\n");
		return -1;
	}

	// Register for public address to do Host self validation in the callback.
	qr2_register_publicaddress_callback(NULL, PublicAddressCallback);


	// Once the QR SDK is initialized, you can initialize the CDKey SDK with 
	// the special QR integration function. Pass in your assigned gameid.
	gcd_init_qr2(NULL, GAMEID);

	SocketStartUp();
	ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_port = htons(DEFAULT_PORT); // Listen on port 5000 by default.
	saddr.sin_family = AF_INET;

	i = bind(ssock, (struct sockaddr *)&saddr, sizeof(saddr));
	if (gsiSocketIsError(i))
	{
		printf("Unable to bind to port %d (%d)\n",DEFAULT_PORT,GOAGetLastError(ssock));
		return 1;
	}
	listen(ssock, SOMAXCONN);

	for (i = 0 ; i < MAXCLIENTS ; i++)
		clients[i].sock = INVALID_SOCKET;

	printf("Running server on port %d... press any key to quit\n", DEFAULT_PORT);
	while (!quit && totalTime < 60000)
	{ // Main loop.
		msleep(10);
		totalTime += 10; // Track the general amount of time passed, albeit unscientifically.

		// We process both QR Queries and any CDKey related activity. The QR 
		// SDK automatically passes any network traffic for the CDKey SDK to 
		// the SDK directly.
		qr2_think(NULL);
		gcd_think();


#ifdef _WIN32
		quit = _kbhit();
#endif

		FD_ZERO ( &set );
		FD_SET ( ssock, &set );
		for (i = 0 ; i < MAXCLIENTS ; i++)
			if (clients[i].sock != INVALID_SOCKET)
				FD_SET(clients[i].sock , &set);
		error = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (/*gsiSocketIsError(ret) ||*/ 0 == error)
			continue;

		// New connection.
		if (FD_ISSET(ssock, &set))
		{
			for (i = 0 ; i < MAXCLIENTS ; i++)
				if (clients[i].sock  == INVALID_SOCKET)
				{
					clients[i].sock  = accept(ssock, (struct sockaddr *)&(clients[i].saddr), &saddrlen);
					listen(ssock,SOMAXCONN);
					strcpy(clients[i].challenge,randomchallenge(8));
					len = sprintf(buf,"c:%s",clients[i].challenge);
					send(clients[i].sock ,buf, len,0); // Send a challenge.
					clients[i].auth = 0;
					printf("Client %d connected\n",i);
					break;
				}
		}

		//client data
		for (i = 0 ; i < MAXCLIENTS ; i++)
		{
			if (clients[i].sock != INVALID_SOCKET && FD_ISSET(clients[i].sock, &set))
			{ 
				len = recv(clients[i].sock,buf, 512, 0);
				if (len <= 0) //the client disconnected
				{
					printf("Client %d disconnected\n",i);
					closesocket(clients[i].sock);
					clients[i].sock = INVALID_SOCKET;
					if (clients[i].auth) //if they were authorized
						gcd_disconnect_user(GAMEID, i);
					continue;
				} 
				buf[len] = 0;
				if (buf[0] == 'r' && buf[1] == ':' && clients[i].auth == 0) // Challenge response.
				{
					printf("Client %d said %s\n",i,buf);
					clients[i].auth = 1;
					gcd_authenticate_user(GAMEID, i,clients[i].saddr.sin_addr.s_addr, 	  
						clients[i].challenge, buf+2, ClientAuthorize, ClientRefreshAuthorize, clients);
				} else if (buf[0] == 'p' && buf[1] == ':' && clients[i].auth == 1) // ison proof response.
				{
					// gcd_send_reauth_response needs to know the sesskey (so 
					// that the keymaster can find the task) and fromaddr (so 
					// that gcdkey knows which keymaster to respond to).
					if (len > 11)
					{
						char hintstr[9];
						int hint = 0;

						memcpy(hintstr, buf+2, 8); // The first 8 characters are the hint.
						hintstr[8] = '\0';
						hint = atoi(hintstr);

						printf("Client %d prooved %d, %s\n",i,hint,buf+10);
						gcd_process_reauth(GAMEID, i, hint, buf+10);
					}
				}
			}
		}
	}	
	gcd_disconnect_all(GAMEID);
	gcd_shutdown();
	qr2_shutdown(NULL);

	SocketShutDown();
	printf("All done!\n");
	return errorOccured;

	GSI_UNUSED(argv);
	GSI_UNUSED(argc);
}
