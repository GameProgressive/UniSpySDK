///////////////////////////////////////////////////////////////////////////////
// File:	sciWebServices.c
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "../common/gsCore.h"

#include "sci.h"
#include "sciInterface.h"
#include "sciWebServices.h"
#include "sciReport.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define SC_CHECKBANLIST_SOAPACTION  "SOAPAction: \"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/CheckProfileOnBanList\""
#define SC_CREATEMATCHLESSSESSION_SOAPACTION "SOAPAction: \"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/CreateMatchlessSession\""
#define SC_CREATESESSION_SOAPACTION "SOAPAction: \"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/CreateSession\""
#define SC_SUBMITREPORT_SOAPACTION  "SOAPAction: \"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/SubmitReport\""
#define SC_SETINTENTION_SOAPACTION  "SOAPAction: \"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/SetReportIntention\""

#define SC_SERVICE_NAMESPACE_COUNT     1
const char * SC_SERVICE_NAMESPACES[SC_SERVICE_NAMESPACE_COUNT] =
{
	"gsc=\"" GSI_HTTP_PROTOCOL_URL "gamespy.net/competition/\""
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsInit(SCWebServices* webServices,
                   SCInterfacePtr theInterface)
{

	
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(!webServices->mInit);

	// Check gsCore
	if (gsCoreIsShutdown() != GSCore_IN_USE)
	{
		return SCResult_CORE_NOT_INITIALIZED;
	}

	// Initialize SCWebServices struct
	webServices->mInterface                  = theInterface;
	webServices->mCreateSessionCallback      = NULL;
	webServices->mSetReportIntentionCallback = NULL;
	webServices->mSubmitReportDataCallback   = NULL;
	webServices->mCreateSessionUserData      = NULL;
	webServices->mSetReportIntentionUserData = NULL;
	webServices->mSubmitReportUserData       = NULL;
	webServices->mCreateSessionPending       = gsi_false;
	webServices->mSetReportIntentionPending  = gsi_false;
	webServices->mSubmitReportPending        = gsi_false;

	// Now initialized
	webServices->mInit = gsi_true;

	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsDestroy(SCWebServices* webServices)
{
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	// No longer initialized
	webServices->mInit = gsi_false;

	// Destroy SCWebServices struct
	webServices->mCreateSessionCallback      = NULL;
	webServices->mSetReportIntentionCallback = NULL;
	webServices->mSubmitReportDataCallback   = NULL;
	webServices->mCreateSessionUserData      = NULL;
	webServices->mSetReportIntentionUserData = NULL;
	webServices->mSubmitReportUserData       = NULL;
	webServices->mCreateSessionPending       = gsi_false;
	webServices->mSetReportIntentionPending  = gsi_false;
	webServices->mSubmitReportPending        = gsi_false;
	webServices->mInterface = NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsThink(SCWebServices* webServices)
{
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	gsCoreThink(0);

	GSI_UNUSED(webServices);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsCheckBanList  (gsi_u32         hostProfileId,
							 SCPlatform      hostPlatform,
                             SCWebServices * webServices,
							 gsi_u32         gameId,
							 const GSLoginCertificate * certificate,
							 const GSLoginPrivateData * privateData,
							 SCCheckBanListCallback callback,
							 gsi_time        timeoutMs,
							 void *          userData)
{
	GSXmlStreamWriter request = NULL;

	// Check parameters
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	// Check for pending request
	if (webServices->mCheckBanListCallback)
		return SCResult_CALLBACK_PENDING;

	// Create the XML message writer
	request = gsXmlCreateStreamWriter(SC_SERVICE_NAMESPACES, SC_SERVICE_NAMESPACE_COUNT);
	if (request == NULL)
		return SCResult_OUT_OF_MEMORY;

	// Fill in the request data
	if (gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "CheckProfileOnBanList")) ||
		gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "certificate")) ||
		gsi_is_false(wsLoginCertWriteXML(certificate, "gsc", request)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "certificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(request, "gsc", "Proof", (const gsi_u8*)privateData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "GameId", (gsi_u32)gameId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "HostProfileId", (gsi_u32)hostProfileId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "HostPlatformId", (gsi_u32)hostPlatform)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "CheckProfileOnBanList")) ||
		gsi_is_false(gsXmlCloseWriter(request))
		)
	{
		gsXmlFreeWriter(request);
		return SCResult_HTTP_ERROR;
	}

	// Set callback
	webServices->mCheckBanListCallback = callback;
	webServices->mCheckBanListUserData = userData;
	webServices->mCheckBanListPending  = gsi_true;

	// Execute soap call
	gsiExecuteSoap(scGameConfigDataServiceURL, SC_CHECKBANLIST_SOAPACTION, request, sciWsCheckBanListCallback, webServices);

	GSI_UNUSED(timeoutMs);
	return SCResult_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsCheckBanListCallback(GHTTPResult httpResult,
								GSXmlStreamWriter  requestData,
								GSXmlStreamReader  responseData,
								void*        userData)
{
	int profileId = 0;
	gsi_bool profileBanned = gsi_false;
	int platformId = -1;
	SCResult aTranslatedResult     = SCResult_HTTP_ERROR;
	SCWebServices* aWebServices    = (SCWebServices*)userData;
	
	GS_ASSERT(aWebServices != NULL);
	GS_ASSERT(aWebServices->mCheckBanListPending);

	// Check for shutdown
	if (!aWebServices->mInit)
		return;

	if (httpResult == GHTTPSuccess)
	{
		int result = SCServiceResult_NO_ERROR;

		// Make sure a basic User Config Response is there.
		if (gsi_is_false(gsXmlMoveToStart(responseData)) 
			|| gsi_is_false(gsXmlMoveToNext(responseData, "CheckProfileOnBanListResponse")) 
			|| gsi_is_false(gsXmlMoveToNext(responseData, "CheckProfileOnBanListResult")) 
			|| gsi_is_false(gsXmlReadChildAsInt(responseData, "result", &result)) 
			)
		{
			aTranslatedResult = SCResult_RESPONSE_INVALID;
		}
		else
		{
			// Read the User Config data.
			if (result == SCServiceResult_NO_ERROR)
			{
				if(gsi_is_false(gsXmlMoveToNext(responseData, "UserConfig"))
                || gsi_is_false(gsXmlReadAttributeAsInt(responseData, "ProfileID", &profileId))
				|| gsi_is_false(gsXmlReadAttributeAsInt(responseData, "PlatformID", &platformId))
				|| gsi_is_false(gsXmlReadAttributeAsBool(responseData, "IsBanned", &profileBanned))
				)
				{
					aTranslatedResult = SCResult_RESPONSE_INVALID;
				}

				aTranslatedResult = SCResult_NO_ERROR;
			}
			else
			{
				aTranslatedResult = SCResult_UNKNOWN_RESPONSE;
			}
		}
	}
	else
	{
		aTranslatedResult = SCResult_HTTP_ERROR;
	}

	// Client callback
	aWebServices->mCheckBanListPending = gsi_false;
	if (aWebServices->mCheckBanListCallback != NULL)
	{
		aWebServices->mCheckBanListCallback(aWebServices->mInterface, httpResult, aTranslatedResult, aWebServices->mCheckBanListUserData, profileId, platformId, profileBanned);
		aWebServices->mCheckBanListUserData = NULL;
		aWebServices->mCheckBanListCallback = NULL;
	}
	GSI_UNUSED(requestData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsCreateSession (SCWebServices * webServices,
							 gsi_u32         gameId,
  						     gsi_u16         platformId,
							 const GSLoginCertificate * certificate,
							 const GSLoginPrivateData * privateData,
							 SCCreateSessionCallback callback,
							 gsi_time        timeoutMs,
							 void *          userData)
{
	GSXmlStreamWriter request = NULL;

	// Check parameters
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	// Check for pending request
	if (webServices->mCreateSessionPending)
		return SCResult_CALLBACK_PENDING;

	// Create the XML message writer
	request = gsXmlCreateStreamWriter(SC_SERVICE_NAMESPACES, SC_SERVICE_NAMESPACE_COUNT);
	if (request == NULL)
		return SCResult_OUT_OF_MEMORY;

	// Fill in the request data
	if (gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "CreateSession")) ||
		gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "certificate")) ||
		gsi_is_false(wsLoginCertWriteXML(certificate, "gsc", request)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "certificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(request, "gsc", "proof", (const gsi_u8*)privateData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "gameid", (gsi_u32)gameId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "platformid", (gsi_u16)platformId)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "CreateSession")) ||
		gsi_is_false(gsXmlCloseWriter(request))
		)
	{
		gsXmlFreeWriter(request);
		return SCResult_HTTP_ERROR;
	}

	// Set callback
	webServices->mCreateSessionCallback = callback;
	webServices->mCreateSessionUserData = userData;
	webServices->mCreateSessionPending  = gsi_true;

	// Execute soap call
	gsiExecuteSoap(scServiceURL, SC_CREATESESSION_SOAPACTION, 
		request, sciWsCreateSessionCallback, webServices);
	GSI_UNUSED(timeoutMs);
	return SCResult_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsCreateMatchlessSession (SCWebServices * webServices,
							          gsi_u32         gameId,
  								      gsi_u16         platformId,
							          const GSLoginCertificate * certificate,
							          const GSLoginPrivateData * privateData,
							          SCCreateSessionCallback callback,
							          gsi_time        timeoutMs,
							          void *          userData)
{
	GSXmlStreamWriter request = NULL;

	// Check parameters
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	// Check for pending request
	if (webServices->mCreateSessionPending)
		return SCResult_CALLBACK_PENDING;

	// Create the XML message writer
	request = gsXmlCreateStreamWriter(SC_SERVICE_NAMESPACES, SC_SERVICE_NAMESPACE_COUNT);
	if (request == NULL)
		return SCResult_OUT_OF_MEMORY;

	// Fill in the request data
	if (gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "CreateMatchlessSession")) ||
		gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "certificate")) ||
		gsi_is_false(wsLoginCertWriteXML(certificate, "gsc", request)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "certificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(request, "gsc", "proof", (const gsi_u8*)privateData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "gameid", (gsi_u32)gameId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "platformid", (gsi_u16)platformId)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "CreateMatchlessSession")) ||
		gsi_is_false(gsXmlCloseWriter(request))
		)
	{
		gsXmlFreeWriter(request);
		return SCResult_HTTP_ERROR;
	}

	// Set callback
	webServices->mCreateSessionCallback = callback;
	webServices->mCreateSessionUserData = userData;
	webServices->mCreateSessionPending  = gsi_true;

	// Execute soap call
	gsiExecuteSoap(scServiceURL, SC_CREATEMATCHLESSSESSION_SOAPACTION, 
		request, sciWsCreateSessionCallback, webServices);
	GSI_UNUSED(timeoutMs);
	return SCResult_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsCreateSessionCallback(GHTTPResult httpResult,
								GSXmlStreamWriter  requestData,
								GSXmlStreamReader  responseData,
								void*        userData)
{
	SCResult aTranslatedResult     = SCResult_HTTP_ERROR;
	SCWebServices* aWebServices    = (SCWebServices*)userData;
	
	char csid[255];
	char ccid[255];
	int csidLen = 255;
	int ccidLen = 255;

	GS_ASSERT(aWebServices != NULL);
	GS_ASSERT(aWebServices->mCreateSessionPending);

	// Check for shutdown
	if (!aWebServices->mInit)
		return;

	if (httpResult == GHTTPSuccess)
	{
		int createResult = 0;

		// Parse through in a way that will work for either type of CreateSession response.
		if (gsi_is_false(gsXmlMoveToStart(responseData)))
		{
			aTranslatedResult = SCResult_RESPONSE_INVALID;
		}
		else if(gsi_is_false(gsXmlMoveToNext(responseData, "CreateSessionResponse")))
		{
			if(gsi_is_false(gsXmlMoveToNext(responseData, "CreateMatchlessSessionResponse")))
			{
				aTranslatedResult = SCResult_RESPONSE_INVALID;
			}
		}
		
		if(gsi_is_false(gsXmlMoveToNext(responseData, "CreateSessionResult")))
		{
			if(gsi_is_false(gsXmlMoveToNext(responseData, "CreateMatchlessSessionResult")))
			{
				aTranslatedResult = SCResult_RESPONSE_INVALID;
			}
		}

		if(gsi_is_false(gsXmlReadChildAsInt(responseData, "result", &createResult)))
		{
			aTranslatedResult = SCResult_RESPONSE_INVALID;
		}
		else if(aTranslatedResult != SCResult_RESPONSE_INVALID)
		{
			// Parse server reported result
			if (createResult == SCServiceResult_NO_ERROR)
			{
				// Read session and connection ID
				if(gsi_is_false(gsXmlReadChildAsStringNT(responseData, "csid", csid, csidLen)) ||
				   gsi_is_false(gsXmlReadChildAsStringNT(responseData, "ccid", ccid, ccidLen))
				   )
				{
					aTranslatedResult = SCResult_RESPONSE_INVALID;
				}
				else
				{
					sciInterfaceSetSessionId((SCInterface*)aWebServices->mInterface, csid);
					sciInterfaceSetConnectionId((SCInterface*)aWebServices->mInterface, ccid);
					aTranslatedResult = SCResult_NO_ERROR;
				}
			}
			else
			{
				aTranslatedResult = SCResult_RESPONSE_INVALID;
			}
		}
	}
	else
	{
		aTranslatedResult = SCResult_HTTP_ERROR;
	}

	// Client callback
	aWebServices->mCreateSessionPending = gsi_false;
	if (aWebServices->mCreateSessionCallback != NULL)
	{
		aWebServices->mCreateSessionCallback(aWebServices->mInterface, httpResult, aTranslatedResult, aWebServices->mCreateSessionUserData);
		aWebServices->mCreateSessionUserData = NULL;
		aWebServices->mCreateSessionCallback = NULL;
	}
	GSI_UNUSED(requestData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsSetReportIntention(SCWebServices* webServices,
								 gsi_u32        gameId,
								 const char *   theSessionId,
								 const char *   theConnectionId,
								 gsi_bool       isAuthoritative,
								 const GSLoginCertificate * certificate,
								 const GSLoginPrivateData * privateData,
                                 SCSetReportIntentionCallback callback,
                                 gsi_time       timeoutMs,
								 void *         userData)
{
	GSXmlStreamWriter request = NULL;

	// Check parameters
	GS_ASSERT(webServices != NULL);
	GS_ASSERT(webServices->mInit);

	// Check for pending request
	if (webServices->mSetReportIntentionPending)
		return SCResult_CALLBACK_PENDING;

	// Create the XML message writer
	request = gsXmlCreateStreamWriter(SC_SERVICE_NAMESPACES, SC_SERVICE_NAMESPACE_COUNT);
	if (request == NULL)
		return SCResult_OUT_OF_MEMORY;

	// Fill in the request data
	if (gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "SetReportIntention")) ||
		gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "certificate")) ||
		gsi_is_false(wsLoginCertWriteXML(certificate, "gsc", request)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "certificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(request, "gsc", "proof", (const gsi_u8*)privateData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
		gsi_is_false(gsXmlWriteStringElement(request, "gsc", "csid", theSessionId)) ||
		gsi_is_false(gsXmlWriteStringElement(request, "gsc", "ccid", theConnectionId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "gameid", (gsi_u32)gameId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "authoritative", (gsi_u32)(gsi_is_true(isAuthoritative) ? 1:0))) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "SetReportIntention")) ||
		gsi_is_false(gsXmlCloseWriter(request))
		)
	{
		gsXmlFreeWriter(request);
		return SCResult_HTTP_ERROR;
	}

	// Set callback
	webServices->mSetReportIntentionCallback = callback;
	webServices->mSetReportIntentionUserData = userData;
	webServices->mSetReportIntentionPending = gsi_true;

	// Execute soap call
	gsiExecuteSoap(scServiceURL, SC_SETINTENTION_SOAPACTION, 
		request, sciWsSetReportIntentionCallback, webServices);
	
	GSI_UNUSED(timeoutMs);
	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsSetReportIntentionCallback(GHTTPResult httpResult,
                                     GSXmlStreamWriter requestData,
                                     GSXmlStreamReader responseData,
                                     void*       userData)
{
	SCResult aTranslatedResult     = SCResult_HTTP_ERROR;
	SCWebServices* aWebServices    = (SCWebServices*)userData;

	char ccid[255];
	int ccidLen = 255;


	GS_ASSERT(aWebServices != NULL);
	GS_ASSERT(aWebServices->mSetReportIntentionPending);

	// Check for shutdown
	if (!aWebServices->mInit)
		return;

	if (httpResult == GHTTPSuccess)
	{
		int intentionResult = 0;

		if (gsi_is_false(gsXmlMoveToStart(responseData)) ||
			gsi_is_false(gsXmlMoveToNext(responseData, "SetReportIntentionResponse")) ||
			gsi_is_false(gsXmlMoveToNext(responseData, "SetReportIntentionResult")) ||
			gsi_is_false(gsXmlReadChildAsInt(responseData, "result", &intentionResult)) ||
  		    gsi_is_false(gsXmlReadChildAsStringNT(responseData, "ccid", ccid, ccidLen))
			)
		{
			aTranslatedResult = SCResult_RESPONSE_INVALID;
		}
		else
		{
			if (intentionResult == SCServiceResult_NO_ERROR)
			{
				aTranslatedResult = SCResult_NO_ERROR;
				sciInterfaceSetConnectionId((SCInterface*)aWebServices->mInterface, ccid);
			}
			else
				aTranslatedResult = SCResult_UNKNOWN_RESPONSE;
		}
	}
	else
	{
		aTranslatedResult = SCResult_HTTP_ERROR;
	}

	// Client callback
	aWebServices->mSetReportIntentionPending = gsi_false;
	if (aWebServices->mSetReportIntentionCallback != NULL)
	{
		aWebServices->mSetReportIntentionCallback(aWebServices->mInterface,
			                                      httpResult,
		                                          aTranslatedResult,
												  aWebServices->mSetReportIntentionUserData);
		aWebServices->mSetReportIntentionUserData = NULL;
		aWebServices->mSetReportIntentionCallback = NULL;
	}
	GSI_UNUSED(requestData);
}


