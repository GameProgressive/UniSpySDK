// GameSpy Direct2Game SDK
// This SDK is designed and developed by GameSpy Tech.
// Copyright (c) 2008, GameSpy Technology

#ifndef __GS_DIRECT2GAME_H__
#define __GS_DIRECT2GAME_H__
///////////////////////////////////////////////////////////////////////////////
// This header file should be used as a reference for the Direct2Game interface
// All structures and SDK API functions are defined in this file.

#include "../ghttp/ghttpSoap.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"
#include "../common/gsResultCodes.h"
#include "../webservices/AuthService.h"

#if defined(__cplusplus)
extern "C"
{
#endif

	// Defaults
#define GS_D2G_DEFAULT_CATALOG_REGION   L"global"      
#define GS_D2G_DEFAULT_CATALOG_VERSION  0
#define GS_D2G_DEFAULT_CULTURECODE      L"en-US"
#define GS_D2G_DEFAULT_CURRENCYCODE     L"USD"

// Summary
//     D2GResultCode enum contains the error codes 
//     which might be returned by the D2G SDK API calls.
// 
// Remarks 
//     To obtain these codes for use in implementation, Please use the 
//     macros associated with GSResult.  They can be found in gsResultCodes.h
// Example
//	   To obtain the D2GResultCode from a GSResult, call:
// <code lang="c++">
//     GSResult result;
//     ...
//     D2GResultCode d2gCode = GS_RESULT_CODE(result);
// </code>
typedef enum D2GResultCode
{
	/// account problems
	D2GResultCode_InsufficientFunds,    
	D2GResultCode_NoPaymentMethodFound, 

	/// store availability
	D2GResultCode_StoreOnline,                 
	D2GResultCode_StoreOfflineForMaintenance,   
	D2GResultCode_StoreOfflineRetired,         
	D2GResultCode_StoreNotYetLaunched,  

	/// Order
	D2GResultCode_Order_Ok,                     
	D2GResultCode_Order_DuplicateItemIds,       
	D2GResultCode_Order_OrderInitiateFailed,    
	D2GResultCode_Order_UserNotAuthorized,     
	D2GResultCode_Order_UserNotFound,           
	D2GResultCode_Order_UserAccountNotFound,   
	D2GResultCode_Order_OrderItemFailed,        
	D2GResultCode_Order_BillingFailed,          
	D2GResultCode_Order_BillingPending,         
	D2GResultCode_Order_BillingRequestNotFound,
	D2GResultCode_Order_InsufficientFunds,
	D2GResultCode_Order_RequiredFieldNotSet,
	D2GResultCode_Order_UnexepectedError,
	D2GResultCode_Order_UnknownResultCode,

	/// order item
	D2GResultCode_OrderItem_Ok,
	D2GResultCode_OrderItem_CountryRestriction,
	D2GResultCode_OrderItem_AgeRestriction,
	D2GResultCode_OrderItem_QuantityRestriction,
	D2GResultCode_OrderItem_ItemNotAuthorized,
	D2GResultCode_OrderItem_ItemDoesNotExist,
	D2GResultCode_OrderItem_PriceChanged,
	D2GResultCode_OrderItem_ItemRefunded,
	D2GResultCode_OrderItem_VariantDoesNotExist,
	D2GResultCode_OrderItem_UnexepectedError,
	D2GResultCode_OrderItem_UnspecificedRestriction,
	D2GResultCode_OrderItem_UnknownResultCode,

	/// download
	D2GResultCode_File_NotOwnedByUser,

	/// catalog
	D2GResultCode_Catalog_Empty,
	D2GResultCode_Catalog_MaxCategoryNumReached,

	/// extra info
	D2GResultCode_ExtraInfo_Empty,
	D2GResultCode_ExtraInfo_KeyNotFound,

	D2GResultCode_Max
} D2GResultCode;

// Summary
// Sort direction parameters for the d2g sort functions
typedef enum D2GSortDirection
{
	D2GSort_Ascending,
	D2GSort_Descending
} D2GSortDirection;

// Summary
// Used in d2gContentUpdatesCallback.
// Each enum indicates content update type. 
typedef enum D2GContentUpdateType
{
	D2GContentNew     = 10,
	D2GContentUpdated = 20,
	D2GContentRemoved = 30
} D2GContentUpdateType;

// Summary
// Used in d2gContentUpdatesCallback.
// Each enum indicates how the application should proceed 
// when installing an update. 
typedef enum D2GDownloadType
{
	D2GDownloadForced	 = 0,
	D2GDownloadMandatory = 1,
	D2GDownloadOptional	 = 2
} D2GDownloadType;


///Type Definitions  

// Summary
// D2GInstancePtr is a reference to D2G SDK Instance. 
// This pointer has to be maintained throughout the life time of the SDK.
typedef void* D2GInstancePtr; 

// Summary
// D2GCatalogPtr is used by the SDK to get a presence of a specific store.
// It is a reference to a unique identifier based on game id, region, version, 
// and access token.
typedef void*   D2GCatalogPtr;

///Basic Types

typedef gsi_u32     D2GItemId;      // a 32-bit number to uniquely identify an item
typedef gsi_u32     D2GUrlId;       // a 32-bit number to uniquely identify a downloadable item     
typedef gsi_u32     D2GFileId;      // a 32-bit number which uniquely identifies a file 
typedef gsi_u32     D2GPackageId;   // a 32-bit number used to identify an item in a DRM solution 

/// Base Structures

// Summary
// D2GBasicItemInfo contains the basic information about an item 
// that exists on the backend.
// It is part of D2GOrderItem and D2GCatalogItem structures.
typedef struct D2GBasicItemInfo
{
	D2GItemId       mItemId;            // unique item id.
	UCS2String      mExternalItemCode;  // unique external item id.
	UCS2String      mName;              // the name of the item. 
	UCS2String      mPrice;             // the unit price of the item in string format
	UCS2String      mTax;               // the tax for the unit price for this particular item 
} D2GBasicItemInfo;

// Summary
// D2GGeoInfo contains the region information associated with an item.  Used
//		 by D2GOrderInfo and D2GCatalogItem.
typedef struct D2GGeoInfo
{
	UCS2String  mCurrencyCode; // The currency set up for the current store
	UCS2String  mCultureCode;  // A culture code associated with the catalog, e.g. "en-us".
} D2GGeoInfo;


/// Extra Info

// Summary
// D2GExtraInfo contains a <Key,Value> pair. 
// This can be used by the developer to specify meta data for the store.
// The data this structure contains is configured on the backend admin site.
typedef struct D2GExtraInfo
{
	UCS2String mKey;      
	UCS2String mValue;
} D2GExtraInfo;

// Summary
// D2GExtraInfoList contains a list of D2GExtraInfo objects
typedef struct D2GExtraInfoList
{
	gsi_u32          mCount;                // Number of D2GExtraInfo items that mExtraInfoElements points to
	D2GExtraInfo     *mExtraInfoElements;   // Actual D2GExtraInfo objects that contain the <key, value> pairs
} D2GExtraInfoList;

/// D2G Catalog Types

// Summary
// Names of Images associated with a catalog item
typedef struct D2GImageList
{
	gsi_u32     mCount;         // Number of images
	UCS2String  *mImageName;    // A pointer to an array of names of an item's images. 
} D2GImageList;

// Summary
// Categories associated with a catalog item
typedef struct D2GCategoryList
{
	gsi_u32     mCount;           // Number of categories.
	UCS2String  *mCategoryNames;   // A pointer to the array of category names.
} D2GCategoryList;

// Summary
// Product information of a catalog item
typedef struct D2GProductInfo
{
	UCS2String      mPublisher;     // Name of the publisher
	UCS2String      mDeveloper;     // Name of the developer
	UCS2String      mSummary;       // Summary as set by the developer
	time_t          mReleaseDate;   // Date when item was released.
	gsi_u32			mFileSize;      // Total size of the files associated with the item in bytes.
} D2GProductInfo;

// Summary
// D2GCatalogItem contains catalog information for an item.
typedef struct D2GCatalogItem
{
	D2GBasicItemInfo mItem; // Basic information about this product
	D2GGeoInfo mGeoInfo; // Geographical information about this catalog item
	D2GProductInfo mProductInfo; // Information about the publisher/developer associated with this item
	D2GCategoryList mCategories; // Categories list that contain organizational information based on category
	D2GImageList mImages; // Image list for image URLs
	D2GExtraInfoList mExtraItemInfoList; // Extra Info list related to this specific item
} D2GCatalogItem;

// Summary
// List of catalog items
typedef struct D2GCatalogItemList
{    
	gsi_u32         mCount;         //  Number of items in the list 
	D2GCatalogItem  *mCatalogItems; // A pointer to an array of catalog items.
} D2GCatalogItemList;

///Accounts

// Summary
// D2GCreditCardInfo contains the credit card information for the
//		 authenticated user.
// A credit card must exist with the user's account on the back end in order
//		 to use 
// the purchasing functionality.
typedef struct D2GCreditCardInfo
{
	gsi_u32    mAccountId;          // The unique account id used when ordering.
	UCS2String mCreditCardType;     // String for the credit card type.
	time_t     mExpirationDate;     // the expiration date for the credit card.  
	gsi_bool   mIsDefault;          // true if this is the default card on the user's account.
	int        mLastFourDigits;     // the last for digits of the credit card.
} D2GCreditCardInfo;

// Summary
// D2GCreditCardInfoList is a container for a list of D2GCreditCardInfo 
// associated with the user's account. It is part of the
// D2GGetUserCreditCardsResponse
typedef struct D2GCreditCardInfoList
{
	gsi_u32           mCount;           // The number of credit card 
	D2GCreditCardInfo *mCreditCardInfos; // A pointer to the array which contains the credit cards.
} D2GCreditCardInfoList;

/// D2G Order Item Types

// Summary
// Order Validation used as part of an order to determine if the total order
//		 has been processed
// appropriately. It is part of  D2GOrderItem and D2GOrderInfo structures.
typedef struct D2GOrderValidation
{
	gsi_bool    mIsValid;   // gsi_true, if order valid. gsi_false indicates that the order is invalid, and the items below need to be checked.
	UCS2String  mMessage;   // The text message for validation sent from the back end if invalid.
	GSResult    mResult;    // Set to a result with a D2GResultCode
} D2GOrderValidation;

// Summary
// Top level order information that is contained by D2GOrderTotal and
//		 D2GOrderPurchase.
typedef struct D2GOrderInfo
{
	gsi_u32            mAccountId;     // account id from Credit card that was used to make the purchase.
	UCS2String         mRootOrderGuid; // Root order ID that uniquely identifies an order
	D2GGeoInfo         mGeoInfo;       // Culture and currency information for the order
	D2GOrderValidation mValidation;    // Validation for the entire order
	UCS2String         mSubTotal;      // Subtotal for the entire order
	UCS2String         mTax;           // Tax for the entire order
	UCS2String         mTotal;         // Total price for the entire order
} D2GOrderInfo;

// Summary
// D2GOrderItemTotal contains information about the quantity, subtotal, and
//		 total
// for a given item.  It is contained by D2GOrderItem.
typedef struct D2GOrderItemTotal
{
	gsi_u32        mQuantity;
	UCS2String     mSubTotal;
	UCS2String     mTotal;
} D2GOrderItemTotal;

// Summary
// D2GOrderItem is contained by D2GOrderPurchase, and D2GOrderTotal as a list.
// It contains some catalog information, pricing, and validation.
typedef struct D2GOrderItem
{
	D2GBasicItemInfo   mItem;       // Item information similar to the catalog
	D2GOrderValidation mValidation; // Validation information for the given item.
	D2GOrderItemTotal  mItemTotal;  // Pricing information for the given item.
} D2GOrderItem;

// Summary
// List of order items of an Order Total.  They are used to show 
// the order total for each item.
typedef struct D2GOrderItemList
{
	gsi_u32         mCount;        // Count of mOrderItems.
	D2GOrderItem    *mOrderItems;  // Order item that contains per item total price information.
} D2GOrderItemList;

// Summary
// D2GOrderTotal contains the order information contained in the
//		 D2GOrderTotalResponse.
// It is an input to the d2gStartOrder.
typedef struct D2GOrderTotal
{
	time_t           mQuoteDate;    // The date that the quote was given for an order.
	D2GOrderInfo     mOrder;        // Top level order information
	D2GOrderItemList mOrderItemList;   // Items which are part of the order.  See D2GOrderItemList.
} D2GOrderTotal;

// Summary
// A License associated with a purchased item 
typedef struct D2GLicenseItem
{
	UCS2String mLicenseKey;     // the license key for the purchased item
	UCS2String mLicenseName;    // the name for the license. 
} D2GLicenseItem;

// Summary
// D2GLicenseItemList contains the List of licenses for a purchased item.
// If an item has more than one downloadable items,
// there may be more than one license associated with it.
// It is contained by the D2GOrderItemPurchase structure.
typedef struct D2GLicenseItemList
{
	gsi_u32        mCount;      // The number of licenses in the list
	D2GLicenseItem *mLicenses;  // a pointer to the array which keeps the list of licenses.
} D2GLicenseItemList;

// Summary
// D2GDownloadItem contains the download information of a purchased item. 
//It is also used to start the download process. 
typedef struct D2GDownloadItem
{
	D2GUrlId    mUrlId;         // the UrlId of this particular download
	D2GFileId   mFileId;        // the File Id of this particular download
	gsi_u32     mSequence;      // Some downloads may require one file to be installed before another.  This number indicates the order.
	UCS2String  mName;          // A string which contains the name of the download. 
	UCS2String  mAssetType;     // The asset type indicates how the game will organize files, e.g. map, avitar.
	float       mVersion;       // The version number for this download.
} D2GDownloadItem;

// Summary
// D2GDownloadItemList contains a list of downloads for an item. 
// This is contained by the D2GOrderItemPurchase.
typedef struct D2GDownloadItemList
{
	gsi_u32         mCount;       // The number of download items in the mDownload array.
	D2GDownloadItem *mDownloads;  // The pointer to the array which contains a list of download items.
} D2GDownloadItemList;

// Summary
// D2GOrderItemPurchaseList contains a list of item purchases that are available
// during the purchase process, and purchase history. 
typedef struct D2GOrderItemPurchaseList
{
	gsi_u32              mCount;                      // Number of items in the list
	struct D2GOrderItemPurchase *mOrderItemPurchases; // Pointer to the array of type D2GOrderItemPurchase
} D2GOrderItemPurchaseList;

/// NOTE: D2GOrderItemPurchaseList needs to be defined first because its
//		 nested in D2GOrderItemPurchase 

// Summary
// D2GOrderItemPurchase contains the top level purchase information which in
//		 turn
// can be nested.  mPurchaseList will be filled in with a count if there are
//		 nested items.
typedef struct D2GOrderItemPurchase
{
	D2GOrderItem                mOrderItem;
	D2GLicenseItemList          mLicenseList;
	D2GDownloadItemList         mDownloadList;
	D2GOrderItemPurchaseList    mPurchaseList;
} D2GOrderItemPurchase;

// Summary
// D2GOrderPurchase contains the overall purchase information. It is part of 
// the D2GPurchaseHistory, D2GStartOrderResponse and D2GIsOrderCompleteResponse.
typedef struct D2GOrderPurchase
{
	time_t                   mPurchaseDate;  // The date that the purchase was made.
	D2GOrderInfo             mOrder;         // Top level order information
	D2GOrderItemPurchaseList mItemPurchases; // List of items purchased
} D2GOrderPurchase;

// Summary
// D2GPurchaseHistory contains the list of purchases made by the user. 
typedef struct D2GPurchaseHistory
{
	gsi_u32 mCount;                 // Number of purchases
	D2GOrderPurchase *mPurchases;   // A pointer to an array of past purchases
} D2GPurchaseHistory;

/// Check content updates

// Summary
// D2GContentUpdate contains the content update information for an item.
typedef struct D2GContentUpdate
{
	D2GItemId       mItemId;        // Unique item id for the content update 
	D2GDownloadItem mDownloadItem;  // Download information, see D2GDownloadItem
	gsi_u32         mChangeType;    // See D2GContentUpdateType
	D2GDownloadType mDownloadType;  // See D2GDownloadType
} D2GContentUpdate;

// Summary
// D2GContentUpdateList contains a list of content updates.
// It is part of the D2GCheckContentUpdatesResponse.
typedef struct D2GContentUpdateList
{   
	gsi_u32           mCount;       // Number of downloaded items
	D2GContentUpdate *mContentUpdates;   // A pointer to an array of downloaded items.
}D2GContentUpdateList;


/// Responses to the services requests.

// Summary
// The response message for an Extra Info Request
typedef struct D2GLoadExtraCatalogInfoResponse
{
	GHTTPResult      mHttpResult;	    // HTTP Result 
	D2GExtraInfoList *mExtraCatalogInfoList;   // See D2GExtraInfoList. This must be freed using d2gFreeExtraInfoList
} D2GLoadExtraCatalogInfoResponse;

// Summary
// The response message for Get Store Availability Request
typedef struct D2GGetStoreAvailabilityResponse
{
	GHTTPResult mHttpResult;        // HTTP Result 
	GSResult    mAvailabilityCode;  // A code returned by the back end. See D2GResultCode_Store... codes
} D2GGetStoreAvailabilityResponse;

// Summary
// The response message for Load Catalog Items Request
typedef struct D2GLoadCatalogItemsResponse
{
	GHTTPResult     mHttpResult;    // HTTP Result 
	D2GCatalogItemList *mItemList;     // See D2GCatalogItems. This must be freed using d2gFreeCatalogItems.
} D2GLoadCatalogItemsResponse;

// Summary
// The response message for Load Catalog Items by Category Request
typedef struct D2GLoadCatalogItemsByCategoryResponse
{
	GHTTPResult     mHttpResult;    // HTTP Result 
	D2GCatalogItemList *mItemList;     // See D2GCatalogItems. This must be freed using d2gFreeCatalogItems.
} D2GLoadCatalogItemsByCategoryResponse;

// Summary
// The response message for Get User Credit Cards Request
typedef struct D2GGetUserCreditCardsResponse
{
	GHTTPResult           mHttpResult;	        // HTTP Result 
	D2GCreditCardInfoList *mListOfCreditCards;  // See D2GCreditCardInfoList. This must be free using d2gFreeCreditCardInfoList.
} D2GGetUserCreditCardsResponse;

// Summary
// The response message for Get Order Total Request
typedef struct D2GGetOrderTotalResponse
{
	GHTTPResult   mHttpResult;    // HTTP Result 
	D2GOrderTotal *mOrderTotal;	  // See D2GOrderTotal. This must be freed using d2gFreeOrderTotal.
} D2GGetOrderTotalResponse;

// Summary
// The response message for Start Order Request
typedef struct D2GStartOrderResponse
{
	GHTTPResult      mHttpResult;       // HTTP Result   
	D2GOrderPurchase *mOrderPurchase;   // See D2GOrderPurchase. This must be freed using d2gFreeOrderPurchase.
} D2GStartOrderResponse;

// Summary
// The response message for Is Order Complete Request
typedef struct D2GIsOrderCompleteResponse
{
	GHTTPResult      mHttpResult;       // HTTP Result 
	D2GOrderPurchase *mOrderPurchase;   // See D2GOrderPurchase. This must be freed using d2gFreeOrderPurchase.
} D2GIsOrderCompleteResponse;

// Summary
// The response message for Get Purchase History Request.
typedef struct D2GGetPurchaseHistoryResponse
{
	GHTTPResult       mHttpResult;	        // HTTP Result  
	D2GPurchaseHistory *mPurchaseHistory;   // See D2GPurchaseHistory. This must freed using d2gFreePurchaseHistory.
} D2GGetPurchaseHistoryResponse;

// Summary
// The response message for Get Item Activation Request
// Used as part of an DRM solution.
typedef struct D2GGetItemActivationResponse
{
	GHTTPResult mHttpResult;        // HTTP Result 
	UCS2String  mStatusMessage;     // Status message from the back end. This must be freed using gsifree.
	D2GItemId   mItemId;            // The unique item id.
	UCS2String  mActivationCode;    // Activation code for the item. This must be freed using gsifree.
} D2GGetItemActivationResponse;

// Summary
// The response message for Check Content Updates Request
typedef struct D2GCheckContentUpdatesResponse
{
	GHTTPResult     mHttpResult;    // HTTP Result 
	D2GContentUpdateList *mContentUpdateList;    // See D2GDownloadList. This must be freed using d2gFreeDownloadList.
} D2GCheckContentUpdatesResponse;

// Public SDK Interface

// Initialization and Termination
//  d2gCreateInstance
//  Summary
//      Creates Direct2Game SDK instance. 
//  Parameters
//      None.
//  Return Value 
//      D2GInstancePtr  - a void pointer to the internal D2G Instance.
//  Remarks
//      This function is mandatory. This API function must be called first
//		 before 
//      any other D2G API function.
//  See Also 
//      d2gCleanup
//
D2GInstancePtr d2gCreateInstance();

//  d2gInitialize
//  Summary
//      Initializes the SDK Instance created by d2gCreateInstance. 
//  Parameters
//      theInstance     : [in] This pointer is created by the d2gCreateInstance.
//      theCertificate  : [in] This is the certificate obtained from
//		 authorization 
//                           service.
//      thePrivateData  : [in] This is the private data obtained from
//		 authorization 
//                           service.
//  Return Value 
//      GSResult - Enumeration indicating success (GS_SUCCESS) or a failure
//		 code.
//  Remarks
//      This function is mandatory. 
//  See Also 
//      d2gCleanup
//
GSResult d2gInitialize( D2GInstancePtr  theInstance, 
	const GSLoginCertificate * theCertificate, 
	const GSLoginPrivateData * thePrivateData);

//  d2gCleanup
//  Summary
//      De-allocates memory allocated by d2gCreateInstance and d2gInitialize.  
//  Parameters
//      theInstance     : [in] This pointer is created by the d2gCreateInstance.
//  Return Value 
//      None.
//  Remarks
//      This function is mandatory. 
//  See Also 
//      d2gCreateInstance
//
void d2gCleanup(D2GInstancePtr theInstance);

//  d2gCreateCatalog
//  Summary
//      Creates a local catalog d2gInstance to access the backend catalog
//		 services 
//      from within the game. 
//  Parameters
//      theInstance : [in] This is pointer created by the d2gCreateInstance.
//      gameId      : [in] Unique game id which is pre-assigned by GameSpy Tech.
//      version\    : [in] A version of the Catalog for the game.
//      region      : [in] A string that specifies the region for the catalog.
//      accessToken : [in] A unique token that allows access to the backend
//		 catalog services.
//  Return Value 
//      D2GCatalogPtr - Pointer to the catalog d2gInstance initialized with
//		 the input parameters.
//  Remarks
//      This API function is mandatory. After this API function is called, all
//		 of the 
//      backend catalog and purchasing services are available to the game. For
//		 the D2G SDK 
//      to terminate properly, d2gCleanupCatalog must be called before calling
//		 d2gCleanup.
//  See Also 
//      d2gCleanupCatalog
//
D2GCatalogPtr d2gCreateCatalog( D2GInstancePtr  theInstance,
	int              gameId,
	gsi_u32          version,
	UCS2String       region,
	UCS2String       accessToken);
//
//  d2gCleanupCatalog
//  Summary
//      De-allocates the memory associated with the catalog d2gInstance.   
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//  Return Value 
//      None.
//  Remarks
//      This API function is mandatory. This must be called before calling
//		 d2gCleanup.
//  See Also 
//      d2gCreateCatalog
//
void d2gCleanupCatalog(D2GInstancePtr theInstance,
	D2GCatalogPtr  theCatalog);



///////////////////////////////////////
/// Callbacks for asynchronous calls //
///////////////////////////////////////

//  D2GGetStoreAvailabilityCallback
//  Summary
//      This callback function is invoked when the d2gGetStoreAvailability
//		 request completes. 
//  Parameters 
//  result\   :  [in]  returned result in response to backend service request. 
//                 If successful GS_SUCCESS otherwise an error code of
//		 GSResult type is returned.
//  response :  [in]  response as defined by D2GGetStoreAvailabilityResponse. 
//  userData :  [in/out]  a void pointer to user-defined data.
// Return Value
//  None.
// Remarks
//  The developer will receive the store availability in the response
//		 parameter. 
//  The store availability status can be one of the following:  
//  D2GResultCode_StoreOnline, D2GResultCode_StoreOfflineForMaintenance, 
//  D2GResultCode_StoreOfflineRetired, D2GResultCode_StoreNotYetLaunched.
// 
typedef void(*D2GGetStoreAvailabilityCallback) (GSResult    result, 
	D2GGetStoreAvailabilityResponse *response, 
	void        *userData);

//  Summary
//      This callback function is invoked when the d2gLoadCatalogItems request
//		 completes. 
//      When a successful result is returned the Catalog Items are ready to be
//		 used in the cache.
//  Parameters
//      result\   :  [in]  returned result in response to backend service
//		 request. 
//                        If successful GS_SUCCESS otherwise an error code of 
//                        GSResult type is returned.
//      response :  [in]  response as defined by D2GLoadCatalogItemsResponse. 
//      userData :  [in/out]  a void pointer to user defined data.
//  Return Value
//      None.
//  Remarks
//  The response contains the D2GCatalogItems list. A successful result means
//		 the D2G cache contains 
//  catalog items retrieved from the backend. In the callback, the developer
//		 can make local copy of 
//  the Catalog Items from the list by calling d2gCloneCatalogItems or can add
//		 to an existing list 
//  by calling d2gAppendCatalogItems. The developer should call the
//		 d2gFreeCatalogItems to free 
//  the items copied from the cache.
//
typedef void(*D2GLoadCatalogItemsCallback) (GSResult result, 
	D2GLoadCatalogItemsResponse *response, 
	void     *userData);

//  Summary 
//      This callback function is invoked when the
//		 d2gLoadCatalogItemsByCategory 
//      request completes.
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. 
//                          If successful GS_SUCCESS otherwise an error code
//		 of GSResult 
//                          type is returned. 
//      response    : [in]  response as defined by
//		 D2GLoadCatalogItemsByCategoryResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//      The response contains the D2GCatalogItems list. A successful result
//		 means 
//      the D2G cache contains the catalog items of a specific category
//		 retrieved 
//      from the backend or they already exist in the cache. In the callback,
//		 the 
//      developer can make local copy of the Catalog Items from the list by
//		 calling 
//      d2gCloneCatalogItems or can add to an existing list by calling
//		 d2gAppendCatalogItems. 
//      The developer should call the d2gFreeCatalogItems to free the items
//		 copied from the cache.
typedef void(*D2GLoadCatalogItemsByCategoryCallback)(GSResult result, 
	D2GLoadCatalogItemsByCategoryResponse *response, 
	void     *userData);
//  Summary
//      This callback function is invoked when the d2gLoadExtraCatalogInfo
//		 request completes.  
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. If successful,
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response as defined by
//		 D2GLoadExtraCatalogInfoResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  The response contains the D2GExtraInfoList. When the result is success,
//		 the D2G cache has been populated 
//  with the ExtraInfo <key, value> pairs. Then,   d2gGetExtraCatalogInfo can
//		 be used to lookup a key from 
//  the D2G cache.
//
typedef void(*D2GLoadExtraCatalogInfoCallback) (GSResult   result, 
	D2GLoadExtraCatalogInfoResponse *response, 
	void       *userData);

//  Summary
//      This callback function is invoked when the d2gGetUserCreditCards
//		 request completes. 
//  Parameters
//      result\   : [in]  returned result in response to backend service
//		 request. 
//                       If successful GS_SUCCESS otherwise an error code of
//		 GSResult type 
//                       is returned.
//      response : [in]  response as defined by D2GGetUserCreditCardsResponse. 
//      userData : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  The response from the service contains the account information for a
//		 purchase. 
//  The developer should keep a copy of the “D2GCreditCardInfo.mAccountIdE as
//		 input to the 
//  d2gGetOrderTotal request function.
//
typedef void(*D2GGetUserCreditCardsCallback) (GSResult  result, 
	D2GGetUserCreditCardsResponse *response, 
	void      *userData);

//  Summary 
//      This callback function is invoked when the d2gGetOrderTotal request
//		 completes. 
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. If successful 
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response as defined by D2GGetOrderTotalResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  This callback response returns a total quote for the items to be purchased. 
//  The developer should keep a copy of the order total  by calling
//		 d2gCloneOrderTotal  
//  since it is required as input to the d2gStartOrder request and free it by
//		 calling 
//  d2gFreeOrderTotal after no longer needed.
//
typedef void(*D2GGetOrderTotalCallback) (GSResult   result, 
	D2GGetOrderTotalResponse *response, 
	void       *userData);

//  Summary
//      The D2G SDK invokes this callback when the d2gStartOrder request
//		 completes.  
//  Parameters
//      result\      : [in]  returned result in response to backend service
//		 request. 
//                          If successful GS_SUCCESS otherwise an error code
//		 of GSResult type is returned.
//      response    : [in]  response as defined by D2GStartOrderResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  This callback response returns the Order Purchase. The  developer should
//		 keep a copy 
//  of the order purchase by calling d2gCloneOrderPurchase since it is
//		 required as input 
//  to the d2gIsOrderComplete request,  and free it by calling
//		 d2gFreeOrderPurchase 
//  after no longer needed.
typedef void(*D2GStartOrderCallback) (GSResult  result, 
	D2GStartOrderResponse *response, 
	void      *userData);

//  Summary   
//      The D2G SDK invokes D2GIsOrderCompleteCallback when the
//		 d2gIsOrderComplete request completes. 
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. If successful, 
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response  as defined by D2GIsOrderCompleteResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//      This callback function indicates whether a purchase has completed.
//		 D2GIsOrderCompleteResponse 
//      contains a list of licenses and the associated download items for the
//		 purchased item(s) 
//      as well as the Order Information.
//
typedef void(*D2GIsOrderCompleteCallback)(GSResult  result, 
	D2GIsOrderCompleteResponse *response, 
	void      *userData);

//  Summary
//      The D2G SDK invokes this callback when the d2gGetPurchaseHistory
//		 request completes.
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. If successful 
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response as defined by
//		 D2GGetPurchaseHistoryResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  This callback response is contains the entire purchase history of the
//		 game. It is simply 
//  a list of purchases, which contain the same information, returned as in
//		 the D2GIsOrderCompleteCallback.
//
typedef void(*D2GGetPurchaseHistoryCallback)(GSResult result, 
	D2GGetPurchaseHistoryResponse  *response, 
	void     *userData);

//
//  Summary 
//      The D2G SDK invokes this callback when the d2gGetItemActivationData
//		 request completes.
//  Parameters
//      result\      : [in]  returned result in response to backend service
//		 request. If successful 
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response as defined by
//		 D2GGetItemActivationResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is part of an upcoming DRM functionality. 
typedef void(*D2GGetItemActivationDataCallback) (GSResult result, 
	D2GGetItemActivationResponse *response, 
	void *userData);
//  Summary
//      The D2G SDK invokes this callback when the d2gCheckContentUpdates
//		 request completes.
//  Parameters 
//      result\      : [in]  returned result in response to backend service
//		 request. If successful 
//                          GS_SUCCESS otherwise an error code of GSResult
//		 type is returned.
//      response    : [in]  response as defined by
//		 D2GCheckContentUpdatesResponse. 
//      userData    : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  The response includes a list of updates, if any, to previously
//		 purchased/downloaded content. The types of
//  available updates are FORCED , MANDATORY and OPTIONAL.
//
typedef void(*D2GCheckContentUpdatesCallback) (GSResult result, 
	D2GCheckContentUpdatesResponse *response, 
	void *userData);

//  Summary 
//      D2G SDK invokes this call function periodically during the file
//		 download to indicate download progress 
//      status during file download after it receives a successful response
//		 for d2gStartDownloadFileById request.
//  Parameters
//      bytesReceived   : [in]  number of bytes received so far.
//      bytesTotal      : [in]  the total size of the file being downloaded.
//      userData        : [in/out]  a void pointer to user defined data.
//  Return Value
//  None.
//  Remarks
//  None.
typedef void(*D2GDownloadProgressCallback)     (gsi_u32 bytesReceived, gsi_u32 bytesTotal, void *userData);

//  Summary
//      This callback is invoked when the download operation completes whether
//		 successfully or due to failure.
//  Parameters 
//      result\      : [in] returned result in response to backend service
//		 request. If successful, 
//                      GS_SUCCESS otherwise an error code of GSResult type is
//		 returned.
//      saveFile    : [in] the name of the file which was downloaded. When
//		 there is an error during download, 
//                      this parameter is set to NULL. 
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value
//  None.
//  Remarks
//  If download completes successfully, EFILENAME>Eis saved in the download
//		 location.
//
typedef void(*D2GDownloadCompleteCallback)     (GSResult result, gsi_char *saveFile, void *userData);

///////////////////////////////////////////////////////////////////////////////////
// Service Request Calls                                                      
//		   //
// These calls are asynchronous to allow for uninterrupted application
//		 execution //
///////////////////////////////////////////////////////////////////////////////////

//d2gGetStoreAvailability
//  Summary
//      This API call retrieves whether a store implementation for 
//      a given catalog is available. 
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gGetStoreAvailability(D2GInstancePtr theInstance,
	D2GCatalogPtr  theCatalog,
	D2GGetStoreAvailabilityCallback callback, 
	void           *userData);

//
//d2gLoadExtraCatalogInfo
//  Summary
//      This API function retrieves a list of ExtraInfo items that is
//		 particular 
//      to the game from the backend services. 
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function is optional if no ExtraInfo exist for a game. If
//		 ExtraInfo 
//      exist, this function should be called during the SDK initialization.
//      Game developers can specify key/value pairs, which we call Extra
//		 Information 
//      for the purposes of modifying the store experience to their users
//		 within 
//      their D2G interface. Specific to a catalog, ExtraInfo data is generic
//		 enough 
//      that the developer can determine where it is best used, e.g. indicating 
//      a custom account management URL or the default item category. 
//      The values rare cached within the D2G SDK and can be retrieved later
//		 one 
//      at a time. In order to add ExtraInfo data to your catalog, 
//      email devsupport@gamespy.com to set up the data on our backend.
//
GSResult d2gLoadExtraCatalogInfo(D2GInstancePtr                  theInstance,
	D2GCatalogPtr                   theCatalog,
	D2GLoadExtraCatalogInfoCallback callback, 
	void                           *userData);

//
//d2gLoadCatalogItems
//  Summary
//      This API call retrieves the items of a catalog from the backend.  
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      When this request is completed successfully the D2G cache is populated 
//      with the items retrieved from the Catalog. 
//         
GSResult d2gLoadCatalogItems(D2GInstancePtr theInstance, 
	D2GCatalogPtr  theCatalog, 
	D2GLoadCatalogItemsCallback callback, 
	void           *userData);

//d2gLoadCatalogItemsByCategory
//  Summary
//      This function retrieves a list of the items which are in a specific 
//      category.
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//
GSResult d2gLoadCatalogItemsByCategory(D2GInstancePtr   theInstance, 
	D2GCatalogPtr    theCatalog, 
	const UCS2String theCategory, 
	D2GLoadCatalogItemsByCategoryCallback callback, 
	void             *userData);

//d2gGetUserCreditCards
//  Summary
//      This API call retrieves the user's credit cards from the backend. 
//      D2G needs the user account information prior to user's purchases. .
//  Parameters
//      theInstance : [in] This pointer is created by the d2gCreateInstance.
//      theCatalog  : [in] This is pointer created by the d2gCreateCatalog.
//      validOnly   : [in] A boolean to indicate if the request is for all or 
//                       only valid credit cards.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out] a void pointer to user-defined  data.
//  Return Value 
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function should be called  before calling d2gGetOrderTotal 
//      since it needs the account information.
//
GSResult d2gGetUserCreditCards(D2GInstancePtr  theInstance,	
	D2GCatalogPtr   theCatalog,
	gsi_bool        validOnly, 
	D2GGetUserCreditCardsCallback callback, 
	void            *userData);

//d2gGetOrderTotal
//  Summary
//      This API function is the first step in the ordering process.  
//      A list of items selected from the Catalog along with their quantities
//		 passed 
//      as input. Then, this function will send an Order Request to backend to
//		 retrieve 
//      the OrderTotal.  
//  Parameters
//      theInstance     : [in] This is the pointer created by the
//		 d2gCreateInstance.
//      theCatalog      : [in] This is the pointer created by the
//		 d2gCreateCatalog.
//      accountId       : [in] accountId is retrieved  with the
//		 d2gGetUserCreditCards 
//                       request from the back-end.
//      cultureCode     : [in] The culture code in which this order will be
//		 processed. 
//      currencyCode    : [in] The currency in which this order will be
//		 processed. 
//      itemIds         : [in] Pointer to the list of itemIds selected from
//		 the catalog. 
//      itemCount       : [in] The number items in the itemIds list.
//      itemQuantities  : [in] The pointer to the corresponding list of item
//		 quantities
//                           for each item in the  itemIds  list.
//      callback        : [in] This is the pointer to developer's callback
//		 function.
//      userData        : [in/out] a void pointer to user-defined  data.
//  Return Value
//      GSResult - Enumeration indicating success (GS_SUCCESS) or a failure
//		 code.
//  Remarks
//      This function call is the first step to make purchase. It simply builds 
//      the shopping cart information for the game. A list of items selected
//		 from 
//      the Catalog along with their quantities passed as input. Then, this
//		 function 
//      will send an Order Request to back-end to retrieve the OrderTotal. 
//
GSResult d2gGetOrderTotal(D2GInstancePtr  theInstance, 
	D2GCatalogPtr   theCatalog,
	gsi_u32         accountId, 
	D2GItemId       *itemIds, 
	gsi_u32         itemCount, 
	gsi_u32         *itemQuantities, 
	D2GGetOrderTotalCallback callback, 
	void            *userData);    

//d2gStartOrder
//  Summary
//      This API call is for starting the purchase process. It needs the
//		 OrderTotal 
//      obtained with the d2gGetOrderTotal request.  
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  : [in] This is the pointer created by the d2gCreateCatalog.
//      orderTotal  : [in] This is received from the backend in the response 
//                       to d2gGetOrderTotal request.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gStartOrder(D2GInstancePtr  theInstance,
	D2GCatalogPtr   theCatalog, 
	D2GOrderTotal   *orderTotal, 
	D2GStartOrderCallback callback, 
	void            *userData);

