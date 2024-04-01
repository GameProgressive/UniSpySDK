///////////////////////////////////////////////////////////////////////////////
// File:	gsSocketXbox.c
// SDK:		GameSpy Common Xbox code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char * inet_ntoa(IN_ADDR in_addr)
{
	static char buffer[16];
	sprintf(buffer, "%d.%d.%d.%d", in_addr.S_un.S_un_b.s_b1, in_addr.S_un.S_un_b.s_b2, 
		in_addr.S_un.S_un_b.s_b3, in_addr.S_un.S_un_b.s_b4);
	return buffer;
}

gsi_u32 gsiGetBroadcastIP(void)
{
	return UINT_MAX;
}

