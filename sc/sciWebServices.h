///////////////////////////////////////////////////////////////////////////////
// File:	sciWebServices.h
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SCWEBSERVICES_H__
#define __SCWEBSERVICES_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../ghttp/ghttpSoap.h"
#include "../ghttp/ghttpPost.h"

#include "sci.h"
#include "sciReport.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Web service result codes (must match definitions in CompetitionService)
typedef enum
{
	// Competition system errors:
	SCServiceResult_NO_ERROR = 0,
	SCServiceResult_COULD_NOT_START,
	SCServiceResult_COULD_NOT_JOIN,
	SCServiceResult_COULD_NOT_LEAVE,

	// Input validation errors:
	SCServiceResult_PROFILE_ID_INVALID,
	SCServiceResult_IP_INVALID,
	SCServiceResult_ID_INVALID,
	SCServiceResult_REPORT_INVALID,
	SCServiceResult_AUTH_INVALID,

	// System errors:
	SCServiceResult_SERVER_ERROR,
	SCServiceResult_DATABASE_ERROR,

	// More input validation errors:
	SCServiceResult_GAMEID_INVALID

} SCServiceResult;

	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	SCInterfacePtr          mInterface;

	SCCheckBanListCallback  mCheckBanListCallback;
	SCCreateSessionCallback  mCreateSessionCallback;
	SCSetReportIntentionCallback   mSetReportIntentionCallback;
	SCSubmitReportCallback   mSubmitReportDataCallback;

	gsi_bool                mCheckBanListPending;
	gsi_bool                mSetReportIntentionPending;
	gsi_bool                mCreateSessionPending;
	gsi_bool                mSubmitReportPending;
//	gsi_bool                mGetStatsPending;
//	gsi_bool                mGetLeaderboardPending;
//	gsi_bool                mGetRanksPending;

	void *                  mCheckBanListUserData;
	void *                  mSetReportIntentionUserData;
	void *                  mCreateSessionUserData;
	void *                  mSubmitReportUserData;
	void *                  mGetStatsUserData;
	void *                  mGetLeaderboardUserData;
	void *                  mGetRanksUserData;

	gsi_u8*                 mSubmitReportData;
	gsi_u32                 mSubmitReportLength;
	gsi_bool                mInit;

} SCWebServices;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsInit   (SCWebServices* webServices,
                      SCInterfacePtr theInterface);
void     sciWsDestroy(SCWebServices* webServices);
void     sciWsThink  (SCWebServices* webServices);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsCheckBanList        (gsi_u32            hostProfileId,
								   SCPlatform         hostPlatform,
                                   SCWebServices *    webServices,
								   gsi_u32            gameId,
								   const GSLoginCertificate * certificate,
								   const GSLoginPrivateData * privateData,
                                   SCCheckBanListCallback callback,
                                   gsi_time           timeoutMs,
								   void *       userData);

void     sciWsCheckBanListCallback(GHTTPResult       httpResult,
                                   GSXmlStreamWriter  requestData,
                                   GSXmlStreamReader  responseData,
                                   void*        userData);

SCResult sciWsCreateSession       (SCWebServices *    webServices,
								   gsi_u32            gameId,
								   gsi_u16            platformId,
								   const GSLoginCertificate * certificate,
								   const GSLoginPrivateData * privateData,
                                   SCCreateSessionCallback callback,
                                   gsi_time           timeoutMs,
								   void *       userData);

SCResult sciWsCreateMatchlessSession(SCWebServices *    webServices,
								   gsi_u32              gameId,
								   gsi_u16              platformId,
								   const GSLoginCertificate * certificate,
								   const GSLoginPrivateData * privateData,
								   SCCreateSessionCallback callback,
								   gsi_time     timeoutMs,
								   void *       userData);

void     sciWsCreateSessionCallback(GHTTPResult       httpResult,
                                   GSXmlStreamWriter  requestData,
                                   GSXmlStreamReader  responseData,
                                   void*        userData);

SCResult sciWsSetReportIntention  (SCWebServices*     webServices,
								   gsi_u32            gameId,
								   const char *       theSessionId,
								   const char *       theConnectionId,
								   gsi_bool           isAuthoritative,
								   const GSLoginCertificate * certificate,
								   const GSLoginPrivateData * privateData,
                                   SCSetReportIntentionCallback callback,
                                   gsi_time           timeoutMs,
								   void *       userData);

void     sciWsSetReportIntentionCallback(GHTTPResult  httpResult,
                                   GSXmlStreamWriter  requestData,
                                   GSXmlStreamReader  responseData,
                                   void*        userData);

SCResult sciWsSubmitReport        (SCWebServices*     webServices,
								   gsi_u32            gameId,
								   const char *       theSessionId,
								   const char *       theConnectionId,
								   const SCIReport*   theReport,
								   gsi_bool           isAuthoritative,
                                   const GSLoginCertificate * certificate,
								   const GSLoginPrivateData * privateData,
                                   SCSubmitReportCallback callback,
                                   gsi_time           timeoutMs,
								   void *       userData);

void     sciWsSubmitReportCallback(GHTTPResult        httpResult,
                                   GSXmlStreamWriter  requestData,
                                   GSXmlStreamReader  responseData,
                                   void*        userData);






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SCWEBSERVICES_H__
