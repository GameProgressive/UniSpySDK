///////////////////////////////////////////////////////////////////////////////
// File:	gsbSerialize.c
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited. 


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// includes
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttpCommon.h"
#include "brigades.h"
#include "gsbMain.h"
#include "gsbServices.h"
#include "gsbSerialize.h"
#include "gsbUtil.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseResponseResult(GSXmlStreamReader theResponseXml, 
								 GSResultSet **resultSet, 
								 const char *theRequestName)
{
	GSResult result, result2;
	int statusCode;
    char resultTag[128];

    sprintf(resultTag, "%s%s", theRequestName, GSB_RESULT); //GSB_RESPONSE);

    if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
        gsi_is_false(gsXmlMoveToNext(theResponseXml, resultTag)) ||
        gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_SERVICE_RESULT_CODE, &statusCode)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", resultTag);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

	result = gsbiStatusCodeToServiceResult(statusCode);
	if (GS_FAILED(result))
	{
		*resultSet = NULL;
		return result;
	}
	
	result2 = gsbiParseResponseCodes(theResponseXml, resultSet);
	if (GS_FAILED(result2))
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Failed to parse through result set");
		return result2;
	}
    
    return result;

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseResponseCodes(GSXmlStreamReader theResponseXml,
								GSResultSet **resultSet)
{
	GSResult result;
	int i, errorMessageLen;
	const char *errorMessage;
	GSResultSet *outResultSet;
	int code;
	result = gsResultCodesCreateResultSet(&outResultSet);
	if (GS_FAILED(result))
	{
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not create a new result set");
		return result;
	}
	
	// we should keep track of the pointer before setting any of the data outResultSet points to
	// that way we can clean up properly
	*resultSet = outResultSet;
	outResultSet->mNumResults = 0;

    if( gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_RESULT_SECTION)))
    {
        // No additional status is sent back
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
            "Missing results.Could not parse xml tag: %s", GSB_RESULT_SECTION);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
    else
    {
	    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, &outResultSet->mNumResults)))
	    {
		    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                "Could not parse xml tags: %s\n", GSB_COUNT_ELEMENT);
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	    }

	    if (outResultSet->mNumResults == 0)
	    {
		    if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		    {
			    GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                    "Could not move to parent xml tag of: %s" GSB_COUNT_ELEMENT);
			    return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		    }

		    // outResultSet points to clean data!
		    // just return success
            return GS_SUCCESS;
	    }
    }
	outResultSet->mResults = (GSResult *)gsimalloc(sizeof(GSResult) * outResultSet->mNumResults);
	if (!outResultSet->mResults)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
            "Failed to allocate memory for resultSet->mNumResults");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(outResultSet->mResults, 0 , (sizeof(GSResult) * outResultSet->mNumResults));
    for (i=0; i < outResultSet->mNumResults; i++)
    {
        if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_RESULT_CODE_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                    "Could not parse tag: %s", GSB_RESULT_CODE_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_RESULT_CODE_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                    "Could not parse tag: %s", GSB_RESULT_CODE_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		
		if (gsi_is_false(gsXmlReadValueAsInt(theResponseXml, GSB_RESULT_CODE_ELEMENT, &code)))
        {
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml single result code");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }
		outResultSet->mResults[i] = GSB_ERROR(GSResultSection_SdkSpecifc, code);
    }

	// move back up to status, read in the text message, and move up from status to the actual response
    if (gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_RESULT_MSG, &errorMessage, &errorMessageLen)) ||
		gsi_is_false(gsXmlMoveToParent(theResponseXml)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
            "Could not parse xml tag: %s", GSB_RESULT_MSG);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

	if (errorMessageLen > 0)
	{
		outResultSet->mErrorMessage = UTF8ToUCS2StringAllocLen(errorMessage, errorMessageLen);
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Base Requests start here - as each request takes a similar format
gsi_bool gsbiStartBaseRequest(GSBInternalInstance         *theInstance,
                              GSXmlStreamWriter   theWriter)
{
    if (//gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_GAMEID_ELEMENT, theInstance->mGameId)) ||
		gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, "LoginCertificate")) ||
        gsi_is_false(wsLoginCertWriteXML(&theInstance->mCertificate, GSB_NAMESPACE, theWriter)) ||
        gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, "LoginCertificate")) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(theWriter, GSB_NAMESPACE, "Proof", (const gsi_u8*)theInstance->mPrivateData.mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_GAMEID_ELEMENT, theInstance->mGameId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data.");
        return gsi_false;
    }
    return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeBrigadeLogoList(GSXmlStreamWriter theWriter, GSBBrigadeLogoList *brigadeLogoList)
{
	gsi_u32 i;
	if (brigadeLogoList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_Notice, "brigadeLogoList is empty.");
		return gsi_true;
	}

	if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_LOGOS_ELEMENT)) || 
		gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_COUNT_ELEMENT, brigadeLogoList->mCount)) ||
		gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_BRIGADELOGOENTRIES_ELEMENT)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not serialize tags: %s, %s, %s", 
			GSB_LOGOS_ELEMENT, GSB_COUNT_ELEMENT, GSB_BRIGADELOGOENTRIES_ELEMENT);
		return gsi_false;
	}

	for (i = 0; i < brigadeLogoList->mCount; i++)
	{
		GSBBrigadeLogo *currentBrigadeLogo = &brigadeLogoList->mLogos[i];
		if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_BRIGADELOGO_ELEMENT)) || 
			gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_FILEID_ELEMENT, currentBrigadeLogo->mFileId)) ||
			gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_SIZEID_ELEMENT, currentBrigadeLogo->mSizeId)) ||
			gsi_is_false(gsXmlWriteStringElement(theWriter, GSB_NAMESPACE, GSB_DEFAULTLOGO_ELEMENT, 
				currentBrigadeLogo->mDefaultLogo ? GSB_TRUE_TEXT_ELEMENT : GSB_FALSE_TEXT_ELEMENT)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                "Could not serialize: %s", GSB_BRIGADELOGO_ELEMENT);
			return gsi_false;
		}

		if (currentBrigadeLogo->mPath)
		{
			if (gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, GSB_PATH_ELEMENT, currentBrigadeLogo->mPath)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                    "Could not serialize: %s", GSB_PATH_ELEMENT);
				return gsi_false;
			}
		}
		
		if (currentBrigadeLogo->mUrl)
		{
			if (gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, GSB_URL_ELEMENT, currentBrigadeLogo->mUrl)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                    "Could not serialize: %s", GSB_URL_ELEMENT);
				return gsi_false;
			}
		}
		
		
		if (gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, GSB_BRIGADELOGO_ELEMENT)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                "Could not serialize: %s", GSB_BRIGADELOGO_ELEMENT);
			return gsi_false;
		}
			
	}

	if (gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, GSB_BRIGADELOGOENTRIES_ELEMENT)) ||
		gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, GSB_LOGOS_ELEMENT)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
            "Could not serialize: %s, %s", GSB_BRIGADELOGOENTRIES_ELEMENT, GSB_LOGOS_ELEMENT);
		return gsi_false;
	}
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeEntitlementList(GSXmlStreamWriter theWriter, GSBEntitlementList *entitlementList)
{
	gsi_u32 i;
	if (entitlementList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_Notice, "entitlementList is empty.");
		return gsi_true;
	}

	if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENTLIST_ELEMENT)) || 
		gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_COUNT_ELEMENT, entitlementList->mCount)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not serialize tags: %s, %s, %s", 
			GSB_ENTITLEMENTLIST_ELEMENT, GSB_COUNT_ELEMENT, GSB_ENTITLEMENTENTRIES_ELEMENT);
		return gsi_false;
	}

	for (i = 0; i < entitlementList->mCount; i++)
	{
		GSBEntitlement *currentEntitlement = &entitlementList->mEntitlements[i];
		if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENT_ELEMENT)) || 
			gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENTID_ELEMENT, currentEntitlement->mEntitlementId)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENTNAME_ELEMENT, currentEntitlement->mEntitlementName )) ||
			gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENT_ELEMENT)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, 
                "Could not serialize: %s", GSB_ENTITLEMENT_ELEMENT);
			return gsi_false;
		}
	}

	if (gsi_is_false(gsXmlWriteCloseTag(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENTLIST_ELEMENT)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not serialize: %s, %s", 
            GSB_ENTITLEMENTENTRIES_ELEMENT, GSB_ENTITLEMENTLIST_ELEMENT);
		return gsi_false;
	}
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeEntitlementIDs(GSXmlStreamWriter theWriter, GSBEntitlementIdList *entitlementIdList)
{
	gsi_u32 i;

	if (entitlementIdList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_Notice, "entitlementIdList is empty.");
		return gsi_true;
	}

	for (i = 0; i < entitlementIdList->mCount; i++)
	{
		if (gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_ENTITLEMENTIDS_ELEMENT, 
												entitlementIdList->mEntitlementIds[i])))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not serialize: %s", 
                GSB_ENTITLEMENTIDS_ELEMENT);
			return gsi_false;
		}
	}

	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeBrigade(GSBInternalInstance       *theInstance,
                              GSXmlStreamWriter theWriter,
                              GSBBrigade        *theBrigade)
{
	if (theBrigade == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not serialize Brigade structure.");
		return gsi_false;
	}

    if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_BRIGADE_ELEMENT)) || 
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_GAMEID_ELEMENT, theInstance->mGameId)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, GSB_BRIGADEID_ELEMENT, theBrigade->mBrigadeId)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, GSB_NAME_ELEMENT, theBrigade->mName)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "Tag", theBrigade->mTag)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "Url", theBrigade->mUrl)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "CreatorProfileID", theBrigade->mCreatorProfileId)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "LeaderProfileID", theBrigade->mLeaderProfileId)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "RecruitingType", theBrigade->mRecruitingType)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "MotD", theBrigade->mMessageOfTheDay)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "Disbanded", theBrigade->mDisbanded)) ||
        gsi_is_false(gsXmlWriteDateTimeElement(theWriter, GSB_NAMESPACE, "DisbandDate", theBrigade->mDisbandDate)) ||
		gsi_is_false(gsbiSerializeBrigadeLogoList(theWriter, &theBrigade->mLogoList)) ||
        gsi_is_false(gsXmlWriteCloseTag  (theWriter, GSB_NAMESPACE, GSB_BRIGADE_ELEMENT)))
    {               
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not serialize Brigade structure.");
        return gsi_false;
    }	
    return gsi_true;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeBrigadeMember(GSXmlStreamWriter theWriter,
                                    GSBBrigadeMember  *theMember)
{
    if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_BRIGADEMEMBER_ELEMENT)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "BrigadeMemberID", theMember->mBrigadeMemberId)) || 
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "BrigadeID", theMember->mBrigadeId)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "ProfileID", theMember->mProfileId)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "Description", theMember->mDescription)) ||
        gsi_is_false(gsXmlWriteDateTimeElement(theWriter, GSB_NAMESPACE, "DateAdded", theMember->mDateAdded)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "Status", theMember->mStatus)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "Title", theMember->mTitle)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "RoleID", theMember->mRoleId)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "EmailOptIn", theMember->mEmailOptIn)) ||
        gsi_is_false(gsXmlWriteCloseTag  (theWriter, GSB_NAMESPACE, GSB_BRIGADEMEMBER_ELEMENT)))
    {               
        // Should only fail to write if the buffer is full
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data.");
        return gsi_false;
    }	
    return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool gsbiSerializeRole(GSXmlStreamWriter    theWriter,
                           GSBRole              *theRole)
{
    if (gsi_is_false(gsXmlWriteOpenTag(theWriter, GSB_NAMESPACE, GSB_ROLE_ELEMENT)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "BrigadeID", theRole->mBrigadeId)) || 
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "RoleID", theRole->mRoleId)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(theWriter, GSB_NAMESPACE, "RoleName", theRole->mRoleName)) ||
        gsi_is_false(gsXmlWriteIntElement(theWriter, GSB_NAMESPACE, "IsDefault", theRole->mIsDefault)) ||
        gsi_is_false(gsXmlWriteCloseTag  (theWriter, GSB_NAMESPACE, GSB_ROLE_ELEMENT)))
    {               
        // Should only fail to write if the buffer is full
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not write request data.");
        return gsi_false;
    }	
    return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseBrigadeLogoList(GSXmlStreamReader theResponseXml,
								  GSBBrigadeLogoList * logoList)
{
	gsi_u32 i;
	gsi_i32 tempLen;
	const char *tempStr;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_LOGOS_ELEMENT)))
	{
		logoList->mCount = 0;
		logoList->mLogos = NULL;
		return GS_SUCCESS;
	}
	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&logoList->mCount)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tags: %s, %s", 
            GSB_LOGOS_ELEMENT, GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (logoList->mCount == 0)
	{
		logoList->mLogos = NULL;
        gsXmlMoveToParent(theResponseXml);
		return GS_SUCCESS;
	}

	logoList->mLogos = (GSBBrigadeLogo *)gsimalloc(sizeof(GSBBrigadeLogo) * logoList->mCount);
	if (logoList->mLogos == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate logoList->mLogos");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	
    memset(logoList->mLogos, 0 , sizeof(GSBBrigadeLogo) * logoList->mCount);
	for (i = 0; i < logoList->mCount; i++)
	{
		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADELOGOENTRIES_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", 
                    GSB_BRIGADELOGO_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_BRIGADELOGOENTRIES_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to: %s", 
                    GSB_BRIGADELOGOENTRIES_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

		if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_FILEID_ELEMENT, (int *)&logoList->mLogos[i].mFileId)) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_SIZEID_ELEMENT, (int *)&logoList->mLogos[i].mSizeId)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tags: %s, %s", 
                GSB_FILEID_ELEMENT, GSB_SIZEID_ELEMENT);
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_DEFAULTLOGO_ELEMENT, &tempStr, &tempLen)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_DEFAULTLOGO_ELEMENT);
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		logoList->mLogos[i].mDefaultLogo = strncmp(tempStr, GSB_TRUE_TEXT_ELEMENT, tempLen) == 0 ? gsi_true : gsi_false;
		
		if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_PATH_ELEMENT, &tempStr, &tempLen)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_PATH_ELEMENT);
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		if (tempLen > 0)
		{
			logoList->mLogos[i].mPath = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
		}

		if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_URL_ELEMENT, &tempStr, &tempLen)))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_URL_ELEMENT);
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		if (tempLen > 0)
		{
			logoList->mLogos[i].mUrl = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
		}

	}
    gsXmlMoveToParent(theResponseXml);
    gsXmlMoveToParent(theResponseXml);
	return GS_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseBrigade(GSXmlStreamReader theResponseXml,
						  GSBBrigade		*theBrigade) 
{
	int tempLen;
	int outValue;
	const char *tempStr;
	const char *emptyStr = "";
	GSResult result;

// 	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADE_ELEMENT)))
// 	{
// 		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADE_ELEMENT);
//         return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
// 	}

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_GAMEID_ELEMENT , (int *)&theBrigade->mGameId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: GameID");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&theBrigade->mBrigadeId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: BrigadeID");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Name", &tempStr, &tempLen)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Name");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
	
    if (tempStr && tempLen > 0)
	{
		theBrigade->mName = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
	}
    else
	{
        theBrigade->mUrl = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

    if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Tag", &tempStr, &tempLen)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Tag");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (tempStr && tempLen > 0)
	{
		theBrigade->mTag = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
	}
    else
	{
        theBrigade->mTag = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

    if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Url", &tempStr, &tempLen)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Url");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (tempStr && tempLen > 0)
	{
		theBrigade->mUrl = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
	}
    else
	{
        theBrigade->mUrl = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "CreatorProfileID", (int *)&theBrigade->mCreatorProfileId)))
    {
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not parse creator profile Id");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "LeaderProfileID", (int *)&theBrigade->mLeaderProfileId)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: LeaderProfileID");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "RecruitingType", &outValue)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: RecruitingType");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
	theBrigade->mRecruitingType = (GSBRecruitingType)outValue;

    if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "MotD", &tempStr, &tempLen)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: MotD");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (tempStr && tempLen > 0)
	{
		theBrigade->mMessageOfTheDay = UTF8ToUCS2StringAllocLen(tempStr, tempLen);
	}
    else
	{
        theBrigade->mMessageOfTheDay = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}
	
	if (theBrigade->mMessageOfTheDay == NULL || theBrigade->mName == NULL || theBrigade->mTag == NULL || theBrigade->mUrl == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError, "Failed to allocate one of the brigade strings!");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	// read in boolean and then compare it below to set the value of mDisbanded
	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Disbanded", &tempStr, &tempLen)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Disbanded");
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	theBrigade->mDisbanded = strncmp(tempStr, GSB_TRUE_TEXT_ELEMENT, tempLen) == 0 ? gsi_true : gsi_false;
	
	theBrigade->mDisbandDate = 0;
	gsXmlReadChildAsDateTimeElement(theResponseXml, GSB_DISBANDDATE_ELEMENT, &theBrigade->mDisbandDate);
    theBrigade->mCreatedDate = 0;
    gsXmlReadChildAsDateTimeElement(theResponseXml, GSB_CREATEDDATE_ELEMENT, &theBrigade->mCreatedDate);

	result = gsbiParseBrigadeLogoList(theResponseXml, &theBrigade->mLogoList);
	if (GS_FAILED(result))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse brigade logo list");
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseEntitlements(GSXmlStreamReader theResponseXml,
                               GSBEntitlementList	*entitlementList) 
{
	int len;
	gsi_u32 i;
	const char *entitlementStr;
	const char *emptyStr = "";

    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ENTITLEMENTLIST_ELEMENT)) ||
        gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&entitlementList->mCount)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (entitlementList->mCount == 0)
	{
		entitlementList->mEntitlements = NULL;
		return GS_SUCCESS;
	}

    entitlementList->mEntitlements = gsimalloc(sizeof(GSBEntitlement) * entitlementList->mCount);
	if (entitlementList->mEntitlements == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate entitlementList->mEntitlements");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(entitlementList->mEntitlements, 0 , sizeof(GSBEntitlement) * entitlementList->mCount );
    for (i = 0; i < entitlementList->mCount; i++) 
    {

		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_ENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to entitlement");
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_ENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to entitlement");
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

        if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "EntitlementID", (int *)&entitlementList->mEntitlements[i].mEntitlementId)))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse entitlement Id");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }
            
        if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "EntitlementName", &entitlementStr, &len)))
        {
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse entitlement name");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }
    	
        if (entitlementStr)
	    {
		    entitlementList->mEntitlements[i].mEntitlementName = UTF8ToUCS2StringAllocLen(entitlementStr, len);
	    }
        else
		{
            entitlementList->mEntitlements[i].mEntitlementName = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
		}

		if (entitlementList->mEntitlements[i].mEntitlementName == NULL)
		{
			GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in entitlementList->mEntitlements[%d].mEntitlementName", i);
			return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}
	}
	
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseRole(GSXmlStreamReader theResponseXml,
					   GSBRole	*role)
{
	const char *roleStr;
	const char *emptyStr = "";
	int len;

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&role->mBrigadeId)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADEID_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ROLEID_ELEMENT, (int *)&role->mRoleId)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_ROLEID_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}  

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "RoleName", &roleStr, &len)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml: RoleName");
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (roleStr)
	{	
		role->mRoleName = UTF8ToUCS2StringAllocLen(roleStr, len);
	}
	else
	{
		role->mRoleName = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	if (role->mRoleName == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in mRoleName");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_ISDEFAULT_ELEMENT, &roleStr, &len)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml: %s", GSB_ISDEFAULT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	role->mIsDefault = strncmp(roleStr, GSB_TRUE_TEXT_ELEMENT, len) == 0 ? gsi_true : gsi_false;

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_ISGAMEROLE_ELEMENT, &roleStr, &len)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml: %s", GSB_ISDEFAULT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	role->mIsGameRole = strncmp(roleStr, GSB_TRUE_TEXT_ELEMENT, len) == 0 ? gsi_true : gsi_false;
	
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseRoleList(GSXmlStreamReader theResponseXml,
                               GSBRoleList	**roleList) 
{
	gsi_u32 i;
	GSResult result;
	GSBRoleList *roleListToFill;
	
	// use a temp to allocate and then fill *roleList with this
	roleListToFill = (GSBRoleList *)gsimalloc(sizeof(GSBRoleList));
	if (roleListToFill == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not allocate GSBRoleList");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(roleListToFill, 0 , sizeof(GSBRoleList));
    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ROLELIST_ELEMENT)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&roleListToFill->mCount)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (roleListToFill->mCount == 0)
	{
		roleListToFill->mRoles = NULL;
		return GS_SUCCESS;
	}

    roleListToFill->mRoles = (GSBRole *)gsimalloc(sizeof(GSBRole) * roleListToFill->mCount);
	if (roleListToFill->mRoles == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate roleList->mRoles");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(roleListToFill->mRoles, 0, sizeof(GSBRole) * roleListToFill->mCount);
    for (i = 0; i < roleListToFill->mCount; i++) 
    {
		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ROLE_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLE_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}	
		}
		else
		{	
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_ROLE_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLE_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		result = gsbiParseRole(theResponseXml, &roleListToFill->mRoles[i]);
		if (GS_FAILED(result))
		{
			GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not parse: %s", GSB_ROLE_ELEMENT);
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}

	*roleList = roleListToFill;
	
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseRoleEntitlements(GSXmlStreamReader               theResponseXml,
                                            GSBRoleEntitlementList	*roleEntitlementList) 
{
	gsi_u32 i;

    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ROLEENTITLEMENTLIST_ELEMENT)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&roleEntitlementList->mCount)))
	{		
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (roleEntitlementList->mCount == 0)
	{
		roleEntitlementList->mRoleEntitlements = NULL;
		return GS_SUCCESS;
	}

    roleEntitlementList->mRoleEntitlements = (GSBRoleEntitlement *)gsimalloc(sizeof(GSBRoleEntitlement) * roleEntitlementList->mCount);
	if (roleEntitlementList->mRoleEntitlements == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate roleEntitlementList->mRoleEntitlements");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(roleEntitlementList->mRoleEntitlements, 0,(sizeof(GSBRoleEntitlement)*roleEntitlementList->mCount));
    for (i = 0; i < roleEntitlementList->mCount; i++) 
    {

		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

        if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "RoleEntitlementID", (int *)&roleEntitlementList->mRoleEntitlements[i].mRoleEntitlementId)))
        {
			GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not parse xml tag: RoleEntitlementID");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        }
            
//         if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ROLEID_ELEMENT, &roleEntitlementList->mRoleEntitlements[i]->mRoleId)))
//         {
// 			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_ROLEID_ELEMENT);
// 			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
//         }
            
//         if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ENTITLEMENTID_ELEMENT, &roleEntitlementList->mRoleEntitlements[i].mEntitlementId)))
//         {
// 			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_ENTITLEMENTID_ELEMENT);
// 			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
//         }
    }
	
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseRoleEntitlementsForRoleIds(GSXmlStreamReader		theResponseXml,
											 GSBRoleIdList			*roleIdList) 
{
	gsi_u32 i;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ROLEENTITLEMENTLIST_ELEMENT)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&roleIdList->mCount)))
	{		
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (roleIdList->mCount == 0)
	{
		roleIdList->mRoleIds = NULL;
		return GS_SUCCESS;
	}

	roleIdList->mRoleIds = (gsi_u32 *)gsimalloc(sizeof(gsi_u32) * roleIdList->mCount);
	if (roleIdList->mRoleIds == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate roleIdList->mRoleIds");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(roleIdList->mRoleIds, 0, sizeof(gsi_u32) * roleIdList->mCount);
	for (i = 0; i < roleIdList->mCount; i++) 
	{

		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

		if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ROLEID_ELEMENT, (int *)&roleIdList->mRoleIds[i])))
		{
			GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not parse xml tag: RoleEntitlementID");
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseRoleEntitlementsForEntitlementIds(GSXmlStreamReader       theResponseXml,
													GSBEntitlementIdList	*entitlementIdList) 
{
	gsi_u32 i;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_ROLEENTITLEMENTLIST_ELEMENT)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&entitlementIdList->mCount)))
	{		
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (entitlementIdList->mCount == 0)
	{
		entitlementIdList->mEntitlementIds = NULL;
		return GS_SUCCESS;
	}

	entitlementIdList->mEntitlementIds = (gsi_u32 *)gsimalloc(sizeof(gsi_u32) * entitlementIdList->mCount);
	if (entitlementIdList->mEntitlementIds == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate entitlementIdList->mEntitlementIds");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(entitlementIdList->mEntitlementIds, 0, sizeof(gsi_u32) * entitlementIdList->mCount); 
	for (i = 0; i < entitlementIdList->mCount; i++) 
	{

		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_ROLEENTITLEMENT_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_ROLEENTITLEMENT_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

		if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ENTITLEMENTID_ELEMENT, (int *)&entitlementIdList->mEntitlementIds[i])))
		{
			GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "Could not parse xml tag: RoleEntitlementID");
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}

	return GS_SUCCESS;
}

