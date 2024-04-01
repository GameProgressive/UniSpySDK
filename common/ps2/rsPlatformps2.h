///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformPs2.h
// SDK:		GameSpy Common EE code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef __RSPLATFORMPS2_H__
#define __RSPLATFORMPS2_H__

// PlayStation2:     _PS2
//    w/ EENET:      EENET       (set by developer project)
//    w/ INSOCK:     INSOCK      (set by developer project)
//    w/ SNSYSTEMS:  SN_SYSTEMS  (set by developer project)
//    Codewarrior:   __MWERKS__

#ifndef _PS2
	#define _PS2
#endif

// Make sure a network stack was defined
#if !defined(SN_SYSTEMS) && !defined(EENET) && !defined(INSOCK)
	#error "PlayStation2 network stack was not defined!"
#endif

// EENet headers must be included before common PS2 headers
#ifdef EENET 
	#include <libeenet.h>
	#include <eenetctl.h>
	#include <ifaddrs.h>
	#include <sys/socket.h>
	#include <sys/errno.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <net/if.h>
	#include <sys/select.h>
	#include <malloc.h>
	
#endif // EENET

// Common PS2 headers
#include <eekernel.h>
#include <stdio.h>
#include <malloc.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <sifcmd.h>
#include <ilink.h>
#include <ilsock.h>
#include <ilsocksf.h>
#include <limits.h>

#ifdef SN_SYSTEMS
	// undefine socket defines from sys/types.h
	// This is to workaround sony now automatically including sys/types.h
	// and SNSystems having not produce a patch yet (they'll likely do the same since
	// the SNSystems fd_set is a slightly different size than the sys/types.h.
	#undef FD_CLR	
	#undef FD_ZERO
	#undef FD_SET	
	#undef FD_ISSET
	#undef FD_SETSIZE
	#undef fd_set
	#include "snskdefs.h"
	#include "sntypes.h"
	#include "snsocket.h"
	#include "sneeutil.h"
	#include "sntcutil.h"
#endif // SN_SYSTEMS

#ifdef INSOCK
	#include "libinsck.h"
	#include "libnet.h"
	#include "sys/errno.h"
	//#include "libmrpc.h"
#endif // INSOCK

#define GS_NO_FILE 1

typedef signed long           gsi_i64;
typedef unsigned long         gsi_u64;

#ifdef GSI_UNICODE
	#define _tsnprintf _snwprintf
#endif

#define PRE_ALIGN(x)	// ignored this on psp/ps2
#define POST_ALIGN(x)	__attribute__((aligned (x)))		// 

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

#if defined(SN_SYSTEMS) 
	#define IPPROTO_TCP PF_INET
	#define IPPROTO_UDP PF_INET
	#define FD_SETSIZE  SN_MAX_SOCKETS
#endif


#ifdef INSOCK
	//#define NETBUFSIZE (sceLIBNET_BUFFERSIZE)
	#define NETBUFSIZE (32768) // buffer size for our samples

// used in place of shutdown function to avoid blocking shutdown call
int gsiShutdown(SOCKET s, int how);

	#define GOAGetLastError(s) sceInsockErrno  // not implemented
	#define closesocket(s)	   gsiShutdown(s,SCE_INSOCK_SHUT_RDWR)
	#undef shutdown
	#define shutdown(s,h) gsiShutdown(s,h)
#endif

#if defined(SN_SYSTEMS) 
	int GOAGetLastError(SOCKET s);
	
	#if !defined(__MWERKS__)
		#define send(s,b,l,f) (int)send(s,b,(unsigned long)l,f)
		#define recv(s,b,l,f) (int)recv(s,b,(unsigned long)l,f)
		#define sendto(s,b,l,f,a,al) (int)sendto(s,b,(unsigned long)l,f,a,al)
		#define recvfrom(s,b,l,f,a,al) (int)recvfrom(s,b,(unsigned long)l,f,a,al)
	#endif
#endif

// SN Systems doesn't support gethostbyaddr
#if defined(SN_SYSTEMS)
	#define gethostbyaddr(a,b,c)   NULL
#endif

#ifdef EENET
	#define GOAGetLastError(s) sceEENetErrno
	#define closesocket        sceEENetClose
#endif

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
#define ALIGNED	__attribute__ ((aligned(16)))

#define memalign_direct memalign
#define GSI_NO_ASYNC_DNS 1

#endif // __RSPLATFORMPS2_H__
