///////////////////////////////////////////////////////////////////////////////
//  This test application uses the GameSpy Direct 2 Game SDK
//  Implemented by GameSpy Tech
//  Copyright (c) 2008, GameSpy Technology
///////////////////////////////////////////////////////////////////////////////
// These files need to be added to application includes
#include "../../common/gsCommon.h"
#include "../../common/gsCore.h"
#include "../../common/gsAvailable.h"
#include "../../webservices/AuthService.h"
#include "../Direct2Game.h"
#include "../../GP/gp.h"

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <errno.h>


#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG) && defined(USE_CRTDBG)
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#if defined(_NITRO)
	#include "../../common/nitro/screen.h"
	#define printf Printf
	#define vprintf VPrintf
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Globals
//STAGE
#define SAMPLE_GAMENAME            "gmtest"
#define SAMPLE_GAMEID			   0
#define SAMPLE_PARTERCODE          0
#define SAMPLE_NAMESPACEID         1
#define LOCAL_PLAYER_UNIQUENICK    "spyguy"
#define LOCAL_PLAYER_PASSWORD      "0000"
#define GAME_ID                    0
#define GAME_CATALOG_REGION		  L"en"
#define GAME_CATALOG_VERSION	      0 // stands for the default version
//#define GAME_GATALOG_VERSION     2 // use this to view other content
#define MAX_FILEID_DOWNLOADS       10 // for example purposes
#define SAMPLE_ACCESS_TOKEN        L"BA42DAE5-D8A4-4762-88C7-4F4CFEFEF34B"
#define SAMPLE_MAX_PURCHASES       3
// This structure is used only by  
// sampleDownloadFileById function

//DEV
// #define SAMPLE_GAMENAME            "gmtest"
// #define SAMPLE_PARTERCODE          0
// #define SAMPLE_NAMESPACEID         1
// #define LOCAL_PLAYER_UNIQUENICK    "D2GTest02"
// #define LOCAL_PLAYER_PASSWORD      "IGcash01"
// #define GAME_ID                    1774
// #define GAME_CATALOG_REGION		   L"global"
// #define GAME_CATALOG_VERSION	   0 // stands for the default version
// //#define GAME_GATALOG_VERSION     2 // use this to view other content
// #define MAX_FILEID_DOWNLOADS       10 // for example purposes
// #define SAMPLE_ACCESS_TOKEN        L"9FA19EAE-C9A6-4FFF-8233-30429B3BABFC"
// #define SAMPLE_MAX_PURCHASES       3

// This structure is used only by  
// sampleDownloadFileById function
typedef struct gameDownloadItem
{
    D2GItemId   itemId;
    D2GUrlId    urlId;
    D2GFileId   fileId;
    float       version;
} gameDownloadItem;
typedef struct gameDownloads
{
	int			downloadCount;
    gameDownloadItem downloads[MAX_FILEID_DOWNLOADS];
}gameDownloads;

typedef struct gameAppData
{
    gsi_bool gWaitingForAuth;       // simple state control
    gsi_bool gLoggedIn;             
    gsi_bool gWaiting;              
    gsi_bool gPurchaseComplete;		// for a simple purchase example

    GSLoginCertificate  gLoginCertificate;
    GSLoginPrivateData  gLoginPrivData;
    gameDownloads		gDownloads;
    D2GInstancePtr      gD2GInstancePtr;
    
} gameAppData;

gameAppData sampleGameAppData;

