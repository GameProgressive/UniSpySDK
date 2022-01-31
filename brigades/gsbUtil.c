///////////////////////////////////////////////////////////////////////////////
// File:	gsbUtil.c
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited. 

#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttpCommon.h"

#include "brigades.h"
#include "gsbMain.h"
#include "gsbServices.h"
#include "gsbSerialize.h"
#include "gsbUtil.h"

GSResult gsbiStatusCodeToServiceResult(int statusCode)
{
	switch(statusCode)
	{
		case 0: return GSB_SUCCESS(GSResultSection_NULL, GSResultCode_Success);
		case 1: return GSB_ERROR(GSResultSection_Network, GSResultCode_ServerError);
		case 2: return GSB_ERROR(GSResultSection_Network, GSResultCode_RequestTimedOut);
		case 3: return GSB_ERROR(GSResultSection_Network, GSResultCode_InvalidCertificate);
		// This case will not happen when translating an error from the backend.
		// They will be all set before release
		default: return GSB_ERROR(GSResultSection_NULL, GSResultCode_UnknownError);
	}
}

GSResult gsbiSetServiceUrl(GSBInternalInstance *theInstance)
{
	int charsWritten;
	
	GSB_ASSERT_CHECK_PARAM(theInstance, "theInstance is NULL");
	
	if (theInstance->mBaseServiceURL[0] == '\0')
	{
#ifndef UNISPY_FORCE_IP
		charsWritten = snprintf(theInstance->mBaseServiceURL, GSB_SERVICE_MAX_URL_BASE_LEN, GSB_SERVICE_URL_BASE, __GSIACGamename);
#else
        charsWritten = snprintf(theInstance->mBaseServiceURL, GSB_SERVICE_MAX_URL_BASE_LEN, "%s/Brigades/", UNISPY_FORCE_IP);
#endif
		if (charsWritten < 0)
		{
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError, "Could not write to Base URL string");
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_ArgumentNull);
		}
	}

	charsWritten = snprintf(theInstance->mServiceURL, GSB_SERVICE_MAX_URL_LEN, "%s%s%s", GSB_HTTP, theInstance->mBaseServiceURL, GSB_SERVICE_CONTRACT);
	if (charsWritten < 0)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError, "Could not write to URL string");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_ArgumentNull);
	}

	return GS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GSResult gsbiValidateBrigadeSearchFilter(GSBSearchFilterList *filter)
{
    GSResult result = GS_SUCCESS;
    gsi_u32 i = 0;
    gsi_u32 SearchParamFound[GSBSearchFilterKey_Max] = {0};
    if (filter ->mCount <= 0)
    {
        GSB_ASSERT_CHECK_PARAM(filter->mCount > 0, "search filter is empty");
    }
    for (i = 0; i < filter->mCount; i++)
    {
        switch (filter->mFilters[i].mKey)
        {
        case GSBSearchFilterKey_Name :
        case GSBSearchFilterKey_Tag  :
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Unicode)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Tag or Name or Brigade search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }
            // name and the tag are mutually exclusive 
            // so remember if it was found previously
            if (SearchParamFound[GSBSearchFilterKey_Name] || SearchParamFound[GSBSearchFilterKey_Tag])
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Cannot have more than one Search Pattern; Tag or Name ");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            break ; 

        case GSBSearchFilterKey_BrigadeId:
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Uint)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Brigade id search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }
            if (SearchParamFound[GSBSearchFilterKey_BrigadeId])
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Cannot have more than one Brigade Id ");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            break ;

        case GSBSearchFilterKey_BrigadeStatus :
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Uint)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Brigade status search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }

            if (SearchParamFound[GSBSearchFilterKey_BrigadeStatus])
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Cannot have more than one Brigade Status ");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            break ;
        case GSBSearchFilterKey_DateCreated :
            
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Time)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Date search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }

            if (SearchParamFound[GSBSearchFilterKey_DateCreated])
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Cannot have more than one Date Created");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            break ;
        case GSBSearchFilterKey_ExtraParams :
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Unicode)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Extra params search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }
            if (SearchParamFound[GSBSearchFilterKey_ExtraParams] >= GSB_SEARCH_EXTRAPARAMS_MAX)
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Cannot have more than one Date Created");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            break ;

        case GSBSearchFilterKey_MaxResultCount :
            if (filter->mFilters[i].mType != GSBSearchFilterValueType_Uint)
            {
                // either a Name and and a Tag is given at the
                // same time so we need to return back an error code.
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "Maximum result count search filter value has wrong data type!");
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
            }
            if (SearchParamFound[GSBSearchFilterKey_MaxResultCount])
            {
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Cannot have more than one  Maximum Result Count");
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
            }   
            if (filter->mFilters[i].mValue.mValueUint > GSB_SEARCH_MAX_RESULTS)
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                    "The maximum number of results returned cannot be greater than %d", 
                    GSB_SEARCH_MAX_RESULTS);
                return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaNotMet);
            }
            break ;

        default :
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Unexpected Search Criteria %d", filter->mFilters[i].mKey );
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
        }
        SearchParamFound[filter->mFilters[i].mKey]++;
    }
    // Now check if the required parameters exist.
    if gsi_is_false(SearchParamFound[GSBSearchFilterKey_Name] || SearchParamFound[GSBSearchFilterKey_Tag])
    {

        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "Missing required Search Pattern; Tag or Name ");
        return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
    }   
    // Check if have more results than what backend expects

    return result;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GSResult gsbiValidateMemberSearchFilter(GSBSearchFilterList *filter)
{
    GSResult result = GS_SUCCESS;
    gsi_u32 i = 0 ;
    gsi_u32 SearchParamFound[GSBSearchFilterKey_Max] = {0};
    if (filter ->mCount <= 0)
    {
        GSB_ASSERT_CHECK_PARAM(filter->mCount > 0, "search filter is empty");
    }

    for ( i = 0 ; i < filter->mCount; i++)
    {
        switch (filter->mFilters[i].mKey)
        {
            case GSBSearchFilterKey_Name :
            case GSBSearchFilterKey_Tag  :
                if (filter->mFilters[i].mType != GSBSearchFilterValueType_Unicode)
                {
                    // either a Name and and a Tag is given at the
                    // same time so we need to return back an error code.
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                        "Tag or Name or Brigade search filter value has wrong data type!");
                    return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
                }
                // name and the tag are mutually exclusive 
                // so remember if it was found previously
                if (SearchParamFound[GSBSearchFilterKey_Name] || SearchParamFound[GSBSearchFilterKey_Tag])
                {
                    // either a Name and and a Tag is given at the
                    // same time so we need to return back an error code.
                    GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                        "Cannot have more than one Search Pattern; Tag or Name or Team");
                    return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
                }   
                break ; 

            case GSBSearchFilterKey_BrigadeId:
                if (filter->mFilters[i].mType != GSBSearchFilterValueType_Uint)
                {
                    // either a Name and and a Tag is given at the
                    // same time so we need to return back an error code.
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                        "Brigade Id search filter value has wrong data type!");
                    return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
                }
                if (SearchParamFound[GSBSearchFilterKey_BrigadeId])
                {
                    GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                        "Cannot have more than one Brigade Id ");
                    return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
                }   
                break ;

            case GSBSearchFilterKey_ExtraParams :
                if (filter->mFilters[i].mType != GSBSearchFilterValueType_Unicode)
                {
                    // either a Name and and a Tag is given at the
                    // same time so we need to return back an error code.
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                        "Extra params search filter value has wrong data type!");
                    return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
                }
                if (SearchParamFound[GSBSearchFilterKey_ExtraParams] >= GSB_SEARCH_EXTRAPARAMS_MAX)
                {
                    GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                        "Cannot have more than %d Extra Patameters", 
                        GSB_SEARCH_EXTRAPARAMS_MAX);
                    return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
                }   
                break ;

            case GSBSearchFilterKey_MaxResultCount :
                if (filter->mFilters[i].mType != GSBSearchFilterValueType_Uint)
                {
                    // either a Name and and a Tag is given at the
                    // same time so we need to return back an error code.
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,
                        "Maximum result count search filter value has wrong data type!");
                    return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaWrongDataType);
                }
                if (SearchParamFound[GSBSearchFilterKey_MaxResultCount])
                {
 
                    GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                        "Cannot have more than one Maximum Result Count");
                    return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
                }  

                if (filter->mFilters[i].mValue.mValueUint > GSB_SEARCH_MAX_RESULTS)
                {
                    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, 
                        "The maximum number of results returned cannot be greater than %d", 
                        GSB_SEARCH_MAX_RESULTS);
                    return GSB_ERROR(GSResultSection_SdkSpecifc, GSBResultCode_SearchCriteriaNotMet);
                }
                break ;

            default :
                GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
                    "Unexpected Search Criteria %d", filter->mFilters[i].mKey );
                return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
        }
        SearchParamFound[filter->mFilters[i].mKey]++;
    }
    // Now check if the required parameters exist.
    if gsi_is_false(SearchParamFound[GSBSearchFilterKey_Name] || SearchParamFound[GSBSearchFilterKey_Tag])
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "Missing required Search Pattern; Tag or Name or Brigade name ");
        return GSB_ERROR(GSResultSection_Memory, GSBResultCode_SearchCriteriaNotMet);
    }   
    return result;
}