//d2gIsOrderComplete 
//  Summary
//      This API call is for retrieving the order status from the backend. 
//      The order status is returned by the callback when the request
//		 completes. 
//  Parameters
//      theInstance   : [in] This is the pointer created by the
//		 d2gCreateInstance.
//      theCatalog    : [in] This is the pointer created by the
//		 d2gCreateCatalog.
//      orderPurchase : [in] This is received from the backend in the response 
//                         to the d2gStartOrder request.
//      callback      : [in] This is the pointer to developer's callback
//		 function. 
//      userData      : [in/out] a void pointer to user-defined  data.
//  Return Value    
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function should be called by the game periodically to poll after 
//      d2gStartOrder request completes successfully to retrieve the order
//		 status.
//      The recommended polling is 2 seconds after a successful start of an
//		 order,
//      followed by every 5 seconds until 30 seconds total have passed.  If
//		 the order
//		still hasn't completed, a message to the user should be displayed explaining
//		that the order is still in progress, but is taking longer than
//		 expected to complete.
//		There can be a number of reasons the order may not complete.  One
//		 example is the credit
//		card has expired, or is invalid.  The order validation result can
//		 indicate that.
GSResult d2gIsOrderComplete(D2GInstancePtr   theInstance,
	D2GCatalogPtr    theCatalog, 
	D2GOrderPurchase *orderPurchase, 
	D2GIsOrderCompleteCallback callback, 
	void             *userData);
