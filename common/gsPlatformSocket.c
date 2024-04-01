///////////////////////////////////////////////////////////////////////////////
// File:	gsPlatformSocket.c
// SDK:		GameSpy Common
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "gsPlatformSocket.h"
#include "gsPlatformUtil.h"
#include "gsMemory.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int SetSockBlocking(SOCKET sock, int isblocking)
{
	int rcode;

#if defined(_REVOLUTION)
	int val;
	
	val = SOFcntl(sock, SO_F_GETFL, 0);
	
	if(isblocking)
		val &= ~SO_O_NONBLOCK;
	else
		val |= SO_O_NONBLOCK;
	
	rcode = SOFcntl(sock, SO_F_SETFL, val);
#elif defined(_NITRO)
	int val;
	
	val = SOC_Fcntl(sock, SOC_F_GETFL, 0);
	
	if(isblocking)
		val &= ~SOC_O_NONBLOCK;
	else
		val |= SOC_O_NONBLOCK;
	
	rcode = SOC_Fcntl(sock, SOC_F_SETFL, val);
#else
	#if defined(_PS2) || defined(_PS3)
		// EENet requires int
		// SNSystems requires int
		// Insock requires int
		// PS3 requires int
		gsi_i32 argp;
	#else
		unsigned long argp;
	#endif
		
		if(isblocking)
			argp = 0;
		else
			argp = 1;

	#ifdef _PS2
		#ifdef SN_SYSTEMS
			rcode = setsockopt(sock, SOL_SOCKET, (isblocking) ? SO_BIO : SO_NBIO, &argp, sizeof(argp));
		#endif

		#ifdef EENET
			rcode = setsockopt(sock, SOL_SOCKET, SO_NBIO, &argp, sizeof(argp));
		#endif

		#ifdef INSOCK
			if (isblocking)
				argp = -1;
			else
				argp = 5; //added longer timeout to 5ms
			sceInsockSetRecvTimeout(sock, argp);
			sceInsockSetSendTimeout(sock, argp);
			sceInsockSetShutdownTimeout(sock, argp);
			GSI_UNUSED(sock);
			rcode = 0;
		#endif
	#elif defined(_PSP)
		rcode = setsockopt(sock, SCE_NET_INET_SOL_SOCKET, SCE_NET_INET_SO_NBIO, &argp, sizeof(argp));
	#elif defined(_PS3)
		rcode = setsockopt(sock, SOL_SOCKET, SO_NBIO, &argp, sizeof(argp));
	#else
		rcode = ioctlsocket(sock, FIONBIO, &argp);
	#endif
#endif

	if(rcode == 0)
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Network, GSIDebugLevel_Comment,
			"SetSockBlocking: Set socket %d to %s\r\n", (unsigned int)sock, isblocking ? "blocking":"non-blocking");
		return 1;
	}

	gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Network, GSIDebugLevel_Comment,
			"SetSockBlocking failed: tried to set socket %d to %s\r\n", (unsigned int)sock, isblocking ? "blocking":"non-blocking");
	return 0;
}

int SetSockBroadcast(SOCKET sock)
{
#if !defined(INSOCK) && !defined(_NITRO) && !defined(_REVOLUTION)
	int optval = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
		return 0;
#else
	GSI_UNUSED(sock);
#endif

	return 1;
}

int DisableNagle(SOCKET sock)
{
#if defined(_WIN32) || defined(_UNIX)
	int rcode;
	int noDelay = 1;

	rcode = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&noDelay, sizeof(int));
	return gsiSocketIsError(rcode);	
#else
	GSI_UNUSED(sock);

	// not supported
	return 0;
#endif // moved this to here to silence VC warning
}


#ifndef INSOCK
	int SetReceiveBufferSize(SOCKET sock, int size)
	{
		int rcode;
		rcode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(int));
		return gsiSocketIsNotError(rcode);
	}

	int SetSendBufferSize(SOCKET sock, int size)
	{
		int rcode;
		rcode = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(int));
		return gsiSocketIsNotError(rcode);
	}

	int GetReceiveBufferSize(SOCKET sock)
	{
		int rcode;
		int size;
		socklen_t len = sizeof(size);

		rcode = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&size, &len);

		if(gsiSocketIsError(rcode))
			return -1;

		return size;
	}

	int GetSendBufferSize(SOCKET sock)
	{
		int rcode;
		int size;
		socklen_t len = sizeof(size);

		rcode = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, &len);

		if(gsiSocketIsError(rcode))
			return -1;

		return size;
	}
	
	// Formerly known as ghiSocketSelect