GSResult gsbiGetHeaderValue(const char *headers, const char *headerName, gsi_u32 *headerValue)
{
	const char * header;
	int rcode;

	// find this header in the list of headers
	header = strstr(headers, headerName);
	if(header)
	{
		// skip the header name
		header += strlen(headerName);

		// scan in the result
		rcode = sscanf(header, " %d", headerValue);
		if(rcode == 1)
		{
			return GS_SUCCESS;
		}
	}

	// failed to find what we needed
	GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Unable to read header: %s", headerName);
	return GSB_ERROR(GSResultSection_State, GSResultCode_HttpError);
}

GHTTPBool gsbiUploadCompletedCallback(GHTTPRequest httpRequest, GHTTPResult httpResult, char * httpBuffer, GHTTPByteCount httpBufferLen, char * responseHeaders, void * param)
{
	GSBIUploadData *uploadData;
	GSResult result;
	const char *headers;
	gsi_u32 uploadResult;

	// should check this value
	uploadData = (GSBIUploadData *)param;

    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_Debug, "%s Upload Logo Completed ",httpResult == GHTTPSuccess?"SUCCESS:":"FAILURE:" );

	if (httpResult != GHTTPSuccess)
	{
		GSB_DEBUG_LOG(GSIDebugType_Network, GSIDebugLevel_WarmError, 
            "Http result not successful, %d", httpResult);
		result = GSB_ERROR(GSResultSection_Network, GSResultCode_HttpError);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}

	headers = ghttpGetHeaders(httpRequest);
	if (headers == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Http Headers NULL");
		result = GSB_ERROR(GSResultSection_State, GSResultCode_HttpError);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}
	
	result = gsbiGetHeaderValue(headers, GSB_UPLOAD_RESULT_HEADER, &uploadResult);
	if (GS_FAILED(result))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to get upload result");
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}
	else if (uploadResult != GSBIUploadResult_SUCCESS)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
            "Upload result failed: %d", uploadResult);
		result = GSB_ERROR(GSResultSection_State, GSResultCode_HttpError);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}

	result = gsbiGetHeaderValue(headers, GSB_UPLOAD_FILE_ID_HEADER, &uploadData->mFileId);
	if (GS_FAILED(result))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to obtain upload fileid");
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}
	
	if (GS_FAILED(gsbiServiceSaveBrigadeLogoFileId(uploadData->mInstance, uploadData->mBrigadeId, uploadData->mFileId, uploadData->mCallback, uploadData->mUserData)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to obtain upload fileid");
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		return GHTTPTrue;
	}	

	GSI_UNUSED(httpBuffer);
	GSI_UNUSED(httpBufferLen);
	GSI_UNUSED(responseHeaders);
	return GHTTPTrue;
}

