///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformNitro.h
// SDK:		GameSpy Common Nitro code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Nintendo DS:      _NITRO

#ifndef __RSPLATFORMNITRO_H__
#define __RSPLATFORMNITRO_H__

#include <nitro.h>
#define NINET_NWBASE_MD5_H_  // resolves md5 conflicts
#include <nitroWiFi.h>
#include <extras.h>  // mwerks
#include <limits.h>
#include <time.h> //NAB - check if this causes any issues. It does not exist in the Nitro branch
// Raw sockets are undefined on Nitro
#define SB_NO_ICMP_SUPPORT

#include "../../common/nitro/screen.h"
#define printf Printf
#define vprintf VPrintf

#define GS_NO_FILE 1
#define GS_WIRELESS_DEVICE 1
#define GSI_NO_ASYNC_DNS 1

typedef s64                   gsi_i64;
typedef u64                   gsi_u64;

typedef OSMutex GSICriticalSection;
typedef struct
{
	OSMutex mLock;
	gsi_i32 mValue;
	gsi_i32 mMax;
} GSISemaphoreID;
typedef struct
{
	OSThread mThread;
	void * mStack;
} GSIThreadID;
typedef void (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return

#define WSAEWOULDBLOCK      SOC_EWOULDBLOCK             
#define WSAEINPROGRESS      SOC_EINPROGRESS             
#define WSAEALREADY         SOC_EALREADY                
#define WSAENOTSOCK         SOC_ENOTSOCK                
#define WSAEDESTADDRREQ     SOC_EDESTADDRREQ            
#define WSAEMSGSIZE         SOC_EMSGSIZE                
#define WSAEPROTOTYPE       SOC_EPROTOTYPE              
#define WSAENOPROTOOPT      SOC_ENOPROTOOPT             
#define WSAEPROTONOSUPPORT  SOC_EPROTONOSUPPORT         
#define WSAEOPNOTSUPP       SOC_EOPNOTSUPP              
#define WSAEAFNOSUPPORT     SOC_EAFNOSUPPORT            
#define WSAEADDRINUSE       SOC_EADDRINUSE              
#define WSAEADDRNOTAVAIL    SOC_EADDRNOTAVAIL           
#define WSAENETDOWN         SOC_ENETDOWN                
#define WSAENETUNREACH      SOC_ENETUNREACH             
#define WSAENETRESET        SOC_ENETRESET               
#define WSAECONNABORTED     SOC_ECONNABORTED            
#define WSAECONNRESET       SOC_ECONNRESET              
#define WSAENOBUFS          SOC_ENOBUFS                 
#define WSAEISCONN          SOC_EISCONN                 
#define WSAENOTCONN         SOC_ENOTCONN                
#define WSAETIMEDOUT        SOC_ETIMEDOUT               
#define WSAECONNREFUSED     SOC_ECONNREFUSED            
#define WSAELOOP            SOC_ELOOP                   
#define WSAENAMETOOLONG     SOC_ENAMETOOLONG            
#define WSAEHOSTUNREACH     SOC_EHOSTUNREACH            
#define WSAENOTEMPTY        SOC_ENOTEMPTY               
#define WSAEDQUOT           SOC_EDQUOT                  
#define WSAESTALE           SOC_ESTALE                  
#define WSAEINVAL           SOC_EINVAL

#define AF_INET SOC_PF_INET
#define SOCK_DGRAM SOC_SOCK_DGRAM
#define SOCK_STREAM SOC_SOCK_STREAM
#define IPPROTO_UDP 0
#define IPPROTO_TCP 0
#define INADDR_ANY SOC_INADDR_ANY
#define SOL_SOCKET SOC_SOL_SOCKET
#define SO_SNDBUF SOC_SO_SNDBUF
#define SO_RCVBUF SOC_SO_RCVBUF
#define SO_REUSEADDR SOC_SO_REUSEADDR

typedef int SOCKET;
typedef int socklen_t;
typedef struct SOSockAddr SOCKADDR;
#define sockaddr SOSockAddr
typedef struct SOSockAddrIn SOCKADDR_IN;
#define sockaddr_in SOSockAddrIn
	#define sin_family family
	#define sin_port port
	#define sin_addr addr
typedef struct SOInAddr IN_ADDR;
#define in_addr SOInAddr
	#define s_addr addr
typedef struct SOHostEnt HOSTENT;
#define hostent SOHostEnt
	#define h_name name
	#define h_aliases aliases
	#define h_addrtype addrType
	#define h_length length
	#define h_addr_list addrList
	#define h_addr addrList[0]

int socket(int pf, int type, int protocol);
int closesocket(SOCKET sock);
int closesocketsure(SOCKET sock, gsi_bool *notInCloseSocket);
int shutdown(SOCKET sock, int how);
int bind(SOCKET sock, const SOCKADDR* addr, int len);

int connect(SOCKET sock, const SOCKADDR* addr, int len);
int listen(SOCKET sock, int backlog);
SOCKET accept(SOCKET sock, SOCKADDR* addr, int* len);

int recv(SOCKET sock, char* buf, int len, int flags);
int recvfrom(SOCKET sock, char* buf, int len, int flags, SOCKADDR* addr, int* fromlen);
int send(SOCKET sock, const char* buf, int len, int flags);
int sendto(SOCKET sock, const char* buf, int len, int flags, const SOCKADDR* addr, int tolen);

int getsockopt(SOCKET sock, int level, int optname, char* optval, int* optlen);
int setsockopt(SOCKET sock, int level, int optname, const char* optval, int optlen);

#define gethostbyaddr(a,l,t) SOC_GetHostByAddr(a,l,t)
#define gethostbyname(n) SOC_GetHostByName(n)

int getsockname(SOCKET sock, SOCKADDR* addr, int* len);

#define htonl(l) SOC_HtoNl(l)
#define ntohl(l) SOC_NtoHl(l)
#define htons(s) SOC_HtoNs(s)
#define ntohs(s) SOC_NtoHs(s)

#define inet_ntoa(n) SOC_InetNtoA(n)
unsigned long inet_addr(const char * name);

int GOAGetLastError(SOCKET sock);

#define GS_HAVE_SOCKET_TYPEDEF 1
#define GSI_UNUSED(x) {void* y=&x;y=NULL;}

#define GS_NO_TIME_H 1
#define gmtime(t)	gsiSecondsToDate(t)
#define ctime(t)	gsiSecondsToString(t)
#define mktime(t)	gsiDateToSeconds(t)	

#if defined(DWC_GAMESPYSDK_BUILD)
	#define time(t) gsiTimeNitro(t)
		time_t time(time_t *timer);
		// string util (light but no float support)
	#define sprintf     OS_SPrintf
	#define snprintf    OS_SNPrintf
	#define vsprintf    OS_VSPrintf
	#define vsnprintf   OS_VSNPrintf
	#define sscanf      STD_TSScanf
#endif
	
#endif // __RSPLATFORMNITRO_H__
