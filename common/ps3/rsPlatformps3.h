///////////////////////////////////////////////////////////////////////////////
// File:	rsPlatformPS3.c
// SDK:		GameSpy Common CELL code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PS3:              _PS3

#ifndef __RSPLATFORMPS3_H__
#define __RSPLATFORMPS3_H__

#include <netex/errno.h>
#include <sys/process.h>
#include <sys/time.h>
#include <sys/types.h>	
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sys_time.h>
#include <sys/timer.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netex/net.h>
#include <netex/ifctl.h>
//	#include <netex/netset.h>
#include <limits.h>
#include <time.h>

#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>

#define GSI_BIG_ENDIAN 1
#define GS_NO_FILE 1
#define GSI_64BIT 1

typedef int64_t               gsi_i64;
typedef uint64_t              gsi_u64;

#define PRE_ALIGN(x)	// ignored this on psp/ps2
#define POST_ALIGN(x)	__attribute__((aligned (x)))		// 

#ifdef GSI_UNICODE
	#define GS_STR  _T("%s")
	#define GS_USTR _T("%ls")
#endif

#define  gsiSocketIsError(theReturnValue)		((theReturnValue) <  0)
#define  gsiSocketIsNotError(theReturnValue)	((theReturnValue) >= 0)

// Todo: Test PS3 ppu thread code, then remove this define
//#define GSI_NO_THREADS
typedef sys_ppu_thread_t GSIThreadID;
typedef int GSISemaphoreID;
typedef sys_mutex_t GSICriticalSection;
typedef void (*GSThreadFunc)(uint64_t arg);
#define GS_THREAD_RETURN_TYPE void
#define GS_THREAD_RETURN return
#define GS_THREAD_RETURN_NEGATIVE return

#define WSAEWOULDBLOCK      SYS_NET_EWOULDBLOCK	            
#define WSAEINPROGRESS      SYS_NET_EINPROGRESS		          //SYS_NET_ERROR_EINPROGRESS		          
#define WSAEALREADY         SYS_NET_EALREADY                
#define WSAENOTSOCK         SYS_NET_ENOTSOCK                
#define WSAEDESTADDRREQ     SYS_NET_EDESTADDRREQ            
#define WSAEMSGSIZE         SYS_NET_EMSGSIZE 
#define WSAEPROTOTYPE       SYS_NET_EPROTOTYPE              
#define WSAENOPROTOOPT      SYS_NET_ENOPROTOOPT             
#define WSAEPROTONOSUPPORT  SYS_NET_EPROTONOSUPPORT         
#define WSAESOCKTNOSUPPORT  SYS_NET_ESOCKTNOSUPPORT         
#define WSAEOPNOTSUPP       SYS_NET_EOPNOTSUPP              
#define WSAEPFNOSUPPORT     SYS_NET_EPFNOSUPPORT            
#define WSAEAFNOSUPPORT     SYS_NET_EAFNOSUPPORT            
#define WSAEADDRINUSE       SYS_NET_EADDRINUSE              
#define WSAEADDRNOTAVAIL    SYS_NET_EADDRNOTAVAIL           
#define WSAENETDOWN         SYS_NET_ENETDOWN                
#define WSAENETUNREACH      SYS_NET_ENETUNREACH             
#define WSAENETRESET        SYS_NET_ENETRESET               
#define WSAECONNABORTED     SYS_NET_ECONNABORTED            
#define WSAECONNRESET       SYS_NET_ECONNRESET 				// SYS_NET_ERROR_ECONNRESET 
#define WSAENOBUFS          SYS_NET_ENOBUFS    				// SYS_NET_ERROR_ENOBUFS               
#define WSAEISCONN          SYS_NET_EISCONN                 
#define WSAENOTCONN         SYS_NET_ENOTCONN                
#define WSAESHUTDOWN        SYS_NET_ESHUTDOWN               
#define WSAETOOMANYREFS     SYS_NET_ETOOMANYREFS            
#define WSAETIMEDOUT        SYS_NET_ERROR_ETIMEDOUT 
#define WSAECONNREFUSED     SYS_NET_ECONNREFUSED            
#define WSAELOOP            SYS_NET_ELOOP                   
#define WSAENAMETOOLONG     SYS_NET_ENAMETOOLONG            
#define WSAEHOSTDOWN        SYS_NET_EHOSTDOWN             
#define WSAEHOSTUNREACH     SYS_NET_EHOSTUNREACH             
#define WSAENOTEMPTY        SYS_NET_ENOTEMPTY               
#define WSAEPROCLIM         SYS_NET_EPROCLIM                
#define WSAEUSERS           SYS_NET_EUSERS                  
#define WSAEDQUOT           SYS_NET_EDQUOT                  
#define WSAESTALE           SYS_NET_ESTALE                  
#define WSAEREMOTE          SYS_NET_EREMOTE
#define WSAEINVAL           SYS_NET_EINVAL
	
#define accept(s,a,al) accept(s,a,(socklen_t*)(al))
#define bind(s,a,al) bind(s,a,(socklen_t)(al))
#define connect(s,a,al) connect(s,a,(socklen_t)(al))
#define getpeername(s,a,al) getpeername(s,a,(socklen_t*)(al))
#define getsockname(s,a,al) getsockname(s,a,(socklen_t*)(al))
#define getsockopt(s,l,o,v,vl) getsockopt(s,l,o,v,(socklen_t*)(vl))
#define recvfrom(s,b,l,f,a,al) recvfrom(s,b,l,f,a,(socklen_t*)(al))
#define sendto(s,b,l,f,a,al) sendto(s,b,l,f,a,(socklen_t)(al))
#define setsockopt(s,l,o,v,vl) setsockopt(s,l,o,v,(socklen_t)(vl))
#define closesocket socketclose
#define GOAGetLastError(s) sys_net_errno
#define EWOULDBLOCK sceNET_EWOULDBLOCK

#define GSI_UNUSED(x) {void* y=&x;y=NULL;}
#define memalign_direct memalign

#endif // __RSPLATFORMPS3_H__
