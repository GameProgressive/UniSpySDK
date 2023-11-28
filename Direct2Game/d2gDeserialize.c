///////////////////////////////////////////////////////////////////////////////
// d2gDeserialize.c
//
// GameSpy  DIRECT TO GAME SDK
// This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
// Copyright (c) 2008, GameSpy Technology
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// includes
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "../ghttp/ghttpCommon.h"
#include "Direct2Game.h"
#include "d2gMain.h"
#include "d2gServices.h"
#include "d2gUtil.h"
#include "d2gDeserialize.h"

///////////////////////////////////////////////////////////////////////////////
// General Parsing functions
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetResponse(GSXmlStreamReader theResponseXml , 
							  const char        *theResponseTag)
{
	if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
		gsi_is_false(gsXmlMoveToNext(theResponseXml, theResponseTag)))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError, 
			"%s :: %s (%d): could not parse xml: start; tag %s", 
			__FILE__, __FUNCTION__, __LINE__, theResponseTag);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	return d2giParseResponseHeader(theResponseXml);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseResponseHeader(GSXmlStreamReader theResponseXml)
{
	GSResult result;
	int responseStatusCode;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_RESULT_SECTION)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_RESULT_CODE, &responseStatusCode)) ||
		gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{		
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	result = d2giResultFromStatusCode(responseStatusCode);
	return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseItemInfoFromResponse(D2GIInstance      *theInstance, 
									   D2GBasicItemInfo       *pItem,  
									   GSXmlStreamReader theResponse)
{
	const char  *emptyStr = "";
	const char  *itemStrings[4];
	int         lenItemStrings[4];


	if (gsi_is_false(gsXmlReadChildAsInt   (theResponse, "itemid",     (int *)&pItem->mItemId)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "externalid", &itemStrings[0], &lenItemStrings[0])) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "name",       &itemStrings[1], &lenItemStrings[1])) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "price",      &itemStrings[2],&lenItemStrings[2]))  ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "tax",        &itemStrings[3], &lenItemStrings[3]))
		)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: itemid\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);

	}

	//externalId
	if (itemStrings[0] != NULL && lenItemStrings[0] > 0)
	{
		pItem->mExternalItemCode = UTF8ToUCS2StringAllocLen(itemStrings[0], lenItemStrings[0]);
	}

	else
	{				
		pItem->mExternalItemCode = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	// Name: if name is empty we should still set it to the empty string so 
	// that we don't have NULLs
	if (itemStrings[1] && lenItemStrings[1] > 0)
	{
		pItem->mName = UTF8ToUCS2StringAllocLen(itemStrings[1], lenItemStrings[1]);
	}
	else 
	{
		pItem->mName = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	// Price: should not be null
	pItem->mPrice = UTF8ToUCS2StringAllocLen(itemStrings[2], lenItemStrings[2]);

	// Tax: should not be null
	pItem->mTax = UTF8ToUCS2StringAllocLen(itemStrings[3], lenItemStrings[3]);

	if (pItem->mExternalItemCode == NULL ||
		pItem->mName  == NULL || 
		pItem->mPrice == NULL || 
		pItem->mTax   == NULL)
	{
		gsifree(pItem->mExternalItemCode);
		gsifree(pItem->mName);
		gsifree(pItem->mPrice);
		gsifree(pItem->mTax);

		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s :: %s (%d): out of memory: item strings\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);		
	}	
	GSI_UNUSED(theInstance);
	return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseProductInfoFromResponse( D2GIInstance      *theInstance, 
										  D2GProductInfo    *pProductInfo,  
										  GSXmlStreamReader theResponse)
{
	const char  *emptyStr = "";
	const char  *itemStrings[3];
	int         lenItemStrings[3];

	if (gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponse, "releasedate", &pProductInfo->mReleaseDate)))
	{
		pProductInfo->mReleaseDate = 0;
	}

	if (gsi_is_false(gsXmlReadChildAsInt(theResponse, "filesizebytessum", (int *) &pProductInfo->mFileSize)))
	{
		pProductInfo->mFileSize = 0;
	}

	if (gsi_is_false(gsXmlReadChildAsString(theResponse, "publisher", &itemStrings[0], &lenItemStrings[0])) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "developer", &itemStrings[1], &lenItemStrings[1])) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "summary", &itemStrings[2], &lenItemStrings[2])))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: itemStrings[1-6]\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);

	}

	// Publisher: same as Name
	if (itemStrings[0] && lenItemStrings[0] > 0)
	{
		pProductInfo->mPublisher = UTF8ToUCS2StringAllocLen(itemStrings[0], lenItemStrings[0]);
	}
	else
	{
		pProductInfo->mPublisher = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	// Developer: same as Name
	if (itemStrings[1] && lenItemStrings[1] > 0)
	{
		pProductInfo->mDeveloper = UTF8ToUCS2StringAllocLen(itemStrings[1], lenItemStrings[1]);
	}
	else
	{
		pProductInfo->mDeveloper = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	// Summary: same as Summary
	if (itemStrings[2] && lenItemStrings[2] > 0)
	{
		pProductInfo->mSummary = UTF8ToUCS2StringAllocLen(itemStrings[2], lenItemStrings[2]);
	}
	else
	{
		pProductInfo->mSummary = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	if (pProductInfo->mPublisher == NULL || 
		pProductInfo->mDeveloper == NULL || 
		pProductInfo->mSummary == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s :: %s (%d): out of memory: item strings\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);		
	}	
	GSI_UNUSED(theInstance);

	return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGeoInfoFromResponse( D2GIInstance  *theInstance, 
									  D2GGeoInfo    *pGeoInfo,  
									  GSXmlStreamReader theResponse)
{
	const char  *emptyStr = "";
	const char  *itemStrings[2];
	int         lenItemStrings[2];

	if (
		gsi_is_false(gsXmlReadChildAsString(theResponse, "culturecode", &itemStrings[0], &lenItemStrings[0])) ||
		gsi_is_false(gsXmlReadChildAsString(theResponse, "currencycode", &itemStrings[1], &lenItemStrings[1])))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: itemStrings[1-6]\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);

	}

	// Publisher: same as Name
	if (itemStrings[0] && lenItemStrings[0] > 0)
	{
		pGeoInfo->mCultureCode = UTF8ToUCS2StringAllocLen(itemStrings[0], lenItemStrings[0]);
	}
	else
	{
		pGeoInfo->mCultureCode = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	// Developer: same as Name
	if (itemStrings[1] && lenItemStrings[1] > 0)
	{
		pGeoInfo->mCurrencyCode = UTF8ToUCS2StringAllocLen(itemStrings[1], lenItemStrings[1]);
	}
	else
	{
		pGeoInfo->mCurrencyCode = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
	}

	if (pGeoInfo->mCultureCode == NULL || 
		pGeoInfo->mCurrencyCode == NULL )
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s :: %s (%d): out of memory: item strings\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);		
	}	

	GSI_UNUSED(theInstance);
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseImageListFromResponse( D2GIInstance      *theInstance, 
										D2GImageList      *pImageList,  
										GSXmlStreamReader theResponseXml)
{
	gsi_u32 i = 0;

	if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "imagelist")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", (int *)&pImageList->mCount)) ||
		gsi_is_false(gsXmlMoveToChild(theResponseXml, "image")))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: images\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}		

	pImageList->mImageName = (UCS2String *)gsimalloc(sizeof(UCS2String) * pImageList->mCount);
	if (pImageList->mImageName == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s :: %s (%d): out of memory: mImageName\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}	
	memset(pImageList->mImageName, 0, sizeof(UCS2String *) * pImageList->mCount);

	for (i = 0; i< pImageList->mCount; i++)
	{
		const char *tempStr;
		int        tempStrLen;

		if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "url", &tempStr, &tempStrLen)))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: image url\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		pImageList->mImageName[i] = UTF8ToUCS2StringAllocLen(tempStr, tempStrLen);
		if (pImageList->mImageName[i] == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"%s :: %s (%d): out of memory: pImageList->mImageName[i]\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		if ((i < pImageList->mCount -1))
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "image")))
			{
				gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse xml stream: images\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}			
	}
	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	GSI_UNUSED(theInstance);
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseCategoryListFromResponse(D2GIInstance     *theInstance, 
										   D2GCategoryList   *pCategoryList,  
										   GSXmlStreamReader theResponseXml)
{
	gsi_u32 i = 0;
	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "categorylist")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", (int*)&pCategoryList->mCount)) ||
		gsi_is_false(gsXmlMoveToChild(theResponseXml, "category")))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: categories\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	pCategoryList->mCategoryNames= (UCS2String *)gsimalloc(sizeof(UCS2String) * pCategoryList->mCount);
	if (pCategoryList->mCategoryNames == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s :: %s (%d): out of memory: pCategoryList->mCategoryName\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	memset(pCategoryList->mCategoryNames, 0, sizeof(UCS2String) * pCategoryList->mCount);

	for (i = 0; i < pCategoryList->mCount; i++)
	{
		const char *tempStr;
		int   tempStrLen;
		if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "name", &tempStr, &tempStrLen)))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d) could not parse xml stream: name\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);			
		}

		pCategoryList->mCategoryNames[i] = UTF8ToUCS2StringAllocLen(tempStr, tempStrLen);
		if (pCategoryList->mCategoryNames[i] == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"%s :: %s (%d): out of memory: mCategoryName[i]\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);			
		}

		if (i < pCategoryList->mCount -1)
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "category")))
			{
				gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse XML stream: categories\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);				
			}
		}
	}
	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}  
	GSI_UNUSED(theInstance);
	return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseItemFromResponse(D2GIInstance      *theInstance, 
								   D2GICatalog       *theCatalog,
								   D2GCatalogItem    **item, 
								   GSXmlStreamReader theResponse)
{

	D2GBasicItemInfo itemInfo;
	D2GCatalogItem * currentItem;	

	GSResult    result = GS_SUCCESS; 

	if ((result= d2giParseItemInfoFromResponse(theInstance, &itemInfo, theResponse))
		!= GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse Item Info.");
		return result;
	}

	// Update the catalog with the item received in the response.
	// If we do not have an entry for this item then a new item 
	// is added to the catalog.
	currentItem = d2giGetCatalogItem(theInstance, theCatalog, itemInfo.mItemId );
	if (currentItem == NULL)
	{
		currentItem = d2giNewCatalogItem(theInstance, theCatalog, itemInfo.mItemId);
	}
	else 
	{
		d2giResetCatalogItem(currentItem);
		currentItem->mItem.mItemId = itemInfo.mItemId;
	}

	if (currentItem == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Out of memory: currentItem.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}		

	//Shallow Copy
	currentItem->mItem = itemInfo;
	// parse release date separately 
	result= d2giParseGeoInfoFromResponse(theInstance, &currentItem->mGeoInfo, theResponse);
	if (result != GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse Geo Info.");
		return result;
	}

	result= d2giParseProductInfoFromResponse(theInstance, &currentItem->mProductInfo, theResponse);
	if (result != GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse Product Info.");
		return result;
	}

	result = d2giParseCategoryListFromResponse(theInstance, &currentItem->mCategories, theResponse);
	if (result != GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse Category List.");
		return result;
	}

	result = d2giParseImageListFromResponse(theInstance, &currentItem->mImages, theResponse);
	if (result!= GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse Image List.");
		return result;
	}

	result = d2giParseExtraInfoList(theInstance, NULL, theResponse, &currentItem->mExtraItemInfoList);
	if (result != GS_SUCCESS)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse extra info list.");
		return result;
	}
	

	*item = currentItem;
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseItemTotalFromResponse( D2GIInstance      *theInstance, 
										D2GOrderItemTotal *theItemTotal,
										GSXmlStreamReader theResponseXml)

