///////////////////////////////////////////////////////////////////////////////
// File:	gsbService.h
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited. 


// Includes
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttpCommon.h"

#include "brigades.h"
#include "gsbMain.h"
#include "gsbServices.h"
#include "gsbSerialize.h"
#include "gsbUtil.h"

// This string is used as part of the SOAP messages
const char * GSB_NAMESPACES[GSB_NAMESPACE_COUNT] =
{
    GSB_NAMESPACE "=\"http://gamespy.net/brigades/2008/08\"",
};


//Matching string for enum GSBBrigadeMemberAction
const char *GSBBrigadeMemberActionStr[] =
{
	"RequestJoin", 
	"Invite", "InviteAccept", "InviteDecline", 
	"RequestAccept", "RequestDecline", 
	"LeaveBrigade", "RemoveMember", "BanMember",
	"RescindInvite"
};

// convert the common error code enum (for response header errors) to SC error  
GSResult gsbiAuthErrorToGSResult(GSAuthErrorCode errorCode) 
{
	GSResult result;

	switch(errorCode)
	{
	case GSAuthErrorCode_InvalidGameID: 
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_INVALID_GAMEID);
		break;
	case GSAuthErrorCode_InvalidSessionToken: 
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_INVALID_SESSIONTOKEN);
		break;
	case GSAuthErrorCode_SessionTokenExpired: 
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_SESSIONTOKEN_EXPIRED);
		break;
	default:
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_UnknownError);
		break;
	}

	// reset AuthError for subsequent calls
	gsiCoreSetAuthError("");
	gsiCoreSetAuthErrorCode(0);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceUpdateBrigadeCallback(GHTTPResult       httpResult,
                                      GSXmlStreamWriter theRequestXml,
                                      GSXmlStreamReader theResponseXml,
                                      void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *updateResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &updateResultSet, GSB_UPDATEBRIGADE);
		if (GS_SUCCEEDED(result))
		{
			if (requestData->mUpdateBrigade->mBrigadeId == 0)
			{
				if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&requestData->mUpdateBrigade->mBrigadeId)))
				{
					GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADEID_ELEMENT);
					result = GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}			
		}
    }
    
    // trigger developer callback
    requestData->mUserCallback.mUpdateBrigadeCallback(result, updateResultSet, requestData->mUpdateBrigade, requestData->mUserData);

	gsifree(requestData);	
	gsResultCodesFreeResultSet(updateResultSet);
    gsifree(updateResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceUpdateBrigade(GSBInternalInstance *theInstance, GSBBrigade *theBrigade, GSBSaveBrigadeCallback callback, void * userData)
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError, "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mUpdateBrigadeCallback= callback;
		requestData->mUpdateBrigade = theBrigade;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_UPDATEBRIGADE)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsbiSerializeBrigade(theInstance, writer, theBrigade)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_UPDATEBRIGADE)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
							   GSB_UPDATEBRIGADE_SOAP, 
							   writer, 
							   gsbiServiceUpdateBrigadeCallback, 
							   (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceDisbandBrigadeCallback(GHTTPResult       httpResult,
                                       GSXmlStreamWriter theRequestXml,
                                       GSXmlStreamReader theResponseXml,
                                       void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *disbandResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &disbandResultSet, GSB_DISBANDBRIGADE);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mDisbandBrigadeCallback(result, disbandResultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(disbandResultSet);
    gsifree(disbandResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceDisbandBrigade(GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBDisbandBrigadeCallback callback, void *userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mDisbandBrigadeCallback = callback;
        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_DISBANDBRIGADE)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_DISBANDBRIGADE)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_DISBANDBRIGADE_SOAP,
            writer, 
            gsbiServiceDisbandBrigadeCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceGetBrigadeByIdCallback(GHTTPResult       httpResult,
									   GSXmlStreamWriter theRequestXml,
									   GSXmlStreamReader theResponseXml,
									   void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *getBrigadeResultSet = NULL;
	GSBBrigade *theBrigade = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_GETBRIGADEBYID);
        
		// if the service call was successful and there were no error result codes,
		// we can further parse the response for a brigade
        if (GS_SUCCEEDED(result) && getBrigadeResultSet->mNumResults == 0)
        {
			theBrigade = (GSBBrigade *)gsimalloc(sizeof(GSBBrigade));
            if (theBrigade != NULL)
            {
                memset(theBrigade, 0, sizeof(GSBBrigade));
                if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADE_ELEMENT)))    
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADE_ELEMENT);
                    result = GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
                }
                else
                {
                    result = gsbiParseBrigade(theResponseXml,theBrigade);
                }
            }
            else
            
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate GSBBrigade");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
		}       
    }

    // trigger developer callback
    requestData->mUserCallback.mGetBrigadeByIdCallback(result, getBrigadeResultSet, theBrigade, requestData->mUserData);
    
    gsifree(requestData);	
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetBrigadeById( GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBGetBrigadeByIdCallback callback, void *userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetBrigadeByIdCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEBYID)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEBYID)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
            gsXmlFreeWriter(writer);
            gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	
						
		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
		                        GSB_GETBRIGADEBYID_SOAP,
			                    writer, 
			                    gsbiServiceGetBrigadeByIdCallback, 
			                    (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceRescindInviteCallback( GHTTPResult       httpResult,
											  GSXmlStreamWriter theRequestXml,
											  GSXmlStreamReader theResponseXml,
											  void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{
		// parse response to see the result of the update request
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PERFORMBRIGADEMEMBERACTION);
	}

	// trigger developer callback
	requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, resultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceRescindInvite(GSBInternalInstance *theInstance, 
									gsi_u32 brigadeId, 
									gsi_u32 profileIdToCancelInvite, 
									GSBPerformBrigadeMemberActionCallback callback, 
									void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[ GSBBrigadeMemberAction_RescindInvite ])) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TARGETPROFILEID, profileIdToCancelInvite)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
			writer, 
			gsbiServiceRescindInviteCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceInviteToBrigadeCallback( GHTTPResult       httpResult,
                                             GSXmlStreamWriter theRequestXml,
                                             GSXmlStreamReader theResponseXml,
                                             void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *inviteResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &inviteResultSet, GSB_PERFORMBRIGADEMEMBERACTION);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, inviteResultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(inviteResultSet);
    gsifree(inviteResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceInviteToBrigade(GSBInternalInstance *theInstance, 
									gsi_u32 brigadeId, 
									gsi_u32 profileIdToInvite, 
									GSBPerformBrigadeMemberActionCallback callback, 
									void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[ GSBBrigadeMemberAction_Invite ])) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TARGETPROFILEID, profileIdToInvite)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
					GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
					writer, 
					gsbiServiceInviteToBrigadeCallback, 
					(void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/*  Answer Invite                                                      */
/************************************************************************/
///////////////////////////////////////////////////////////////////////////////
/**
*   @function   gsbiServiceAnswerInviteCallback
*   @param      httpResult
*   @param      theRequestXml
*   @param      theResponseXml 
*   @param      theRequestData 
*   @return     None
*/
static void gsbiServiceAnswerInviteCallback( GHTTPResult       httpResult,
                                             GSXmlStreamWriter theRequestXml,
                                             GSXmlStreamReader theResponseXml,
                                             void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PERFORMBRIGADEMEMBERACTION);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, resultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceAnswerInvite(GSBInternalInstance *theInstance, 
								 gsi_u32 brigadeId, 
								 gsi_bool acceptInvite, 
								 GSBPerformBrigadeMemberActionCallback callback, 
								 void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[(acceptInvite ? GSBBrigadeMemberAction_InviteAccept : GSBBrigadeMemberAction_InviteDecline)])) ||			
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
            writer, 
            gsbiServiceAnswerInviteCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceJoinBrigadeCallback(GHTTPResult       httpResult,
                                           GSXmlStreamWriter theRequestXml,
                                           GSXmlStreamReader theResponseXml,
                                           void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *joinResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &joinResultSet, GSB_PERFORMBRIGADEMEMBERACTION);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, joinResultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(joinResultSet);
    gsifree(joinResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceJoinBrigade(GSBInternalInstance *theInstance, 
								gsi_u32 brigadeId, 
								GSBPerformBrigadeMemberActionCallback callback, 
								void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[ GSBBrigadeMemberAction_RequestJoin ])) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
            writer, 
            gsbiServiceJoinBrigadeCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceAnswerJoinCallback( GHTTPResult       httpResult,
										   GSXmlStreamWriter theRequestXml,
										   GSXmlStreamReader theResponseXml,
										   void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{
		// parse response to see the result of the update request
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PERFORMBRIGADEMEMBERACTION);
	}

	// trigger developer callback
	requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, resultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceAnswerJoin(GSBInternalInstance  *theInstance, 
                               gsi_u32              brigadeId, 
                               gsi_u32              playerProfileId, 
                               gsi_bool             acceptJoin, 
                               GSBPerformBrigadeMemberActionCallback callback, 
                               void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[(acceptJoin ? GSBBrigadeMemberAction_RequestAccept : GSBBrigadeMemberAction_RequestDecline)])) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TARGETPROFILEID, playerProfileId)) ||			
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
			writer, 
			gsbiServiceAnswerJoinCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceRemoveBrigadeMemberCallback(GHTTPResult       httpResult,
                                            GSXmlStreamWriter theRequestXml,
                                            GSXmlStreamReader theResponseXml,
                                            void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *removeMemberResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &removeMemberResultSet, GSB_PERFORMBRIGADEMEMBERACTION);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, removeMemberResultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(removeMemberResultSet);
    gsifree(removeMemberResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceRemoveBrigadeMember(GSBInternalInstance *theInstance, 
										gsi_u32 brigadeId, 
										gsi_u32 profileIdToRemove, 
										GSBPerformBrigadeMemberActionCallback callback, 
										void * userData)
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[GSBBrigadeMemberAction_RemoveMember])) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TARGETPROFILEID, profileIdToRemove)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
            writer, 
            gsbiServiceRemoveBrigadeMemberCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceBanMemberCallback(GHTTPResult       httpResult,
										 GSXmlStreamWriter theRequestXml,
										 GSXmlStreamReader theResponseXml,
										 void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{
		// parse response to see the result of the update request
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PERFORMBRIGADEMEMBERACTION);
	}

	// trigger developer callback
	requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, resultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceBanMember(GSBInternalInstance *theInstance, 
							  gsi_u32 brigadeId, 
							  gsi_u32 profileIdToBan, 
							  GSBPerformBrigadeMemberActionCallback callback, 
							  void * userData)
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[GSBBrigadeMemberAction_BanMember])) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TARGETPROFILEID, profileIdToBan)) ||			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
			writer, 
			gsbiServiceBanMemberCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceLeaveBrigadeCallback(GHTTPResult       httpResult,
										   GSXmlStreamWriter theRequestXml,
										   GSXmlStreamReader theResponseXml,
										   void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{
		// parse response to see the result of the update request
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PERFORMBRIGADEMEMBERACTION);
	}

	// trigger developer callback
	requestData->mUserCallback.mPerformBrigadeMemberActionCallback(result, resultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceLeaveBrigade(GSBInternalInstance *theInstance, 
								 gsi_u32 brigadeId, 
								 GSBPerformBrigadeMemberActionCallback callback, 
								 void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mPerformBrigadeMemberActionCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GSB_NAMESPACE, GSB_ACTION_ELEMENT, GSBBrigadeMemberActionStr[ GSBBrigadeMemberAction_LeaveBrigade ])) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PERFORMBRIGADEMEMBERACTION)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_PERFORMBRIGADEMEMBERACTION_SOAP,
			writer, 
			gsbiServiceLeaveBrigadeCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServicePromoteLeaderCallback(GHTTPResult       httpResult,
                                            GSXmlStreamWriter theRequestXml,
                                            GSXmlStreamReader theResponseXml,
                                            void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
    GSResultSet *resultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_PROMOTETOLEADER);
    }

    // trigger developer callback
    requestData->mUserCallback.mPromoteToLeaderCallback(result, resultSet, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
    GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServicePromoteToLeader(GSBInternalInstance *theInstance, 
                              gsi_u32 brigadeId, 
                              gsi_u32 memberId, 
                              GSBPromoteToLeaderCallback callback, 
                              void * userData)
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mPromoteToLeaderCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_PROMOTETOLEADER)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEMEMBERID_ELEMENT,memberId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_PROMOTETOLEADER)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_PROMOTETOLEADER_SOAP,
            writer, 
            gsbiServicePromoteLeaderCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetEntitlementListCallback(GHTTPResult       httpResult,
												  GSXmlStreamWriter theRequestXml,
												  GSXmlStreamReader theResponseXml,
												  void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *getEntitResultSet = NULL;
	GSBEntitlementList *entitlementList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &getEntitResultSet, GSB_GETENTITLEMENTLIST);
        
        if (GS_SUCCEEDED(result))
        {
			GSResult resultParse;

			entitlementList = (GSBEntitlementList *)gsimalloc(sizeof(GSBEntitlementList));
			resultParse = gsbiParseEntitlements(theResponseXml,entitlementList);
			if (GS_FAILED(resultParse))
				result = resultParse;
		}       
    }

    // trigger developer callback
    requestData->mUserCallback.mGetEntitlementListCallback(result, getEntitResultSet,entitlementList, requestData->mUserData);
    
    gsifree(requestData);	
	gsResultCodesFreeResultSet(getEntitResultSet);
    gsifree(getEntitResultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetEntitlementList(GSBInternalInstance *theInstance, GSBGetEntitlementListCallback callback, void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetEntitlementListCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETENTITLEMENTLIST)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETENTITLEMENTLIST)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_GETENTITLEMENTLIST_SOAP,
            writer, 
            gsbiServiceGetEntitlementListCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetRoleListCallback( GHTTPResult       httpResult,
                                             GSXmlStreamWriter theRequestXml,
                                             GSXmlStreamReader theResponseXml,
                                             void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *getRoleListResultSet = NULL;
	GSBRoleList *roleList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &getRoleListResultSet, GSB_GETROLELISTBYBRIGADEID);
        
        if (GS_SUCCEEDED(result))
        {
			result = gsbiParseRoleList(theResponseXml, &roleList);
		}       
    }

    // trigger developer callback
    requestData->mUserCallback.mGetRoleListCallback(result, getRoleListResultSet,roleList, requestData->mUserData);
    
    gsifree(requestData);
    gsResultCodesFreeResultSet(getRoleListResultSet);
    gsifree(getRoleListResultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetRoleList(GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBGetRoleListCallback callback, void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError,"Failed to gsimalloc GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
       
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetRoleListCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETROLELISTBYBRIGADEID)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETROLELISTBYBRIGADEID )) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
                               GSB_GETROLELIST_SOAP,
							   writer, 
							   gsbiServiceGetRoleListCallback, 
							   (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "gsiExecuteSoap failed");
            return GSB_ERROR(GSResultSection_State, GSResultCode_OperationFailed);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetEntitlementIdListByRoleIdCallback(GHTTPResult httpResult,
													   GSXmlStreamWriter theRequestXml,
													   GSXmlStreamReader theResponseXml,
													   void *theRequestData)
{
    GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *fillResultSet = NULL;
	GSBEntitlementIdList *entitlementIdList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &fillResultSet, GSB_GETROLEENTITLEMENTLISTBYROLEID);
        
        if (GS_SUCCEEDED(result))
        {
			GSResult resultParse;
			entitlementIdList = (GSBEntitlementIdList *)gsimalloc(sizeof(GSBEntitlementIdList));
            if (entitlementIdList != NULL)
            { 
                memset(entitlementIdList,0,sizeof(GSBEntitlementIdList));
			    resultParse = gsbiParseRoleEntitlementsForEntitlementIds(theResponseXml,entitlementIdList);
			    if (GS_FAILED(resultParse))
			    {
				    result = resultParse;
			    }
            }
            else
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                    "Failed to allocating memory for GSBEntitlementIdList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
		}       
    }

    // trigger developer callback
    requestData->mUserCallback.mGetEntitlementIdListByRoleIdCallback(result, fillResultSet, entitlementIdList, requestData->mUserData);
    
    gsifree(requestData);	
	gsResultCodesFreeResultSet(fillResultSet);
    gsifree(fillResultSet);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetEntitlementIdListByRoleId(GSBInternalInstance *theInstance, 
												   gsi_u32 brigadeId, 
												   gsi_u32 roleId, 
												   GSBGetEntitlementIdListByRoleIdCallback callback, 
												   void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mGetEntitlementIdListByRoleIdCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETROLEENTITLEMENTLISTBYROLEID )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_ROLEID_ELEMENT, roleId)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETROLEENTITLEMENTLISTBYROLEID )) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_GETROLEENTITLEMENTLISTBYROLEID_SOAP,
			writer, 
			gsbiServiceGetEntitlementIdListByRoleIdCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetRoleIdListByEntitlementIdCallback(GHTTPResult httpResult,
															  GSXmlStreamWriter theRequestXml,
															  GSXmlStreamReader theResponseXml,
															  void *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *fillResultSet = NULL;
	GSBRoleIdList *roleIdList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
		// parse results first
		result = gsbiParseResponseResult(theResponseXml, &fillResultSet, GSB_GETROLEIDLISTBYENTITLEMENTID);

		if (GS_SUCCEEDED(result))
		{
			GSResult resultParse;
			roleIdList = (GSBRoleIdList *)gsimalloc(sizeof(GSBRoleIdList));
            if (roleIdList != NULL)
            {
                memset(roleIdList, 0 , sizeof(GSBRoleIdList));
                
                resultParse = gsbiParseRoleEntitlementsForRoleIds(theResponseXml, roleIdList);
			    if (GS_FAILED(resultParse))
			    {
				    result = resultParse;
			    }
            }
            else
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                    "Failed to allocating memory for GSBRoleIdList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }

		}       
	}

	// trigger developer callback
	requestData->mUserCallback.mGetRoleIdListByEntitlementIdCallback(result, fillResultSet, roleIdList, requestData->mUserData);

	gsifree(requestData);	
	gsResultCodesFreeResultSet(fillResultSet);
    gsifree(fillResultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetRoleIdListByEntitlementId(GSBInternalInstance *theInstance, 
														  gsi_u32 brigadeId, 
														  gsi_u32 entitlementId, 
														  GSBGetRoleIdListByEntitlementIdCallback callback, 
														  void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mGetRoleIdListByEntitlementIdCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETROLEIDLISTBYENTITLEMENTID )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_ENTITLEMENTID_ELEMENT, entitlementId)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETROLEIDLISTBYENTITLEMENTID )) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_GETROLEENTITLEMENTLISTBYENTITLEMENTID_SOAP,
			writer, 
			gsbiServiceGetRoleIdListByEntitlementIdCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetPendingInvitesAndJoinsCallback( GHTTPResult       httpResult,
														  GSXmlStreamWriter theRequestXml,
														  GSXmlStreamReader theResponseXml,
														  void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *resultSet = NULL;
	GSBBrigadePendingActionsList *actionList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
		// parse results first
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS );

		if (GS_SUCCEEDED(result))
		{
			GSResult resultParse;

			actionList = (GSBBrigadePendingActionsList *)gsimalloc(sizeof(GSBBrigadePendingActionsList));
            if (actionList != NULL)
            {
			    memset(actionList,0, sizeof(GSBBrigadePendingActionsList));
			    resultParse = gsbiParsePendingActions(theResponseXml, actionList);
			    if (GS_FAILED(resultParse))
			    {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse soap message");

				    //TODO better error handling
				    result = resultParse;
			    }
            }
            else
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                    "Failed to allocating memory for GSBBrigadePendingActionsList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
		}       
	}

	// trigger developer callback
	requestData->mUserCallback.mGetPendingInvitesAndJoinsCallback(result, resultSet, actionList, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetPendingInvitesAndJoins(GSBInternalInstance *theInstance, 
											  GSBGetPendingInvitesAndJoinsCallback callback, 
											  void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	
	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;
        int          memberStatus =  (GSBBrigadeMemberStatus_Invited | GSBBrigadeMemberStatus_RequestJoin) ; 

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mGetPendingInvitesAndJoinsCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_STATUSES_ELEMENT, memberStatus )) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS )) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "gsbiServiceGetPendingInvitesAndJoins: could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS_SOAP,
			writer, 
			gsbiServiceGetPendingInvitesAndJoinsCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceGetBrigadeMemberListCallback( GHTTPResult       httpResult,
                                             GSXmlStreamWriter theRequestXml,
                                             GSXmlStreamReader theResponseXml,
                                             void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
	GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
	GSResultSet *getMembListResultSet = NULL;
	GSBBrigadeMemberList *memberList = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &getMembListResultSet, GSB_GETBRIGADEMEMBERLISTBYSTATUS);
        
        if (GS_SUCCEEDED(result))
        {
			GSResult resultParse;
			
			memberList = (GSBBrigadeMemberList *)gsimalloc(sizeof(GSBBrigadeMemberList));
            if (memberList != NULL)
            {
			    memset(memberList,0, sizeof(GSBBrigadeMemberList));
			    resultParse = gsbiParseBrigadeMembers(theResponseXml, memberList);
			    if (GS_FAILED(resultParse))
			    {
                    //TODO better error handling
				    result = resultParse;
			    }
            }
            else
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                    "Failed to allocating memory for GSBBrigadeMemberList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
		}       
    }

    // trigger developer callback
    requestData->mUserCallback.mGetBrigadeMemberListCallback(result, getMembListResultSet, memberList, requestData->mUserData);
    
    gsifree(requestData);
	gsResultCodesFreeResultSet(getMembListResultSet);
    gsifree(getMembListResultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceGetBrigadeMemberList(GSBInternalInstance *theInstance, 
										 gsi_u32 brigadeId, 
										 gsi_u32 statuses, 
										 GSBGetBrigadeMemberListCallback callback, 
										 void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetBrigadeMemberListCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMEMBERLISTBYSTATUS )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_STATUSES_ELEMENT, statuses)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMEMBERLISTBYSTATUS )) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "gsbiServiceGetBrigadeMemberList: could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_GETBRIGADEMEMBERLISTBYSTATUS_SOAP,
            writer, 
            gsbiServiceGetBrigadeMemberListCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceUpdateBrigadeMemberCallback(GHTTPResult       httpResult,
												   GSXmlStreamWriter theRequestXml,
												   GSXmlStreamReader theResponseXml,
                                                   void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *updateResultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &updateResultSet, GSB_UPDATEBRIGADEMEMBER);
    }
    
    // trigger developer callback
    requestData->mUserCallback.mUpdateBrigadeMemberCallback(result, updateResultSet, requestData->mUserData);

    gsifree(requestData);
	gsResultCodesFreeResultSet(updateResultSet);
    gsifree(updateResultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceUpdateBrigadeMember(GSBInternalInstance *theInstance, 
										GSBBrigadeMember *theMember, 
										GSBUpdateBrigadeMemberCallback callback, 
										void * userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mUpdateBrigadeMemberCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_UPDATEBRIGADEMEMBER )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, theMember->mBrigadeId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEMEMBERID_ELEMENT, theMember->mBrigadeMemberId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_ROLEID_ELEMENT, theMember->mRoleId)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_CUSTOMTITLE_ELEMENT, theMember->mTitle)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_UPDATEBRIGADEMEMBER )) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
			                   GSB_UPDATEBRIGADEMEMBER_SOAP,
							   writer, 
							   gsbiServiceUpdateBrigadeMemberCallback, 
							   (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceUpdateRoleCallback(GHTTPResult		httpResult,
										  GSXmlStreamWriter	theRequestXml,
										  GSXmlStreamReader	theResponseXml,
										  void				*theRequestData)
{
    gsi_u32 roleId = 0;
    GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
    	// parse results first
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_UPDATEROLE);
        
        if (GS_SUCCEEDED(result))
        {
			// if we got here it was a success, now get the Role Id
             if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ROLEID_ELEMENT, (int *) &roleId)))
             {
                 GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse RoleId");
                 result = GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
             }                
        }       
    }

    // trigger developer callback
    requestData->mUserCallback.mUpdateRoleCallback(result, resultSet, roleId, requestData->mUserData);
    
    gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceUpdateRole(GSBInternalInstance *theInstance, GSBRole *theRole, GSBEntitlementIdList *entitlementIdList, GSBUpdateRoleCallback callback, void * userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mUpdateRoleCallback = callback;

		

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_UPDATEROLE )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsbiSerializeRole(writer, theRole)) ||
			gsi_is_false(gsbiSerializeEntitlementIDs(writer, entitlementIdList)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_UPDATEROLE )) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
            gsXmlFreeWriter(writer);
            gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	
						
		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
		                        GSB_UPDATEROLE_SOAP,
			                    writer, 
			                    gsbiServiceUpdateRoleCallback, 
			                    (void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceRemoveRoleCallback(GHTTPResult		httpResult,
										  GSXmlStreamWriter	theRequestXml,
										  GSXmlStreamReader	theResponseXml,
										  void				*theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *resultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
		result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_REMOVEROLE);
	}

	// trigger developer callback
	requestData->mUserCallback.mRemoveRoleCallback(result, resultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceRemoveRole(GSBInternalInstance		*theInstance, 
							   gsi_u32					brigadeId, 
							   gsi_u32					roleId, 
							   GSBRemoveRoleCallback	callback, 
							   void *					userData )
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mRemoveRoleCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REMOVEROLE )) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_ROLEID_ELEMENT, roleId)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_REMOVEROLE )) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);

			// Should only fail to write if the buffer is full
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap( theInstance->mServiceURL, 
			GSB_REMOVEROLE_SOAP,
			writer, 
			gsbiServiceRemoveRoleCallback, 
			(void*)requestData);
		if (aTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseBrigadeList(GSXmlStreamReader theResponseXml,
                              GSBBrigadeList    *brigadeList)
{

    GSResult result = GS_SUCCESS;
    gsi_u32 i = 0;

    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADELIST_ELEMENT)) ||
        gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&brigadeList->mCount)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
    if (brigadeList->mCount == 0 )
    {
        brigadeList->mBrigades = NULL;
        return GS_SUCCESS;
    }

    brigadeList->mBrigades = (GSBBrigade *)gsimalloc(sizeof(GSBBrigade) * brigadeList->mCount);
    if (brigadeList->mBrigades == NULL)
    {
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate brigadeList->mBrigades");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    memset(brigadeList->mBrigades, 0,(sizeof(GSBBrigade) * brigadeList->mCount) );

    for (i = 0; i < brigadeList->mCount; i++)
    {
        if (i==0)
        {
            if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADE_ELEMENT)))    
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADE_ELEMENT);
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
            }
        }
        else
        if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_BRIGADE_ELEMENT)))
        {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADE_ELEMENT);
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }

        result = gsbiParseBrigade(theResponseXml, &brigadeList->mBrigades[i]);
        if (GS_FAILED(result))
        {
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADE_ELEMENT);
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }
    }
    return result; 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceSearchBrigadesCallback(GHTTPResult       httpResult,
                                       GSXmlStreamWriter theRequestXml,
                                       GSXmlStreamReader theResponseXml,
                                       void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
    GSResultSet *getBrigadeResultSet = NULL;
    GSBBrigadeList  *brigadeList = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_SEARCHBRIGADE);

        if (GS_SUCCEEDED(result))
        {
            brigadeList = (GSBBrigadeList*) gsimalloc(sizeof(GSBBrigadeList));
            if (brigadeList == NULL)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate brigadeList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
            else
            {
                memset(brigadeList, 0, sizeof(GSBBrigadeList));

                result = gsbiParseBrigadeList(theResponseXml, brigadeList);
                if (GS_FAILED(result))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse brigadeList");
                }
            }
        }
    }

    // trigger developer callback
    requestData->mUserCallback.mSearchBrigadesCallback(result, getBrigadeResultSet, brigadeList, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
    GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceSearchBrigades( GSBInternalInstance *theInstance, GSBSearchFilterList *filterList, GSBSearchBrigadesCallback callback, void *userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
		GSSoapTask * aTask = NULL;
		gsi_u32 i = 0;
		//gsi_bool SearchParamFound[GSBSearchFilterKey_Max] = {gsi_false} ;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mSearchBrigadesCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_SEARCHBRIGADE )) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        for (i=0 ; i<filterList->mCount ; i++)
        {

            switch (filterList->mFilters[i].mKey)
            {
                case GSBSearchFilterKey_Name :
                case GSBSearchFilterKey_Tag  :
                    if gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_SEARCHPATTERN_ELEMENT,filterList->mFilters[i].mValue.mValueStr ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, SearchPattern");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break ;
                case GSBSearchFilterKey_BrigadeId:
                    if gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, filterList->mFilters[i].mValue.mValueUint ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, BrigadeID");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break ;

                case GSBSearchFilterKey_BrigadeStatus :
                    if gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADERECRUITINGTYPE_ELEMENT, filterList->mFilters[i].mValue.mValueUint ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, BrigadeRecruitingType");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break ;
                case GSBSearchFilterKey_DateCreated :
                    if gsi_is_false(gsXmlWriteDateTimeElement(writer, GSB_NAMESPACE, GSB_DATECREATED_ELEMENT, filterList->mFilters[i].mValue.mValueTime ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, DateCreated");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break;
                case GSBSearchFilterKey_ExtraParams :
                    if gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_EXTRAPARAMS_ELEMENT, filterList->mFilters[i].mValue.mValueStr ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, ExtraParams");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break;

                case GSBSearchFilterKey_MaxResultCount :
                    if gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_MAXRESULTS_ELEMENT, filterList->mFilters[i].mValue.mValueUint ))
                    {
                        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, MaxResults");		
                        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                    }
                    break;
                default :
                    // Unexpected search criteria
                    // So we should return with an error, do not proceed.
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Unexpected search criteria %d",filterList->mFilters[i].mKey);		
                    return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaUnexpected);
                    break;
            }
            //SearchParamFound[filterList->mFilters[i].mKey] = gsi_true;
        }

        // Go through the search filter and validate each item in the filter
        if (gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_SEARCHBRIGADE)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_SEARCHBRIGADE_SOAP,
            writer, 
            gsbiServiceSearchBrigadesCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceSearchPlayersCallback(GHTTPResult       httpResult,
                                       GSXmlStreamWriter theRequestXml,
                                       GSXmlStreamReader theResponseXml,
                                       void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
    GSResultSet *getBrigadeResultSet = NULL;
    GSBBrigadeMemberList  *brigadeMemberList = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_SEARCHPLAYER);

        if (GS_SUCCEEDED(result))
        {
            brigadeMemberList = (GSBBrigadeMemberList*) gsimalloc(sizeof(GSBBrigadeMemberList));
            if (brigadeMemberList == NULL)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate brigadeMemberList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
            else
            {
                memset(brigadeMemberList, 0, sizeof(GSBBrigadeList));

                result = gsbiParsePlayers(theResponseXml, brigadeMemberList );
                if (GS_FAILED(result))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse brigadeList");
                }
            }
        }
    }

    // trigger developer callback
    requestData->mUserCallback.mSearchPlayersCallback(result, 
                                                       getBrigadeResultSet, 
                                                       brigadeMemberList, 
                                                       requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
    GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceSearchPlayers( GSBInternalInstance *theInstance, GSBSearchFilterList *filterList, GSBSearchPlayersCallback callback, void *userData )
{
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;
        gsi_u32 i = 0;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                "Failed on gsimalloc(%d) for GSBIRequestData", sizeof(GSBIRequestData));
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        memset(requestData, 0, sizeof(GSBIRequestData)); // clear out request data
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mSearchPlayersCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_SEARCHPLAYER)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        for (i =0; i < filterList->mCount; i++)
        {
            switch (filterList->mFilters[i].mKey)
            {
            case GSBSearchFilterKey_Name :
            case GSBSearchFilterKey_Tag  :
                if gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_SEARCHPATTERN_ELEMENT, filterList->mFilters[i].mValue.mValueStr ))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, SearchPattern");		
                    return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                }
                break ; 
            case GSBSearchFilterKey_BrigadeId :
                if gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_ONBRIGADE_ELEMENT, filterList->mFilters[i].mValue.mValueUint ))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, OnBrigade");		
                    return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                }
                break ; 

            case GSBSearchFilterKey_ExtraParams :
                if gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_EXTRAPARAMS_ELEMENT, filterList->mFilters[i].mValue.mValueStr ))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, ExtraParams");		
                    return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                }
                break;
            case GSBSearchFilterKey_MaxResultCount :
                if gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_MAXRESULTS_ELEMENT, filterList->mFilters[i].mValue.mValueUint ))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data, MaxResults");		
                    return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
                }
                break;
            default :
                break;
            }
        }

        // Go through the search filter and validate each item in the filter
        if (gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_SEARCHPLAYER)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);

            // Should only fail to write if the buffer is full
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "could not write request data");		
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap( theInstance->mServiceURL, 
            GSB_SEARCHPLAYER_SOAP,
            writer, 
            gsbiServiceSearchPlayersCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceSaveBrigadeLogoFileIdCallback(GHTTPResult       httpResult,
											  GSXmlStreamWriter theRequestXml,
											  GSXmlStreamReader theResponseXml,
											  void              *theRequestData)
{
	GSResult result = GS_SUCCESS;
	GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
	GSResultSet *saveBrigLogoResultSet = NULL;

	if (httpResult == GHTTPRequestCancelled)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
	else if (httpResult != GHTTPSuccess)
		result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
	else 
	{ 	    
		// parse results first
		result = gsbiParseResponseResult(theResponseXml, &saveBrigLogoResultSet, GSB_SAVEBRIGADELOGO); 
	}

	// trigger developer callback
	requestData->mUserCallback.mUploadLogoCompleteCallback(result, saveBrigLogoResultSet, requestData->mUserData);

	gsifree(requestData);
	gsResultCodesFreeResultSet(saveBrigLogoResultSet);
    gsifree(saveBrigLogoResultSet);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceSaveBrigadeLogoFileId(GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 fileId, GSBUploadLogoCompleteCallback callback, void * userData)
{
	GSXmlStreamWriter writer;
	GSBIRequestData   *requestData = NULL;

	gsbiSetServiceUrl(theInstance);
	writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
	if (writer == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
			"Failed on gsXmlCreateStreamWriter (assuming out of memory)");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		GSSoapTask * aTask = NULL;

		// make a copy of the request callback and user param
		requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
		if (requestData == NULL)
		{
			gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
		memset(requestData, 0, sizeof(GSBIRequestData));
		requestData->mInstance = theInstance;
		requestData->mUserData = userData;
		requestData->mUserCallback.mUploadLogoCompleteCallback = callback;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_SAVEBRIGADELOGO)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_FILEID_ELEMENT, fileId)) ||
			gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_SAVEBRIGADELOGO)) ||
			gsi_is_false(gsXmlCloseWriter(writer)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}	

		aTask = gsiExecuteSoap(theInstance->mServiceURL, 
			                   GSB_SAVEBRIGADELOGO_SOAP,
							   writer, 
							   gsbiServiceSaveBrigadeLogoFileIdCallback, 
							   (void*)requestData);
		if (aTask == NULL)
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiServiceSendBrigadeMessageCallback(GHTTPResult       httpResult,
                                        GSXmlStreamWriter theRequestXml,
                                        GSXmlStreamReader theResponseXml,
                                        void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
    GSResultSet *resultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_SENDTEAMMESSAGE); 
    }

    // trigger developer callback
    requestData->mUserCallback.mUploadLogoCompleteCallback(result, resultSet, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceSendMessageToBrigade ( GSBInternalInstance *theInstance, 
                                      gsi_u32             brigadeId,  
                                      UCS2String          message, 
                                      GSBSendMessageToBrigadeCallback callback, 
                                      void               *userData)
{
    GSResult          result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mSendMessageToBrigadeCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_SENDTEAMMESSAGE)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, brigadeId)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_MESSAGE_ELEMENT, message)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_SENDTEAMMESSAGE)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_SENDTEAMMESSAGE_SOAP,
            writer, 
            gsbiServiceSendBrigadeMessageCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsbiServiceSendMemberMessageCallback(GHTTPResult       httpResult,
                                                 GSXmlStreamWriter theRequestXml,
                                                 GSXmlStreamReader theResponseXml,
                                                 void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
    GSResultSet *resultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_SENDMEMBERMESSAGE); 
    }

    // trigger developer callback
    requestData->mUserCallback.mSendMessageToMemberCallback(result, resultSet, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
    GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiServiceSendMessageToMember(GSBInternalInstance *theInstance,  
                                        gsi_u32             toProfileId, 
                                        UCS2String          message, 
                                        GSBSendMessageToMemberCallback callback, 
                                        void               *userData)
{
    GSResult          result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mSendMessageToMemberCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_SENDMEMBERMESSAGE)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, GSB_TOPROFILEID_ELEMENT , toProfileId)) ||
            gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GSB_NAMESPACE, GSB_MESSAGE_ELEMENT, message)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_SENDMEMBERMESSAGE)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_SENDMEMBERMESSAGE_SOAP,
            writer, 
            gsbiServiceSendMemberMessageCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;

}

