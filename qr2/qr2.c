///////////////////////////////////////////////////////////////////////////////
// File:	qr2.c
// SDK:		GameSpy Query and Reporting 2 (QR2) SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

/********
INCLUDES
********/
#include "../common/gsCommon.h"
#include "../common/gsAvailable.h"
#include "qr2.h"
#include "qr2regkeys.h"
#include "../natneg/natneg.h"

#ifdef __cplusplus
extern "C" {
#endif	
	
#if defined(__MWERKS__) || defined(_PSP2) // Codewarrior requires function prototypes.
qr2_error_t qr2_initW(/*[out]*/qr2_t *qrec, const unsigned short *ip, int baseport, const unsigned short *gamename, 
					 const unsigned short *secret_key, int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback, 
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback, qr2_keylistcallback_t key_list_callback, 
					 qr2_countcallback_t playerteam_count_callback, qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_init_socketW(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const unsigned short *gamename, 
					 const unsigned short *secret_key, int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback, qr2_keylistcallback_t key_list_callback, 
					 qr2_countcallback_t playerteam_count_callback, qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_init_socketA(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback, qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_initA(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback, qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback, void *userdata);
#endif

/********
DEFINES
********/
#define MASTER_PORT 27900
#define FIRST_HB_TIME 10000 /* 10 sec */
#define HB_TIME 60000 /* 1 minute */
#define KA_TIME 20000 /* 20 sec */
#define MIN_STATECHANGED_HB_TIME 10000 /* 10 sec */
#define MAX_FIRST_COUNT 4 /* 4 tries */
#define MAX_DATA_SIZE 1400
#define INBUF_LEN 256
#define PUBLIC_ADDR_LEN 12
#define QR2_OPTION_USE_QUERY_CHALLENGE 128
	
#define PACKET_QUERY              0x00
#define PACKET_CHALLENGE          0x01
#define PACKET_ECHO               0x02
#define PACKET_ECHO_RESPONSE      0x05  // 0x05, not 0x03 (order)
#define PACKET_HEARTBEAT          0x03
#define PACKET_ADDERROR           0x04
#define PACKET_CLIENT_MESSAGE     0x06
#define PACKET_CLIENT_MESSAGE_ACK 0x07
#define PACKET_KEEPALIVE          0x08
#define PACKET_PREQUERY_IP_VERIFY 0x09
#define PACKET_CLIENT_REGISTERED  0x0A

	 
#define MAX_LOCAL_IP 5

// These are the magic bytes for the NAT negotiation message.
#define NATNEG_MAGIC_LEN 6
#define NN_MAGIC_0 0xFD
#define NN_MAGIC_1 0xFC
#define NN_MAGIC_2 0x1E
#define NN_MAGIC_3 0x66
#define NN_MAGIC_4 0x6A
#define NN_MAGIC_5 0xB2

// Ex flags are the 11th byte in the query packet.
// Old queries will end at 10 bytes.
#define QR2_EXFLAG_SPLIT		(1<<0)

// Here are some other settings for split packet responses.
#define QR2_SPLITNUM_MAX		7
#define QR2_SPLITNUM_FINALFLAG	(1<<7)


/********
TYPEDEFS
********/
typedef unsigned char uchar;

struct qr2_keybuffer_s
{
	uchar keys[MAX_REGISTERED_KEYS];
	int numkeys;
};

struct qr2_buffer_s
{
	char buffer[MAX_DATA_SIZE];
	int len;
};

#define AVAILABLE_BUFFER_LEN(a) (MAX_DATA_SIZE - (a)->len)

/********
VARS
********/
struct qr2_implementation_s static_qr2_rec = {INVALID_SOCKET};
static qr2_t current_rec = &static_qr2_rec;
char qr2_hostname[64];

static int num_local_ips = 0;
static IN_ADDR local_ip_list[MAX_LOCAL_IP];


/********
PROTOTYPES
********/
static void send_heartbeat(qr2_t qrec, int statechanged);
static void send_keepalive(qr2_t qrec);
static int get_sockaddrin(const char *host, int port, SOCKADDR_IN *saddr, struct hostent **savehent);
static void qr2_check_queries(qr2_t qrec);
static void qr2_check_send_heartbeat(qr2_t qrec);
static void enum_local_ips();
static void qr2_expire_ip_verify(qr2_t qrec);
qr2_error_t qr2_create_socket(/*[out]*/SOCKET *sock, const char *ip, /*[in/out]*/int * port);

/****************************************************************************/
/* PUBLIC FUNCTIONS */
/****************************************************************************/

// qr2_init: Initializes the sockets, etc.
// This returns an error value if an error occurred, or 0 otherwise.
qr2_error_t qr2_init_socketA(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char hostname[64];
	int ret;
	int i;
	qr2_t cr;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_init_socket()\r\n");

	if (qrec == NULL)
	{
		cr = &static_qr2_rec;		
	}
	else
	{
		*qrec = (qr2_t)gsimalloc(sizeof(struct qr2_implementation_s));
		cr = *qrec;
	}
	srand((unsigned int)current_time());
	gsiSafeStrcpyA(cr->gamename, gamename, sizeof(cr->gamename));
	gsiSafeStrcpyA(cr->secret_key, secret_key, sizeof(cr->secret_key));
	cr->qport = boundport;
	cr->lastheartbeat = 0;
	cr->lastka = 0;
	cr->hbsock = s;
	cr->listed_state = 1;
	cr->udata = userdata;
	cr->server_key_callback = server_key_callback;
	cr->player_key_callback = player_key_callback;
	cr->team_key_callback = team_key_callback;
	cr->key_list_callback = key_list_callback;
	cr->playerteam_count_callback = playerteam_count_callback;
	cr->adderror_callback = adderror_callback;
	cr->nn_callback = NULL;
	cr->cm_callback = NULL;
	cr->cdkeyprocess = NULL;
	cr->ispublic = ispublic;
	cr->read_socket = 0;
	cr->nat_negotiate = natnegotiate;
	cr->publicip = 0;
	cr->publicport = 0;
	cr->pa_callback = NULL;
	cr->hr_callback = NULL;
	cr->cc_callback = NULL;
	cr->userstatechangerequested = 0;
	cr->backendoptions = 0;

	for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		cr->instance_key[i] = (char)(rand() % 0xFF);
	for (i = 0 ; i < RECENT_CLIENT_MESSAGES_TO_TRACK ; i++)
		cr->client_message_keys[i] = -1;
	cr->cur_message_key = 0;

	memset(cr->ipverify, 0, sizeof(cr->ipverify));
	
	// If (num_local_ips == 0) - caching IPs can result in problems if DHCP has
	// allocated a new one.
	enum_local_ips();

	if (ispublic)
	{
#ifndef UNISPY_FORCE_IP
		int override = qr2_hostname[0];
		if(!override)
			sprintf(hostname, "%s.master." GSI_DOMAIN_NAME, gamename);
#else
		const int override = 1;
		strcpy(hostname, UNISPY_FORCE_IP);
#endif

		ret = get_sockaddrin(override ? qr2_hostname : hostname, MASTER_PORT, &(cr->hbaddr), NULL);

		if (ret == 1)
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
				"%s resolved to %s\r\n", override?qr2_hostname:hostname, inet_ntoa(cr->hbaddr.sin_addr));
		}
		else
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError,
				"Failed on DNS lookup for %s \r\n", override?qr2_hostname:hostname);
		}
	}
	else // We don't need to look up the IP address.
		ret = 1;
	if (!ret)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"qr2_init_socket() returned failed (DNS error)\r\n");
		return e_qrdnserror;
	}
	else
	{
		return e_qrnoerror;
	}


}

