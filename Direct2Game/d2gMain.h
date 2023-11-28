/**
* d2gMain.h
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/
#ifndef __D2GMAIN_H__
#define __D2GMAIN_H__
///////////////////////////////////////////////////////////////////////////////
/** 
*  Internal header for GameSpy Commerce SDK.
*  SEE Direct2Game.h FOR PUBLIC INTERFACE
*/ 

#if defined(__cplusplus)
extern "C"
{
#endif

// D2G Shortcuts for creating a GSResult
#define GS_D2G_ERROR(s,c)  GS_ERROR(GSResultSDK_Direct2Game,s,c)
#define GS_D2G_SUCCESS(s,c)  GS_RESULT(0,GSResultSDK_Direct2Game,s,c)

// Constants and Settings
static const int GS_D2G_ITEMARRAY_INITIAL_COUNT      = 32;
static const int GS_D2G_ITEMARRAY_GROWBY             = 32;
static const int GS_D2G_EXTRAINFOCACHE_INITIAL_COUNT = 32;
static const int GS_D2G_EXTRAINFOCACHE_GROWBY        = 8;
static const int GS_D2G_DEFAULT_SERVICE_TIMEOUTMS    = 30000;

#define GS_D2G_LOG(t, l, m, ... ) gsDebugFormat(GSIDebugCat_Direct2Game, t, l, "%s(%d): In %s: \n" m "\n", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////////

//
// D2G SDK Instance
//
 typedef struct D2GIInstance
{
	GSLoginCertificate mCertificate;
	GSLoginPrivateData mPrivateData;
	gsi_time    mTimeoutMs;  // MS = milliseconds
    char        *mManifestFilePath;
    char        *mServiceURLBase;
    char        *mServiceURL;
} D2GIInstance;


//
// D2GCatalog     Keeps the cached data	
//
 typedef struct D2GICatalog
 {
     int         mGameId;
     gsi_u32     mVersion;
     UCS2String  mRegion;
     UCS2String  mAccessToken;

     // Item caching data
     DArray   mItemArray;
     gsi_bool mAllItemsReceived;
     DArray   mExtraInfoCache;
 } D2GICatalog;
 
#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__D2GMAIN_H__
