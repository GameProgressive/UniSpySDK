///////////////////////////////////////////////////////////////////////////////
// File:	nninternal.h
// SDK:		GameSpy NAT Negotiation SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _NNINTERNAL_H_
#define _NNINTERNAL_H_

#include "natneg.h"
#define MATCHUP1_HOSTNAME "natneg1." GSI_DOMAIN_NAME
#define MATCHUP2_HOSTNAME "natneg2." GSI_DOMAIN_NAME
#define MATCHUP3_HOSTNAME "natneg3." GSI_DOMAIN_NAME
#define MATCHUP_PORT1 27901
#define MATCHUP_PORT2 27901
#define MATCHUP_PORT3 27901

#define FINISHED_NOERROR 0
#define FINISHED_ERROR_DEADBEAT_PARTNER 1
#define FINISHED_ERROR_INIT_PACKETS_TIMEDOUT 2

#define PREINIT_RETRY_TIME 500
#define PREINIT_RETRY_COUNT 10
#define PREINITACK_RETRY_TIME 10000
#define PREINITACK_RETRY_COUNT 12
#define INIT_RETRY_TIME 500
#define INIT_RETRY_COUNT 10
#define NNINBUF_LEN 512
#define PING_RETRY_TIME 700
#define PING_RETRY_COUNT 7
#define PARTNER_WAIT_TIME 60000
#define REPORT_RETRY_TIME 500
#define REPORT_RETRY_COUNT 4

#define NN_PROTVER 4
//#define NN_PROTVER 3
//#define NN_PROTVER 2

#define NN_PT_GP  0
#define NN_PT_NN1 1
#define NN_PT_NN2 2
#define NN_PT_NN3 3

#define NN_INIT	0
#define NN_INITACK 1
#define NN_ERTTEST 2
#define NN_ERTACK 3
#define NN_STATEUPDATE 4
#define NN_CONNECT 5
#define NN_CONNECT_ACK 6
#define NN_CONNECT_PING 7
#define NN_BACKUP_TEST 8
#define NN_BACKUP_ACK 9
#define NN_ADDRESS_CHECK 10
#define NN_ADDRESS_REPLY 11
#define NN_NATIFY_REQUEST 12
#define NN_REPORT 13
#define NN_REPORT_ACK 14
#define NN_PREINIT 15
#define NN_PREINIT_ACK 16

#define NN_PREINIT_WAITING_FOR_CLIENT 0
#define NN_PREINIT_WAITING_FOR_MATCHUP 1
#define NN_PREINIT_READY 2


#if !defined(_PS2) && !defined(_NITRO)
# pragma pack(1)
#endif


#define INITPACKET_SIZE				(BASEPACKET_SIZE + 9)
#define INITPACKET_ADDRESS_OFFSET	(BASEPACKET_SIZE + 3)
typedef struct _InitPacket
{
	unsigned char porttype;
	unsigned char clientindex;
	unsigned char usegameport;
	unsigned int localip;
	unsigned short localport;
} InitPacket;

#define REPORTPACKET_SIZE			(BASEPACKET_SIZE + 61)
typedef struct _ReportPacket
{
	unsigned char porttype;
	unsigned char clientindex;
	unsigned char negResult;
	NatType natType;
	NatMappingScheme natMappingScheme;
	char gamename[50];
} ReportPacket;

#define CONNECTPACKET_SIZE			(BASEPACKET_SIZE + 8)
typedef struct _ConnectPacket
{
	unsigned int remoteIP;
	unsigned short remotePort;
	unsigned char gotyourdata;
	unsigned char finished;
} ConnectPacket;

#define PREINITPACKET_SIZE			(BASEPACKET_SIZE + 6)
typedef struct _PreinitPacket
{
	unsigned char clientindex;
	unsigned char state;
	int clientID;
} PreinitPacket;

#define BASEPACKET_SIZE				12
#define BASEPACKET_TYPE_OFFSET		7
typedef struct _NatNegPacket {
	// Base members:
	unsigned char magic[NATNEG_MAGIC_LEN];
	unsigned char version;
	unsigned char packettype;
	int cookie;	

	union 
	{
		InitPacket Init;
		ConnectPacket Connect;
		ReportPacket Report;
		PreinitPacket Preinit;
	} Packet;

} NatNegPacket;


#if !defined(_PS2) && !defined(_NITRO)
# pragma pack()
#endif

#endif