#ifdef SN_SYSTEMS
	#undef FD_SET
	#define FD_SET(s,p)   ((p)->array[((s) - 1) >> SN_FD_SHR] |= \
                       (unsigned int)(1 << (((s) - 1) & SN_FD_BITS)) )

#endif
#endif

#if !defined(_NITRO) && !defined(INSOCK) && !defined(_REVOLUTION)
	int GSISocketSelect(SOCKET theSocket, int* theReadFlag, int* theWriteFlag, int* theExceptFlag)
	{
		fd_set aReadSet;
		fd_set aWriteSet;
		fd_set aExceptSet;
		fd_set * aReadFds   = NULL;
		fd_set * aWriteFds  = NULL;
		fd_set * aExceptFds = NULL;
		int aResult;
// 04-13-2005:
// Added case for SN Systems that would 
// handle errors after performing selects.
#ifdef SN_SYSTEMS
		int aOut, aOutLen = sizeof(aOut);
#endif
		
		struct timeval aTimeout = { 0, 0 };

		GS_ASSERT(theSocket != INVALID_SOCKET);

		// Setup the parameters.
		////////////////////////
		if(theReadFlag != NULL)
		{
			FD_ZERO(&aReadSet);
			FD_SET(theSocket,&aReadSet);
			aReadFds = &aReadSet;
		}
		if(theWriteFlag != NULL)
		{
			FD_ZERO(&aWriteSet);
			FD_SET(theSocket, &aWriteSet);
			aWriteFds = &aWriteSet;
		}
		if(theExceptFlag != NULL)
		{
			FD_ZERO(&aExceptSet);
			FD_SET(theSocket, &aExceptSet);
			aExceptFds = &aExceptSet;
		}
#ifdef _PS3
		// to do, port what is below in the else
//int socketselect(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout);
	aResult = socketselect(FD_SETSIZE, aReadFds, aWriteFds, aExceptFds, &aTimeout);
#else
	// Perform the select
	aResult = select(FD_SETSIZE, aReadFds, aWriteFds, aExceptFds, &aTimeout);
#endif
	if(gsiSocketIsError(aResult))
		return -1;

// 04-13-2005:
// Added case for SN Systems that would 
// handle errors after performing selects.
#ifdef SN_SYSTEMS
		getsockopt(theSocket, SOL_SOCKET, SO_ERROR, (char *)&aOut, &aOutLen);
		if (aOut != 0)
		{
			return 0;
		}		
#endif
		// Check results.
		/////////////////
		if(theReadFlag != NULL)
		{
			if((aResult > 0) && FD_ISSET(theSocket, aReadFds))
				*theReadFlag = 1;
			else
				*theReadFlag = 0;
		}
		if(theWriteFlag != NULL)
		{
			if((aResult > 0) && FD_ISSET(theSocket, aWriteFds))
				*theWriteFlag = 1;
			else
				*theWriteFlag = 0;
		}
		if(theExceptFlag != NULL)
		{
			if((aResult > 0) && FD_ISSET(theSocket, aExceptFds))
				*theExceptFlag = 1;
			else
				*theExceptFlag = 0;
		}
		return aResult; // 0 or 1 at this point
	}
#endif // !nitro && !revolution && !insock
	

// Return 1 for immediate recv, otherwise 0
int CanReceiveOnSocket(SOCKET sock)
{
	int aReadFlag = 0;
	if (1 == GSISocketSelect(sock, &aReadFlag, NULL, NULL))
		return aReadFlag;

	// SDKs expect 0 on SOCKET_ERROR
	return 0;
}

// Return 1 for immediate send, otherwise 0
int CanSendOnSocket(SOCKET sock)
{
	int aWriteFlag = 0;
	if (1 == GSISocketSelect(sock, NULL, &aWriteFlag, NULL))
		return aWriteFlag;

	// SDKs expect 0 on SOCKET_ERROR
	return 0;
}


#if defined(_PS3) || defined (_PSP)

#else

