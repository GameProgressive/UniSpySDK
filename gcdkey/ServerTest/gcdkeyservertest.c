///////////////////////////////////////////////////////////////////////////////
// File:	gcdkeyservertest.c
// SDK:		GameSpy CD Key SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "../gcdkeys.h"
#include "../../common/gsCommon.h"
#include "../../common/gsAvailable.h"
#ifdef _WIN32
	#include <conio.h>
#endif
 

#define MAXCLIENTS 64
#define MY_GAMEID 0		// GameId as assigned by GameSpy.
#define MY_GAMENAME _T("gmtest")
#define PORT 2000

// A structure for storing client information. 
// You will probably have most or all of these things in your client 
// structure/object.
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
		printf("Client %d was NOT authenticated (%s)\n",localid, errmsg);
		sprintf(outbuf,"E:%s\n",errmsg);
		send(clients[localid].sock, outbuf, strlen(outbuf),0);
		shutdown(clients[localid].sock, 2);
		closesocket(clients[localid].sock);
		clients[localid].sock = INVALID_SOCKET;
	} else
	{
		printf("Client %d was authenticated (%s)\n",localid, errmsg);
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
	// will include it with the response. It is passed as the first 8 
	// characters along with the challenge to the client.
	sprintf(outbuf, "r:%08d%s", hint, challenge);
	send(clients[localid].sock, outbuf, strlen(outbuf),0);

	GSI_UNUSED(gameid);
}


// Primary "game" logic. Basically, we:
// 1. Set up a "server" listen socket
// 2. Initialize the client structures
// 3. Enter a main loop
// 	a. Let the gcd code think / do callbacks
//	b. Check for a new connection on the server socket and create a new client
// 	c. Check for data on the client sockets
// 	d. Check for disconnects on the client sockets
// 4. Disconnect remaining players and exit

int test_main(int argc, char **argv)
{
	
	client_t clients[MAXCLIENTS];
	SOCKET ssock;
	struct sockaddr_in saddr;
	socklen_t saddrlen = sizeof(saddr);
	fd_set set; 
	struct timeval timeout = {0,0};
	int error;
	int i,len;
	int quit = 0;
	char buf[512];
	unsigned short port;
	GSIACResult result;
	
	// Perform the standard GameSpy Availability Check.
	GSIStartAvailableCheck(MY_GAMENAME);
	while((result = GSIAvailableCheckThink()) == GSIACWaiting)
		msleep(5);
	if(result != GSIACAvailable)
	{
		printf("The backend is not available\n");
		return 1;
	}

	// Initialize the gcd system with your assigned gameid.
	if(gcd_init(MY_GAMEID) != 0)
	{
		printf("Error initializing\n");
		return 1;
	}

	SocketStartUp();
	ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;

	// Initialize here to prevent strict compiler warnings.
	i = 0;
	for(port = PORT ; port < (PORT + 100) ; port++)
	{
		saddr.sin_port = htons(port);
		i = bind(ssock, (struct sockaddr *)&saddr, sizeof(saddr));
		if(gsiSocketIsNotError(i))
			break;
	}
	if (gsiSocketIsError(i))
	{
		printf("Unable to bind to any of the ports %d through %d (%d)\n",PORT, port - 1, GOAGetLastError(ssock));
		return 1;
	}
	listen(ssock, SOMAXCONN);

	for (i = 0 ; i < MAXCLIENTS ; i++)
		clients[i].sock = INVALID_SOCKET;

	printf("Running on port %d... press any key to quit\n", port);
	while (!quit)
	{ // Main loop.
		msleep(10);
		gcd_think();
		
	
		#ifdef _WIN32
			quit = _kbhit();
		#else
		 // On other systems, perform a hard shutdown.
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
		
		// Client data.
		for (i = 0 ; i < MAXCLIENTS ; i++)
			if (clients[i].sock != INVALID_SOCKET && FD_ISSET(clients[i].sock, &set))
			{ 
				len = recv(clients[i].sock,buf, 512, 0);
				if (len <= 0) // The client disconnected, so print a message.
				{
					printf("Client %d disconnected\n",i);
					closesocket(clients[i].sock);
					clients[i].sock = INVALID_SOCKET;
					if (clients[i].auth) // If they were authorized.
						gcd_disconnect_user(MY_GAMEID, i);
					continue;
				} 
				buf[len] = 0;
				if (buf[0] == 'r' && buf[1] == ':' && clients[i].auth == 0) // Challenge response.
				{
					printf("Client %d said %s\n",i,buf);
					clients[i].auth = 1;
					gcd_authenticate_user(MY_GAMEID, i,clients[i].saddr.sin_addr.s_addr, 	  
						clients[i].challenge, buf+2, ClientAuthorize, ClientRefreshAuthorize, clients);
				}
				else if (buf[0] == 'p' && buf[1] == ':' && clients[i].auth == 1) // ison proof response.
				{
					// gcd_send_reauth_response needs to know the sesskey (so 
					// that the keymaster can find the task) and fromaddr (so 
					// gcdkey knows which keymaster to respond to).
					if (len > 11)
					{
						char hintstr[9];
						int hint = 0;

						memcpy(hintstr, buf+2, 8); // The first 8 characters are the hint.
						hintstr[8] = '\0';
						hint = atoi(hintstr);

						printf("Client %d prooved %d, %s\n",i,hint,buf+10);
						gcd_process_reauth(MY_GAMEID, i, hint, buf+10);
					}
				}
			}
	}
	gcd_disconnect_all(MY_GAMEID);
	gcd_shutdown();
	SocketShutDown();
	printf("All done!\n");
	return 0;

	GSI_UNUSED(argv);
	GSI_UNUSED(argc);
}
