///////////////////////////////////////////////////////////////////////////////
// GameSpy Direct2Game SDK
// This file is part of the Direct to Game SDK designed and developed by 
// GameSpy Tech.
// Copyright (c) 2008, GameSpy Technology
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// includes
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "Direct2Game.h"
#include "d2gMain.h"
#include "d2gServices.h"
#include "d2gUtil.h"
#include "d2gDeserialize.h"
#include "d2gDownloads.h"
#include <sys/stat.h>
#include "stddef.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giResultFromStoreAvailabilityCode
//
GSResult d2giResultFromStoreAvailabilityCode(int availableCode)
{
    GSResult result;
    switch(availableCode)
    {
    case 10:
        {
            result = GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_StoreOnline);
            break;
        }
    case 20:
        {
            result = GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_StoreOfflineForMaintenance);
            break;
        }
    case 50:
        {	
            result = GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_StoreOfflineRetired);
            break;
        }
    case 100:
        {
            result = GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_StoreNotYetLaunched);
            break;
        }
    default: 
        if (((~GS_RESULT_CODEBITS)&availableCode) == 0)
        {
            // report the error code as it is.
            result = GS_RESULT(0, GSResultSDK_Unknown, GSResultSection_NULL, availableCode);
        }
        else
        {
            result = GS_D2G_SUCCESS(GSResultSection_NULL, GSResultCode_UnknownError);
        } 
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void d2giSetServiceUrl(D2GIInstance *theInstance, 
                       gsi_bool secure, 
                       const char *service)
{

#ifdef UNISPY_FORCE_IP
	if (theInstance->mServiceURLBase == NULL)
	{
        // allocate memory and initialize it
        theInstance->mServiceURLBase = gsimalloc(GS_D2G_MAX_BASE_URL_LEN);
        snprintf(theInstance->mServiceURLBase, GS_D2G_MAX_BASE_URL_LEN, "%s/commerce/1.1", UNISPY_FORCE_IP);
	}
#else
    if (theInstance->mServiceURLBase == NULL)
    {
        // allocate memory and initialize it
        theInstance->mServiceURLBase = gsimalloc(GS_D2G_MAX_BASE_URL_LEN);
#ifdef GS_D2G_STAGE
        snprintf(theInstance->mServiceURLBase, GS_D2G_MAX_BASE_URL_LEN, GS_D2G_SERVICE_URL_BASE);
#else
        snprintf(theInstance->mServiceURLBase, GS_D2G_MAX_BASE_URL_LEN, GS_D2G_SERVICE_URL_BASE, __GSIACGamename);
#endif
    }
#endif

    if (theInstance->mServiceURL == NULL)
    {
        theInstance->mServiceURL = gsimalloc(GS_D2G_MAX_URL_LEN);
    }

	if (secure)
		snprintf(theInstance->mServiceURL, GS_D2G_MAX_URL_LEN, "%s%s%s", GS_D2G_SECURE_HTTP, theInstance->mServiceURLBase, service);
	else
		snprintf(theInstance->mServiceURL, GS_D2G_MAX_URL_LEN, "%s%s%s", GS_D2G_HTTP, theInstance->mServiceURLBase, service);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giResultFromStatusCode(int statusCode)
{
	GSResult result;
	switch(statusCode)
	{
	case -1: 
	    result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_ServiceUninitialized);
        break;
	case 0: 
		result = GS_D2G_SUCCESS(GSResultSection_NULL,GSResultCode_Success);
		break;
	case 1:
		result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_ServerError);
		break;
	case 2:
		result = GS_D2G_ERROR(GSResultSection_Network, GSResultCode_ServerError);
		break;
	case 3:
		result = GS_D2G_ERROR(GSResultSection_Account, GSResultCode_InvalidCertificate);
		break;
	case 101:
		result = GS_D2G_ERROR(GSResultSection_Account, GSResultCode_AccountNotFound);
		break;
	case 102:
		result = GS_D2G_ERROR(GSResultSection_Account, GSResultCode_UnknownError);
		break;
	case 201:
		result = GS_D2G_ERROR(GSResultSection_NULL, GSResultCode_GameNotFound);
		break;
	case 202:
		result = GS_D2G_ERROR(GSResultSection_NULL, GSResultCode_NotFound);
		break;
	case 203:
		result = GS_D2G_SUCCESS(GSResultSection_NULL, GSResultCode_Success);
		break;
	case 301:
		result = GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_InsufficientFunds);
		break;
	case 302:
		result = GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_NoPaymentMethodFound);
		break;
	default:
	    // unsupported error code
		if (((~GS_RESULT_CODEBITS)&statusCode) == 0)
		{
		    // report the error code as it is.
            result = GS_ERROR(GSResultSDK_Unknown, GSResultSection_NULL, statusCode);
		}
		else
		{
		    result = GS_D2G_ERROR(GSResultSection_NULL, GSResultCode_UnknownError);
		} 
		break;
	};

	return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giResultFromHttpResult(GHTTPResult result)
{
	switch(result)
	{
		case GHTTPSuccess:
			return GS_D2G_SUCCESS(GSResultSection_NULL, GSResultCode_Success);
		case GHTTPOutOfMemory:
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
		case GHTTPSocketFailed:
			return GS_D2G_ERROR(GSResultSection_Network, GSResutlCode_SocketError);
		case GHTTPFileWriteFailed:
			return GS_D2G_ERROR(GSResultSection_State, GSResultCode_FileWriteError);
		case GHTTPFileToBig:
			return GS_D2G_ERROR(GSResultSection_State, GSResultCode_FileTooLarge);
		case GHTTPForbidden:
		    return GS_D2G_ERROR(GSResultSection_Network, GSResultCode_HttpForbidden);
		default:
            // unsupported error code
            if (((~GS_RESULT_CODEBITS)&result) == 0)
            {
                // report the error code as it is.
                return GS_ERROR(GSResultSDK_Unknown, GSResultSection_Network, result);
            }
            else
            {
                return GS_D2G_ERROR(GSResultSection_Network, GSResultCode_HttpError);
            } 
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giResultFromHttpRequest(GHTTPRequest request)
{
    switch(request)
    {
    case 0:
        return GS_D2G_SUCCESS(GSResultSection_NULL, GSResultCode_Success);

    case GHTTPInvalidURL:
        return GS_D2G_ERROR(GSResultSection_Network, GSResultCode_BadUrl);

    case GHTTPInvalidFileName:
        return GS_D2G_ERROR(GSResultSection_File, GSResultCode_InvalidParameters);

    case GHTTPInsufficientMemory:
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);

    case GHTTPInvalidPost:
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_HttpError);

    case GHTTPFailedToOpenFile:
        return GS_D2G_ERROR(GSResultSection_File, GSResultCode_FileFailedToOpen);

    default:
        // unsupported error code
        if (((~GS_RESULT_CODEBITS)&request) == 0)
        {
            // report the error code as it is.
            return GS_ERROR(GSResultSDK_Unknown, GSResultSection_NULL, request);
        }
        else
        {
            return GS_D2G_ERROR(GSResultSection_SdkSpecifc, GSResultCode_UnknownError);
        } 
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giOrderValidationResultFromCode(int orderCode)
{
	switch(orderCode)
	{
	case 1:
		return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_Order_Ok);
	case 20:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_OrderInitiateFailed);
	case 40:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UserNotAuthorized);
	case 41:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UserNotFound);
	case 42:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UserAccountNotFound);
	case 60:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_BillingFailed);
	case 61:
		return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_Order_BillingPending);		
	case 62:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_BillingRequestNotFound);
    case 63:
        return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_InsufficientFunds);
	case 80:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_OrderItemFailed);
	case 100:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_RequiredFieldNotSet);
	case 1000:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UnexepectedError);
	default:
		// Note the change from success to error. 
        if (((~GS_RESULT_CODEBITS)&orderCode) == 0)
        {
            // report the error code as it is.
            return GS_ERROR(GSResultSDK_Unknown, GSResultSection_NULL, orderCode);
        }
        else
        {
            return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UnknownResultCode);
        } 
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giOrderItemValidationResultFromCode(int orderItemCode)
{
	switch(orderItemCode)
	{
	case 1:
		return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_Ok);
	case 10:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_CountryRestriction);
	case 11:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_AgeRestriction);
	case 12:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_QuantityRestriction);
	case 22:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_VariantDoesNotExist);
	case 23:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_ItemDoesNotExist);
	case 29:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_UnspecificedRestriction);
	case 30:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_ItemNotAuthorized);
	case 31:
		return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_PriceChanged);
	case 32:
		return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_ItemRefunded);
	case 1000:
		return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_Order_UnexepectedError);
	default:
        if (((~GS_RESULT_CODEBITS)&orderItemCode) == 0)
        {
            // report the error code as it is.
            return GS_ERROR(GSResultSDK_Unknown, GSResultSection_NULL, orderItemCode);
        }
        else
        {
            return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_UnknownResultCode);
        } 
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giResultFromDownloadStatusCode(int statusCode)
{
    switch(statusCode)
    {
    case 1:
        return GS_D2G_SUCCESS(GSResultSection_SdkSpecifc, GSResultCode_Success);
    case 220:
        return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_File_NotOwnedByUser);
    default:
        if (statusCode>>GS_RESULT_CODEBITS == 0)
        {
            // report the error code as it is.
            return GS_ERROR(GSResultSDK_Unknown, GSResultSection_NULL, statusCode);
        }
        else
        {
            return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_OrderItem_UnknownResultCode);
        } 

    }

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Order total request will have to write the items requested into the soap request payload
gsi_bool d2giWriteCachedItemsToXml(D2GIInstance      *theInstance, 
                                   D2GICatalog       *theCatalog,
								   GSXmlStreamWriter writer, 
								   gsi_u32           accountId, 
								   UCS2String        cultureCode,
								   UCS2String        currencyCode, 
								   gsi_u32           *itemIds, 
								   int               itemCount, 
								   gsi_u32           *itemQuantities)
{
	int i;

	GS_ASSERT(theInstance);
	GS_ASSERT(itemIds);
	GS_ASSERT(writer);
	GS_ASSERT(itemCount);
	GS_ASSERT(itemQuantities);
	GS_ASSERT(currencyCode);
	GS_ASSERT(currencyCode);

	if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "order")) || 
		gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "accountid", accountId)) ||
		gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "culturecode", cultureCode)) ||
		gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "currencycode", currencyCode)) ||
		gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "subtotal", "0")) ||
        gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "tax", "0")) ||
		gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "total", "0")) ||
		//gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "orderitemcount", itemCount))
		gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "orderitems")) ||
		gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, GS_D2G_COUNT_ELEMENT, itemCount)))
	{
		return gsi_false;
	}
	
	for (i = 0; i < itemCount; i++)
	{
		D2GCatalogItem *itemToSerialize;

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "orderitem")))
		{
			return gsi_false;
		}

		itemToSerialize = d2giGetCatalogItem(theInstance, theCatalog, itemIds[i]);
		if (itemToSerialize == NULL)
			return gsi_false;

		if (gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "itemid", itemToSerialize->mItem.mItemId)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "externalid", itemToSerialize->mItem.mExternalItemCode)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "name", itemToSerialize->mItem.mName)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "price", itemToSerialize->mItem.mPrice)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "tax", itemToSerialize->mItem.mTax)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "quantity", itemQuantities[i])) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "subtotal", "0")) ||
			gsi_is_false(gsXmlWriteStringElement(writer, GS_D2G_NAMESPACE, "total", "0")))
		{
			return gsi_false;
		}
		if (gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "orderitem")))
		{
			return gsi_false;
		}		
	}
	if (gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "orderitems")))
	{
		return gsi_false;
	}
	if (gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "order")))
	{
		return gsi_false;
	}
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giWriteOrderTotalToXml
//  Order total request will have to write the items requested into 
//  the soap request payload
//
gsi_bool d2giWriteOrderTotalToXml(D2GIInstance        *theInstance, 
                                  GSXmlStreamWriter   writer, 
                                  const D2GOrderTotal *theOrderTotal)
{
	gsi_u32 i;

	GS_ASSERT(theInstance);
	GS_ASSERT(writer);
	GS_ASSERT(theOrderTotal);

	if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "order")) || 
		gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "accountid", theOrderTotal->mOrder.mAccountId)) ||
		gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "culturecode", theOrderTotal->mOrder.mGeoInfo.mCultureCode)) ||
		gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "currencycode", theOrderTotal->mOrder.mGeoInfo.mCurrencyCode)) ||
        gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "subtotal", theOrderTotal->mOrder.mSubTotal)) ||		        
        gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "tax", theOrderTotal->mOrder.mTax)) ||
		gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "total", theOrderTotal->mOrder.mTotal)) ||
		gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "orderitems")) ||
		gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, GS_D2G_COUNT_ELEMENT, theOrderTotal->mOrderItemList.mCount)))
	{
		return gsi_false;
	}

	for (i = 0; i < theOrderTotal->mOrderItemList.mCount; i++)
	{
		D2GOrderItem *itemToSerialize = &theOrderTotal->mOrderItemList.mOrderItems[i];

		if (gsi_is_false(gsXmlWriteOpenTag(writer, GS_D2G_NAMESPACE, "orderitem")))
		{
			return gsi_false;
		}

		if (itemToSerialize == NULL)
			return gsi_false;

		if (gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "itemid", itemToSerialize->mItem.mItemId)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "externalid", itemToSerialize->mItem.mExternalItemCode)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "name", itemToSerialize->mItem.mName)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "price", itemToSerialize->mItem.mPrice)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "tax", itemToSerialize->mItem.mTax)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "quantity", itemToSerialize->mItemTotal.mQuantity)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "subtotal", itemToSerialize->mItemTotal.mSubTotal)) ||
			gsi_is_false(gsXmlWriteUnicodeStringElement(writer, GS_D2G_NAMESPACE, "total", itemToSerialize->mItemTotal.mTotal)))
		{
			return gsi_false;
		}

		if (gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "orderitem")))
		{
			return gsi_false;
		}		
	}

	if (gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "orderitems")) ||
		gsi_is_false(gsXmlWriteCloseTag(writer, GS_D2G_NAMESPACE, "order")))
	{
		return gsi_false;
	}
	GSI_UNUSED(theInstance);
	return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
