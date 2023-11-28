// d2gServices.c
//
// GameSpy  DIRECT TO GAME SDK
// This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
// Copyright (c) 2008, GameSpy Technology
//
///////////////////////////////////////////////////////////////////////////////
// The functions in this file are internal to D2G SDK and they implement the 
// the messaging interface between the SDK and the D2G server.
// They are called directly from the API.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Includes 
///////////////////////////////////////////////////////////////////////////////
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttpCommon.h"

#include "Direct2Game.h"

#include "d2gMain.h"
#include "d2gServices.h"
#include "d2gUtil.h"
#include "d2gDownloads.h"
#include "d2gDeserialize.h"


// This string is used as part of the SOAP messages
const char * GS_D2G_NAMESPACES[GS_D2G_NAMESPACE_COUNT] =
{
    GS_D2G_NAMESPACE "=\"http://gamespy.net/commerce/2009/02\""
};


///////////////////////////////////////////////////////////////////////////////
// Get Store Availability                                               
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giServiceIsOrderCompleteCallback
//      Internal function to process response for store availibility
// 
static void d2giGetStoreAvailCallback(GHTTPResult httpResult,
                                      GSXmlStreamWriter theRequestXml,
                                      GSXmlStreamReader theResponseXml,
                                      void * theRequestData)
{
    GSResult result = GS_SUCCESS;
    D2GIRequestData * requestData = NULL;

    // leave on stack, it goes away after the developer callback is triggered
    D2GGetStoreAvailabilityResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    requestData = (D2GIRequestData*)theRequestData;

    if (httpResult == GHTTPSuccess)
    {
        result = d2giParseGetStoreAvailResponse(requestData->mInstance, theResponseXml, &response);
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_HttpError);
    }

    requestData->mUserCallback.mGetStoreAvailCallback(result, &response, requestData->mUserData);
    gsifree(requestData);
    GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giServiceGetStoreAvail
