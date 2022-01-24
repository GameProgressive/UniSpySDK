// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"
#include "../gsPlatformSocket.h"
#include <netex/libnetctl.h>



// ToDo: Move this to PS3 implemenation
HOSTENT * getlocalhost(void)
{	// Global storage

	#define MAX_IPS		10
	static HOSTENT		localhost;
	static char 		* aliases = NULL;
	static char 		* ipPtrs[MAX_IPS + 1];
	static unsigned int ips		[MAX_IPS];
	int r;
	union CellNetCtlInfo gCellNetInfo;
	// Todo: support mutliple ip's.

	// initialize the host
	localhost.h_name		= "localhost";
	localhost.h_aliases		= &aliases;
	localhost.h_addrtype	= AF_INET;
	localhost.h_length		= sizeof(unsigned int);
	localhost.h_addr_list	= ipPtrs;
	ipPtrs[0] = (char *)&ips[0];
	ipPtrs[1] = NULL;

	// to do, cache this, and do this once at init.
	r = cellNetCtlGetInfo(	CELL_NET_CTL_INFO_IP_ADDRESS,&gCellNetInfo	);				
	if (r == CELL_OK)		
	{
		ips[0] = inet_addr(gCellNetInfo.ip_address);
		

		return &localhost;
	}
	else	
	return NULL;

}
