/**
* d2gMain.c 
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "Direct2Game.h"
#include "d2gMain.h"
#include "d2gServices.h"
#include "d2gUtil.h"
#include "../common/gsAvailable.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <errno.h>

///////////////////////////////////////////////////////////////////////////////
// Initialization and Termination                                                          
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  d2gCreateInstance
//  Brief
//      Creates Direct2Game SDK instance. 
//  Parameters
//      None.
//  Return Value 
//      D2GInstancePtr  - a void pointer to the internal D2G Instance.
//  Remarks
//      This function is mandatory. This API function must be called first before 
//      any other D2G API function.
//  See Also 
//      d2gCleanup
//
D2GInstancePtr d2gCreateInstance()
{
	D2GIInstance * instance = NULL;

	gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
		"%s :: %s (%d): Creating Commerce SDK instance\n",
		__FILE__, __FUNCTION__, __LINE__);

	instance = (D2GIInstance*)gsimalloc(sizeof(D2GIInstance));
	if (instance == NULL)
		return NULL;

	// initialize the struct
	memset(instance, 0, sizeof(D2GIInstance));
	return instance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  d2gInitialize
//  Brief
//      Initializes the SDK Instance created by d2gCreateInstance. 
//  Parameters
//      theInstance     [in] This pointer is created by the d2gCreateInstance.
//      theCertificate  [in] This is the certificate obtained from authorization 
//                           service.
//      thePrivateData  [in] This is the private data obtained from authorization 
//                           service.
//  Return Value 
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code.
//  Remarks
//      This function is mandatory. 
//  See Also 
//      d2gCleanup
//
GSResult d2gInitialize(D2GInstancePtr theInstance, 
					   const GSLoginCertificate * theCertificate, 
					   const GSLoginPrivateData * thePrivateData)
{
	D2GIInstance * instance = (D2GIInstance*)theInstance;
	GS_ASSERT(instance);
	if (!instance)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	
	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	if ( theCertificate == NULL || thePrivateData == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d): called with NULL parameter",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}
   	
    memcpy(&instance->mCertificate, theCertificate, sizeof(GSLoginCertificate));
    memcpy(&instance->mPrivateData, thePrivateData, sizeof(GSLoginPrivateData));
	instance->mTimeoutMs = GS_D2G_DEFAULT_SERVICE_TIMEOUTMS;
    instance->mManifestFilePath = NULL;
    instance->mServiceURL = NULL;
    instance->mServiceURLBase = NULL;
	GSI_UNUSED(instance);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  d2gCleanup
//  Brief
//      De-allocates memory allocated by d2gCreateInstance and d2gInitialize.  
//  Parameters
//      theInstance     [in] This pointer is created by the d2gCreateInstance.
//  Return Value 
//      None.
//  Remarks
//      This function is mandatory. 
//  See Also 
//      d2gCreateInstance
//
void d2gCleanup(D2GInstancePtr theInstance)
{
	D2GIInstance * instance = (D2GIInstance*)theInstance;
	GS_ASSERT(instance);
	if (!instance)
	{	
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
		"%s :: %s (%d): called, destroying Direct2Game interface\n",
        __FILE__, __FUNCTION__, __LINE__);

    gsifree(instance->mManifestFilePath);
    instance->mManifestFilePath = NULL;

    gsifree(instance->mManifestFilePath);
    instance->mManifestFilePath = NULL;
    
    gsifree(instance->mServiceURL);
    instance->mServiceURL = NULL;

    gsifree(instance->mServiceURLBase);
    instance->mServiceURLBase = NULL;

	gsifree(instance);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Internal function referenced by d2gCreateCatalog
static void d2giCatalogCacheFreeItem(void * elem)
{
    D2GCatalogItem * item = (D2GCatalogItem *)elem;
    if (item != NULL)
        d2giResetCatalogItem(item);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Internal function referenced by d2gCreateCatalog
static void d2giExtraInfoCacheFree(void * elem)
{
    D2GExtraInfo * item = (D2GExtraInfo *)elem;
    if (item != NULL)
        d2giResetExtraInfo(item);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  d2gCreateCatalog
//  Brief
//      Creates a local catalog d2gInstance to access the backend catalog services 
//      from within the game. 
//  Parameters
//      theInstance [in] This is pointer created by the d2gCreateInstance.
//      gameId      [in] Unique game id which is pre-assigned by GameSpy Tech.
//      version     [in] A version of the Catalog for the game.
//      region      [in] An enumerated type that specifies the region for the catalog.
//      accessToken [in] A unique token that allows access to the backend catalog services.
//  Return Value 
//      D2GCatalogPtr - Pointer to the catalog d2gInstance initialized with the input parameters.
//  Remarks
//      This API function is mandatory. After this API function is called, all of the 
//      backend catalog and purchasing services are available to the game. For the D2G SDK 
//      to terminate properly, d2gCleanupCatalog must be called before calling d2gCleanup.
//  See Also 
//      d2gCleanupCatalog
//
D2GCatalogPtr d2gCreateCatalog( D2GInstancePtr  theInstance,
                                int             gameId,
                                gsi_u32         version,
                                UCS2String      region,
                                UCS2String      accessToken)
{
    D2GICatalog  *d2gCatalog  = NULL;
    D2GIInstance *d2gInstance = (D2GIInstance*)theInstance;
    
    gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
        "Creating Commerce SDK Catalog for a given instance\n");

    GS_ASSERT(d2gInstance);
    if (!d2gInstance)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
            "%s :: %s (%d): Null d2gInstance\n",
            __FILE__, __FUNCTION__, __LINE__);        
    }
    else
    {
        GS_ASSERT(region);
        GS_ASSERT(accessToken);
        if (region      == NULL ||
            accessToken == NULL)
        {
            gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
                "%s :: %s (%d): Null pointer to accessToken or region \n",
                __FILE__, __FUNCTION__, __LINE__);
        }
        else
        {
            d2gCatalog = (D2GICatalog*)gsimalloc(sizeof(D2GICatalog));
            if (d2gCatalog == NULL)
            {
                gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
                    "%s :: %s (%d): Null Catalog possibly out of memory\n",
                    __FILE__, __FUNCTION__, __LINE__);
                return d2gCatalog;
            }
            else
            {
                // initialize the catalog
                memset(d2gCatalog, 0, sizeof(D2GICatalog));
                d2gCatalog->mGameId      = gameId;
                d2gCatalog->mVersion     = version;
                d2gCatalog->mAccessToken = goawstrdup(accessToken);
                d2gCatalog->mRegion      = goawstrdup(region);
                d2gCatalog->mItemArray   = NULL;
                d2gCatalog->mAllItemsReceived = gsi_false;
                d2gCatalog->mExtraInfoCache   = NULL;
                
                // Initialize the item array
                d2gCatalog->mItemArray      = ArrayNew(sizeof(D2GCatalogItem), 
                                                        GS_D2G_ITEMARRAY_INITIAL_COUNT, 
                                                        d2giCatalogCacheFreeItem);
                                                   
                d2gCatalog->mExtraInfoCache = ArrayNew(sizeof(D2GExtraInfo), 
                                                        GS_D2G_EXTRAINFOCACHE_INITIAL_COUNT,
                                                        d2giExtraInfoCacheFree);
            }
        }
    }
    return d2gCatalog;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  d2gCleanupCatalog
//  Brief
//      De-allocates the memory associated with the catalog d2gInstance.   
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//  Return Value 
//      None.
//  Remarks
//      This API function is mandatory. This must be called before calling d2gCleanup.
//  See Also 
//      d2gCreateCatalog
//
void d2gCleanupCatalog(D2GInstancePtr theInstance,
                       D2GCatalogPtr  theCatalog)
{
    D2GIInstance *d2gInstance = (D2GIInstance*)theInstance;

    D2GICatalog  *d2gCatalog  = (D2GICatalog*)theCatalog;

    gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
        "d2gCleanupCatalog called, destroying Direct2Game Cache\n");

    GS_ASSERT(d2gInstance);
    GS_ASSERT(d2gCatalog);
    if (!d2gInstance || !d2gCatalog)
    {	
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    if (d2gCatalog->mItemArray != NULL)
    {
        ArrayFree(d2gCatalog->mItemArray);
    }
    if (d2gCatalog->mExtraInfoCache != NULL)
    {   
        ArrayFree(d2gCatalog->mExtraInfoCache);
    }        
    gsifree(d2gCatalog->mRegion);
    gsifree(d2gCatalog->mAccessToken);
    gsifree(d2gCatalog);
    d2gCatalog = NULL;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gGetStoreAvailability
//  Brief
//      This API call retrieves whether a store implementation for 
//      a given catalog is available. 
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gGetStoreAvailability(D2GInstancePtr theInstance, 
                                 D2GCatalogPtr  theCatalog,
                                 D2GGetStoreAvailabilityCallback callback, 
                                 void           *userData)
{
	D2GIInstance    *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog     *d2gCatalog  = (D2GICatalog *)theCatalog;
	GSResult result;

	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d):  called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d):  called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

    // A callback should be provided.
    GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giServiceGetStoreAvail(d2gInstance, d2gCatalog, callback, userData);
	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gLoadExtraCatalogInfo
//  Brief
//      This API function retrieves a list of ExtraInfo items that is particular 
//      to the game from the backend services. 
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function is optional if no ExtraInfo exist for a game. If ExtraInfo 
//      exist, this function should be called during the SDK initialization.
//
GSResult d2gLoadExtraCatalogInfo(D2GInstancePtr    theInstance, 
                          D2GCatalogPtr     theCatalog,
                          D2GLoadExtraCatalogInfoCallback callback, 
                          void              *userData)

{
    D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog = (D2GICatalog*)theCatalog;
    GSResult result;

    GS_ASSERT(d2gInstance);
    if (!d2gInstance)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game,GSIDebugType_State,GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    // A callback must be provided
    GS_ASSERT(callback);
    if (!callback)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game,GSIDebugType_State,GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    result = d2giServiceLoadExtraInfo(d2gInstance, d2gCatalog, callback, userData);
    if (GS_SUCCEEDED(result))
        return GS_SUCCESS; // this strips extended success information
    else
        return result;

    GSI_UNUSED(userData);
    GSI_UNUSED(callback);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gLoadCatalogItems
//  Brief
//      This API call retrieves the items of a catalog from the backend.  
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      When this request is completed successfully the D2G cache is populated 
//      with the items retrieved from the Catalog. 
//
GSResult d2gLoadCatalogItems(D2GInstancePtr theInstance,
                             D2GCatalogPtr  theCatalog, 
						     D2GLoadCatalogItemsCallback callback, 
						     void           *userData)
{
	GSResult     result       = GS_SUCCESS;
	D2GIInstance *d2gInstance = (D2GIInstance*)theInstance;
	D2GICatalog  *d2gCatalog  = (D2GICatalog*) theCatalog;	
	
    GS_ASSERT(d2gInstance);
	GS_ASSERT(d2gCatalog);
	if (!d2gInstance || !d2gCatalog)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d):  called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}	
	
	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d):  called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}
	
	// A callback must be provided
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	if (d2gCatalog->mAllItemsReceived)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
			"%s :: %s (%d): getting data from cache.\n",
            __FILE__, __FUNCTION__, __LINE__);
		result = d2giGetAllItemsFromCache(d2gCatalog, callback, userData);
	}
	else
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
			"%s :: %s (%d): data not in cache, getting data from service.\n",
            __FILE__, __FUNCTION__, __LINE__);
		// do the web service call
		result = d2giServiceLoadCatalogItems(d2gInstance, d2gCatalog, callback, userData);
	}

	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; 
	else
		return result;

	GSI_UNUSED(userData);
	GSI_UNUSED(callback);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gLoadCatalogItemsByCategory
//  Brief
//      This function retrieves a list of the items which are in a specific 
//      category.
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gLoadCatalogItemsByCategory(D2GInstancePtr   theInstance,  
                                       D2GCatalogPtr    theCatalog,
                                       const UCS2String theCategory,
                                       D2GLoadCatalogItemsByCategoryCallback callback, 
                                       void             *userData)
{
	D2GIInstance   *d2gInstance    = (D2GIInstance*)theInstance;
    D2GICatalog    *d2gCatalog  = (D2GICatalog*)theCatalog;

	GSResult result;
	GS_ASSERT(d2gInstance);
	GS_ASSERT(d2gCatalog);
	if (!d2gInstance || !d2gCatalog)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance or catalog\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

	// They should provide a callback from the beginning
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}


	if (d2gCatalog->mAllItemsReceived)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
			"%s :: %s (%d): getting data from cache.\n",
            __FILE__, __FUNCTION__, __LINE__);
		// Catalog is already in the cache
		result = d2giGetCatalogItemsByCategoryFromCache(d2gCatalog, 
                                                 theCategory, 
                                                 callback, 
                                                 userData);
	}
	else
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Debug,
			"%s :: %s (%d): data not in cache, getting data from service.\n",
            __FILE__, __FUNCTION__, __LINE__);
		// do the web service call
		result = d2giServiceGetItemsByCategory(d2gInstance, 
                                               d2gCatalog, 
                                               theCategory, 
                                               callback, 
                                               userData);
	}

	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;

	GSI_UNUSED(userData);
	GSI_UNUSED(callback);
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gGetUserCreditCards
//  Brief
//      This API call retrieves the user’s credit cards from the backend. 
//      D2G needs the user account information prior to user’s purchases. .
//  Parameters
//      theInstance [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  [in] This is pointer created by the d2gCreateCatalog.
//      validOnly   [in] A boolean to indicate if the request is for all or 
//                       only valid credit cards.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function should be called  before calling d2gGetOrderTotal 
//      since it needs the account information.
//
GSResult d2gGetUserCreditCards(D2GInstancePtr theInstance,
							   D2GCatalogPtr  theCatalog,
							   gsi_bool validOnly,
							   D2GGetUserCreditCardsCallback callback, 
							   void * userData) 
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
	D2GICatalog  *d2gCatalog  = (D2GICatalog*)theCatalog;
	GSResult result;

	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

	// They should provide a callback from the beginning
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giServiceGetUserCreditCards(d2gInstance, d2gCatalog, validOnly, callback, userData);
	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;

	GSI_UNUSED(userData);
	GSI_UNUSED(callback);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gGetOrderTotal
//  Brief
//      This API function is the first step in the ordering process.  
//  Parameters
//      theInstance     [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog      [in] This is the pointer created by the d2gCreateCatalog.
//      accountId       [in] accountId is retrieved  with the d2gGetUserCreditCards 
//                       request from the back-end.
//      cultureCode     [in] The culture code in which this order will be processed. 
//      currencyCode    [in] The currency in which this order will be processed. 
//      itemIds         [in] Pointer to the list of itemIds selected from the catalog. 
//      itemCount       [in] The number items in the itemIds list.
//      itemQuantities  [in] The pointer to the corresponding list of item quantities
//                           for each item in the  itemIds  list.
//      callback        [in] This is the pointer to developer’s callback function.
//      userData        [in/out] a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code.
//  Remarks
//      This function call is the first step to make purchase. It simply builds 
//      the shopping cart information for the game. A list of items selected from 
//      the Catalog along with their quantities passed as input. Then, this function 
//      will send an Order Request to back-end to retrieve the OrderTotal. 
//
GSResult d2gGetOrderTotal( D2GInstancePtr theInstance, 
					       D2GCatalogPtr  theCatalog,
					       gsi_u32        accountId, 
					       D2GItemId      *itemIds, 
					       gsi_u32        itemCount, 
					       gsi_u32        *itemQuantities, 
					       D2GGetOrderTotalCallback callback, 
					       void           *userData)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog*)theCatalog;
	GSResult result;

	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

	GS_ASSERT(accountId);
	if (accountId == 0)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): accountId is zero!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	GS_ASSERT(itemIds);
	if (itemIds == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): itemIds is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	GS_ASSERT(itemCount);
	if (itemCount == 0)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): itemIdsCount is 0!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	GS_ASSERT(itemQuantities);
	if (itemQuantities == 0)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): itemQuantities is 0!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giCheckItemsQuantities(d2gInstance, itemQuantities, itemCount);
	if (GS_FAILED(result))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): one of the item quantities is zero!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return result;
	}

	// A callback must be provided
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	
	result = d2giServiceGetOrderTotal( d2gInstance, 
	                                   d2gCatalog, 
	                                   accountId, 
	                                   itemIds, 
	                                   itemCount, 
	                                   itemQuantities, 
	                                   callback, 
	                                   userData);
	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gStartOrder
//  Brief
//      This API call is for starting the purchase process. It needs the OrderTotal 
//      obtained with the d2gGetOrderTotal request.  
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  [in] This is the pointer created by the d2gCreateCatalog.
//      orderTotal  [in] This is received from the backend in the response 
//                       to d2gGetOrderTotal request.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gStartOrder(D2GInstancePtr theInstance, 
					   D2GCatalogPtr  theCatalog,
					   D2GOrderTotal  *orderTotal,
					   D2GStartOrderCallback callback, 
					   void           *userData)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog = (D2GICatalog*)theCatalog;

	GSResult result;
	
	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}
	
	GS_ASSERT(orderTotal->mOrder.mValidation.mIsValid);
	if (!orderTotal->mOrder.mValidation.mIsValid)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): pending order was not valid, need to retry getordertotal!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// A callback must be provided
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giServicePurchaseItems(d2gInstance, 
                                      d2gCatalog, 
                                      orderTotal, 
                                      callback, 
                                      userData);
	
	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gIsOrderComplete 
//  Brief
//      This API call is for retrieving the order status from the backend. 
//      The order status is returned by the callback when the request completes. 
//  Parameters
//      theInstance   [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog    [in] This is the pointer created by the d2gCreateCatalog.
//      orderPurchase [in] This is received from the backend in the response 
//                         to the d2gStartOrder request.
//      callback      [in] This is the pointer to developer’s callback function. 
//      userData      [in/out] a void pointer to user-defined  data.
//  Return Value    GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function should be called by the game periodically to poll after 
//      d2gStartOrder request completes successfully to retrieve the order status.
//
GSResult d2gIsOrderComplete(D2GInstancePtr    theInstance,
							 D2GCatalogPtr    theCatalog,
							 D2GOrderPurchase *orderPurchase, 
							 D2GIsOrderCompleteCallback callback,
							 void             *userData)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog *)theCatalog;
	GSResult result;

	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
			"%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

	GS_ASSERT(orderPurchase->mOrder.mValidation.mIsValid);
	if (!orderPurchase->mOrder.mValidation.mIsValid)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): pending order was not valid, need to retry getordertotal!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// They should provide a callback from the beginning
	GS_ASSERT(callback);
	if (!callback)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giServiceIsOrderComplete(d2gInstance, 
                                        d2gCatalog, 
                                        orderPurchase, 
                                        callback, 
                                        userData);
	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gGetPurchaseHistory
//  Brief   
//      This API call retrieves the user’s purchase history from the backend 
//      service for the purchases made related to the game. 
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  [in] This is the pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The callback returns the list of items purchased for this game.
//
GSResult d2gGetPurchaseHistory(D2GInstancePtr theInstance, 
                               D2GCatalogPtr  theCatalog,
                               D2GGetPurchaseHistoryCallback callback, 
                               void           *userData)

{
    D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog*)theCatalog;
    GSResult result;

    GS_ASSERT(d2gInstance);
    if (!d2gInstance)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    // They should provide a callback from the beginning
    GS_ASSERT(callback);
    if (!callback)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    result = d2giServiceGetPurchaseHistory(d2gInstance, d2gCatalog, callback, userData);
    if (GS_SUCCEEDED(result))
        return GS_SUCCESS; // this strips extended success information
    else
        return result;
        
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gStartDownloadFileById 
//  Brief
//      This function is called to download a purchased file. The download 
//      information is part of the response to d2gGetPurchaseHistory and 
//      d2gIsOrderComplete requests.
//  Parameters
//      theInstance      [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog       [in] This is the pointer created by the d2gCreateCatalog.
//      fileId           [in] The file id to be downloaded.
//      downloadLocation [in] The directory path where the downloaded file 
//                            to be saved. The download directory must exist.
//      progressCallback [in] This is the pointer to developer’s progress callback.
//      completedCallback[in] This is the pointer to developer’s completed callback.
//      userData         [in/out] a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      After the purchase completed, the developer can download the files  from 
//      the download list retrieved either with d2gIsOrderComplete request or 
//      d2gGetPurchaseHistory request.
//
GSResult d2gStartDownloadFileById(  D2GInstancePtr theInstance, 
                                    D2GCatalogPtr  theCatalog,
                                    D2GFileId      fileId, 
                                    const gsi_char *downloadLocation, 
							        D2GDownloadProgressCallback progressCallback, 
							        D2GDownloadCompleteCallback completedCallback, 
							        void           *userData)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog *)theCatalog;
	GSResult     result       = GS_SUCCESS;;

	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance, please create and initialize.\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}

	// The callbacks must be provided
	GS_ASSERT(progressCallback && completedCallback);
	if (!progressCallback || !completedCallback)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callbacks are NULL!\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	result = d2giServiceGetDownloadInfo(d2gInstance,
	                                    d2gCatalog,  
	                                    fileId, 
	                                    downloadLocation, 
	                                    progressCallback, 
	                                    completedCallback,
	                                    userData);

	if (GS_SUCCEEDED(result))
		return GS_SUCCESS; // this strips extended success information
	else
		return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//NOT SUPPORTED YET! DO NOT USE!
//d2gGetItemActivationData
//  Brief   
//      This function retrieves the licensing information from the backend.
//      if any of the licenses owned by the game expires.
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  [in] This is the pointer created by the d2gCreateCatalog.
//      callback    [in] This is the pointer to developer’s callback function.
//      thePackageId[in] This is parameter is an input from the DRM functionality.
//      userData    [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is part of an upcoming DRM functionlity. Do not use yet!
//
GSResult d2gGetItemActivationData(D2GInstancePtr theInstance, 
                                  D2GCatalogPtr  theCatalog,
                                  D2GGetItemActivationDataCallback callback, 
                                  D2GPackageId   thePackageId,
                                  void           *userData)
{
    D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog *)theCatalog;
    GSResult     result = GS_SUCCESS;

    GS_ASSERT(d2gInstance);
    GS_ASSERT(d2gCatalog);
    if (!d2gInstance || !d2gCatalog)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d):  called without doing availability check\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    // They should provide a callback from the beginning
    GS_ASSERT(callback);
    if (!callback)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callback is NULL!\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    result = d2giServiceGetItemActivationData(d2gInstance, d2gCatalog, callback, thePackageId, userData);
    if (GS_SUCCEEDED(result))
        return GS_SUCCESS; // this strips extended success information
    else
        return result;
        
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gCloneCatalogItems
//  Brief
//      This function makes a shallow copy of the items from theSrcItemList 
//      to theDstItemList. 
//  Parameters
//      theDstItemList  [out] Pointer to the destination list.
//                            It is initialized by d2gCloneCatalogItems. 
//      theSrcItemList  [in] Pointer to the source list.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The reason why the shallow copy is used is that the actual catalog items 
//      are located in the D2G cache. If this function is called, its corresponding 
//      free function d2gFreeCatalogItems must be called to de-allocate the memory 
//      containing theDstItemList. See D2GLoadCatalogItemsCallback for when this is used.
//  See Also
//      d2gFreeCatalogItems
//
GSResult d2gCloneCatalogItemList(D2GCatalogItemList       *dstList, 
                              const D2GCatalogItemList *srcList)
{
	gsi_u32 i;
    
	GS_ASSERT(srcList);
	GS_ASSERT(dstList);
	
	if (dstList == NULL || srcList == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_Warning,
            "%s :: %s (%d): called with null itemList", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters); 
	}

    // initialize the destination catalog items list
	dstList->mCount = srcList->mCount;
	if (dstList->mCount == 0)
		dstList->mCatalogItems = NULL;
	else
	{
		dstList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem) * srcList->mCount);
		if (dstList->mCatalogItems == NULL)
		{
			gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
				"%s :: %s (%d): failed to allocate clone structure", 
				__FILE__, __FUNCTION__, __LINE__);
			return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory); 
		}

		memset(dstList->mCatalogItems, 0, sizeof(D2GCatalogItem) * srcList->mCount);
		for (i = 0; i < srcList->mCount; i++)
		{
			//shallow copy
			dstList->mCatalogItems[i] = srcList->mCatalogItems[i];	
		}
	}
		
	return GS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gFreeCatalogItems
//  Brief
//      This function de-allocates the memory for theItemList and assigns NULL 
//      to theItemList. 
//  Parameters
//      theItemList [in] Pointer to the list.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is the corresponding API function to d2gCloneCatalogItems 
//      to de-allocate and reset memory.
GSResult d2gFreeCatalogItemList(D2GCatalogItemList *theItemList)
{
	if (theItemList == NULL)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
            "%s :: %s (%d): called with null ItemList", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_SUCCESS; 
	}
	
	// gsi_false - Only free the container for the list items
	return d2giFreeCatalogItems(NULL, theItemList, GS_SUCCESS, gsi_false);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gAppendCatalogItems
// Brief
//      This function allows the developer append the one list to another. 
//      The srcList is not modified. The dstList contains its original items 
//      as well as the all the items from the srcList.
//  Parameters
//      dstItemList [out] Pointer to the destination list the source list.  
//      srcItemList [in]  Pointer to source list given by the caller. 
//                        The contents of this list are not modified.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 	
//  Remarks
//      See D2GLoadCatalogItemsCallback for when this may be used.
//
GSResult d2gAppendCatalogItemList(D2GCatalogItemList *dstList, 
                                  D2GCatalogItemList *srcList)
{
    GSResult    result = GS_SUCCESS;
    GS_ASSERT(srcList);
    GS_ASSERT(dstList);

    if (dstList == NULL || srcList == NULL)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_Warning,
            "%s :: %s (%d): called with null itemList", 
            __FILE__, __FUNCTION__, __LINE__);
        result = GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters); 
    }
    else
    {
        // sort the destination list according to item id first.
        result =  d2giSortCatalogItemsbyID(dstList, D2GSort_Ascending); 
        if (GS_SUCCEEDED(result))
        {
            // initialize the destination catalog items list
            if (srcList->mCount != 0)
            {
                // sort the source list now.
                result =  d2giSortCatalogItemsbyID(srcList, D2GSort_Ascending); 
                if (GS_SUCCEEDED(result))
                {
                    // allocate enough space for the merged lists.
                    gsi_u32        maxCount = dstList->mCount+srcList->mCount;
                    D2GCatalogItem *pDstList = NULL;

                    pDstList = (D2GCatalogItem *)realloc( dstList->mCatalogItems, 
                                                (sizeof(D2GCatalogItem)*maxCount));
                    if (pDstList != NULL)
                    {   
                        gsi_u32 i,j = 0;
                        gsi_u32 dstListCountOriginal = dstList->mCount;

                        dstList->mCatalogItems = pDstList;
                        memset((pDstList+dstList->mCount), 0 , sizeof(D2GCatalogItem)*srcList->mCount);
                        for ( i = 0 ; i<srcList->mCount ; i++)
                        {
                            while ((j<dstListCountOriginal) && 
                                   (srcList->mCatalogItems[i].mItem.mItemId < dstList->mCatalogItems[j].mItem.mItemId))
                            {
                                j++;
                            }

                            if (j <dstListCountOriginal)
                            {
                                if (srcList->mCatalogItems[i].mItem.mItemId != dstList->mCatalogItems[j].mItem.mItemId)
                                {
                                    // We do not have a duplicate of this item.
                                    // Add this item at the end.
                                    memcpy(&dstList->mCatalogItems[dstList->mCount], &srcList->mCatalogItems[i], sizeof(D2GCatalogItem));
                                    dstList->mCount++;

                                }
                                j++; // now move to the next item in the destination list
                            }  

                        }
                        // So now we have a merged list
                        if (maxCount > dstList->mCount )
                        {
                            pDstList = (D2GCatalogItem *)realloc( dstList->mCatalogItems, 
                                                                  (sizeof(D2GCatalogItem)*dstList->mCount));
                            dstList->mCatalogItems = pDstList;

                        }

                    }
                    else
                    {
                        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
                            "%s :: %s (%d): Not enough space to append lists", 
                            __FILE__, __FUNCTION__, __LINE__);
                        result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory); 
                    }
                }
            }
			else
			{
				gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
					"%s :: %s (%d): Not enough space to append lists", 
					__FILE__, __FUNCTION__, __LINE__);
				result = GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory); 
			}
		}		
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gCloneOrderTotal
//  Brief   
//      This function makes a deep copy of the items from sourceOrderTotal to 
//      destinationOrderTotal. 
//  Parameters
//      destinationOrderTotal [out] Pointer to the destination order total. 
//      sourceOrderTotal      [in]  Pointer to source order total. The contents 
//      of this list are not modified.
//  Return Value
//      GSResult – Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function is important, since in the D2GGetOrderTotalCallback function, 
//      the developer needs to retain the order total. The memory for the destination 
//      list is allocated by D2G. If this function is called, its corresponding free 
//      function d2gFreeOrderTotal must be called to de-allocate the memory containing 
//      destinationOrderTotal.
//
GSResult d2gCloneOrderTotal(D2GOrderTotal **destinationOrderTotal, 
                            D2GOrderTotal *sourceOrderTotal)
{
	D2GOrderTotal *tempOrderTotal;
	gsi_u32 i;
	GS_ASSERT(sourceOrderTotal);
	GS_ASSERT(destinationOrderTotal);
	if (sourceOrderTotal == NULL || destinationOrderTotal == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): called with null sourceOrderTotal", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	tempOrderTotal = *destinationOrderTotal = (D2GOrderTotal *)gsimalloc(sizeof(D2GOrderTotal));
	
	tempOrderTotal->mQuoteDate = sourceOrderTotal->mQuoteDate;

	tempOrderTotal->mOrder.mAccountId = sourceOrderTotal->mOrder.mAccountId;
	GS_ASSERT(sourceOrderTotal->mOrder.mRootOrderGuid);
	GS_ASSERT(sourceOrderTotal->mOrder.mSubTotal);
	GS_ASSERT(sourceOrderTotal->mOrder.mTax);
	GS_ASSERT(sourceOrderTotal->mOrder.mTotal);
	GS_ASSERT(sourceOrderTotal->mOrder.mValidation.mIsValid || 
	         (sourceOrderTotal->mOrder.mValidation.mIsValid == gsi_false && 
              sourceOrderTotal->mOrder.mValidation.mMessage));
	
	if (sourceOrderTotal->mOrder.mRootOrderGuid == NULL || 
	    sourceOrderTotal->mOrder.mSubTotal == NULL ||
		sourceOrderTotal->mOrder.mTax      == NULL || 
		sourceOrderTotal->mOrder.mTotal    == NULL ||
		sourceOrderTotal->mOrder.mGeoInfo.mCultureCode == NULL ||
		sourceOrderTotal->mOrder.mGeoInfo.mCurrencyCode == NULL || 
		(sourceOrderTotal->mOrder.mValidation.mIsValid == gsi_false && 
         sourceOrderTotal->mOrder.mValidation.mMessage == NULL))
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): called with null items in sourceOrderTotal.", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	tempOrderTotal->mOrder.mRootOrderGuid = goawstrdup(sourceOrderTotal->mOrder.mRootOrderGuid);
	tempOrderTotal->mOrder.mSubTotal = goawstrdup(sourceOrderTotal->mOrder.mSubTotal);
	tempOrderTotal->mOrder.mTax   = goawstrdup(sourceOrderTotal->mOrder.mTax);
	tempOrderTotal->mOrder.mTotal = goawstrdup(sourceOrderTotal->mOrder.mTotal);
	tempOrderTotal->mOrder.mValidation.mIsValid = sourceOrderTotal->mOrder.mValidation.mIsValid;
	tempOrderTotal->mOrder.mValidation.mMessage = goawstrdup(sourceOrderTotal->mOrder.mValidation.mMessage);
	tempOrderTotal->mOrder.mValidation.mResult  = sourceOrderTotal->mOrder.mValidation.mResult;
	tempOrderTotal->mOrder.mGeoInfo.mCultureCode  = goawstrdup(sourceOrderTotal->mOrder.mGeoInfo.mCultureCode);
	tempOrderTotal->mOrder.mGeoInfo.mCurrencyCode = goawstrdup(sourceOrderTotal->mOrder.mGeoInfo.mCurrencyCode);
	
	// Initialize the Order Item List
	GS_ASSERT(sourceOrderTotal->mOrderItemList.mCount > 0);
	tempOrderTotal->mOrderItemList.mCount = sourceOrderTotal->mOrderItemList.mCount;
	tempOrderTotal->mOrderItemList.mOrderItems  = (D2GOrderItem *)gsimalloc(sizeof(D2GOrderItem) * tempOrderTotal->mOrderItemList.mCount);
	memset(tempOrderTotal->mOrderItemList.mOrderItems, 0, sizeof(D2GOrderItem) * tempOrderTotal->mOrderItemList.mCount);

    // Now copy the Order Item List
	for (i = 0; i < tempOrderTotal->mOrderItemList.mCount; i++)
	{
		d2giCloneOrderItem(&tempOrderTotal->mOrderItemList.mOrderItems[i], &sourceOrderTotal->mOrderItemList.mOrderItems[i]);	
	}

	return GS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gFreeOrderTotal
//  Brief
//      This API function de-allocates and resets the memory pointed to by 
//      orderTotal.
//  Parameters
//      orderTotal [in] The pointer to the order total.
//  Return Value
//      None. 
//  Remarks
//      This is the corresponding API function to d2gCloneOrderTotal.
//
void d2gFreeOrderTotal(D2GOrderTotal *orderTotal)
{
	d2giFreeOrderTotal(orderTotal);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gCloneOrderPurchase
//  Brief 
//      This function makes a deep copy of the items from sourceOrderPurchase to 
//      destOrderPurchase. 
//  Parameters
//      destOrderPurchase [out] a list of pointers to the destination order purchases. 
//      sourceOrderTotal  [in]  Pointer to source order total. 
//                              The contents of this list are not modified.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function is important, since in the D2GStartOrderCallback function, the 
//      developer needs retain the order purchase. The D2G SDK allocates the memory 
//      space for the destination list. 
//      If this function is called, d2gFreeOrderPurchase must be called to de-allocate 
//      the memory space.
//  See Also 
//      D2GStartOrderCallback for when this may be used.
//
GSResult d2gCloneOrderPurchase(D2GOrderPurchase **destOrderPurchase, 
                               D2GOrderPurchase *sourceOrderPurchase)
{
	D2GOrderPurchase *anOrderPurchase;
	GSResult result;

	GS_ASSERT(sourceOrderPurchase);
	GS_ASSERT(destOrderPurchase);
	if (sourceOrderPurchase == NULL || destOrderPurchase == NULL)
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
			"%s :: %s (%d): called with null sourceOrderTotal", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	anOrderPurchase = *destOrderPurchase = (D2GOrderPurchase *)gsimalloc(sizeof(D2GOrderPurchase));

	anOrderPurchase->mPurchaseDate = sourceOrderPurchase->mPurchaseDate;
	
	anOrderPurchase->mOrder.mAccountId = sourceOrderPurchase->mOrder.mAccountId;
	GS_ASSERT(sourceOrderPurchase->mOrder.mRootOrderGuid);
	GS_ASSERT(sourceOrderPurchase->mOrder.mSubTotal);
	GS_ASSERT(sourceOrderPurchase->mOrder.mTax);
	GS_ASSERT(sourceOrderPurchase->mOrder.mTotal);
	GS_ASSERT(sourceOrderPurchase->mOrder.mValidation.mMessage);
	GS_ASSERT(sourceOrderPurchase->mOrder.mGeoInfo.mCultureCode);
	GS_ASSERT(sourceOrderPurchase->mOrder.mGeoInfo.mCurrencyCode);
	
	if (sourceOrderPurchase->mOrder.mRootOrderGuid == NULL || 
	    sourceOrderPurchase->mOrder.mSubTotal == NULL ||
		sourceOrderPurchase->mOrder.mTax == NULL || 
		sourceOrderPurchase->mOrder.mTotal == NULL || 
		sourceOrderPurchase->mOrder.mValidation.mMessage == NULL ||
        sourceOrderPurchase->mOrder.mGeoInfo.mCultureCode == NULL ||
        sourceOrderPurchase->mOrder.mGeoInfo.mCurrencyCode == NULL )
	{
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): called with null items in sourceOrderPurchase", 
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	anOrderPurchase->mOrder.mRootOrderGuid = goawstrdup(sourceOrderPurchase->mOrder.mRootOrderGuid);
	anOrderPurchase->mOrder.mSubTotal = goawstrdup(sourceOrderPurchase->mOrder.mSubTotal);
	anOrderPurchase->mOrder.mTax    = goawstrdup(sourceOrderPurchase->mOrder.mTax);
	anOrderPurchase->mOrder.mTotal  = goawstrdup(sourceOrderPurchase->mOrder.mTotal);
	
    anOrderPurchase->mOrder.mGeoInfo.mCultureCode = goawstrdup(sourceOrderPurchase->mOrder.mGeoInfo.mCultureCode);
    anOrderPurchase->mOrder.mGeoInfo.mCurrencyCode = goawstrdup(sourceOrderPurchase->mOrder.mGeoInfo.mCurrencyCode);

	anOrderPurchase->mOrder.mValidation.mIsValid = sourceOrderPurchase->mOrder.mValidation.mIsValid;
	anOrderPurchase->mOrder.mValidation.mMessage = goawstrdup(sourceOrderPurchase->mOrder.mValidation.mMessage);
	anOrderPurchase->mOrder.mValidation.mResult  = sourceOrderPurchase->mOrder.mValidation.mResult;

	GS_ASSERT(sourceOrderPurchase->mItemPurchases.mCount > 0);
	anOrderPurchase->mItemPurchases.mCount = sourceOrderPurchase->mItemPurchases.mCount;

    result = d2giCloneOrderItemPurchases(&anOrderPurchase->mItemPurchases.mOrderItemPurchases, 
                                         &anOrderPurchase->mItemPurchases.mCount, 
		                                 sourceOrderPurchase->mItemPurchases.mOrderItemPurchases, 
                                         sourceOrderPurchase->mItemPurchases.mCount);
    if (GS_FAILED(result))
    {
	    gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
		    "%s :: %s (%d): Failed to copy subitem list for item using d2giCloneOrderItemPurchases.\n", 
            __FILE__, __FUNCTION__, __LINE__);
	    return result;
    }
    
	return GS_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gFreeOrderPurchase
//  Brief
//      This API function de-allocates and resets the memory pointed to 
//      by orderPurchase.
//  Parameters
//      orderPurchase : [in] Pointer to the memory to be released.
//  Return Value
//     None. 
//  Remarks
//      This is the corresponding  API call  to d2gCloneOrderPurchase .
//
void d2gFreeOrderPurchase(D2GOrderPurchase *orderPurchase)
{
	d2giFreeOrderPurchase(orderPurchase);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2gFreePuchaseHistory
//  Brief
//      This API function de-allocates and resets the memory pointed to 
//      by purchaseHistory.
//  Parameters
//      purchaseHistory : [in] Pointer to the memory to be released.
//  Return Value
//     None. 
//  Remarks
//      Purchase histories should be freed explicitly because callbacks give
//      pointers to the objects but does not free them.
//
void d2gFreePurchaseHistory(D2GPurchaseHistory *purchaseHistory)
{
	d2giFreePurchaseHistory(purchaseHistory);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2gFreeDownloadItemList
//  Brief
//      This API function de-allocates and resets the memory pointed to 
//      by downloadList.
//  Parameters
//      contentUpdateList : [in] Pointer to the memory to be released.
//  Return Value
//     None. 
//  Remarks
//      content update lists should be freed explicitly because callbacks give
//      pointers to the objects but does not free them.  The function will 
//		free internal data and the list.  Developers can set the pointer to 
//      NULL afterwards.
void d2gFreeContentUpdateList(D2GContentUpdateList *contentUpdateList)
{
	d2giFreeContentUpdateList(contentUpdateList);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2gFreeExtraInfoList
//  Brief
//      This API function de-allocates and resets the memory pointed to 
//      by downloadList.
//  Parameters
//      extraInfoList : [in] Pointer to the memory to be released.
//  Return Value
//     None. 
//  Remarks
//      Download lists should be freed explicitly because callbacks give
//      pointers to the objects but does not free them.  The function will 
//		free internal data and the list.  Developers can set the pointer to 
//      NULL afterwards.
void d2gFreeExtraInfoList(D2GExtraInfoList *extraInfoList)
{
	d2giFreeExtraInfoList(NULL, extraInfoList, GS_SUCCESS, gsi_false);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2gFreeCreditCardInfoList
//  Brief
//      This API function de-allocates and resets the memory pointed to 
//      by creditCardList.
//  Parameters
//      creditCardList : [in] Pointer to the memory to be released.
//  Return Value
//     None. 
//  Remarks
//      Credit card lists should be freed explicitly because callbacks give
//      pointers to the objects but does not free them.  The function will 
//		free internal data and the list.  Developers can set the pointer to 
//      NULL afterwards.
void d2gFreeCreditCardInfoList(D2GCreditCardInfoList *creditCardList)
{
	d2giFreeCreditCardInfoList(creditCardList);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbyPrice 
//  Brief   
//      This function sorts a list of catalog items onto itself, according to 
//      the price of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbyPrice(D2GCatalogItemList *list, D2GSortDirection direction )
{
    if ((list == NULL) || (list->mCount <=0))
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
            "%s :: %s (%d): null or empty Catalog Item List", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_SUCCESS;  // Nothing to sort return success here 
    }
    return d2giSortCatalogItemsbyPrice(list, direction) ; 

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbyName
//  Brief
//      This function sorts a list of catalog items onto itself, according to 
//      the item names alphabetically either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter. 
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; 
//                     ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbyName(D2GCatalogItemList *list, D2GSortDirection direction)
{
    if ((list == NULL) || (list->mCount <=0))
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
            "%s :: %s (%d): null or empty Catalog Item List", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_SUCCESS;  // Nothing to sort return success here 
    }
    return d2giSortCatalogItemsbyName(list, direction); 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbyReleaseDate 
//  Brief   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbyReleaseDate(D2GCatalogItemList *list, D2GSortDirection direction )
{
    if ((list == NULL) || (list->mCount <=0))
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
            "%s :: %s (%d): null or empty Catalog Item List", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_SUCCESS;  // Nothing to sort return success here 
    }
    return d2giSortCatalogItemsbyDate(list, direction); 

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbyItemId
//  Brief   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbyItemId(D2GCatalogItemList *list, D2GSortDirection direction )
{
	if ((list == NULL) || (list->mCount <=0))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): null or empty Catalog Item List", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_SUCCESS;  // Nothing to sort return success here 
	}
	return d2giSortCatalogItemsbyID(list, direction); 

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbyExternalId
//  Brief   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbyExternalId(D2GCatalogItemList *list, D2GSortDirection direction )
{
	if ((list == NULL) || (list->mCount <=0))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): null or empty Catalog Item List", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_SUCCESS;  // Nothing to sort return success here 
	}
	return d2giSortCatalogItemsbyExtId(list, direction); 

}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//d2gSortCatalogItemsbySize 
//  Brief   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      [in] pointer to the list of catalog items to be sorted.
//      direction [in] an enumerated type for the sort direction; ascending or descending.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gSortCatalogItemsbySize(D2GCatalogItemList *list, D2GSortDirection direction )
{
	if ((list == NULL) || (list->mCount <=0))
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_Warning,
			"%s :: %s (%d): null or empty Catalog Item List", 
			__FILE__, __FUNCTION__, __LINE__);
		return GS_SUCCESS;  // Nothing to sort return success here 
	}
	return d2giSortCatalogItemsbySize(list, direction); 

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gGetCategories
//  Brief
//      This is a helper function designed to retrieve the list of categories 
//      from the catalog. This function is under development.                          
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  [in] This is the pointer created by the d2gCreateCatalog.
//      categoryList[out]a pointer to the category list for the categories found. 
//                       D2G SDK allocates the memory for the list. 
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This helper function helps you render your category based UI if for example 
//      each category were to be assigned to a button in your store navigation.
//  
//  IMPORTANT: 
//  This function does a shallow copy of the categoryName from the cache. 
//  When releasing the memory for  the categoryList->categoryName, 
//  use  gsifree(categoryList->categoryName).
//  
//
GSResult d2gGetCategories(D2GInstancePtr  theInstance, 
                          D2GCatalogPtr   theCatalog,
						  D2GCategoryList *categoryList)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
	D2GICatalog  *d2gCatalog  = (D2GICatalog *) theCatalog;
	GSResult result = GSResultCode_Success;
	
	// initialize the categoryList
    categoryList->mCount = 0;
    categoryList->mCategoryNames = NULL;
    
	GS_ASSERT(d2gInstance);
	GS_ASSERT(d2gCatalog);
	if (!d2gInstance || !d2gCatalog)
	{
        
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance or catalog\n",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	// verify availability check
	if (GSIGetAvailable()!=GSIACAvailable)
	{
        
		gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check",
            __FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
	}
            
	// Catalog is already in the cache
	result = d2giGetCategoriesFromCache(d2gCatalog, 
                                        categoryList);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gGetExtraCatalogInfo
//  Brief
//      This is a lookup function for a <key, value> pair which is locally 
//      available in the D2G cache.  
//  Parameters
//      theInstance  [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog   [in] This is the pointer created by the d2gCreateCatalog.
//      extraInfoKey [in] The key for the value to be retrieved.
//      aValue       [in] Pointer to the value in the D2G cache.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      See d2gLoadExtraCatalogInfo, for more information to initialize the D2G cache 
//      with the list of ExtraInfo items. Since a pointer to the cached value 
//      is returned, the developer should never free aValue pointer.
//
GSResult d2gGetExtraCatalogInfo (D2GInstancePtr theInstance, 
                          D2GCatalogPtr         theCatalog,
                          const UCS2String      extraInfoKey,
                          UCS2String           *extraInfoValue)
{
    GSResult result = GS_SUCCESS;
    D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  *d2gCatalog  = (D2GICatalog*)theCatalog;
    
    GS_ASSERT(d2gInstance);
    GS_ASSERT(d2gCatalog);
    GS_ASSERT(extraInfoKey);
    
    if (!d2gInstance ||
        !d2gCatalog ||
        !extraInfoKey)
    {
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, 
			"Called with NULL instance or NULL extraInfoKey.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "Called without doing availability check.");
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    *extraInfoValue = d2giLookUpExtraInfo(theInstance, theCatalog, extraInfoKey);
    
    if (!*extraInfoValue)
    {
        GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "Key does not exist in the cache.");
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }
    return result;
};

// d2gGetExtraItemInfoByKey
//  Summary
//      Looks up the extra item info value for a given D2GCatalogItem and its 
//		extra info key
//  Parameters
//      theInstance     : [in] Pointer created by the d2gCreateInstance.
//      theCatalogItem  : [in] pointer to item being checked.
//      extraInfoKey	: [in] Extra Info key for the value to be retrieved.
//      aValue          : [out] Pointer to value that is being looked up.
//  Return Value
//      GSResult – Integer built with enums indicating success (GS_SUCCESS) 
//		or a failure code. 
//  Remarks
//      The catalog item being passed in must be valid otherwise the lookup will 
//		not occur.  If the key doesn't exist in the extra info list for the item
//		passed in, extraInfoValue will be NULL.  Valid extraInfoValue will be a copy
//      of the data value given the key.  It is the developer's responsibility to 
//		free the memory pointed to by extraInfoValue.
GSResult d2gGetExtraItemInfoKeyValueByKeyName(const D2GCatalogItem *theCatalogItem,
											  const UCS2String extraInfoKey,
											  UCS2String *extraInfoValue)
{
	gsi_u32 i;

	GS_ASSERT(theCatalogItem);
	GS_ASSERT(extraInfoKey);

	if (theCatalogItem == NULL || extraInfoKey == NULL || extraInfoValue == NULL)
	{
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, 
                   "Called with NULL parameter(s). Please check parameters for any NULL values.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	
	for (i = 0; i < theCatalogItem->mExtraItemInfoList.mCount; i++)
	{
		// grab the pointer to the key for the ith element
		const UCS2String keyI = theCatalogItem->mExtraItemInfoList.mExtraInfoElements[i].mKey;
		const UCS2String valueI = theCatalogItem->mExtraItemInfoList.mExtraInfoElements[i].mValue;
		if (wcscmp(keyI, extraInfoKey) == 0 && valueI != NULL)
		{
			*extraInfoValue = goawstrdup(valueI);
			return GS_SUCCESS;
		}
	}

	*extraInfoValue = NULL;
	GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "Key %s not found", extraInfoKey);
	return GS_D2G_ERROR(GSResultSection_SdkSpecifc, D2GResultCode_ExtraInfo_KeyNotFound);
}

// d2gFilterCatalogItemListByKeyName
//  Summary
//      Creates a subset of a given D2GCatalogItemList based on the 
//		key name passed in.
//  Parameters
//      incomingCatalogItems  : [in] Pointer to D2GCatalogItemList being scanned.
//		outgoingCatalogItems  : [out] Pointer to Pointer of the D2GCatalogItemList being returned.
//      extraInfoKey          : [in] Extra Info Key name used for scanning items.
//  Return Value
//      GSResult – Integer built with enums indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The incomingCatalogItems and extraInfoKey being passed in must be valid otherwise the scan will 
//		not occur.  outgoingCatalogItems will be allocated to store the filtered item list.  If the key 
//		doesn't exist in the extra info list for the list of items being passed in, then 
//		outgoingCatalogItems is set to NULL to represent an empty list.  
GSResult d2gFilterCatalogItemListByKeyName(const D2GCatalogItemList *incomingCatalogItems,
										   D2GCatalogItemList **outgoingCatalogItems,
										   const UCS2String extraInfoKey)
{
	// create a temporary list of the items to include in the output
	// so that we can allocate space appropriately on the outgoing list.
	D2GCatalogItem **itemsChecked;
	gsi_u32 foundItemsCount;
	gsi_u32 i;
	gsi_u32 j;
	D2GCatalogItemList *outList = NULL;

	if (incomingCatalogItems == NULL || outgoingCatalogItems == NULL || extraInfoKey == NULL)
	{
		// we bail because we don't have anything to work with.
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, 
			"Called with NULL parameter(s). Please check parameters for any NULL values.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	
	foundItemsCount = 0; 

	itemsChecked = (D2GCatalogItem **)gsimalloc(sizeof(D2GCatalogItem *) * incomingCatalogItems->mCount);
	if (itemsChecked == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: foundItems.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	for (i = 0; i < incomingCatalogItems->mCount; i++)
	{
		D2GCatalogItem *anItem = &incomingCatalogItems->mCatalogItems[i];
		itemsChecked[i] = NULL;
		if (anItem->mExtraItemInfoList.mCount > 0)
		{			
			for (j = 0; j < anItem->mExtraItemInfoList.mCount; j++)
			{
				if (wcscmp(anItem->mExtraItemInfoList.mExtraInfoElements[j].mKey, extraInfoKey) == 0)
				{
					itemsChecked[i] = anItem;
					foundItemsCount++;
					break;
				}
			}
		}
	}
	
	if (foundItemsCount == 0)
	{
		*outgoingCatalogItems = NULL;
		gsifree(itemsChecked);
		return GS_SUCCESS;
	}

	outList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));
	if (outList == NULL)
	{
		gsifree(itemsChecked);
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: outList.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	outList->mCount = foundItemsCount;
	outList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem) * outList->mCount);
	if (outList->mCatalogItems == NULL)
	{
		gsifree(outList);
		gsifree(itemsChecked);
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: outList->mCatalogItems.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	for(i = 0, j = 0; i < incomingCatalogItems->mCount && j < outList->mCount; i++)
	{
		if (itemsChecked[i])
		{
			outList->mCatalogItems[j++] = *itemsChecked[i];
		}			
	}

	*outgoingCatalogItems = outList;
	gsifree(itemsChecked);
	
	return GS_SUCCESS;	
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// d2gFilterCatalogItemListByKeyValue
//  Summary
//      Creates a subset of a given D2GCatalogItemList based on the 
//		key name and key value passed in.
//  Parameters
//      incomingCatalogItems  : [in] Pointer to D2GCatalogItemList being scanned.
//		outgoingCatalogItems  : [out] Pointer to Pointer of the D2GCatalogItemList being returned.
//      extraInfoKey          : [in] Extra Info Key name used for scanning items.
//      extraInfoValue        : [in] Extra Info Value name used for scanning items.
//  Return Value
//      GSResult – Integer built with enums indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The incomingCatalogItems, extraInfoKey, extraInfoValue being passed in must be valid 
//      otherwise the scan will not occur.  If the combination of the input key and value doesn't 
//		exist in the extra info list for the list of items being passed in, then 
//		outgoingCatalogItems is set to NULL to represent an empty list.
GSResult d2gFilterCatalogItemListByKeyNameValue(const D2GCatalogItemList *incomingCatalogItems,
												D2GCatalogItemList **outgoingCatalogItems,
												const UCS2String extraInfoKey,
												const UCS2String extraInfoValue)
{
	// create a temporary list of the items to include in the output
	// so that we can allocate space appropriately on the outgoing list.
	D2GCatalogItem **itemsChecked;
	gsi_u32 foundItemsCount;
	gsi_u32 i;
	gsi_u32 j;
	D2GCatalogItemList *outList = NULL;

	if (incomingCatalogItems == NULL || outgoingCatalogItems == NULL ||	extraInfoKey == NULL || 
		extraInfoValue == NULL)
	{
		// we bail because we don't have anything to work with.
		GS_D2G_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, 
			"Called with NULL parameter(s). Please check parameters for any NULL values.");
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}

	foundItemsCount = 0; 

	itemsChecked = (D2GCatalogItem **)gsimalloc(sizeof(D2GCatalogItem *) * incomingCatalogItems->mCount);
	if (itemsChecked == NULL)
	{
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: foundItems.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	for (i = 0; i < incomingCatalogItems->mCount; i++)
	{
		D2GCatalogItem *anItem = &incomingCatalogItems->mCatalogItems[i];
		itemsChecked[i] = NULL;
		if (anItem->mExtraItemInfoList.mCount > 0)
		{
							
			for (j = 0; j < anItem->mExtraItemInfoList.mCount; j++)
			{
				if (wcscmp(anItem->mExtraItemInfoList.mExtraInfoElements[j].mKey, extraInfoKey) == 0 &&
					wcscmp(anItem->mExtraItemInfoList.mExtraInfoElements[j].mValue, extraInfoValue) == 0)
				{
					itemsChecked[i] = anItem;
					foundItemsCount++;
					break;
				}
			}
		}
	}

	if (foundItemsCount == 0)
	{
		*outgoingCatalogItems = NULL;
		gsifree(itemsChecked);
		return GS_SUCCESS;
	}

	outList = (D2GCatalogItemList *)gsimalloc(sizeof(D2GCatalogItemList));
	if (outList == NULL)
	{
		gsifree(itemsChecked);
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: outList.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	outList->mCount = foundItemsCount;
	outList->mCatalogItems = (D2GCatalogItem *)gsimalloc(sizeof(D2GCatalogItem) * outList->mCount);
	if (outList->mCatalogItems == NULL)
	{
		gsifree(outList);
		gsifree(itemsChecked);
		GS_D2G_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for: outList->mCatalogItems.");
		return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	for(i = 0, j = 0; i < incomingCatalogItems->mCount && j < outList->mCount; i++)
	{
		if (itemsChecked[i])
		{
			outList->mCatalogItems[j++] = *itemsChecked[i];
		}			
	}

	*outgoingCatalogItems = outList;
	gsifree(itemsChecked);

	return GS_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gSetManifestFilePath
//  Brief
//      This function is set the path for the manifest file location. 
//  Parameters
//      theInstance        [in] This is the pointer created by the d2gCreateInstance.
//      manifestFilePath   [in] String which includes location for the manifest file.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is an optional function. If Patching Features used, the manifest file path 
//      can be set here. If used, it should be called only once right after 
//      the d2gInitialize. The path should exist. 
//      If function is not called the manifest file is located in 
//      the current working directory by default. Since the manifest file 
//      keeps the installed contents this path should not change once it is set.
//
GSResult d2gSetManifestFilePath( D2GInstancePtr     theInstance,
                                 const char         *manifestFilePath) 
{
    GSResult result = GS_SUCCESS;

    GS_ASSERT(theInstance);
    GS_ASSERT(manifestFilePath);
    if ( !theInstance || !manifestFilePath)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance, please create and initialize.\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }
    else
    {
        int failCode;
        D2GIInstance    *d2giInstance = (D2GIInstance *) theInstance;
        d2giInstance->mManifestFilePath = goastrdup(manifestFilePath);
#ifdef _WIN32
        failCode = _mkdir(manifestFilePath);
#else
		failCode = mkdir(manifestFilePath, S_IRWXU | S_IRWXG);
#endif

        if (failCode != EEXIST || failCode != 0)
        {
            gsifree(d2giInstance->mManifestFilePath);
            d2giInstance->mManifestFilePath = NULL; 
            gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_File,GSIDebugLevel_WarmError,
                "%s :: %s (%d): Could not create the specified path for manifest file, path given: %s.\n", 
                __FILE__, __FUNCTION__, __LINE__, manifestFilePath);
            return GS_D2G_ERROR(GSResultSection_File, GSResultCode_FileCannotCreateDirectory);
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gCheckContentUpdates
//  Brief
//      This function invokes a service call to the backend to check any in-game 
//      content update  
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  [in] This is the pointer created by the d2gCreateCatalog.
//      requiredOnly[in] Is the update for required content only.
//      callback    [in] This is the pointer to developer’s callback function.
//      userData    [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      
//
GSResult d2gCheckContentUpdates(D2GInstancePtr theInstance, 
                                D2GCatalogPtr  theCatalog,
                                gsi_bool       requiredOnly,
                                D2GCheckContentUpdatesCallback callback, 
                                void           *userData)
{
    D2GIInstance * d2gInstance = (D2GIInstance *)theInstance;
    D2GICatalog  * d2gCatalog = (D2GICatalog*)theCatalog;
    GSResult result;

    GS_ASSERT(d2gInstance);
    GS_ASSERT(d2gCatalog);
    if (!d2gInstance||!d2gCatalog)

    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance, please create and initialize.\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    // They should provide a callback from the beginning
    GS_ASSERT(callback);
    if (!callback)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_HotError,
            "%s :: %s (%d): developer callbacks are NULL!\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    result = d2giServiceCheckContentUpdates(d2gInstance,
                                            d2gCatalog,
                                            requiredOnly,
                                            callback,
                                            userData);
                                     
    if (GS_SUCCEEDED(result))
        return GS_SUCCESS; 
    else
        return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gUpdateManifestInstalledContent
//  Brief
//      This function is updates to the manifest information kept by the SDK .
//      Whenever a new content is downloaded and installed, it should be called
//		after installing it.
//  Parameters
//      theItemId   [in] Item Id which the content belongs to.
//      theDownload [in] Download item for the particular content.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is a mandatory function and should be call after each successful installation
//      of a downloaded item. This helper function is used in conjunction with the Check
//		for updates service call.
//
GSResult d2gUpdateManifestInstalledContent( D2GInstancePtr  theInstance,
                                            D2GItemId       theItemId, 
                                            D2GDownloadItem *theDownload)
{
    D2GIManifestRecord manifest;
    GS_ASSERT(theInstance);
    GS_ASSERT(theDownload);

    if(!theDownload || !theInstance)

    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance, please create and initialize.\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }

    manifest.itemId  = theItemId;
    manifest.urlId   = theDownload->mUrlId;
    manifest.version = theDownload->mVersion;
    
    return d2giUpdateManifestRecord( (D2GIInstance *) theInstance, manifest);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//d2gUpdateManifestRemovedContent
//  Brief
//      This function is deletes the un-installed content to the manifest 
//      information kept by the SDK .
//      Whenever a new content is deleted, it should be called.
//  Parameters
//      theInstance [in] This is the pointer created by the d2gCreateInstance.
//      theItemId   [in] Item Id which the content belongs to.
//      theDownload [in] Download item for the particular content.
//  Return Value
//      GSResult – Enumeration indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is a mandatory function and should be call after each uninstallation
//      of an item.This helper function is used in conjunction with the Check
//		for updates service call.
//
GSResult d2gUpdateManifestRemovedContent(D2GInstancePtr  theInstance,
                                         D2GItemId       theItemId, 
                                         D2GDownloadItem *theDownload)
{
    D2GIManifestRecord manifest;

    GS_ASSERT(theDownload);
    GS_ASSERT(theInstance);
    if (!theDownload || !theInstance)

    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError,
            "%s :: %s (%d): called with NULL instance, please create and initialize.\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
    }

    // verify availability check
    if (GSIGetAvailable()!=GSIACAvailable)
    {
        gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State,GSIDebugLevel_WarmError, 
            "%s :: %s (%d): called without doing availability check", 
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_NoAvailabilityCheck);
    }
    manifest.itemId  = theItemId;
    manifest.urlId   = theDownload->mUrlId;
    manifest.version = theDownload->mVersion;

    return d2giDeleteManifestRecord((D2GIInstance *)theInstance, manifest);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 
// For testing purposes only!
//
GSResult d2gSetServiceURL(D2GInstancePtr theInstance, 
						  const char * serviceUrl)
{
    GS_ASSERT(theInstance != NULL);
    GS_ASSERT(serviceUrl != NULL);

    // Check parameters
    if (theInstance == NULL || serviceUrl == NULL)
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);

    if (strlen(serviceUrl) >= GS_D2G_MAX_BASE_URL_LEN)
        return GS_D2G_ERROR(GSResultSection_State, GSResultCode_SizeExceedsMaximum);
    ((D2GIInstance *)theInstance)->mServiceURLBase = gsimalloc(GS_D2G_MAX_BASE_URL_LEN);
    strcpy(((D2GIInstance *)theInstance)->mServiceURLBase , serviceUrl);
    printf("Service URL is %s", ((D2GIInstance *)theInstance)->mServiceURLBase);
    return GS_SUCCESS;
}

GSResult d2gSetServiceTimeout(D2GInstancePtr theInstance, 
							  gsi_time timeoutMs)
{
	D2GIInstance *d2gInstance = (D2GIInstance *)theInstance;
	
	GS_ASSERT(d2gInstance);
	if (!d2gInstance)
	{
		gsDebugFormat( GSIDebugCat_Direct2Game, GSIDebugType_State, GSIDebugLevel_WarmError,
			"%s :: %s (%d): called with NULL instance\n",
			__FILE__, __FUNCTION__, __LINE__);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_InvalidParameters);
	}
	
	d2gInstance->mTimeoutMs = timeoutMs;
	return GS_SUCCESS;
}
//////////////////////////////////////////////////////////////////
// For Content Updates testing purposes only 
void d2gDisplayManifestFileContents(D2GInstancePtr d2gInstance)
{
    D2GIInstance *theInstance = (D2GIInstance *) d2gInstance;
    char        manifestRead[GS_D2G_MAX_FILE_BUFFER_LEN];
    FILE        *pManifest = NULL;
    char        *manifestFileName = NULL;

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
        printf (" NO MANIFEST");
    } 
    else
    {
        memset(manifestRead, '\0', sizeof(manifestRead)); 
        while(!feof(pManifest)&& (fgets(manifestRead, sizeof(manifestRead)-1, pManifest) != NULL))
        {  
            D2GIString manifestXml;
            manifestXml.str = gsimalloc((strlen(manifestRead)+2) * sizeof(char));         
            memset (manifestXml.str, '\0',(strlen(manifestRead)*sizeof(char)) );
            // Uses URL safe encoding
            B64Decode(manifestRead, manifestXml.str, (int) strlen(manifestRead), &manifestXml.len, 2);
            printf("%s\n", manifestXml.str);
            gsifree(manifestXml.str);
        }
        fclose(pManifest);
    }
}