///////////////////////////////////////////////////////////////////////////////
// declared here to allow function to get around Unicode calls
extern GHTTPBool ghiPostAddFileFromMemory(GHTTPPost post,const char * name,const char * buffer,
	int bufferLen,const char * reportFilename,const char * contentType);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Private GSSoapCustomFunc used by sciWsSubmitReport
static void sciWsSubmitReportCustom(GHTTPPost thePost, void* userData)
{
	SCWebServices* aWebServices = (SCWebServices*)userData;

	//Use internal method to get around unicode calls
	ghiPostAddFileFromMemory(thePost, "report", (char *)aWebServices->mSubmitReportData,
		(gsi_i32)aWebServices->mSubmitReportLength, "report", "application/bin");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult sciWsSubmitReport(SCWebServices* webServices,
						   gsi_u32        gameId,
						   const char *   theSessionId,
						   const char *   theConnectionId,
						   const SCIReport *    theReport,
						   gsi_bool       isAuthoritative,
						   const GSLoginCertificate * certificate,
						   const GSLoginPrivateData * privateData,
                           SCSubmitReportCallback callback,
                           gsi_time       timeoutMs,
						   void *         userData)
{
	GSXmlStreamWriter request = NULL;

	// Check parameters
	GS_ASSERT(webServices != NULL);

	// Check for pending request
	if (webServices->mSubmitReportPending)
	{
		return SCResult_CALLBACK_PENDING;
	}

	// Check for complete report
	if (theReport->mBuffer.mPos < sizeof(SCIReportHeader))
		return SCResult_REPORT_INVALID;

	// Create the XML message writer
	request = gsXmlCreateStreamWriter(SC_SERVICE_NAMESPACES, SC_SERVICE_NAMESPACE_COUNT);
	if (request == NULL)
		return SCResult_OUT_OF_MEMORY;

	// Fill in the request data
	if (gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "SubmitReport")) ||
		gsi_is_false(gsXmlWriteOpenTag(request, "gsc", "certificate")) ||
		gsi_is_false(wsLoginCertWriteXML(certificate, "gsc", request)) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "certificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(request, "gsc", "proof", (const gsi_u8*)privateData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
		gsi_is_false(gsXmlWriteStringElement(request, "gsc", "csid", theSessionId)) ||
		gsi_is_false(gsXmlWriteStringElement(request, "gsc", "ccid", theConnectionId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "gameid", (gsi_u32)gameId)) ||
		gsi_is_false(gsXmlWriteIntElement(request, "gsc", "authoritative", (gsi_u32)(gsi_is_true(isAuthoritative) ? 1:0))) ||
		gsi_is_false(gsXmlWriteCloseTag(request, "gsc", "SubmitReport")) ||
		gsi_is_false(gsXmlCloseWriter(request))
		)
	{
		gsXmlFreeWriter(request);
		return SCResult_OUT_OF_MEMORY;
	}
	
	// Get submission size
	webServices->mSubmitReportData  = (gsi_u8*)theReport->mBuffer.mData;
	webServices->mSubmitReportLength = theReport->mBuffer.mPos;

	// Set callback
	webServices->mSubmitReportDataCallback = callback;
	webServices->mSubmitReportUserData     = userData;
	webServices->mSubmitReportPending      = gsi_true;

	// Execute soap call
	gsiExecuteSoapCustom(scServiceURL, SC_SUBMITREPORT_SOAPACTION, 
		request, sciWsSubmitReportCallback,sciWsSubmitReportCustom, webServices);
	
	GSI_UNUSED(timeoutMs);
	return SCResult_NO_ERROR;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sciWsSubmitReportCallback(GHTTPResult       httpResult,
                               GSXmlStreamWriter requestData,
                               GSXmlStreamReader responseData,
                               void*             userData)
{
	SCResult aTranslatedResult     = SCResult_HTTP_ERROR;
	SCWebServices* aWebServices    = (SCWebServices*)userData;

	GS_ASSERT(aWebServices != NULL);

	// Check for shutdown
	if (!aWebServices->mInit)
		return;

	GS_ASSERT(aWebServices->mSubmitReportPending);

	if (httpResult == GHTTPSuccess)
	{
		int submitResult = 0;

		if (gsi_is_false(gsXmlMoveToStart(responseData)) ||
			gsi_is_false(gsXmlMoveToNext(responseData, "SubmitReportResponse")) ||
			gsi_is_false(gsXmlMoveToNext(responseData, "SubmitReportResult")) ||
			gsi_is_false(gsXmlReadChildAsInt(responseData, "result", &submitResult))
			)
		{
			aTranslatedResult = SCResult_RESPONSE_INVALID;
		}
		else
		{
			switch (submitResult)
			{
			case SCServiceResult_NO_ERROR:       
				aTranslatedResult = SCResult_NO_ERROR;          
				break;
			case SCServiceResult_REPORT_INVALID: 
				aTranslatedResult = SCResult_REPORT_INVALID;    
				break;
			default:        
				aTranslatedResult = SCResult_UNKNOWN_RESPONSE; 
				break;
			};
		}
	}
	else
	{
		aTranslatedResult = SCResult_HTTP_ERROR;
	}

	// Client callback
	aWebServices->mSubmitReportPending = gsi_false;
	if (aWebServices->mSubmitReportDataCallback != NULL)
	{
		aWebServices->mSubmitReportDataCallback(aWebServices->mInterface,
			                                httpResult,
		                                    aTranslatedResult,
											aWebServices->mSubmitReportUserData);
		aWebServices->mSubmitReportUserData = NULL;
		aWebServices->mSubmitReportDataCallback = NULL;
	}
	GSI_UNUSED(requestData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