//      Internal function send a request for store availability to the backend.
//
GSResult d2giServiceGetStoreAvail(D2GIInstance *theInstance, 
                                  D2GICatalog  *theCatalog,
                                  D2GGetStoreAvailabilityCallback callback, 
                                  void *userData)
{
    GSXmlStreamWriter writer;
    D2GIRequestData * requestData = NULL;

    // no need to use secure URL
    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    requestData->mInstance = theInstance;
    requestData->mCatalog  = theCatalog;
    requestData->mUserData = userData;
    requestData->mUserCallback.mGetStoreAvailCallback = callback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsifree(requestData);
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        if (gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_GETSTOREAVAIL_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "version", theCatalog->mVersion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"region", theCatalog->mRegion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"accesstoken", theCatalog->mAccessToken)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_GETSTOREAVAIL_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter    (writer)))
        {
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETSTOREAVAIL_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
            // Should only fail to write if the buffer is full
            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                               GS_D2G_GETSTOREAVAIL_SOAP,
                               writer, 
                               d2giGetStoreAvailCallback, theInstance->mTimeoutMs,
                               (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giServiceLoadExtraInfoCallback(GHTTPResult       httpResult, 
                                            GSXmlStreamWriter theRequestXml, 
                                            GSXmlStreamReader theResponseXml, 
                                            void *            theRequestData)
{
    GSResult        result       = GS_SUCCESS;
    D2GIRequestData *requestData = (D2GIRequestData*)theRequestData;

    // leave on stack, it goes away after the developer callback is triggered
    D2GLoadExtraCatalogInfoResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    if (httpResult == GHTTPSuccess)
    {
        result = d2giParseLoadExtraCatalogInfoResponse(requestData->mInstance, 
                                               requestData->mCatalog, 
                                               theResponseXml, 
                                               &response);
		if (GS_FAILED(result))
		{
			d2giFreeExtraInfoList(requestData->mCatalog, response.mExtraCatalogInfoList, result, gsi_true);
			response.mExtraCatalogInfoList = NULL;
		}
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    }

	// don't delete the extra info object because developers will keeping track of those.
    requestData->mUserCallback.mGetExtraInfoCallback(result, &response, requestData->mUserData);

	// Done with the request free it before leaving
    gsifree(requestData);

    GSI_UNUSED(theRequestXml);

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceLoadExtraInfo(D2GIInstance *theInstance, 
                                  D2GICatalog  *theCatalog,
                                  D2GLoadExtraCatalogInfoCallback callback, 
                                  void *userData)
{
    GSXmlStreamWriter writer;
    D2GIRequestData * requestData = NULL;

    // no need to use secure URL
    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    requestData->mInstance = theInstance;
    requestData->mCatalog  = theCatalog;
    requestData->mUserData = userData;
    requestData->mUserCallback.mGetExtraInfoCallback = callback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GET_STORE_EXTENSION_DATA_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "version", theCatalog->mVersion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"region", theCatalog->mRegion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"accesstoken", theCatalog->mAccessToken)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_GET_STORE_EXTENSION_DATA_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter(writer))
            )
        {
            gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETEXTRAINFO_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
            // Should only fail to write if the buffer is full
            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                               GS_D2G_GET_STORE_EXTENSION_DATA_SOAP,
                               writer, 
                               d2giServiceLoadExtraInfoCallback, 
							   theInstance->mTimeoutMs,
                               (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giServiceLoadCatalogItemsCallback(GHTTPResult httpResult,
									            GSXmlStreamWriter theRequestXml,
									            GSXmlStreamReader theResponseXml,
									            void        *theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;

	// initialize the Response data.
	D2GLoadCatalogItemsResponse response;
	memset(&response, 0, sizeof(response));  
	response.mHttpResult = httpResult;

	requestData  = (D2GIRequestData*)theRequestData;

	if (httpResult == GHTTPRequestCancelled)
	{
		result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_RequestTimedOut);
	}
	else if (httpResult != GHTTPSuccess)
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	}
	else // httpResult == GHTTPSuccess
	{
	    result = d2giParseLoadCatalogItemsFromResponse(requestData->mInstance, 
    	                                               requestData->mCatalog, 
    	                                               theResponseXml, 
    	                                               &response);
	}

	requestData->mUserCallback.mLoadCatalogItemsCallback(result, &response, requestData->mUserData);
	
	// NOTE: Developer will be responsible for calling d2gFreeCatalogItems on mItemList in response.
	if (GS_SUCCEEDED(result))
	{
		// we successfully received all the items 
		requestData->mCatalog->mAllItemsReceived = gsi_true;
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceLoadCatalogItems(D2GIInstance *theInstance, 
                                     D2GICatalog  *theCatalog, 
                                     D2GLoadCatalogItemsCallback callback, 
                                     void         *userData )
{
	GSXmlStreamWriter writer;
	D2GIRequestData   *requestData = NULL;

	d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

	writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc,GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter(assuming out of memory)",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory,GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
                __FILE__, __FUNCTION__, __LINE__,sizeof(D2GIRequestData));
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        requestData->mInstance = theInstance;
        requestData->mCatalog =  theCatalog;
        requestData->mUserData = userData;
        requestData->mUserCallback.mLoadCatalogItemsCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETALLITEMS_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "version", theCatalog->mVersion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"region", theCatalog->mRegion)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"accesstoken", theCatalog->mAccessToken)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETALLITEMS_REQUEST)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
            gsXmlFreeWriter(writer);
            gsifree(requestData);
			// Should only fail to write if the buffer is full
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETALLITEMS_REQUEST.", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	
						
		aTask = gsiExecuteSoapWithTimeout( theInstance->mServiceURL, 
		                        GS_D2G_GETALLITEMS_SOAP,
			                    writer, 
			                    d2giServiceLoadCatalogItemsCallback, 
								theInstance->mTimeoutMs,
			                    (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giGetItemsByCategoryCallback( GHTTPResult httpResult,
									        GSXmlStreamWriter theRequestXml,
									        GSXmlStreamReader theResponseXml,
									        void        *theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;

	// leave on stack, it goes away after the developer callback is triggered
	D2GLoadCatalogItemsByCategoryResponse response;
	memset(&response, 0, sizeof(response));
	response.mHttpResult = httpResult;

	requestData = (D2GIRequestData*)theRequestData;

	if (httpResult == GHTTPSuccess)
	{
	    result = d2giParseGetItemsByCategoryFromResponse(requestData->mInstance, 
	                                                     requestData->mCatalog, 
	                                                     theResponseXml, 
	                                                     &response);	
	}
	else if (httpResult == GHTTPRequestCancelled)
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
	}
	else
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	}

	// NOTE: Developer will be responsible for calling d2gFreeCatalogItems on mItemList in response.
	requestData->mUserCallback.mGetItemsByCategoryCallback(result, &response, requestData->mUserData);
	
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceGetItemsByCategory(D2GIInstance *theInstance, 
                                       D2GICatalog  *theCatalog,
									   const UCS2String theCategory, 
									   D2GLoadCatalogItemsByCategoryCallback usercallback, 
									   void         *userData)
{
	GSXmlStreamWriter writer;
	D2GIRequestData * requestData = NULL;

	d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

	requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
	if (requestData == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d):Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	requestData->mInstance = theInstance;
    requestData->mCatalog =  theCatalog;
    requestData->mUserData = userData;
	requestData->mUserCallback.mGetItemsByCategoryCallback = usercallback;

	writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d):Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_GETITEMSBYCATEGORY_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "version", theCatalog->mVersion)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"region", theCatalog->mRegion)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"accesstoken", theCatalog->mAccessToken)) ||
			gsi_is_false(gsXmlWriteOpenTag  (writer, GS_D2G_NAMESPACE, "category")) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "name", theCategory)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, "category")) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_GETITEMSBYCATEGORY_REQUEST)) ||
			gsi_is_false(gsXmlCloseWriter    (writer)))
		{
			// Should only fail to write if the buffer is full
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETITEMSBYCATEGORY_REQUEST.", 
                __FILE__, __FUNCTION__, __LINE__);
			gsXmlFreeWriter(writer);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
		                       GS_D2G_GETITEMSBYCATEGORY_SOAP,
			                   writer, 
			                   d2giGetItemsByCategoryCallback,
							   theInstance->mTimeoutMs,
			                   (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giServiceGetUserCreditCardsCallback(GHTTPResult httpResult,
                                                  GSXmlStreamWriter theRequestXml,
                                                  GSXmlStreamReader theResponseXml,
                                                  void        *theRequestData)
{
    GSResult result = GS_SUCCESS;
    D2GIRequestData * requestData = NULL;

    D2GGetUserCreditCardsResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    requestData = (D2GIRequestData*)theRequestData;

    if (httpResult == GHTTPSuccess)
    {
        result = d2giParseCreditCardInfoList(requestData->mInstance, theResponseXml, &response);
		if (GS_FAILED(result))
		{
			d2giFreeCreditCardInfoList(response.mListOfCreditCards);
			response.mListOfCreditCards = NULL;
		}
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    }

    requestData->mUserCallback.mGetUserCreditCardsCallback(result, &response, requestData->mUserData);

    gsifree(requestData);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceGetUserCreditCards(D2GIInstance *theInstance, 
                                       D2GICatalog  *theCatalog,
                                       gsi_bool     validOnly,
                                       D2GGetUserCreditCardsCallback callback, 
                                       void         *userData)
{
    GSXmlStreamWriter writer;
    D2GIRequestData   *requestData = NULL;

    gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Memory,GSIDebugLevel_Verbose,
        "%s::%s entered\r\n", __FILE__, __FUNCTION__);

    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_ACCOUNT_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Memory,GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData\r\n", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    requestData->mInstance = theInstance;    
    requestData->mCatalog =  theCatalog;
    requestData->mUserData = userData;
    requestData->mUserCallback.mGetUserCreditCardsCallback = callback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Misc,GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\r\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask *aTask = NULL;
        char       *returnValidOnly = validOnly ? "true" : "false";

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETUSERCREDITCARDS_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
            (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "returnvalidonly", returnValidOnly)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETUSERCREDITCARDS_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter(writer))
            )
        {
            // Should only fail to write if the buffer is full
            gsXmlFreeWriter(writer);
            gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Misc,GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETUSERCREDITCARDS_REQUEST, etc.",
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout( theInstance->mServiceURL, 
                                GS_D2G_GETUSERCREDITCARDS_SOAP,
                                writer, 
                                d2giServiceGetUserCreditCardsCallback, 
								theInstance->mTimeoutMs,
                                (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Misc,GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n",
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giGetOrderTotalCallback(GHTTPResult httpResult, 
									  GSXmlStreamWriter theRequestXml, 
									  GSXmlStreamReader theResponseXml, 
									  void        *theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;

	// leave on stack, it goes away after the developer callback is triggered
	D2GGetOrderTotalResponse response;
	memset(&response, 0, sizeof(response));
	response.mHttpResult = httpResult;

	requestData = (D2GIRequestData*)theRequestData;

	if (httpResult == GHTTPSuccess)
	{
		result = d2giParseGetOrderTotalResponse(requestData->mInstance, 
		                                        requestData->mCatalog, 
		                                        theResponseXml, 
		                                        &response);
	}
	else if (httpResult == GHTTPRequestCancelled)
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
	}
	else
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	}

	requestData->mUserCallback.mGetOrderTotalCallback(result, &response, requestData->mUserData);
	
	// Developer is responsible for calling d2gFreeOrderTotal to clean memory.
	// We just need to clear our pointer to it.
	response.mOrderTotal = NULL;
	
	// clean up the request
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceGetOrderTotal(D2GIInstance  *theInstance, 
                                  D2GICatalog   *theCatalog,
								  gsi_u32       accountId, 
								  D2GItemId     *itemIds, 
								  gsi_u32       itemCount, 
								  gsi_u32       *itemQuantities, 
								  D2GGetOrderTotalCallback callback, 
								  void          *userData)
{
	GSXmlStreamWriter   writer;
	D2GIRequestData     *requestData = NULL;
	
	d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_PURCHASE_SERVICE_URL_FORMAT);
	
	// make a copy of the request callback and user param
	requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
	if (requestData == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData\n", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	requestData->mInstance = theInstance;
    requestData->mCatalog =  theCatalog;
    requestData->mUserData = userData;
	requestData->mUserCallback.mGetOrderTotalCallback = callback;

	writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\n", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;
        D2GCatalogItem  *item = d2giGetCatalogItem(theInstance, theCatalog, itemIds[0]);

		if ((item == NULL) || 
		    gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETORDERTOTAL_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(d2giWriteCachedItemsToXml(theInstance, theCatalog,  writer, accountId, 
                    item->mGeoInfo.mCultureCode , item->mGeoInfo.mCurrencyCode, itemIds, itemCount, itemQuantities)) ||

			gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
			gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
				(const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_GETORDERTOTAL_REQUEST)) ||
			gsi_is_false(gsXmlCloseWriter    (writer))
			)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETORDERTOTAL_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);

			gsXmlFreeWriter(writer);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		
		aTask = gsiExecuteSoapWithTimeout( theInstance->mServiceURL, 
		                        GS_D2G_GETORDERTOTAL_SOAP,
			                    writer, 
			                    d2giGetOrderTotalCallback, 
								theInstance->mTimeoutMs,
			                    (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void d2giServicePurchaseItemsCallback(GHTTPResult httpResult, 
								      GSXmlStreamWriter theRequestXml, 
								      GSXmlStreamReader theResponseXml, 
								      void * theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;

	// leave on stack, it goes away after the developer callback is triggered
	D2GStartOrderResponse response;
	memset(&response, 0, sizeof(response));
	response.mHttpResult = httpResult;

	requestData = (D2GIRequestData*)theRequestData;

	if (httpResult == GHTTPSuccess)
	{
		result = d2giParseStartOrderResponse(requestData->mInstance, 
		                                     requestData->mCatalog, 
		                                     theResponseXml, &response);
	}
	else if (httpResult == GHTTPRequestCancelled)
	{
		// TODO: ask for more info
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
	}
	else
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	}

	requestData->mUserCallback.mBeginPurchaseCallback(result, &response, requestData->mUserData);
	
	// Developer is responsible for calling d2gFreeOrderPurchase to clean memory.
	// We just need to clear our pointer to it.
	response.mOrderPurchase = NULL;

	// cleanup the request
	gsifree(requestData);
	
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServicePurchaseItems(D2GIInstance * theInstance, 
                                  D2GICatalog *  theCatalog,
								  const D2GOrderTotal *theOrderTotal, 
								  D2GStartOrderCallback callback, 
								  void * userData)
{
	GSXmlStreamWriter writer;
	D2GIRequestData * requestData = NULL;

	// no need to use secure URL
	d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_PURCHASE_SERVICE_URL_FORMAT);

	// make a copy of the request callback and user param
	requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
	if (requestData == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	requestData->mInstance = theInstance;
    requestData->mCatalog =  theCatalog;
    requestData->mUserData = userData;
	requestData->mUserCallback.mBeginPurchaseCallback = callback;

	writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_PURCHASEITEMS_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
			gsi_is_false(d2giWriteOrderTotalToXml(theInstance, writer, theOrderTotal)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
			gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
			                    (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_PURCHASEITEMS_REQUEST)) ||
			gsi_is_false(gsXmlCloseWriter(writer))
			)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_PURCHASEITEMS_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
                
			// Should only fail to write if the buffer is full
			gsXmlFreeWriter(writer);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		aTask = gsiExecuteSoapWithTimeout( theInstance->mServiceURL, 
		                        GS_D2G_PURCHASEITEMS_SOAP,
			                    writer, 
			                    d2giServicePurchaseItemsCallback, 
								theInstance->mTimeoutMs,
			                    (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void d2giServiceIsOrderCompleteCallback(GHTTPResult httpResult, 
									    GSXmlStreamWriter theRequestXml, 
									    GSXmlStreamReader theResponseXml, 
									    void        *theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;

	// leave on stack, it goes away after the developer callback is triggered
	D2GIsOrderCompleteResponse response;
	memset(&response, 0, sizeof(response));
	response.mHttpResult = httpResult;

	requestData = (D2GIRequestData*)theRequestData;

	if (httpResult == GHTTPSuccess)
	{
		result = d2giParseIsOrderCompleteResponse(requestData->mInstance, 
		                                          requestData->mCatalog, 
		                                          theResponseXml, 
		                                          &response);
	}
	else if (httpResult == GHTTPRequestCancelled)
	{
		// TODO: ask for more info
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
	}
	else
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	}

	requestData->mUserCallback.mIsOrderCompleteCallback(result, &response, requestData->mUserData);

	// Developer is responsible for calling d2gFreeOrderPurchase to clean memory.
	// We just need to clear our pointer to it.
	response.mOrderPurchase = NULL;

	// cleanup the request
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceIsOrderComplete( D2GIInstance  *theInstance, 
                                     D2GICatalog   *theCatalog,
                                     const D2GOrderPurchase *theOrderPurchase, 
                                     D2GIsOrderCompleteCallback callback, 
                                     void          *userData )
{
	GSXmlStreamWriter writer;
	D2GIRequestData * requestData = NULL;

	// no need to use secure URL
	d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_PURCHASE_SERVICE_URL_FORMAT);

	// make a copy of the request callback and user param
	requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
	if (requestData == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	requestData->mInstance = theInstance;
	requestData->mCatalog  = theCatalog;
	requestData->mUserData = userData;
	requestData->mUserCallback.mIsOrderCompleteCallback = callback;

	writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_PLACEDORDER_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "rootorderid", 
			        theOrderPurchase->mOrder.mRootOrderGuid)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
 			gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
 			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
 			gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
 			        (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_PLACEDORDER_REQUEST)) ||
			gsi_is_false(gsXmlCloseWriter(writer))
			)
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_PLACEDORDER_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
			// Should only fail to write if the buffer is full
			gsXmlFreeWriter(writer);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
		                       GS_D2G_PLACEDORDER_SOAP,
			                   writer, 
			                   d2giServiceIsOrderCompleteCallback, 
							   theInstance->mTimeoutMs,
			                   (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void d2giServiceDownloadFileByIdCallback(GHTTPResult httpResult,
											    GSXmlStreamWriter theRequestXml,
											    GSXmlStreamReader theResponseXml,
											    void * theRequestData)
{
	GSResult result = GS_SUCCESS;
	D2GIRequestData * requestData = NULL;
      
	requestData = (D2GIRequestData*)theRequestData;    
  
	if (httpResult == GHTTPSuccess)
	{
        result = d2giParseGetResponse(theResponseXml, GS_D2G_GETDOWNLOADINFO_RESULT);
		if (GS_SUCCEEDED(result))
		{
            int downloadUrlLen = 4096;
            gsi_char *downloadUrl;
			int statusCode;
	        const char *statusMessage;
			int statusMessageLen;
	        	
            if (gsi_is_false(gsXmlMoveToChild(theResponseXml,    "downloadinfo")) ||
                gsi_is_false(gsXmlMoveToChild(theResponseXml,    "status")) ||
                gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "code", &statusCode)) ||
                gsi_is_false(gsXmlReadChildAsString(theResponseXml, "message", &statusMessage, &statusMessageLen))||
                gsi_is_false(gsXmlMoveToParent(theResponseXml)))
            {
                result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
                requestData->mCompleteCallback(result, NULL, requestData->mUserData);
            }
            else
            {
                result = d2giResultFromDownloadStatusCode(statusCode);          
                if (GS_SUCCEEDED(result))
                {
                    downloadUrl = gsimalloc(sizeof(gsi_char) * downloadUrlLen + 1);

			        if (gsi_is_false(gsXmlReadChildAsTStringNT(theResponseXml, "url", downloadUrl, downloadUrlLen)))
			        {
				        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                            "%s :: %s (%d): could not parse xml stream: downloadurl\n", 
                            __FILE__, __FUNCTION__, __LINE__);
					        
        				gsifree(downloadUrl);
				        
				        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				        requestData->mCompleteCallback(result, NULL, requestData->mUserData);
			        }
			        else
			        {
                        gsi_char    *httpUserHeaders = NULL;

			            // get additional http headers, each has to be terminated by a <CR><LF>(CRLF).
                        result = d2giParseDownloadUserHeadersFromResponse(theResponseXml, &httpUserHeaders);
                                    
                        if (GS_SUCCEEDED(result))
                        {
                            // Now we can start downloading
				            result = d2giStartDownloadThread(requestData->mDownloadLocation, 
				                                             downloadUrl, 
				                                             httpUserHeaders,
				                                             requestData->mProgressCallback, 
					                                         requestData->mCompleteCallback, 
					                                         requestData->mUserData);

                            if (GS_FAILED(result))
                            {
                                requestData->mCompleteCallback(result, NULL, requestData->mUserData);
                            }
			            }
			            else
                        {
                            requestData->mCompleteCallback(result, NULL, requestData->mUserData);
                        }
			        }
                }
                else
                {
                    // Received Failure from the backend
                    requestData->mCompleteCallback(result, NULL, requestData->mUserData);
                }
	        }	
		}
		else
		{
		    // Received a failure from the back end.
			requestData->mCompleteCallback(result, NULL, requestData->mUserData);
		}
	}
	else if (httpResult == GHTTPRequestCancelled)
	{
		result = GSResultCode_RequestTimedOut;
		requestData->mCompleteCallback(result, NULL, requestData->mUserData);
	}
	else
	{
		result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_HttpError);
		requestData->mCompleteCallback(result, NULL, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceGetDownloadInfo(D2GIInstance *theInstance, 
                                    D2GICatalog  *theCatalog,
                                    D2GFileId fileId,  
                                    const gsi_char *downloadLocation,
                                    D2GDownloadProgressCallback progressCallback, 
                                    D2GDownloadCompleteCallback completeCallback, 
                                    void *userData)
{
    GSXmlStreamWriter writer;
    D2GIRequestData * requestData = NULL;

    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData *)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData\n", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    memset(requestData, 0, sizeof(D2GIRequestData));
    requestData->mInstance = theInstance;
    requestData->mCatalog  = theCatalog;
    requestData->mUserData = userData;
    requestData->mDownloadLocation = downloadLocation;
    requestData->mProgressCallback = progressCallback;
    requestData->mCompleteCallback = completeCallback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;
        if (gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_GETDOWNLOADINFO_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "fileid", fileId)) ||
            gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(wsLoginCertWriteXML (&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
                    (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_GETDOWNLOADINFO_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter    (writer))
            )
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETDOWNLOADINFO_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
            
            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                               GS_D2G_GETDOWNLOADINFO_SOAP, 
                               writer, 
                               d2giServiceDownloadFileByIdCallback,
							   theInstance->mTimeoutMs,
                               (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void d2giServiceGetPurchaseHistoryCallback(GHTTPResult httpResult, 
                                                  GSXmlStreamWriter theRequestXml, 
                                                  GSXmlStreamReader theResponseXml, 
                                                  void * theRequestData)
{
    GSResult          result      = GS_SUCCESS;
    D2GIRequestData * requestData = (D2GIRequestData*)theRequestData;
    
    // leave on stack, it goes away after the developer callback is triggered
    D2GGetPurchaseHistoryResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    if (httpResult == GHTTPSuccess)
    {
		result = d2giParseGetPurchaseHistoryResponse(requestData->mInstance, 
                                                     requestData->mCatalog, 
                                                     theResponseXml, 
			                                         &response);
		if (GS_FAILED(result))
		{			
			d2giFreePurchaseHistory(response.mPurchaseHistory);
			response.mPurchaseHistory = NULL;
		}
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    }

	// we don't need to clean up the purchase history object in the case of success because the developer 
	// will need to keep track of it using a pointer.  It saves them from having to clone the history
	// because that eats up twice as much memory during the callback scope.
    requestData->mUserCallback.mGetPurchaseHistoryCallback(result, &response, requestData->mUserData);    
    
	gsifree(requestData);

    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giServiceGetPurchaseHistory( D2GIInstance *theInstance, 
                                        D2GICatalog  *theCatalog,
                                        D2GGetPurchaseHistoryCallback callback, 
                                        void *userData )
{
    GSXmlStreamWriter writer;
    D2GIRequestData * requestData = NULL;

    // no need to use secure URL
    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_PURCHASE_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    requestData->mInstance = theInstance;
    requestData->mCatalog  = theCatalog;
    requestData->mUserData = userData;
    requestData->mUserCallback.mGetPurchaseHistoryCallback = callback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETPURCHASEHISTORY_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer,GS_D2G_NAMESPACE,"accesstoken", theCatalog->mAccessToken)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
                    (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETPURCHASEHISTORY_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter(writer))
            )
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETPURCHASEHISTORY_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);
            // Should only fail to write if the buffer is full
            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                               GS_D2G_GETPURCHASEHISTORY_SOAP,
                               writer, 
                               d2giServiceGetPurchaseHistoryCallback, 
							   theInstance->mTimeoutMs,
                               (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n", 
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// GeT Item Activation Data   - Upcoming feature not fully supported                                        
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giServiceGetItemActivationDataCallback
//
static void d2giServiceGetItemActivationDataCallback(GHTTPResult       httpResult, 
                                                     GSXmlStreamWriter theRequestXml, 
                                                     GSXmlStreamReader theResponseXml, 
                                                     void *            theRequestData)
{
    GSResult        result = GS_SUCCESS;
    D2GIRequestData *requestData = (D2GIRequestData*)theRequestData;

    D2GGetItemActivationResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    if (httpResult == GHTTPSuccess)
    {
        result = d2giParseGetItemActivationResponse(requestData->mInstance, 
                                                    requestData->mCatalog, 
                                                    theResponseXml, 
                                                    &response);
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    }

    requestData->mUserCallback.mGetActivationDataCallback(result, &response, requestData->mUserData);

    // Done with the request free it before leaving
    gsifree(requestData);

    GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giServiceGetItemActivationData
//
GSResult d2giServiceGetItemActivationData( D2GIInstance *theInstance, 
										   D2GICatalog  *theCatalog, 
										   D2GGetItemActivationDataCallback callback,
                                           D2GPackageId thePackageId,  
                                           void         *userData)
{
  GSResult result = GS_SUCCESS;  
  GSXmlStreamWriter writer;
  D2GIRequestData   *requestData = NULL;

  // no need to use secure URL
  d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

  // make a copy of the request callback and user param
  requestData = (D2GIRequestData*)gsimalloc(sizeof(D2GIRequestData));
  if (requestData == NULL)
  {
      gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on malloc(%d) for D2GIRequestData\n", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));
      return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
  }
  requestData->mInstance = theInstance;
  requestData->mCatalog  = theCatalog;
  requestData->mUserData = userData;
  requestData->mUserCallback.mGetActivationDataCallback = callback;

  writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
  if (writer == NULL)
  {
      gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\n",
            __FILE__, __FUNCTION__, __LINE__ );
      return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
  }
  else
  {
      GSSoapTask * aTask = NULL;

      if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETITEMACTIVATIONDATA_REQUEST)) ||
          gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
          gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
          gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "packageid", thePackageId)) ||
          gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
          gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
          gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "certificate")) ||
          gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", 
                          (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
          gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
          gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, GS_D2G_GETITEMACTIVATIONDATA_REQUEST)) ||
          gsi_is_false(gsXmlCloseWriter(writer))
          )
      {
          gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data.\n", 
                __FILE__, __FUNCTION__, __LINE__ );
          
          // Should only fail to write if the buffer is full
          gsXmlFreeWriter(writer);
          return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
      }

      aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                             GS_D2G_GETITEMACTIVATIONDATA_SOAP,
                             writer, 
                             d2giServiceGetItemActivationDataCallback, 
							 theInstance->mTimeoutMs,
                             (void*)requestData);
      if (aTask == NULL)
      {
          gsXmlFreeWriter(writer);
          gsifree(requestData);
          gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n",
                __FILE__, __FUNCTION__, __LINE__ );
          return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
      }
  }

  return result;
}
///////////////////////////////////////////////////////////////////////////////
// Downloadable Content Updates          
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2giServiceCheckContentUpdatesCallback
//      Internal callback to process CheckContentUpdates Service Response. 
// 
static void d2giServiceCheckContentUpdatesCallback(GHTTPResult httpResult,
                                                   GSXmlStreamWriter theRequestXml,
                                                   GSXmlStreamReader theResponseXml,
                                                   void        *theRequestData)
                                               