HOSTENT * getlocalhost(void)
{
#ifdef EENET
	#define MAX_IPS  5

	static HOSTENT localhost;
	static char * aliases = NULL;
	static char * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];

	struct sceEENetIfname * interfaces;
	struct sceEENetIfname * interface;
	int num;
	int i;
	int count;
	int len;
	u_short flags;
	IN_ADDR address;

	// initialize the host
	localhost.h_name = "localhost";
	localhost.h_aliases = &aliases;
	localhost.h_addrtype = AF_INET;
	localhost.h_length = 0;
	localhost.h_addr_list = ipPtrs;

	// get the local interfaces
	sceEENetGetIfnames(NULL, &num);
	interfaces = (struct sceEENetIfname *)gsimalloc(num * sizeof(struct sceEENetIfname));
	if(!interfaces)
		return NULL;
	sceEENetGetIfnames(interfaces, &num);

	// loop through the interfaces
	count = 0;
	for(i = 0 ; i < num ; i++)
	{
		// the next interface
		interface = &interfaces[i];
		//printf("eenet%d: %s\n", i, interface->ifn_name);

		// get the flags
		len = sizeof(flags);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_IFFLAGS, &flags, &len) != 0)
			continue;
		//printf("eenet%d flags: 0x%X\n", i, flags);

		// check for up, running, and non-loopback
		if(!(flags & (IFF_UP|IFF_RUNNING)) || (flags & IFF_LOOPBACK))
			continue;
		//printf("eenet%d: up and running, non-loopback\n", i);

		// get the address
		len = sizeof(address);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_ADDR, &address, &len) != 0)
			continue;
		//printf("eenet%d: %s\n", i, inet_ntoa(address));

		// add this address
		ips[count] = address.s_addr;
		ipPtrs[count] = (char *)&ips[count];
		count++;
	}

	// free the interfaces
	gsifree(interfaces);

	// check that we got at least one IP
	if(!count)
		return NULL;

	// finish filling in the host struct
	localhost.h_length = (gsi_u16)sizeof(ips[0]);
	ipPtrs[count] = NULL;

	return &localhost;

	////////////////////
	// INSOCK
#elif defined(INSOCK)
	// Global storage
	#define MAX_IPS  sceLIBNET_MAX_INTERFACE
	static HOSTENT   localhost;
	static char    * aliases = NULL;
	static char    * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];

	// Temp storage
	int aInterfaceIdArray[MAX_IPS];
	int aNumInterfaces = 0;
	int aInterfaceNum = 0;
	int aCount = 0;
	
	// Get the list of interfaces
	aNumInterfaces = sceInetGetInterfaceList(&gGSIInsockClientData, 
		                 &gGSIInsockSocketBuffer, aInterfaceIdArray, MAX_IPS);
	if (aNumInterfaces < 1)
		return NULL;

	// initialize the HOSTENT
	localhost.h_name      = "localhost";
	localhost.h_aliases   = &aliases;
	localhost.h_addrtype  = AF_INET;
	localhost.h_addr_list = ipPtrs;

	// Look up each address and copy into the HOSTENT structure
	aCount = 0; // count of valid interfaces
	for (aInterfaceNum = 0; aInterfaceNum < aNumInterfaces; aInterfaceNum++)
	{
		sceInetAddress_t anAddr;
		int result = sceInetInterfaceControl(&gGSIInsockClientData, &gGSIInsockSocketBuffer,
			                    aInterfaceIdArray[aInterfaceNum], sceInetCC_GetAddress,
								&anAddr, sizeof(anAddr));
		if (result == 0)
		{
			// Add this interface to the array
			memcpy(&ips[aCount], anAddr.data, sizeof(ips[aCount]));
			ips[aCount] = htonl(ips[aCount]);
			ipPtrs[aCount] = (char*)&ips[aCount];
			aCount++;
		}
	}

	// Set the final hostent data, then return
	localhost.h_length = (gsi_u16)sizeof(ips[0]);
	ipPtrs[aCount]     = NULL;
	return &localhost;
	
#elif defined(_NITRO)
	#define MAX_IPS  5

	static HOSTENT localhost;
	static char * aliases = NULL;
	static char * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];

	int count = 0;

	localhost.h_name = "localhost";
	localhost.h_aliases = &aliases;
	localhost.h_addrtype = AF_INET;
	localhost.h_length = 0;
	localhost.h_addr_list = (u8 **)ipPtrs;

	ips[count] = 0;
	IP_GetAddr(NULL, (u8*)&ips[count]);
	if(ips[count] == 0)
		return NULL;
	ipPtrs[count] = (char *)&ips[count];
	count++;

	localhost.h_length = (gsi_u16)sizeof(ips[0]);
	ipPtrs[count] = NULL;

	return &localhost;