{
	GSResult    result = GS_SUCCESS;

	const char  *tempSubTotal,   *tempTotal;
	int         tempSubTotalLen, tempTotalLen;

	if (gsi_is_false(gsXmlReadChildAsInt   (theResponseXml, "quantity", (int *)&theItemTotal->mQuantity)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "subtotal", &tempSubTotal, &tempSubTotalLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "total",    &tempTotal, &tempTotalLen)))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: total", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	theItemTotal->mSubTotal = UTF8ToUCS2StringAllocLen(tempSubTotal, tempSubTotalLen);
	theItemTotal->mTotal = UTF8ToUCS2StringAllocLen(tempTotal, tempTotalLen);

	if (theItemTotal->mSubTotal == NULL || theItemTotal->mTotal == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to convert to UCS2String, out of memory", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}    
	GSI_UNUSED(theInstance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderItemValidationFromResponse(GSXmlStreamReader  theResponseXml, 
												  D2GOrderValidation *theValidation)

{
	GSResult   result = GS_SUCCESS;
	const char *tempIsValid,*tempMsg;
	int        tempValidCode, tempIsValidLen, tempMsgLen;

	memset(theValidation, 0, sizeof(D2GOrderValidation));
	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderitemvalidation")))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: ordervalidation\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "isvalid", &tempIsValid, &tempIsValidLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "message", &tempMsg, &tempMsgLen)) ||
		gsi_is_false(gsXmlReadChildAsInt   (theResponseXml, "orderitemstatuscode", &tempValidCode))
		)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: orderstatuscode\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	theValidation->mIsValid = strncmp(tempIsValid, "true", tempIsValidLen) == 0 ? gsi_true : gsi_false;
	theValidation->mResult  = d2giOrderValidationResultFromCode(tempValidCode);
	theValidation->mMessage = UTF8ToUCS2StringAllocLen(tempMsg, tempMsgLen);

	if (theValidation->mIsValid == gsi_false && 
		theValidation->mMessage == NULL)
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to convert to UCS2String, possibly out of memory", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
	}

	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderItemFromResponse(D2GIInstance *theInstance, 
										D2GICatalog  *theCatalog,
										D2GOrderItem *theOrderItem, 
										GSXmlStreamWriter theResponseXml)
{
	D2GCatalogItem *pCatalogItem = NULL;


	GSResult result = GS_SUCCESS;

	//=========================================
	// Parse the item (itemBase) first
	//=========================================

	memset(&theOrderItem->mItem, 0, sizeof(D2GBasicItemInfo));

	result = d2giParseItemInfoFromResponse(theInstance, &theOrderItem->mItem,theResponseXml);  
	if (GS_FAILED(result))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed to call d2giParseItemFromResponse\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	// Check if have it in the catalog.
	pCatalogItem = d2giGetCatalogItem(theInstance, theCatalog, theOrderItem->mItem.mItemId);

	result =  d2giParseItemTotalFromResponse(  theInstance, &theOrderItem->mItemTotal,theResponseXml);
	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed to call d2giParseItemTotalFromResponse\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	result = d2giParseOrderItemValidationFromResponse(theResponseXml, &theOrderItem->mValidation);

	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed to call d2giParseOrderValidationFromResponse\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderItemLicensesFromResponse(D2GIInstance        *theInstance, 
												D2GLicenseItemList  *theLicenseList, 
												GSXmlStreamReader   theResponseXml)
{
	GSResult result = GS_SUCCESS;


	if (gsi_is_false(gsXmlMoveToChild   (theResponseXml, "licenses")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, (int *)&theLicenseList->mCount)))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: licenses, count\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (theLicenseList->mCount == 0)
	{	
		theLicenseList->mLicenses = NULL;
	}
	else
	{
		const char    *tempLicenseKey, *tempLicenseName;
		int     lenLicenseKey,   lenLicenseName; 
		gsi_u32 i=0;

		theLicenseList->mLicenses = (D2GLicenseItem *)gsimalloc(sizeof(D2GLicenseItem) 
			* theLicenseList->mCount);
		if (theLicenseList->mLicenses == NULL)
		{
			gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to allocate licenses, out of memory", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "license")))
		{
			gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: license\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		for (i = 0; i < theLicenseList->mCount; i++)
		{
			D2GLicenseItem *currentLicense = &theLicenseList->mLicenses[i];
			if(gsi_is_false(gsXmlReadChildAsString(theResponseXml,  "licensekey", &tempLicenseKey, &lenLicenseKey)) ||
				gsi_is_false(gsXmlReadChildAsString(theResponseXml, "licensename", &tempLicenseName, &lenLicenseName)))
			{
				gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse xml stream: licensename\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
			currentLicense->mLicenseKey  = UTF8ToUCS2StringAllocLen(tempLicenseKey, lenLicenseKey);
			currentLicense->mLicenseName = UTF8ToUCS2StringAllocLen(tempLicenseName, lenLicenseName);

			if (currentLicense->mLicenseKey == NULL || currentLicense->mLicenseName == NULL)
			{
				gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): Failed to convert to UCS2String, out of memory", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			}
			if (i < theLicenseList->mCount - 1)
			{	
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "license")))
				{
					gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream (move): license\n",
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}
		}

		if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}    
	GSI_UNUSED(theInstance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderItemDownloadsFromResponse(D2GIInstance        *theInstance, 
												 D2GDownloadItemList *theDownloadList, 
												 GSXmlStreamReader   theResponseXml)
{
	const char  *emptyStr = "";

	if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "downloads")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, (int *)&theDownloadList->mCount)))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream (move): downloads\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (theDownloadList->mCount == 0)
	{
		theDownloadList->mDownloads = NULL;
	}
	else
	{	
		const char *tempName, *tempAssetType;
		int tempNameLen, tempAssetTypeLen;
		gsi_u32 i=0;
		theDownloadList->mDownloads = (D2GDownloadItem *)gsimalloc(sizeof(D2GDownloadItem) * theDownloadList->mCount);
		if (theDownloadList->mDownloads == NULL)
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to allocate downloads, out of memory", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "download")))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: license\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		for (i = 0; i < theDownloadList->mCount; i++)
		{
			D2GDownloadItem *currentItem = &theDownloadList->mDownloads[i];

			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "downloadurlid", (int *)&currentItem->mUrlId)) ||
				gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "fileid", (int *)&currentItem->mFileId)) ||
				gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "sequence", (int *)&currentItem->mSequence)) ||
				gsi_is_false(gsXmlReadChildAsString(theResponseXml, "assettype", &tempAssetType, &tempAssetTypeLen)) ||
				gsi_is_false(gsXmlReadChildAsFloat(theResponseXml, "version",&currentItem->mVersion)) )
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse xml stream: assettype\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}

			if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "name", &tempName, &tempNameLen)))
			{
				currentItem->mName = UTF8ToUCS2StringAllocLen(emptyStr, (int)strlen(emptyStr));
			}
			else 
			{
				currentItem->mName = UTF8ToUCS2StringAllocLen(tempName, tempNameLen);
			}
			// asset type is nullable.
			currentItem->mAssetType = UTF8ToUCS2StringAllocLen(tempAssetType, tempAssetTypeLen);

			if (i < theDownloadList->mCount - 1)
			{	
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "download")))
				{
					gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream (move): download\n", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}
		}
		if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}    
	GSI_UNUSED(theInstance);
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderItemPurchaseFromResponse(D2GIInstance         *theInstance, 
												D2GICatalog          *theCatalog,
												D2GOrderItemPurchase *orderItemPurchase, 
												GSXmlStreamWriter    theResponseXml,
												gsi_bool			 updateManifest)
{
	GSResult result;
	gsi_u32 i;

	result = d2giParseOrderItemFromResponse(theInstance, 
		theCatalog, 
		&orderItemPurchase->mOrderItem, 
		theResponseXml);
	if (GS_FAILED(result))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed to call d2giParseOrderItemFromResponse\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	result = d2giParseOrderItemLicensesFromResponse(theInstance, 
		&orderItemPurchase->mLicenseList, 
		theResponseXml);
	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed parsing license items\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	result = d2giParseOrderItemDownloadsFromResponse(theInstance, 
		&orderItemPurchase->mDownloadList, 
		theResponseXml);
	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): failed parsing download items\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}
	else if (updateManifest)
	{
		// Update manifest file with the purchased content
		for (i= 0; i<orderItemPurchase->mDownloadList.mCount ; i++)
		{
			D2GIManifestRecord manifest;
			manifest.itemId = orderItemPurchase->mOrderItem.mItem.mItemId;
			manifest.urlId  = orderItemPurchase->mDownloadList.mDownloads[i].mUrlId;
			manifest.version = 0.0;
			result = d2giUpdateManifestRecord(theInstance, manifest);
			if (GS_FAILED(result))
			{
				gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): failed when updating Manifest Info for the purchased content.\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return result;
			}

		}
	}
	// Now we start parsing the order item purchases.
	if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "orderitempurchases")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, 
		(int *)&orderItemPurchase->mPurchaseList.mCount)))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream (move): orderitempurchases\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (orderItemPurchase->mPurchaseList.mCount == 0)
	{
		orderItemPurchase->mPurchaseList.mOrderItemPurchases = NULL;
		if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}
	else
	{	
		orderItemPurchase->mPurchaseList.mOrderItemPurchases = (D2GOrderItemPurchase *)gsimalloc(sizeof(D2GOrderItemPurchase) 
			* orderItemPurchase->mPurchaseList.mCount);
		if (orderItemPurchase->mPurchaseList.mOrderItemPurchases == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to allocate sub item purchases,out of memory", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderitempurchase")))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: orderitempurchase\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		for (i = 0; i < orderItemPurchase->mPurchaseList.mCount; i++)
		{

			D2GOrderItemPurchase *currentItem = &orderItemPurchase->mPurchaseList.mOrderItemPurchases[i];

			result = d2giParseOrderItemPurchaseFromResponse(theInstance, 
				theCatalog, 
				currentItem, 
				theResponseXml,
				updateManifest);
			if (GS_FAILED(result))
			{
				gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): failed in d2giParseOrderItemPurchaseFromResponse\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return result;
			}

			if (i < orderItemPurchase->mPurchaseList.mCount - 1)
			{	
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "orderitempurchase")))
				{
					gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream (move): orderitempurchase\n", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}
		}
		if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetStoreAvailResponse(D2GIInstance * instance,
										GSXmlStreamReader theResponseXml,
										D2GGetStoreAvailabilityResponse *response)
{
	GSResult result;
	int storeAvailability;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_GETSTOREAVAIL_RESULT);
	if (GS_FAILED(result))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"%s :: %s (%d): failed to parse status in d2giParseResponseHeader\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "storestatusid", &storeAvailability)))		
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Could not parse XML: storestatusid\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	response->mAvailabilityCode = d2giResultFromStoreAvailabilityCode(storeAvailability);

	GSI_UNUSED(instance);
	return result;
}