#if GSI_UNICODE
qr2_error_t qr2_init_socketW(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const gsi_char *gamename, const gsi_char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char gamename_A[255];
	char secretkey_A[255];

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_init_socketW()\r\n");

	UCSToAsciiString(gamename, gamename_A);
	UCSToAsciiString(secret_key, secretkey_A);

	return qr2_init_socketA(qrec, s, boundport, gamename_A, secretkey_A, ispublic, natnegotiate,
							server_key_callback, player_key_callback, team_key_callback,
							key_list_callback, playerteam_count_callback, adderror_callback, userdata);
}
#endif

qr2_error_t qr2_create_socket(/*[out]*/SOCKET *sock, const char *ip, /*[in/out]*/int * port)
{
	SOCKADDR_IN saddr;	
	SOCKET hbsock;
	int maxport;
	int lasterror = 0;
	int baseport = *port;

#if defined(_LINUX) && !defined(ANDROID)
	unsigned int saddrlen;
#else
	int saddrlen;
#endif

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_create_socket()\r\n");

	SocketStartUp();
	
	hbsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == hbsock)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError,
			"Failed to create heartbeat socket\r\n");
		return e_qrwsockerror;
	}
	
	maxport = baseport + NUM_PORTS_TO_TRY;
	while (baseport < maxport)
	{
		get_sockaddrin(ip,baseport,&saddr,NULL);
		if (saddr.sin_addr.s_addr == htonl(0x7F000001)) // This is localhost -- we don't want that.
			saddr.sin_addr.s_addr = INADDR_ANY;
		
		lasterror = bind(hbsock, (SOCKADDR *)&saddr, sizeof(saddr));
		if (lasterror == 0)
			break; // We found a port.
		baseport++;
	}
	
	if (lasterror != 0) // We weren't able to find a port.
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError,
			"Failed to bind() query socket\r\n");
		return e_qrbinderror;
	}
	
	if (baseport == 0) // We bound it dynamically.
	{
		saddrlen = sizeof(saddr);

		lasterror = getsockname(hbsock,(SOCKADDR *)&saddr, &saddrlen);

		if (lasterror)
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError,
				"Query socket bind() success, but getsockname() failed\r\n");
			return e_qrbinderror;
		}
		baseport = ntohs(saddr.sin_port);
	}

	*sock = hbsock;
	*port = baseport;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
		"Query socket created and bound to port %d\r\n", *port);

	return e_qrnoerror;
}

qr2_error_t qr2_initA(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	SOCKET hbsock;
	qr2_error_t ret;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_init()\r\n");

	ret = qr2_create_socket(&hbsock, ip, &baseport);
	if(ret != e_qrnoerror)
	{
		SocketShutDown();
		return ret;
	}

	ret = qr2_init_socketA(qrec, hbsock, baseport, gamename, secret_key, ispublic, natnegotiate, server_key_callback, player_key_callback, team_key_callback, key_list_callback, playerteam_count_callback, adderror_callback,  userdata);
	if (qrec == NULL)
		qrec = &current_rec;
	(*qrec)->read_socket = 1;
	
	return ret;
}

#if GSI_UNICODE
qr2_error_t qr2_initW(/*[out]*/qr2_t *qrec, const gsi_char *ip, int baseport, const gsi_char *gamename, const gsi_char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char ip_A[255];
	char gamename_A[255];
	char secretkey_A[255];

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_initW()\r\n");

	if (ip != NULL) // NULL value is valid for IP.
		UCSToAsciiString(ip, ip_A);
	UCSToAsciiString(gamename, gamename_A);
	UCSToAsciiString(secret_key, secretkey_A);

	return qr2_initA(qrec, (ip!=NULL)?ip_A:NULL, baseport, gamename_A, secretkey_A, ispublic, natnegotiate, server_key_callback, player_key_callback, team_key_callback, key_list_callback, playerteam_count_callback, adderror_callback,  userdata);
}
#endif

void qr2_register_natneg_callback(qr2_t qrec, qr2_natnegcallback_t nncallback)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_natneg_callback()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	qrec->nn_callback = nncallback;

}
void qr2_register_clientmessage_callback(qr2_t qrec, qr2_clientmessagecallback_t cmcallback)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_clientmessage_callback()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	qrec->cm_callback = cmcallback;
}
void qr2_register_publicaddress_callback(qr2_t qrec, qr2_publicaddresscallback_t pacallback)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_publicaddress_callback()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	qrec->pa_callback = pacallback;
}
void qr2_register_clientconnected_callback(qr2_t qrec, qr2_clientconnectedcallback_t cccallback)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_clientconnected_callback()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	qrec->cc_callback = cccallback;
}

void qr2_register_hostregistered_callback(qr2_t qrec, qr2_hostregisteredcallback_t hrcallback)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_hostregistered_callback()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	qrec->hr_callback = hrcallback;
}

// qr2_think: Processes any waiting queries, and sends a heartbeat if needed.
void qr2_think(qr2_t qrec)
{
	if (qrec == NULL)
		qrec = current_rec;
	if (qrec->ispublic)
		qr2_check_send_heartbeat(qrec);
	qr2_check_queries(qrec);
	qr2_expire_ip_verify(qrec);
	NNThink();
}

// qr2_check_queries: Processes any waiting queries.
void qr2_check_queries(qr2_t qrec)
{
	static char indata[INBUF_LEN]; // 256 byte input buffer.
	SOCKADDR_IN saddr;
	int error;

#if defined(_LINUX) && !defined(ANDROID)
	unsigned int saddrlen = sizeof(SOCKADDR_IN);
#else
	int saddrlen = sizeof(SOCKADDR_IN);
#endif

	if (!qrec->read_socket)
		return; // This is not our job.

	while(CanReceiveOnSocket(qrec->hbsock))
	{
		// Else we have data.
		error = (int)recvfrom(qrec->hbsock, indata, (INBUF_LEN - 1), 0, (SOCKADDR *)&saddr, &saddrlen);
		
		if (gsiSocketIsNotError(error))
		{
			indata[error] = '\0';

			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
				"Received %d bytes on query socket\r\n", error);
			gsDebugBinary(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_RawDump,
				indata, error);

			qr2_parse_queryA(qrec, indata, error, (SOCKADDR *)&saddr);
		}
		else if (error == 0)
		{
			// Is the socket closed?
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
				"CanReceiveOnSocket() returned true, but recvfrom return 0, socket closed!\r\n", error);
		}
		else
		{
			int error2;
			error2 = GOAGetLastError(qrec->hbsock);
			GSI_UNUSED(error2);
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
				"CanReceiveOnSocket() returned true, but recvfrom failed, error: %d!\r\n", error2);
		}
	}
}

// check_send_heartbeat: Perform any scheduled outgoing heartbeats.
void qr2_check_send_heartbeat(qr2_t qrec)
{
	gsi_time tc = current_time();

	if (INVALID_SOCKET == qrec->hbsock)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"HBSock is invalid\r\n");
		return; // There are no sockets to work with.
	}

	// Here we check if we need to send a heartbeat.
	if (qrec->listed_state > 0 && tc - qrec->lastheartbeat > FIRST_HB_TIME) 
	{ // Here we check to see if we haven't gotten a query yet.
		if (qrec->listed_state >= MAX_FIRST_COUNT)
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError,
				"No response from master, generating NoChallengeResponse error\r\n");

			qrec->listed_state = 0; // We failed to get a challenge! Send a message.
#ifndef GSI_UNICODE
			qrec->adderror_callback(e_qrnochallengeerror, "No challenge value was received from the master server.", qrec->udata);
