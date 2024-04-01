///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformMacOSX.h
// SDK:		GameSpy Common Mac OSX code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

// MacOSX:           _MACOSX + _UNIX
// iPhone:           _IPHONE + _UNIX

#ifndef __RSPLATFORMMACOSX_H__
#define __RSPLATFORMMACOSX_H__

#define _UNIX 1

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
	#define _IPHONE
#else
	#define _MACOSX
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>
//#include <sys/syslimits.h>
#include <netinet/tcp.h>

// ICMP ping support is unsupported on Linux/MacOSX due to needing super-user access for raw sockets
#define SB_NO_ICMP_SUPPORT

#ifdef _IPHONE
	#define GS_WIRELESS_DEVICE 1
	
	#import <net/if.h>
	#import <ifaddrs.h>
#endif

#include <ctype.h>
#undef _T // TODO: verify if this is required


//---------- Mac autorelease pool for working with CoreFoundation types ----------------------
#include "macAutoreleasePool.h"

// Platform dependent types
typedef long long             gsi_i64;
typedef unsigned long long    gsi_u64;

#define gsi_char gsi_u32	// for unix/linux, unicode is 4 bytes

// gsPlatformThread
typedef pthread_mutex_t GSICriticalSection;
typedef struct
{
	pthread_mutex_t mLock;
	gsi_i32 mValue;
	gsi_i32 mMax;
} GSISemaphoreID;
typedef struct  
{
	pthread_t thread;
	pthread_attr_t attr;
} GSIThreadID;
typedef void * (*GSThreadFunc)(void *arg);
#define GS_THREAD_RETURN_TYPE void *
#define GS_THREAD_RETURN return 0
#define GS_THREAD_RETURN_NEGATIVE return -1

#define accept(s,a,al) accept(s,a,(socklen_t*)(al))
#define bind(s,a,al) bind(s,a,(socklen_t)(al))
#define connect(s,a,al) connect(s,a,(socklen_t)(al))
#define getpeername(s,a,al) getpeername(s,a,(socklen_t*)(al))
#define getsockname(s,a,al) getsockname(s,a,(socklen_t*)(al))
#define getsockopt(s,l,o,v,vl) getsockopt(s,l,o,v,(socklen_t*)(vl))
#define recvfrom(s,b,l,f,a,al) recvfrom(s,b,l,f,a,(socklen_t*)(al))
#define sendto(s,b,l,f,a,al) sendto(s,b,l,f,a,(socklen_t)(al))
#define setsockopt(s,l,o,v,vl) setsockopt(s,l,o,v,(socklen_t)(vl))
	
#define GOAGetLastError(s) errno
#define closesocket        close //on unix

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

#define GSI_UNUSED(x) {void* y=&x;y=NULL;}


#endif // __RSPLATFORMMACOSX_H__