///////////////////////////////////////////////////////////////////////////////
//              Credit Cards 
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giParseCreditCardInfoList.
// This function parses the received SOAP message to retrieve 
// user's credit cards.
//
GSResult d2giParseCreditCardInfoList(D2GIInstance      *instance, 
									 GSXmlStreamReader theResponseXml, 
									 D2GGetUserCreditCardsResponse *responseOut)
{
	GSResult result;
	int creditCardIndex = 0;
	int count           = 0;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_GETUSERCREDITCARDS_RESULT);

	if (GS_SUCCEEDED(result))
	{
		responseOut->mListOfCreditCards = (D2GCreditCardInfoList *)gsimalloc(sizeof(D2GCreditCardInfoList));
		if (responseOut->mListOfCreditCards == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_WarmError,
				"%s :: %s (%d): out of memory: item strings\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);	
		}

		// try to parse the soap request message
		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "creditcardlist")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", &count)))
		{
			result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			return result;
		}

		// Initialize the mListOfCreditCards
		if (count == 0)
		{
			responseOut->mListOfCreditCards->mCount = count;
			responseOut->mListOfCreditCards->mCreditCardInfos = NULL;
			return result;
		}
		else
		{
			responseOut->mListOfCreditCards->mCount = count;
			responseOut->mListOfCreditCards->mCreditCardInfos = 
				(D2GCreditCardInfo *)gsimalloc(sizeof(D2GCreditCardInfo) *count);
			memset(responseOut->mListOfCreditCards->mCreditCardInfos, 0, sizeof(D2GCreditCardInfo) * count);
		}

		if (responseOut->mListOfCreditCards->mCreditCardInfos == NULL)
		{
			result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			return result;
		}

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "creditcard")))
		{
			result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			return result;
		}

		for (creditCardIndex = 0; creditCardIndex < count; creditCardIndex++)
		{
			D2GCreditCardInfo *pCreditCard = NULL;
			int nAccountId;
			const char *tempCardType, *tempDefault;
			int tempCardTypeLen, tempDefaultLen;
			int nLastFour;
			time_t tExpireDate;

			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "accountid", &nAccountId)) ||
				gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "lastfournumbers", &nLastFour)) ||
				gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml, "expirationdate", &tExpireDate)) ||
				gsi_is_false(gsXmlReadChildAsString(theResponseXml, "accounttype", &tempCardType, &tempCardTypeLen)) ||
				gsi_is_false(gsXmlReadChildAsString(theResponseXml, "isdefault", &tempDefault, &tempDefaultLen)))
			{
				result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				return result;
			}

			pCreditCard = &responseOut->mListOfCreditCards->mCreditCardInfos[creditCardIndex];
			if (pCreditCard == NULL)
			{
				// This should never happen but just in case
				result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
				return result;
			}

			pCreditCard->mAccountId      = nAccountId;

			pCreditCard->mCreditCardType = UTF8ToUCS2StringAllocLen(tempCardType, tempCardTypeLen);
			if (pCreditCard->mCreditCardType == NULL)
			{
				result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
				return result;
			}

			pCreditCard->mExpirationDate = tExpireDate;
			pCreditCard->mIsDefault = (strncmp(tempDefault, "true", tempDefaultLen)==0)?gsi_true:gsi_false;
			pCreditCard->mLastFourDigits = nLastFour;

			if ((creditCardIndex < count - 1))
			{
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "creditcards")))
				{
					result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
					return result;
				}
			}
		}
	}

	GSI_UNUSED(instance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// Catalog Items  
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giParseItemsFromResponse.
// This is utility function to parse items from a response.
// For those items parsed if they do not exist in the cache already
// they are added to the cache. If they do then they updated with the
// information received with the response message.
// 
// On a failure all the items parsed so far is removed from 
// the cache as well.
//
GSResult d2giParseItemsFromResponse(D2GIInstance      *theInstance, 
									D2GICatalog       *theCatalog,
									GSXmlStreamReader theResponseXml, 
									D2GCatalogItemList   *outItemList)
{
	GSResult result = GS_SUCCESS;
	int count;
	int i;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_CATALOG_ITEM_LIST)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, &count)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream: %s, %s.", 
			GS_D2G_CATALOG_ITEM_LIST, GS_D2G_COUNT_ELEMENT);
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		return d2giFreeCatalogItems(theCatalog, outItemList, result, gsi_true);
	}

	if (count == 0)
	{
		outItemList->mCount = 0;
		outItemList->mCatalogItems = NULL;
		return result;
	}
	// Initialize the outItemList
	outItemList->mCount = count;
	outItemList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem ) * count);
	if (outItemList->mCatalogItems == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Out of memory: outItemList->mCatalogItems.");
		result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		return d2giFreeCatalogItems(theCatalog, outItemList, result, gsi_true);
	}
	memset(outItemList->mCatalogItems, 0, sizeof(D2GCatalogItem) * count);

	// Start processing all the items from the response
	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_CATALOG_ITEM)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream: %s.", GS_D2G_CATALOG_ITEM);
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		return d2giFreeCatalogItems(theCatalog, outItemList, result, gsi_true);
	}

	for (i = 0; i < count; i++)
	{
		D2GCatalogItem *currentItem=NULL;			

		result = d2giParseItemFromResponse(theInstance, theCatalog, &currentItem, theResponseXml);
		if (GS_FAILED(result))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream for %s.", GS_D2G_CATALOG_ITEM);
			return d2giFreeCatalogItems(theCatalog, outItemList, result, gsi_true);
		}

		//shallow copy
		outItemList->mCatalogItems[i] = *currentItem;

		if ((i < count-1))
		{
			if (gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
				gsi_is_false(gsXmlMoveToSibling(theResponseXml, GS_D2G_CATALOG_ITEM)))
			{
				GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream: %s.", GS_D2G_CATALOG_ITEM);
				result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				return d2giFreeCatalogItems(theCatalog, outItemList, result, gsi_true);
			}
		}	
	}   // for loop

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseLoadCatalogItemsFromResponse(D2GIInstance      *theInstance, 
											   D2GICatalog       *theCatalog,
											   GSXmlStreamReader theResponseXml, 
											   D2GLoadCatalogItemsResponse *getAllItemsResponse)
{

	GSResult result = GS_SUCCESS;

	result = d2giParseGetResponse(theResponseXml,GS_D2G_GETALLITEMS_RESULT );
	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): got bad status code %x\n", 
			__FILE__, __FUNCTION__, __LINE__ , result);

		// gsi_true - We failed remove all items permanently 
		return d2giFreeCatalogItems(theCatalog, getAllItemsResponse->mItemList, result, gsi_true);
	}
	getAllItemsResponse->mItemList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));

	if (getAllItemsResponse->mItemList == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): got bad status code %x\n", 
			__FILE__, __FUNCTION__, __LINE__ , result);

		// We failed; Remove all items permanently (the last parameter tells the method to do this)
		return d2giFreeCatalogItems(theCatalog, getAllItemsResponse->mItemList, GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory), gsi_true);
	}
	memset(getAllItemsResponse->mItemList, 0, sizeof(D2GCatalogItemList *));
	return d2giParseItemsFromResponse(theInstance, theCatalog, theResponseXml, getAllItemsResponse->mItemList);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetItemsByCategoryFromResponse(D2GIInstance        *theInstance, 
												 D2GICatalog         *theCatalog,
												 GSXmlStreamReader   theResponseXml, 
												 D2GLoadCatalogItemsByCategoryResponse *response)
{

	GSResult result = GS_SUCCESS;

	result =  d2giParseGetResponse(theResponseXml,GS_D2G_GETITEMSBYCATEGORY_RESULT );
	if (GS_FAILED(result))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): got bad status code\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return d2giFreeCatalogItems(theCatalog, response->mItemList, result, gsi_true);
	}

	response->mItemList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));
	if (response->mItemList == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): got bad status code %x\n", 
			__FILE__, __FUNCTION__, __LINE__ , result);

		// We failed; Remove all items permanently (the last parameter tells the method to do this)
		return d2giFreeCatalogItems(theCatalog, response->mItemList, GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory), gsi_true);
	}

	return d2giParseItemsFromResponse(theInstance, theCatalog, theResponseXml, response->mItemList);
}

