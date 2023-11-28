/**
* d2gDeserialize.h
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/
#ifndef __D2GDESERIALIZE_H__
#define __D2GDESERIALIZE_H__

#include "../ghttp/ghttpSoap.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"
#include "d2gUtil.h"

///////////////////////////////////////////////////////////////////////////////
// Common
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetResponse(GSXmlStreamReader theResponseXml , 
                              const char        *theResponseTag);

GSResult d2giParseResponseHeader(GSXmlStreamReader theResponseXml);

///////////////////////////////////////////////////////////////////////////////
//  Store Availability 
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseGetStoreAvailResponse(D2GIInstance * instance,
                                        GSXmlStreamReader theResponseXml,
                                        D2GGetStoreAvailabilityResponse *response);
                                        
///////////////////////////////////////////////////////////////////////////////
// Extra Info
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseExtraInfo(D2GIInstance        *theInstance, 
                                        D2GICatalog         *theCatalog, 
                                        GSXmlStreamReader   theResponseXml,  
                                        D2GExtraInfo        *theExtraInfo);

GSResult d2giParseExtraInfoList(D2GIInstance     *theInstance, 
								D2GICatalog      *theCatalog, 
								GSXmlStreamReader theResponseXml, 
								D2GExtraInfoList *response);

GSResult d2giParseLoadExtraCatalogInfoResponse(D2GIInstance                    *theInstance,
											   D2GICatalog                     *theCatalog, 
											   GSXmlStreamReader                theResponseXml, 
											   D2GLoadExtraCatalogInfoResponse *response);

GSResult d2giParseGetItemActivationResponse(D2GIInstance                 *theInstance,
                                            D2GICatalog                  *theCatalog,  
                                            GSXmlStreamReader             theResponseXml, 
                                            D2GGetItemActivationResponse *response);
///////////////////////////////////////////////////////////////////////////////
// Parsing Catalog Items                                                      
///////////////////////////////////////////////////////////////////////////////

GSResult d2giParseGeoInfoFromResponse(D2GIInstance *theInstance, 
                                      D2GGeoInfo    *pGeoInfo,  
                                      GSXmlStreamReader theResponse);

GSResult d2giParseItemFromResponse(D2GIInstance *theInstance, 
                                   D2GICatalog  *theCatalog,
                                   D2GCatalogItem **item, 
                                   GSXmlStreamReader theResponse);


GSResult d2giParseItemsFromResponse(D2GIInstance      *theInstance, 
                                    D2GICatalog       *theCatalog,
                                    GSXmlStreamReader theResponseXml, 
                                    D2GCatalogItemList   *outItemList);

GSResult d2giParseLoadCatalogItemsFromResponse(D2GIInstance      *theInstance, 
                                               D2GICatalog       *theCatalog,
                                               GSXmlStreamReader theResponseXml, 
                                               D2GLoadCatalogItemsResponse *getAllItemsResponse);

GSResult d2giParseGetItemsByCategoryFromResponse(D2GIInstance      *theInstance,
                                                 D2GICatalog       *theCatalog, 
                                                 GSXmlStreamReader theResponseXml, 
                                                 D2GLoadCatalogItemsByCategoryResponse *response);

///////////////////////////////////////////////////////////////////////////////
// Credit Cards                                            
///////////////////////////////////////////////////////////////////////////////
GSResult d2giParseCreditCardInfoList(D2GIInstance      *instance, 
                                     GSXmlStreamReader theResponseXml, 
                                     D2GGetUserCreditCardsResponse *responseOut);

///////////////////////////////////////////////////////////////////////////////
// Parsing Orders and Purchases                                       
///////////////////////////////////////////////////////////////////////////////

GSResult d2giParseOrderItemFromResponse(D2GIInstance      *theInstance, 
                                        D2GICatalog       *theCatalog,
                                        D2GOrderItem      *theOrderItem, 
                                        GSXmlStreamWriter theResponseXml);

GSResult d2giParseOrderItemPurchaseFromResponse(D2GIInstance         *theInstance, 
                                                D2GICatalog          *theCatalog,
                                                D2GOrderItemPurchase *theOrderItem, 
                                                GSXmlStreamWriter	 theResponseXml,
												gsi_bool			 updateManifest);
                                                
GSResult d2giParseOrderValidationFromResponse(GSXmlStreamReader   theResponseXml, 
                                              D2GOrderValidation *theValidation);

GSResult d2giParseOrderFromResponse(GSXmlStreamWriter theResponseXml, 
                                    D2GOrderInfo      *order);

GSResult d2giParseOrderTotalFromResponse(D2GIInstance  *theInstance,
                                         D2GICatalog   *theCatalog,
                                         GSXmlStreamWriter theResponseXml,
                                         D2GOrderTotal *orderTotal);

GSResult d2giParseOrderPurchaseFromResponse(D2GIInstance      *theInstance,
                                            D2GICatalog       *theCatalog,
                                            GSXmlStreamWriter theResponseXml,
                                            D2GOrderPurchase  *orderPurchase,
											gsi_bool		  updateManifest);

GSResult d2giParseGetOrderTotalResponse(D2GIInstance      *theInstance,
                                        D2GICatalog       *theCatalog, 
                                        GSXmlStreamWriter theResponseXml, 
                                        D2GGetOrderTotalResponse *theResponse);

GSResult d2giParseStartOrderResponse(D2GIInstance *theInstance,
                                     D2GICatalog  *theCatalog, 
                                     GSXmlStreamReader theResponseXml, 
                                     D2GStartOrderResponse *response);


GSResult d2giParseIsOrderCompleteResponse( D2GIInstance *theInstance,
                                           D2GICatalog   *theCatalog, 
                                           GSXmlStreamReader theResponseXml, 
                                           D2GIsOrderCompleteResponse *response);
                                           
GSResult d2giParseGetPurchaseHistoryResponse(D2GIInstance        *theInstance,
                                             D2GICatalog         *theCatalog, 
                                             GSXmlStreamReader   theResponseXml, 
                                             D2GGetPurchaseHistoryResponse *response);
                                             
///////////////////////////////////////////////////////////////////////////////
// Download Info  
///////////////////////////////////////////////////////////////////////////////

GSResult d2giParseDownloadHeaderFromResponse(GSXmlStreamReader   theResponseXml,  
                                             gsi_char            *userHeader);

GSResult d2giParseDownloadUserHeadersFromResponse(GSXmlStreamReader   theResponseXml, 
                                                  gsi_char            **userHeader);
                  
GSResult d2giParseGetItemActivationResponse(D2GIInstance        *theInstance,
                                            D2GICatalog         *theCatalog,  
                                            GSXmlStreamReader   theResponseXml, 
                                            D2GGetItemActivationResponse *response);
                                            
GSResult d2giParseCheckContentUpdates(D2GIInstance        *theInstance,
                                      D2GICatalog         *theCatalog,  
                                      GSXmlStreamReader   theResponseXml, 
                                      D2GCheckContentUpdatesResponse *response);
                                      
GSResult d2giParseManifestRecord(char              *buffer,
                                 D2GContentUpdate *theContentUpdate);

GSResult d2giParseAndDecodeManifestRecord(char *manifestRead,
                                          D2GContentUpdate *contentUpdate);

GSResult d2giParseDownload(GSXmlStreamReader    *reader,
                           D2GContentUpdate    *contentUpdate);

#endif