void gsbiUploadThreadCleanup(GSBIUploadData *uploadData, GHTTPPost aPost)
{
	if (aPost)
		ghttpFreePost(aPost);
	if (uploadData)
	{
		if (uploadData->mLogoFileName)
			gsifree(uploadData->mLogoFileName);

		gsifree(uploadData);
	}	
}

GS_THREAD_RETURN_TYPE gsbiUploadThreadFunc(void *arg)
{
	GSResult result;
	GHTTPRequest aRequest;
	GHTTPPost aPost;
	GSBIUploadData *uploadData = NULL;
	gsi_char *uploadFileName;
	gsi_char *uploadUrl;
	
	uploadData = (GSBIUploadData *)arg;
	if (uploadData == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, 
            "Failed to get uploadData parameter from arg");
		GS_THREAD_RETURN_NEGATIVE;
	}

	aPost = ghttpNewPost();
	if (aPost == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "Failed to allocate new post");
		result = GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		gsbiUploadThreadCleanup(uploadData, NULL);
		GS_THREAD_RETURN_NEGATIVE;
	}
	
	// Find the actual file name minus the path
	uploadFileName = _tcsrchr(uploadData->mLogoFileName, '/');
	uploadFileName++;
	if (!ghttpPostAddFileFromDisk(aPost, uploadFileName, uploadData->mLogoFileName, uploadFileName, NULL))
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "Failed to add post from disk");
		result = GSB_ERROR(GSResultSection_State, GSResultCode_OperationCancelled);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		gsbiUploadThreadCleanup(uploadData, aPost);
		GS_THREAD_RETURN_NEGATIVE;
	}

	// setup url to use sake backend for now until we implement MTOM attachments
	uploadUrl = (gsi_char *)gsimalloc(sizeof(gsi_char) * GSB_SERVICE_MAX_URL_LEN);
	if (uploadUrl == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Failed to allocate memory for uploadUrl");
		result = GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		gsbiUploadThreadCleanup(uploadData, aPost);
		GS_THREAD_RETURN_NEGATIVE;
	}