GSResult gsbiParseBrigadeMember(GSXmlStreamReader    theResponseXml,
                                GSBBrigadeMember     *brigadeMember)
{

    const char *  tempStr = NULL;
    int     len = 0;

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEMEMBERID_ELEMENT, (int *)&brigadeMember->mBrigadeMemberId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: BrigadeMemberID");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&brigadeMember->mBrigadeId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_BRIGADEID_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_PROFILEID_ELEMENT, (int *)&brigadeMember->mProfileId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_PROFILEID_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    brigadeMember->mDescription = gsbiParseXmlToUCString(theResponseXml, "Description", gsi_false);

    if (gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml, "DateAdded", &brigadeMember->mDateAdded)))
    {
//         GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: DateAdded");
//         return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
        // do not fail for the time being it might be NULL
        brigadeMember->mDateAdded = 0;
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "Status", (int *) &brigadeMember->mStatus )))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Status");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    brigadeMember->mTitle = gsbiParseXmlToUCString(theResponseXml, "Title", gsi_false); 

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_ROLEID_ELEMENT, (int *)&brigadeMember->mRoleId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_ROLEID_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "EmailOptIn", &brigadeMember->mEmailOptIn)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: EmailOptIn");
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, GSB_ISLEADER_ELEMENT, &tempStr, &len)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_ISLEADER_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    brigadeMember->mIsLeader = strncmp(tempStr, GSB_TRUE_TEXT_ELEMENT, len) == 0 ? gsi_true : gsi_false;
    return GS_SUCCESS;
}