//d2gGetPurchaseHistory
//  Summary   
//      This API call retrieves the user's purchase history from the backend 
//      service for the purchases made related to the game. 
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  : [in] This is the pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The callback returns the list of items purchased for this game.
//
GSResult d2gGetPurchaseHistory(D2GInstancePtr  theInstance, 
	D2GCatalogPtr   theCatalog,
	D2GGetPurchaseHistoryCallback callback, 
	void            *userData);

//d2gStartDownloadFileById 
//  Summary
//      This function is called to download a purchased file. The download 
//      information is part of the response to d2gGetPurchaseHistory and 
//      d2gIsOrderComplete requests.
//  Parameters
//      theInstance      : [in] This is the pointer created by the
//		 d2gCreateInstance.
//      theCatalog       : [in] This is the pointer created by the
//		 d2gCreateCatalog.
//      fileId           : [in] The file id to be downloaded.
//      downloadLocation : [in] The directory path where the downloaded file 
//                            to be saved. The download directory must exist.
//      progressCallback : [in] This is the pointer to developer's progress
//		 callback.
//      completedCallback: [in] This is the pointer to developer's completed
//		 callback.
//      userData         : [in/out] a void pointer to user-defined  data.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      After the purchase completed, the developer can download the files 
//		 from 
//      the download list retrieved either with d2gIsOrderComplete request or 
//      d2gGetPurchaseHistory request.
//
GSResult d2gStartDownloadFileById(D2GInstancePtr  theInstance, 
	D2GCatalogPtr   theCatalog,
	D2GFileId       fileId, 
	const gsi_char  *downloadLocation, 
	D2GDownloadProgressCallback progressCallback, 
	D2GDownloadCompleteCallback completedCallback, 
	void            *userData);