int gLocalPlayerID = 0;
const UCS2String gFirstCategoryID = L"0";
const UCS2String gSampleCategoryBad = L"PC Game Downloads";
const UCS2String gSampleCategoryGood = L"D2G Levels";
const UCS2String gSampleExtraInfoKeyName = L"RACE_LANGUAGE";
const UCS2String gSampleExtraInfoKeyValue = L"Moktar";
D2GItemId *gLocalItemIds = NULL;
gsi_u32 *gLocalItemQuantities = NULL;
gsi_u32 gLocalItemCount = 0;
gsi_u32 gLocalAccountId = 0;
D2GOrderTotal *gOrderTotal;
D2GOrderPurchase *gOrderPurchase;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Prototypes
static gsi_bool sampleBackendAvailable();
static gsi_bool sampleAuthPlayer();
void sampleAuthCallback(GHTTPResult result, WSLoginResponse * response, void * userData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Run the required availability check (all apps must do this)
static gsi_bool sampleBackendAvailable()
{
	GSIACResult result;

	GSIStartAvailableCheck(SAMPLE_GAMENAME);
	do 
	{
		result = GSIAvailableCheckThink();
		msleep(10);
	} while(result == GSIACWaiting);

	// check result
	if (result == GSIACUnavailable)
	{
		wprintf(L"Availability check returned GSIACUnavailable -- Backend services have been disabled\r\n");
		return gsi_false;
	}
	else if (result == GSIACTemporarilyUnavailable)
	{
		wprintf(L"Availability check returned GSIACTemporarilyUnavailable -- Backend services are temporarily down\r\n");
		return gsi_false;
	}

	// GSIACAvailable
	return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Commerce requires authentication against the Gamespy AuthService
static gsi_bool sampleAuthPlayer()
{
	int result = wsLoginUnique(SAMPLE_GAMEID, SAMPLE_PARTERCODE, SAMPLE_NAMESPACEID, LOCAL_PLAYER_UNIQUENICK, LOCAL_PLAYER_PASSWORD, "", sampleAuthCallback, NULL);
	if (result != WSLogin_Success)
	{
		wprintf(L"Failed to start wsLoginUnique: %d\r\n", result);
		return gsi_false;
	}

	sampleGameAppData.gWaitingForAuth = gsi_true;
	while(gsi_is_true(sampleGameAppData.gWaitingForAuth))
	{
		msleep(10);
		gsCoreThink(0);
	}
    return sampleGameAppData.gLoggedIn;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleAuthCallback(GHTTPResult result, WSLoginResponse * response, void * userData)
{
	// Check for HTTP errors
	if (result != GHTTPSuccess)
	{
		wprintf(L"HTTP error when logging in: %d\r\n", result);
		//gLoggedIn = gsi_false;
		sampleGameAppData.gLoggedIn = gsi_false;
	}

	// Check server result....invalid password etc
	else if (response->mLoginResult != WSLogin_Success)
	{
		wprintf(L"Login failed, server reported: %d\r\n", response->mLoginResult);
        sampleGameAppData.gLoggedIn = gsi_false;

	}
	else
	{
		wprintf(L"Logged in as %d (%S)\r\n", response->mCertificate.mProfileId, response->mCertificate.mUniqueNick);
		memcpy(&sampleGameAppData.gLoginCertificate, &response->mCertificate, sizeof(GSLoginCertificate));
		memcpy(&sampleGameAppData.gLoginPrivData, &response->mPrivateData, sizeof(GSLoginPrivateData));
        sampleGameAppData.gLoggedIn = gsi_false;
	}
	sampleGameAppData.gWaitingForAuth = gsi_false;
	GSI_UNUSED(userData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void printItemValidationCode(GSResult itemValidationCode)
{
	if (GS_RESULT_CODE(itemValidationCode) == D2GResultCode_OrderItem_QuantityRestriction)
	{
		wprintf(L"Item already purchased or is over quantity limit\n");
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleCheckStoreAvailabilityCallback(GSResult result, D2GGetStoreAvailabilityResponse *response, void *userData)
{
    wprintf(L"\n=====================BEGIN GET STORE AVAILABILITY RESPONSE=====================\r\n");

	if (GS_FAILED(result))
	{
		wprintf(L"Failed on  d2gGetStoreAvailability(0x%8x)\n", result);
	}
	else
	{
		D2GResultCode availableCode;
		availableCode =  GS_RESULT_CODE(response->mAvailabilityCode);
		if (availableCode == D2GResultCode_StoreOnline)
		{
			wprintf(L"Store is online\n");
		}
		else if (availableCode == D2GResultCode_StoreOfflineForMaintenance)
		{
			wprintf(L"Store is offline for maintenance\n");
		}
		else if (availableCode == D2GResultCode_StoreOfflineRetired)
		{
			wprintf(L"Store is offline and retired\n");
		}
		else if (availableCode == D2GResultCode_StoreNotYetLaunched)
		{
			wprintf(L"Store is coming soon!\n");
		}
	}
    wprintf(L"======================END GET STORE AVAILABILITY RESPONSE=======================\r\n");

	GSI_UNUSED(userData);
	sampleGameAppData.gWaiting = gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleCheckStoreAvailability(D2GInstancePtr instance,
                                  D2GCatalogPtr d2gCatalog)
{
	GSResult result;
	wprintf(L"Getting store availability...\n");
	
	result = d2gGetStoreAvailability(instance, d2gCatalog, sampleCheckStoreAvailabilityCallback, NULL);
	if (GS_FAILED(result))
	{
		wprintf(L"Failed calling d2gBeginGetStoreAvailability (0x%8x)\n", result);
		return;
	}

	sampleGameAppData.gWaiting = gsi_true;
	while(sampleGameAppData.gWaiting)
	{
		msleep(10);
		gsCoreThink(0);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void sampleAppInitializeDownloadList()
{
	int i = 0;
	sampleGameAppData.gDownloads.downloadCount = 0;
    for (i = 0; i< MAX_FILEID_DOWNLOADS; i++ )
	{
        sampleGameAppData.gDownloads.downloads[i].itemId = 0;
        sampleGameAppData.gDownloads.downloads[i].urlId  = 0;
		sampleGameAppData.gDownloads.downloads[i].fileId = 0;
        sampleGameAppData.gDownloads.downloads[i].version = 0.0;

	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleAppUpdateDownloadList(D2GItemId itemId, D2GUrlId urlId, D2GFileId fileId, float version)
{
	if (sampleGameAppData.gDownloads.downloadCount< MAX_FILEID_DOWNLOADS)
	{
		int index = sampleGameAppData.gDownloads.downloadCount++ ;
        sampleGameAppData.gDownloads.downloads[index].itemId = itemId;
        sampleGameAppData.gDownloads.downloads[index].urlId  = urlId;
		sampleGameAppData.gDownloads.downloads[index].fileId = fileId;
        sampleGameAppData.gDownloads.downloads[index].version = version;
		wprintf(L"sampleAppUpdateDownloadList inserted FileId %d. Download Count %d.\n", fileId, (index+1));
	}
	else
	{
		wprintf(L"sampleAppUpdateDownloadList FAILED to insert FileId. Maximum number is reached.\n");
	}

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void traverseOrderItemPurchases(D2GOrderItemPurchase *items)
{
    if (items != NULL)
    {
        gsi_u32 i,k;
        D2GOrderItem anOrderItem = items->mOrderItem ;

        wprintf(L"-----------------------------------------------------------------\r\n");
        if (anOrderItem.mValidation.mIsValid)
            wprintf(L"Valid\r\n");
        else
            wprintf(L"Invalid, check result code in validation\r\n");
        wprintf(L"\t          Item name: %s\r\n", anOrderItem.mItem.mName);
        wprintf(L"\t            Item Id: %d\r\n", anOrderItem.mItem.mItemId);
        wprintf(L"\t Item External Code: %s\r\n", anOrderItem.mItem.mExternalItemCode);
        wprintf(L"\t   Item Unite Price: %s\r\n", anOrderItem.mItem.mPrice);
        wprintf(L"\t           Item Tax: %s\r\n", anOrderItem.mItem.mTax);
        wprintf(L"\t      Item Quantity: %d\r\n", anOrderItem.mItemTotal.mQuantity);
        wprintf(L"\t      Item Subtotal: %s\r\n", anOrderItem.mItemTotal.mSubTotal);
        wprintf(L"\t         Item Total: %s\r\n", anOrderItem.mItemTotal.mTotal);

        // Print licenses  here
        wprintf(L"\n\t Number of Licenses: %d\r\n\n", items->mLicenseList.mCount);
        for (k= 0 ; k < items->mLicenseList.mCount; k++)
        {
            wprintf(L"\t         License %i:\r\n", k);
            wprintf(L"\t      Licenses Name: %s\r\n", items->mLicenseList.mLicenses[k].mLicenseName);
            wprintf(L"\t        License Key: %s\r\n\n", items->mLicenseList.mLicenses[k].mLicenseKey);
        }                    
        // Print Downloads  here
        wprintf(L"\tNumber of Downloads: %d\r\n\n", items->mDownloadList.mCount);
        for (k= 0 ; k < items->mDownloadList.mCount; k++)
        {
            wprintf(L"\t        Download %i:\r\n", k);

			wprintf(L"\tDownload Asset Type: %s\r\n", items->mDownloadList.mDownloads[k].mAssetType ? items->mDownloadList.mDownloads[k].mAssetType : L"");
            wprintf(L"\t      Download Name: %s\r\n", items->mDownloadList.mDownloads[k].mName);
            wprintf(L"\t    Download URL Id: %i\r\n", items->mDownloadList.mDownloads[k].mUrlId);
            wprintf(L"\t   Download File Id: %i\r\n", items->mDownloadList.mDownloads[k].mFileId);
            wprintf(L"\t   Download Version: %f\r\n", items->mDownloadList.mDownloads[k].mVersion);
            wprintf(L"\t  Download Sequence: %i\r\n", items->mDownloadList.mDownloads[k].mSequence);
			
			// Call the sample's function to update the download list
			// Note this is only to provide an example
            sampleAppUpdateDownloadList(anOrderItem.mItem.mItemId,
                                        items->mDownloadList.mDownloads[k].mUrlId,
                                        items->mDownloadList.mDownloads[k].mFileId,
                                        items->mDownloadList.mDownloads[k].mVersion);
        }              

        for (i = 0; i < items->mPurchaseList.mCount; i++)
        {
            traverseOrderItemPurchases(&items->mPurchaseList.mOrderItemPurchases[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleDisplayCatalogItem(D2GCatalogItem *anItem)
{
	gsi_u32 j = 0;

	wprintf(L"\t            Name:%s\r\n"
		L"\t         Item Id:%i\r\n"
		L"\t       Developer:%s\r\n"
		L"\t       Publisher:%s\r\n"
		L"\t           Price:%s\r\n"
		L"\t             Tax:%s\r\n"
		L"\t         Culture:%s\r\n"
		L"\t        Currency:%s\r\n"
		L"\t         Summary:%s\r\n", 
		anItem->mItem.mName, 
		anItem->mItem.mItemId,
		anItem->mProductInfo.mDeveloper,
		anItem->mProductInfo.mPublisher,
		anItem->mItem.mPrice,
		anItem->mItem.mTax,
		anItem->mGeoInfo.mCultureCode,
		anItem->mGeoInfo.mCurrencyCode,
		anItem->mProductInfo.mSummary);
	printf("\t    Release Date:%s\r\n",gsiSecondsToString(&anItem->mProductInfo.mReleaseDate));
	wprintf(L"\t Total File Size:%i\r\n",anItem->mProductInfo.mFileSize);

	if (wcslen(anItem->mItem.mExternalItemCode) > 0)
	{
		wprintf(L"\tExternalItemCode:%s\r\n", anItem->mItem.mExternalItemCode);				
	}

	wprintf(L"\t     Image count: %d\r\n", anItem->mImages.mCount);
	for (j = 0; j < anItem->mImages.mCount; j++)
	{
		wprintf(L"\t        ImageUrl:%s\r\n", anItem->mImages.mImageName[j]);
	}		 

	wprintf(L"\t  Category count: %d\r\n", anItem->mCategories.mCount);
	for (j = 0; j < anItem->mCategories.mCount; j++)
	{
		wprintf(L"\t        Category:%s\r\n", anItem->mCategories.mCategoryNames[j]);
	}

	wprintf(L"\t  Extra Info key/value pairs count: %d\r\n", anItem->mExtraItemInfoList.mCount);
	for (j = 0; j < anItem->mExtraItemInfoList.mCount; j++)
	{
		UCS2String myValue;
		GSResult result;
		wprintf(L"\t        ExtraInfo key %d: %s\r\n", j, anItem->mExtraItemInfoList.mExtraInfoElements[j].mKey);
		result = d2gGetExtraItemInfoKeyValueByKeyName(anItem, anItem->mExtraItemInfoList.mExtraInfoElements[j].mKey, &myValue);
		if (GS_SUCCEEDED(result))
		{
			wprintf(L"\t        ExtraInfo value %d: %s\r\n", j, myValue);
			gsifree(myValue);
		}		
	}

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleShowAllItemsCallback(GSResult result, 
								D2GLoadCatalogItemsResponse * response, 
								void * userData)
{
    wprintf(L"\n=======================BEGIN GET ALL ITEMS RESPONSE=======================\r\n");

	if (GS_FAILED(result))
	{
		wprintf(L"d2gBeginLoadCatalogItems failed! Result = %X\r\n", result);
	}
	else
	{
		if (response->mItemList)
		{
			gsi_u32 i;
			D2GCatalogItemList *lItemList, *lItemList2;
			D2GCatalogItemList *filteredItems = NULL;
			gLocalItemCount = SAMPLE_MAX_PURCHASES < response->mItemList->mCount ? 
							  SAMPLE_MAX_PURCHASES : 
							  response->mItemList->mCount;
			gLocalItemIds = (D2GItemId *)gsimalloc(sizeof(D2GItemId) * gLocalItemCount);
			gLocalItemQuantities = (gsi_u32 *)gsimalloc(sizeof(gsi_u32) * gLocalItemCount);


			wprintf(L"Got All Items, Items shown below\r\n");
			wprintf(L"Number of items: %d\r\n", response->mItemList->mCount);
			for (i = 0; i < response->mItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &response->mItemList->mCatalogItems[i];
				// save itemId for use later
				if (gLocalItemIds && i < gLocalItemCount)
				{
					gLocalItemIds[i] = anItem->mItem.mItemId;
					gLocalItemQuantities[i] = 1;
				}

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);
				sampleDisplayCatalogItem(anItem);

			}

			lItemList = response->mItemList;
			lItemList2 = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));

			//For testing d2gAppendCatalogItems
			d2gCloneCatalogItemList(lItemList2, response->mItemList);
			d2gAppendCatalogItemList(lItemList, lItemList2);

			printf("Sort by name in descending order \n");
			d2gSortCatalogItemsbyName(lItemList,D2GSort_Descending);
			printf("\n");

			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);

			}

			printf("Sort by price in ascending order\n");
			d2gSortCatalogItemsbyPrice(lItemList,D2GSort_Ascending);

			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);

			}

			printf("Sort by release date in descending order \n");
			d2gSortCatalogItemsbyReleaseDate(lItemList,D2GSort_Descending);
			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);
			}    
			printf("Sort by item id in descending order \n");
			d2gSortCatalogItemsbyItemId(lItemList,D2GSort_Descending);
			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);
			}

			printf("Sort by external id in descending order \n");
			d2gSortCatalogItemsbyExternalId(lItemList,D2GSort_Descending);
			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);
			}

			printf("Sort by file size in descending order \n");
			d2gSortCatalogItemsbySize(lItemList,D2GSort_Descending);
			for (i = 0; i < lItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &lItemList->mCatalogItems[i];

				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);

				sampleDisplayCatalogItem(anItem);
			}
			
 			d2gFilterCatalogItemListByKeyName(response->mItemList, &filteredItems, gSampleExtraInfoKeyName);
			if (filteredItems != NULL)
			{
				for (i = 0; i < filteredItems->mCount; i++)
				{
					D2GCatalogItem *anItem = &filteredItems->mCatalogItems[i];

					wprintf(L"==================================================\r\n");
					wprintf(L"Item # %d\r\n", i);

					sampleDisplayCatalogItem(anItem);
				}
				d2gFreeCatalogItemList(filteredItems);
				filteredItems = NULL;
			}
			else 
			{
				wprintf(L"NOTE: List returned with empty results!");
			}
			d2gFilterCatalogItemListByKeyNameValue(response->mItemList, &filteredItems, gSampleExtraInfoKeyName, 
				L"");//gSampleExtraInfoKeyValue);
			if (filteredItems != NULL)
			{
				for (i = 0; i < filteredItems->mCount; i++)
				{
					D2GCatalogItem *anItem = &filteredItems->mCatalogItems[i];

					wprintf(L"==================================================\r\n");
					wprintf(L"Item # %d\r\n", i);

					sampleDisplayCatalogItem(anItem);
				}	
			}
			else 
			{
				wprintf(L"NOTE: List returned with empty results!");
			}

			// Clean up the list and any references to it that the callback gave us.
			d2gFreeCatalogItemList(lItemList);
			lItemList = NULL;
			response->mItemList = NULL;

			//For testing d2gAppendCatalogItems
			d2gFreeCatalogItemList(lItemList2);
			lItemList2 = NULL;

			// Clean up the filtered Item list based on extra info key
			d2gFreeCatalogItemList(filteredItems);
			filteredItems = NULL;
		}				
	}
	
	wprintf(L"=======================END GET ALL ITEMS RESPONSE=======================\r\n");
	sampleGameAppData.gWaiting = gsi_false;
	GSI_UNUSED(userData);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// This is sample app to retrieve categories from the catalog
//
void sampleGetCategories(D2GInstancePtr d2gInstance,
                         D2GCatalogPtr  d2gCatalog)
{ 
    GSResult result = GS_SUCCESS;
    D2GCategoryList categoryList;
    gsi_u32 i = 0;
    
    result = d2gGetCategories(d2gInstance, d2gCatalog, &categoryList);
    wprintf(L"=======================BEGIN CATEGORIES =======================\r\n");
    if (GS_SUCCEEDED(result))
    {
        for (i= 0; i< categoryList.mCount; i++)
        {
            wprintf(L"%s \r\n", categoryList.mCategoryNames[i]);
        }
    }
    else
    {
        wprintf(L" Failed to get Categories . Result code = 0x%x", result);
    }
    wprintf(L"=======================END CATEGORIES =========================\r\n");
    
    ////////
    // IMPORTANT: release the memory allocated for category list with 
    // gsifree() function
    // 
    gsifree(categoryList.mCategoryNames);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleShowAllItems(D2GInstancePtr d2gInstance,
                        D2GCatalogPtr  d2gCatalog)
{
	GSResult result;
	sampleGameAppData.gWaiting = gsi_true;
	result = d2gLoadCatalogItems(d2gInstance, d2gCatalog, sampleShowAllItemsCallback, NULL);
	if (GS_FAILED(result))
	{
		wprintf(L"Failed on d2gBeginLoadCatalogItems (0x%8x)\n", result);
		return;
	}
	
	while(sampleGameAppData.gWaiting)
	{
		msleep(10);
		gsCoreThink(0);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleShowItemsByCategoryCallback(GSResult result, D2GLoadCatalogItemsByCategoryResponse * response, void * userData)
{
    wprintf(L"\n===================BEGIN GET ITEMS BY CATEGORY RESPONSE=======================\r\n");

	if (GS_FAILED(result))
	{
		wprintf(L"d2gBeginGetItemsBy Category failed! Result = %X\r\n", result);
	}
	else
	{
		if (response->mItemList)
		{
			UCS2String sampleCategory;
			gsi_u32 i;
		
			sampleCategory = (UCS2String)userData;
			wprintf(L"Got Items in category %s, Items shown below\r\n", sampleCategory);
			wprintf(L"Number of items: %d\r\n", response->mItemList->mCount);

			for (i = 0; i < response->mItemList->mCount; i++)
			{
				D2GCatalogItem *anItem = &response->mItemList->mCatalogItems[i];
				
				// save itemId for use later
				wprintf(L"==================================================\r\n");
				wprintf(L"Item # %d\r\n", i);
                sampleDisplayCatalogItem(anItem);
			}
        }
		else
			wprintf(L"Successfully got data, but list was empty\n");
		// Clear out catalog item list that we won't be using later explicitly
		// because the callback gives us control of the list and we're not 
		// referencing it later.
		d2gFreeCatalogItemList(response->mItemList);
        response->mItemList = NULL;
	}
	wprintf(L"===================END GET ITEMS BY CATEGORY RESPONSE=======================\r\n");
	sampleGameAppData.gWaiting = gsi_false;
	GSI_UNUSED(userData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleShowItemsByCategory(D2GInstancePtr d2gInstance, 
                               D2GCatalogPtr  d2gCatalog,
                               const UCS2String aCategory)
{
	GSResult result;	
	sampleGameAppData.gWaiting = gsi_true;
	result = d2gLoadCatalogItemsByCategory(d2gInstance, d2gCatalog, aCategory, sampleShowItemsByCategoryCallback, (void *)aCategory);
	if (GS_FAILED(result))
	{
		wprintf(L"Failed on d2gLoadCatalogItemsByCategory (0x%8x)\n", result);
		return;
	}
	
	while(sampleGameAppData.gWaiting)
	{
		msleep(10);
		gsCoreThink(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetUserCreditCardsCallback(GSResult result, D2GGetUserCreditCardsResponse * response, void * userData)
{

    wprintf(L"\n====================BEGIN GET CREDIT CARDS RESPONSE=======================\r\n");

	if (GS_FAILED(result))
	{
		wprintf(L"d2gBeginGetUserCreditCards failed! Result = %X\r\n", result);
	}
	else
	{
		wprintf(L"=======================BEGIN GET CREDIT CARDS RESPONSE=======================\r\n");
		if (response->mListOfCreditCards->mCount)
		{
			gsi_u32 i;
			
			wprintf(L"Got credit cards, number of accounts: %d\r\n", response->mListOfCreditCards->mCount);
			for (i = 0; i < response->mListOfCreditCards->mCount; i++)
			{
				D2GCreditCardInfo *cinfo = &response->mListOfCreditCards->mCreditCardInfos[i];
				struct tm *expire = gsiSecondsToDate(&cinfo->mExpirationDate);
				
				wprintf(L"==================================================\r\n");
				wprintf(L"Account %d\r\n", i);
				wprintf(L"\t            AccountId: %d\r\n"
						L"\t     Credit Card Type: %s\r\n"
						L"\t          Expire date: %d-%d\r\n"
						L"\t              Default: %s\r\n"
						L"\t     Last four digits: %d\r\n",
						cinfo->mAccountId, 
						cinfo->mCreditCardType,
						expire->tm_year + 1900, expire->tm_mon + 1,
						cinfo->mIsDefault ? L"true" : L"false",
						cinfo->mLastFourDigits);
				if (i == 0)
					gLocalAccountId = cinfo->mAccountId;
			}

			
		}
		
		// since we got the account id, we won't need any of the 
		// rest of the account info.
		d2gFreeCreditCardInfoList(response->mListOfCreditCards);
		response->mListOfCreditCards = NULL;
	}
	
    wprintf(L"======================END GET CREDIT CARDS RESPONSE=========================\r\n");

	sampleGameAppData.gWaiting = gsi_false;
	GSI_UNUSED(response);
	GSI_UNUSED(userData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetUserCreditCards(D2GInstancePtr d2gInstance,
                              D2GCatalogPtr  d2gCatalog)
{
	GSResult result;
	gsi_bool validOnly = gsi_false;

	result = d2gGetUserCreditCards(d2gInstance, d2gCatalog, validOnly, sampleGetUserCreditCardsCallback, NULL);
	if (GS_FAILED(result))
	{	
		wprintf(L"Failed on d2gBeginGetUserCreditCards (0x%8x)\n", result);
		return;
	}

	sampleGameAppData.gWaiting = gsi_true;
	while(sampleGameAppData.gWaiting)
	{
		msleep(10);
		gsCoreThink(0);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetOrderTotalCallback(GSResult result, D2GGetOrderTotalResponse * response, void * userData)
{

    wprintf(L"\n=====================BEGIN GET ORDER TOTAL RESPONSE=========================\r\n");

	if (GS_FAILED(result))
	{
        wprintf(L"sampleGetOrderTotal failed! Result = %X\r\n", result);

        // Error handling should be done here.
        // We also need to release the memory allocated during the response.
        d2gFreeOrderTotal(response->mOrderTotal);
        response->mOrderTotal = NULL;
	}
	else
	{
		if (response->mOrderTotal)
		{
			// The order in the response is a pointer to the pending order that the SDK stores.
			gsi_u32 i;
			
			wprintf(L"sampleGetOrderTotal successful, order listed below: \r\n");

			wprintf(L"\t      Account Id: %d\r\n", response->mOrderTotal->mOrder.mAccountId);
            wprintf(L"\t         Culture: %s\r\n", response->mOrderTotal->mOrder.mGeoInfo.mCultureCode);
            wprintf(L"\t        Currency: %s\r\n", response->mOrderTotal->mOrder.mGeoInfo.mCurrencyCode);
			wprintf(L"\t        Subtotal: %s\r\n", response->mOrderTotal->mOrder.mSubTotal);
			wprintf(L"\t             Tax: %s\r\n", response->mOrderTotal->mOrder.mTax);
			wprintf(L"\t           Total: %s\r\n", response->mOrderTotal->mOrder.mTotal);
            wprintf(L"\t Root Order Guid: %s\r\n", response->mOrderTotal->mOrder.mRootOrderGuid);
            printf("\t     Date Quoted: %s\n", gsiSecondsToString(&response->mOrderTotal->mQuoteDate));

			if (response->mOrderTotal->mOrder.mValidation.mIsValid)
				wprintf(L"Order is valid\r\n");
			else
				wprintf(L"Order is invalid, check result code and message\r\n");

			
			wprintf(L"Number of items in order total: %d\r\n", response->mOrderTotal->mOrderItemList.mCount);
			for (i = 0; i < response->mOrderTotal->mOrderItemList.mCount; i++)
			{
			    D2GOrderItem    pOrderItem = response->mOrderTotal->mOrderItemList.mOrderItems[i];
			    
				wprintf(L"=====================\r\n");
				wprintf(L"Item %d, ", i);
				if (pOrderItem.mValidation.mIsValid)
					wprintf(L"Valid\r\n");
				else
				{
					wprintf(L"Invalid\r\n");
					printItemValidationCode(pOrderItem.mValidation.mResult);
				}
				wprintf(L"\t          Item name: %s\r\n", pOrderItem.mItem.mName);
				wprintf(L"\t            Item Id: %d\r\n", pOrderItem.mItem.mItemId);
				wprintf(L"\t Item External Code: %s\r\n", pOrderItem.mItem.mExternalItemCode);
				wprintf(L"\t   Item Unite Price: %s\r\n", pOrderItem.mItem.mPrice);
				wprintf(L"\t     Item Unite Tax: %s\r\n", pOrderItem.mItem.mTax);
				wprintf(L"\t      Item Quantity: %d\r\n", pOrderItem.mItemTotal.mQuantity);
				wprintf(L"\t      Item Subtotal: %s\r\n", pOrderItem.mItemTotal.mSubTotal);
				wprintf(L"\t         Item Total: %s\r\n", pOrderItem.mItemTotal.mTotal);
				
			}
			
			if (response->mOrderTotal->mOrder.mValidation.mIsValid)
			{				
				// IMPORTANT: We need to keep track of the Order Total because it will be 
				// used to confirm the placement of an order.  Once the order is successfully 
				// started, we destroy it.
				gOrderTotal =  response->mOrderTotal;
			}
            else
            {
                // Since the order is returned as invalid. Error handling should be done here.
                // We also need to release the memory allocated during the response.
                d2gFreeOrderTotal(response->mOrderTotal);
                response->mOrderTotal = NULL;
            }
			
			
		}
		wprintf(L"==========================END GET ORDER TOTAL RESPONSE==========================\r\n");
	}
    wprintf(L"======================END GET ORDER TOTAL RESPONSE==========================\r\n");
	
	GSI_UNUSED(response);
	GSI_UNUSED(userData);
	sampleGameAppData.gWaiting = gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetOrderTotal(D2GInstancePtr d2gInstance, D2GCatalogPtr d2gCatalog)
{
	GSResult result;
	
	D2GItemId *itemIds = gLocalItemIds;
	gsi_u32 *itemQuantities = gLocalItemQuantities;
	gsi_u32 itemCount = gLocalItemCount;
	
	if (gLocalItemIds)
	{
        result = d2gGetOrderTotal(d2gInstance, 
                                  d2gCatalog,
                                  gLocalAccountId,
                                  itemIds, 
                                  itemCount,
                                  itemQuantities,
                                  sampleGetOrderTotalCallback, 
                                  NULL);
		if (GS_FAILED(result))
		{
			wprintf(L"Failed on sampleGetOrderTotal (0x%8x)\n", result);
			return;
		}

		sampleGameAppData.gWaiting = gsi_true;
		while(sampleGameAppData.gWaiting)
		{
			msleep(10);
			gsCoreThink(0);
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleStartOrderCallback(GSResult result, D2GStartOrderResponse * response, void * userData)
{

    wprintf(L"\n=====================BEGIN PURCHASE RESPONSE=========================\r\n");
	if (GS_FAILED(result))
	{
		wprintf(L"sampleBeginPurchase failed! Result = %X\r\n", result);
	}
	else
	{
		if (response->mOrderPurchase)
		{
			// The order in the response is a pointer to the purchased order that the SDK stores.
			gsi_u32 i;
			wprintf(L"sampleGetOrderTotal successful, order listed below: \r\n");

			wprintf(L"\t      Account Id: %d\r\n", response->mOrderPurchase->mOrder.mAccountId);
            wprintf(L"\t        Currency: %s\r\n", response->mOrderPurchase->mOrder.mGeoInfo.mCurrencyCode);
            wprintf(L"\t         Culture: %s\r\n", response->mOrderPurchase->mOrder.mGeoInfo.mCultureCode);
	        wprintf(L"\t        Subtotal: %s\r\n", response->mOrderPurchase->mOrder.mSubTotal);
			wprintf(L"\t             Tax: %s\r\n", response->mOrderPurchase->mOrder.mTax);
			wprintf(L"\t           Total: %s\r\n", response->mOrderPurchase->mOrder.mTotal);
            wprintf(L"\t Root Order Guid: %s\r\n", response->mOrderPurchase->mOrder.mRootOrderGuid);
            printf("\t  Date Purchased: %s\r\n", gsiSecondsToString(&response->mOrderPurchase->mPurchaseDate));

			if (response->mOrderPurchase->mOrder.mValidation.mIsValid)
				wprintf(L"Order is valid\r\n");
			else
			{
				wprintf(L"Order is invalid\r\n");
				wprintf(L"Check order items for individual validations");
			}

			wprintf(L"Number of items in order total: %d\r\n", response->mOrderPurchase->mItemPurchases.mCount);
			for (i = 0; i < response->mOrderPurchase->mItemPurchases.mCount; i++)
			{
			    D2GOrderItemPurchase anItemPurchase = response->mOrderPurchase->mItemPurchases.mOrderItemPurchases[i];
				wprintf(L"=====================\r\n");
				wprintf(L"Item %d, ", i);
				if (anItemPurchase.mOrderItem.mValidation.mIsValid)
					wprintf(L"Valid\r\n");
				else				
					wprintf(L"Invalid\r\n");
				
				// Initialize the sample applications download file count here 
				// Since the traverseOrderItemPurchases will call sampleAppUpdateDownloadList()
				sampleAppInitializeDownloadList();
                traverseOrderItemPurchases(&response->mOrderPurchase->mItemPurchases.mOrderItemPurchases[i]);
			}

			if (response->mOrderPurchase->mOrder.mValidation.mIsValid)
			{
				// IMPORTANT: We need to keep track of the Order Purchase because it will be 
				// used during the confirmation of an order.  Once the order is successfully 
				// confirmed, we destroy it.
				gOrderPurchase = response->mOrderPurchase;
			}
            else
            {
                // Since the order is invalid, if no longer needed, cleanup the response object here.
                d2gFreeOrderPurchase(response->mOrderPurchase);
                response->mOrderPurchase = NULL;
            }
			
		}

	}

	// We're freeing this here because this is a test.  
	// But Developers needs to free this at their discretion 
	// because they may be retrying if the request fails.
	d2gFreeOrderTotal(gOrderTotal);
	gOrderTotal = NULL;

    wprintf(L"=======================END PURCHASE RESPONSE==========================\r\n");
	GSI_UNUSED(response);
	GSI_UNUSED(userData);
	sampleGameAppData.gWaiting = gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleBeginPurchase(D2GInstancePtr d2gInstance,
                         D2GCatalogPtr  d2gCatalog)
{
	GSResult result;

	if (gOrderTotal && gOrderTotal->mOrder.mValidation.mIsValid)
	{
		// Note that the current pending order is stored by the SDK
		result = d2gStartOrder(d2gInstance, d2gCatalog, gOrderTotal, sampleStartOrderCallback, NULL);
        
		if (GS_FAILED(result))
		{
			wprintf(L"Failed on sampleGetOrderTotal (0x%8x)\n", result);
			return;
		}

		sampleGameAppData.gWaiting = gsi_true;
		while(sampleGameAppData.gWaiting)
		{
			msleep(10);
			gsCoreThink(0);
		}
	}
	else
	{
		wprintf(L"Order is invalid, please check order before proceeding\n");
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleIsOrderCompleteCallback(GSResult result, D2GIsOrderCompleteResponse * response, void * userData)
{

    wprintf(L"\n=====================BEGIN IS ORDER COMPLETE RESPONSE=========================\r\n");
	if (GS_FAILED(result))
	{
		wprintf(L"sampleIsOrderComplete failed! Result = %X\r\n", result);
	}
	else
	{
		if (response->mOrderPurchase)
		{
			// The order in the response is a pointer to the purchased order that the SDK stores.
			gsi_u32 i;
			wprintf(L"sampleIsOrderCompleteCallback successful, order listed below: \r\n");

			wprintf(L"\t      Account Id: %d\r\n", response->mOrderPurchase->mOrder.mAccountId);
            wprintf(L"\t        Currency: %s\r\n", response->mOrderPurchase->mOrder.mGeoInfo.mCurrencyCode);
            wprintf(L"\t         Culture: %s\r\n", response->mOrderPurchase->mOrder.mGeoInfo.mCultureCode);
		    wprintf(L"\t        Subtotal: %s\r\n", response->mOrderPurchase->mOrder.mSubTotal);
			wprintf(L"\t             Tax: %s\r\n", response->mOrderPurchase->mOrder.mTax);
			wprintf(L"\t           Total: %s\r\n", response->mOrderPurchase->mOrder.mTotal);
            wprintf(L"\t Root Order Guid: %s\r\n", response->mOrderPurchase->mOrder.mRootOrderGuid);
            printf("\t  Date Purchased: %s\r\n", gsiSecondsToString(&response->mOrderPurchase->mPurchaseDate));

			if (response->mOrderPurchase->mOrder.mValidation.mIsValid)
				wprintf(L"Order is valid\r\n");
			else
				wprintf(L"Order is invalid\r\n");

            if (GS_SUCCEEDED(response->mOrderPurchase->mOrder.mValidation.mResult))
            {
                switch (GS_RESULT_CODE(response->mOrderPurchase->mOrder.mValidation.mResult))
                {
					// We're handling two cases here for now.
					// Other cases will be handled in this app accordingly in the future.
					case D2GResultCode_Order_Ok: 
						// we should be able to download the file after this 
						printf("\t      Order Status: Completed.\n");
						sampleGameAppData.gPurchaseComplete = gsi_true;
						break;
					case D2GResultCode_Order_BillingPending:
						printf("\t      Order Status: Billing Pending.\n");
						break;
					default:
						printf("Order succeeded with an unknown status code\n");
                }
            }


			wprintf(L"Number of items in order total: %d\r\n", response->mOrderPurchase->mItemPurchases.mCount);
			for (i = 0; i < response->mOrderPurchase->mItemPurchases.mCount; i++)
			{
			   D2GOrderItemPurchase anItemPurchase = response->mOrderPurchase->mItemPurchases.mOrderItemPurchases[i];
			    
				wprintf(L"=====================\r\n");
				wprintf(L"Item %d, ", i);
				
				if (anItemPurchase.mOrderItem.mValidation.mIsValid)
					wprintf(L"Valid\r\n");
				else
					wprintf(L"Invalid, check result code in validation\r\n");

				sampleAppInitializeDownloadList();
                traverseOrderItemPurchases(&response->mOrderPurchase->mItemPurchases.mOrderItemPurchases[i]);

			}

		
           // Free the confirmed order purchase also since we don't have a use for it
           // after the callback.
           d2gFreeOrderPurchase(response->mOrderPurchase);
           response->mOrderPurchase = NULL;
        }

	}

   
    wprintf(L"======================END IS ORDER COMPLETE RESPONSE==========================\r\n");

	GSI_UNUSED(userData);
	sampleGameAppData.gWaiting = gsi_false;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleIsOrderComplete(D2GInstancePtr theInstance,
                           D2GCatalogPtr  theCatalog)
{
	GSResult result;

	sampleGameAppData.gPurchaseComplete = gsi_false;
	if (gOrderPurchase && gOrderPurchase->mOrder.mValidation.mIsValid)
	{
		// Note that the current pending order is stored by the SDK
		// Call this function 2 seconds after the callback to place order is received
		// Call this function subsequently no more than once every 5 seconds 
		result = d2gIsOrderComplete(theInstance, 
									theCatalog, 
									gOrderPurchase, 
									sampleIsOrderCompleteCallback, 
									NULL);
		if (GS_FAILED(result))
		{
			wprintf(L"Failed on sampleIsOrderComplete (0x%8x)\n", result);
			return;
		}

		sampleGameAppData.gWaiting = gsi_true;
		while(sampleGameAppData.gWaiting)
		{
			msleep(10);
			gsCoreThink(0);
		}
	}
	else
	{
		wprintf(L"sampleIsOrderComplete: Order is invalid, please check order before proceeding\n");
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleIsOrderCompleteCheck(D2GInstancePtr theInstance,
								D2GCatalogPtr  theCatalog)
{

	// Note that this is just a simple example of how the IsOrderComplete could be used
	if (gOrderPurchase && gOrderPurchase->mOrder.mValidation.mIsValid)
	{
	    int retrialCount = 3;
		int i = 0;
		sampleGameAppData.gPurchaseComplete = gsi_false;
		for (i = 0; ((i< retrialCount) && !sampleGameAppData.gPurchaseComplete); i++) 
		{
			// Call this function 2 seconds after the callback to place order is received
			// Call this function subsequently no more than once every 5 seconds 
			sampleIsOrderComplete(theInstance, theCatalog);
			if (i==0)
			{
				msleep(2000);
			}
			else
			{
				msleep(5000);
			}
		}
		if (!sampleGameAppData.gPurchaseComplete)
		{
			// The Order Result codes are provided in the callback.
			wprintf(L"Order could not be completed\n");
		}

        // Free the order purchases since it is not needed anymore.
		// But developers must free this object at some point in their app.
        d2gFreeOrderPurchase(gOrderPurchase);
        gOrderPurchase = NULL;
	}
	else
	{
		wprintf(L"Order is invalid, please check order before proceeding\n");
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DownloadProgressCallback(gsi_u32 bytesReceived, gsi_u32 bytesTotal, void *userData)
{
	if ((bytesReceived * 100/ bytesTotal) %10 == 0)
		wprintf(L"sampleDownloadFileById: received %d / %d \r\n", bytesReceived, bytesTotal);
	
	GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DownloadCompleteCallback(GSResult result, gsi_char *saveFile, void *userData)
{
    wprintf(L"\n======================BEGIN DOWNLOAD FILE ID RESPONSE=========================\r\n");

	if (GS_FAILED(result))
	{
		wprintf(L"sampleDownloadFileById failed! Result = %X\r\n", result);
	}
	else
	{
		wprintf(L"sampleDownloadFileById complete with filename: %S\r\n", saveFile);
	}
    wprintf(L"======================END DOWNLOAD FILE ID RESPONSE=============================\r\n");

	sampleGameAppData.gWaiting = gsi_false;
	GSI_UNUSED(userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const gsi_char *gDownloadLocation = _T("maps\\");
void sampleDownloadFileById(D2GInstancePtr theInstance,
                            D2GCatalogPtr  theCatalog,
							D2GFileId	   fileId)
{
	GSResult result;

	if (fileId != 0)
	{
		// Keep in mind that this starts a download thread so thinking does not need to be done here.
		// The benefits are non-blocking calls, callbacks will let developer know of status and completion.

		result = d2gStartDownloadFileById(theInstance, 
			theCatalog, 
			fileId, 
			gDownloadLocation, 
			DownloadProgressCallback, 
			DownloadCompleteCallback, 
			NULL);

		sampleGameAppData.gWaiting = gsi_true;
		while(sampleGameAppData.gWaiting)
		{
			msleep(10);
			gsCoreThink(0);
		}
	}
	else
	{
		wprintf(L"sampleDownloadFileById: uninitialized fileId\n");
	}

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleDownloadListOfFilesById(D2GInstancePtr theInstance,
								   D2GCatalogPtr  theCatalog)
{
	int i = 0;

	wprintf(L"Number of Downloads available %d \n", sampleGameAppData.gDownloads.downloadCount);
	for (i=0; i< sampleGameAppData.gDownloads.downloadCount; i++)
	{
		D2GFileId fileId = sampleGameAppData.gDownloads.downloads[i].fileId;
        if (fileId != 0 )
		{
		    // we have a fileId start downloading
			sampleDownloadFileById(theInstance, theCatalog, fileId);
		}
		else
		{
			wprintf(L"sampleDownloadListOfFilesById: uninitialized fileId\n");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// This sample is for updating the manifest file after a successful installation 
// of a just purchased product. 
void sampleUpdateInstalledContent(D2GInstancePtr theInstance)
{
   int i = 0;

   wprintf(L"\n======================BEGIN UPDATE MANIFEST INSTALLED CONTENT ======================\r\n");

   wprintf(L"Number of Downloads %d \n", sampleGameAppData.gDownloads.downloadCount);
   for (i=0; i< sampleGameAppData.gDownloads.downloadCount; i++)
   {
       D2GDownloadItem  downloadItem ;
       D2GItemId        itemId = sampleGameAppData.gDownloads.downloads[i].itemId;
    
       downloadItem.mUrlId  = sampleGameAppData.gDownloads.downloads[i].urlId;
       downloadItem.mVersion = sampleGameAppData.gDownloads.downloads[i].version;
       if (downloadItem.mUrlId != 0 )
       {
           GSResult result = GS_SUCCESS;
           // we have a fileId start downloading
           result = d2gUpdateManifestInstalledContent(theInstance, itemId, &downloadItem);
           if (GS_SUCCEEDED(result))
           {
               wprintf(L"Updated Installed Content \n");
               wprintf(L"\t Item Id : %d\n", itemId);
               wprintf(L"\t  URL Id : %d\n", downloadItem.mUrlId);
               wprintf(L"\t Version : %f\n", downloadItem.mVersion);
           }
       }
       else
       {
           wprintf(L"sampleUpdateInstalledContent: uninitialized download item\n");
       }
   }
   wprintf(L"\n======================END UPDATE MANIFEST INSTALLED CONTENT ======================\r\n");

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetPurchaseHistoryCallback(GSResult  result, 
                                      D2GGetPurchaseHistoryResponse * response, 
                                      void      *userData)
{
    gsi_u32 j=0;
    
    wprintf(L"\n======================BEGIN GET PURCHASE HISTORY RESPONSE ======================\r\n");
    if (GS_FAILED(result))
    {
        wprintf(L"sampleGetPurchaseHistory failed! Result = %X\r\n", result);
    }
    else
    {
        wprintf(L"sampleGetOrderTotal successful, order listed below: \r\n");

						sampleAppInitializeDownloadList();

        for (j = 0; j < response->mPurchaseHistory->mCount; j++)
        {
            gsi_u32 i;
            wprintf(L"=========================PURCHASE ORDER[%i]=========================\r\n", j);

            // The order in the response is a pointer to the purchased order that the SDK stores.
            
            wprintf(L"\t      Account Id: %d\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mAccountId);
            wprintf(L"\t        Currency: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mGeoInfo.mCurrencyCode);
            wprintf(L"\t         Culture: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mGeoInfo.mCultureCode);
            wprintf(L"\t        Subtotal: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mSubTotal);
            wprintf(L"\t             Tax: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mTax);
            wprintf(L"\t           Total: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mTotal);
            wprintf(L"\t Root Order Guid: %s\r\n", response->mPurchaseHistory->mPurchases[j].mOrder.mRootOrderGuid);
            printf("\t  Date Purchased: %s\r\n", gsiSecondsToString(&response->mPurchaseHistory->mPurchases[j].mPurchaseDate));
 
            if (response->mPurchaseHistory->mPurchases[j].mOrder.mValidation.mIsValid)
                wprintf(L"Order is valid\r\n");
            else
                wprintf(L"Order is invalid\r\n");


            wprintf(L"Number of items in order total: %d\r\n", response->mPurchaseHistory->mPurchases[j].mItemPurchases.mCount);
            for (i = 0; i < response->mPurchaseHistory->mPurchases[j].mItemPurchases.mCount; i++)
            {
                D2GOrderItem anOrderItem = response->mPurchaseHistory->mPurchases[j].mItemPurchases.mOrderItemPurchases[i].mOrderItem;
                                
                wprintf(L"=====================\r\n");
                wprintf(L"Item %d, ", i);
                if (anOrderItem.mValidation.mIsValid)
                    wprintf(L"Valid\r\n");
                else
                    wprintf(L"Invalid, check result code in validation\r\n");

				// Initialize the gameAppData download list for the sample here 
				// since traverseOrderItemPurchases updates this list.
                traverseOrderItemPurchases(&response->mPurchaseHistory->mPurchases[j].mItemPurchases.mOrderItemPurchases[i]);
            }
            wprintf(L"=========================END OF PURCHASE ORDER[%i]=========================\r\n\n", j);
        }
		
		if (response->mPurchaseHistory)
		{
			d2gFreePurchaseHistory(response->mPurchaseHistory);
			response->mPurchaseHistory = NULL;
		}
    }
    wprintf(L"=======================END GET PURCHASE HISTORY RESPONSE========================\r\n");

    GSI_UNUSED(userData);
    sampleGameAppData.gWaiting = gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetPurchaseHistory(D2GInstancePtr theInstance,
                              D2GCatalogPtr  theCatalog)
{
    GSResult result;

    // Keep in mind that this starts a download thread so thinking does not need to be done here.
    // The benefits are non-blocking calls, callbacks will let developer know of status and completion.
    result = d2gGetPurchaseHistory(theInstance, theCatalog, sampleGetPurchaseHistoryCallback, NULL);
    sampleGameAppData.gWaiting = gsi_true;
    while(sampleGameAppData.gWaiting)
    {
        msleep(10);
        gsCoreThink(0);
    }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleLoadExtraInfoCallback( GSResult  result, 
                              D2GLoadExtraCatalogInfoResponse * response, 
                              void *    userData)
{
    gsi_u32 j=0;
    wprintf(L"\n=========================BEGIN EXTRA INFO RESPONSE %u =========================\r\n", j);

    if (GS_FAILED(result))
    {
        wprintf(L"sampleExtraInfo failed! Result = %X\r\n", result);
    }
    else
    {
		wprintf(L"sampleExtraInfo Success!\r\n");

		if (response->mExtraCatalogInfoList)
		{
			wprintf(L"Number of elements: %d\r\n", response->mExtraCatalogInfoList->mCount);
			
			for (j=0; j<response->mExtraCatalogInfoList->mCount; j++)
			{
				wprintf(L"EXTRA INFO [%i]\r\n", j);
				wprintf(L"\tKey  : %s\r\n", response->mExtraCatalogInfoList->mExtraInfoElements[j].mKey);
				wprintf(L"\tValue: %s\r\n", response->mExtraCatalogInfoList->mExtraInfoElements[j].mValue);
				wprintf(L"\r\n");
			}

			// since we are not using the extra info for other than
			// printing, we will delete the data to avoid memory leaks
			d2gFreeExtraInfoList(response->mExtraCatalogInfoList);
			response->mExtraCatalogInfoList = NULL;
		}        
    }
    wprintf(L"==========================END EXTRA INFO RESPONSE==========================\r\n");

    GSI_UNUSED(userData);
    sampleGameAppData.gWaiting = gsi_false;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleLoadExtraInfo(D2GInstancePtr theInstance,
                         D2GCatalogPtr  theCatalog)
{
    GSResult result;
    result = d2gLoadExtraCatalogInfo(theInstance, theCatalog, sampleLoadExtraInfoCallback, NULL);
    sampleGameAppData.gWaiting = gsi_true;
    while(sampleGameAppData.gWaiting)
    {
        msleep(10);
        gsCoreThink(0);
    }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void sampleGetExtraInfo(D2GInstancePtr theInstance,
                        D2GCatalogPtr  theCatalog,
                        UCS2String     aKey)
{
    GSResult    result;
    UCS2String  aValue = NULL;
    
    wprintf(L"=========================GET EXTRA INFO =========================\r\n" );
 
    result = d2gGetExtraCatalogInfo(theInstance, theCatalog, aKey, &aValue);
    
    if (GS_FAILED(result))
    {
        wprintf(L"Get Extra Info Failed for key <%s>: Key does not exist.\r\n", aKey);
    }
    else
    {
        wprintf(L"\tKey  : %s\r\n", aKey);
        wprintf(L"\tValue: %s\r\n", aValue);
    }
    wprintf(L"=================================================================\r\n");

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetItemActivationDataCallback(GSResult  result, 
                                         D2GGetItemActivationResponse * response, 
                                         void *    userData)
{
    gsi_u32 j=0;
    wprintf(L"\n======================BEGIN ITEM ACTIVATION DATA RESPONSE %u ======================\r\n", j);

    if (GS_FAILED(result))
    {
        wprintf(L"sampleGetItemActivationData failed! Result = 0x%X\r\n", result);
        wprintf(L"\t   HTTP result: 0x%x \r\n", response->mHttpResult);
        wprintf(L"\tStatus message: %s\r\n", response->mStatusMessage);
    }
    else
    {
		wprintf(L"sampleGetItemActivationData success! Result = %X\r\n", result);
		wprintf(L"=================================================================\r\n");
		wprintf(L"\t      Item Id  : %d\r\n", response->mItemId);
        wprintf(L"\tActivation Code: %s\r\n", response->mActivationCode?response->mActivationCode:L"");
		wprintf(L"=================================================================\r\n");

		// free strings that we will no longer be using
		// because the callback gives us control of these.
		gsifree(response->mActivationCode);
		response->mActivationCode = NULL;

		gsifree(response->mStatusMessage);
		response->mStatusMessage = NULL;

    }
    wprintf(L"=======================END ITEM ACTIVATION DATA RESPONSE=======================\r\n");

	

    GSI_UNUSED(userData);
    sampleGameAppData.gWaiting = gsi_false;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleGetItemActivationData(D2GInstancePtr theInstance,
                                 D2GCatalogPtr  theCatalog)
{
    GSResult result= GS_SUCCESS;
    D2GPackageId    aPackageId = 11336;
    result = d2gGetItemActivationData(theInstance, 
                                      theCatalog,
                                      sampleGetItemActivationDataCallback,
                                      aPackageId,
                                      NULL);
    sampleGameAppData.gWaiting = gsi_true;
    while(sampleGameAppData.gWaiting)
    {
        msleep(10);
        gsCoreThink(0);
    }                                
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// FOR TESTING ONLY !
void sampleDisplayManifestFileContents(D2GInstancePtr theInstance)
{
    wprintf(L"\n======================BEGIN MANIFEST FILE CONTENTS ======================\r\n");
    d2gDisplayManifestFileContents(theInstance);
    wprintf(L"\n========================END MANIFEST FILE CONTENTS ======================\r\n");

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleCheckContentUpdatesCallback( GSResult    result, 
                                        D2GCheckContentUpdatesResponse *response, 
                                        void        *userData)
{
    wprintf(L"\n======================BEGIN CHECK CONTENTS UPDATES RESPONSE ======================\r\n");

    if (GS_FAILED(result))
    {
        wprintf(L"sampleCheckContentUpdatesCallback failed! Result = 0x%X\r\n", result);
        wprintf(L"\t   HTTP result: 0x%x \r\n", response->mHttpResult);
    }
    else
    {
        gsi_u32 j=0;

        wprintf(L"sampleCheckContentUpdatesCallback success! Result = %X\r\n", result);

        wprintf(L"=================================================================\r\n");
        wprintf(L"\tNumber of available updates: %d\r\n", response->mContentUpdateList->mCount);
        for (j = 0 ; j<response->mContentUpdateList->mCount; j++)
        {
            wprintf(L"==================Update [%d]====================================\r\n",j);
            wprintf(L"\t        Item Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
            wprintf(L"\tDownload URL Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
            wprintf(L"\t    Change Type : %s\r\n",(response->mContentUpdateList->mContentUpdates[j].mChangeType == D2GContentRemoved)? L"Removed" : L"New/Updated");
            wprintf(L"\t        File Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mFileId);
            wprintf(L"\t       Sequence : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mSequence);
            wprintf(L"\t           Name : %s\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mName);
			wprintf(L"\t     Asset Type : %s\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mAssetType ? response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mAssetType : L"");
            wprintf(L"\t        Version : %f\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);
			switch (response->mContentUpdateList->mContentUpdates[j].mDownloadType)
			{
				case D2GDownloadOptional :
					wprintf(L"\t  Download type : Optional\r\n");
					break;
				case D2GDownloadMandatory :
					wprintf(L"\t  Download type : Mandatory\r\n");
					break;
				case D2GDownloadForced :
					wprintf(L"\t  Download type : Forced\r\n");
					break;
				default :
					wprintf(L"\t  Download type : Unidentified\r\n");
			}
        }
        wprintf(L"=================================================================\r\n");

		// The callback gives us control of the objects
		// free the object since we don't need any more.
		d2gFreeContentUpdateList(response->mContentUpdateList);
		response->mContentUpdateList = NULL;
    }
    wprintf(L"=======================END CHECK CONTENTS UPDATES RESPONSE=======================\r\n");

    GSI_UNUSED(userData);
    sampleGameAppData.gWaiting = gsi_false;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleCheckContentUpdates (D2GInstancePtr theInstance,
                                D2GCatalogPtr  theCatalog)
{
    GSResult result= GS_SUCCESS;
    result = d2gCheckContentUpdates(theInstance, 
        theCatalog,
        gsi_false,
        sampleCheckContentUpdatesCallback,
        NULL);

    sampleGameAppData.gWaiting = gsi_true;
    while(sampleGameAppData.gWaiting)
    {
        msleep(10);
        gsCoreThink(0);
    }                                
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleInstallContentFromUpdatesCallback( GSResult    result, 
                                             D2GCheckContentUpdatesResponse *response, 
                                             void        *userData)
{
   wprintf(L"\n======================BEGIN CHECK CONTENTS UPDATES RESPONSE ======================\r\n");

   if (GS_FAILED(result))
   {
       wprintf(L"sampleInstallContentFromUpdatesCallback failed! Result = 0x%X\r\n", result);
       wprintf(L"\t   HTTP result: 0x%x \r\n", response->mHttpResult);
   }
   else
   {
       gsi_u32 j=0;

       wprintf(L"sampleInstallContentFromUpdatesCallback success! Result = %X\r\n", result);

       wprintf(L"=================================================================\r\n");
       wprintf(L"\tNumber of available updates: %d\r\n", response->mContentUpdateList->mCount);
       for (j = 0 ; j<response->mContentUpdateList->mCount; j++)
       {
           wprintf(L"==================Update [%d]====================================\r\n",j);
           wprintf(L"\t        Item Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
           wprintf(L"\tDownload URL Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
           wprintf(L"\t    Change Type : %s\r\n",(response->mContentUpdateList->mContentUpdates[j].mChangeType == D2GContentRemoved)? L"Removed" : L"New/Updated");
           wprintf(L"\t        File Id : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mFileId);
           wprintf(L"\t       Sequence : %d\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mSequence);
           wprintf(L"\t           Name : %s\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mName);
			wprintf(L"\t     Asset Type : %s\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mAssetType ? response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mAssetType : L"");
			wprintf(L"\t        Version : %f\r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);
			switch (response->mContentUpdateList->mContentUpdates[j].mDownloadType)
			{
				case D2GDownloadOptional :
					wprintf(L"\t  Download type : Optional\r\n");
					break;
				case D2GDownloadMandatory :
					wprintf(L"\t  Download type : Mandatory\r\n");
					break;
				case D2GDownloadForced :
					wprintf(L"\t  Download type : Forced\r\n");
					break;
				default :
					wprintf(L"\t  Download type : Unidentified\r\n");
			}
           
           switch (response->mContentUpdateList->mContentUpdates[j].mChangeType)
           {
               case D2GContentNew:
               case D2GContentUpdated:
					// Note for downloading, refer to sampleDownloadByFileId function and how it is used.
                   wprintf(L"***Assuming Developer update is downloaded****************************\r\n");
                   wprintf(L"***Assuming Developer's Install function can be called here **********\r\n");
                   wprintf(L"==================Installed Update [%d]================================\r\n",j);
                   result = d2gUpdateManifestInstalledContent(sampleGameAppData.gD2GInstancePtr,
                                                              response->mContentUpdateList->mContentUpdates[j].mItemId,
                                                              &response->mContentUpdateList->mContentUpdates[j].mDownloadItem);
                   if (GS_SUCCEEDED(result))
                   {
                       wprintf(L"Manifest file updated with installed content successfully \r\n");
                       wprintf(L"\t         Item Id : %d \r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
                       wprintf(L"\t  Download URL Id: %d \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
                       wprintf(L"\t Download Version: %f \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);
                   }
                   else 
                   {
                       wprintf(L"Failed to update manifest file : 0x%x\r\n", result);
                       wprintf(L"\t         Item Id : %d \r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
                       wprintf(L"\t  Download URL Id: %d \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
                       wprintf(L"\t Download Version: %f \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);

                   }
                   break;
                   
               case D2GContentRemoved:
                   wprintf(L"***Assuming Developer's Uninstall function can be called here **********\r\n");
                   wprintf(L"================= Removed Content [%d]=================================\r\n",j);
                   result = d2gUpdateManifestRemovedContent(sampleGameAppData.gD2GInstancePtr, 
                                                            response->mContentUpdateList->mContentUpdates[j].mItemId,
                                                            &response->mContentUpdateList->mContentUpdates[j].mDownloadItem);
                   if (GS_SUCCEEDED(result))
                   {
                       wprintf(L"Manifest file updated for the removed content successfully \r\n");
                       wprintf(L"\t         Item Id : %d \r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
                       wprintf(L"\t  Download URL Id: %d \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
                       wprintf(L"\t Download Version: %f \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);
                   }
                   else 
                   {
                       wprintf(L"Failed to remove content from manifest file : 0x%x\r\n", result);
                       wprintf(L"\t         Item Id : %d \r\n", response->mContentUpdateList->mContentUpdates[j].mItemId);
                       wprintf(L"\t  Download URL Id: %d \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mUrlId);
                       wprintf(L"\t Download Version: %f \r\n", response->mContentUpdateList->mContentUpdates[j].mDownloadItem.mVersion);

                   }

                   break;
              default:
                   wprintf(L"\t Unknown Action %d\r\n",response->mContentUpdateList->mContentUpdates[j].mChangeType );
               
           }
       }
       wprintf(L"=================================================================\r\n");
		
		// The callback gives us control of the objects
		// free the object since we don't need any more.
		d2gFreeContentUpdateList(response->mContentUpdateList);
		response->mContentUpdateList = NULL;
	}
   wprintf(L"=======================END CHECK CONTENTS UPDATES RESPONSE=======================\r\n");
   GSI_UNUSED(userData);
   sampleGameAppData.gWaiting = gsi_false;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// The purpose of this function is to demonstrate the use of 
// install/remove helper functions in the callback
void sampleInstallContentFromUpdates (D2GInstancePtr theInstance,
                                     D2GCatalogPtr  theCatalog)
{
   GSResult result= GS_SUCCESS;
   result = d2gCheckContentUpdates(theInstance, 
       theCatalog,
       gsi_false,
       sampleInstallContentFromUpdatesCallback,
       NULL);
       
   sampleGameAppData.gWaiting = gsi_true;
   while(sampleGameAppData.gWaiting)
   {
       msleep(10);
       gsCoreThink(0);
   }                                
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sampleShutdown(D2GInstancePtr d2gInstance)
{
	d2gCleanup(d2gInstance);

	gsifree(gLocalItemQuantities);
	gsifree(gLocalItemIds);
	// Shutdown the core
	gsCoreShutdown();
	while(gsCoreIsShutdown() == GSCore_SHUTDOWN_PENDING)
	{
		gsCoreThink(0);
		msleep(10);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int test_main(int argc, char *argv[])
{	
	gsi_bool result;
	D2GInstancePtr d2gInstance = NULL;
	D2GCatalogPtr  d2gCatalog  = NULL;
#if defined(_WIN32) && defined(USE_CRTDBG)
	_CrtMemState s1, s2, s3;
#endif

    wprintf(L"=================BEGIN INITIALIZE D2G============================\r\n");

#if defined(_WIN32) && defined(USE_CRTDBG)
    _CrtMemCheckpoint( &s1 );
#endif

	// Set debug output options
	gsSetDebugFile(stdout);
	gsSetDebugLevel(GSIDebugCat_All, GSIDebugType_All, GSIDebugLevel_Debug);
	

	// enable Win32 C Runtime debugging 
	#if defined(_WIN32) && defined(USE_CRTDBG)
	{
		int tempFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		_CrtSetDbgFlag(tempFlag | _CRTDBG_LEAK_CHECK_DF);
	}
	#endif

	gsCoreInitialize();

	result = sampleBackendAvailable();
	if (gsi_is_false(result))
	{
		sampleShutdown(d2gInstance);
	}

	//strcpy(wsAuthServiceURL, "https://mwstage.gamespy.com/AuthService/AuthService.asmx");
	sampleAuthPlayer();
	
	/**
     *	Test 1 : Create D2G Instance
     */
	wprintf(L"Creating GameSpy Commerce SDK Instance\r\n");
	sampleGameAppData.gD2GInstancePtr = d2gInstance = d2gCreateInstance();
	if (d2gInstance == NULL)
	{
		wprintf(L"d2gCreateInstance failed!");
		sampleShutdown(d2gInstance);
	}
	
    /**
     *	Test 2 : Initialize D2G Instance
     */
	wprintf(L"Initializing GameSpy Commerce SDK Instance(gameid = %d)\r\n", GAME_ID);
	d2gInitialize(d2gInstance, &sampleGameAppData.gLoginCertificate, &sampleGameAppData.gLoginPrivData);

    /**
    *	Test 3 : Initialize Manifest File Path (if needed)
    */
    // If Patching Features used, the manifest file path can be set here.
    // If not set, the current working directory is use. Always the same path 
    // should be used. The path should exist.
    //d2gSetManifestFilePath( d2gInstance, "manifest\\");

    /**
     *	Test 4 : Create D2G Catalog
     */
     wprintf(L"Creating A GameSpy Commerce SDK Catalog for: \
               \r\n\t gameid: %d \r\n\t version: %d \r\n\t region: %s \
               \r\n\t access token: %s \r\n", 
               GAME_ID, GAME_CATALOG_VERSION, GAME_CATALOG_REGION, SAMPLE_ACCESS_TOKEN);
               
     
	
     d2gCatalog = d2gCreateCatalog(d2gInstance, 
         GAME_ID, 
         GAME_CATALOG_VERSION, 
         GAME_CATALOG_REGION, 
         SAMPLE_ACCESS_TOKEN);

	// Testing for different environments.

	// STAGE
	//d2gSetServiceURL(d2gInstance, "mwstage.gamespy.com/commerce/1.0/");

    /**
    *	Test 5: Check Store Availability
    */
	wprintf(L"\nChecking store availability (gameid = %d)\r\n", GAME_ID);
    sampleCheckStoreAvailability(d2gInstance, d2gCatalog);
    wprintf(L"\n===================END INITIALIZE D2G============================\r\n\n");

    /**
    *	Test 6: Extra Info
    */
    wprintf(L"\nLoading Extra Info\r\n");
    sampleLoadExtraInfo(d2gInstance, d2gCatalog);
        
    /**
     *	Test 7: Retrieve all items of a catalog
     */
    wprintf(L"\nRetrieving all items (gameid = %d)\r\n", GAME_ID);
    sampleShowAllItems(d2gInstance, d2gCatalog);
    
    /**
    *	Test 8: Retrieve all the categories from a catalog
    */
    wprintf(L"\nRetrieving Categories(gameid = %d)\r\n", GAME_ID);
    sampleGetCategories(d2gInstance,  d2gCatalog);

	/**
	*	Test 9: Retrieve items of a given category
	*/
	wprintf(L"Retrieving Items for a given Category %s\r\n", gSampleCategoryBad);
	sampleShowItemsByCategory(d2gInstance, d2gCatalog, gSampleCategoryBad);

    wprintf(L"Retrieving Items for a given Category %s\r\n", gSampleCategoryGood);
    sampleShowItemsByCategory(d2gInstance, d2gCatalog, gSampleCategoryGood);

    /**
     *	Test 10: Get Credit cards of the user
     */
 	wprintf(L"\nRetrieving user credit cards\r\n");
 	sampleGetUserCreditCards(d2gInstance, d2gCatalog);

    /**
     *	Test 11: Get an order filled in
     */
 	wprintf(L"\nGetting order total\r\n");
 	sampleGetOrderTotal(d2gInstance, d2gCatalog);

    /**
     *	Test 12: Purchase items
     */
	// NOTE: Before running this functionality, make sure to refund previous 
	// purchases of the same item.  
    wprintf(L"\nMaking a purchase\r\n");
 	sampleBeginPurchase(d2gInstance, d2gCatalog);
	
	/**
	 *	Test 13: Verify purchase 
	 */
	// NOTE: Before running this functionality, make sure to refund previous 
	// purchases of the same item.  
	wprintf(L"\nChecking placed order\r\n");
 	sampleIsOrderCompleteCheck(d2gInstance, d2gCatalog);
 	
	/**
	*	Test 14: Get the purchase history of the user
	*/
	wprintf(L"\nGetting Purchase History\n");
	sampleGetPurchaseHistory(d2gInstance, d2gCatalog);

    /**
     *	Test 15: Download by FileId
	 *	For now only do it for one fileId
     */
	wprintf(L"\nStarting download for a given file id: %d.\n",sampleGameAppData.gDownloads.downloads[0].fileId);
    sampleDownloadFileById(d2gInstance, d2gCatalog, sampleGameAppData.gDownloads.downloads[0].fileId);

	/**
	*	Test 16: Download a list of files by FileId
	*   For each download it calls sampleDownloadFileById
	*/
	wprintf(L"\nStarting download for a list of file ids.\n");
	sampleDownloadListOfFilesById(d2gInstance, d2gCatalog);
 
    /**
    *	Test 17: Update Installed Content
    *   Assumption is the installation took place after downloading
    *   a purchased item successfully. Now we need to update the
    *   installed contents for future patches available. 
    *   Note that the best practice is to update installed content
    *   after each download one at a time. The following function does
    *   this for a list of files for demonstration purposes.
    */
    wprintf(L"\nUpdate Installed Content for a list of file ids.\n");
    sampleUpdateInstalledContent(d2gInstance);

	//sampleDisplayManifestFileContents(d2gInstance);
    /**
    *	Test 18: Get Extra Info <key,value>
    */
   
    wprintf(L"\nGetting an Extra Info Item\n");
    sampleGetExtraInfo(d2gInstance, d2gCatalog, L"TOS1");
    
    sampleGetExtraInfo(d2gInstance, d2gCatalog, L"TOS");

    sampleGetExtraInfo(d2gInstance, d2gCatalog, L"MOTD");

    /**
     *	Test 19: Get Item Activation Data
     */
    wprintf(L"\nGetting Item Activation Data\n");
    sampleGetItemActivationData(d2gInstance, d2gCatalog);

   	/**
     *	Test 20: Installing/Removing Content
     */
    wprintf(L"\nInstall/Remove Content given available Updates\n" );    
	sampleInstallContentFromUpdates (d2gInstance,d2gCatalog);

	sampleDisplayManifestFileContents(d2gInstance);

    /**
    *	Test 21: Cleanup Catalog
    */
    
    d2gCleanupCatalog(d2gInstance, d2gCatalog);
    
	// Pause before exit
	sampleShutdown(d2gInstance);

#if defined(_WIN32) && defined(USE_CRTDBG)
	_CrtMemCheckpoint( &s2 );
    if ( _CrtMemDifference( &s3, &s1, &s2 ) )
    { 
        printf("s3 stats\n");
        _CrtMemDumpStatistics( &s3 );
    }
    printf ("All Stats \n");
	_CrtMemDumpAllObjectsSince( NULL );
	if (! _CrtDumpMemoryLeaks())
		printf("No Leaks\n");
#endif

	GSI_UNUSED(argc);
	GSI_UNUSED(argv);
	GSI_UNUSED(result);

	fflush(stderr);
	wprintf(L"Done - Press Enter\r\n"); 
	fflush(stdout);
	getc(stdin);

	return 0;
}