#else
			qrec->adderror_callback(e_qrnochallengeerror, L"No challenge value was received from the master server.", qrec->udata);
#endif
			return;
		} else
		{
			send_heartbeat(qrec, 3);
			qrec->listed_state++;
		}
	}
	else if (qrec->userstatechangerequested && (tc - qrec->lastheartbeat > MIN_STATECHANGED_HB_TIME))
		send_heartbeat(qrec,1);  // Here we send out a pending statechange request.
	else if (tc - qrec->lastheartbeat > HB_TIME || qrec->lastheartbeat == 0 || tc < qrec->lastheartbeat)
		send_heartbeat(qrec,0);  // Here we send out a normal heartbeat.
	
	if (current_time() - qrec->lastka > KA_TIME) // Here we send a keep alive (to keep NAT port mappings the same, if possible).
		send_keepalive(qrec);
}

// qr2_send_statechanged: Sends a statechanged heartbeat; call this when your
// gamemode changes.
void qr2_send_statechanged(qr2_t qrec)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_send_statechanged()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	if (!qrec->ispublic)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Warning,
			"Requested send statechange for LAN game, discarding\r\n");
		return;
	}
	if (current_time() - qrec->lastheartbeat < MIN_STATECHANGED_HB_TIME)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Notice,
			"Queing statechange for later send (too soon)\r\n");

		// Queue up the statechange and send later.
		qrec->userstatechangerequested = 1;
		return;  // Don't allow the server to spam statechanges.
	}

	send_heartbeat(qrec, 1);
	qrec->userstatechangerequested = 0; // Clear the flag in case a queued statechange was still pending.
}


// qr2_shutdown: Cleans up the sockets and shuts down.
void qr2_shutdown(qr2_t qrec)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_StackTrace,
		"qr2_shutdown()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	if (qrec->ispublic)
		send_heartbeat(qrec, 2);
	if (INVALID_SOCKET != qrec->hbsock && qrec->read_socket) // If we own the socket.
	{
		closesocket(qrec->hbsock);
	}
	qrec->hbsock = INVALID_SOCKET;
	qrec->lastheartbeat = 0;
	if(qrec->read_socket) // If we own the socket.
	{
		SocketShutDown();
	}

	// We only free NatNeg Negotiator list if we used ACE.
	if (qrec->cc_callback)
		NNFreeNegotiateList();

	// Need to gsifree it, it was dynamically allocated.
	if (qrec != &static_qr2_rec) 
	{
		gsifree(qrec);
	}

    // free ACE negotiate list
    NNFreeNegotiateList(); //comment out if ACE is not being used

	// Removed - Peer SDK repeatedly calls qr2_shutdown, but keys should only
	// be deallocated once.

	// Developers should call this manually (when in GSI_UNICODE mode).
	// qr2_internal_key_list_free();
}


gsi_bool qr2_keybuffer_add(qr2_keybuffer_t keybuffer, int keyid)
{
	// These are codetime -- not runtime -- errors, changed to assert.
	if (keybuffer->numkeys >= MAX_REGISTERED_KEYS)
		return gsi_false;
	if (keyid < 1 || keyid > MAX_REGISTERED_KEYS)
		return gsi_false;

	keybuffer->keys[keybuffer->numkeys++] = (uchar)keyid;
	return gsi_true;
}

gsi_bool qr2_buffer_add_int(qr2_buffer_t outbuf, int value)
{
	char temp[20];
	sprintf(temp, "%d", value);
	return qr2_buffer_addA(outbuf, temp);
}

gsi_bool qr2_buffer_addA(qr2_buffer_t outbuf, const char *value)
{
	GS_ASSERT(outbuf);
	GS_ASSERT(value);
	{
		int copylen;
		copylen = (int)strlen(value) + 1;
		if (copylen > AVAILABLE_BUFFER_LEN(outbuf))
			copylen = AVAILABLE_BUFFER_LEN(outbuf); // This is the max length we can fit in the buffer.
		if (copylen <= 0)
			return gsi_false; // No buffer space.
		memcpy(outbuf->buffer + outbuf->len, value, (unsigned int)copylen);
		outbuf->len += copylen;
		outbuf->buffer[outbuf->len - 1] = 0; // Here we make sure it's null terminated.
		return gsi_true;
	}
}
#if defined(GSI_UNICODE)
gsi_bool qr2_buffer_addW(qr2_buffer_t outbuf, const gsi_char *value)
{
	char value_A[4096];
	UCSToUTF8String(value, value_A);
	return qr2_buffer_addA(outbuf, value_A);
}
#endif


static void enum_local_ips()
{
#if defined(ANDROID) || defined(_LINUX)
	int fdc;
    SOCKADDR_IN sin_him, ouraddr;
    socklen_t sin_len;
    int errcode;
    unsigned int dip = inet_addr("8.8.8.8");
    unsigned short dport = 53;

    memset((void *)&sin_him, 0, sizeof(sin_him));
    sin_him.sin_family      = AF_INET;
    sin_him.sin_port        = htons(dport);
    sin_him.sin_addr.s_addr = dip;
    sin_len                 = sizeof(SOCKADDR);

    fdc = socket(PF_INET, SOCK_STREAM, 0);
    if (fdc == -1)
    {
    	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError, "socket() failed!\r\n");
    }

    errcode = connect(fdc, (SOCKADDR *)&sin_him, sin_len);
    if (errcode == -1)
    {
    	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError, "connect() failed!\r\n");
    }

    errcode = getsockname(fdc, (SOCKADDR *)&ouraddr, &sin_len);
    if (errcode == -1)
    {
    	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError, "getsockname() failed!\r\n");
    }

    gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Source IP:   %s\n", inet_ntoa(ouraddr.sin_addr));
    gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Source Port: %d\n", ntohs(ouraddr.sin_port));
    gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Dest IP:     %s\n", inet_ntoa(sin_him.sin_addr));
    gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Dest Port:   %d\n", ntohs(sin_him.sin_port));

    if(close (fdc) == -1)
    {
    	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_HotError, "close() failed!\r\n");
    }

	num_local_ips = 0;
    memcpy(&local_ip_list[num_local_ips], &ouraddr.sin_addr, sizeof(IN_ADDR));
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Has local ip %s at index %d\n", inet_ntoa(local_ip_list[num_local_ips]), num_local_ips);
	num_local_ips = 1;

#else
	struct hostent *phost;
	phost = getlocalhost();
	if (phost == NULL) {
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Couldn't get the local host!\n");
		return;
	}
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Enumerating local ips ...\n");
	for (num_local_ips = 0 ; num_local_ips < MAX_LOCAL_IP ; num_local_ips++)
	{
		if (phost->h_addr_list[num_local_ips] == 0) {
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Stopping local ip enumeration at index %d\n", num_local_ips);
			break;
		}

		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment, "Has local ip %s at index %d\n", inet_ntoa(*((IN_ADDR*)phost->h_addr_list[num_local_ips])), num_local_ips);
		memcpy(&local_ip_list[num_local_ips], phost->h_addr_list[num_local_ips], sizeof(IN_ADDR));
	}
#endif
}

/****************************************************************************/


// Return a sockaddrin for the given host (numeric or DNS) and port.
// Returns the hostent in savehent if it is not NULL.
static int get_sockaddrin(const char *host, int port, SOCKADDR_IN *saddr, struct hostent **savehent)
{
	struct hostent *hent = NULL;

	memset(saddr, 0, sizeof(SOCKADDR_IN));   // Don't assume the sockaddr getting sent to this call has been zeroed out.

	saddr->sin_family = AF_INET;
	saddr->sin_port = htons((unsigned short)port);
	if (host == NULL)
		saddr->sin_addr.s_addr = INADDR_ANY;
	else
		saddr->sin_addr.s_addr = inet_addr(host);
	
	if (saddr->sin_addr.s_addr == INADDR_NONE && strcmp(host,"255.255.255.255") != 0)
	{
		hent = gethostbyname(host);
		if (!hent)
			return 0;
		saddr->sin_addr.s_addr = *(unsigned int *)hent->h_addr_list[0];
	}
	if (savehent != NULL)
		*savehent = hent;
	return 1;

} 