//
//d2gGetItemActivationData
//  Summary   
//      This function retrieves the licensing information from the backend.
//      if any of the licenses owned by the game expires.
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  : [in] This is the pointer created by the d2gCreateCatalog.
//      callback    : [in] This is the pointer to developer's callback function.
//      thePackageId: [in] This is parameter is an input from the DRM
//		 functionality.
//      userData    : [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is part of an upcoming DRM functionality.
//
GSResult d2gGetItemActivationData(D2GInstancePtr theInstance, 
	D2GCatalogPtr  theCatalog,
	D2GGetItemActivationDataCallback callback, 
	D2GPackageId    thePackageId,
	void * userData);

//d2gCheckContentUpdates
//  Summary
//      This function invokes a service call to the backend to check any
//		 in-game 
//      content updates.  
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  : [in] This is the pointer created by the d2gCreateCatalog.
//      requiredOnly: [in] Is the update for required content only.
//      callback    : [in] This is the pointer to developer's callback function.
//      userData    : [in/out]  a void pointer to user-defined  data.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      None.
//
GSResult d2gCheckContentUpdates(D2GInstancePtr theInstance, 
	D2GCatalogPtr  theCatalog,
	gsi_bool       requiredOnly,
	D2GCheckContentUpdatesCallback callback, 
	void           *userData);