static void gsbiServiceGetBrigadesByProfileIdCallback(GHTTPResult       httpResult,
                                                      GSXmlStreamWriter theRequestXml,
                                                      GSXmlStreamReader theResponseXml,
                                                      void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
    GSResultSet *getBrigadeResultSet = NULL;
    GSBBrigadeList  *brigadeList = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_GETBRIGADESBYPROFILEID);

        if (GS_SUCCEEDED(result))
        {
            brigadeList = (GSBBrigadeList*) gsimalloc(sizeof(GSBBrigadeList));
            if (brigadeList == NULL)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate brigadeList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
            else
            {
                memset(brigadeList, 0, sizeof(GSBBrigadeList));

                result = gsbiParseBrigadeList(theResponseXml, brigadeList);
                if (GS_FAILED(result))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse brigadeList");
                }
            }
        }
    }

    // trigger developer callback
    requestData->mUserCallback.mGetBrigadesByProfileIdCallback(result, getBrigadeResultSet, brigadeList, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
    GSI_UNUSED(theRequestXml);
}
GSResult gsbiServiceGetBrigadesByProfileId(GSBInternalInstance *theInstance, 
                                           gsi_u32 profileId, 
                                           GSBGetBrigadesByProfileIdCallback callback, 
                                           void *userData)
{
    GSResult          result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetBrigadesByProfileIdCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADESBYPROFILEID)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "TargetProfileID", profileId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADESBYPROFILEID)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_GETBRIGADESBYPROFILEID_SOAP,
            writer, 
            gsbiServiceGetBrigadesByProfileIdCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;
}