///////////////////////////////////////////////////////////////////////////////
// Orders & Purchases
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderValidationFromResponse(GSXmlStreamReader   theResponseXml, 
											  D2GOrderValidation *theValidation)

{
	GSResult   result = GS_SUCCESS;
	const char *tempIsValid,*tempMsg;
	int        tempValidCode, tempIsValidLen, tempMsgLen;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "ordervalidation")))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: ordervalidation\n",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "isvalid", &tempIsValid, &tempIsValidLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "message", &tempMsg, &tempMsgLen)) ||
		gsi_is_false(gsXmlReadChildAsInt   (theResponseXml, "orderstatuscode", &tempValidCode))
		)
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game,GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: orderstatuscode\n",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	theValidation->mIsValid = strncmp(tempIsValid, "true", tempIsValidLen) == 0 ? gsi_true : gsi_false;
	theValidation->mResult  = d2giOrderValidationResultFromCode(tempValidCode);
	theValidation->mMessage = UTF8ToUCS2StringAllocLen(tempMsg, tempMsgLen);

	if (theValidation->mIsValid == gsi_false && 
		theValidation->mMessage == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to convert to UCS2String, possibly out of memory",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
	}

	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderFromResponse(GSXmlStreamWriter theResponseXml, 
									D2GOrderInfo      *order)
{

	int     aAccountId;
	const char *tempSubTotal, *tempTax, *tempTotal, *tempGuid, *tempCultureCode, *tempCurrencyCode;
	int tempSubTotalLen, tempTaxLen, tempTotalLen, tempGuidLen, tempCultureCodeLen, tempCurrencyCodeLen;

	if (gsi_is_false(gsXmlReadChildAsInt   (theResponseXml, "accountid",  &aAccountId)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "culturecode",&tempCultureCode, &tempCultureCodeLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "currencycode",&tempCurrencyCode, &tempCurrencyCodeLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "subtotal",   &tempSubTotal, &tempSubTotalLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "tax",        &tempTax, &tempTaxLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "total",      &tempTotal, &tempTotalLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "rootorderid",&tempGuid, &tempGuidLen))
		)
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"s :: %s (%d): could not parse xml stream: rootorderid\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	order->mAccountId     = aAccountId;
	order->mRootOrderGuid = UTF8ToUCS2StringAllocLen(tempGuid, tempGuidLen);
	order->mGeoInfo.mCultureCode = UTF8ToUCS2StringAllocLen(tempCultureCode, tempCultureCodeLen);
	order->mGeoInfo.mCurrencyCode= UTF8ToUCS2StringAllocLen(tempCurrencyCode, tempCurrencyCodeLen);
	order->mSubTotal = UTF8ToUCS2StringAllocLen(tempSubTotal, tempSubTotalLen);
	order->mTax      = UTF8ToUCS2StringAllocLen(tempTax, tempTaxLen);
	order->mTotal    = UTF8ToUCS2StringAllocLen(tempTotal, tempTotalLen);


	if (order->mSubTotal == NULL || 
		order->mTax      == NULL || 
		order->mTotal    == NULL || 
		order->mGeoInfo.mCultureCode == NULL ||
		order->mGeoInfo.mCurrencyCode == NULL ||
		order->mRootOrderGuid == NULL)
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"s :: %s (%d): Failed to convert to UCS2String, out of memory",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
	}
	return d2giParseOrderValidationFromResponse(theResponseXml, &order->mValidation);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderTotalFromResponse(D2GIInstance  *theInstance,
										 D2GICatalog   *theCatalog,
										 GSXmlStreamWriter theResponseXml,
										 D2GOrderTotal *orderTotal)
{
	GSResult result;
	gsi_u32 i;

	result = d2giParseOrderFromResponse(theResponseXml, &orderTotal->mOrder);
	if (GS_FAILED(result))
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to parse D2GOrderInfo structure in d2giParseOrderFromResponse", 
			__FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	if (gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml, "datequoted", &orderTotal->mQuoteDate)))
	{
		gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"%s :: %s (%d): could not parse xml stream: datequoted\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderitemtotals")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, (int *)&orderTotal->mOrderItemList.mCount)))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"%s :: %s (%d): could not parse xml stream: count\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (orderTotal->mOrderItemList.mCount == 0)
	{
		orderTotal->mOrderItemList.mOrderItems = NULL;
		return GS_SUCCESS;
	}

	orderTotal->mOrderItemList.mOrderItems = (D2GOrderItem *)gsimalloc(sizeof(D2GOrderItem) * orderTotal->mOrderItemList.mCount);
	if (orderTotal->mOrderItemList.mOrderItems == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to allocate order items, out of memory", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
	}

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderitemtotal")))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"%s :: %s (%d): could not parse xml stream (moveto): orderitemtotal\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	for(i = 0; i < orderTotal->mOrderItemList.mCount; i++)
	{	
		result = d2giParseOrderItemFromResponse(theInstance, 
			theCatalog, 
			&orderTotal->mOrderItemList.mOrderItems[i], 
			theResponseXml);
		if (GS_FAILED(result))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to parse D2GOrderTotal structure in d2giParseOrderTotalFromResponse", 
				__FILE__, __FUNCTION__, __LINE__);
			break;
		}

		if (i < orderTotal->mOrderItemList.mCount - 1)
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "orderitemtotal")))
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
					"%s :: %s (%d): could not parse xml stream (movetosibling): orderitemtotal\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giParseOrderPurchaseFromResponse(D2GIInstance *theInstance,
											D2GICatalog  *theCatalog,
											GSXmlStreamWriter theResponseXml,
											D2GOrderPurchase *orderPurchase,
											gsi_bool		 updateManifest)
{
	GSResult    result = GS_SUCCESS;
	gsi_u32     i;

	result = d2giParseOrderFromResponse(theResponseXml, &orderPurchase->mOrder);

	if (GS_FAILED(result))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Failed to parse D2GOrderInfo structure in d2giParseOrderFromResponse");
		return result;
	}

	if (gsi_is_false(gsXmlReadChildAsDateTimeElement(theResponseXml,"datepurchased",&orderPurchase->mPurchaseDate)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError,"Could not parse xml stream: datequoted.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	//===============================================
	// Now we start parsing the order item purchases 
	//===============================================

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_ORDER_ITEM_PURCHASES)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT,
		(int *)&orderPurchase->mItemPurchases.mCount)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError,"Could not parse xml stream: %s, %s.", 
			GS_D2G_ORDER_ITEM_PURCHASES, GS_D2G_COUNT_ELEMENT);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	if (orderPurchase->mItemPurchases.mCount == 0)
	{
		orderPurchase->mItemPurchases.mOrderItemPurchases = NULL;
		return GS_SUCCESS;
	}

	orderPurchase->mItemPurchases.mOrderItemPurchases = (D2GOrderItemPurchase *)
		gsimalloc(sizeof(D2GOrderItemPurchase) * orderPurchase->mItemPurchases.mCount);

	if (orderPurchase->mItemPurchases.mOrderItemPurchases == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed to allocate order items, out of memory.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
	}

	//==================================================
	// Now we start parsing the each order item purchase
	//==================================================

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_ORDER_ITEM_PURCHASE)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml stream: %s.", GS_D2G_ORDER_ITEM_PURCHASE);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	//determine if we need to update Manifest 
	if ((updateManifest == gsi_true) &&
		(orderPurchase->mOrder.mValidation.mIsValid))
	{
		updateManifest =(GS_RESULT_CODE(orderPurchase->mOrder.mValidation.mResult) == D2GResultCode_Order_Ok);
	}

	for(i = 0; i < orderPurchase->mItemPurchases.mCount; i++)
	{	
		result = d2giParseOrderItemPurchaseFromResponse(theInstance,
			theCatalog,  
			&orderPurchase->mItemPurchases.mOrderItemPurchases[i], 
			theResponseXml,
			updateManifest);

		if (GS_FAILED(result))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed to parse D2GOrderItemPurchase from response.");
			return result;
		}

		if (i < orderPurchase->mItemPurchases.mCount - 1)
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GS_D2G_ORDER_ITEM_PURCHASE)))
			{
				GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_HotError, "Could not parse xml stream: %s.", GS_D2G_ORDER_ITEM_PURCHASE);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

	}
	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetOrderTotalResponse(D2GIInstance      *theInstance, 
										D2GICatalog       *theCatalog,
										GSXmlStreamWriter theResponseXml, 
										D2GGetOrderTotalResponse *theResponse)
{
	GSResult result;
	result = d2giParseGetResponse(theResponseXml, GS_D2G_GETORDERTOTAL_RESULT);

	if (GS_SUCCEEDED(result))
	{
		// move to response and start with the order object
		GSResult orderResult;
		if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "ordertotal")))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: GS_D2G_GETORDERTOTAL_RESPONSE",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		theResponse->mOrderTotal = (D2GOrderTotal *)gsimalloc(sizeof(D2GOrderTotal));
		memset(theResponse->mOrderTotal, 0, sizeof(D2GOrderTotal));
		if(theResponse->mOrderTotal == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to allocate D2GOrderTotal, out of memory",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_State, GSResultCode_OutOfMemory);
		}

		// go through the order total object and grab all the internal data
		orderResult = d2giParseOrderTotalFromResponse(theInstance, 
			theCatalog, 
			theResponseXml, 
			theResponse->mOrderTotal);
		if (GS_FAILED(orderResult))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed parse in d2giParseOrderFromResponse",
				__FILE__, __FUNCTION__, __LINE__);
			d2giFreeOrderTotal(theResponse->mOrderTotal);
			theResponse->mOrderTotal = NULL;
			return result;
		}
	}

	GSI_UNUSED(theInstance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseStartOrderResponse(D2GIInstance *theInstance,
									 D2GICatalog  *theCatalog, 
									 GSXmlStreamReader theResponseXml, 
									 D2GStartOrderResponse *response)
{
	GSResult result;

	result = d2giParseGetResponse(theResponseXml,GS_D2G_PURCHASEITEMS_RESULT);

	if (GS_SUCCEEDED(result))
	{

		GSResult orderResult;
		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderpurchase")))
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: orderpurchase\n",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		response->mOrderPurchase = (D2GOrderPurchase *)gsimalloc(sizeof(D2GOrderPurchase));
		memset(response->mOrderPurchase, 0, sizeof(D2GOrderPurchase));

		// getting order parsed with purchase gsi_true
		orderResult = d2giParseOrderPurchaseFromResponse(theInstance, 
			theCatalog, 
			theResponseXml, 
			response->mOrderPurchase,
			gsi_false);
		if (GS_FAILED(orderResult))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to parse order",
				__FILE__, __FUNCTION__, __LINE__);
			d2giFreeOrderPurchase(response->mOrderPurchase);
			response->mOrderPurchase = NULL;
			return orderResult;
		}
	}

	GSI_UNUSED(theInstance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseIsOrderCompleteResponse( D2GIInstance  *theInstance, 
										  D2GICatalog   *theCatalog,
										  GSXmlStreamReader theResponseXml, 
										  D2GIsOrderCompleteResponse *response)
{
	GSResult result;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_PLACEDORDER_RESULT);

	if (GS_SUCCEEDED(result))
	{
		GSResult orderResult;
		gsi_bool updateManifest = gsi_true;
		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderpurchase")))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream: GS_D2G_PLACEDORDER_RESPONSE",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		response->mOrderPurchase = (D2GOrderPurchase *)gsimalloc(sizeof(D2GOrderPurchase));
		if (response->mOrderPurchase == NULL)
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Out of memory when allocating: response->mOrderPurchase",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		memset(response->mOrderPurchase, 0, sizeof(D2GOrderPurchase));

		//this call will update the manifest file if the purchase is complated
		orderResult = d2giParseOrderPurchaseFromResponse(theInstance, 
			theCatalog, 
			theResponseXml, 
			response->mOrderPurchase,
			updateManifest);
		if (GS_FAILED(orderResult))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Failed to parse order",
				__FILE__, __FUNCTION__, __LINE__);
			d2giFreeOrderPurchase(response->mOrderPurchase);
			response->mOrderPurchase = NULL;
			return orderResult;
		}
	}

	GSI_UNUSED(theInstance);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giParseGetPurchaseHistoryResponse