//////////////////////
//Helper Functions  //
//////////////////////

//
// Data access/management helper functions
//

//d2gFreeCatalogItems
//  Summary
//      This function de-allocates the memory for theItemList and assigns NULL 
//      to theItemList. 
//  Parameters
//      theItemList : [in] Pointer to the list.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This is the corresponding API function to d2gCloneCatalogItems 
//      to de-allocate and reset memory.
GSResult d2gFreeCatalogItemList(D2GCatalogItemList *theItemList);

//d2gCloneCatalogItems
//  Summary
//      This function makes a shallow copy of the items from theSrcItemList 
//      to theDstItemList. 
//  Parameters
//      theDstItemList  : [out] Pointer to the destination list.
//                              It is initialized by d2gCloneCatalogItems. 
//      theSrcItemList  : [in]  Pointer to the source list.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      The reason why the shallow copy is used is that the actual catalog
//		 items 
//      are located in the D2G cache. If this function is called, its
//		 corresponding 
//      free function d2gFreeCatalogItems must be called to de-allocate the
//		 memory 
//      containing theDstItemList. See D2GLoadCatalogItemsCallback for when
//		 this is used.
//  See Also
//      d2gFreeCatalogItems
//
GSResult d2gCloneCatalogItemList(D2GCatalogItemList         *theDstItemList, 
	const D2GCatalogItemList   *theSrcItemList);