/*****************************************************************************/
// Various encryption / encoding routines:

static void swap_byte ( uchar *a, uchar *b )
{
	uchar swapByte; 
	
	swapByte = *a; 
	*a = *b;      
	*b = swapByte;
}

static uchar encode_ct ( uchar c )
{
	if (c <  26) return (uchar)('A'+c);
	if (c <  52) return (uchar)('a'+c-26);
	if (c <  62) return (uchar)('0'+c-52);
	if (c == 62) return (uchar)('+');
	if (c == 63) return (uchar)('/');
	
	return 0;
}

static void gs_encode ( uchar *ins, int size, uchar *result )
{
	int    i,pos;
	uchar  trip[3];
	uchar  kwart[4];
	
	i=0;
	while (i < size)
	{
		for (pos=0 ; pos <= 2 ; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
			kwart[0] = (unsigned char)(  (trip[0])       >> 2);
			kwart[1] = (unsigned char)((((trip[0]) &  3) << 4) + ((trip[1]) >> 4));
			kwart[2] = (unsigned char)((((trip[1]) & 15) << 2) + ((trip[2]) >> 6));
			kwart[3] = (unsigned char)(  (trip[2]) & 63);
			for (pos=0; pos <= 3; pos++) *result++ = encode_ct(kwart[pos]);
	}
	*result='\0';
}

static void gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len )
{ 
	short counter;     
	uchar x, y, xorIndex;
	uchar state[256];       
	
	for ( counter = 0; counter < 256; counter++) state[counter] = (uchar) counter;
	
	x = 0; y = 0;
	for ( counter = 0; counter < 256; counter++)
	{
		y = (uchar)((key[x] + state[counter] + y) % 256);
		x = (uchar)((x + 1) % key_len);
		swap_byte ( &state[counter], &state[y] );
	}
	
	x = 0; y = 0;
	for ( counter = 0; counter < buffer_len; counter ++)
	{
		x = (uchar)((x + buffer_ptr[counter] + 1) % 256);
		y = (uchar)((state[x] + y) % 256);
		swap_byte ( &state[x], &state[y] );
		xorIndex = (uchar)((state[x] + state[y]) % 256);
		buffer_ptr[counter] ^= state[xorIndex];
	}
}

/*****************************************************************************/
// NAT Negotiation support:

static void NatNegProgressCallback(NegotiateState state, void *userdata)
{
	// We don't do anything here.
	GSI_UNUSED(state);
	GSI_UNUSED(userdata);
}

static void NatNegCompletedCallback(NegotiateResult result, SOCKET gamesocket, SOCKADDR_IN *remoteaddr, void *userdata)
{
	qr2_t qrec = (qr2_t)userdata;

	if(qrec->cc_callback)
	{
		if(result == nr_success)
		{
			qrec->cc_callback(gamesocket, remoteaddr, qrec->udata);
		}
	}
}

/*****************************************************************************/


static void qr_add_packet_header(qr2_buffer_t buf, char ptype, char *reqkey)
{
	buf->buffer[0] = ptype;
	memcpy(buf->buffer + 1, reqkey, REQUEST_KEY_LEN);
	buf->len = REQUEST_KEY_LEN  + 1;
}

#define MAX_CHALLENGE 64
static void compute_challenge_response(qr2_t qrec, qr2_buffer_t buf, char *challenge, int challengelen)
{
	char encrypted_val[MAX_CHALLENGE + 1]; // We don't need to null terminate.
	
	if(challengelen < 1)
		return; // Invalid, need room for the NUL.
	if (challengelen > (MAX_CHALLENGE + 1))
		return; // Invalid
	if (challenge[challengelen - 1] != 0)
		return; // Invalid - must be NTS.
	
	gsiSafeStrcpyA(encrypted_val, challenge, sizeof(encrypted_val));
	gs_encrypt((uchar *)qrec->secret_key, (int)strlen(qrec->secret_key), (uchar *)encrypted_val, challengelen - 1);
	gs_encode((uchar *)encrypted_val,challengelen - 1, (uchar *)(buf->buffer + buf->len));
	buf->len += (int)strlen(buf->buffer + buf->len) + 1;
	
}

static void handle_public_address(qr2_t qrec, char * buffer)
{
	unsigned int ip;
	unsigned int portTemp;
	unsigned short port;

	// Get the public ip and port as the master server sees it.
	sscanf(buffer, "%08X%04X", &ip, &portTemp);
	port = (unsigned short)portTemp;
	ip = htonl(ip);

	// Sanity check for errors.
	if((ip == 0) || (port == 0))
		return;

#ifdef GSI_COMMON_DEBUG
	{
		IN_ADDR addr;
		addr.s_addr = ip;
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Received public address (%s:%d)\r\n", inet_ntoa(addr), port);
	}
#endif

	// Has anything changed?
	if((qrec->publicip != ip) || (qrec->publicport != port))
	{
		qrec->publicip = ip;
		qrec->publicport = port;
		qrec->pa_callback(ip, port, qrec->udata);
	}
}

static void qr_build_partial_query_reply(qr2_t qrec, qr2_buffer_t buf, qr2_key_type keytype, int keycount, uchar *keys)
{
	struct qr2_keybuffer_s kb;
	int playerteamcount;
	unsigned short cttemp;
	int i;
	int pindex;
	const char *k;
	int len;

	kb.numkeys = 0;
	if (keycount == 0)
		return; // No keys are wanted.
	
	if (keytype == key_player || keytype == key_team) // Here we need to add the player/team counts.
	{
		if (AVAILABLE_BUFFER_LEN(buf) < (int)sizeof(cttemp))
			return; // No more buffer space.
		playerteamcount = qrec->playerteam_count_callback(keytype, qrec->udata);
		cttemp = htons((unsigned short)playerteamcount);
		memcpy(buf->buffer + buf->len, &cttemp, sizeof(cttemp));
		buf->len += sizeof(cttemp);
	} else
		playerteamcount = 1;

	if (keycount == 0xFF) // We need to get the list of keys.
	{
		qrec->key_list_callback(keytype, &kb, qrec->udata);
		// Add all of the keys.
		for (i = 0 ; i < kb.numkeys ; i++)
		{
			k = qr2_registered_key_list[kb.keys[i]];
			if (k == NULL)
				k = "unknown";
			qr2_buffer_addA(buf, k);
			if (keytype == key_server) // Add the server values.
			{
				len = buf->len;
				qrec->server_key_callback(kb.keys[i], buf, qrec->udata);
				if(len == buf->len)
					qr2_buffer_addA(buf, "");
			}
		}
		// Add an extra null.
		if (AVAILABLE_BUFFER_LEN(buf) < 1)
			return; // No buffer space.
		buf->buffer[buf->len++] = 0;
		keycount = kb.numkeys;
		keys = kb.keys;
		if (keytype == key_server)
			return;	// We already added the keys.
	}
	for (pindex = 0 ; pindex < playerteamcount ; pindex++)
	{
		for (i = 0 ; i < keycount ; i++)
		{
			len = buf->len;
			if (keytype == key_server) // Here we add the server keys.
				qrec->server_key_callback(keys[i], buf, qrec->udata);
			else if (keytype == key_player)
				qrec->player_key_callback(keys[i], pindex, buf, qrec->udata);
			else if (keytype == key_team)
				qrec->team_key_callback(keys[i], pindex, buf, qrec->udata);
			if(len == buf->len)
				qr2_buffer_addA(buf, "");
		}
	}		
}