static void gsbiGetBrigadeHistoryCallback(GHTTPResult       httpResult,
                                          GSXmlStreamWriter theRequestXml,
                                          GSXmlStreamReader theResponseXml,
                                          void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
    GSResultSet *getBrigadeResultSet = NULL;
    GSBBrigadeHistoryList  *historyList = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_GETBRIGADEHISTORY);

        if (GS_SUCCEEDED(result))
        {
            historyList = (GSBBrigadeHistoryList*) gsimalloc(sizeof(GSBBrigadeHistoryList));
            if (historyList == NULL)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate historyList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
            else
            {
                memset(historyList, 0, sizeof(GSBBrigadeHistoryList));

                result = gsbiParseHistoryList(theResponseXml, historyList);
                if (GS_FAILED(result))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse historyList");
                }
            }
        }
    }

    // trigger developer callback
    requestData->mUserCallback.mGetBrigadeHistoryCallback(result, getBrigadeResultSet, historyList, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
    GSI_UNUSED(theRequestXml);
    
}

GSResult gsbiServiceGetBrigadeHistory( GSBInternalInstance *theInstance,
                                       gsi_u32 brigadeId, 
                                       gsi_u32 profileId, 
                                       GSBBrigadeHistoryAccessLevel historyAccessLevel, 
                                       GSBGetBrigadeHistoryCallback callback, 
                                       void    *userData)
{
    GSResult result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetBrigadeHistoryCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEHISTORY)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "BrigadeID", brigadeId)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        if (profileId != 0) 
        {
            if (gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "TargetProfileID", profileId)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
                gsXmlFreeWriter(writer);
                gsifree(requestData);
                return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);

            }
        }

        if (historyAccessLevel != GSBBrigadeHistoryAccessLevel_None)
        {
            if (gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "BrigadeHistoryEntryType", historyAccessLevel)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
                gsXmlFreeWriter(writer);
                gsifree(requestData);
                return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);

            }
        }

        if (gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEHISTORY)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_GETBRIGADEHISTORY_SOAP,
            writer, 
            gsbiGetBrigadeHistoryCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;
}


