///////////////////////////////////////////////////////////////////////////////
// File:	gsbUtil.h
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __GSB_UTIL_H__
#define __GSB_UTIL_H__

// This define is kept for compatibility purpouse.
#define GSB_HTTP             GSI_HTTP_PROTOCOL_URL

#if defined(GSI_OPEN_DOMAIN_NAME)
	#define GSB_SERVICE_URL_BASE "" GSI_OPEN_DOMAIN_NAME "/Brigades/"
#else
	#define GSB_SERVICE_URL_BASE "" GSI_DOMAIN_NAME "/Brigades/"	
#endif

#define GSB_SERVICE_CONTRACT "BrigadesService.svc"

// Temporary!
#define GSB_UPLOAD_GID       2603
#define GSB_UPLOAD_RESULT_HEADER "Sake-File-Result:"
#define GSB_UPLOAD_FILE_ID_HEADER "Sake-File-Id:"

// Maximums 
#define GSB_SEARCH_EXTRAPARAMS_MAX  6
#define GSB_SEARCH_MAX_RESULTS      100
#define GSB_SEND_MESSAGE_MAX_LEN    2048

typedef struct GSBIUploadData
{
	gsi_char *mLogoFileName;
	gsi_u32 mBrigadeId;
	gsi_u32 mFileId;
	GSBInternalInstance *mInstance;
	GSBUploadLogoCompleteCallback mCallback;
	void *mUserData;
} GSBIUploadData;

typedef struct GSBIDownloadData
{
	GSBInternalInstance *mInstance;
	GSBBrigadeLogo *mBrigadeLogo;
	gsi_char *mLogoFileName;
	GSBDownloadLogoCompleteCallback mCallback;
	void *mUserData;
} GSBIDownloadData;

typedef enum
{
	GSBIUploadResult_SUCCESS           = 0,
	GSBIUploadResult_BAD_HTTP_METHOD   = 1,
	GSBIUploadResult_BAD_FILE_COUNT    = 2,
	GSBIUploadResult_MISSING_PARAMETER = 3,
	GSBIUploadResult_FILE_NOT_FOUND    = 4,
	GSBIUploadResult_FILE_TOO_LARGE    = 5,
	GSBIUploadResult_SERVER_ERROR      = 6,
	GSBIUploadResult_UNKNOWN_ERROR
} GSBIUploadResult;

GSResult gsbiStatusCodeToServiceResult(int statusCode);
GSResult gsbiSetServiceUrl(GSBInternalInstance *theInstance);
GSResult gsbiValidateBrigadeSearchFilter(GSBSearchFilterList *filter);
GSResult gsbiValidateMemberSearchFilter(GSBSearchFilterList *filter);
GSResult gsbiStartUploadRequest(GSBInternalInstance *theInstance, 
								const gsi_char *logoFileName, 
								gsi_u32 brigadeId, 
								GSBUploadLogoCompleteCallback callback, 
								void *userData);
GSResult gsbiStartDownloadRequest(GSBInternalInstance *theInstance, 
								  const gsi_char *logoFileName, 
								  GSBBrigadeLogo *logo, 
								  GSBDownloadLogoCompleteCallback callback, 
								  void *userData);
GSResult gsbiValidateBrigadeSearchFilter(GSBSearchFilterList *filter);

GSResult gsbiValidateMemberSearchFilter(GSBSearchFilterList *filter);

GSResult gsbiCloneBrigadeLogo(GSBBrigadeLogo **destLogo, 
							  GSBBrigadeLogo *srcLogo);

GSResult gsbiCloneBrigadeLogoList(GSBBrigadeLogoList *destLogoList, 
                                  GSBBrigadeLogoList *srcLogoList);
void gsbiFreeBrigadeLogoList(GSBBrigadeLogoList *logoList);

GSResult gsbiCloneBrigadeMemberContents(GSBBrigadeMember *destMember, GSBBrigadeMember *srcMember);

GSResult gsbiCloneEntitlement(GSBEntitlement *destEntitlement, 
                              GSBEntitlement *srcEntitlement);

void gsbiFreeEntitlementList(GSBEntitlementList *entitlementList);

void gsbiFreeEntitlementIdList(GSBEntitlementIdList *entitlementIdList);

void gsbiFreeBrigadeHistoryEntryContents(GSBBrigadeHistoryEntry *historyEntry);

void gsbiFreeBrigadeHistoryList(GSBBrigadeHistoryList *historyList);

void gsbiFreeBrigadeContents(GSBBrigade *brigade);

void gsbiFreeBrigadeList(GSBBrigadeList *brigadeList);

void gsbiFreeRole(GSBRole *role);

void gsbiFreeRoleList(GSBRoleList *roleList);

void gsbiFreeRoleIdList(GSBRoleIdList *roleIdList);

void gsbiFreePendingActionsList(GSBBrigadePendingActionsList *actionList);

void gsbiFreeBrigadeMemberContents(GSBBrigadeMember *member);

void gsbiFreeBrigadeMember(GSBBrigadeMember *member);

void gsbiFreeBrigadeMemberList(GSBBrigadeMemberList *memberList);

void gsbiFreeFilterList(GSBSearchFilterList *filterList);

#endif