static void qr_build_query_reply(qr2_t qrec, qr2_buffer_t buf, int serverkeycount, uchar *serverkeys, int playerkeycount, uchar *playerkeys, int teamkeycount, uchar *teamkeys)
{
	qr_build_partial_query_reply(qrec, buf, key_server, serverkeycount, serverkeys);
	qr_build_partial_query_reply(qrec, buf, key_player, playerkeycount, playerkeys);
	qr_build_partial_query_reply(qrec, buf, key_team, teamkeycount, teamkeys);
}


struct QRSplitQueryProgress
{
	qr2_key_type mCurKeyType;
	int mCurPacketNum;
	int mCurKeyIndex;		// serverkey index, playerkey index, teamkey index.
	int mCurSubCount;       // Number of players or number of teams.
	int mCurSubIndex;		// Current player num or current team num.
	struct qr2_keybuffer_s mKeyBuffer; // keybuffer, for key name indexing.
};


// Return values:
//     gsi_true = Send buffer, then call this function again.
//     gsi_false = Don't send buffer, don't call this function again.
static gsi_bool qr_build_split_query_reply(qr2_t qrec, qr2_buffer_t buf, struct QRSplitQueryProgress* progress)
{
	unsigned char* packetNumPos = NULL; // This is used to store the byte position of the packet number.

	// Make sure the key type is valid (the key type is set to invalid when all
	// keys have been processed).
	//if (progress->mCurKeyType < 0 ||progress->mCurKeyType >= key_type_count)
	if (progress->mCurKeyType >= key_type_count)
		return gsi_false; // Stop processing.

	// Check buffer space (the buffer should only contain header at this 
	// point).
	if (AVAILABLE_BUFFER_LEN(buf) < 32)
		return gsi_false; // No buffer space?

	// Dump the split packet "header".
	qr2_buffer_addA(buf, "splitnum");
	packetNumPos = (unsigned char*)&buf->buffer[buf->len++];
	*packetNumPos = (gsi_u8)progress->mCurPacketNum++;

	// Resume dumping at key_type level.
	while (progress->mCurKeyType < key_type_count)
	{
		// Get the list of keys, if we don't already have it.
		if (progress->mKeyBuffer.numkeys == 0)
			qrec->key_list_callback(progress->mCurKeyType, &progress->mKeyBuffer, qrec->udata);

		// Get the list of players/teams, if we don't already have it.
		if (progress->mCurSubCount == 0 && progress->mCurKeyType != key_server)
			progress->mCurSubCount = qrec->playerteam_count_callback(progress->mCurKeyType, qrec->udata);

		// Check buffer space.
		if (AVAILABLE_BUFFER_LEN(buf) < 100)
			return gsi_true; // No buffer space.

		// Write the key type.
		buf->buffer[buf->len++] = (char)progress->mCurKeyType;

		// Write for each key.
		while(progress->mCurKeyIndex < progress->mKeyBuffer.numkeys)
		{
			// Check buffer space.
			int aRegisteredKeyIndex = progress->mKeyBuffer.keys[progress->mCurKeyIndex];
			const char* aKeyName = qr2_registered_key_list[aRegisteredKeyIndex];

			// Write the key name.
			if (gsi_is_false( qr2_buffer_addA(buf,aKeyName) ))
				return gsi_true; // Send, then try again.

			if (progress->mCurKeyType == key_server)
			{
				// Write the key-value.
				qrec->server_key_callback(aRegisteredKeyIndex, buf, qrec->udata);

				// Make sure the key was written.
				if (AVAILABLE_BUFFER_LEN(buf) < 1)
					return gsi_true; // The buffer ran out of space; retry this key-value next packet.
			}
			else
			{
				if (AVAILABLE_BUFFER_LEN(buf) < 1)
					return gsi_true; // The buffer ran out of space; retry this key-value next packet.

				// Non-split packets implicitly begin with player/team number 
				// zero, while split packets explicitly specify the player/team 
				// starting number.
				buf->buffer[buf->len++] = (char)progress->mCurSubIndex;

				// For each player/team.
				while(progress->mCurSubIndex < progress->mCurSubCount)
				{
					// Dump the value into the buffer.
					if (progress->mCurKeyType == key_player)
						qrec->player_key_callback(aRegisteredKeyIndex, progress->mCurSubIndex, buf, qrec->udata);
					else if (progress->mCurKeyType == key_team)
						qrec->team_key_callback(aRegisteredKeyIndex, progress->mCurSubIndex, buf, qrec->udata);

					// Make sure the key was written.
					if (AVAILABLE_BUFFER_LEN(buf) < 1)
						return gsi_true; // Buffer ran out of space, try again next packet.
				
					// Move onto the next player/team value.
					progress->mCurSubIndex++;
				}
				// Append a null to signify end of this team/player key.
				if (AVAILABLE_BUFFER_LEN(buf) > 0)
					buf->buffer[buf->len++] = '\0';
			}
			// Move onto next key.
			progress->mCurKeyIndex++;
			progress->mCurSubIndex = 0;
		}

		// Append a null to signify end of this key_type section.
		if (AVAILABLE_BUFFER_LEN(buf) > 0)
			buf->buffer[buf->len++] = '\0';

		// Move onto next key type.
		progress->mCurKeyType = (qr2_key_type)(progress->mCurKeyType + 1);
		progress->mCurKeyIndex = 0;
		progress->mCurSubCount = 0;
		progress->mCurSubIndex = 0;
		progress->mKeyBuffer.numkeys = 0;
	}

	// Add the "final" flag to the packet number.
	*packetNumPos |= QR2_SPLITNUM_FINALFLAG;
	return gsi_true; // The function will bail without sending the next iteration.
}

static void qr_process_query(qr2_t qrec, qr2_buffer_t buf, uchar *qdata, int len, SOCKADDR* sender)
{
	uchar serverkeycount;
	uchar playerkeycount;
	uchar teamkeycount;
	uchar exflags = 0;

	uchar *serverkeys = NULL;
	uchar *playerkeys = NULL;
	uchar *teamkeys = NULL;
	if (len < 3)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Discarding invalid query (too short#1: %d bytes total)\r\n", len);
		return; // The query was invalid.
	}
	serverkeycount = qdata[0];
	qdata++;
	len--;
	if (serverkeycount != 0 && serverkeycount != 0xFF)
	{
		serverkeys = qdata;
		qdata += serverkeycount;
		len -= serverkeycount;
	}
	if (len < 2)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Discarding invalid query (too short#2: %d bytes remain)\r\n", len);
		return; // The query was invalid.
	}
	playerkeycount = qdata[0];
	qdata++;
	len--;
	if (playerkeycount != 0 && playerkeycount != 0xFF)
	{
		playerkeys = qdata;
		qdata += playerkeycount;
		len -= playerkeycount;
	}
	if (len < 1)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Discarding invalid query (too short#3: %d bytes remain)\r\n", len);
		return; // The query was invalid.
	}
	teamkeycount = qdata[0];
	qdata++;
	len--;
	if (teamkeycount != 0 && teamkeycount != 0xFF)
	{
		teamkeys = qdata;
		qdata += teamkeycount;
		len -= teamkeycount;
	}
	if (len < 0)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Discarding invalid query (too short#4: %d bytes remain)\r\n", len);
		return; // The query was invalid.
	}

	// Check the exflags.
	if (len > 0)
	{
		exflags = qdata[0];
		len--;
	}
	
	// Support split queries?
	if ((exflags & QR2_EXFLAG_SPLIT)==QR2_EXFLAG_SPLIT)
	{
		struct QRSplitQueryProgress progress;
		progress.mCurPacketNum = 0;
		progress.mCurKeyType = key_server;
		progress.mCurKeyIndex = 0;
		progress.mCurSubCount = 0;
		progress.mCurSubIndex = 0;
		progress.mKeyBuffer.numkeys = 0;
			
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
			"Building query reply (split packet supported)\r\n");

		// Send packets as long as we need to.
		while (gsi_true == qr_build_split_query_reply(qrec, buf, &progress))
		{
			sendto(qrec->hbsock, buf->buffer, buf->len, 0, sender, sizeof(SOCKADDR_IN));
			buf->len = 5; // Reset the buffer, but preserve 5-byte qr2 header.
			if (progress.mCurPacketNum > QR2_SPLITNUM_MAX)
				return; // More than 7 isn't supported (likely a bug if you hit it).
		}

		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
				"Finished split query reply (%d packets)\r\n", progress.mCurPacketNum);
	}
	else
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
			"Building query reply (single packet)\r\n");
		qr_build_query_reply(qrec, buf, serverkeycount, serverkeys, playerkeycount, playerkeys, teamkeycount, teamkeys);
		sendto(qrec->hbsock, buf->buffer, buf->len, 0, sender, sizeof(SOCKADDR_IN));
	}

	GSI_UNUSED(sender);
}