static void gsbiGetBrigadeMatchHistoryCallback(GHTTPResult       httpResult,
                                          GSXmlStreamWriter theRequestXml,
                                          GSXmlStreamReader theResponseXml,
                                          void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData *requestData = (GSBIRequestData *)theRequestData;
    GSResultSet *getBrigadeResultSet = NULL;
    GSBBrigadeHistoryList  *historyList = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
    else 
    { 	    
        // parse results first
        result = gsbiParseResponseResult(theResponseXml, &getBrigadeResultSet, GSB_GETBRIGADEMATCHHISTORY);

        if (GS_SUCCEEDED(result))
        {
            historyList = (GSBBrigadeHistoryList*) gsimalloc(sizeof(GSBBrigadeHistoryList));
            if (historyList == NULL)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate historyList");
                result =  GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
            }
            else
            {
                memset(historyList, 0, sizeof(GSBBrigadeHistoryList));

                result = gsbiParseHistoryList(theResponseXml, historyList);
                if (GS_FAILED(result))
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse historyList");
                }
            }
        }
    }

    // trigger developer callback
    requestData->mUserCallback.mGetBrigadeHistoryCallback(result, getBrigadeResultSet, historyList, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(getBrigadeResultSet);
    gsifree(getBrigadeResultSet);
    GSI_UNUSED(theRequestXml);

}