GSResult gsbiParsePendingAction(GSXmlStreamReader			theResponseXml,
								GSBBrigadePendingActions    *pendingAction)
{

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&pendingAction->mBrigadeId))
		)
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: BrigadeID");
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if(gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml, GSB_DATEADDED_ELEMENT, &pendingAction->mDateAdded)))
	{
		// do not fail for the time being it might be NULL
		pendingAction->mDateAdded = 0;
	}

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_STATUS_ELEMENT, (int *) &pendingAction->mStatus )))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: Status");
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	pendingAction->mBrigadeName = gsbiParseXmlToUCString(theResponseXml, GSB_BRIGADENAME_ELEMENT, gsi_false); 

	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParsePlayers(GSXmlStreamReader             theResponseXml,
                          GSBBrigadeMemberList	        *memberList) 
{
    gsi_u32 i;

    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_PLAYERLIST_ELEMENT)) ||
        gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&memberList->mCount)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (memberList->mCount == 0)
    {
        memberList->mBrigadeMembers = NULL;
        return GS_SUCCESS;
    }

    memberList->mBrigadeMembers = (GSBBrigadeMember *)gsimalloc(sizeof(GSBBrigadeMember) * memberList->mCount);
    if (memberList->mBrigadeMembers == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memberList->mBrigadeMembers");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    
    memset(memberList->mBrigadeMembers, 0, sizeof(GSBBrigadeMember) * memberList->mCount);
    for (i = 0; i < memberList->mCount; i++) 
    { 
        if (i == 0)
        {
            if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_BRIGADEMEMBER_ELEMENT)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_BRIGADEMEMBER_ELEMENT);
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
            }
        }
        else
        {
            if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_BRIGADEMEMBER_ELEMENT)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_BRIGADEMEMBER_ELEMENT);
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
            }
        }

        if( GS_FAILED(gsbiParseBrigadeMember(theResponseXml, &memberList->mBrigadeMembers[i])))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse BrigadeMember ");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);

        }
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParsePendingActions(GSXmlStreamReader              theResponseXml,
								 GSBBrigadePendingActionsList	*actionList) 
{
	gsi_u32 i;
	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_GETBRIGADEMEMBERWITHBRIGADELIST)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&actionList->mCount)))
	{
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (actionList->mCount == 0)
	{
		actionList->mPendingActions = NULL;
		return GS_SUCCESS;
	}

	actionList->mPendingActions = (GSBBrigadePendingActions *)gsimalloc(sizeof(GSBBrigadePendingActions) * actionList->mCount);
	if (actionList->mPendingActions == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate actionList->mPendingActions");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(actionList->mPendingActions, 0, sizeof(GSBBrigadePendingActions) * actionList->mCount);
	for (i = 0; i < actionList->mCount; i++) 
	{ 
		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_GETBRIGADEMEMBERWITHBRIGADE)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_GETBRIGADEMEMBERWITHBRIGADE);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_GETBRIGADEMEMBERWITHBRIGADE)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_GETBRIGADEMEMBERWITHBRIGADE);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

		if( GS_FAILED(gsbiParsePendingAction(theResponseXml, &actionList->mPendingActions[i])))
		{
			GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse PendingAction ");
			return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseBrigadeMembers(GSXmlStreamReader              theResponseXml,
                                 GSBBrigadeMemberList	        *memberList) 
{
	gsi_u32 i;
    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADEMEMBERLIST_ELEMENT)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&memberList->mCount)))
    {
		GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
		return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (memberList->mCount == 0)
	{
		memberList->mBrigadeMembers = NULL;
		return GS_SUCCESS;
	}

    memberList->mBrigadeMembers = (GSBBrigadeMember *)gsimalloc(sizeof(GSBBrigadeMember) * memberList->mCount);
	if (memberList->mBrigadeMembers == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memberList->mBrigadeMembers");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    memset(memberList->mBrigadeMembers,0,sizeof(GSBBrigadeMember) * memberList->mCount);
    for (i = 0; i < memberList->mCount; i++) 
    { 
		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_BRIGADEMEMBER_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_BRIGADEMEMBER_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_BRIGADEMEMBER_ELEMENT)))
			{
				GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to child: %s", GSB_BRIGADEMEMBER_ELEMENT);
				return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

        if( GS_FAILED(gsbiParseBrigadeMember(theResponseXml, &memberList->mBrigadeMembers[i])))
        {
            GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Failed to parse BrigadeMember ");
            return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);

        }

    }
	
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseHistoryList (GSXmlStreamReader        theResponseXml,
                               GSBBrigadeHistoryList	*historyList) 
{
    gsi_u32 i;
    GSResult result = GS_SUCCESS;

    if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GSB_BRIGADEHISTORY_ELEMENT)) ||
        gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_COUNT_ELEMENT, (int *)&historyList->mCount)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml tag: %s", GSB_COUNT_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (historyList->mCount == 0)
    {
        historyList->mBrigadeHistory = NULL;
        return GS_SUCCESS;
    }

    historyList->mBrigadeHistory = (GSBBrigadeHistoryEntry *)gsimalloc(sizeof(GSBBrigadeHistoryEntry ) * historyList->mCount);
    if (historyList->mBrigadeHistory == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate historyList->mEntitlements");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
   
    memset(historyList->mBrigadeHistory, 0 , (sizeof(GSBBrigadeHistoryEntry ) * historyList->mCount));
    for (i = 0; i < historyList->mCount; i++) 
    {

        if (i == 0)
        {
            if (gsi_is_false(gsXmlMoveToChild(theResponseXml,GSB_HISTORYENTRY_ELEMENT)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to history entry");
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
            }
        }
        else
        {
            if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GSB_HISTORYENTRY_ELEMENT)))
            {
                GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not move to history entry");
                return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
            }
        }
        result = gsbiParseHistoryEntry(theResponseXml, &historyList->mBrigadeHistory[i]);
        if (GS_FAILED(result))
        {
            // to do deallocate memory and return
            return result;
        }
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbiParseHistoryEntry (GSXmlStreamReader theResponseXml,
                                GSBBrigadeHistoryEntry	*historyEntry) 
{

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_HISTORYENTRYID_ELEMENT, (int *)&historyEntry->mHistoryEntryId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s", GSB_HISTORYENTRYID_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_BRIGADEID_ELEMENT, (int *)&historyEntry->mBrigadeId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s", GSB_BRIGADEID_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }
    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_INSTIGATINGPROFILEID, (int *)&historyEntry->mInstigatingProfileId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s", GSB_INSTIGATINGPROFILEID);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_TARGETPROFILEID, (int *)&historyEntry->mTargetProfileId)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s", GSB_TARGETPROFILEID);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    //<EntryType>
    historyEntry->mAccessLevel = gsbiParseXmlToUCString(theResponseXml,GSB_ENTRYTYPE_ELEMENT, gsi_false);
    if (historyEntry->mAccessLevel == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mAccessLevel");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    historyEntry->mHistoryAction = gsbiParseXmlToUCString(theResponseXml,GSB_ACTION_ELEMENT, gsi_false);
    if (historyEntry->mHistoryAction == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mHistoryAction");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    //<Notes></Notes>
    historyEntry->mNotes = gsbiParseXmlToUCString(theResponseXml,GSB_NOTES_ELEMENT, gsi_false);
    if (historyEntry->mNotes == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mNotes");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    //<DateCreated></DateCreated>
    if (gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml, GSB_DATECREATED_ELEMENT,&historyEntry->mDateCreated)))
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s", GSB_DATECREATED_ELEMENT);
        return GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    historyEntry->mSourceProfileNickname = gsbiParseXmlToUCString(theResponseXml,GSB_SOURCEPROFILENICK_ELEMENT, gsi_false);
    if (historyEntry->mSourceProfileNickname == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mSourceProfileNickname");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    historyEntry->mTargetProfileNickname = gsbiParseXmlToUCString(theResponseXml,GSB_TARGETPROFILENICK_ELEMENT,gsi_false);
    if (historyEntry->mTargetProfileNickname == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mTargetProfileNickname");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    if(gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GSB_REFERENCEID_ELEMENT, (int *)&historyEntry->mReferenceId)))
    {
        historyEntry->mReferenceId = 0;
//         GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to fill in historyEntry->mTargetProfileNickname");
//         return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    return GS_SUCCESS;
}

UCS2String gsbiParseXmlToUCString( GSXmlStreamReader theResponseXml, const char *matchtag, gsi_bool required )
{
    int len;
    const char *xmlStr = NULL;
    const char *emptyStr = "";

    if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, matchtag, &xmlStr, &len)) && required)
    {
        GSB_DEBUG_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse %s",matchtag );
        return NULL; //GSB_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
    }

    if (xmlStr)
    {
        return UTF8ToUCS2StringAllocLen(xmlStr, len);
    }
    else
    {
        return UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
    }
}