//
GSResult d2giParseGetPurchaseHistoryResponse( D2GIInstance       *theInstance,
											 D2GICatalog         *theCatalog, 
											 GSXmlStreamReader   theResponseXml, 
											 D2GGetPurchaseHistoryResponse *response)
{
	GSResult result;
	gsi_u32  i;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_GETPURCHASEHISTORY_RESULT);

	if (GS_SUCCEEDED(result))
	{

		response->mPurchaseHistory = (D2GPurchaseHistory *)gsimalloc(sizeof(D2GPurchaseHistory));
		if (response->mPurchaseHistory == NULL)
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): Out of memory when allocating: response->mPurchaseHistory",
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		}

		// clear this out since we want to have count = 0 and purchases = NULL for cleanup.
		response->mPurchaseHistory->mCount = 0;
		response->mPurchaseHistory->mPurchases = NULL;

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml,    "orderpurchases")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", (int *)&response->mPurchaseHistory->mCount)))
		{
			result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			return result;
		}

		if (response->mPurchaseHistory->mCount == 0)
		{
			// we've already nulled the purchases and just need to return success.
			return result;
		}
		else
		{
			response->mPurchaseHistory->mPurchases = (D2GOrderPurchase *)
				gsimalloc(sizeof(D2GOrderPurchase) * response->mPurchaseHistory->mCount);
			memset(response->mPurchaseHistory->mPurchases, 0, sizeof(D2GOrderPurchase) * response->mPurchaseHistory->mCount);
		}

		for (i=0 ; i < response->mPurchaseHistory->mCount ; i++)
		{
			// Now go through each purchase order

			GSResult orderResult;
			if (i == 0)
			{
				if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "orderpurchase")))
				{
					gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream: orderpurchase\n", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}
			else
			{
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "orderpurchase")))
				{
					gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream: orderpurchase\n", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}

			orderResult = d2giParseOrderPurchaseFromResponse(theInstance, 
				theCatalog, 
				theResponseXml, 
				&response->mPurchaseHistory->mPurchases[i],
				gsi_false);
			if (GS_FAILED(orderResult))
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): Failed to parse order\n", 
					__FILE__, __FUNCTION__, __LINE__);
				return orderResult;
			}
		}
		if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
	}
	else
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): Failed to parse D2GOrderInfo structure in d2giParseGetPurchaseHistoryResponse\n", 
			__FILE__, __FUNCTION__, __LINE__);
	}

	GSI_UNUSED(theInstance);
	return result;
}