{
    GSResult        result = GS_SUCCESS;
    D2GIRequestData *requestData = (D2GIRequestData*)theRequestData;
    D2GCheckContentUpdatesResponse response;
    memset(&response, 0, sizeof(response));
    response.mHttpResult = httpResult;

    if (httpResult == GHTTPSuccess)
    {
        result = d2giParseCheckContentUpdates(requestData->mInstance, 
			                                  requestData->mCatalog, 
                                              theResponseXml, 
											  &response);
		if (GS_FAILED(result))
		{			
			d2giFreeContentUpdateList(response.mContentUpdateList);
			response.mContentUpdateList = NULL;			
		}		
    }
    else if (httpResult == GHTTPRequestCancelled)
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_RequestTimedOut);
    }
    else
    {
        result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    }

    requestData->mUserCallback.mCheckContentUpdatesCallback(result, &response, requestData->mUserData);

    gsifree(requestData);

    GSI_UNUSED(httpResult);
    GSI_UNUSED(theRequestXml);
    GSI_UNUSED(theResponseXml);
    GSI_UNUSED(theRequestData);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giServiceCheckContentUpdates
//	This is called when the application offers updates on a previously downloaded
//	or purchased content. It makes a service call to the backend.
//
GSResult d2giServiceCheckContentUpdates(D2GIInstance *theInstance,
                                        D2GICatalog  *theCatalog, 
                                        gsi_bool     requiredOnly,
                                        D2GCheckContentUpdatesCallback callback, 
                                        void         *userData)
{
    GSResult    result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    D2GIRequestData * requestData = NULL;

    d2giSetServiceUrl(theInstance, GS_D2G_URL_SECURE, GS_D2G_CATALOG_SERVICE_URL_FORMAT);

    // make a copy of the request callback and user param
    requestData = (D2GIRequestData *)gsimalloc(sizeof(D2GIRequestData));
    if (requestData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "%s :: %s (%d):   Failed on malloc(%d) for D2GIRequestData\n", 
            __FILE__, __FUNCTION__, __LINE__, sizeof(D2GIRequestData));

        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    memset(requestData, 0, sizeof(D2GIRequestData));
    requestData->mInstance = theInstance;
    requestData->mCatalog  = theCatalog;
    requestData->mUserData = userData;
    requestData->mUserCallback.mCheckContentUpdatesCallback = callback;

    writer = gsXmlCreateStreamWriter(GS_D2G_NAMESPACES, GS_D2G_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask *aTask = NULL;
        
        if (gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_CHECKFORCONTENTUPDATES_REQUEST)) ||
            gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "gameid", theCatalog->mGameId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "requiredonly", requiredOnly)) ||
            gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, "downloadeditems")) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "count",0 )) ||
            gsi_is_false(d2giWriteManifestDownloadsToXML(writer, theInstance)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, "downloadeditems")) ||
            gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(wsLoginCertWriteXML (&theInstance->mCertificate, GS_D2G_NAMESPACE, writer)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, "certificate")) ||
            gsi_is_false(gsXmlWriteHexBinaryElement(writer, GS_D2G_NAMESPACE, "proof", (const gsi_u8 *)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||

            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_REQUEST)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, GS_D2G_CHECKFORCONTENTUPDATES_REQUEST)) ||
            gsi_is_false(gsXmlCloseWriter    (writer))
            )
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_CHECKFORCONTENTUPDATES_REQUEST\n",
                __FILE__, __FUNCTION__, __LINE__);
            // Should only fail to write if the buffer is full
            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        aTask = gsiExecuteSoapWithTimeout(theInstance->mServiceURL, 
                               GS_D2G_CHECKFORCONTENTUPDATES_SOAP, 
                               writer, 
                               d2giServiceCheckContentUpdatesCallback, 
							   theInstance->mTimeoutMs,
                               (void*)requestData);
       if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): Failed on gsiExecuteSoapWithTimeout\n",
                __FILE__, __FUNCTION__, __LINE__);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

    }
    return result;
};
