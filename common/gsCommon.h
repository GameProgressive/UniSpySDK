///////////////////////////////////////////////////////////////////////////////
// File:	gsCommon.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef __GSCOMMON_H__
#define __GSCOMMON_H__


// Common is more like "all"

// Build settings  (set by developer project)
//#define GS_MEM_MANAGED       // use GameSpy memory manager for SDK allocations
//#define GS_COMMON_DEBUG      // use GameSpy debug utilities for SDK debug output
//#define GS_WINSOCK2          // use winsock2
//#define GS_UNICODE           // Use widechar (UCS2) SDK interface.
//#define GS_NO_FILE           // disable file storage (on by default for PS2/DS)
//#define GS_NO_THREAD         // no multi-thread support
//#define GS_PEER		       // MUST be defined if you are using Peer SDK

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#define GSI_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define GSI_DEPRECATED	__attribute__((__deprecated__))
#else
#define GSI_DEPRECATED
#endif

#ifdef GS_PEER
	#define	UNIQUEID			// enable unique id support
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "gsPlatform.h"
#include "gsPlatformSocket.h"
#include "gsPlatformThread.h"
#include "gsPlatformUtil.h"

// platform independent
#include "gsMemory.h"
#include "gsDebug.h"
#include "gsAssert.h"
#include "gsStringUtil.h"

//#include "md5.h"
//#include "darray.h"
//#include "hashtable.h"

// Namespace constants.
#define GSI_NAMESPACE_GAMESPY_DEFAULT	1

// Partner Id constants.
#define GSI_PARTNERID_GAMESPY_DEFAULT	0

// Old Partner Id constants, use GSI_PARTNERID_ constants above.
#define GP_PARTNERID_GAMESPY	0
#define GP_PARTNERID_IGN		10

// Commonly-used macro functions.
#define	GSI_MIN(a,b)				(((a) < (b) ? (a) : (b)))
#define	GSI_MAX(a,b)				(((a) > (b) ? (a) : (b)))
#define	GSI_LIMIT(x,minx,maxx)		(((x) < (minx) ? (minx) : ((x) > (maxx) ? (maxx) : (x))))
#define	GSI_WRAP(x,minx,maxx)		(((x) < (minx) ? (maxx-1) : ((x) >= (maxx) ? (minx) : (x))))
#define GSI_DIM(x)					(sizeof(x) / sizeof((x)[0]))

#define GS_GUID_SIZE	40
#define GS_SECRET_KEY_LENGTH  8
#define GS_ACCESS_KEY_LENGTH 32
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __GSCOMMON_H__