///////////////////////////////////////////////////////////////////////////////
// Download Info        
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseDownloadHeaderFromResponse(GSXmlStreamReader   theResponseXml,  
											 gsi_char            *userHeader)
{
	GSResult result = GS_SUCCESS;

	int     tempKeyLen   = 0,
		tempValueLen = 0;

	const char  *tempKey   = NULL, 
		*tempValue = NULL;

	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "key",  &tempKey, &tempKeyLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "value",&tempValue, &tempValueLen)))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: value\n",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	strncat(userHeader,tempKey, tempKeyLen);
	strncat(userHeader,":", 1);
	strncat(userHeader,tempValue, tempValueLen);
	strcat(userHeader,CRLF);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseDownloadUserHeadersFromResponse(GSXmlStreamReader   theResponseXml, 
												  gsi_char            **userHeader)
{
	GSResult result = GS_SUCCESS;
	gsi_u32  nHeaderCount = 0;
	gsi_u32  i = 0;

	if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "urlrequestheaderlist")) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", (int*)&nHeaderCount)))
	{
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		return result;
	}

	if (nHeaderCount == 0)
	{
		*userHeader = NULL;
		return result;
	}
	else
	{
		int nMaxUserHeaderLen = 4096;

		*userHeader = gsimalloc(sizeof(gsi_char) * nMaxUserHeaderLen + 1);
		memset(*userHeader, '\0', (sizeof(gsi_char) * nMaxUserHeaderLen + 1));

		for (i=0 ; i < nHeaderCount ; i++)
		{
			if (i == 0)
			{
				if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "urlrequestheader")))
				{
					gsDebugFormat(  GSIDebugCat_Direct2Game, 
						GSIDebugType_Misc, 
						GSIDebugLevel_WarmError,
						"d2giParseDownloadUserHeadersFromResponse: could not \
						parse xml stream: urlrequestheader\n");
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}
			else
			{
				if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "urlrequestheader")))
				{
					gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream: urlrequestheader\n",
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}
			}

			result = d2giParseDownloadHeaderFromResponse(theResponseXml, *userHeader);

			if (GS_FAILED(result))
			{
				gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse xml stream: d2giParseDownloadUserHeadersFromResponse\n",
					__FILE__, __FUNCTION__, __LINE__);
				return result;
			}
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// Extra Info    
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseExtraInfo(D2GIInstance        *theInstance, 
										D2GICatalog         *theCatalog, 
										GSXmlStreamReader   theResponseXml,  
										D2GExtraInfo        *theExtraInfo)
{
	GSResult result = GS_SUCCESS;
	const char *aKey, *aValue;
	int aKeyLen, aValueLen;
	
	if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "key",  &aKey, &aKeyLen)) ||
		gsi_is_false(gsXmlReadChildAsString(theResponseXml, "value",&aValue, &aValueLen)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream: value.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	// First Check if we have it in the cache
	// If not add it. If we do then update it.
	theExtraInfo->mKey   = UTF8ToUCS2StringAllocLen(aKey,   aKeyLen);
	theExtraInfo->mValue = UTF8ToUCS2StringAllocLen(aValue, aValueLen);

	if (theExtraInfo->mKey == NULL || theExtraInfo->mValue == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory: theExtraInfo->mValue, theExtraInfo->mKey.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	if (theCatalog)
	{
		D2GExtraInfo *pExtraInfo;
		// Update the cache with the key,value pair.
		pExtraInfo = d2giGetExtraInfo(theInstance, theCatalog, theExtraInfo->mKey);
		if (pExtraInfo == NULL)
		{
			pExtraInfo = d2giNewExtraInfo(theInstance, theCatalog, theExtraInfo);
		}
		else 
		{
			d2giResetExtraInfo(pExtraInfo);
			pExtraInfo->mKey   = theExtraInfo->mKey;
			pExtraInfo->mValue = theExtraInfo->mValue;
		}
	}
	
	GSI_UNUSED(theInstance);
	return result;
}

GSResult d2giParseExtraInfoList(D2GIInstance     *theInstance, 
								D2GICatalog      *theCatalog, 
								GSXmlStreamReader theResponseXml, 
								D2GExtraInfoList *extraInfoList)
{
	gsi_u32 i;
	GSResult result = GS_SUCCESS;
	if (gsi_is_false(gsXmlMoveToNext(theResponseXml, GS_D2G_ETRA_INFO_LIST)) ||
		gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "count", 
		(int *)&extraInfoList->mCount)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Could not parse xml stream: extrainfolist.");
		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		return result;
	}

	if (extraInfoList->mCount == 0)
	{
		extraInfoList->mExtraInfoElements = NULL;
		return result;
	}
	else
	{
		extraInfoList->mExtraInfoElements = (D2GExtraInfo *) gsimalloc(sizeof(D2GExtraInfo)* extraInfoList->mCount);
		memset(extraInfoList->mExtraInfoElements, 0, sizeof(D2GExtraInfo) * extraInfoList->mCount);
	}

	for (i=0 ; i < extraInfoList->mCount ; i++)
	{
		if (i == 0)
		{
			if (gsi_is_false(gsXmlMoveToChild(theResponseXml, GS_D2G_EXTRA_INFO)))
			{
				GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse xml stream: %s", GS_D2G_EXTRA_INFO);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}
		else
		{
			if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, GS_D2G_EXTRA_INFO)))
			{
				GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse xml stream: %s", GS_D2G_EXTRA_INFO);
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}

		result = d2giParseExtraInfo(theInstance, 
			theCatalog, 
			theResponseXml, 
			&extraInfoList->mExtraInfoElements[i]);
		if (GS_FAILED(result))
		{
			GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not parse xml stream: %s", GS_D2G_EXTRA_INFO);
			return result;
		}
	}
	if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}

	return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseLoadExtraCatalogInfoResponse(D2GIInstance  *theInstance,
											   D2GICatalog         *theCatalog,  
											   GSXmlStreamReader   theResponseXml, 
											   D2GLoadExtraCatalogInfoResponse *response)
{
	GSResult result;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_GET_STORE_EXTENSION_DATA_RESULT);
	if (GS_SUCCEEDED(result))
	{
		response->mExtraCatalogInfoList = (D2GExtraInfoList *)gsimalloc(sizeof(D2GExtraInfoList));
		if (response->mExtraCatalogInfoList == NULL)
		{
			GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Unable to allocate D2GExtraInfoList.");
			result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			return result;
		}

		// clear this array otherwise the free function will fail since
		// it's not clean memory
		memset(response->mExtraCatalogInfoList, 0, sizeof(D2GExtraInfoList));
		
		result = d2giParseExtraInfoList(theInstance, theCatalog, theResponseXml, response->mExtraCatalogInfoList);
	}
	else
	{
		GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "Failed to parse D2GExtraItem structure in d2giParseGetExtraInfoResponse");
	}

	GSI_UNUSED(theInstance);
	return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetItemActivationResponse(D2GIInstance        *theInstance,
											D2GICatalog         *theCatalog,  
											GSXmlStreamReader   theResponseXml, 
											D2GGetItemActivationResponse *response)