GSResult gsbiServiceGetBrigadeMatchHistory( GSBInternalInstance *theInstance, 
                                            gsi_u32 matchId, 
                                            GSBGetBrigadeMatchHistoryCallback callback, 
                                            void *userData)
{
    GSResult result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mGetBrigadeMatchHistoryCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMATCHHISTORY)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "MatchID", matchId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_GETBRIGADEMATCHHISTORY)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_GETBRIGADEMATCHHISTORY_SOAP,
            writer, 
            gsbiGetBrigadeMatchHistoryCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;
}

static void gsbiServiceUpdateMemberEmailAndNickCallback( GHTTPResult       httpResult,
                                            GSXmlStreamWriter theRequestXml,
                                            GSXmlStreamReader theResponseXml,
                                            void              *theRequestData)
{
    GSResult result = GS_SUCCESS;
    GSBIRequestData * requestData = (GSBIRequestData*)theRequestData;
    GSResultSet *resultSet = NULL;

    if (httpResult == GHTTPRequestCancelled)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_OperationCancelled);
    else if (httpResult != GHTTPSuccess)
        result = GSB_ERROR(GSResultSection_Soap, GSResultCode_HttpError);
	else if (gsiCoreGetAuthErrorCode() != GSAuthErrorCode_None)
		result = gsbiAuthErrorToGSResult(gsiCoreGetAuthErrorCode());
    else 
    {
        // parse response to see the result of the update request
        result = gsbiParseResponseResult(theResponseXml, &resultSet, GSB_UPDATEMEMBEREMAILANDNICK);
    }

    // trigger developer callback
    requestData->mUserCallback.mUpdateEmailAndNickCallback(result, resultSet, requestData->mUserData);

    gsifree(requestData);
    gsResultCodesFreeResultSet(resultSet);
    gsifree(resultSet);
    GSI_UNUSED(theRequestXml);
}