// Catalog Helper Functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giNewCatalogItem
//  This is a helper function, given an item id, it returns a pointer to 
//  the new item's location in the catalog.
//
D2GCatalogItem * d2giNewCatalogItem(D2GIInstance *theInstance, 
                                    D2GICatalog  *theCatalog,
                                    D2GItemId    itemId)
{
	D2GCatalogItem item;

	GS_ASSERT(theInstance != NULL);
	GS_ASSERT(theCatalog  != NULL);
	if (theInstance == NULL || theCatalog == NULL)
		return NULL;

	memset(&item, 0, sizeof(D2GCatalogItem));

	item.mItem.mItemId = itemId;
	ArrayAppend(theCatalog->mItemArray, &item);

	return (D2GCatalogItem *)ArrayNth(theCatalog->mItemArray, ArrayLength(theCatalog->mItemArray)-1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giGetCatalogItem
//  This is a helper function, given an item id, 
//  to retrieve a catalog item from the catalog saved in the cache.
//
D2GCatalogItem * d2giGetCatalogItem(D2GIInstance *theInstance, 
                                    D2GICatalog  *theCatalog,
                                    gsi_u32      itemid)
{
	int index=0;
	for (index=0; index < ArrayLength(theCatalog->mItemArray); index++)
	{
		D2GCatalogItem * info = (D2GCatalogItem *)ArrayNth(theCatalog->mItemArray, index);
		if (info->mItem.mItemId == itemid)
			return info;
	}
    GSI_UNUSED(theInstance);
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeCatalogItem
//      This is a helper function to remove a catalog item
//      from the catalog saved in the cache.
//
GSResult  d2giFreeCatalogItem(D2GICatalog      *theCatalog, 
                              D2GCatalogItem   *item)
{
	int i=0;
	GS_ASSERT(theCatalog != NULL);
	GS_ASSERT(theCatalog->mItemArray != NULL);
	if (theCatalog == NULL || theCatalog->mItemArray == NULL || ArrayLength(theCatalog->mItemArray) == 0)
	{
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "The ItemArray is null or is empty.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	if (item == NULL)
	{
		GS_D2G_LOG(GSIDebugType_State,GSIDebugLevel_WarmError, "Called with a null item.");
		return GS_SUCCESS; 
	}

	for (i=0; i < ArrayLength(theCatalog->mItemArray); i++)
	{
		D2GCatalogItem * elem = (D2GCatalogItem *) ArrayNth(theCatalog->mItemArray, i);
		if (elem->mItem.mItemId == item->mItem.mItemId)
		{
			ArrayDeleteAt(theCatalog->mItemArray, i);
			break;
		}
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giResetCatalogItem
//      Clears the contents of a given catalog item.  
//
GSResult d2giResetCatalogItem(D2GCatalogItem * catalogItem)
{
	gsi_u32 i;
	GS_ASSERT(catalogItem != NULL);
	if (catalogItem == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Misc,GSIDebugLevel_HotError,"Received NULL input parameter.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_ArgumentNull);
	}

    // Free D2GBasicItemInfo Contents
	gsifree(catalogItem->mItem.mExternalItemCode);
	gsifree(catalogItem->mItem.mName);
	gsifree(catalogItem->mItem.mPrice);
	gsifree(catalogItem->mItem.mTax);
    catalogItem->mItem.mItemId = 0;
    catalogItem->mItem.mExternalItemCode = NULL;
    catalogItem->mItem.mName  = NULL;
    catalogItem->mItem.mPrice = NULL;
    catalogItem->mItem.mTax   = NULL;

    // Free D2GGeoInfo
    gsifree(catalogItem->mGeoInfo.mCultureCode);
    gsifree(catalogItem->mGeoInfo.mCurrencyCode);
    catalogItem->mGeoInfo.mCultureCode  = NULL;
    catalogItem->mGeoInfo.mCurrencyCode = NULL;
    
	// Free D2GProductInfo Contents
	catalogItem->mProductInfo.mReleaseDate = 0;
	gsifree(catalogItem->mProductInfo.mPublisher);
	gsifree(catalogItem->mProductInfo.mDeveloper);	
	gsifree(catalogItem->mProductInfo.mSummary);

	catalogItem->mProductInfo.mPublisher = NULL;
	catalogItem->mProductInfo.mDeveloper = NULL;
	catalogItem->mProductInfo.mSummary   = NULL;

    // Free D2GImageList Contents
	for(i = 0; i < catalogItem->mImages.mCount; i++)
	{
		gsifree(catalogItem->mImages.mImageName[i]);
		catalogItem->mImages.mImageName[i] = NULL;
	}
	gsifree(catalogItem->mImages.mImageName);
	catalogItem->mImages.mImageName = NULL;
	catalogItem->mImages.mCount = 0;

    // Free D2GCategoryList Contents
	for(i = 0; i < catalogItem->mCategories.mCount; i++)
	{
		gsifree(catalogItem->mCategories.mCategoryNames[i]);
		catalogItem->mCategories.mCategoryNames[i] = NULL;
	}
	gsifree(catalogItem->mCategories.mCategoryNames);
	catalogItem->mCategories.mCategoryNames  = NULL;
	catalogItem->mCategories.mCount = 0;

	for (i = 0; i < catalogItem->mExtraItemInfoList.mCount; i++)
	{
		gsifree(catalogItem->mExtraItemInfoList.mExtraInfoElements[i].mKey);
		gsifree(catalogItem->mExtraItemInfoList.mExtraInfoElements[i].mValue);
	}

	gsifree(catalogItem->mExtraItemInfoList.mExtraInfoElements);
	
	return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giFreeCatalogItems
//  Deletes the contents of a given catalog.
//
GSResult d2giFreeCatalogItems(D2GICatalog  *theCatalog,
                              D2GCatalogItemList *itemList, 
                              GSResult result, 
                              gsi_bool freeItem)
{
    gsi_u32 i;

	if (itemList == NULL)
		return result;

    for (i = 0; i < itemList->mCount; i++)
    {
        if (freeItem)
        {
            d2giFreeCatalogItem(theCatalog, &itemList->mCatalogItems[i]);
        }
    }
    
    if (itemList->mCount > 0)
    {
        gsifree(itemList->mCatalogItems);
        itemList->mCatalogItems = NULL;
        itemList->mCount = 0;
    }
	
	gsifree(itemList);
    return result;
}

void d2giSwap(void *ptr1, void *ptr2, size_t size)
{
	unsigned char *ptrTmp = gsimalloc(size);
	memcpy(ptrTmp, (unsigned char*) ptr1, size);
	memcpy((unsigned char*) ptr1, (unsigned char*) ptr2, size);
	memcpy((unsigned char*) ptr2, ptrTmp, size);
	gsifree(ptrTmp);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giCompareU32
//
gsi_bool d2giCompareU32(const void *ptrA,
						const void *ptrB,
						D2GSortDirection direction)
{
	gsi_u32 valueA = *(gsi_u32 *)ptrA;
	gsi_u32 valueB = *(gsi_u32 *)ptrB;
	if (direction == D2GSort_Ascending)
	{
		return (valueA <= valueB);
	}
	else
	{
		return (valueA >= valueB);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giCompareInt
//
gsi_bool d2giCompareInt(const void *ptrA,
						const void *ptrB,
						D2GSortDirection direction)
{
	int valueA = *((int *)ptrA);
	int valueB = *((int *)ptrB);
	if (direction == D2GSort_Ascending)
	{
		return (valueA <= valueB);
	}
	else
	{
		return (valueA >= valueB);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giCompareWNum
//
gsi_bool d2giCompareWFloat(const void *ptrA,
						   const void *ptrB,
						   D2GSortDirection direction)
{

	if (direction == D2GSort_Ascending)
	{
		return (gsiWStringToDouble(*(UCS2String *) ptrA) <= gsiWStringToDouble(*(UCS2String *)ptrB));
	}
	else
	{
		return (gsiWStringToDouble(*(UCS2String *) ptrA) >= gsiWStringToDouble(*(UCS2String *)ptrB));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giCompareWStr
//
gsi_bool d2giCompareWStr(const void *ptrA,
						 const void *ptrB,
						 D2GSortDirection direction)
{
	if (direction == D2GSort_Ascending)
	{
		return (wcscmp(*(UCS2String *)ptrA, *(UCS2String *) ptrB) <= 0);
	}
	else
	{
		return (wcscmp(*(UCS2String *)ptrA,*(UCS2String *) ptrB) >= 0);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giCompareTime
//
gsi_bool d2giCompareTime(const void *ptrA,
						 const void *ptrB,
						 D2GSortDirection direction)
{
	time_t valueA = *(time_t *)ptrA;
	time_t valueB = *(time_t *)ptrB;

	if (direction == D2GSort_Ascending)
	{
		return (valueA <= valueB);
	}
	else
	{
		return (valueA >= valueB);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giQSort
//
void d2giQSort(void         *list,
			   size_t       size,
			   unsigned int offset,
			   D2GSortDirection  direction,
			   int          left, 
			   int          right, 
			   compareFunc	compareFunction)
{
	int pivotIndex, l_hold, r_hold;

	l_hold = left;
	r_hold = right;
	pivotIndex = left;

	while (left < right)
	{

		if (direction == D2GSort_Ascending)
		{ 
			while ((left <= right) && (*compareFunction)((unsigned char*)list+left*size+offset,
														 (unsigned char*)list+pivotIndex*size+offset, 
														 direction))
				left++;
			while ((left <= right) && (*compareFunction)((unsigned char*)list+pivotIndex*size+offset,
														 (unsigned char*)list+right*size+offset, 
														 direction ))
				right--;

		}
		else
		{
			while ((left <= right) &&(*compareFunction)((unsigned char*)list+left*size+offset,
														(unsigned char*)list+pivotIndex*size+offset, 
														direction))
				left++;
			while ((left <= right) &&(*compareFunction)((unsigned char*)list+pivotIndex*size+offset,
														(unsigned char*)list+right*size+offset, 
														direction))
				right--;
		}

		if (left < right)
		{
			d2giSwap(((unsigned char*)list+left*size), ((unsigned char*)list+right*size), size);
		}
	}
	if (pivotIndex != right) 
	{
		d2giSwap(((unsigned char*)list+pivotIndex*size), ((unsigned char*)list+right*size), size);
	}
	pivotIndex = right;
	if (l_hold < (pivotIndex-1))
		d2giQSort(list, size, offset, direction, l_hold, pivotIndex-1, compareFunction);
	if (r_hold > (pivotIndex+1))
		d2giQSort(list, size, offset, direction, pivotIndex+1, r_hold, compareFunction);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbyName
//
GSResult d2giSortCatalogItemsbyName(D2GCatalogItemList  *list, 
                                    D2GSortDirection direction)
{
    int offset = offsetof(D2GCatalogItem, mItem.mName);
	d2giQSort(&list->mCatalogItems[0], 
			  sizeof(D2GCatalogItem),
			  offset, 
			  direction, 
			  0, 
			  (list->mCount-1),
			  &d2giCompareWStr);

    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbyPrice
//
GSResult d2giSortCatalogItemsbyPrice(D2GCatalogItemList  *list, 
                                     D2GSortDirection direction)
{
    int offset = offsetof(D2GCatalogItem, mItem.mPrice);
	d2giQSort(&list->mCatalogItems[0], 
			  sizeof(D2GCatalogItem),
			  offset, 
			  direction, 
			  0, 
			  (list->mCount-1),
			  &d2giCompareWFloat);

    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbyDate
//
GSResult d2giSortCatalogItemsbyDate(D2GCatalogItemList  *list, 
                                    D2GSortDirection direction)
{
    int offset = offsetof(D2GCatalogItem, mProductInfo.mReleaseDate);
	d2giQSort(&list->mCatalogItems[0], 
		sizeof(D2GCatalogItem),
		offset, 
		direction, 
		0, 
		(list->mCount-1),
		&d2giCompareTime);

    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbyID
//
GSResult d2giSortCatalogItemsbyID(D2GCatalogItemList  *list, 
									D2GSortDirection direction)
{
	int offset = offsetof(D2GCatalogItem, mItem.mItemId);

	d2giQSort(&list->mCatalogItems[0], 
		sizeof(D2GCatalogItem),
		offset, 
		direction, 
		0, 
		(list->mCount-1),
		&d2giCompareU32);

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbyExtId
//
GSResult d2giSortCatalogItemsbyExtId(D2GCatalogItemList  *list, 
								  D2GSortDirection direction)
{
	int offset = offsetof(D2GCatalogItem, mItem.mExternalItemCode);
	d2giQSort(&list->mCatalogItems[0], 
		sizeof(D2GCatalogItem),
		offset, 
		direction, 
		0, 
		(list->mCount-1),
		&d2giCompareWStr);

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giSortCatalogItemsbySize
//
GSResult d2giSortCatalogItemsbySize(D2GCatalogItemList  *list, 
								  D2GSortDirection direction)
{
	int offset = offsetof(D2GCatalogItem, mProductInfo.mFileSize);
	d2giQSort(&list->mCatalogItems[0], 
		sizeof(D2GCatalogItem),
		offset, 
		direction, 
		0, 
		(list->mCount-1),
		&d2giCompareU32);

	return GS_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2giFreeCreditCardInfoList
//      Deletes the contents of credit card list in the response.
//
void d2giFreeCreditCardInfoList(D2GCreditCardInfoList *creditCardList)
{
	gsi_u32 i;

	if (creditCardList == NULL)
		return;
	
	for(i = 0; i < creditCardList->mCount; i++)
	{
		gsifree(creditCardList->mCreditCardInfos[i].mCreditCardType);
	}
	
	gsifree(creditCardList->mCreditCardInfos);
    creditCardList->mCount = 0;
    creditCardList->mCreditCardInfos=NULL;
    gsifree(creditCardList);
	GSI_UNUSED(creditCardList);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2giFreeOrder
//  Frees all contents of the OrderInfo
//
void d2giFreeOrder(D2GOrderInfo *theOrder)
{
	if (theOrder)
	{
	    gsifree(theOrder->mGeoInfo.mCultureCode);
	    theOrder->mGeoInfo.mCultureCode = NULL;
	    
	    gsifree(theOrder->mGeoInfo.mCurrencyCode);
	    theOrder->mGeoInfo.mCurrencyCode = NULL;
		
		gsifree(theOrder->mSubTotal);
		theOrder->mSubTotal = NULL;
		
		gsifree(theOrder->mTax);
		theOrder->mTax = NULL;
		
		gsifree(theOrder->mTotal);
		theOrder->mTotal = NULL;
		
		gsifree(theOrder->mRootOrderGuid);
		theOrder->mRootOrderGuid = NULL;
		
		gsifree(theOrder->mValidation.mMessage);
		theOrder->mValidation.mMessage = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeOrderItem
//
void d2giFreeOrderItem(D2GOrderItem *orderItem)
{
	if (orderItem)
	{
        // Free D2GBasicItemInfo Contents. 
        gsifree(orderItem->mItem.mExternalItemCode);
        gsifree(orderItem->mItem.mName);
        gsifree(orderItem->mItem.mPrice);
        gsifree(orderItem->mItem.mTax);
        
        orderItem->mItem.mItemId = 0;
        orderItem->mItem.mExternalItemCode = NULL;
        orderItem->mItem.mName  = NULL;
        orderItem->mItem.mPrice = NULL;
        orderItem->mItem.mTax   = NULL;

        // Free D2GOrderValidation Contents
        gsifree(orderItem->mValidation.mMessage);
        
        orderItem->mValidation.mMessage = NULL;
        orderItem->mValidation.mIsValid = gsi_false;
        orderItem->mValidation.mResult  = 0;
        
     	// Free D2GOrderItemTotal Contents    
		gsifree(orderItem->mItemTotal.mTotal);
		gsifree(orderItem->mItemTotal.mSubTotal);
		
		orderItem->mItemTotal.mQuantity = 0;
		orderItem->mItemTotal.mSubTotal = NULL;
		orderItem->mItemTotal.mTotal    = NULL;		
	}	
}

void d2giFreeDownloadItemContents(D2GDownloadItem * downloadItem)
{
    if (downloadItem)
	{
		downloadItem->mUrlId  = 0;
		downloadItem->mFileId = 0;
		downloadItem->mSequence = 0;
		downloadItem->mVersion  = 0;

		gsifree(downloadItem->mName);
		downloadItem->mName = NULL;
		gsifree(downloadItem->mAssetType);
		downloadItem->mAssetType = NULL;
	}	
}


void d2giFreeContentUpdate(D2GContentUpdate *downloadRecord)
{
	if (downloadRecord)
	{
		d2giFreeDownloadItemContents(&downloadRecord->mDownloadItem);
	}
}


void d2giFreeContentUpdateList(D2GContentUpdateList *downloadList)
{
	gsi_u32 i;
	if (downloadList == NULL)
		return;
	
	for (i = 0; i < downloadList->mCount; i++ )
	{
		d2giFreeContentUpdate(&downloadList->mContentUpdates[i]);
	}
	gsifree(downloadList->mContentUpdates);
	downloadList->mCount = 0;
	downloadList->mContentUpdates = NULL;
	gsifree(downloadList);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeOrderItemPurchases
//  recursively deletes the purchase list
//
void d2giFreeOrderItemPurchases(D2GOrderItemPurchase *orderItemPurchases, 
                                gsi_u32              orderItemPurchaseCount)
{
	gsi_u32 i;
	gsi_u32 j;
	
	if (orderItemPurchases != NULL)
	{
		for (i = 0; i < orderItemPurchaseCount; i++)
		{
		    // Free D2GOrderItem
			d2giFreeOrderItem(&orderItemPurchases[i].mOrderItem);
						
			// Free D2GLicenseItemList
			for (j = 0; j < orderItemPurchases[i].mLicenseList.mCount; j++)
			{
				gsifree(orderItemPurchases[i].mLicenseList.mLicenses[j].mLicenseKey);
				orderItemPurchases[i].mLicenseList.mLicenses[j].mLicenseKey = NULL;

				gsifree(orderItemPurchases[i].mLicenseList.mLicenses[j].mLicenseName);
				orderItemPurchases[i].mLicenseList.mLicenses[j].mLicenseName = NULL;
			}
			gsifree(orderItemPurchases[i].mLicenseList.mLicenses);
			orderItemPurchases[i].mLicenseList.mLicenses = NULL;
			orderItemPurchases[i].mLicenseList.mCount = 0;

			// Free D2GDownloadItemList
			for (j = 0; j < orderItemPurchases[i].mDownloadList.mCount; j++)
			{
				d2giFreeDownloadItemContents(&orderItemPurchases[i].mDownloadList.mDownloads[j]);
			}
			gsifree(orderItemPurchases[i].mDownloadList.mDownloads);
			orderItemPurchases[i].mDownloadList.mDownloads = NULL;
			orderItemPurchases[i].mDownloadList.mCount = 0;
						
			// Recursively free the D2GOrderItemPurchaseList
			d2giFreeOrderItemPurchases(orderItemPurchases[i].mPurchaseList.mOrderItemPurchases, 
			                           orderItemPurchases[i].mPurchaseList.mCount);
		}
		gsifree(orderItemPurchases);
	}	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeOrderTotal
//
void d2giFreeOrderTotal(D2GOrderTotal *orderTotal)
{
	gsi_u32 i;

	if (orderTotal != NULL)
	{
		d2giFreeOrder(&orderTotal->mOrder);

		for(i = 0; i < orderTotal->mOrderItemList.mCount; i++)
		{
			d2giFreeOrderItem(&orderTotal->mOrderItemList.mOrderItems[i]);
		}

		gsifree(orderTotal->mOrderItemList.mOrderItems);
        orderTotal->mOrderItemList.mOrderItems = NULL;
        orderTotal->mOrderItemList.mCount = 0 ;
		gsifree(orderTotal);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeOrderPurchase
//
void d2giFreeOrderPurchase(D2GOrderPurchase *purchase)
{
	if (purchase)
	{
		d2giFreeOrder(&purchase->mOrder);
		
		d2giFreeOrderItemPurchases(purchase->mItemPurchases.mOrderItemPurchases, 
		                           purchase->mItemPurchases.mCount);
		                           
		purchase->mItemPurchases.mOrderItemPurchases = NULL;
		purchase->mItemPurchases.mCount = 0;
		gsifree(purchase);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeOrderPurchase
//
void d2giFreeOrderPurchaseContents(D2GOrderPurchase *purchase)
{
	if (purchase)
	{
		d2giFreeOrder(&purchase->mOrder);

		d2giFreeOrderItemPurchases(purchase->mItemPurchases.mOrderItemPurchases,
								   purchase->mItemPurchases.mCount);

		purchase->mItemPurchases.mOrderItemPurchases = NULL;
		purchase->mItemPurchases.mCount = 0;
	}
}


void d2giFreePurchaseHistory(D2GPurchaseHistory *purchaseHistory)
{
	gsi_u32 i;
	if (purchaseHistory == NULL)
		return;

	// cleanup each purchase in the list
	for (i = 0 ; i < purchaseHistory->mCount; i++)
	{
		d2giFreeOrderPurchaseContents(&purchaseHistory->mPurchases[i]);
	}
	
	// cleanup the rest of the structure
	gsifree(purchaseHistory->mPurchases);
	purchaseHistory -> mPurchases = NULL;
	purchaseHistory -> mCount = 0;
	gsifree(purchaseHistory);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giGetAllItemsFromCache
//      Used to retrieve the catalog items from the cache. The result is 
//      returned by the callback 
//
GSResult d2giGetAllItemsFromCache(D2GICatalog *theCatalog, 
                                  D2GLoadCatalogItemsCallback callback, 
                                  void *userData )
{
    D2GLoadCatalogItemsResponse aResponse;
    GSResult result = GS_SUCCESS;

	aResponse.mHttpResult = GHTTPSuccess;
	aResponse.mItemList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));
	if (aResponse.mItemList == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
			"%s :: %s (%d): Out of memory during allocation of aResponse.mItemList\n"
			__FILE__, __FUNCTION__, __LINE__);
		result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}
	else
	{
		aResponse.mItemList->mCount        = ArrayLength(theCatalog->mItemArray);
		aResponse.mItemList->mCatalogItems = NULL;

		if (aResponse.mItemList->mCount)
		{
			gsi_u32 i;

			aResponse.mItemList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem) * aResponse.mItemList->mCount);
			if (aResponse.mItemList->mCatalogItems == NULL)
			{
				gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
					"%s :: %s (%d): Out of memory during allocation of aResponse.mItemList->mCatalogItems\n"
					__FILE__, __FUNCTION__, __LINE__);
				result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
			}

			for (i = 0; i < aResponse.mItemList->mCount; i++)
			{
				//shallow copy
				aResponse.mItemList->mCatalogItems[i] = *(D2GCatalogItem *)ArrayNth(theCatalog->mItemArray, i);
			}
		}
	}
    
	if (GS_FAILED(result))
	{
		d2giFreeCatalogItems(theCatalog, aResponse.mItemList, result, gsi_false);
		aResponse.mItemList = NULL;
	}

    callback(result, &aResponse, userData);

    return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFindOrAddCategory
//      Internal function to add the found categories in the cache to a list.
//
static gsi_bool d2giFindOrAddCategory(UCS2String      newCategory, 
                                      D2GCategoryList *categoryList)
{
    GSResult result = GS_SUCCESS;
    gsi_bool found = gsi_false;
    
    int upperBound = (int) categoryList->mCount;
    int lowerBound = 0;
    
    while ((lowerBound < upperBound) && !found)
    {
        UCS2String currentCategory = categoryList->mCategoryNames[(lowerBound + upperBound) / 2];
        
        int cmpResult = wcscmp(newCategory,currentCategory); 

        if (cmpResult > 0)
        {
            lowerBound = ((lowerBound + upperBound) / 2) + 1;
        }
        else if (cmpResult < 0)
        {
            upperBound = ((lowerBound + upperBound) / 2);
        }
        else
        {
            // we found it in the list just return true
            found = gsi_true;
        }
    }
    if (!found)
    {
        if (categoryList->mCount<GS_D2G_MAX_CATEGORIES-1)
        {
            // add the category to the list at the insertion point
            // which is lowerBound
            if ((int)categoryList->mCount>lowerBound)
            {
                memmove(((categoryList->mCategoryNames)+lowerBound+1), 
                        ((categoryList->mCategoryNames)+lowerBound), 
                        (sizeof(UCS2String *)*(categoryList->mCount-lowerBound)));
            }            
            categoryList->mCategoryNames[lowerBound] = newCategory;
            categoryList->mCount++;
        }
        else
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
                "%s :: %s (%d): Maximum number of categories have been reached : %d\n"
                __FILE__, __FUNCTION__, __LINE__, GS_D2G_MAX_CATEGORIES);
            result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_SizeExceedsMaximum);

        }
    }  
    return result ;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giGetCategoriesFromCache
//      Used to retrieve the categories from cache. 
//
GSResult d2giGetCategoriesFromCache(D2GICatalog *theCatalog, D2GCategoryList *categoryList )
{
    GSResult result = GS_SUCCESS;
   
    D2GCatalogItemList catalogItems;
    
    // the list is empty
    categoryList->mCount = 0; 
    categoryList->mCategoryNames = NULL;
    
    catalogItems.mCount        = ArrayLength(theCatalog->mItemArray);
    catalogItems.mCatalogItems = NULL;

    if (catalogItems.mCount > 0)
    {
        gsi_u32 i, j;

        categoryList->mCategoryNames = (UCS2String *) gsimalloc(sizeof(UCS2String)*GS_D2G_MAX_CATEGORIES);
        memset(categoryList->mCategoryNames, 0, sizeof(UCS2String)*GS_D2G_MAX_CATEGORIES);
        
        for (i = 0; i < catalogItems.mCount ; i++)
        {
            D2GCatalogItem *catalogItem;   
            catalogItem = (D2GCatalogItem *)ArrayNth(theCatalog->mItemArray, i);
            for (j = 0; j < catalogItem->mCategories.mCount; j++)
            {
                result = d2giFindOrAddCategory(catalogItem->mCategories.mCategoryNames[j], categoryList);
                if (GS_FAILED(result))
                {
                    return result;
                }
            }
        }
    }
    else
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): data not in cache!\n",
            __FILE__, __FUNCTION__, __LINE__);
        result = GS_D2G_ERROR(GSResultSection_State, D2GResultCode_Catalog_Empty);
    }

	return result;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giCheckItemsQuantities(D2GIInstance *theInstance, 
                                  gsi_u32      *itemQuantities, 
                                  gsi_u32      itemQuantitiesCount)
{
    gsi_u32 i;
    GS_ASSERT(theInstance);
    GS_ASSERT(itemQuantities);
    GS_ASSERT(itemQuantitiesCount);

    for(i = 0; i < itemQuantitiesCount; i++)
    {
        if (itemQuantities[i] == 0)
            return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }
    GSI_UNUSED(theInstance);
    return GS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2giCloneOrderItem
//      makes a deep copy of the Order Item function
//
GSResult d2giCloneOrderItem(D2GOrderItem *dstOrderItem, 
                            D2GOrderItem *srcOrderItem)
{
    GS_ASSERT(srcOrderItem);
    GS_ASSERT(dstOrderItem);

    if (srcOrderItem == NULL || dstOrderItem == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_HotError,
            "%s :: %s (%d): called with null dstOrderItem, srcOrderItem",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    dstOrderItem->mValidation.mIsValid = srcOrderItem->mValidation.mIsValid;
    dstOrderItem->mValidation.mResult  = srcOrderItem->mValidation.mResult;

    // copy D2GBasicItemInfo
    dstOrderItem->mItem.mItemId = srcOrderItem->mItem.mItemId;

    dstOrderItem->mItemTotal.mQuantity = srcOrderItem->mItemTotal.mQuantity;

    GS_ASSERT(srcOrderItem->mValidation.mIsValid || 
        (!srcOrderItem->mValidation.mIsValid && srcOrderItem->mValidation.mMessage));
    GS_ASSERT(srcOrderItem->mItemTotal.mSubTotal);
    GS_ASSERT(srcOrderItem->mItemTotal.mTotal);
    GS_ASSERT(srcOrderItem->mItem.mExternalItemCode);
    GS_ASSERT(srcOrderItem->mItem.mName);
    GS_ASSERT(srcOrderItem->mItem.mPrice);
    GS_ASSERT(srcOrderItem->mItem.mTax); 

    if (srcOrderItem->mItem.mExternalItemCode == NULL ||
        srcOrderItem->mItem.mName == NULL ||
        srcOrderItem->mItem.mPrice == NULL ||
        srcOrderItem->mItem.mTax == NULL || 
        srcOrderItem->mItemTotal.mSubTotal == NULL || 
        srcOrderItem->mItemTotal.mTotal == NULL || 
        (srcOrderItem->mValidation.mIsValid == gsi_false && srcOrderItem->mValidation.mMessage == NULL)) 
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): called with null srcOrderItem->mIsValidMsg\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // Copy D2GBasicItemInfo
    dstOrderItem->mItem.mExternalItemCode = goawstrdup(srcOrderItem->mItem.mExternalItemCode);
    dstOrderItem->mItem.mName   = goawstrdup(srcOrderItem->mItem.mName);
    dstOrderItem->mItem.mPrice  = goawstrdup(srcOrderItem->mItem.mPrice);
    dstOrderItem->mItem.mTax    = goawstrdup(srcOrderItem->mItem.mTax); 

    // Copy D2GOrderValidation
    dstOrderItem->mValidation.mMessage = goawstrdup(srcOrderItem->mValidation.mMessage);

    // Copy D2GOrderItemTotal
    dstOrderItem->mItemTotal.mSubTotal = goawstrdup(srcOrderItem->mItemTotal.mSubTotal);
    dstOrderItem->mItemTotal.mTotal    = goawstrdup(srcOrderItem->mItemTotal.mTotal);

    return GS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2giCloneDownloadList
//      a deep copy function
GSResult d2giCloneDownloadList(D2GDownloadItemList *dstDownloadItemList, 
                               D2GDownloadItemList *srcDownloadItemList)
{
    gsi_u32 i;

    // srcDownloadItemList is checked first because it is the source
    GS_ASSERT(srcDownloadItemList);
    GS_ASSERT(dstDownloadItemList);

    if (srcDownloadItemList == NULL || dstDownloadItemList == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): called with null dstDownloadItemList or srcDownloadItemList.\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    dstDownloadItemList->mCount = srcDownloadItemList->mCount;

    if (dstDownloadItemList->mCount == 0)
    {
        dstDownloadItemList->mDownloads = NULL;
        return GS_SUCCESS;
    }

    dstDownloadItemList->mDownloads = (D2GDownloadItem *)gsimalloc(sizeof(D2GDownloadItem) * dstDownloadItemList->mCount);
    if (dstDownloadItemList->mDownloads == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): failed to allocate memory for dstDownloadItemList->mDownloads.\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    for (i = 0; i < dstDownloadItemList->mCount; i++)
    {
        dstDownloadItemList->mDownloads[i].mUrlId    = srcDownloadItemList->mDownloads[i].mUrlId;
        dstDownloadItemList->mDownloads[i].mFileId   = srcDownloadItemList->mDownloads[i].mFileId;
        dstDownloadItemList->mDownloads[i].mSequence = srcDownloadItemList->mDownloads[i].mSequence;
        dstDownloadItemList->mDownloads[i].mVersion  = srcDownloadItemList->mDownloads[i].mVersion;

        GS_ASSERT(srcDownloadItemList->mDownloads[i].mAssetType);
        if (srcDownloadItemList->mDownloads[i].mAssetType == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): called with null mAssetType in srcDownloadItemList at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
        }
        dstDownloadItemList->mDownloads[i].mAssetType = goawstrdup(srcDownloadItemList->mDownloads[i].mAssetType);
        if (dstDownloadItemList->mDownloads[i].mAssetType == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): failed to allocate mAssetType in dstDownloadItemList at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }


        GS_ASSERT(srcDownloadItemList->mDownloads[i].mName);
        if (srcDownloadItemList->mDownloads[i].mName == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): called with null mName in srcDownloadItemList at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
        }
        dstDownloadItemList->mDownloads[i].mName = goawstrdup(srcDownloadItemList->mDownloads[i].mName);
        if (dstDownloadItemList->mDownloads[i].mName == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): failed to allocate mName in dstDownloadItemList at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
    }

    return GS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  d2giCloneLicenseList
//      a deep copy function
//
GSResult d2giCloneLicenseList(D2GLicenseItemList *dstLicenses, 
                              D2GLicenseItemList *srcLicenses)
{
    gsi_u32 i;
    GS_ASSERT(srcLicenses);
    GS_ASSERT(dstLicenses);

    if (srcLicenses == NULL || dstLicenses == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): called with null dstLicenses or srcLicenses\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    dstLicenses->mCount = srcLicenses->mCount;

    if (dstLicenses->mCount == 0)
    {
        dstLicenses->mLicenses = NULL;
        return GS_SUCCESS;
    }

    dstLicenses->mLicenses = (D2GLicenseItem *)gsimalloc(sizeof(D2GLicenseItem) * dstLicenses->mCount);
    if (dstLicenses->mLicenses == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): failed to allocate memory for dstLicenses->mLicenses.\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    for (i = 0; i < dstLicenses->mCount; i++)
    {
        GS_ASSERT(srcLicenses->mLicenses[i].mLicenseKey);
        if (srcLicenses->mLicenses[i].mLicenseKey == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): called with null mLicenseKey in srcLicenses at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
        }

        dstLicenses->mLicenses[i].mLicenseKey = goawstrdup(srcLicenses->mLicenses[i].mLicenseKey);
        if (dstLicenses->mLicenses[i].mLicenseKey == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): failed to allocate mLicenseKey in dstLicenses at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

        GS_ASSERT(srcLicenses->mLicenses[i].mLicenseName);
        if (srcLicenses->mLicenses[i].mLicenseName == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): called with null mLicenseName in srcLicenses at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
        }

        dstLicenses->mLicenses[i].mLicenseName = goawstrdup(srcLicenses->mLicenses[i].mLicenseName);
        if (dstLicenses->mLicenses[i].mLicenseName == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): failed to allocate mLicenseName in dstLicenses at %d.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }

    }

    return GS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2giCloneOrderItemPurchases
//      a deep copy function - recursive
//
GSResult d2giCloneOrderItemPurchases(D2GOrderItemPurchase **destPurchases, 
                                     gsi_u32              *dstCount, 
                                     D2GOrderItemPurchase *srcPurchases, 
                                     gsi_u32              srcCount)
{
    D2GOrderItemPurchase *tempPurchases;
    gsi_u32 i, count;
    GS_ASSERT(destPurchases);
    if (destPurchases == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): called with null destPurchases\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    count = *dstCount = srcCount;
    if (count == 0)
    {
        *destPurchases = NULL;
        return GS_SUCCESS;
    }

    tempPurchases = *destPurchases = (D2GOrderItemPurchase *)gsimalloc(sizeof(D2GOrderItemPurchase) * count);
    if (tempPurchases == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): failed to allocate memory for tempPurchases.\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    for (i = 0; i < count; i++)
    {
        GSResult result;
        result = d2giCloneOrderItem(&tempPurchases[i].mOrderItem, &srcPurchases[i].mOrderItem);
        if (GS_FAILED(result))
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): Failed to copy order item %d using d2giCloneOrderItem.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return result;
        }

        result = d2giCloneDownloadList(&tempPurchases[i].mDownloadList, &srcPurchases[i].mDownloadList);
        if (GS_FAILED(result))
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): Failed to copy download list for item %d using d2giCloneDownloadList.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return result;
        }

        result = d2giCloneLicenseList(&tempPurchases[i].mLicenseList, &srcPurchases[i].mLicenseList);
        if (GS_FAILED(result))
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): Failed to copy license list for item %d using d2giCloneLicenseList.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return result;
        }

        result = d2giCloneOrderItemPurchases(&tempPurchases[i].mPurchaseList.mOrderItemPurchases, 
                                             &tempPurchases[i].mPurchaseList.mCount, 
                                             srcPurchases[i].mPurchaseList.mOrderItemPurchases, 
                                             srcPurchases[i].mPurchaseList.mCount);
        if (GS_FAILED(result))
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
                "%s :: %s (%d): Failed to copy subitem list for item %d using d2giCloneOrderItemPurchases.\n",
                __FILE__, __FUNCTION__, __LINE__, i);
            return result;
        }
    }

    return GS_SUCCESS;	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giGetItemsByCategoryFromCache
//      Used to retrieve the catalog items for cache 
//
GSResult d2giGetCatalogItemsByCategoryFromCache(D2GICatalog  *theCatalog,
												const        UCS2String theCategory, 
												D2GLoadCatalogItemsByCategoryCallback callback, 
												void         *userData )
{
    D2GLoadCatalogItemsByCategoryResponse aResponse;
    gsi_u32 itemArrayCount;
    GSResult result = GS_SUCCESS;

    memset(&aResponse, 0, sizeof(aResponse));
    aResponse.mHttpResult = GHTTPSuccess;
    itemArrayCount = ArrayLength(theCatalog->mItemArray);

    if (itemArrayCount > 0)
    {
        gsi_u32 i,j;
        gsi_u32 foundItems = 0;
        D2GCatalogItem **aItemArray;

        // allocate a list of pointers to the CatalogItems cached
        aItemArray = (D2GCatalogItem **)gsimalloc(sizeof(D2GCatalogItem *) * itemArrayCount);
        memset(aItemArray, 0, sizeof(D2GCatalogItem *)*itemArrayCount);

        // for each catalog item in the cache
        for(i = 0; i < itemArrayCount; i++)
        {
            D2GCatalogItem *anItem = (D2GCatalogItem *)ArrayNth(theCatalog->mItemArray, i);
            if (anItem->mCategories.mCount)
            {
                // there is category attached to the catalog item 
                for (j = 0; j < anItem->mCategories.mCount; j++)
                {
                    // if it belongs to the category we are interested in
                    if (wcscmp((wchar_t *)anItem->mCategories.mCategoryNames[j], (wchar_t *)theCategory)==0)
                    {
                        // copy the pointer to the item into the pointer list
                        aItemArray[i] = anItem;
                        foundItems++;
                        break;
                    }
                }
            }
        }
        aResponse.mItemList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));
        if (aResponse.mItemList == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
                "%s :: %s (%d): Out of memory during allocation of aResponse.mItemList\n"
                __FILE__, __FUNCTION__, __LINE__);
            result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        else
        {
            memset(aResponse.mItemList, 0, sizeof(D2GCatalogItemList));

            // Now we have list for items belongs to the same category retrieved from the catalog
            aResponse.mItemList->mCount = foundItems;
            if (aResponse.mItemList->mCount)
            {
                aResponse.mItemList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem) * aResponse.mItemList->mCount);
            }

            for (j = 0, i = 0; j < foundItems && i < itemArrayCount; i++)
            {
                if (aItemArray[i])
                {
                    aResponse.mItemList->mCatalogItems[j++] = *aItemArray[i];
                }
            }
        }
        // free the pointer array, just the container. 
        gsifree(aItemArray);
    }

	if (GS_FAILED(result))
	{
		d2giFreeCatalogItems(theCatalog, aResponse.mItemList, result, gsi_false);
		aResponse.mItemList = NULL;
	}

	// We don't release the mItemList. That's for the developer to choose.  
	// They can keep it until they need to get rid of it by calling 
	// the free function for the item.
    callback(result, &aResponse, userData);

    // return result as always!
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Extra Info Cache Helper Functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giLookUpExtraInfo
//
UCS2String d2giLookUpExtraInfo( D2GIInstance *theInstance, D2GICatalog *theCatalog, const UCS2String key )
{
    int index=0;
    for (index=0; index < ArrayLength(theCatalog->mExtraInfoCache); index++)
    {
        D2GExtraInfo * info = (D2GExtraInfo *)ArrayNth(theCatalog->mExtraInfoCache, index);
        if (wcscmp(info->mKey, key) == 0)
        {
            return info->mValue;
        }
    }
    GSI_UNUSED(theInstance);
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giNewCatalogItem
//      This is a helper function, given an item id,it returns a pointer to 
//      the new item's location in the 
//
D2GExtraInfo * d2giNewExtraInfo(D2GIInstance *theInstance, 
                                D2GICatalog  *theCatalog,
                                D2GExtraInfo *theExtraInfo)
{

    GS_ASSERT(theInstance != NULL);
    GS_ASSERT(theCatalog  != NULL);
    if (theInstance == NULL || theCatalog == NULL)
        return NULL;

    ArrayAppend(theCatalog->mExtraInfoCache, theExtraInfo);

    return (D2GExtraInfo *)ArrayNth(theCatalog->mExtraInfoCache, ArrayLength(theCatalog->mExtraInfoCache)-1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giGetExtraInfo
//      This is a helper function, given a key, to retrieve its value pair 
//      from the cache
//
D2GExtraInfo * d2giGetExtraInfo(D2GIInstance *theInstance, 
                                D2GICatalog  *theCatalog,
                                UCS2String   key)
{
    int index=0;
    for (index=0; index < ArrayLength(theCatalog->mExtraInfoCache); index++)
    {
        D2GExtraInfo * info = (D2GExtraInfo *)ArrayNth(theCatalog->mExtraInfoCache, index);
        if (wcscmp(info->mKey, key) == 0)
        {
            return info;
        }
    }
    GSI_UNUSED(theInstance);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeExtraInfo
//      This is a helper function to remove a Extra Info item from the cache.
//
GSResult  d2giFreeExtraInfo(D2GICatalog     *theCatalog, 
                            D2GExtraInfo    *extraInfo)
{
    int i=0;

	GS_ASSERT(theCatalog != NULL);
    GS_ASSERT(theCatalog->mExtraInfoCache != NULL);
	if (theCatalog == NULL || theCatalog->mExtraInfoCache == NULL || ArrayLength(theCatalog->mExtraInfoCache) == 0)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): the ItemArray is null or is empty\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }
    if (extraInfo == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with a null item\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_SUCCESS; 
    }

    for (i=0; i < ArrayLength(theCatalog->mExtraInfoCache); i++)
    {
        D2GExtraInfo * elem = (D2GExtraInfo *) ArrayNth(theCatalog->mExtraInfoCache, i);
        if (memcmp(elem->mKey,extraInfo->mKey, sizeof(extraInfo->mKey)) == 0)
        {
            ArrayDeleteAt(theCatalog->mExtraInfoCache, i);
            break;
        }
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giResetCatalogItem
//      Clears the contents of a given catalog item.
//
GSResult d2giResetExtraInfo(D2GExtraInfo *extraInfo)
{
    GS_ASSERT(extraInfo != NULL);
    if (extraInfo == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s :: %s (%d): received NULL input parameter\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_ArgumentNull);
    }

    // Free Contents
    gsifree(extraInfo->mKey);
    gsifree(extraInfo->mValue);
    extraInfo->mKey    = NULL;
    extraInfo->mValue  = NULL;
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giFreeCatalogItems
//      Deletes the contents of a given catalog.
//  
GSResult d2giFreeExtraInfoList(D2GICatalog  *theCatalog,
                               D2GExtraInfoList *extraInfoList, 
                               GSResult result, 
                               gsi_bool freeItem)
{
    gsi_u32 i;
    if (extraInfoList != NULL)
    {
        for (i = 0; i < extraInfoList->mCount; i++)
        {
            if (freeItem)
            {
                d2giFreeExtraInfo(theCatalog, &extraInfoList->mExtraInfoElements[i]);
            }
			//d2giResetExtraInfo(&extraInfoList->mExtraInfoElements[i]);
        }
    }
    if (extraInfoList->mCount > 0)
    {
        gsifree(extraInfoList->mExtraInfoElements);
        extraInfoList->mExtraInfoElements = NULL;
        extraInfoList->mCount = 0;
    }

	gsifree(extraInfoList);
    return result;
}

//////////////////////////////////////////////////////////////////////////
//  Manifest File Helper Functions
//////////////////////////////////////////////////////////////////////////

GSResult d2giSearchandUpdateManifestFile( FILE     *pManifest,
                                          FILE     *pManifestTemp,
                                          D2GIManifestRecord manifest,
                                          D2GContentUpdate   *contentUpdate,
                                          char      *manifestRead,
                                          gsi_bool *manifestRecordFound )
{
    GSResult    result = GS_SUCCESS ;

    memset(contentUpdate, 0, sizeof(D2GContentUpdate));
    memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);

    // read a manifest record from the manifest file into the downloadRecord
    while ( (*manifestRecordFound == gsi_false) &&
            (feof(pManifest) == 0) &&
            (fgets(manifestRead, (GS_D2G_MAX_FILE_BUFFER_LEN-1), pManifest) != NULL)
        )
    {
        result = d2giParseAndDecodeManifestRecord(manifestRead, contentUpdate);

        if (GS_SUCCEEDED(result))
        {
            // assuming we either have a new file or update an existing download
            // Now we have downloadRecord
            // compare with the download we have. itemId+urlId combination
            if (contentUpdate->mItemId < manifest.itemId)
            {
                // write buffer directly to the manifest file
                fputs(manifestRead, pManifestTemp);
            }
            else 
                if (contentUpdate->mItemId == manifest.itemId)
                {
                    if (contentUpdate->mDownloadItem.mUrlId < manifest.urlId)
                    {
                        // write the buffer directly to the manifest file
                        fputs(manifestRead, pManifestTemp);
                    }
                    else 
                        if (contentUpdate->mDownloadItem.mUrlId == manifest.urlId)
                        {
                            // we have found a matching manifest record
                            *manifestRecordFound = gsi_true;
                            break;
                        }
                        else
                        {
                            // we do not have this record so get out of here
                            break;
                        }
                } //itemid==
                else // itemId >
                {
                    // we do not have this record so get out of here
                    break;
                }
        } //GS_SUCCEEDED // add else condition here.
        memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);
    } //while fgets
    return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giCreateXmlStr
// 
GSResult d2giCreateAndEncodeManifestRecord(D2GIManifestRecord manifest, 
                                           char              *manifestUpdate)

{
    GSResult    result = GS_SUCCESS;
    D2GIString     manifestXml;

    memset(&manifestXml,0, sizeof(D2GIString));
    // create the XML string for the updated item
    result = d2giCreateManifestXmlStr (manifest,&manifestXml);
    if (GS_SUCCEEDED(result) ) 
    {
        // Use URL safe encoding
        if (manifestXml.len >0)
        {   
            B64Encode(manifestXml.str,manifestUpdate,(int) strlen(manifestXml.str), 2);
            strcat(manifestUpdate, "\n");
        }
    }
    gsifree(manifestXml.str);
    return result;
}    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giUpdateManifestRecord
// 
GSResult d2giUpdateManifestRecord(D2GIInstance       *theInstance,
                                  D2GIManifestRecord manifest)

{
    GSResult    result = GS_SUCCESS;
    char        manifestRead [GS_D2G_MAX_FILE_BUFFER_LEN];
    char        manifestUpdate[GS_D2G_MAX_FILE_BUFFER_LEN];
    FILE        *pManifest = NULL;
    char        *manifestFileName = NULL; 

    memset(manifestUpdate, '\0', sizeof(manifestUpdate));
    result = d2giCreateAndEncodeManifestRecord(manifest, manifestUpdate);
    
    if (GS_FAILED(result))
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s :: %s (%d): Cannot create XML string (%s)", 
            __FILE__, __FUNCTION__, __LINE__, manifestUpdate);
        return result;
    }
    
    // determine the filename & path
    if (!theInstance->mManifestFilePath)
    {
        manifestFileName = goastrdup(GS_D2G_MANIFEST_FILENAME);
    }
    else
    {
        manifestFileName = gsimalloc(sizeof(char) * (strlen(GS_D2G_MANIFEST_FILENAME)+strlen(theInstance->mManifestFilePath)+1));
        _stprintf(manifestFileName, "%s%s", theInstance->mManifestFilePath, GS_D2G_MANIFEST_FILENAME);
    }

    pManifest = fopen(manifestFileName, "r"); 
    if (pManifest == NULL)
    {
        // manifest file down not exist
        // so just write the one and only record we have
        // open it as "w+" first

        pManifest = fopen(manifestFileName, "w+" );
        if (pManifest == NULL)
        {        
            // Cannot write to directory
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_File, GSIDebugLevel_HotError,
                "%s :: %s (%d): Cannot open file %s", 
                __FILE__, __FUNCTION__, __LINE__, manifestFileName);
            result = GS_D2G_ERROR(GSResultSection_File, GSResultCode_FileWriteError);

        }
        else
        {
            // write the XML string into the manifest
            fputs(manifestUpdate, pManifest);
            fclose(pManifest);
        }
    } 
    else
    {
        // we have a manifest file so read a record from it and write to the manifest.temp
        // create a "manifest.tmp"
        char	manifestFileNameTmp[GS_MAX_FILENAME_LEN];

        FILE *pManifestTemp; 
		memset(manifestFileNameTmp, '\0',sizeof(manifestFileNameTmp));
		strncpy(manifestFileNameTmp, manifestFileName, strlen(manifestFileName));
		strcat(manifestFileNameTmp, GS_D2G_DOWNLOAD_FILE_TEMP_EXT);
		pManifestTemp = fopen(manifestFileNameTmp, "w+");

        while((pManifestTemp != NULL) && (!feof(pManifest)))
        {
            D2GContentUpdate contentUpdate; 
            gsi_bool         manifestRecordFound = gsi_false;

            result = d2giSearchandUpdateManifestFile( pManifest,
                                                      pManifestTemp,
                                                      manifest,
                                                      &contentUpdate,
                                                      manifestRead,
                                                      &manifestRecordFound );

            if (GS_SUCCEEDED(result))
            {
                // write the XML string into the manifest.tmp
                fputs(manifestUpdate, pManifestTemp);

                if ((!manifestRecordFound) && (strlen(manifestRead)>0))
                {
                    // we do not have this record so just write the last record read
                    // write the buffer directly to the manifest file
                    if ((contentUpdate.mItemId > manifest.itemId) ||
                        ((contentUpdate.mItemId == manifest.itemId) && 
                         (contentUpdate.mDownloadItem.mUrlId >= manifest.urlId))  
                        )
                    {                         
                        fputs(manifestRead, pManifestTemp);
                    }
                }
                // Now complete copying the remaining records if any 
                memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);                      
                while (!feof(pManifest) && 
                       (fgets(manifestRead, GS_D2G_MAX_FILE_BUFFER_LEN, pManifest) != NULL))
                {
                    fputs(manifestRead, pManifestTemp);
                    memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);       
                }
            }
        } // while
        fclose(pManifestTemp);
        fclose(pManifest);
        if (GS_SUCCEEDED(result))
        {
            _tremove(manifestFileName);
            _trename(manifestFileNameTmp, manifestFileName);
        }
    }
    gsifree(manifestFileName);
    return result;    

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giDeleteManifestRecord
// 

GSResult d2giDeleteManifestRecord(D2GIInstance       *theInstance,
                                  D2GIManifestRecord manifest)

{
    GSResult    result = GS_SUCCESS;
    char        *manifestRead = gsimalloc(GS_D2G_MAX_FILE_BUFFER_LEN);
    FILE        *pManifest = NULL;
    char        *manifestFileName = NULL ; 

    // determine the filename & path
    if (!theInstance->mManifestFilePath)
    {
        manifestFileName = goastrdup(GS_D2G_MANIFEST_FILENAME);
    }
    else
    {
        manifestFileName = gsimalloc(sizeof(char) * (strlen(GS_D2G_MANIFEST_FILENAME)+strlen(theInstance->mManifestFilePath)+1));
        _stprintf(manifestFileName, "%s%s", theInstance->mManifestFilePath, GS_D2G_MANIFEST_FILENAME);
    }

    pManifest = fopen(manifestFileName, "r"); 
    if (pManifest == NULL)
    {
        // The manifest file either was never created or must have been deleted 
        // because all entries have been deleted prior to this call.
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_File, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Manifest File does not exist", 
            __FILE__, __FUNCTION__, __LINE__);
        result = GS_D2G_ERROR(GSResultSection_File, GSResultCode_FileNotFound);
    } 
    else
    {
        // we have a manifest file so read a record from it and write to the manifest.temp
        // create a "manifest.tmp"
		char	manifestFileNameTmp[GS_MAX_FILENAME_LEN];

		FILE *pManifestTemp; 
		memset(manifestFileNameTmp, '\0',sizeof(manifestFileNameTmp));
		strncpy(manifestFileNameTmp, manifestFileName, strlen(manifestFileName));
		strcat(manifestFileNameTmp, GS_D2G_DOWNLOAD_FILE_TEMP_EXT);

        pManifestTemp = fopen(manifestFileNameTmp, "w+");


        while((pManifestTemp != NULL) && (!feof(pManifest)))
        { 
            D2GContentUpdate contentUpdate; 
            gsi_bool         manifestRecordFound = gsi_false;

            result = d2giSearchandUpdateManifestFile( pManifest,
                                                      pManifestTemp,
                                                      manifest,
                                                      &contentUpdate,
                                                      manifestRead,
                                                      &manifestRecordFound );

            if (GS_SUCCEEDED(result))
            {
                if (!manifestRecordFound)
                {
                    gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_File, GSIDebugLevel_Warning,
                        "%s :: %s (%d): Item Id %d already does not exist in the manifest file. No error generated", 
                        __FILE__, __FUNCTION__, __LINE__, manifest.itemId);

                    if(strlen(manifestRead)>0)
                    {
                        // we do not have this record so just write the last record read
                        // write the buffer directly to the manifest file
                        if ((contentUpdate.mItemId > manifest.itemId) ||
                            ((contentUpdate.mItemId == manifest.itemId) && 
                            (contentUpdate.mDownloadItem.mUrlId >= manifest.urlId))  
                            )
                        {
                            fputs(manifestRead, pManifestTemp);
                        }
                    }
                }
                // Now complete copying the remaining records if any 
                memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);                      
                while (!feof(pManifest) && 
                    (fgets(manifestRead, GS_D2G_MAX_FILE_BUFFER_LEN, pManifest) != NULL))
                {
                    fputs(manifestRead, pManifestTemp);
                    memset(manifestRead, '\0', GS_D2G_MAX_FILE_BUFFER_LEN);       
                }
            }
        } // while
        fclose(pManifestTemp);
        fclose(pManifest);
        if (GS_SUCCEEDED(result))
        {
            _tremove(manifestFileName);
            _trename(manifestFileNameTmp, manifestFileName);

        }
    }
    gsifree(manifestFileName);
    gsifree(manifestRead);
    return result;    
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giWriteManifestDownloadsToXML
// 
gsi_bool d2giWriteManifestDownloadsToXML(GSXmlStreamWriter writer,
                                         D2GIInstance *theInstance) 
{
    char        manifestRead[GS_D2G_MAX_FILE_BUFFER_LEN];
    FILE        *pManifest = NULL;
    GSResult    result= GS_SUCCESS;      
    char        *manifestFileName = NULL;

    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s :: %s (%d): NULL pointer passed.\n",
            __FILE__, __FUNCTION__, __LINE__);
        return gsi_false;
    }

    // determine the filename & path
    if (!theInstance->mManifestFilePath)
    {
        manifestFileName = goastrdup(GS_D2G_MANIFEST_FILENAME);
    }
    else
    {
        manifestFileName = gsimalloc(sizeof(char) * (strlen(GS_D2G_MANIFEST_FILENAME)+strlen(theInstance->mManifestFilePath)+1));
        _stprintf(manifestFileName, "%s%s", theInstance->mManifestFilePath, GS_D2G_MANIFEST_FILENAME);
    }

    pManifest = fopen(manifestFileName, "r"); 
    gsifree(manifestFileName);

    if (pManifest == NULL)
    {
        // manifest file down not exist
        // No downloaded content
        return gsi_true;
    } 
    else
    {
        memset(manifestRead, '\0', sizeof(manifestRead)); 
        while(!feof(pManifest)&& (fgets(manifestRead, sizeof(manifestRead)-1, pManifest) != NULL))
        {  
            D2GContentUpdate contentUpdate;
            D2GIString manifestXml;
            manifestXml.str = gsimalloc((strlen(manifestRead)+2) * sizeof(char));         
            memset (manifestXml.str, '\0',(strlen(manifestRead)*sizeof(char)) );
            memset(&contentUpdate, 0 , sizeof(contentUpdate));
            
            // Uses URL safe encoding
            B64Decode(manifestRead, manifestXml.str, (int) strlen(manifestRead), &manifestXml.len, 2);

            result = d2giParseManifestRecord(manifestXml.str, &contentUpdate);
            
            gsifree(manifestXml.str);
            
            if (GS_SUCCEEDED(result))
            {
                if (gsi_is_false(gsXmlWriteOpenTag   (writer, GS_D2G_NAMESPACE, "download")) ||
                    gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "itemid", contentUpdate.mItemId)) ||
                    gsi_is_false(gsXmlWriteIntElement(writer, GS_D2G_NAMESPACE, "downloadurlid", contentUpdate.mDownloadItem.mUrlId)) ||
                    gsi_is_false(gsXmlWriteFloatElement(writer, GS_D2G_NAMESPACE, "version", contentUpdate.mDownloadItem.mVersion)) ||
                    gsi_is_false(gsXmlWriteCloseTag  (writer, GS_D2G_NAMESPACE, "download")))

                {
                    gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                        "%s :: %s (%d): could not write request data: GS_D2G_GETDOWNLOADINFO_REQUEST\n", 
                        __FILE__, __FUNCTION__, __LINE__);
                    fclose(pManifest);
                    return gsi_false;
                }
            }
        }
        fclose(pManifest);
    }
    return gsi_true;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// d2giCreateManifestXmlStr
// 
// <download><itemid>123</itemid><downloadurlid>175446</downloadurlid><version>1.4</version></download>

GSResult d2giCreateManifestXmlStr(D2GIManifestRecord manifest, 
                                  D2GIString *manifestBuffer)
{
    GSResult result = GS_SUCCESS;
    GSXmlStreamWriter writer = NULL;

    writer = gsXmlCreateStreamWriterNoNamespace();
    if (writer == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
            "%s :: %s (%d): Failed on gsXmlCreateStreamWriter (assuming out of memory)\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    else
    {

        if (gsi_is_false(gsXmlWriteOpenTagNoNamespace(writer,"download")) ||
            gsi_is_false(gsXmlWriteIntElementNoNamespace(writer, "itemid", manifest.itemId)) ||
            gsi_is_false(gsXmlWriteIntElementNoNamespace(writer, "downloadurlid", manifest.urlId)) ||
            gsi_is_false(gsXmlWriteFloatElementNoNamespace(writer,"version", manifest.version)) ||
            gsi_is_false(gsXmlWriteCloseTagNoNamespace(writer, "download")) ||
            gsi_is_false(gsXmlCloseWriterNoNamespace(writer))
            )

        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                "%s :: %s (%d): could not write request data: GS_D2G_GETDOWNLOADINFO_REQUEST\n", 
                __FILE__, __FUNCTION__, __LINE__);

            gsXmlFreeWriter(writer);
            return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
        }
        manifestBuffer->len = gsXmlWriterGetDataLength(writer) + 3;
        manifestBuffer->str = gsimalloc(manifestBuffer->len);
        memset(manifestBuffer->str, '\0', manifestBuffer->len);
        strncpy(manifestBuffer->str, gsXmlWriterGetData(writer), gsXmlWriterGetDataLength(writer));
        gsXmlFreeWriter(writer);
    }  
    return result;
}