#ifdef UNISPY_FORCE_IP
    _tsnprintf(uploadUrl,
        sizeof(gsi_char) * GSB_SERVICE_MAX_URL_LEN,
        _T("%s%s/SakeFileServer/upload.aspx?gameid=%d&pid=%d"),
        GSI_HTTP_PROTOCOL_URL, UNISPY_FORCE_IP, GSB_UPLOAD_GID,
        uploadData->mInstance->mCertificate.mProfileId);
	#else
	_tsnprintf(uploadUrl, 
		       sizeof(gsi_char) * GSB_SERVICE_MAX_URL_LEN, 
			   _T("%sbrigades.sake.%s/SakeFileServer/upload.aspx?gameid=%d&pid=%d"),
        GSI_HTTP_PROTOCOL_URL, GSI_DOMAIN_NAME, GSB_UPLOAD_GID,
			   uploadData->mInstance->mCertificate.mProfileId);
#endif

	aRequest = ghttpPostEx(uploadUrl, NULL, aPost, GHTTPFalse, GHTTPTrue, NULL, gsbiUploadCompletedCallback, uploadData);
    gsifree(uploadUrl);
	if (IS_GHTTP_ERROR(aRequest))
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, 
            "Failed to call ghttpSaveEx, request error: %d", aRequest);
		result = GSB_ERROR(GSResultSection_Network, GSResultCode_HttpError);
		uploadData->mCallback(result, NULL, uploadData->mUserData);
		gsbiUploadThreadCleanup(uploadData, aPost);
		GS_THREAD_RETURN_NEGATIVE;
	}

	// Because we went through the actual POST, it's been automatically freed by the above call.
	gsbiUploadThreadCleanup(uploadData, NULL);
	GS_THREAD_RETURN;
}