{
	GSResult result = GS_SUCCESS;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_GETITEMACTIVATIONDATA_RESULT);

	if (GS_SUCCEEDED(result))
	{
		const char *activationCode;
		int activationCodeLen;
		if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "itemid", (int *)&response->mItemId)))
		{
			result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			return result;
		}

		gsXmlReadChildAsString(theResponseXml, "activationcode",&activationCode, &activationCodeLen);

		if (activationCodeLen > 0)
		{
			response->mActivationCode = UTF8ToUCS2StringAllocLen(activationCode,activationCodeLen);
		}
		else
		{
			response->mActivationCode = NULL;
		}
	}
	GSI_UNUSED(theInstance);
	GSI_UNUSED(theCatalog);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseCheckContentUpdates(D2GIInstance        *theInstance,
									  D2GICatalog         *theCatalog,  
									  GSXmlStreamReader   theResponseXml, 
									  D2GCheckContentUpdatesResponse *response)

{
	GSResult result = GS_SUCCESS;

	result = d2giParseGetResponse(theResponseXml, GS_D2G_CHECKFORCONTENTUPDATES_RESULT);

	if (GS_SUCCEEDED(result))
	{
		response->mContentUpdateList = (D2GContentUpdateList *)gsimalloc(sizeof(D2GContentUpdateList));
		if (response->mContentUpdateList == NULL)
		{
			gsDebugFormat(  GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
				"%s :: %s (%d): unable to allocate D2GDownloadList\n",
				__FILE__, __FUNCTION__, __LINE__);
			result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			return result;
		}

		if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "downloaditemupdates")) ||
			gsi_is_false(gsXmlReadChildAsInt(theResponseXml, GS_D2G_COUNT_ELEMENT, (int *)&response->mContentUpdateList->mCount)))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"%s :: %s (%d): could not parse xml stream (move): downloads\n", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}

		if (response->mContentUpdateList->mCount == 0)
		{
			response->mContentUpdateList->mContentUpdates = NULL;
		}
		else
		{	
			gsi_u32 i = 0;

			if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "download")))
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): could not parse xml stream: license\n", 
					__FILE__, __FUNCTION__, __LINE__);
				result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
			response->mContentUpdateList->mContentUpdates = (D2GContentUpdate *)gsimalloc(sizeof(D2GContentUpdate) * response->mContentUpdateList->mCount);
			if (response->mContentUpdateList->mContentUpdates == NULL)
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
					"%s :: %s (%d): Failed to allocate downloads, out of memory", 
					__FILE__, __FUNCTION__, __LINE__);
				return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			}

			for (i = 0; i < response->mContentUpdateList->mCount; i++)
			{
				const char *aName, *anAssetType, *isRequired;
				int aNameLen, anAssetTypeLen, isRequiredLen;
				D2GContentUpdate *currentItem = &response->mContentUpdateList->mContentUpdates[i];

				if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "itemid", (int *)&currentItem->mItemId))||
					gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "downloadurlid", (int *)&currentItem->mDownloadItem.mUrlId))||
					gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "changetype", (int *)&currentItem->mChangeType)) ||
					gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "fileid", (int *)&currentItem->mDownloadItem.mFileId)) ||
					gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "sequence", (int *)&currentItem->mDownloadItem.mSequence)) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "name", &aName, &aNameLen)) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "assettype", &anAssetType, &anAssetTypeLen)) ||
					gsi_is_false(gsXmlReadChildAsFloat(theResponseXml, "version",&currentItem->mDownloadItem.mVersion)) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "requirementlevelcode",&isRequired, &isRequiredLen)))
				{
					gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): could not parse xml stream: assettype\n", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
				}

				currentItem->mDownloadItem.mName = UTF8ToUCS2StringAllocLen(aName, aNameLen);
				if (currentItem->mDownloadItem.mName == NULL)
				{
					gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
						"%s :: %s (%d): Failed to convert to UCS2String, out of memory", 
						__FILE__, __FUNCTION__, __LINE__);
					return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
				}

				currentItem->mDownloadItem.mAssetType = UTF8ToUCS2StringAllocLen(anAssetType, anAssetTypeLen);

				// check if the download is optional if not set mRequired to true
				if (strncmp("MANDATORY", isRequired, isRequiredLen) ==0)
				{
					currentItem->mDownloadType = D2GDownloadMandatory ; 
				}
				else if (strncmp("FORCED", isRequired, isRequiredLen) == 0)
				{
					currentItem->mDownloadType = D2GDownloadForced ; 
				}
				else 
				{
					currentItem->mDownloadType = D2GDownloadOptional ;
				}

				if (i < response->mContentUpdateList->mCount - 1)
				{	
					if (gsi_is_false(gsXmlMoveToSibling(theResponseXml, "download")))
					{
						gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
							"%s :: %s (%d): could not parse xml stream (move): download\n", 
							__FILE__, __FUNCTION__, __LINE__);
						return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
					}
				}
			}
			if (gsi_is_false(gsXmlMoveToParent(theResponseXml)))
			{
				GS_D2G_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError,"Could not move to parent.");
				return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
			}
		}// else    
	} // if (GS_SUCCEEDED(result))
	GSI_UNUSED(theInstance);
	GSI_UNUSED(theCatalog);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giParseManifestRecord