/*
static void qr_build_partial_old_query_reply(qr2_t qrec, qr2_buffer_t buf, qr2_key_type keytype)
{
	char tempkeyname[128];
	struct qr2_keybuffer_s kb;
	int playerteamcount;
	int i;
	int pindex;
	const char *k;
	int len;

	kb.numkeys = 0;

	if (keytype == key_player || keytype == key_team) //need to add the player/team counts
	{
		playerteamcount = qrec->playerteam_count_callback(keytype, qrec->udata);
	} else
		playerteamcount = 1;

	qrec->key_list_callback(keytype, &kb, qrec->udata);
	//add all the keys
	for (i = 0 ; i < kb.numkeys ; i++)
	{
		k = qr2_registered_key_list[kb.keys[i]];
		if (k == NULL)
			k = "unknown";
		if (keytype == key_server) //add the server values
		{
			qr2_buffer_addA(buf, k);
			buf->buffer[buf->len - 1] = '\\';
			len = buf->len;
			qrec->server_key_callback(kb.keys[i], buf, qrec->udata);
			if(len == buf->len)
				qr2_buffer_addA(buf, "");
			buf->buffer[buf->len - 1] = '\\';
		} else //need to look it up for each player/team
		{

			for (pindex = 0 ; pindex < playerteamcount ; pindex++)
			{
				sprintf(tempkeyname, "%s%d", k, pindex);
				qr2_buffer_addA(buf, tempkeyname);
				buf->buffer[buf->len - 1] = '\\';
				len = buf->len;
				if (keytype == key_player)
					qrec->player_key_callback(kb.keys[i], pindex, buf, qrec->udata);
				else if (keytype == key_team)
					qrec->team_key_callback(kb.keys[i], pindex, buf, qrec->udata);
				if(len == buf->len)
					qr2_buffer_addA(buf, "");
				buf->buffer[buf->len - 1] = '\\';
			}		
		}
	}
}
*/

//we just build a status reply, since we don't have equivalent callbacks
/*static void qr_process_old_query(qr2_t qrec, qr2_buffer_t buf)
{
	buf->len = 1;
	buf->buffer[0] = '\\';

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Comment,
		"Processing QR1 style query\r\n");

	qr_build_partial_old_query_reply(qrec, buf, key_server);
	qr_build_partial_old_query_reply(qrec, buf, key_player);
	qr_build_partial_old_query_reply(qrec, buf, key_team);
	qr2_buffer_addA(buf, "final\\\\queryid\\1.1");
	buf->len--; //remove the final null;
}*/

static void qr_process_client_message(qr2_t qrec, char *buf, int len)
{
	unsigned char natNegBytes[NATNEG_MAGIC_LEN] = {NN_MAGIC_0,NN_MAGIC_1,NN_MAGIC_2,NN_MAGIC_3,NN_MAGIC_4,NN_MAGIC_5};
	int i;
	int isnatneg = 1;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Notice,
		"Processing client message\r\n");

	// See if it's a NatNeg request.
	if (len >= NATNEG_MAGIC_LEN + 4)
	{
		for (i = 0 ; i < NATNEG_MAGIC_LEN ; i++)
			if ((unsigned char)buf[i] != natNegBytes[i])
			{
				isnatneg = 0;
				break;
			}
	} else
		isnatneg = 0;
	if (isnatneg)
	{
		int cookie;
		memcpy(&cookie, buf + NATNEG_MAGIC_LEN, 4);
		cookie = (int)ntohl((unsigned int)cookie);

		// Check if we should do NatNeg.
		if(qrec->cc_callback)
		{
			NegotiateError error;

			if(__GSIACResult != GSIACAvailable)
			{
				// Here we fake that we passed the availability check.
				__GSIACResult = GSIACAvailable;
				gsiSafeStrcpyA(__GSIACGamename, qrec->gamename, sizeof(__GSIACGamename));
			}

			// do the negotiation
			error = NNInternalBeginNegotiationWithSocket(qrec->hbsock, cookie, 1, gsi_true, NatNegProgressCallback, NatNegCompletedCallback, qrec);
			if(error != ne_noerror)
			{
				// We can ignore errors.
			}
		}
		else if (qrec->nn_callback)
		{
			qrec->nn_callback(cookie, qrec->udata); 
		}
	} else
		if (qrec->cm_callback)
		{
#ifndef GSI_UNICODE
			qrec->cm_callback(buf, len, qrec->udata);
#else
			gsi_char *buf_W;
			buf_W = UTF8ToUCSStringAlloc(buf);
			qrec->cm_callback(buf_W, wcslen(buf_W), qrec->udata);
#endif
		}
}

static int qr_got_recent_message(qr2_t qrec, int msgkey)
{
	int i;
	for (i = 0 ; i < RECENT_CLIENT_MESSAGES_TO_TRACK ; i++)
	{
		if (qrec->client_message_keys[i] == msgkey)
			return 1;
	}
	// Else, add it to the list.
	qrec->cur_message_key = (qrec->cur_message_key + 1) % RECENT_CLIENT_MESSAGES_TO_TRACK;
	qrec->client_message_keys[qrec->cur_message_key] = msgkey;
	return 0;
}

// Send a random value to the user to verify IP address.
static gsi_bool qr2_process_ip_verify(qr2_t qrec, struct qr2_buffer_s* buf, SOCKADDR_IN* sender)
{
	int i=0;
	gsi_time now = current_time();
	int firstFreeIndex = -1;
	int numDuplicates = 0;

	// If the query challenge is disabled, return 0 as the challenge.
	if ((qrec->backendoptions & QR2_OPTION_USE_QUERY_CHALLENGE) == 0)
	{
		qr2_buffer_add_int(buf, 0);
		return gsi_true;
	}

	// Check to see if this ip/port combo is already in the list.
	for (; i < QR2_IPVERIFY_ARRAY_SIZE; i++)
	{
		// Mark the first free index when/if found.
		if (firstFreeIndex == -1 && qrec->ipverify[i].addr.sin_addr.s_addr == 0)
			firstFreeIndex = i;
		// Count any indexes that match this IP/port.
		if (qrec->ipverify[i].addr.sin_addr.s_addr == sender->sin_addr.s_addr &&
			qrec->ipverify[i].addr.sin_port == sender->sin_port)
		{
			numDuplicates++;
		}
	}

	// Discard if there are too many duplicates or if no index is found.
	if (numDuplicates > QR2_IPVERIFY_MAXDUPLICATES)
		return gsi_false;
	if (firstFreeIndex == -1)
		return gsi_false; // No free indexes.

	
	// Create a random challenge for this ip/port combo.
	qrec->ipverify[firstFreeIndex].addr = *sender;
	qrec->ipverify[firstFreeIndex].challenge = htonl( (rand() << 16) | rand() );
	qrec->ipverify[firstFreeIndex].createtime = now;

	qr2_buffer_add_int(buf, (int)qrec->ipverify[firstFreeIndex].challenge);
	return gsi_true; // The buffer is ready to be sent.
}