#elif defined(_REVOLUTION)
	#define MAX_IPS  5
	static HOSTENT aLocalHost;
	static char * aliases = NULL;
	int aNumOfIps, i;
	int aSizeNumOfIps;
	static IPAddrEntry aAddrs[MAX_IPS];
	int aAddrsSize, aAddrsSizeInitial;
	static u8 * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];
	int ret;
	aSizeNumOfIps = sizeof(aNumOfIps);
	ret = SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, SO_CONFIG_IP_ADDR_NUMBER, &aNumOfIps, &aSizeNumOfIps);
	if (ret != 0)
		return NULL;
	
	aAddrsSize = (int)(MAX_IPS * sizeof(IPAddrEntry));
	aAddrsSizeInitial = aAddrsSize;
	ret = SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, SO_CONFIG_IP_ADDR_TABLE, &aAddrs, &aAddrsSize);
	if (ret != 0)
		return NULL;
	
	if (aAddrsSize != aAddrsSizeInitial)
	{
		aNumOfIps = aAddrsSize / (int)sizeof(IPAddrEntry);
	}
	
	aLocalHost.h_name = "localhost";
	aLocalHost.h_aliases = &aliases;
	aLocalHost.h_addrtype = AF_INET;
	aLocalHost.h_length = SO_IP4_ALEN;

	for (i = 0; i < MAX_IPS; i++)
	{
		if (i < aNumOfIps)
		{
			memcpy(&ips[i], &aAddrs[i].addr, sizeof(aAddrs[i].addr));
			ipPtrs[i] = (u8 *)&ips[i];
		}			
		else 
			ipPtrs[i] = NULL;
	}
	aLocalHost.h_addr_list = ipPtrs;
	
	return &aLocalHost;

#elif defined(_X360)
	XNADDR addr;
	DWORD rcode;
	static HOSTENT localhost;
	static char * ipPtrs[2];
	static IN_ADDR ip;

	while((rcode = XNetGetTitleXnAddr(&addr)) == XNET_GET_XNADDR_PENDING)
		msleep(1);

	if((rcode == XNET_GET_XNADDR_NONE) || (rcode == XNET_GET_XNADDR_TROUBLESHOOT))
		return NULL;

	localhost.h_name = "localhost";
	localhost.h_aliases = NULL;
	localhost.h_addrtype = AF_INET;
	localhost.h_length = (gsi_u16)sizeof(IN_ADDR);
	localhost.h_addr_list = (gsi_i8 **)ipPtrs;

	ip = addr.ina;
	ipPtrs[0] = (char *)&ip;
	ipPtrs[1] = NULL;

	return &localhost;

#elif defined(_XBOX)
	return NULL;


#elif defined(_IPHONE)
	static HOSTENT localhost;
	static char * ipPtrs[2];
	static unsigned int ips[5];
	char hostname[256] = "";
	struct ifaddrs *address, *addresses;
	int error = getifaddrs(&addresses);
	
	if (error == 0) {
		address = addresses;
		while (address != NULL)
		{
			if ((address->ifa_addr->sa_family == AF_INET) &&
				(strcmp(address->ifa_name, "lo0") != 0))
			{
				struct sockaddr_in* inet_address =
				(struct sockaddr_in*)address->ifa_addr;
				
				// get the local host's name
				gethostname(hostname, sizeof(hostname));
				
				localhost.h_name = hostname;
				localhost.h_aliases = NULL;
				localhost.h_addrtype = AF_INET;
				localhost.h_length =
				(gsi_u16)sizeof(IN_ADDR);
				
				memcpy(&ips[0],
					   &inet_address->sin_addr.s_addr, sizeof(inet_address->sin_addr.s_addr));
				ipPtrs[0] = (char *)&ips[0];
				ipPtrs[1] = NULL;
				localhost.h_addr_list = (gsi_i8
										 **)ipPtrs;
				
				//ipPtrs[0] = inet_address->sin_addr.s_addr;
				//ipPtrs[1] = NULL;
				
				break;
			}
			address = address->ifa_next;
		}
	}
	freeifaddrs(addresses);
       return &localhost;
#else
	char hostname[256] = "";

	// get the local host's name
	gethostname(hostname, sizeof(hostname));

	// return the host for that name
	return gethostbyname(hostname);
#endif
}
#endif

int IsPrivateIP(IN_ADDR * addr)
{
	int b1;
	int b2;
	unsigned int ip;

	// get the first 2 bytes
	ip = ntohl(addr->s_addr);
	b1 = (int)((ip >> 24) & 0xFF);
	b2 = (int)((ip >> 16) & 0xFF);

	// 10.X.X.X
	if(b1 == 10)
		return 1;

	// 172.16-31.X.X
	if((b1 == 172) && ((b2 >= 16) && (b2 <= 31)))
		return 1;

	// 192.168.X.X
	if((b1 == 192) && (b2 == 168))
		return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
