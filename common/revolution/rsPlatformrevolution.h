///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformRevolution.h
// SDK:		GameSpy Common Revolution code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Nintendo Wii:     _REVOLUTION

#ifndef __RSPLATFORMREVOLUTION_H__
#define __RSPLATFORMREVOLUTION_H__

#include <revolution.h>
#include <revolution/soex.h>
#include <revolution/ncd.h>	
#include <limits.h>
#include <time.h>

#define GSI_DOMAIN_NAME "gs.nintendowifi.net"
// Raw sockets are undefined on Revolution
#define SB_NO_ICMP_SUPPORT 1

#define GSI_BIG_ENDIAN 1
#define GS_NO_FILE 1

typedef signed long long      gsi_i64;
typedef unsigned long long    gsi_u64;

#define PRE_ALIGN(x)  // not needed
#define POST_ALIGN(x) __attribute__((aligned(32)))

typedef OSMutex GSICriticalSection;
typedef OSSemaphore GSISemaphoreID;
typedef struct
{
	OSThread *mThread;
	void * mStack;
} GSIThreadID;
typedef void *(*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void *
#define GS_THREAD_RETURN return 0
#define GS_THREAD_RETURN_NEGATIVE return -1


#define WSAEWOULDBLOCK      SO_EWOULDBLOCK             
#define WSAEINPROGRESS      SO_EINPROGRESS             
#define WSAEALREADY         SO_EALREADY                
#define WSAENOTSOCK         SO_ENOTSOCK                
#define WSAEDESTADDRREQ     SO_EDESTADDRREQ            
#define WSAEMSGSIZE         SO_EMSGSIZE                
#define WSAEPROTOTYPE       SO_EPROTOTYPE              
#define WSAENOPROTOOPT      SO_ENOPROTOOPT             
#define WSAEPROTONOSUPPORT  SO_EPROTONOSUPPORT         
#define WSAEOPNOTSUPP       SO_EOPNOTSUPP              
#define WSAEAFNOSUPPORT     SO_EAFNOSUPPORT            
#define WSAEADDRINUSE       SO_EADDRINUSE              
#define WSAEADDRNOTAVAIL    SO_EADDRNOTAVAIL           
#define WSAENETDOWN         SO_ENETDOWN                
#define WSAENETUNREACH      SO_ENETUNREACH             
#define WSAENETRESET        SO_ENETRESET               
#define WSAECONNABORTED     SO_ECONNABORTED            
#define WSAECONNRESET       SO_ECONNRESET              
#define WSAENOBUFS          SO_ENOBUFS                 
#define WSAEISCONN          SO_EISCONN                 
#define WSAENOTCONN         SO_ENOTCONN                
#define WSAETIMEDOUT        SO_ETIMEDOUT               
#define WSAECONNREFUSED     SO_ECONNREFUSED            
#define WSAELOOP            SO_ELOOP                   
#define WSAENAMETOOLONG     SO_ENAMETOOLONG            
#define WSAEHOSTUNREACH     SO_EHOSTUNREACH            
#define WSAENOTEMPTY        SO_ENOTEMPTY               
#define WSAEDQUOT           SO_EDQUOT                  
#define WSAESTALE           SO_ESTALE                  
#define WSAEINVAL           SO_EINVAL
	
		#define AF_INET SO_PF_INET
	#define SOCK_DGRAM SO_SOCK_DGRAM
	#define SOCK_STREAM SO_SOCK_STREAM
	#define IPPROTO_UDP SO_IPPROTO_UDP
	#define IPPROTO_TCP SO_IPPROTO_TCP
	#define INADDR_ANY SO_INADDR_ANY
	#define SOL_SOCKET SO_SOL_SOCKET
	#define SO_SNDBUF SO_SO_SNDBUF
	#define SO_RCVBUF SO_SO_RCVBUF
	#define SO_REUSEADDR SO_SO_REUSEADDR

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

    // Revolution uses a custom DNS caching version of gethostbyname
    struct hostent* gethostbyname( const char* name );

	#define gethostbyaddr(a,l,t)	SOGetHostByAddr(a,l,t)

	// thread safe DNS lookups
	#define getaddrinfo(n,s,h,r)	SOGetAddrInfo(n,s,h,r)
	#define freeaddrinfo(a)			SOFreeAddrInfo(a)
	
	
	int getsockname(SOCKET sock, SOCKADDR* addr, int* len);

	#define htonl(l) SOHtoNl((u32)l)
	#define ntohl(l) SONtoHl((u32)l)
	#define htons(s) SOHtoNs((u16)s)
	#define ntohs(s) SONtoHs((u16)s)

	#define inet_ntoa(n) SOInetNtoA(n)
	unsigned long inet_addr(const char * name);

	int GOAGetLastError(SOCKET sock);
	
#define GS_HAVE_SOCKET_TYPEDEF 1
#define GSI_UNUSED(x) x

#define GS_NO_TIME_H 1
time_t gsiTimeInSec(time_t *timer);
struct tm *gsiGetGmTime(time_t *theTime);
char *gsiCTime(time_t *theTime);
#define time(t) gsiTimeInSec(t)
#define gmtime(t) gsiGetGmTime(t)
#define ctime(t) gsiCTime(t)
	

#endif // __RSPLATFORMREVOLUTION_H__
