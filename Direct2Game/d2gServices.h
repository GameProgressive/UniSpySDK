/**
* d2gServices.h
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __D2GSERVICES_H__
#define __D2GSERVICES_H__


// ***** Commerce web services.
//
// ***** PUBLIC INTERFACE AT THE BOTTOM OF THE FILE

#include "../ghttp/ghttpSoap.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"
#include "d2gUtil.h"
#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// general header SOAP elements
#define GS_D2G_RESULT_SECTION              "status"
#define GS_D2G_RESULT_CODE                 "code"
#define GS_D2G_RESULT_MSG                  "message"
#define GS_D2G_COUNT_ELEMENT               "count"
#define GS_D2G_REQUEST                     "request"
#define GS_D2G_EXTRA_INFO                  "extensiondata"
#define GS_D2G_ETRA_INFO_LIST              "extensiondatalist"
#define GS_D2G_CATALOG_ITEM_LIST           "catalogitemlist"
#define GS_D2G_CATALOG_ITEM                "catalogitem"
#define GS_D2G_ORDER_ITEM_PURCHASE         "orderitempurchase"
#define GS_D2G_ORDER_ITEM_PURCHASES        "orderitempurchases"
// item list soap elements

// Catalog Service SOAP elements
#define GS_D2G_GETALLITEMS_REQUEST         "GetAllItems"
#define GS_D2G_GETALLITEMS_RESULT          "GetAllItemsResult"
#define GS_D2G_GETCATEGORIES_REQUEST       "GetCategories"
#define GS_D2G_GETCATEGORIES_RESULT        "GetCategoriesResult"
#define GS_D2G_GETITEMSBYCATEGORY_REQUEST  "GetItemsByCategory"
#define GS_D2G_GETITEMSBYCATEGORY_RESULT   "GetItemsByCategoryResult"
#define GS_D2G_GETITEMDETAILS_REQUEST      "GetItemDetails"
#define GS_D2G_GETITEMDETAILS_RESULT       "GetItemDetailsResult"
#define GS_D2G_GETSTOREAVAIL_REQUEST       "GetStoreAvailability"
#define GS_D2G_GETSTOREAVAIL_RESULT        "GetStoreAvailabilityResult"
#define GS_D2G_GETDOWNLOADINFO_REQUEST     "GetDownloadInfo"
#define GS_D2G_GETDOWNLOADINFO_RESULT      "GetDownloadInfoResult"
#define GS_D2G_GET_STORE_EXTENSION_DATA_REQUEST  "GetStoreExtensionData"
#define GS_D2G_GET_STORE_EXTENSION_DATA_RESULT   "GetStoreExtensionDataResult"
#define GS_D2G_GETITEMACTIVATIONDATA_REQUEST    "GetItemActivationData"
#define GS_D2G_GETITEMACTIVATIONDATA_RESULT     "GetItemActivationDataResult"
#define GS_D2G_CHECKFORCONTENTUPDATES_REQUEST   "CheckForContentUpdates"
#define GS_D2G_CHECKFORCONTENTUPDATES_RESULT    "CheckForContentUpdatesResult"

// Catalog Service SOAP Actions
#define GS_D2G_GETITEMDETAILS_SOAP        "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetItemDetails\""
#define GS_D2G_GETITEMSBYCATEGORY_SOAP    "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetItemsByCategory\""
#define GS_D2G_GETALLITEMS_SOAP           "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetAllItems\""
#define GS_D2G_GETCATEGORIES_SOAP         "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetCategories\""
#define GS_D2G_GETSTOREAVAIL_SOAP         "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetStoreAvailability\""
#define GS_D2G_GETDOWNLOADINFO_SOAP        "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetDownloadInfo\""
#define GS_D2G_GET_STORE_EXTENSION_DATA_SOAP "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetStoreExtensionData\""
#define GS_D2G_GETITEMACTIVATIONDATA_SOAP "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/GetItemActivationData\""								
#define GS_D2G_CHECKFORCONTENTUPDATES_SOAP "SOAPAction: \"http://gamespy.net/commerce/2009/02/CatalogService/CheckForContentUpdates\""
			 
// Account Service SOAP Objects
#define GS_D2G_GETUSERCREDITCARDS_REQUEST        "GetUserCreditCards"
#define GS_D2G_GETUSERCREDITCARDS_RESPONSE       "GetUserCreditCardsResponse"
#define GS_D2G_GETUSERCREDITCARDS_RESULT         "GetUserCreditCardsResult"

// Account Service SOAP Actions
#define GS_D2G_GETUSERCREDITCARDS_SOAP      "SOAPAction: \"http://gamespy.net/commerce/2009/02/AccountService/GetUserCreditCards\""

// Purchase Service SOAP Actions
#define GS_D2G_GETORDERTOTAL_REQUEST       "GetOrderTotal"
#define GS_D2G_GETORDERTOTAL_RESPONSE      "GetOrderTotalResponse"
#define GS_D2G_GETORDERTOTAL_RESULT        "GetOrderTotalResult"
#define GS_D2G_PURCHASEITEMS_REQUEST       "PlaceOrder"
#define GS_D2G_PURCHASEITEMS_RESPONSE      "PlaceOrderResponse"
#define GS_D2G_PURCHASEITEMS_RESULT        "PlaceOrderResult"
#define GS_D2G_PLACEDORDER_REQUEST         "GetPlacedOrder" 
#define GS_D2G_PLACEDORDER_RESPONSE        "GetPlacedOrderResponse"
#define GS_D2G_PLACEDORDER_RESULT          "GetPlacedOrderResult"
#define GS_D2G_GETPURCHASEHISTORY_REQUEST  "GetPurchaseHistory"
#define GS_D2G_GETPURCHASEHISTORY_RESPONSE "GetPurchaseHistoryResponse"
#define GS_D2G_GETPURCHASEHISTORY_RESULT   "GetPurchaseHistoryResult"

#define GS_D2G_GETORDERTOTAL_SOAP         "SOAPAction: \"http://gamespy.net/commerce/2009/02/PurchaseService/GetOrderTotal\""
#define GS_D2G_PURCHASEITEMS_SOAP         "SOAPAction: \"http://gamespy.net/commerce/2009/02/PurchaseService/PlaceOrder\""
#define GS_D2G_PLACEDORDER_SOAP           "SOAPAction: \"http://gamespy.net/commerce/2009/02/PurchaseService/GetPlacedOrder\""
#define GS_D2G_GETPURCHASEHISTORY_SOAP    "SOAPAction: \"http://gamespy.net/commerce/2009/02/PurchaseService/GetPurchaseHistory\""


#define GS_D2G_NAMESPACE              "gsc"
#define GS_D2G_AUTHSERVICE_NAMESPACE  "gsa"
#define GS_D2G_NAMESPACE_COUNT  1

#define GS_D2G_CATALOG_SERVICE_URL_FORMAT "catalogservice.svc"
#define GS_D2G_ACCOUNT_SERVICE_URL_FORMAT "accountservice.svc"
#define GS_D2G_PURCHASE_SERVICE_URL_FORMAT "purchaseservice.svc"

// This is used by the manifest file
#define GS_D2G_MANIFEST_MAX_XML_LEN (1024)

// Private data to help match async requests with callbacks
typedef struct D2GIRequestData
{
	D2GIInstance* mInstance;
	D2GICatalog*  mCatalog;
	void* mUserData; // data from the developer, to be passed through
	union 
	{
		D2GLoadCatalogItemsByCategoryCallback mGetItemsByCategoryCallback;
//		D2GGetCategoriesCallback           mGetCategoriesCallback;
		D2GLoadCatalogItemsCallback        mLoadCatalogItemsCallback;
		D2GGetUserCreditCardsCallback	   mGetUserCreditCardsCallback;
		D2GGetOrderTotalCallback           mGetOrderTotalCallback;
		D2GStartOrderCallback              mBeginPurchaseCallback;
		D2GGetStoreAvailabilityCallback    mGetStoreAvailCallback;
		D2GIsOrderCompleteCallback         mIsOrderCompleteCallback;
        D2GGetPurchaseHistoryCallback      mGetPurchaseHistoryCallback;
        D2GLoadExtraCatalogInfoCallback           mGetExtraInfoCallback;
        D2GGetItemActivationDataCallback   mGetActivationDataCallback;
        D2GCheckContentUpdatesCallback     mCheckContentUpdatesCallback;
	} mUserCallback;
	
	D2GDownloadProgressCallback mProgressCallback;
	D2GDownloadCompleteCallback mCompleteCallback;
	const gsi_char *mDownloadLocation;
} D2GIRequestData;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Internal service functions
GSResult d2giServiceLoadCatalogItems (D2GIInstance *theInstance, 
                                      D2GICatalog  *theCatalog, 
                                      D2GLoadCatalogItemsCallback callback, 
                                      void         *userData);
                                      
GSResult d2giServiceGetUserCreditCards(D2GIInstance *theInstance, 
                                       D2GICatalog  *theCatalog, 
                                       gsi_bool validOnly, 
                                       D2GGetUserCreditCardsCallback callback, 
                                       void         *userData);
// 
// GSResult d2giServiceGetCategories(D2GIInstance  *theInstance, 
//                                   D2GICatalog   *theCatalog, 
//                                   D2GGetCategoriesCallback callback, 
//                                   void          *userData);
                                  
GSResult d2giServiceGetItemsByCategory(D2GIInstance *theInstance, 
                                       D2GICatalog  *theCatalog, 
                                       const UCS2String theCategory, 
                                       D2GLoadCatalogItemsByCategoryCallback usercallback, 
                                       void         *userData);
                                       
GSResult d2giServiceGetOrderTotal(D2GIInstance  *theInstance, 
                                  D2GICatalog   *theCatalog, 
                                  gsi_u32       accountId, 
                                  D2GItemId     *itemIds, 
                                  gsi_u32       itemCount, 
                                  gsi_u32       *itemQuantities, 
                                  D2GGetOrderTotalCallback callback, 
                                  void          *userData);
                                  
GSResult d2giServicePurchaseItems(D2GIInstance  *theInstance, 
                                  D2GICatalog   *theCatalog, 
                                  const D2GOrderTotal   *theOrderTotal, 
                                  D2GStartOrderCallback callback, 
                                  void          *userData);
                                  
GSResult d2giServiceIsOrderComplete(D2GIInstance    *theInstance, 
                                    D2GICatalog     *theCatalog, 
                                    const D2GOrderPurchase  *theOrderPurchase, 
                                    D2GIsOrderCompleteCallback callback, 
                                    void            *userData);
                                    
GSResult d2giServiceGetStoreAvail(D2GIInstance  *theinstance, 
                                  D2GICatalog   *theCatalog, 
                                  D2GGetStoreAvailabilityCallback callback, 
                                  void          *userData);
                                  
GSResult d2giServiceGetDownloadInfo(D2GIInstance    *theInstance, 
                                    D2GICatalog     *theCatalog, 
                                    D2GFileId fileId, 
                                    const gsi_char  *downloadLocation, 
                                    D2GDownloadProgressCallback progressCallback, 
                                    D2GDownloadCompleteCallback completeCallback, 
                                    void            *userData);
                                    
GSResult d2giServiceGetPurchaseHistory(D2GIInstance *theInstance, 
                                       D2GICatalog  *theCatalog, 
                                       D2GGetPurchaseHistoryCallback callback, 
                                       void         *userData);
                                       
GSResult d2giServiceLoadExtraInfo(D2GIInstance *theInstance, 
                                 D2GICatalog  *theCatalog, 
                                 D2GLoadExtraCatalogInfoCallback callback, 
                                 void         *userData );
                                 
GSResult d2giServiceGetItemActivationData(D2GIInstance *theInstance, 
                                          D2GICatalog  *theCatalog, 
                                          D2GGetItemActivationDataCallback callback, 
                                          D2GPackageId thePackageId, 
                                          void         *userData);
                                   
GSResult d2giServiceCheckContentUpdates(D2GIInstance *theInstance, 
                                        D2GICatalog  *theCatalog, 
                                        gsi_bool     requiredOnly, 
                                        D2GCheckContentUpdatesCallback callback, 
                                        void         *userData);


// Data functions
GSResult d2giFreeGetItemsByCategoryResponse(D2GIInstance *theInstance, 
                                            D2GLoadCatalogItemsByCategoryResponse *response, 
                                            GSResult result, 
                                            gsi_bool freeItem);
                                            
                                   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__D2GSERVICES_H__