GSResult gsbiServiceUpdateMemberEmailAndNick( GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBUpdateMemberEmailAndNickCallback callback, void *userData )
{
    GSResult result = GS_SUCCESS;
    GSXmlStreamWriter writer;
    GSBIRequestData   *requestData = NULL;

    gsbiSetServiceUrl(theInstance);
    writer = gsXmlCreateStreamWriter(GSB_NAMESPACES, GSB_NAMESPACE_COUNT);
    if (writer == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
            "Failed on gsXmlCreateStreamWriter (assuming out of memory)");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {
        GSSoapTask * aTask = NULL;

        // make a copy of the request callback and user param
        requestData = (GSBIRequestData*)gsimalloc(sizeof(GSBIRequestData));
        if (requestData == NULL)
        {
            gsXmlFreeWriter(writer);
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBIRequestData");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset(requestData, 0, sizeof(GSBIRequestData));
        requestData->mInstance = theInstance;
        requestData->mUserData = userData;
        requestData->mUserCallback.mUpdateEmailAndNickCallback = callback;

        if (gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_UPDATEMEMBEREMAILANDNICK)) ||
            gsi_is_false(gsXmlWriteOpenTag(writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsbiStartBaseRequest(theInstance, writer)) ||
            gsi_is_false(gsXmlWriteIntElement(writer, GSB_NAMESPACE, "BrigadeID", brigadeId)) ||
            gsi_is_false(gsXmlWriteCloseTag  (writer, GSB_NAMESPACE, GSB_REQUEST_ELEMENT)) ||
            gsi_is_false(gsXmlWriteCloseTag(writer, GSB_NAMESPACE, GSB_UPDATEMEMBEREMAILANDNICK)) ||
            gsi_is_false(gsXmlCloseWriter(writer)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data");		
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }	

        aTask = gsiExecuteSoap(theInstance->mServiceURL, 
            GSB_UPDATEMEMBEREMAILANDNICK_SOAP,
            writer, 
            gsbiServiceUpdateMemberEmailAndNickCallback, 
            (void*)requestData);
        if (aTask == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed on gsiExecuteSoap");
            gsXmlFreeWriter(writer);
            gsifree(requestData);
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }
    return result;
}