//d2gAppendCatalogItems
// Brief
//      This function allows the developer append the one list to another. 
//      The srcList is not modified. The dstList contains its original items 
//      as well as the all the items from the srcList.
//  Parameters
//      dstItemList : [out] Pointer to the destination list the source list.  
//      srcItemList : [in]  Pointer to source list given by the caller. 
//                        The contents of this list are not modified.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 	
//  Remarks
//      See D2GLoadCatalogItemsCallback for when this may be used.
//
GSResult d2gAppendCatalogItemList(D2GCatalogItemList       *dstList, 
	D2GCatalogItemList *srcList);

//d2gCloneOrderTotal
//  Summary   
//      This function makes a deep copy of the items from sourceOrderTotal to 
//      destinationOrderTotal. 
//  Parameters
//      destinationOrderTotal : [out] Pointer to the destination order total. 
//      sourceOrderTotal      : [in]  Pointer to source order total. The
//		 contents 
//      of this list are not modified.
//  Return Value
//      GSResult - Enum indicating success (GS_SUCCESS) or a failure code. 
//  Remarks
//      This function is important, since in the D2GGetOrderTotalCallback
//		 function, 
//      the developer needs to retain the order total. The memory for the
//		 destination 
//      list is allocated by D2G. If this function is called, its
//		 corresponding free 
//      function d2gFreeOrderTotal must be called to de-allocate the memory
//		 containing 
//      destinationOrderTotal.
//
GSResult d2gCloneOrderTotal(D2GOrderTotal   **destinationOrderTotal, 
	D2GOrderTotal   *sourceOrderTotal);