GSResult gsbiStartUploadRequest(GSBInternalInstance *theInstance, 
								const gsi_char *logoFileName, 
								gsi_u32 brigadeId, 
								GSBUploadLogoCompleteCallback callback, 
								void *userData)
{
	int rcode;
	GSIThreadID threadId;
	GSBIUploadData *uploadData = NULL;

	uploadData = (GSBIUploadData *)gsimalloc(sizeof(GSBIUploadData));
    if (!uploadData)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Could not duplicate string: uploadData->logoFileName");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
	uploadData->mBrigadeId = brigadeId;
	uploadData->mCallback = callback;
#ifdef GSI_UNICODE
	uploadData->mLogoFileName = goawstrdup(logoFileName);
#else
	uploadData->mLogoFileName = goastrdup(logoFileName);
#endif

	if (uploadData->mLogoFileName == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Could not duplicate string: uploadData->logoFileName");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	uploadData->mInstance = theInstance;
	uploadData->mUserData = userData;

	rcode = gsiStartThread(gsbiUploadThreadFunc, 0, (void *)uploadData, &threadId);

	if (rcode == -1) 
	{
		gsifree(uploadData->mLogoFileName);
		gsifree(uploadData);
		return GSB_ERROR(GSResultSection_State, GSResultCode_ThreadCreateFailed);
	}

	return GS_SUCCESS;
}

GHTTPBool gsbiDownloadCompletedCallback(GHTTPRequest httpRequest, GHTTPResult httpResult, char * httpBuffer, GHTTPByteCount httpBufferLen, char * responseHeaders, void * param)
{
	GSResult result = GS_SUCCESS;
	GSBIDownloadData *downloadData = (GSBIDownloadData *)param;
	
	if (httpResult != GHTTPSuccess)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Http result is: %d", httpResult);
		result = GSB_ERROR(GSResultSection_Network, GSResultCode_HttpError);	
	}
	
	downloadData->mCallback(result, NULL, downloadData->mUserData);

	GSI_UNUSED(httpRequest);
	GSI_UNUSED(httpBuffer);
	GSI_UNUSED(httpBufferLen);
	GSI_UNUSED(responseHeaders);

	return GHTTPTrue;
}

void gsbiDownloadThreadCleanup(GSBIDownloadData *dloadData, gsi_char *dloadUrl)
{
	if (dloadData)
	{
		gsifree(dloadData->mBrigadeLogo->mPath);
		gsifree(dloadData->mBrigadeLogo->mUrl);
		gsifree(dloadData->mBrigadeLogo);
		gsifree(dloadData->mLogoFileName);
		gsifree(dloadData);
	}
#ifndef GSI_UNICODE
	gsifree(dloadUrl);
#endif
}

GS_THREAD_RETURN_TYPE gsbiDownloadThreadFunc(void *arg)
{
	GSResult result;
	GHTTPRequest aRequest;
	GSBIDownloadData *dLoadData = NULL;
	gsi_char *dloadUrl;

	dLoadData = (GSBIDownloadData *)arg;
	if (dLoadData == NULL)
	{
		// This should never happen!
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Null dLoadData");
		GS_THREAD_RETURN_NEGATIVE;
	}
	
#ifndef GSI_UNICODE
	dloadUrl = UCS2ToUTF8StringAlloc(dLoadData->mBrigadeLogo->mUrl);
#else
	dloadUrl = dLoadData->mBrigadeLogo->mUrl;
#endif

	if (dloadUrl == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Null dLoadUrl");
		result = GSB_ERROR(GSResultSection_Network, GSResultCode_HttpError);
		dLoadData->mCallback(result, NULL, dLoadData->mUserData);
		gsbiDownloadThreadCleanup(dLoadData, dloadUrl);
		GS_THREAD_RETURN_NEGATIVE;
	}

	aRequest = ghttpSaveEx(dloadUrl, dLoadData->mLogoFileName, NULL,  NULL, GHTTPFalse, GHTTPTrue, NULL, gsbiDownloadCompletedCallback, (void *)dLoadData);	
	if (IS_GHTTP_ERROR(aRequest))
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, 
            "Failed to call ghttpSaveEx, request error: %d", aRequest);
		result = GSB_ERROR(GSResultSection_Network, GSResultCode_HttpError);
		dLoadData->mCallback(result, NULL, dLoadData->mUserData);
		gsbiDownloadThreadCleanup(dLoadData, dloadUrl);
		GS_THREAD_RETURN_NEGATIVE;
	}
	
	gsbiDownloadThreadCleanup(dLoadData, dloadUrl);
	GS_THREAD_RETURN;
}

