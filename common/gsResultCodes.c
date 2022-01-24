// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
///////////////////////////////////////////////////////////////////////////////
#include "gsCommon.h"
#include "gsResultCodes.h"


void gsResultCodesFreeResultSet(GSResultSet *resultSet)
{
	if (resultSet)
	{
		gsifree(resultSet->mErrorMessage);
		gsifree(resultSet->mResults);
		resultSet->mNumResults = 0;
	}	
}

GSResult gsResultCodesCreateResultSet(GSResultSet **resultSet)
{
	GS_ASSERT(resultSet);
	if (!resultSet)
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_State, GSIDebugLevel_WarmError, "resultSet is NULL");
		return GS_ERROR(GSResultSDK_Common, GSResultSection_State, GSResultCode_InvalidParameters);
	}

	*resultSet = (GSResultSet *)gsimalloc(sizeof(GSResultSet));
	if (!(*resultSet))
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_WarmError, "Failed to allocate memory for resultSet");
		return GS_ERROR(GSResultSDK_Common, GSResultSection_Memory, GSResultCode_InvalidParameters);
	}
	memset(*resultSet, 0, sizeof(GSResultSet));
	return GS_SUCCESS;
}