//d2gCloneOrderPurchase
//  Summary 
//      This function makes a deep copy of the items from sourceOrderPurchase
//		 to 
//      destOrderPurchase. 
//  Parameters
//      destOrderPurchase : [out] a list of pointers to the destination order
//		 purchases. 
//      sourceOrderTotal  : [in]  Pointer to source order total. 
//                              The contents of this list are not modified.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      This function is important, since in the D2GStartOrderCallback
//		 function, the 
//      developer needs retain the order purchase. The D2G SDK allocates the
//		 memory 
//      space for the destination list. 
//      If this function is called, d2gFreeOrderPurchase must be called to
//		 de-allocate 
//      the memory space.
//  See Also 
//      D2GStartOrderCallback for when this may be used.
//
GSResult d2gCloneOrderPurchase(D2GOrderPurchase **destOrderPurchase, 
	D2GOrderPurchase *sourceOrderPurchase);

// d2gFreeExtraInfoList
//  Summary
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
void d2gFreeExtraInfoList(D2GExtraInfoList *extraInfoList);

// d2gFreeCreditCardInfoList
//  Summary
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
void d2gFreeCreditCardInfoList(D2GCreditCardInfoList *creditCardList);

//d2gFreeOrderTotal
//  Summary
//      This API function de-allocates and resets the memory pointed to by 
//      orderTotal.
//  Parameters
//      orderTotal : [in] The pointer to the order total.
//  Return Value
//      None. 
//  Remarks
//      This is the corresponding API function to d2gCloneOrderTotal.
//
void d2gFreeOrderTotal(D2GOrderTotal *orderTotal);

//d2gFreeOrderPurchase
//  Summary
//      This API function de-allocates and resets the memory pointed to 
//      by orderPurchase.
//  Parameters
//      orderPurchase : [in] Pointer to the memory to be release.
//  Return Value
//     None. 
//  Remarks
//      This is the corresponding  API call  to d2gCloneOrderPurchase .
//
void d2gFreeOrderPurchase(D2GOrderPurchase *orderPurchase);

// d2gFreePuchaseHistory
//  Summary
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
void d2gFreePurchaseHistory(D2GPurchaseHistory *purchaseHistory);

// d2gFreeDownloadItemList
//  Summary
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
void d2gFreeContentUpdateList(D2GContentUpdateList *contentUpdateList);

//
// Sorting 
//

//d2gSortCatalogItemsbyPrice 
//  Summary   
//      This function sorts a list of catalog items onto itself, according to 
//      the price of the items either in ascending or in descending order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; ascending
//		 or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbyPrice(D2GCatalogItemList *list, 
	D2GSortDirection direction );

//d2gSortCatalogItemsbyName
//  Summary
//      This function sorts a list of catalog items onto itself, according to 
//      the item names alphabetically either in ascending or in descending
//		 order. 
//      Direction of the order is specified as an input parameter. 
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; 
//                     ascending or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbyName(D2GCatalogItemList *list, 
	D2GSortDirection direction);

//d2gSortCatalogItemsbyReleaseDate 
//  Summary   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending
//		 order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; ascending
//		 or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbyReleaseDate(D2GCatalogItemList *list, 
	D2GSortDirection direction);

//d2gSortCatalogItemsbyItemId
//  Summary   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending
//		 order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; ascending
//		 or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbyItemId(D2GCatalogItemList *list, 
	D2GSortDirection direction );

//d2gSortCatalogItemsbyExternalId
//  Summary   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending
//		 order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; ascending
//		 or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbyExternalId(D2GCatalogItemList *list, 
	D2GSortDirection direction );

//d2gSortCatalogItemsbySize 
//  Summary   
//      This function sorts a list of catalog items onto itself, according to 
//      the release date of the items either in ascending or in descending
//		 order. 
//      Direction of the order is specified as an input parameter.
//  Parameters
//      list      : [in] pointer to the list of catalog items to be sorted.
//      direction : [in] an enumerated type for the sort direction; ascending
//		 or descending.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//
GSResult d2gSortCatalogItemsbySize(D2GCatalogItemList *list, 
	D2GSortDirection direction );


//
// Catalog Helper functions
//

//d2gGetCategories
//  Summary
//      This is a helper function designed to retrieve the list of categories 
//      from the catalog. This function is under development.                 
//		         
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theCatalog  : [in] This is the pointer created by the d2gCreateCatalog.
//      categoryList: [out]a pointer to the category list for the categories
//		 found. 
//                       D2G SDK allocates the memory for the list. 
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      This helper function helps you render your category based UI if for
//		 example 
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
	D2GCategoryList *categoryList);
