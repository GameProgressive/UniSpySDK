///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformPSP.h
// SDK:		GameSpy Common PSP code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PSP:              _PSP

#ifndef __RSPLATFORMPSP_H__
#define __RSPLATFORMPSP_H__

#include <kernel.h>
#include <pspnet.h>
#include <pspnet_error.h>
#include <pspnet_inet.h>
#include <pspnet/sys/select.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>
#include <pspnet_ap_dialog_dummy.h>
#include <rtcsvc.h>
#include <errno.h>
#include <wlan.h>

#include <pspnet/sys/socket.h>
#include <pspnet/netinet/in.h>
#include <utility\utility_common.h>
#include <utility\utility_netconf.h>
#include <utility\utility_module.h>
#include <utility\utility_sysparam.h>

typedef long long             gsi_i64;
typedef unsigned long long    gsi_u64;

#ifdef GSI_UNICODE
	#define _tsnprintf _snwprintf
	#define _stprintf   snwprintf
	#define _vswprintf  vsnwprintf
#endif

#define PRE_ALIGN(x)	// ignored this on psp/ps2
#define POST_ALIGN(x)	__attribute__((aligned (x)))		//

#define GS_NO_FILE 1
#define GS_WIRELESS_DEVICE 1
#define GSI_NO_THREADS 1
#define GSI_NO_ASYNC_DNS 1

#define  gsiSocketIsError(theReturnValue)		((theReturnValue) <  0)
#define  gsiSocketIsNotError(theReturnValue)	((theReturnValue) >= 0)
#define gethostbyaddr(a,b,c)   NULL

typedef int GSIThreadID;
typedef int GSISemaphoreID;
typedef struct 
{
	// A critical section is a re-entrant semaphore
	GSISemaphoreID mSemaphore;
	GSIThreadID mOwnerThread;
	gsi_u32 mEntryCount; // track re-entry
	gsi_u32 mPad; // make 16bytes
} GSICriticalSection;
typedef void (*GSThreadFunc)(void *arg);

#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return