GSResult gsbiStartDownloadRequest(GSBInternalInstance *theInstance, 
								  const gsi_char *logoFileName, 
								  GSBBrigadeLogo *logo, 
								  GSBDownloadLogoCompleteCallback callback, 
								  void *userData )
{
	GSResult result;
	int rcode;
	GSIThreadID threadId;
	GSBIDownloadData *dLoadData = NULL;

	dLoadData = (GSBIDownloadData *)gsimalloc(sizeof(GSBIDownloadData));
    if (dLoadData == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Could not allocate memory for dLoadData");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

	dLoadData->mCallback = callback;
	dLoadData->mInstance = theInstance;
	dLoadData->mUserData = userData;
	
	result = gsbiCloneBrigadeLogo(&dLoadData->mBrigadeLogo, logo);
	if (GS_FAILED(result) || dLoadData->mBrigadeLogo == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Could not allocate");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

#ifdef GSI_UNICODE
	dLoadData->mLogoFileName = goawstrdup(logoFileName);
#else
	dLoadData->mLogoFileName = goastrdup(logoFileName);
#endif

	if (dLoadData->mLogoFileName == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Could not duplicate string: dLoadData->logoFileName");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	rcode = gsiStartThread(gsbiDownloadThreadFunc, 0, (void *)dLoadData, &threadId);

	if (rcode == -1) 
	{
		// TODO: Add freeing code here
		gsifree(dLoadData);
		return GSB_ERROR(GSResultSection_State, GSResultCode_ThreadCreateFailed);
	}

	return GS_SUCCESS;
}

GSResult gsbiCloneBrigadeLogoList( GSBBrigadeLogoList *destLogoList, GSBBrigadeLogoList *srcLogoList)
{
    if (destLogoList == NULL || srcLogoList == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "NULL pointer passed");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_InvalidParameters);

    }
    // The caller allocates the memory for the dstLogoList
    destLogoList->mCount = srcLogoList->mCount;
    if (destLogoList->mCount > 0)
    {
        gsi_u32 i = 0;
        destLogoList->mLogos = (GSBBrigadeLogo *) gsimalloc (sizeof (GSBBrigadeLogo) * srcLogoList->mCount);
        if (destLogoList->mLogos == NULL)
        {
            GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,"Failed to allocate GSBBrigadeLogo");
            return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        memset (destLogoList->mLogos, 0 , (sizeof (GSBBrigadeLogo) * srcLogoList->mCount));
        for (i = 0 ; i<srcLogoList->mCount; i++)
        {
            destLogoList->mLogos[i].mDefaultLogo = srcLogoList->mLogos[i].mDefaultLogo;
            destLogoList->mLogos[i].mFileId = srcLogoList->mLogos[i].mFileId;
            destLogoList->mLogos[i].mSizeId = srcLogoList->mLogos[i].mSizeId;
            if (srcLogoList->mLogos[i].mPath != NULL)
            {
                destLogoList->mLogos[i].mPath = goawstrdup(srcLogoList->mLogos[i].mPath);
            }
            if (srcLogoList->mLogos[i].mUrl != NULL)
            {
                destLogoList->mLogos[i].mUrl = goawstrdup(srcLogoList->mLogos[i].mUrl);
            }
        }
    }
    else
    {
        destLogoList->mLogos = NULL;
    }
    return GS_SUCCESS;
}

void gsbiFreeBrigadeLogoList(GSBBrigadeLogoList *logoList)
{
	if (logoList)
	{
        if (logoList->mLogos)
        {
		    gsi_u32 i;
		    for (i = 0; i < logoList->mCount; i++)
		    {
			    if(logoList->mLogos[i].mPath) 
                    gsifree(logoList->mLogos[i].mPath);
                if (logoList->mLogos[i].mUrl)			    
                    gsifree(logoList->mLogos[i].mUrl);
		    }
		    gsifree(logoList->mLogos);
        }
        memset(logoList, 0, sizeof(GSBBrigadeLogoList));
	}	
}

GSResult gsbiCloneBrigadeLogo(GSBBrigadeLogo **destLogo, GSBBrigadeLogo *srcLogo)
{
	GSBBrigadeLogo *tempLogo;
	if (destLogo == NULL || srcLogo == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Null destLogo or srcLogo");
		return GSB_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	tempLogo = (GSBBrigadeLogo *)gsimalloc(sizeof(GSBBrigadeLogo));
	if (tempLogo == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not allocate");
		return GSB_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	memcpy(tempLogo, srcLogo, sizeof(GSBBrigadeLogo));
	tempLogo->mPath = goawstrdup(srcLogo->mPath);
	tempLogo->mUrl = goawstrdup(srcLogo->mUrl);
	
	*destLogo = tempLogo;
	return GS_SUCCESS;
}

GSResult gsbiCloneBrigadeMemberContents(GSBBrigadeMember *destMember, GSBBrigadeMember *srcMember)
{
	// destMember and srcMember have been screened already so no need to recheck
	// when calling this internal function
	destMember->mDescription = goawstrdup(srcMember->mDescription);
	if (destMember->mDescription == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to duplicate string(s)for mDescription");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	destMember->mTitle = goawstrdup(srcMember->mTitle);
	if (destMember->mTitle == NULL)
	{
		gsifree(destMember->mDescription);

		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Failed to duplicate string(s)for mTitle");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	destMember->mBrigadeId = srcMember->mBrigadeId;
	destMember->mBrigadeMemberId = srcMember->mBrigadeMemberId;
	destMember->mDateAdded = srcMember->mDateAdded;
	destMember->mEmailOptIn = srcMember->mEmailOptIn;
	destMember->mProfileId = srcMember->mProfileId;
	destMember->mRoleId = srcMember->mRoleId;
	destMember->mStatus = srcMember->mStatus;

	return GS_SUCCESS;
}


void gsbiFreeEntitlementList(GSBEntitlementList *entitlementList)
{
    if (entitlementList)
    {
        if (entitlementList->mEntitlements)
        {
            gsi_u32 i;
            for (i = 0; i < entitlementList->mCount; i++)
            {
                if(entitlementList->mEntitlements[i].mEntitlementName) 
                    gsifree(entitlementList->mEntitlements[i].mEntitlementName);
            }
            gsifree(entitlementList->mEntitlements);
        }
        gsifree(entitlementList);
    }	
}

GSResult gsbiCloneEntitlement(GSBEntitlement *destEntitlement, 
                              GSBEntitlement *srcEntitlement)
{
    if (destEntitlement == NULL || srcEntitlement == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Null entitlement destination or source");
        return GSB_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    destEntitlement->mEntitlementName = goawstrdup(srcEntitlement->mEntitlementName);
	if(destEntitlement->mEntitlementName == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failure to strdup entitlement name");
		return GSB_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

    destEntitlement->mEntitlementId = srcEntitlement->mEntitlementId;

    return GS_SUCCESS;
}

void gsbiFreeBrigadeHistoryList(GSBBrigadeHistoryList *historyList)
{
    if (historyList)
    {
        if (historyList->mBrigadeHistory)
        {
            gsi_u32 i;
            for (i = 0; i < historyList->mCount; i++)
            {
                gsbiFreeBrigadeHistoryEntryContents(&historyList->mBrigadeHistory[i]);
            }
            gsifree(historyList->mBrigadeHistory);
        }
        gsifree(historyList);
    }	
}
void gsbiFreeBrigadeHistoryEntryContents(GSBBrigadeHistoryEntry *historyEntry)
{
    if (historyEntry)
    {
        if (historyEntry->mAccessLevel)
            gsifree(historyEntry->mAccessLevel);
        if (historyEntry->mHistoryAction)
            gsifree(historyEntry->mHistoryAction);
        if (historyEntry->mNotes)
            gsifree(historyEntry->mNotes);
        if (historyEntry->mSourceProfileNickname)
            gsifree(historyEntry->mSourceProfileNickname);
        if (historyEntry->mTargetProfileNickname)
            gsifree(historyEntry->mTargetProfileNickname);

        // reset the memory
        memset(historyEntry, 0, sizeof(GSBBrigadeHistoryEntry));
    }
    
}

void gsbiFreeBrigadeContents(GSBBrigade *brigade)
{
    if (brigade)
    {
        if(brigade->mMessageOfTheDay) 
            gsifree(brigade->mMessageOfTheDay);
        if(brigade->mName) 
            gsifree(brigade->mName);
        if(brigade->mTag) 
            gsifree(brigade->mTag);
        if (brigade->mUrl) 
            gsifree(brigade->mUrl);
        gsbiFreeBrigadeLogoList(&brigade->mLogoList);
    }
}

void gsbiFreeBrigadeList(GSBBrigadeList *brigadeList)
{
    if (brigadeList)
    {
        if (brigadeList->mBrigades)
        {
            gsi_u32 i;
            for (i = 0; i < brigadeList->mCount; i++)
            {
                gsbiFreeBrigadeContents(&brigadeList->mBrigades[i]);
            }
            gsifree(brigadeList->mBrigades);
        }
        gsifree(brigadeList);
    }
}
void gsbiFreeFilterList(GSBSearchFilterList *filterList)
{
    if (filterList)
    {
        if (filterList->mFilters)
        {
            gsi_u32 i;
            for (i = 0; i < filterList->mCount; i++)
            {
                
                if(filterList->mFilters[i].mType == GSBSearchFilterValueType_Unicode)
                {
                        gsifree(filterList->mFilters[i].mValue.mValueStr);            
                }
                
            }
            gsifree(filterList->mFilters);
        }
        gsifree(filterList);
    }
}
void gsbiFreeRoleList(GSBRoleList *roleList)
{
    if (roleList)
    {
        if (roleList->mRoles)
        {
            gsi_u32 i;
            for (i = 0; i < roleList->mCount; i++)
            {
                gsifree(roleList->mRoles[i].mRoleName);
            }
            gsifree(roleList->mRoles);
        }
        gsifree(roleList);
    }
}

void gsbiFreeRole(GSBRole *role)
{
    if (role)
    {
        if (role->mRoleName) 
        {
            gsifree(role->mRoleName);
        }
        gsifree(role);
    }
}
void gsbiFreeRoleIdList(GSBRoleIdList *roleIdList)
{
    if (roleIdList)
    {
        if (roleIdList->mRoleIds)
        {
            gsifree(roleIdList->mRoleIds);
        }
        gsifree(roleIdList);
    }
}

void gsbiFreeEntitlementIdList(GSBEntitlementIdList *entitlementIdList)
{
    if (entitlementIdList)
    {
        if (entitlementIdList->mEntitlementIds)
        {
            gsifree(entitlementIdList->mEntitlementIds);
        }
        gsifree(entitlementIdList);
    }	
}


void gsbiFreePendingActionsList(GSBBrigadePendingActionsList *actionList)
{
    if (actionList)
    {
        if (actionList->mPendingActions)
        {
            gsi_u32 i;
            for (i = 0; i < actionList->mCount; i++) 
            {
                if(actionList->mPendingActions[i].mBrigadeName) 
                    gsifree(actionList->mPendingActions[i].mBrigadeName);
            }
            gsifree(actionList->mPendingActions);
        }
        gsifree(actionList);
    }
}

void gsbiFreeBrigadeMemberContents(GSBBrigadeMember *member)
{
    if (member)
    {
        if (member->mDescription)
        {
            gsifree(member->mDescription);
        }
        if (member->mTitle)
        {
            gsifree(member->mTitle);
        }
        memset(member,0, sizeof(GSBBrigadeMember));
    }
}

void gsbiFreeBrigadeMember(GSBBrigadeMember *member)
{
    if (member)
    {
        gsbiFreeBrigadeMemberContents(member);
        gsifree(member);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsbiFreeBrigadeMemberList(GSBBrigadeMemberList *memberList)
{
    if (memberList)
    {
        if (memberList->mBrigadeMembers)
        {
            gsi_u32 i;
            for (i = 0; i < memberList->mCount; i++) 
            {
                gsbiFreeBrigadeMemberContents(&memberList->mBrigadeMembers[i]);
            }
            gsifree(memberList->mBrigadeMembers);
        }
        gsifree(memberList);
    }
}