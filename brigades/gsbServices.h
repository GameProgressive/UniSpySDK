///////////////////////////////////////////////////////////////////////////////
// File:	gsbService.h
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __GSBSERVICES_H__
#define __GSBSERVICES_H__

// This file contains the prototypes for all internal service related functions

//Includes
#include "../ghttp/ghttpSoap.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"

#if defined(__cplusplus)
extern "C"
{
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// general header SOAP elements
#define GSB_SERVICE_RESULT_CODE             "StatusCode"
#define GSB_RESULT_SECTION                  "Status"
#define GSB_STATUS_ELEMENT                  "Status"
#define GSB_RESULT_CODE_ELEMENT             "Code"
#define GSB_RESULT_MSG                      "Message"
#define GSB_MESSAGE_ELEMENT					"Message"
#define GSB_COUNT_ELEMENT                   "Count"
#define GSB_BRIGADE_ELEMENT                 "Brigade"
#define GSB_ENTITLEMENT_ELEMENT             "Entitlement"
#define GSB_ENTITLEMENTLIST_ELEMENT         "EntitlementList"
#define GSB_ENTITLEMENTENTRIES_ELEMENT      "EntitlementEntry"
#define GSB_ROLEENTITLEMENT_ELEMENT			"RoleEntitlement"
#define GSB_ROLEENTITLEMENTLIST_ELEMENT		"RoleEntitlementList"
#define GSB_BRIGADEMEMBER_ELEMENT           "BrigadeMember"
#define GSB_BRIGADEMEMBERLIST_ELEMENT       "BrigadeMemberList"
#define GSB_PLAYERLIST_ELEMENT              "PlayerList"
#define GSB_REQUEST_ELEMENT					"Request"
#define GSB_RESPONSE                        "Response"
#define GSB_RESULT                          "Result"
#define GSB_ROLE_ELEMENT                    "Role"
#define GSB_NAME_ELEMENT                    "Name"
#define GSB_ROLEID_ELEMENT                  "RoleID"
#define GSB_ROLELIST_ELEMENT                "RoleList"
#define GSB_FILEID_ELEMENT                  "FileID"
#define GSB_SIZEID_ELEMENT                  "SizeID"
#define GSB_GAMEID_ELEMENT                  "GameID"
#define GSB_DATEADDED_ELEMENT				"DateAdded"

#define GSB_BRIGADEID_ELEMENT               "BrigadeID"
#define GSB_BRIGADENAME_ELEMENT             "BrigadeName"
#define GSB_BRIGADELIST_ELEMENT             "BrigadeList"
#define GSB_BRIGADEMEMBERID_ELEMENT         "BrigadeMemberID"
#define GSB_BRIGADERECRUITINGTYPE_ELEMENT	"BrigadeRecruitingType"

#define GSB_CUSTOMTITLE_ELEMENT             "CustomTitle"
#define GSB_ISLEADER_ELEMENT                "IsLeader"

#define GSB_ENTITLEMENTID_ELEMENT           "EntitlementID"
#define GSB_ENTITLEMENTIDS_ELEMENT			"EntitlementIDs"
#define GSB_ENTITLEMENTNAME_ELEMENT         "EntitlementName"

#define GSB_PROFILEID_ELEMENT               "ProfileID"
#define GSB_TOPROFILEID_ELEMENT			    "ToProfileID"
#define GSB_FROMPROFILEID_ELEMENT			"FromProfileID"
#define GSB_INSTIGATINGPROFILEID            "InstigatingProfileID"
#define GSB_TARGETPROFILEID                 "TargetProfileID"

#define GSB_ISDEFAULT_ELEMENT               "IsDefault"
#define GSB_ISGAMEROLE_ELEMENT              "IsGameRole"
#define GSB_TRUE_TEXT_ELEMENT               "true"
#define GSB_FALSE_TEXT_ELEMENT              "false"

#define GSB_LOGOS_ELEMENT                   "Logos"
#define GSB_BRIGADELOGOENTRIES_ELEMENT      "BrigadeLogoEntries"
#define GSB_BRIGADELOGO_ELEMENT             "BrigadeLogo"
#define GSB_DEFAULTLOGO_ELEMENT             "DefaultLogo"
#define GSB_PATH_ELEMENT                    "Path"
#define GSB_URL_ELEMENT                     "URL"

#define GSB_CREATEDDATE_ELEMENT             "CreatedDate"
#define GSB_DISBANDDATE_ELEMENT             "DisbandDate"
#define GSB_ACTION_ELEMENT					"Action"

#define GSB_BRIGADEHISTORY_ELEMENT          "HistoryList"
#define GSB_HISTORYENTRY_ELEMENT            "HistoryEntry"
#define GSB_HISTORYENTRYID_ELEMENT          "HistoryEntryID"

#define GSB_ENTRYTYPE_ELEMENT               "EntryType"
#define GSB_NOTES_ELEMENT                   "Notes"
#define GSB_SOURCEPROFILENICK_ELEMENT       "SourceProfileNickname"
#define GSB_TARGETPROFILENICK_ELEMENT       "TargetProfileNickname"
#define GSB_REFERENCEID_ELEMENT             "ReferenceID"
#define GSB_STATUSES_ELEMENT				"Statuses"
#define GSB_SEARCHPATTERN_ELEMENT			"SearchPattern"
#define	GSB_DATECREATED_ELEMENT				"DateCreated"
#define GSB_EXTRAPARAMS_ELEMENT				"ExtraParams"
#define GSB_MAXRESULTS_ELEMENT				"MaxResults"
#define GSB_ONBRIGADE_ELEMENT               "OnBrigade"
#define GSB_REFERENCEID_ELEMENT             "ReferenceID"

// Service SOAP elements
#define GSB_UPDATEBRIGADE                   "BrigadeUpdate"
#define GSB_DISBANDBRIGADE                  "BrigadeDisband"
#define GSB_GETBRIGADEBYID                  "BrigadeByIDGet"
#define GSB_SEARCHBRIGADE                   "BrigadeSearch"
#define GSB_SEARCHPLAYER                    "PlayerSearch"
#define GSB_GETBRIGADEHISTORY               "BrigadeHistoryGet"
#define GSB_PERFORMBRIGADEMEMBERACTION      "BrigadeMemberPerformAction"
#define GSB_GETBRIGADEMEMBERLISTBYSTATUS    "BrigadeMemberListByStatusGet"

#define GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS "BrigadeMemberBrigadeListByStatusGet"
#define GSB_GETBRIGADEMEMBERWITHBRIGADELIST		"BrigadeMemberWithBrigadeList"
#define GSB_GETBRIGADEMEMBERWITHBRIGADE			"BrigadeMemberWithBrigade"

#define GSB_UPDATEROLE                      "RoleUpdate"  // This is used to add a custom role "AddRole"
#define GSB_REMOVEROLE                      "RoleRemove"
#define GSB_ADDORUPDATEROLEENTITLEMENT      "RoleEntitlementUpdate"// replaces AddRoleEntitlement ?? Backend has the old name still should be "RoleEntitlementAddUpdate"
#define GSB_REMOVEROLEENTITLEMENT           "RoleEntitlementRemove"
#define GSB_GETENTITLEMENTLIST			    "EntitlementListByGameIDGet"
#define GSB_GETROLEENTITLEMENTLIST			"RoleEntitlementListGet" 
#define GSB_GETROLEENTITLEMENTLISTBYROLEID	"RoleEntitlementListByRoleIDGet" 
#define GSB_GETROLEIDLISTBYENTITLEMENTID	"RoleEntitlementListByEntitlementIDGet" 

#define GSB_GETROLELISTBYBRIGADEID          "RoleListByBrigadeIDGet" // "GetRoleListByBrigadeID"should be changed to GSB_GETROLELISTBYBRIGADEID
#define GSB_GETBRIGADEMEMBERLIST	        "BrigadeMembersByBrigadeIDGet" // 
#define GSB_SAVEBRIGADELOGO                 "BrigadeLogoUpdate"
#define GSB_SENDTEAMMESSAGE                 "BrigadeMessageSend"
#define GSB_SENDMEMBERMESSAGE               "BrigadeMemberMessageSend"
#define GSB_GETBRIGADESBYPROFILEID          "BrigadesByProfileIDGet"
#define GSB_GETBRIGADEHISTORY               "BrigadeHistoryGet"
#define GSB_GETBRIGADEMATCHHISTORY          "BrigadeMatchHistoryGet"
#define GSB_UPDATEMEMBEREMAILANDNICK        "BrigadeMemberEmailAndNickUpdate"

#define GSB_UPDATEBRIGADEMEMBER             "BrigadeMemberUpdate"    
#define GSB_PROMOTETOLEADER                 "BrigadeMemberPromoteLeader" 

#define GSB_ADDROLEENTITLEMENT              "AddRoleEntitlement"     // for next version


// Service SOAP Actions
#define GSB_UPDATEBRIGADE_SOAP                  "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeUpdate\""
#define GSB_DISBANDBRIGADE_SOAP                 "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeDisband\""
#define GSB_GETBRIGADEBYID_SOAP                 "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeByIDGet\""
#define GSB_SEARCHBRIGADE_SOAP                  "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeSearch\""
#define GSB_SEARCHPLAYER_SOAP                   "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/PlayerSearch\""
#define GSB_GETBRIGADEHISTORY_SOAP              "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeHistoryGet\""
#define GSB_PERFORMBRIGADEMEMBERACTION_SOAP     "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberPerformAction\""
#define GSB_GETBRIGADEMEMBERLISTBYSTATUS_SOAP   "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberListByStatusGet\""
#define GSB_GETROLEENTITLEMENTLIST_SOAP			"SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleEntitlementListGet\""
#define GSB_GETROLEENTITLEMENTLISTBYROLEID_SOAP	"SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleEntitlementListByRoleIDGet\""
#define GSB_GETROLEENTITLEMENTLISTBYENTITLEMENTID_SOAP	"SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleEntitlementListByEntitlementIDGet\""
#define GSB_UPDATEROLE_SOAP                     "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleUpdate\""
#define GSB_REMOVEROLE_SOAP                     "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleRemove\""
#define GSB_ADDORUPDATEROLEENTITLEMENT_SOAP     "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleEntitlementUpdate\""
#define GSB_REMOVEROLEENTITLEMENT_SOAP          "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RemoveRoleEntitlement\""
#define GSB_GETENTITLEMENTLIST_SOAP		        "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/EntitlementListByGameIDGet\""
#define GSB_GETROLELIST_SOAP					"SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/RoleListByBrigadeIDGet\""
#define GSB_GETBRIGADEMEMBERLIST_SOAP           "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMembersByBrigadeIDGet\""
#define GSB_SAVEBRIGADELOGO_SOAP                "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeLogoUpdate\""
#define GSB_SENDTEAMMESSAGE_SOAP                "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMessageSend\""
#define GSB_SENDMEMBERMESSAGE_SOAP              "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberMessageSend\""
#define GSB_GETBRIGADESBYPROFILEID_SOAP         "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadesByProfileIDGet\""
#define GSB_GETBRIGADEMATCHHISTORY_SOAP         "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMatchHistoryGet\""
#define GSB_GETBRIGADEMEMBERBRIGADELISTBYSTATUS_SOAP   "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberBrigadeListByStatusGet\""
#define GSB_UPDATEBRIGADEMEMBER_SOAP            "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberUpdate\""
#define GSB_PROMOTETOLEADER_SOAP                "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberPromoteLeader\""
#define GSB_UPDATEMEMBEREMAILANDNICK_SOAP       "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/BrigadeMemberEmailAndNickUpdate\""

#define GSB_ADDROLEENTITLEMENT_SOAP             "SOAPAction: \"http://gamespy.net/brigades/2008/08/BrigadesService/AddRoleEntitlement\"" // ?? OBSOLETE

#define GSB_NAMESPACE                   "gsb"
#define GSB_NAMESPACE_COUNT  1

#define GSB_BRIGADES_SERVICE_URL_FORMAT "BrigadesService.svc"


// Defaults for the SOAP calls



// Private data to help match asynchronous requests with callbacks
typedef struct GSBIRequestData
{
	GSBInternalInstance     *mInstance;

	void            *mUserData; // data from the developer, to be passed through
	union 
	{
		GSBSaveBrigadeCallback               mUpdateBrigadeCallback;
        GSBDisbandBrigadeCallback            mDisbandBrigadeCallback;
		GSBGetBrigadeByIdCallback            mGetBrigadeByIdCallback;
		GSBPerformBrigadeMemberActionCallback mPerformBrigadeMemberActionCallback;
		GSBGetEntitlementListCallback        mGetEntitlementListCallback;
		GSBGetEntitlementIdListByRoleIdCallback    mGetEntitlementIdListByRoleIdCallback;
		GSBGetRoleIdListByEntitlementIdCallback    mGetRoleIdListByEntitlementIdCallback;
		GSBGetBrigadeMemberListCallback      mGetBrigadeMemberListCallback;
        GSBUpdateBrigadeMemberCallback       mUpdateBrigadeMemberCallback;
		GSBUpdateRoleCallback                mUpdateRoleCallback;
		GSBRemoveRoleCallback                mRemoveRoleCallback;
		GSBGetRoleListCallback               mGetRoleListCallback;
        GSBSearchBrigadesCallback            mSearchBrigadesCallback;
        GSBSearchPlayersCallback             mSearchPlayersCallback;
		GSBUploadLogoCompleteCallback        mUploadLogoCompleteCallback;
        GSBSendMessageToBrigadeCallback      mSendMessageToBrigadeCallback;
        GSBSendMessageToMemberCallback       mSendMessageToMemberCallback;
        GSBGetBrigadesByProfileIdCallback    mGetBrigadesByProfileIdCallback;
        GSBGetBrigadeHistoryCallback         mGetBrigadeHistoryCallback;
        GSBGetBrigadeMatchHistoryCallback    mGetBrigadeMatchHistoryCallback;
		GSBGetPendingInvitesAndJoinsCallback mGetPendingInvitesAndJoinsCallback; 
        GSBPromoteToLeaderCallback           mPromoteToLeaderCallback;
        GSBUpdateMemberEmailAndNickCallback  mUpdateEmailAndNickCallback;
	} mUserCallback;
	GSBBrigade *mUpdateBrigade;
} GSBIRequestData;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Service functions
GSResult gsbiServiceUpdateBrigade(GSBInternalInstance *theInstance, GSBBrigade *theBrigade, GSBSaveBrigadeCallback callback, void * userData);
GSResult gsbiServiceDisbandBrigade(GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBDisbandBrigadeCallback callback, void *userData);
GSResult gsbiServiceGetBrigadeById(GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBGetBrigadeByIdCallback callback, void *userData);
GSResult gsbiServiceGetEntitlementList(GSBInternalInstance *theInstance, GSBGetEntitlementListCallback callback, void * userData);
GSResult gsbiServiceGetEntitlementIdListByRoleId (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 roleId, GSBGetEntitlementIdListByRoleIdCallback callback, void *userData);
GSResult gsbiServiceGetRoleIdListByEntitlementId (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 entitlementId, GSBGetRoleIdListByEntitlementIdCallback callback, void *userData);
GSResult gsbiServiceGetBrigadeMemberList (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 statuses, GSBGetBrigadeMemberListCallback callback, void *userData);
GSResult gsbiServiceUpdateBrigadeMember (GSBInternalInstance *theInstance, GSBBrigadeMember *theMember, GSBUpdateBrigadeMemberCallback callback, void *userData);
GSResult gsbiServiceUpdateRole (GSBInternalInstance *theInstance, GSBRole *theRole, GSBEntitlementIdList *entitlementIdList, GSBUpdateRoleCallback callback, void *userData);
GSResult gsbiServiceRemoveRole (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 roleId, GSBRemoveRoleCallback callback, void *userData);
GSResult gsbiServiceGetRoleList(GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBGetRoleListCallback callback, void *userData);
GSResult gsbiServiceInviteToBrigade (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 profileIdToInvite, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceRescindInvite (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 profileIdToCancelInvite, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceAnswerInvite (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_bool acceptInvite, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceJoinBrigade (GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceAnswerJoin (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 playerProfileId, gsi_bool acceptJoin, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceBanMember (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 profileIdToBan, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceLeaveBrigade (GSBInternalInstance *theInstance, gsi_u32 brigadeId, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceRemoveBrigadeMember (GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 profileIdToRemove, GSBPerformBrigadeMemberActionCallback callback, void *userData);
GSResult gsbiServiceSaveBrigadeLogoFileId(GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 fileId, GSBUploadLogoCompleteCallback callback, void * userData);
GSResult gsbiServiceSearchBrigades(GSBInternalInstance *theInstance, GSBSearchFilterList *filterList, GSBSearchBrigadesCallback callback, void *userData);
GSResult gsbiServiceSearchPlayers(GSBInternalInstance *theInstance, GSBSearchFilterList *filterList, GSBSearchPlayersCallback callback, void *userData);
GSResult gsbiServiceSendMessageToBrigade(GSBInternalInstance *theInstance, gsi_u32 brigadeId, UCS2String message, GSBSendMessageToBrigadeCallback callback, void *userData);
GSResult gsbiServiceSendMessageToMember(GSBInternalInstance *theInstance, gsi_u32 toProfileId, UCS2String message, GSBSendMessageToMemberCallback callback, void *userData);
GSResult gsbiServiceGetBrigadesByProfileId(GSBInternalInstance *theInstance, gsi_u32 profileId, GSBGetBrigadesByProfileIdCallback callback, void *userData);
GSResult gsbiServiceGetBrigadeHistory( GSBInternalInstance *theInstance,gsi_u32 brigadeId, gsi_u32 profileId, GSBBrigadeHistoryAccessLevel historyAccessLevel, GSBGetBrigadeHistoryCallback callback, void *userData);
GSResult gsbiServiceGetBrigadeMatchHistory( GSBInternalInstance *theInstance, gsi_u32 matchId, GSBGetBrigadeMatchHistoryCallback callback, void *userData);
GSResult gsbiServiceGetPendingInvitesAndJoins( GSBInternalInstance *theInstance, GSBGetPendingInvitesAndJoinsCallback callback, void *userData);
GSResult gsbiServicePromoteToLeader(GSBInternalInstance *theInstance, gsi_u32 brigadeId, gsi_u32 memberId, GSBPromoteToLeaderCallback callback, void *userData);
GSResult gsbiServiceUpdateMemberEmailAndNick(GSBInternalInstance *theInstance,  gsi_u32 brigadeId, GSBUpdateMemberEmailAndNickCallback callback, void *userData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__GSBSERVICES_H__
