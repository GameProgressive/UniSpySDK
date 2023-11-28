/**
* d2gUtil.h
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/
#ifndef __D2GUTIL_H__
#define __D2GUTIL_H__
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// URL for direct2game commerce services (backwards compatibility)
#define GS_D2G_HTTP GSI_HTTP_PROTOCOL_URL
#define GS_D2G_SECURE_HTTP GSI_HTTP_PROTOCOL_URL

#ifdef GS_D2G_STAGE
#define	GS_D2G_SERVICE_URL_BASE "mwstage." GSI_DOMAIN_NAME "/commerce/1.1/"
#else
#define GS_D2G_SERVICE_URL_BASE "%s.d2g.pubsvs." GSI_DOMAIN_NAME "/commerce/1.1/"
#endif

//passed into d2giSetServiceUrl
#define GS_D2G_URL_SECURE     gsi_true
#define GS_D2G_URL_NOT_SECURE gsi_false

#define GS_D2G_MAX_BASE_URL_LEN		  128
#define GS_D2G_MAX_URL_LEN		      256

//Manifest File Name
#define  GS_D2G_MANIFEST_FILENAME       "InstalledContent"

// Maximum number of categories supported
#define GS_D2G_MAX_CATEGORIES         1000
#define GS_D2G_MAX_FILE_BUFFER_LEN	  500
typedef struct D2GIString
{
    int     len;
    char    *str;
} D2GIString;

typedef  gsi_bool (* compareFunc)(const void *, const void *, D2GSortDirection direction) ;
void d2giQSort(void         *list,
			   size_t       size,
			   unsigned int offset,
			   D2GSortDirection  direction,
			   int          left, 
			   int          right, 
			   compareFunc	compareFunction);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void d2giSetServiceUrl(D2GIInstance *theInstance, gsi_bool secure, const char *service);
GSResult d2giResultFromStatusCode(int statusCode);
GSResult d2giResultFromHttpResult(GHTTPResult result);
GSResult d2giResultFromHttpRequest(GHTTPRequest request);
GSResult d2giOrderItemValidationResultFromCode(int orderItemCode);
GSResult d2giOrderValidationResultFromCode(int orderCode);
GSResult d2giResultFromStoreAvailabilityCode(int availableCode);
GSResult d2giResultFromDownloadStatusCode(int statusCode);
			
gsi_bool d2giWriteCachedItemsToXml(D2GIInstance *theInstance, 
                                   D2GICatalog  *theCatalog, 
                                   GSXmlStreamWriter writer, 
                                   gsi_u32      accountId, 
								   UCS2String   cultureCode, 
								   UCS2String   currencyCode, 
								   gsi_u32      *itemIds, 
								   int          itemCount, 
								   gsi_u32      *itemQuantities);
								   
gsi_bool d2giWriteOrderTotalToXml(D2GIInstance      *theInstance, 
                                  GSXmlStreamWriter writer, 
                                  const D2GOrderTotal *theOrderTotal);

// Item functionality
GSResult d2giResetCatalogItem(D2GCatalogItem *item);

D2GCatalogItem * d2giGetCatalogItem(D2GIInstance *theinstance, 
                                    D2GICatalog  *theCatalog,
                                    gsi_u32      itemId);

D2GCatalogItem * d2giNewCatalogItem(D2GIInstance *theInstance, 
                                    D2GICatalog  *theCatalog,
                                    gsi_u32      itemId);

///////////////////////////////////////////////////////////////////////////////
//          Extra Info Cache Helper Functions 
///////////////////////////////////////////////////////////////////////////////

D2GExtraInfo * d2giNewExtraInfo(D2GIInstance *theInstance, 
                                D2GICatalog  *theCatalog,
                                D2GExtraInfo *theExtraInfo);
                                
UCS2String d2giLookUpExtraInfo(D2GIInstance *theInstance, D2GICatalog *theCatalog, const UCS2String key);
                                
D2GExtraInfo * d2giGetExtraInfo(D2GIInstance *theInstance, 
                                D2GICatalog  *theCatalog,
                                UCS2String   key);


GSResult  d2giFreeExtraInfo(D2GICatalog  *theCatalog, 
                            D2GExtraInfo *extraInfo);

GSResult d2giResetExtraInfo(D2GExtraInfo *extraInfo);

GSResult d2giFreeExtraInfoList(D2GICatalog  *theCatalog,
                               D2GExtraInfoList *extraInfoList, 
                               GSResult     result, 
                               gsi_bool     freeItem);
//cleanup functions
void d2giFreeCreditCardInfoList(D2GCreditCardInfoList *creditCardList);

void d2giFreeOrder(D2GOrderInfo *theOrder);

GSResult d2giFreeCatalogItems(D2GICatalog  *theCatalog,
                              D2GCatalogItemList *itemList, 
                              GSResult     result, 
                              gsi_bool     freeItem);
                              
GSResult  d2giFreeCatalogItem(D2GICatalog  *theCatalog,
                              D2GCatalogItem * item);

void d2giFreeOrderTotal(D2GOrderTotal *orderTotal);

void d2giFreeOrderPurchase(D2GOrderPurchase *purchase);

void d2giFreePurchaseHistory(D2GPurchaseHistory *purchaseHistory);

void d2giFreeOrderPurchaseContents(D2GOrderPurchase *purchase);

void d2giFreeDownloadItemContents(D2GDownloadItem * downloadItem);

void d2giFreeContentUpdateList(D2GContentUpdateList *contentUpdateList);

// Sorting
GSResult d2giSortCatalogItemsbyPrice(D2GCatalogItemList  *list, 
                                     D2GSortDirection direction);

GSResult d2giSortCatalogItemsbyName(D2GCatalogItemList  *list, 
                                     D2GSortDirection direction);
                                     
GSResult d2giSortCatalogItemsbyDate(D2GCatalogItemList  *list, 
                                    D2GSortDirection direction);

GSResult d2giSortCatalogItemsbyID(D2GCatalogItemList  *list, 
								  D2GSortDirection direction);

GSResult d2giSortCatalogItemsbyExtId(D2GCatalogItemList  *list, 
									 D2GSortDirection direction);

GSResult d2giSortCatalogItemsbySize(D2GCatalogItemList  *list, 
									D2GSortDirection direction);
void d2giQSortInt(void         *list,
				  size_t       size,
				  unsigned int offset,
				  D2GSortDirection  direction,
				  int          left, 
				  int          right );

void d2giQSortStr(D2GCatalogItemList   *list, 
                  D2GSortDirection  direction,
                  int               left, 
                  int               right );                 

GSResult d2giGetAllItemsFromCache(D2GICatalog   *theCatalog, 
                                  D2GLoadCatalogItemsCallback callback, 
                                  void          *userData);
                                  
GSResult d2giGetCategoriesFromCache(D2GICatalog *theCatalog, D2GCategoryList *categoryList);
                                    
//////////////////////////////////////////////////////////////////////////
//Manifest File operations
//////////////////////////////////////////////////////////////////////////
typedef struct D2GIManifestRecord
{
    D2GItemId   itemId;
    D2GUrlId    urlId;
    float       version;
} D2GIManifestRecord;

GSResult d2giDeleteManifestRecord(D2GIInstance       *theInstance,
                                  D2GIManifestRecord manifest);

GSResult d2giUpdateManifestRecord(D2GIInstance       *theInstance,
                                  D2GIManifestRecord manifest);
                                  
gsi_bool d2giWriteManifestDownloadsToXML( GSXmlStreamWriter writer, 
                                          D2GIInstance *theInstance); 

GSResult d2giCreateManifestXmlStr (D2GIManifestRecord manifest, 
                                   D2GIString         *manifestBuffer);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GSResult d2giCheckItemsQuantities(D2GIInstance *theInstance, 
                                  gsi_u32      *itemQuantities, 
                                  gsi_u32      itemQuantitiesCount);

GSResult d2giCloneOrderItem(D2GOrderItem *dstOrderItem, 
                            D2GOrderItem *srcOrderItem);

GSResult d2giCloneDownloadList(D2GDownloadItemList *dstDownloadItemList, 
                               D2GDownloadItemList *srcDownloadItemList);

GSResult d2giCloneLicenseList(D2GLicenseItemList *dstLicenses, 
                              D2GLicenseItemList *srcLicenses);

GSResult d2giCloneOrderItemPurchases(D2GOrderItemPurchase **destPurchases, 
                                     gsi_u32              *dstCount, 
                                     D2GOrderItemPurchase *srcPurchases, 
                                     gsi_u32              srcCount);

GSResult d2giGetCatalogItemsByCategoryFromCache(D2GICatalog    *theCatalog, 
                                         const UCS2String theCategory, 
                                         D2GLoadCatalogItemsByCategoryCallback callback, 
                                         void           *userData);
#endif