// d2gGetExtraCatalogInfo
//  Summary
//      This is a lookup function for a <key, value> pair which is locally 
//      available in the D2G cache.  
//  Parameters
//      theInstance   : [in] This is the pointer created by the
//		 d2gCreateInstance.
//      theCatalog    : [in] This is the pointer created by the
//		 d2gCreateCatalog.
//      extraInfoKey  : [in] The key for the value to be retrieved.
//      extaInfoValue : [out] Pointer to the value in the D2G cache.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      See d2gLoadExtraCatalogInfo, for more information to initialize the
//		 D2G cache 
//      with the list of ExtraInfo items. Since a pointer to the cached value 
//      is returned, the developer should never free extaInfoValue pointer.
//
GSResult d2gGetExtraCatalogInfo(D2GInstancePtr   theInstance,
	D2GCatalogPtr    theCatalog,
	const UCS2String extraInfoKey,
	UCS2String      *extaInfoValue);

/// Extra Item Info Helper functions

// d2gGetExtraItemInfoKeyValueByKeyName
//  Summary
//      Looks up the extra item info value for a given D2GCatalogItem and its 
//		extra info key
//  Parameters
//      theCatalogItem  : [in] pointer to item being checked.
//      aKey	        : [in] Extra Info key for the value to be retrieved.
//      aValue          : [out] Pointer to value that is being looked up.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      The catalog item being passed in must be valid otherwise the lookup
//		 will 
//		not occur.  If the key doesn't exist in the extra info list for the item
//		passed in, extraInfoValue will be NULL and an error will be returned: 
//		D2GResultCode_ExtraInfo_KeyNotFound).  This function will only find
//		 the first 
//		occurrence of the key.  Any additional occurrences will be ignored.
GSResult d2gGetExtraItemInfoKeyValueByKeyName(const D2GCatalogItem *theCatalogItem, 
	const UCS2String extraInfoKey,
	UCS2String *extraInfoValue);

// d2gFilterCatalogItemListByKeyName
//  Summary
//      Creates a subset of a given D2GCatalogItemList based on the 
//		key name passed in.
//  Parameters
//      incomingCatalogItems  : [in] Pointer to D2GCatalogItemList being
//		 scanned.
//		outgoingCatalogItems  : [out] Pointer to Pointer of the
//		 D2GCatalogItemList being returned.
//      extraInfoKey          : [in] Extra Info Key name used for scanning
//		 items.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      The incomingCatalogItems and extraInfoKey being passed in must be
//		 valid otherwise the scan will 
//		not occur.  If the key doesn't exist in the extra info list for the
//		 list of items being
//		passed in, then outgoingCatalogItems is set to NULL to represent an
//		 empty list. Note that 
//		outgoingCatalogItems is allocated internally in this function.  The
//		 outgoingCatalogItems must be 
//		freed using d2gFreeCatalogItemList if this function successfully
//		 finds items in the incomingCatalogItems
//		list.  It should be pretty obvious since outgoingCatalogItems will
//		 *not* be NULL.
GSResult d2gFilterCatalogItemListByKeyName(const D2GCatalogItemList *incomingCatalogItems,
	D2GCatalogItemList **outgoingCatalogItems,
	const UCS2String extraInfoKey);

// d2gFilterCatalogItemListByKeyNameValue
//  Summary
//      Creates a subset of a given D2GCatalogItemList based on the 
//		key name and key value passed in.
//  Parameters
//      incomingCatalogItems  : [in] Pointer to D2GCatalogItemList being
//		 scanned.
//		outgoingCatalogItems  : [out] Pointer to Pointer of the
//		 D2GCatalogItemList being returned.
//      extraInfoKey          : [in] Extra Info Key name used for scanning
//		 items.
//      extraInfoValue        : [in] Extra Info Value name used for scanning
//		 items.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      The incomingCatalogItems, extraInfoKey, extraInfoValue being passed in
//		 must be valid 
//      otherwise the scan will not occur.  If the combination of the input
//		 key and value doesn't 
//		exist in the extra info list for the list of items being passed in, then 
//		outgoingCatalogItems is set to NULL to represent an empty list.  Note that 
//		outgoingCatalogItems is allocated internally in this function.  The
//		 outgoingCatalogItems must 
//		be freed using d2gFreeCatalogItemList if this function successfully
//		 finds items in the 
//		incomingCatalogItems list.  It should be pretty obvious since
//		 outgoingCatalogItems will *not* 
//		be NULL.
GSResult d2gFilterCatalogItemListByKeyNameValue(const D2GCatalogItemList *incomingCatalogItems,
	D2GCatalogItemList **outgoingCatalogItems,
	const UCS2String extraInfoKey,
	const UCS2String extraInfoValue);


/// Manifest Helper Functions


//d2gSetManifestFilePath
//  Summary
//      This function is set the path for the manifest file location. 
//  Parameters
//      manifestFilePath   : [in] String which includes location for the
//		 manifest file.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      This is an optional function. If Patching Features used, the manifest
//		 file path 
//      can be set here. If used, it should be called only once right after 
//      the d2gInitialize. The path should exist. 
//      If function is not called the manifest file is located in 
//      the current working directory by default. Since the manifest file 
//      keeps the installed contents this path should not change once it is set.
//
//
GSResult d2gSetManifestFilePath( D2GInstancePtr     theInstance,
	const char         *manifestFilePath);

//d2gUpdateManifestInstalledContent
//  Summary
//      This function is updates to the manifest information kept by the SDK .
//      Whenever a new content is downloaded and installed, it should be called
//		after installing it.
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theItemId   : [in] Item Id which the content belongs to.
//      theDownload : [in] Download item for the particular content.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      This is a mandatory function and should be call after each successful
//		 installation
//      of a downloaded item. This helper function is used in conjunction with
//		 the Check
//		for updates service call.
//
GSResult d2gUpdateManifestInstalledContent(D2GInstancePtr  theInstance,
	D2GItemId       theItemId, 
	D2GDownloadItem *theDownload);

//d2gUpdateManifestRemovedContent
//  Summary
//      This function is deletes the un-installed content to the manifest 
//      information kept by the SDK .
//      Whenever a new content is deleted, it should be called.
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      theItemId   : [in] Item Id which the content belongs to.
//      theDownload : [in] Download item for the particular content.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      This is a mandatory function and should be call after each
//		 uninstallation
//      of an item.This helper function is used in conjunction with the Check
//		for updates service call.
//
GSResult d2gUpdateManifestRemovedContent(D2GInstancePtr theInstance, 
	D2GItemId      theItemId, 
	D2GDownloadItem *theDownload);

//
//d2gSetServiceTimeout
//  Summary
//      This function is for setting a timeout value for the service calls 
//      for the back-end services. If not set, then the default timeout
//      is used.
//  Parameters
//      theInstance : [in] This is the pointer created by the d2gCreateInstance.
//      timeoutMs   : [in] The timeout value in milliseconds.
//  Return Value
//      GSResult - Integer built with enums indicating success (GS_SUCCESS) or
//		 a failure code. 
//  Remarks
//      If this function used, it should be called during initialization.
//
GSResult d2gSetServiceTimeout(D2GInstancePtr theInstance, gsi_time timeoutMs);

// NOTE: The following functions should only be used for testing purposes!
GSResult d2gSetServiceURL(D2GInstancePtr theInstance, const char * serviceUrl);

void d2gDisplayManifestFileContents(D2GInstancePtr theInstance);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__DIRECT2GAME_H__