// Check to see if the returned ipverify value matches the random we sent 
// earlier. If it matches, remove it.
static gsi_bool qr2_check_ip_verify(qr2_t qrec, SOCKADDR_IN* sender, gsi_u32 ipverify)
{
	int i=0;
	for (; i < QR2_IPVERIFY_ARRAY_SIZE; i++)
	{
		if (qrec->ipverify[i].addr.sin_addr.s_addr == sender->sin_addr.s_addr &&
			qrec->ipverify[i].addr.sin_port == sender->sin_port)
		{
			if (qrec->ipverify[i].challenge == ipverify)
			{
				// Reset the structure.
				qrec->ipverify[i].addr.sin_addr.s_addr = 0;
				qrec->ipverify[i].addr.sin_port = 0;
				return gsi_true;
			}
			// Else, keep searching: a single IP may have multiple outstanding challenges.
		}
	}
	return gsi_false;
}

// Expire old verify attempts.
static void qr2_expire_ip_verify(qr2_t qrec)
{
	int i=0;
	gsi_time now = current_time();

	for (; i < QR2_IPVERIFY_ARRAY_SIZE; i++)
	{
		if (qrec->ipverify[i].addr.sin_addr.s_addr != 0 && (now - qrec->ipverify[i].createtime > QR2_IPVERIFY_TIMEOUT))
			qrec->ipverify[i].addr.sin_addr.s_addr = 0;
	}
}

// parse_query: parse an incoming query and reply to each query.
void qr2_parse_queryA(qr2_t qrec, char *query, int len, SOCKADDR *sender)
{
	struct qr2_buffer_s buf;
	char ptype;
	char *reqkey;
	char *pos;
	gsi_u32 ipverify = 0;
	int i;
	
	buf.len = 0;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_StackTrace,
		"qr2_parse_queryA()\r\n");

	if (qrec == NULL)
		qrec = current_rec;
	if (query[0] == 0x3B) // A CDkey query.
	{
		if (qrec->cdkeyprocess != NULL)
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
				"Forwarding cdkey query onto cdkey sdk\r\n");
			qrec->cdkeyprocess(query, len, sender);
		}
		else
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
				"Received cdkey query but not using qr2-cdkey integration!\r\n");
		}
		return;
	}
	/*if (query[0] == '\\') //it's a QR1-style query
	{
		qr_process_old_query(qrec, &buf);
		sendto(qrec->hbsock, buf.buffer, buf.len, 0, sender, sizeof(struct sockaddr_in));
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
			"Sent %d bytes as QR1 query response\r\n", buf.len);
		gsDebugBinary(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_RawDump,
			buf.buffer, buf.len);

		return;
	}*/
	if((len >= 6) && (memcmp(query, NNMagicData, NATNEG_MAGIC_LEN) == 0)) /* a natneg query */
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Forwarding natneg query onto natneg sdk\r\n");
		NNProcessData(query, len, (SOCKADDR_IN*)sender);
		return;
	}

	if (len < 7)
		return; // This is too small to be valid.
	// Check the magic byte.
	if ((uchar)query[0] != QR_MAGIC_1 || (uchar)query[1] != QR_MAGIC_2)
		return;

//#define SIMULATE_HARD_FIREWALL
#if defined(SIMULATE_HARD_FIREWALL)
	// Ignore all QR2 packets on this port.
	// This generates a "no challenge" error.
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Warning,
			"SIMULATE_HARD_FIREWALL defined, ignoring QR2 packet\r\n");
	return;
#endif

//#define SIMULATE_FIREWALL
#if defined(SIMULATE_FIREWALL) 
	// Ignore messages that are not from the GameSpy master.
	{
		unsigned int aMasterAddr = (qrec->hbaddr.sin_addr.s_addr);
		if (((SOCKADDR_IN*)sender)->sin_addr.s_addr != aMasterAddr)
		{
			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Warning,
				"SIMULATE_FIREWALL defined, ignoring non-master QR2 packet\r\n");
			return;
		}
	}
#endif
//#if defined(QR2_IP_FILTER)
		if (qrec->denyresp2_ip_callback)
		{
			unsigned int aMasterAddr = (qrec->hbaddr.sin_addr.s_addr);
			unsigned int senderAddr = ((SOCKADDR_IN*)sender)->sin_addr.s_addr;	//in hbo
			if ((aMasterAddr & 0xffff) != (senderAddr & 0xffff))				//if sender ip is not in gamespy subnet /16 (fake)
			{
				int deny_result = 0;
				qrec->denyresp2_ip_callback(qrec->udata, senderAddr, &deny_result);
				if (deny_result)
				{
					gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Warning,
						"QR2_IP_FILTER defined, ignoring QR2 packet from denied ip\r\n");
					return;
				}
			}
		}
//#endif //#if defined(QR2_IP_FILTER)

	if (qrec->listed_state > 0)
		qrec->listed_state = 0;

	ptype = query[2];
	reqkey = &query[3];
	pos = query + 7;
	len -= 7;

	qr_add_packet_header(&buf, ptype, reqkey);
	switch (ptype)
	{
	case PACKET_PREQUERY_IP_VERIFY:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Received IP verify challenge request\r\n");
		if (gsi_is_true(qr2_process_ip_verify(qrec, &buf, (SOCKADDR_IN*)sender)))
			break; // Break here so that we send below.
		else
			return; // Otherwise, return and discard buf.


	case PACKET_QUERY:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Processing query packet\r\n");

		// When using the query challenge option, verify that the client sent a
		// PREQUERY_IP_VERIFY.
		if ((qrec->backendoptions & QR2_OPTION_USE_QUERY_CHALLENGE) == QR2_OPTION_USE_QUERY_CHALLENGE)
		{
			if (len < 4)
				return; // This was too small for an ip-verify query.

			ipverify = ntohl(*(gsi_u32*)pos);
			pos += 4;
			len -= 4;

			// Has this client verified their IP? (prevent IP spoofing.)
			if (gsi_is_false(qr2_check_ip_verify(qrec, (SOCKADDR_IN *)sender, ipverify)))
			{
				// Don't send an error. As nice as the debug info would be, 
				// the incompatible SBs will interpret it as a server response.
				return;
			}
		}
		
		// Here, qr_process_query now sends packets.
		qr_process_query(qrec, &buf, (uchar *)pos, len, sender);
		return;
	case PACKET_CHALLENGE:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Processing challenge packet\r\n");

		// Check the instance key (to prove this packet came from the master).
		for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		{
			if (reqkey[i] != qrec->instance_key[i])
				return; // This was not a valid instance key.
		}

		// Calculate the challenge.
		if (len >= (PUBLIC_ADDR_LEN + 3))
		{
			unsigned int backendoptions;

			// Read the options, then the public address.
			sscanf(pos + len - (PUBLIC_ADDR_LEN + 3), "%02x", &backendoptions);
			qrec->backendoptions = (gsi_u8)backendoptions;
			
			#ifdef QR2_DEBUG_FORCE_USE_QUERY_CHALLENGE
			qrec->backendoptions = QR2_OPTION_USE_QUERY_CHALLENGE;
			#endif

			gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Notice,
				"Received setting options: %d\r\n", qrec->backendoptions);

			if(qrec->pa_callback)
				handle_public_address(qrec, pos + len - (PUBLIC_ADDR_LEN + 1));
			else
			{
				gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
					"Discarding public address (no callback set)\r\n");
			}
		}
		compute_challenge_response(qrec, &buf, pos, len);
		break;
	case PACKET_ECHO:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Processing echo packet\r\n");

		// Now add the echo data.
		if (len > 32)
			len = 32; // Max len is 32 bytes.
		buf.buffer[0] = PACKET_ECHO_RESPONSE;
		memcpy(buf.buffer + buf.len, pos, (size_t)len);
		buf.len += len;
		break;

	case PACKET_ADDERROR:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_WarmError,
			"Processing adderror packet\r\n");

		if (qrec->listed_state == -1)
			return; // We already got an error message.
		// Verify the instance code.
		for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		{
			if (reqkey[i] != qrec->instance_key[i])
				return; // Not a valid instance key.
		}
		if (len < 2)
			return; // Not a valid message.
		qrec->listed_state = -1;