// 
GSResult d2giParseManifestRecord(char *buffer,
								 D2GContentUpdate *theContentUpdate)
{
	GSResult result = GS_SUCCESS;
	GSXmlStreamReader *reader ; 

	memset(theContentUpdate, 0 , sizeof(D2GContentUpdate));

	GS_ASSERT(buffer);
	if ((buffer == NULL) || (theContentUpdate == NULL))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
			"%s :: %s (%d): Null pointer passed\n", 
			__FILE__, __FUNCTION__, __LINE__);

		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_InvalidParameters);

	}
	reader = gsXmlCreateStreamReader();

	// now create parse-able xml string form a string
	// convert to xml string
	if(gsi_is_false(gsXmlParseBuffer(reader, buffer, (int)strlen(buffer))))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"%s :: %s (%d): could parse buffer into an xml string \n", 
			__FILE__, __FUNCTION__, __LINE__);

		result = GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_InvalidParameters);

	}
	else
	{
		// now we need to parse it
		if (gsi_is_false(gsXmlMoveToStart(reader)) ||
			gsi_is_false(gsXmlMoveToNext(reader, "download")))
		{
			gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError, 
				"%s :: %s (%d): could not parse xml: start; tag %s", 
				__FILE__, __FUNCTION__, __LINE__, "download");
			return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
		}
		result = d2giParseDownload(reader, theContentUpdate);
	}
	gsXmlFreeReader(reader);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giParseAndDecodeManifestRecord
// 
GSResult d2giParseAndDecodeManifestRecord(char *manifestRead,
										  D2GContentUpdate *contentUpdate)
{
	GSResult   result;
	D2GIString manifestXml;    

	manifestXml.str = gsimalloc((strlen(manifestRead)+2) * sizeof(char));

	memset (manifestXml.str, '\0',((strlen(manifestRead)+2)*sizeof(char)) );
	manifestXml.len = (int)strlen(manifestXml.str);

	// Uses URL safe encoding   
	B64Decode(manifestRead, manifestXml.str, (int)strlen(manifestRead), &manifestXml.len, 2);
	result = d2giParseManifestRecord(manifestXml.str, contentUpdate);
	gsifree(manifestXml.str);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giParseDownload
// 
GSResult d2giParseDownload(GSXmlStreamReader   *reader,
						   D2GContentUpdate    *contentUpdate)
{

	if (gsi_is_false(gsXmlReadChildAsInt(reader,  "itemid",        (int *)&contentUpdate->mItemId)) ||
		gsi_is_false(gsXmlReadChildAsInt(reader,  "downloadurlid", (int *)&contentUpdate->mDownloadItem.mUrlId)) ||
		gsi_is_false(gsXmlReadChildAsFloat(reader,"version",       (float *)&contentUpdate->mDownloadItem.mVersion)))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"%s :: %s (%d): could not parse xml stream: version\n", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_Soap, GSResultCode_CouldNotParseXML);
	}
	return GS_SUCCESS;
}