#define AF_INET     SCE_NET_INET_AF_INET
	#define SOCK_STREAM SCE_NET_INET_SOCK_STREAM
	#define SOCK_DGRAM  SCE_NET_INET_SOCK_DGRAM
	#define SOCK_RAW    SCE_NET_INET_SOCK_RAW
	#define INADDR_ANY  SCE_NET_INET_INADDR_ANY
	#define SOL_SOCKET  SCE_NET_INET_SOL_SOCKET
	#define SO_SNDBUF   SCE_NET_INET_SO_SNDBUF
	#define SO_RCVBUF   SCE_NET_INET_SO_RCVBUF
	#define SO_NBIO     SCE_NET_INET_SO_NBIO
	#define SO_BROADCAST SCE_NET_INET_SO_BROADCAST
    #define SO_KEEPALIVE SCE_NET_INET_SO_KEEPALIVE
    #define SO_REUSEADDR SCE_NET_INET_SO_REUSEADDR

	#define IPPROTO_TCP SCE_NET_INET_IPPROTO_TCP // protocol defined by SOCK_STREAM
	#define IPPROTO_UDP SCE_NET_INET_IPPROTO_UDP // protocol defined by SOCK_DGRAM
	#define IPPROTO_ICMP SCE_NET_INET_IPPROTO_ICMP // protocol for ICMP pings

	// structures
	#define in_addr     SceNetInetInAddr
	#define sockaddr_in	SceNetInetSockaddrIn
	#define sockaddr    SceNetInetSockaddr

	// Remove FD types set in sys/types.h
	// Replace with types in pspnet/sys/select.h
	#if defined(_SYS_TYPES_H) && defined(FD_SET)
		#undef fd_set
		#undef FD_SET
		#undef FD_CLR
		#undef FD_ZERO
		#undef timeval
		#undef FD_SETSIZE	
	#endif
	#define fd_set  SceNetInetFdSet
	#define timeval SceNetInetTimeval
	#define FD_SET  SceNetInetFD_SET
	#define FD_CLR  SceNetInetFD_CLR
	#define FD_ZERO SceNetInetFD_ZERO
	#define FD_SETSIZE SCE_NET_INET_FD_SETSIZE

	// functions
	#define htonl		sceNetHtonl
	#define ntohl		sceNetNtohl
	#define htons		sceNetHtons
	#define ntohs		sceNetNtohs
	#define socket      sceNetInetSocket
    #define shutdown    sceNetInetShutdown
	#define closesocket sceNetInetClose
	
	#define setsockopt					  sceNetInetSetsockopt
	#define getsockopt(s, l, on, ov, ol)  sceNetInetGetsockopt(s, l, on, ov, (SceNetInetSocklen_t *)ol)

	#define bind			sceNetInetBind
	#define select			sceNetInetSelect

	#define connect			sceNetInetConnect
    #define listen			sceNetInetListen
	#define accept(s,a,l)	sceNetInetAccept(s, a, (SceNetInetSocklen_t *)l)
    
	#define send		sceNetInetSend  
	#define recv		sceNetInetRecv
	#define sendto		sceNetInetSendto 
	#define recvfrom(s, b, l, f, fr, fl)	sceNetInetRecvfrom(s, b, l, f, fr, (SceNetInetSocklen_t *)fl)

	
	#define inet_addr   sceNetInetInetAddr
	// This is not the correct function for gethostname, it should get the string name of the local host
	// not the sockaddr_in struct
	#define gethostname // sceNetInetGetsockname 
	#define getsockname(s,n,l) sceNetInetGetsockname(s, n, (SceNetInetSocklen_t *)l)
	
    #define GOAGetLastError(s) sceNetInetGetErrno()
	
	// hostent support
	struct hostent
	{
		char* h_name;       
		char** h_aliases;    
		gsi_u16 h_addrtype; // AF_INET
		gsi_u16 h_length;   
		char** h_addr_list; 
	};

	#define gethostbyname gsSocketGetHostByName
	#define inet_ntoa     gsSocketInetNtoa

	#define GSI_RESOLVER_TIMEOUT  (5*1000*1000) // 5 seconds
	#define GSI_RESOLVER_RETRY    (2)

	struct hostent* gsSocketGetHostByName(const char* name); // gsSocketPSP.c
	const char* gsSocketInetNtoa(struct in_addr in);
	typedef SceNetInetSocklen_t socklen_t;
	
		#define WSAEWOULDBLOCK      EWOULDBLOCK             
	#define WSAEINPROGRESS      EINPROGRESS             
	#define WSAEALREADY         EALREADY                
	#define WSAENOTSOCK         ENOTSOCK                
	#define WSAEDESTADDRREQ     EDESTADDRREQ            
	#define WSAEMSGSIZE         EMSGSIZE                
	#define WSAEPROTOTYPE       EPROTOTYPE              
	#define WSAENOPROTOOPT      ENOPROTOOPT             
	#define WSAEPROTONOSUPPORT  EPROTONOSUPPORT         
	#define WSAESOCKTNOSUPPORT  ESOCKTNOSUPPORT         
	#define WSAEOPNOTSUPP       EOPNOTSUPP              
	#define WSAEPFNOSUPPORT     EPFNOSUPPORT            
	#define WSAEAFNOSUPPORT     EAFNOSUPPORT            
	#define WSAEADDRINUSE       EADDRINUSE              
	#define WSAEADDRNOTAVAIL    EADDRNOTAVAIL           
	#define WSAENETDOWN         ENETDOWN                
	#define WSAENETUNREACH      ENETUNREACH             
	#define WSAENETRESET        ENETRESET               
	#define WSAECONNABORTED     ECONNABORTED            
	#define WSAECONNRESET       ECONNRESET              
	#define WSAENOBUFS          ENOBUFS                 
	#define WSAEISCONN          EISCONN                 
	#define WSAENOTCONN         ENOTCONN                
	#define WSAESHUTDOWN        ESHUTDOWN               
	#define WSAETOOMANYREFS     ETOOMANYREFS            
	#define WSAETIMEDOUT        ETIMEDOUT               
	#define WSAECONNREFUSED     ECONNREFUSED            
	#define WSAELOOP            ELOOP                   
	#define WSAENAMETOOLONG     ENAMETOOLONG            
	#define WSAEHOSTDOWN        EHOSTDOWN               
	#define WSAEHOSTUNREACH     EHOSTUNREACH            
	#define WSAENOTEMPTY        ENOTEMPTY               
	#define WSAEPROCLIM         EPROCLIM                
	#define WSAEUSERS           EUSERS                  
	#define WSAEDQUOT           EDQUOT                  
	#define WSAESTALE           ESTALE                  
	#define WSAEREMOTE          EREMOTE
	#define WSAEINVAL           EINVAL
#define memalign_direct memalign

#endif // __RSPLATFORMPSP_H__