#ifndef GSI_UNICODE
		qrec->adderror_callback((qr2_error_t)*pos, pos + 1, qrec->udata);
#else
		{
			gsi_char *message_W;
			message_W = UTF8ToUCSStringAlloc(pos+1);
			qrec->adderror_callback((qr2_error_t)*pos, message_W, qrec->udata);
			gsifree(message_W);
		}
#endif
		return; // We don't need to send anything back for this type of message.
	case PACKET_CLIENT_MESSAGE:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Notice,
			"Processing clientmessage packet\r\n");

		// Verify the instance code.
		for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		{
			if (reqkey[i] != qrec->instance_key[i])
				return; // Not a valid instance key.
		}
		if (len < 4) // No message key?
			return;
		buf.buffer[0] = PACKET_CLIENT_MESSAGE_ACK;
		// Add the msg key.
		memcpy(buf.buffer + buf.len, pos, (size_t)4);
		buf.len += 4;
		// See if we've recently gotten this same message, to help avoid dupes.
		memcpy(&i, pos, (size_t)4);
		if (!qr_got_recent_message(qrec, i))
			qr_process_client_message(qrec, pos + 4, len - 4);
		// Send an ack response.
		break;
	case PACKET_KEEPALIVE:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
			"Processing keepalive packet\r\n");
		return; //if we get a keep alive, ignore it and return (just used to tell us the server knows about us)

	// NOTE: This does NOT mean the server browsing client.
	// When this qr2 client is registered with the master server, it will 
	// get this message from the master server.
	case PACKET_CLIENT_REGISTERED:
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
			"Processing client packet\r\n");
		if (qrec->hr_callback)
			qrec->hr_callback(qrec->udata);
		return;
	default:
		return; // Not a valid type.

	}
	// Send the reply.
	sendto(qrec->hbsock, buf.buffer, buf.len, 0, sender, sizeof(SOCKADDR_IN));

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
		"Sent %d bytes as QR2 query response\r\n", buf.len);
	gsDebugBinary(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_RawDump,
		buf.buffer, buf.len);
}

// send_keepalive: Send a keepalive packet to the hbmaster3.
static void send_keepalive(qr2_t qrec)
{
	struct qr2_buffer_s buf;
	buf.len = 0;
	qr_add_packet_header(&buf, PACKET_KEEPALIVE, qrec->instance_key);
	sendto(qrec->hbsock, buf.buffer, buf.len, 0, (SOCKADDR *)&(qrec->hbaddr), sizeof(SOCKADDR_IN));

	// Set the ka time to now.
	qrec->lastka = current_time();

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
		"Sent keepalive to master\r\n", buf);
}

// send_heartbeat: Sends a heartbeat to the gamemaster, adds \statechanged\ if statechanged != 0
static void send_heartbeat(qr2_t qrec, int statechanged)
{
	struct qr2_buffer_s buf;
	// int ret;
	int i;
	char ipkey[20];

	buf.len = 0;
	qr_add_packet_header(&buf, PACKET_HEARTBEAT, qrec->instance_key);
	// Now we add our special keys.
	for (i = 0 ; i < num_local_ips ; i++)
	{
		sprintf(ipkey, "localip%d", i);
		qr2_buffer_addA(&buf, ipkey);
		qr2_buffer_addA(&buf, inet_ntoa(local_ip_list[i]));
	}
	qr2_buffer_addA(&buf, "localport");
	qr2_buffer_add_int(&buf, qrec->qport);
	qr2_buffer_addA(&buf, "natneg");
	qr2_buffer_addA(&buf, qrec->nat_negotiate ? "1" : "0");
	if(statechanged)
	{
		qr2_buffer_addA(&buf, "statechanged");
		qr2_buffer_add_int(&buf, statechanged);
	}
	qr2_buffer_addA(&buf, "gamename");
	qr2_buffer_addA(&buf, qrec->gamename);
	if(qrec->pa_callback)
	{
		qr2_buffer_addA(&buf, "publicip");
		qr2_buffer_add_int(&buf, (int)qrec->publicip);
		qr2_buffer_addA(&buf, "publicport");
		qr2_buffer_add_int(&buf, qrec->publicport);
	}
	
	// Add the rest of our keys.
	if (statechanged != 2) // Don't need if we are exiting.
	{
		// The hbmaster will fail if the packet is malformed which might happen
		// if the buffer isn't large enough.
		// So first copy dump the keys into a temporary buffer.
		struct qr2_buffer_s temp;
		memcpy(temp.buffer, buf.buffer, (size_t)buf.len);
		temp.len = buf.len;
		qr_build_query_reply(qrec, &temp, 0xFF, NULL, 0xFF, NULL, 0xFF, NULL);

		// If we maxed out the packet, try again using only the server keys.
		if(AVAILABLE_BUFFER_LEN(&temp) < 1)
		{
			temp.len = buf.len;
			qr_build_query_reply(qrec, &temp, 0xFF, NULL, 0, NULL, 0, NULL);
		}
		// Copy temp back into the buffer.
		memcpy(buf.buffer, temp.buffer, (size_t)temp.len);
		buf.len = temp.len;
	}	
	else
	{
		// Add an extra NUL to end the server keys.
		if (AVAILABLE_BUFFER_LEN(&buf) >= 1)
			buf.buffer[buf.len++] = 0;
	}

	//ret = (int)sendto(qrec->hbsock, buf.buffer, buf.len, 0, (struct sockaddr *)&(qrec->hbaddr), sizeof(struct sockaddr_in));
	sendto(qrec->hbsock, buf.buffer, buf.len, 0, (SOCKADDR *)&(qrec->hbaddr), sizeof(SOCKADDR_IN));

	// Set the ka time and hb time to now.
	qrec->lastka = qrec->lastheartbeat = current_time();

	// Clear the pending heartbeat request flag.
	if (statechanged != 0)
		qrec->userstatechangerequested = 0;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_Comment,
		"Sent heartbeat to master (size %d)\r\n", buf.len);
	gsDebugBinary(GSIDebugCat_QR2, GSIDebugType_Network, GSIDebugLevel_RawDump,
		buf.buffer, buf.len);
}

#ifdef __cplusplus
}
#endif
