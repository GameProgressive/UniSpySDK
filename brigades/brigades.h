///////////////////////////////////////////////////////////////////////////////
// File:	brigades.h
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.


#ifndef __BRIGADES_H__
#define __BRIGADES_H__

// This is the main header file for the GameSpy Brigades SDK.
// All structures and SDK API functions are defined in this file.

// Includes
#include "../ghttp/ghttpSoap.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"
#include "../common/gsResultCodes.h"
#include "../webservices/AuthService.h"
 
#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
// Public SDK Interface
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  Enums  
///////////////////////////////////////////////////////////////////////////////


// GSBResultCode
// Summary
//		These values will be part of a GSResult that will be in a GSResultSet
//		that will be in all callbacks
// Remarks
//		These are SDK Specific result codes the can be retrieved using:
//		GS_RESULT_CODE(result)
//      Please consult gsResultCodes.h file for more information on GSResult and 
//		its helper macros.
typedef enum GSBResultCode
{
	//Brigade
	GSBResultCode_BrigadeNameAlreadyExists = 4,
	GSBResultCode_BrigadeTagAlreadyExists,
	GSBResultCode_NotFound,
	GSBResultCode_BrigadeNameCantBeChanged,
	GSBResultCode_BrigadeGameIDCantBeChanged,
	GSBResultCode_BrigadeIsInviteOnly,
	GSBResultCode_BrigadeMemberHasNotRequestedJoin,
	GSBResultCode_BrigadeMemberIDNotFound,
	GSBResultCode_BrigadeMaxMembersReached,
	GSBResultCode_MaxBrigadeMembershipsReached,
	GSBResultCode_RequestorNotInSameBrigade,
	GSBResultCode_BrigadeMemberAlreadyRemoved,
	GSBResultCode_RequestorNotActive,
	GSBResultCode_LeaderCannotBeRemoved,
	GSBResultCode_GameAlreadyRegistered,
	GSBResultCode_EntitlementIsConfigured,
	GSBResultCode_ProfileNotLeader,
	GSBResultCode_RoleIsGameRole,
	GSBResultCode_NoPermisionForAction,

	GSBResultCode_BadWordClanName,
	GSBResultCode_BadWordClanTag,
	GSBResultCode_BadWordClanURL,
	GSBResultCode_BadWordClanMotD,
	GSBResultCode_BadWordClanRole,
	GSBResultCode_BadWordClanTitle,

	GSBResultCode_EnumNotValid,
	GSBResultCode_MatchIDNotFound,

	// Search Criteria
	GSBResultCode_SearchCriteriaNotMet,
    GSBResultCode_SearchCriteriaUnexpected,
    GSBResultCode_SearchCriteriaWrongDataType,

	// response codes dealing with errors in response headers
	GSBResultCode_INVALID_GAMEID,			// make sure GameID is properly set with wsSetCredentials
	GSBResultCode_INVALID_SESSIONTOKEN,     // make sure wsSetCredentials was called with valid credentials and you have logged in via AuthService
	GSBResultCode_SESSIONTOKEN_EXPIRED,      // re-login via AuthService to refresh your 'session'

	GSBResultCode_UNKNOWN_ERROR,

	GSBResultCode_Max
} GSBResultCode;


// GSBRecruitingType
// Summary
//		Represents how a brigade is recruiting
// Remarks
//		Can be used for searching brigades followed by 
//		joining if GSBRecruitingStatus_OPEN.  
typedef enum GSBRecruitingType
{
    GSBRecruitingStatus_Open,			  // Open Brigade, all join requests are auto-accepted
    GSBRecruitingStatus_Moderated,        // Anyone may request membership, leaders must approve
    GSBRecruitingStatus_InviteOnly,       // Leaders can send invites, but members cannot request to join
    GSBRecruitingStatus_NoNewMembers,     // Closed down/quit and are no longer operating
    GSBRecruitingStatus_Max               // Number of recruiting types.
} GSBRecruitingType;

// GSBSearchFilterKey
// Summary
//		Used to identify the type of search filter.
// Remarks
//		The key can only be used once in a GSBSearchFilter.  Otherwise
//		the result code GSBResultCode_SearchCriteriaNotMet will be returned 
//		by the search functions gsbSearchBrigades() and gsbSearchPlayers().
typedef enum GSBSearchFilterKey
{
    GSBSearchFilterKey_Name,            // Brigade Name or Player Name
    GSBSearchFilterKey_Tag,             // Brigade Tag or All Team Tags which the Member Belongs to 
    GSBSearchFilterKey_BrigadeId,       // Search by brigade id
    GSBSearchFilterKey_BrigadeStatus,   // Brigade Recruiting  Status (Open/Closed)
    GSBSearchFilterKey_DateCreated,     // Brigade Created Date
    GSBSearchFilterKey_ExtraParams,     // a generic search string such as "games>100"
    GSBSearchFilterKey_MaxResultCount,  // Maximum number of results which should be returned.
    GSBSearchFilterKey_Max		      // Number of filter key types.
} GSBSearchFilterKey;

// GSBSearchFilterValueType
// Summary
//		Used to identify the data type of search value 
//		passed in to a search function.
// Remarks
//		The value types identify the data type passed in the mValue field.
typedef enum GSBSearchFilterValueType
{
    GSBSearchFilterValueType_Unicode,       // Unicode string
    GSBSearchFilterValueType_Uint,           // Unsigned int32
    GSBSearchFilterValueType_Time           // time_t
} GSBSearchFilterValueType;

// GSBBrigadeMemberStatus
// Summary
//		A member's status on the team.
// Remarks
//		These represent pending actions that a 
//		Brigade leader can take.  Others may just indicate 
//		a member performing an action on itself
//      These values should be used in a bitwise fashion for 
//      getting brigade members
typedef enum GSBBrigadeMemberStatus
{
	GSBBrigadeMemberStatus_Active       = 1,	// Member is active on this brigade
    GSBBrigadeMemberStatus_Inactive	    = 2,	// Member has made themselves inactive in the brigade
	GSBBrigadeMemberStatus_Invited		= 4,	// Member is currently invited to the brigade
	GSBBrigadeMemberStatus_RequestJoin  = 8,	// Member has requested to join the brigade
	GSBBrigadeMemberStatus_Leader		= 0x10,	// Member in the brigade leader
	GSBBrigadeMemberStatus_Kicked		= 0x20,	// Member has been kicked off the brigade
	GSBBrigadeMemberStatus_Blocked		= 0x40,	// Player has been blocked from joining the brigade
	GSBBrigadeMemberStatus_Max			= 0x80  // Set to next higher bit
} GSBBrigadeMemberStatus;

// GSBBrigadeMemberAction
// Summary
//		Internal actions that can be applied to a brigade member.
// Remarks
//		These represent internal actions used by the SDK functions.
//		Players can request to join, accept/decline invitations and leave a brigade.
//		Entitled members can ban, remove and accept/decline join requests
typedef enum GSBBrigadeMemberAction
{
	GSBBrigadeMemberAction_RequestJoin,	// Request to join

	GSBBrigadeMemberAction_Invite,			// Invites player
	GSBBrigadeMemberAction_InviteAccept,	// Invitation accepted
	GSBBrigadeMemberAction_InviteDecline,	// Invitation declined

	GSBBrigadeMemberAction_RequestAccept,	// Request to join accepted by entitled member
	GSBBrigadeMemberAction_RequestDecline,	// Request to join declined by entitled member

	GSBBrigadeMemberAction_LeaveBrigade,	// Member leaves brigade OR player cancels request to join
	GSBBrigadeMemberAction_RemoveMember,	// Leader or entitled member removes member
	GSBBrigadeMemberAction_BanMember,		// Leader or entitled member bans player
	GSBBrigadeMemberAction_RescindInvite	// Cancel an invitation

} GSBBrigadeMemberAction;

// GSBBrigadeHistoryAccessLevel
// Summary
//		Used for determining the level of access when retrieving brigade history.
// Remarks
//		History information can be retrieved with levels of access.
//		These values will tell the backend to send information up to the
//      level specified.
typedef enum GSBBrigadeHistoryAccessLevel
{
   GSBBrigadeHistoryAccessLevel_None   = 0,
   GSBBrigadeHistoryAccessLevel_Admin  = 1,
   GSBBrigadeHistoryAccessLevel_Leader = 2,
   GSBBrigadeHistoryAccessLevel_Member = 4,
   GSBBrigadeHistoryAccessLevel_Public = 8,
   GSBBrigadeHistoryAccessLevel_Match  = 16,
   GSBBrigadeHistoryAccessLevel_All    = 0x1F

}GSBBrigadeHistoryAccessLevel;


///////////////////////////////////////////////////////////////////////////////
//  Type Definitions  
///////////////////////////////////////////////////////////////////////////////

// GSBInstancePtr
// Summary
//		Pointer to the Brigade instance.
// Remarks
//		It is used by almost every API function. It should be maintained by 
//		the game until the Brigades SDK cleanup/shutdown is called.
typedef void* GSBInstancePtr;


// GSBEntitlement
// Summary
//		Structure that holds an entitlement.
// Remarks
//		An entitlement defines an action that a brigade member can perform.
//		Use gsbGetEntitlementList to get a list of available entitlements.
typedef struct GSBEntitlement
{
	gsi_u32	mEntitlementId;			// Entitlement ID 
	UCS2String mEntitlementName;	// Entitlement name
} GSBEntitlement;

// GSBRole
// Summary
//		Structure that holds an brigade member role.
// Remarks
//		Roles help the backend determine access level for performing
//      any actions or retrieving brigade specific information based
//      on the entitlements assigned to that role.  Players can be 
//      promoted and demoted using gsbUpdateBrigadeMember.
//
//		Use gsbGetRoleList() to get available roles.
//
//		Use gsbCreateRole() to add a new role to a brigade
//
//		Use gsbUpdateRole() to change an existing role in a brigade
//
//		Use gsbRemoveRole() to delete an existing role to a brigade
typedef struct GSBRole
{
	gsi_u32 mBrigadeId;		// brigade ID
	gsi_u32 mRoleId;		// role ID
	UCS2String mRoleName;	// role name
	gsi_bool mIsDefault;	// true if default role for new members
	gsi_bool mIsGameRole;	// true if role is static and cannot be deleted
} GSBRole;


// GSBEntitlementList
// Summary
//		Structure that holds a list of entitlements.
// Remarks
//		An entitlement defines an action that a brigade member can take.
//
//		Use gsbGetEntitlementList() to get 
//		available entitlements given a role id.
typedef struct GSBEntitlementList
{
	GSBEntitlement *mEntitlements;	// pointer to array of entitlement structures
	gsi_u32 mCount;					// number of structures in array
} GSBEntitlementList;

// GSBRoleEntitlement
// Summary
//		Structure that holds maps an entitlement to a role.
// Remarks
//		An entitlement defines an action that a brigade member can take. 
//		A role defines a title that a brigade member has. 
//		A role can have many entitlements.  This structure
//		maps out the relationship between the two.
//
//		Use gsbGetRoleEntitlementList() to get available entitlements.
typedef struct GSBRoleEntitlement
{
	gsi_u32	mRoleEntitlementId;		// ID for  role-entitlement relation
	gsi_u32 mRoleId;				// role ID
	gsi_u32 mEntitlementId;			// entitlement ID	
} GSBRoleEntitlement;

// GSBRoleEntitlementList
// Summary
//		Structure that holds a list of entitlements to roles mapping.
// Remarks
//		Relations are contained in GSBRoleEntitlement. Please see 
//      GSBRoleEntitlement for details on that structure.
//      This list needs to be freed using gsbFreeEntitlementList.
//
//		Use gsbGetRoleEntitlementList() to get available entitlements.
typedef struct GSBRoleEntitlementList
{
	GSBRoleEntitlement *mRoleEntitlements;	// pointer to array of role-entitlement structures
	gsi_u32 mCount;							// number of structures in array
} GSBRoleEntitlementList;

// GSBEntitlementIdList
// Summary
//		Structure that holds a list of entitlement IDs.
// Remarks
//		An entitlement defines an action that a brigade member can perform.
//      Note that this structure holds a list of Entitlement Identifiers, 
//      not the complete entitlement.  
//
//		Use gsbGetEntitlementList() to get a list of available entitlements.
//
//	Note: This structure holds a list of 
//		Entitlement Identifiers not the complete entitlement.
typedef struct GSBEntitlementIdList
{
	gsi_u32 mCount;					// number of structures in array
	gsi_u32 *mEntitlementIds;		// pointer to array of entitlement IDs
} GSBEntitlementIdList;


// GSBRoleIdList
// Summary
//		Structure that holds a list of role IDs.
// Remarks
//		A role defines a title that a brigade member has.
//
//		Use gsbGetRoleList() to get a list of available roles.
typedef struct GSBRoleIdList
{
	gsi_u32 mCount;			// number of structures in array
	gsi_u32 *mRoleIds;		// pointer to array of role IDs
} GSBRoleIdList;


// GSBBrigadeMember
// Summary
//		Structure that holds a brigade member's info
// Remarks
//		Use gsbUpdateBrigadeMember() to change a member's info.
//
//		Use gsbRemoveBrigadeMember() to remove a member from a brigade.
typedef struct GSBBrigadeMember
{
    gsi_u32		mBrigadeMemberId;		// member ID
    gsi_u32		mBrigadeId;				// brigade ID
    gsi_u32		mProfileId;				// player profile ID
    UCS2String	mDescription;			// member general info
    time_t		mDateAdded;				// date added to brigade regardless of initial status
    GSBBrigadeMemberStatus mStatus;		// current status
    UCS2String	mTitle;					// custom title
    gsi_u32		mRoleId;				// current role
    gsi_bool	mEmailOptIn;			// opt-in to receive brigade emails if true	
	gsi_bool    mIsLeader;
} GSBBrigadeMember;

// GSBBrigadeMemberList
// Summary
//		Structure that holds a list of members in a brigade
// Remarks
//		Use gsbGetBrigadeMemberList() to get the list of members on a brigade.
typedef struct GSBBrigadeMemberList
{
    GSBBrigadeMember *mBrigadeMembers;	// pointer to array of brigade member structures
    gsi_u32 mCount;						// number of structures in array
} GSBBrigadeMemberList;

// GSBRoleList
// Summary
//		Structure that holds a list of roles in a brigade
// Remarks
//		Use gsbGetRoleList() to get the list of members on a brigade.
typedef struct GSBRoleList
{
	GSBRole *mRoles;	// pointer to array of role structures
	gsi_u32	mCount;		// number of structures in array
} GSBRoleList;

// GSBSearchFilterValue
// Summary
//		A union of different types of values stored in the mValue field
// Remarks
//		Used by GSBSearchFilterList()
typedef union GSBSearchFilterValue
{
    UCS2String mValueStr;      // used with GSBSearchFilterKey_Name, GSBSearchFilterKey_Tag, GSBSearchFilterKey_ExtraParams
    gsi_u32    mValueUint;      // used with GSBSearchFilterKey_BrigadeId, GSBSearchFilterKey_BrigadeStatus, GSBSearchFilterKey_MaxResultCount
    time_t     mValueTime;      // used with GSBSearchFilterKey_DateCreated
}GSBSearchFilterValue;

// GSBSearchFilter
// Summary
//		Structure that holds a search key/value pair.
// Remarks
//		Used by GSBSearchFilterList()
typedef struct GSBSearchFilter
{
    GSBSearchFilterKey          mKey;		// search filter name
    GSBSearchFilterValueType    mType;	    // search filter value
    GSBSearchFilterValue        mValue;     // search value could be of type UCS2String, int or time_t.
}GSBSearchFilter;
  
// GSBSearchFilterList
// Summary
//		Structure that holds a list of key/value pairs
// Remarks
//		Used in search functions
typedef struct GSBSearchFilterList
{
	gsi_u32			mCount;			// number of structures in array
	GSBSearchFilter *mFilters;	// pointer to array of search filter structures

}GSBSearchFilterList;

// GSBBrigadeLogo
// Summary
//		Structure that represents the URL location for a brigade logo.
// Remarks
//		By default a new brigade does not have a logo set.
//		An entitled brigade member can set a URL for the logo.
//
//		Use gsbiServiceSaveBrigadeLogo to change the brigade logo.
//
//		Use gsbUploadLogo to upload the brigade logo.
//
//		Use gsbDownloadLogo to download the brigade logo.
typedef struct GSBBrigadeLogo
{
	gsi_u32 mFileId;		// file number
	gsi_u32 mSizeId;		// image file size
	gsi_bool mDefaultLogo;	// default logo image if true
	UCS2String mPath;		// image file path
	UCS2String mUrl;		// image file URL
} GSBBrigadeLogo;

// GSBBrigadeLogoList
// Summary
//		Structure that holds a list of brigade logos
// Remarks
//		A list of logos with paths and URLs will be sent from the backend
//      if a brigade has a logo set.
typedef struct GSBBrigadeLogoList
{
	gsi_u32 mCount;				// number of structures in array
	GSBBrigadeLogo *mLogos;		// pointer to array of search filter structures
} GSBBrigadeLogoList;

// GSBBrigade
// Summary
//		Structure that holds information for a brigade .
// Remarks
//		A brigade is associated with a game.
//		Members can be in only one brigade per game.
//
//		Use gsbiServiceSaveBrigadeLogo to change the brigade logo.
//
//		Use gsbDownloadLogo to download the brigade logo.
typedef struct GSBBrigade
{
	gsi_u32 mGameId;		// the game that the brigade is associated with
	gsi_u32 mBrigadeId;		// the brigade ID		

	UCS2String mName;		// the brigade name
	UCS2String mTag;		// tag string associated with brigade
	UCS2String mUrl;		// external URL for brigade home page

	gsi_u32 mCreatorProfileId;	// player profile ID of brigade creator
	gsi_u32 mLeaderProfileId;	// player profile ID of brigade leader

	GSBRecruitingType mRecruitingType;	// Recruitment type for brigade

	UCS2String mMessageOfTheDay;		// a display message for brigade

    time_t mCreatedDate;	// date the brigade created
	gsi_bool mDisbanded;	// brigade is disbanded if true
	time_t mDisbandDate;	// date the brigade disbanded


	// This list will be filled in by the back end
	// No need to fill this in when creating a brigade
	GSBBrigadeLogoList mLogoList;	// list of logos for the brigade
} GSBBrigade;

// GSBBrigadeList
// Summary
//		Structure that holds a list of brigades
// Remarks
//		Use gsbGetBrigadeMemberList() to get the 
//		list of brigades for the current game
typedef struct GSBBrigadeList
{
	gsi_u32 mCount;
	GSBBrigade *mBrigades;
} GSBBrigadeList;

// GSBBrigadeHistoryEntry
// Summary
//		Structure that holds historical actions in a brigade.
// Remarks
//		All changes and invite/join requests to a brigade are saved.
//
//		Use gsbGetBrigadeMatchHistory() to get 
//		the information for a specific brigade match
typedef struct GSBBrigadeHistoryEntry
{
    gsi_u32                    mHistoryEntryId;			// entry ID
    gsi_u32                    mBrigadeId;				// brigade ID
    gsi_u32                    mInstigatingProfileId;	// player doing the request
    gsi_u32                    mTargetProfileId;		// player affected by request
    gsi_u32                    mReferenceId;            // a reference to a specific match 
    UCS2String                 mAccessLevel;            // player role needed to view entry
    UCS2String                 mHistoryAction;			// action
    UCS2String                 mNotes;					// a note

    time_t                     mDateCreated;			// entry creation date/time
    UCS2String                 mTargetProfileNickname;	// nickname of affected player
    UCS2String                 mSourceProfileNickname;	// nickname of player requesting
} GSBBrigadeHistoryEntry;

// GSBBrigadeHistoryList
// Summary
//		Structure that holds a list of brigade history entries
// Remarks
//		Use gsbGetBrigadeHistory() to get the 
//		list of history items for a brigade.
typedef struct GSBBrigadeHistoryList
{
    gsi_u32 mCount;
    GSBBrigadeHistoryEntry *mBrigadeHistory;
} GSBBrigadeHistoryList;

// GSBBrigadePendingActions
// Summary
//		Structure that holds a player's pending invite
//		or join request for a brigade
// Remarks
//		Use gsbGetMyPendingInvitesAndJoins()
typedef struct GSBBrigadePendingActions
{
	gsi_u32		mBrigadeId;				// brigade ID
	UCS2String  mBrigadeName;			// the brigade name
	time_t		mDateAdded;				// date of request
	GSBBrigadeMemberStatus mStatus;		// current status: Invite or RequestJoin
} GSBBrigadePendingActions;

// GSBBrigadePendingActionsList
// Summary
//		Structure that holds a list of pending invites and 
//		join requests made by the current player/member.
//		The returned list may contain invites and 
//		joins for other brigades as well.
// Remarks
//		Used by gsbGetMyPendingInvitesAndJoins()
typedef struct GSBBrigadePendingActionsList
{
	gsi_u32 mCount;
	GSBBrigadePendingActions *mPendingActions;
} GSBBrigadePendingActionsList;

///////////////////////////////////////////////////////////////////////////////
//Callbacks for asynchronous calls
///////////////////////////////////////////////////////////////////////////////

// GSBDisbandBrigadeCallback
// Summary
//		This typedef defines the callback function that you can create to 
//		receive the reply status from the asynchronous call to gsbDisbandBrigade().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBDisbandBrigadeCallback)(GSResult operationResult, 
                                          GSResultSet *resultSet, 
                                          void *userData);

// GSBGetBrigadeByIdCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and theBrigade from the asynchronous call to 
//		gsbGetBrigadeById().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		theBrigade		: [out] A pointer to the requested brigade. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and 
//								returned to the callback 
typedef void (*GSBGetBrigadeByIdCallback)(GSResult operationResult, 
                                          GSResultSet *resultSet, 
										  GSBBrigade *theBrigade, 
                                          void *userData);

// GSBGetBrigadesByProfileIdCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and brigadeList from the asynchronous call to
//		gsbGetBrigadesByProfileId().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		brigadeList		: [out] A pointer to the requested brigade list. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBGetBrigadesByProfileIdCallback)(GSResult operationResult, 
                                                  GSResultSet *resultSet, 
												  GSBBrigadeList *brigadeList, 
                                                  void *userData);

// GSBSaveBrigadeCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and updatedBrigade from the asynchronous call 
//		to create and update a brigade: gsbSaveBrigade().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		updatedBrigade	: [out] A pointer to the created/updated brigade. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBSaveBrigadeCallback)(GSResult operationResult, 
                                       GSResultSet *resultSet, 
									   GSBBrigade *updatedBrigade, 
                                       void *userData);

// GSBSearchBrigadesCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and brigadeList from the asynchronous call to 
//		gsbSearchBrigades().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		brigadeList		: [out] A pointer to the requested brigade list. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and
//		returned to the callback 
typedef void (*GSBSearchBrigadesCallback)(GSResult operationResult, 
                                          GSResultSet *resultSet, 
										  GSBBrigadeList *brigadeList, 
                                          void *userData);

// GSBSearchPlayersCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and brigadeMemberList from the asynchronous call to
//		gsbSearchPlayers().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		brigadeMemberList	: [out] A pointer to the requested brigade member list. 
//									(Memory allocated)							 
//		userData			: [in] Data that was passed in to the function call
//									and returned to the callback 
typedef void (*GSBSearchPlayersCallback)(GSResult operationResult, 
                                         GSResultSet *resultSet, 
										 GSBBrigadeMemberList *brigadeMemberList,
                                         void *userData);

// GSBSendMessageToBrigadeCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to 
//		gsbSendMessageToBrigade().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call
//								and returned to the callback 
typedef void (*GSBSendMessageToBrigadeCallback)(GSResult operationResult, 
                                                GSResultSet *resultSet, 
                                                void *userData);

// GSBSendMessageToMemberCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to 
//		gsbSendMessageToMember().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call
//								and returned to the callback 
typedef void (*GSBSendMessageToMemberCallback)(GSResult operationResult, 
                                               GSResultSet *resultSet, 
                                               void *userData);

// GSBGetBrigadeHistoryCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and brigadeHistoryList from the asynchronous call to 
//		gsbGetBrigadeHistory().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		brigadeHistoryList	: [out] A pointer to the requested list of 
//									brigade history entries. (Memory allocated)							 
//		userData			: [in] Data that was passed in to the function call
//									and returned to the callback 
typedef void (*GSBGetBrigadeHistoryCallback)(GSResult operationResult, 
											 GSResultSet *resultSet, 
											 GSBBrigadeHistoryList *brigadeHistoryList, 
											 void *userData);

// GSBGetBrigadeMatchHistoryCallback
// Summary
//		This typedef defines the callback function that you create to receive
//		the reply status and brigadeHistoryList from the asynchronous call to 
//		gsbGetBrigadeMatchHistory().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		brigadeHistoryList	: [out] A pointer to the requested list of brigade 
//									match history entries. (Memory allocated)							 
//		userData			: [in] Data that was passed in to the function call
//									and returned to the callback 
typedef void (*GSBGetBrigadeMatchHistoryCallback)(GSResult operationResult, 
												  GSResultSet *resultSet, 
												  GSBBrigadeHistoryList *brigadeHistoryList, 
												  void *userData);

// GSBUploadLogoCompleteCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to gsbUploadLogo().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBUploadLogoCompleteCallback)(GSResult operationResult, 
                                              GSResultSet *resultSet, 
                                              void *userData);

// GSBDownloadLogoCompleteCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to 
//		gsbDownloadLogo().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBDownloadLogoCompleteCallback)(GSResult operationResult, 
                                                GSResultSet *resultSet, 
                                                void *userData);

// GSBGetEntitlementListCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and entitlementList from the asynchronous call to
//		gsbGetEntitlementList().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		entitlementList	: [out] A pointer to the requested list of entitlements. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBGetEntitlementListCallback)(GSResult operationResult, 
											  GSResultSet *resultSet, 
											  GSBEntitlementList *entitlementList, 
											  void *userData);

// GSBGetEntitlementIdListByRoleIdCallback
// Summary
//		This typedef defines the callback function that you create to receive
//		the reply status and entitlementIdList from the asynchronous call to
//		gsbGetEntitlementIdListByRoleId().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		entitlementIdList	: [out] A pointer to the requested list of 
//									entitlement IDs. (Memory allocated)							 
//		userData			: [in] Data that was passed in to the function call
//									and returned to the callback 
typedef void (*GSBGetEntitlementIdListByRoleIdCallback)(GSResult operationResult, 
														GSResultSet *resultSet, 
														GSBEntitlementIdList *entitlementIdList, 
														void *userData);

// GSBGetRoleIdListByEntitlementIdCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and roleIdList from the asynchronous call to 
//		gsbGetRoleIdListByEntitlementId().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		roleIdList		: [out] A pointer to the requested list of role IDs.
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBGetRoleIdListByEntitlementIdCallback) (GSResult operationResult, 
														 GSResultSet *resultSet, 
														 GSBRoleIdList *roleIdList, 
														 void *userData);

// GSBGetPendingInvitesAndJoinsCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and pendingActionsList from the asynchronous call to
//		gsbGetMyPendingInvitesAndJoins().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		pendingActionsList	: [out] A pointer to the requested list of pending
//								invites/joins. (Memory allocated)							 
//		userData			: [in] Data that was passed in to the function call
//								and returned to the callback 
typedef void (*GSBGetPendingInvitesAndJoinsCallback)(GSResult operationResult, 
													 GSResultSet *resultSet,
													 GSBBrigadePendingActionsList *pendingActionsList, 
													 void *userData);

// GSBGetBrigadeMemberListCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and theBrigade from the asynchronous call to 
//		gsbGetBrigadeMemberList().
// Parameters
//		operationResult		: [in] Result from the operation
//		resultSet			: [in] Set of result codes from the function call
//		brigadeMemberList	: [out] A pointer to the requested list of 
//									brigade members. (Memory allocated)							 
//		userData			: [in] Data that was passed in to the function 
//									call and returned to the callback 
typedef void (*GSBGetBrigadeMemberListCallback)(GSResult operationResult, 
                                                GSResultSet *resultSet, 
												GSBBrigadeMemberList *brigadeMemberList, void *userData);

// GSBUpdateBrigadeMemberCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and theBrigade from the asynchronous call to 
//		gsbUpdateBrigadeMember().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBUpdateBrigadeMemberCallback)(GSResult operationResult, 
                                               GSResultSet *resultSet, 
                                               void *userData);

// GSBRemoveBrigadeMemberCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and theBrigade from the asynchronous call to 
//		gsbRemoveBrigadeMember().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call 
//								and returned to the callback 
typedef void (*GSBRemoveBrigadeMemberCallback)(GSResult operationResult, 
                                               GSResultSet *resultSet, 
                                               void *userData);

// This callback function is used for getting result from calls to:
// GSBGetBrigadeByIdCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to:
//		gsbInviteToBrigade(), gsbAnswerInvite(), gsbJoinBrigade(), 
//		gsbAnswerJoin(),	gsbRemoveMember(), gsbBanMember(), 
//		gsbLeaveBrigade() and gsbRescindInvite()
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call							 
//		userData		: [in] Data that was passed in to the function call 
//								and returned to the callback 
typedef void (*GSBPerformBrigadeMemberActionCallback)(GSResult operationResult, 
                                                      GSResultSet *resultSet,
													  void *userData);

// GSBRemoveRoleCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to gsbRemoveRole().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call 
//								and returned to the callback 
typedef void (*GSBRemoveRoleCallback)(GSResult operationResult, 
                                      GSResultSet *resultSet, 
                                      void *userData);

// GSBGetRoleListCallback
// Summary
//		This typedef defines the callback function that you create to receive 
//		the reply status and a list of roles from the asynchronous call to 
//		gsbGetRoleList().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		roleList		: [out] A pointer to the requested list of roles. 
//								(Memory allocated)							 
//		userData		: [in] Data that was passed in to the function call 
//								and returned to the callback 
typedef void (*GSBGetRoleListCallback)(GSResult operationResult, 
                                       GSResultSet *resultSet, 
									   GSBRoleList *roleList, 
                                       void *userData);

// GSBUpdateRoleCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status and the role ID from the asynchronous call to
//		gsbCreateRole() and gsbUpdateRole().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		roleId			: [out] Role ID for newly created  or modified role
//		userData		: [in] Data that was passed in to the function call and 
//								returned to the callback 
typedef void (*GSBUpdateRoleCallback)(GSResult operationResult, 
                                      GSResultSet *resultSet, 
									  gsi_u32 roleId, 
                                      void *userData);

// GSBPromoteToLeaderCallback
// Summary
//		This typedef defines the callback function that you create to 
//		receive the reply status from the asynchronous call to 
//      gsbPromotoToLeader().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and 
//								returned to the callback 
typedef void (*GSBPromoteToLeaderCallback)(GSResult     operationResult, 
                                           GSResultSet  *resultSet, 
                                           void         *userData);

// GSBUpdateMemberEmailAndNickCallback
// Summary
//		This typedef defines the callback function that you can create to 
//		receive the reply status from the asynchronous call to 
//      gsbUpdateMemberEmailAndNick().
// Parameters
//		operationResult	: [in] Result from the operation
//		resultSet		: [in] Set of result codes from the function call
//		userData		: [in] Data that was passed in to the function call and
//								returned to the callback 
typedef void (*GSBUpdateMemberEmailAndNickCallback)(GSResult operationResult, 
                                                    GSResultSet *resultSet, 
                                                    void *userData);

///////////////////////////////////////////////////////////////////////////////


// Initialization and Cleanup
// gsbInitialize
// Summary
//		Creates an GSB object instance used for communicating with Brigade server 
// Parameters
//		theInstance    : [out] The pointer to the Brigades SDK instance which 
//								is required by most Brigades SDK functions.
//		theGameId      : [in] The current game ID
//		theCertificate : [in] A pointer to the login certificate to communicate 
//								with the Brigade server 
//		thePrivateData : [in] A pointer to the game's private data
// Returns
//		GSResult - indicates success or failure.  Possible values:
//			GS_SUCCESS on success, 
//			GSResultCode_OutOfMemory
// Remarks
//		Initializes theInstance with the game ID, login certificate and private data
GSResult gsbInitialize(GSBInstancePtr           *theInstanceOut,
                       gsi_u32                   theGameId,
                       const GSLoginCertificate *theCertificate, 
                       const GSLoginPrivateData *thePrivateData);
					    
// gsbShutdown
// Summary
//		Releases the GSB object instance used for 
//		communicating with Brigade server 
// Parameters
//		theInstance : [out] The pointer to the Brigades SDK instance.
// Returns
//		GSResult - indicates success or failure. Possible values:
//			GS_SUCCESS on success.
//			GSResultCode_OutOfMemory
// Remarks
//		Initializes theInstance with the game ID, 
//		login certificate and private data
GSResult gsbShutdown(GSBInstancePtr theInstance);



///////////////////////////////////////////////////////////////////////////////
/// Service Calls 
/// These calls are asynchronous to allow for uninterrupted application execution
///////////////////////////////////////////////////////////////////////////////

// gsbSaveBrigade
// Summary
//		Creates a new brigade or updates an existing brigade.
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance 
//		theBrigade  : [in] A pointer to the instance of the brigade being 
//							created or updated
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		<pre>
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNameAlreadyExists,
//			GSBResultCode_BrigadeTagAlreadyExists,
//			GSBResultCode_NotFound,
//			GSBResultCode_BrigadeNameCantBeChanged,
//			GSBResultCode_BrigadeGameIDCantBeChanged
//
//      When CREATING a brigade, theBrigade must be allocated and
//		the following values must be set:
//				[set/unchanged] mGameId = current game ID
//				mBrigadeId = 0
//				mName = the brigade name
//				mTag = tag string associated with brigade
//				mCreatorProfileId = player profile ID of brigade creator
//				mLeaderProfileId = player profile ID of brigade leader
//				mMessageOfTheDay = a display message for brigade
//				[optional] mUrl = external URL for brigade home page
//				[optional] mRecruitingType = one of GSBRecruitingType values
//											 default = Open
//				[ignored] mDisbanded, mDisbandDate, mLogoList
//
//      When UPDATING a brigade, theBrigade must be allocated and 
//		initialized with its latest values
//		retrieved from the server then modify the following values as needed:
//				[set/unchanged] mGameId = current game ID
//				[set/unchanged] mBrigadeId = existing brigade ID
//				mTag = existing or modified tag string associated with brigade
//				mMessageOfTheDay = a display message for brigade
//				mUrl = external URL for brigade home page
//				mRecruitingType = one of GSBRecruitingType values
//				[ignored] mName, mDisbanded, mDisbandDate, mLogoList
//				[ignored] mCreatorProfileId, mLeaderProfileId
//		</pre>
//
//		NOTE: Once a brigade has been created, its name cannot be changed.
// 
GSResult gsbSaveBrigade(GSBInstancePtr              theInstance,
						GSBBrigade                  *theBrigade, 
						GSBSaveBrigadeCallback       callback, 
						void                        *userData);

// gsbDisbandBrigade
// Summary
//		Removes all members from a brigade then deletes the brigade
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//		brigadeId   : [in] The ID of the brigade to disband
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult
//			GS_SUCCESS on success, 
//			GSBResultCode_BrigadeNotFound
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound
// 
//		Note: Only the brigade leader can disband a brigade
//
GSResult gsbDisbandBrigade(GSBInstancePtr            theInstance,
						   gsi_u32					 brigadeId,
                           GSBDisbandBrigadeCallback callback, 
                           void                     *userData);                            

// gsbGetBrigadeById
// Summary
//		Retrieves all members of a brigade given a brigade ID
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//		brigadeId   : [in] The ID of the brigade to disband
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound
//
GSResult gsbGetBrigadeById(GSBInstancePtr            theInstance,
                           gsi_u32					 brigadeId, 
                           GSBGetBrigadeByIdCallback callback, 
                           void                     *userData);

// gsbInviteToBrigade
// Summary
//		Sends a message to a player that is invited to join a brigade
//		A member entry is created with GSBBrigadeMemberInvited status
//		in the brigade
// Parameters
//		theInstance			: [in] The pointer to the Brigades SDK instance.
//		brigadeId			: [in] The ID of the brigade issuing the invite
//		profileIdToInvite	: [in] The ID of the player invited
//		callback			: [in] The developer callback 
//		userData			: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction
//
//		Note: Only entitled brigade members can invite other players to join
//
GSResult gsbInviteToBrigade(GSBInstancePtr              theInstance,
							    gsi_u32					brigadeId, 
                                gsi_u32					profileIdToInvite, 
                                GSBPerformBrigadeMemberActionCallback callback, 
                                void                    *userData);

// gsbRescindInvite
// Summary
//		Cancel invitation to brigade
//		 
// Parameters
//		theInstance				: [in] The pointer to the Brigades SDK instance.
//		brigadeId				: [in] The ID of the brigade 
//										rescinding the invite
//		profileIdToCancelInvite	: [in] The ID of the player invited
//		callback				: [in] The developer callback 
//		userData				: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction
//
//		Note: Only entitled brigade members can rescind an invitation
//
GSResult gsbRescindInvite(GSBInstancePtr            theInstance,
							gsi_u32					brigadeId, 
							gsi_u32					profileIdToCancelInvite, 
							GSBPerformBrigadeMemberActionCallback callback, 
							void                    *userData);

// gsbAnswerInvite
// Summary
//		Sends a message to a player that is invited to join a brigade
//		A member entry is created with GSBBrigadeMemberInvited status in the brigade
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The ID of the brigade that issued the invite
//		acceptInvite		: [in] 1 = acceptInvite invite,   0 = decline invite
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound,
//			GSBResultCode_NoPermisionForAction,
//			GSBResultCode_BrigadeMemberWasNotInvited,
//			GSBResultCode_BrigadeMemberIDNotFound,
//			GSBResultCode_BrigadeMaxMembersReached,
//			GSBResultCode_BrigadeLimitedOnePerMember
//
//		Note: Only the invited player can acceptInvite/decline 
//		their invitation to join a brigade.
//
GSResult gsbAnswerInvite(GSBInstancePtr				theInstance,
						 gsi_u32					brigadeId,
						 gsi_bool					acceptInvite,
						 GSBPerformBrigadeMemberActionCallback callback, 
						 void						*userData);

// gsbJoinBrigade
// Summary
//		Creates a member entry with GSBBrigadeMemberRequestJoin 
//		status in the brigade for the current player and game
// Parameters
//		theInstance			: [in] The pointer to the Brigades SDK instance.
//		brigadeId			: [in] The ID of the brigade to join
//		callback			: [in] The developer callback 
//		userData			: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_BrigadeMaxMembersReached, 
//			GSBResultCode_BrigadeIsInviteOnly
//
//		Note: Any player can request to join a brigade
//
GSResult gsbJoinBrigade(GSBInstancePtr				theInstance,
						gsi_u32						brigadeId,
                        GSBPerformBrigadeMemberActionCallback    callback, 
                        void						*userData);

// gsbAnswerJoin
// Summary
//		Sends a message to a player that is invited to join a brigade
//		A member entry is created with GSBBrigadeMemberInvited 
//		status in the brigade
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		brigadeId		: [in] The ID of the brigade that issued the invite
//		playerProfileId	: [in] The ID of the player invited
//		acceptJoin		: [in] 1 = acceptJoin invite,   0 = decline invite
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction, 
//			GSBResultCode_BrigadeMemberHasNotRequestedJoin, 
//			GSBResultCode_BrigadeMemberIDNotFound, 
//			GSBResultCode_BrigadeMaxMembersReached, 
//			GSBResultCode_BrigadeLimitedOnePerMember
//
//		Note: Only entitled brigade members can acceptJoin/decline requests to join
//
GSResult gsbAnswerJoin(GSBInstancePtr           theInstance,
						  gsi_u32				brigadeId,
						  gsi_u32				playerProfileId,
						  gsi_bool				acceptJoin,
                          GSBPerformBrigadeMemberActionCallback    callback, 
                          void                  *userData);

// gsbRemoveBrigadeMember
// Summary
//		Sets the brigade member's status to GSBBrigadeMemberInactive 
//		if members remove themselves. If an entitled member removes a member,
//		the status becomes GSBBrigadeMemberKicked
// Parameters
//		theInstance			: [in] The pointer to the Brigades SDK instance.
//		brigadeId			: [in] The ID of the brigade
//		profileIdToRemove	: [in] The ID of the player to remove
//		callback			: [in] The developer callback 
//		userData			: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction, 
//			GSBResultCode_BrigadeMemberIDNotFound
//
GSResult gsbRemoveBrigadeMember(GSBInstancePtr        theInstance,
						gsi_u32						  brigadeId,
						gsi_u32					      profileIdToRemove, 
						GSBPerformBrigadeMemberActionCallback  callback, 
						void                          *userData);

// gsbBanMember
// Summary
//		Sets the brigade member's status to GSBBrigadeMemberBlocked
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		brigadeId		: [in] The ID of the brigade
//		profileIdToBan	: [in] The ID of the player to ban
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction, 
//			GSBResultCode_BrigadeMemberIDNotFound
//
//		Note: Only entitled brigade members can ban a member from the brigade
//
GSResult gsbBanMember(GSBInstancePtr                    theInstance,
					    gsi_u32							brigadeId,
						gsi_u32							profileIdToBan, 
						GSBPerformBrigadeMemberActionCallback  callback, 
						void                            *userData);

// gsbLeaveBrigade
// Summary
//		Sets the brigade member's own status to GSBBrigadeMemberInactive.
//		Status is not changed if member's current status is 
//		GSBBrigadeMemberKicked or GSBBrigadeMemberBlocked.
//		Called to remove self or cancel join/invite request from a brigade
// Parameters
//		theInstance			: [in] The pointer to the Brigades SDK instance.
//		brigadeId			: [in] The ID of the brigade
//		callback			: [in] The developer callback 
//		userData			: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData.
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_NoPermisionForAction, 
//			GSBResultCode_BrigadeMemberIDNotFound, 
//			GSBResultCode_BrigadeMemberStatusBlockedOrKicked
// 
GSResult gsbLeaveBrigade(GSBInstancePtr               theInstance,
						gsi_u32					      brigadeId, 
						GSBPerformBrigadeMemberActionCallback  callback, 
						void                          *userData);

// gsbGetEntitlementList
// Summary
//		Requests the list of entitlements available for the current game
// Parameters
//		theInstance : [in] An instance of the SDK. 
//							Usually required for most functions.
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound
//		
GSResult gsbGetEntitlementList(GSBInstancePtr                theInstance,
                               GSBGetEntitlementListCallback callback, 
                               void                         *userData);

// gsbGetEntitlementIdListByRoleId
// Summary
//		Requests the list of entitlement IDs for a specific role in a brigade
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The ID of the brigade
//		roleId		: [in] The ID of the role
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound
//		
GSResult gsbGetEntitlementIdListByRoleId(GSBInstancePtr				theInstance,
										 gsi_u32					brigadeId,
										 gsi_u32					roleId,
										 GSBGetEntitlementIdListByRoleIdCallback	callback,
										 void						*userData);

// gsbGetRoleIdListByEntitlementId
// Summary
//		Requests the list of role IDs in a brigade with a specific entitlement 
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		brigadeId		: [in] The ID of the brigade
//		entitlementId	: [in] The ID of the entitlement
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound
//
GSResult gsbGetRoleIdListByEntitlementId(GSBInstancePtr						theInstance,
										 gsi_u32							brigadeId,
										 gsi_u32							entitlementId,
										 GSBGetRoleIdListByEntitlementIdCallback   callback, 
										 void								*userData);

// gsbGetBrigadeMemberList
// Summary
//		Requests the list of members in a specified brigade for the current game
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The ID of the brigade
//		statuses	: [in] ORed bitmap of values from GSBBrigadeMemberStatus 
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SEUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete. 
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound
//		
GSResult gsbGetBrigadeMemberList(GSBInstancePtr                    theInstance,
                                 gsi_u32                           brigadeId,
								 gsi_u32						   statuses,
                                 GSBGetBrigadeMemberListCallback   callback, 
                                 void                             *userData);

// gsbGetMyPendingInvitesAndJoins
// Summary
//		Requests the list of brigades which this 
//		player has received invitation to or requested to join for the current game.
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound
//		
GSResult gsbGetMyPendingInvitesAndJoins(GSBInstancePtr                   theInstance,
									  GSBGetPendingInvitesAndJoinsCallback   callback, 
									  void                             *userData);


// gsbUpdateBrigadeMember
// Summary
//		Update a brigade member's info
//				<pre>
//				[set/unchanged] mBrigadeMemberId = current member's ID
//				[set/unchanged] mBrigadeId = current member's brigade ID
//				[set/unchanged] mProfileId = member's player profile ID
//				mDescription = member general info
//				mStatus = new GSBBrigadeMemberStatus value or current status
//				mTitle = custom title
//				mRoleId = current role
//				[ignored] mDateAdded, mEmailOptIn
//				</pre>
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//		theMember	: [in] The brigade member information
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_BrigadeMemberIDNotFound
//		
GSResult gsbUpdateBrigadeMember(GSBInstancePtr                    theInstance,
                                GSBBrigadeMember                 *theMember, 
                                GSBUpdateBrigadeMemberCallback    callback, 
                                void                             *userData);     
// gsbPromoteToLeader
// Summary
//		This API allows a Leader to replace his as the new leader. 
//      The former leader becomes a brigade member.
// Parameters
//		theInstance : [in] The pointer to the Brigades SDK instance.
//      brigadeId   : [in] The leader's brigade Id.
//      memberId    : [in] The member id of the member to be promoted.
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//      Only a leader is permitted to promote a member to replace him.
GSResult gsbPromoteToLeader(GSBInstancePtr	theInstance,
                            gsi_u32         brigadeId, 
                            gsi_u32         memberId, 
                            GSBPromoteToLeaderCallback  callback,
                            void            *userData);

// gsbCreateRole
// Summary
//		Creates a custom role with its associated entitlements
//		A brigade can create custom roles 
//		in addition to the permanent game roles.
//		Each role has an associated list of game entitlements 
//		When creating a role, a list of game entitlements must be provided.
//		The following role information must be set:
//				<pre>
//				mBrigadeId = brigade ID to add role
//				mRoleName = role name
//				[ignored] mRoleId, mIsGameRole, mIsDefault
//				</pre>
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		theRole			: [in] The new role information
//		entitlementList : [in] The list of entitlements associated with the role
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_UnknownEntitlementFound, 
//			GSBResultCode_RoleIsGameRole, 
//			GSBResultCode_RoleAlreadyExists
//		
// Note: Only entitled brigade member can create a brigade role
//
GSResult gsbCreateRole(GSBInstancePtr         theInstance,
					   GSBRole                *theRole,
					   GSBEntitlementIdList	  *entitlementIdList,
					   GSBUpdateRoleCallback  callback, 
					   void                   *userData);

// gsbUpdateRole
// Summary
//		Updates an existing custom role and its entitlements
//		A brigade can create custom roles 
//		in addition to the permanent game roles.
//		Each role has an associated list of game entitlements 
//		When updating a role, get the latest information for a role then change
//		the following role information as needed:
//				<pre>
//				[set/unchanged] mBrigadeId
//				[set/unchanged] mRoleId
//				mRoleName = existing role name or new role name
//				[ignored] mIsGameRole, mIsDefault
//				A list of entitlements to associate 
//				</pre>
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		theRole			: [in] The new role information
//		entitlementList : [in] The list of entitlements associated with the role
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_RoleNotFound, 
//			GSBResultCode_UnknownEntitlementFound, 
//			GSBResultCode_RoleIsGameRole
//
// Note: Only entitled brigade member can update a brigade role
//
GSResult gsbUpdateRole(GSBInstancePtr         theInstance,
					   GSBRole                *theRole,
					   GSBEntitlementIdList	  *entitlementIdList,
					   GSBUpdateRoleCallback  callback, 
					   void                   *userData);

// gsbRemoveRole
// Summary
//		Removes a custom role. Members of the deleted role will be changed to
//		the default brigade role.
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The brigade ID
//		roleId		: [in] The role ID to remove
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_RoleIsGameRole, 
//			GSBResultCode_RoleNotFound
//		
// Note: Only entitled brigade members can remove a brigade role
//
GSResult gsbRemoveRole(GSBInstancePtr				theInstance,
						 gsi_u32					brigadeId,
						 gsi_u32					roleId, 
						 GSBRemoveRoleCallback		callback, 
						 void						*userData);

// gsbGetRoleList
// Summary
//		Requests a list of all roles associated with a brigade. 
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The brigade ID
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_BrigadeNotFound
//		
//
GSResult gsbGetRoleList(GSBInstancePtr			theInstance,
						gsi_u32					brigadeId, 
                        GSBGetRoleListCallback	callback, 
                        void				   *userData);


// gsbSearchBrigades
// Summary
//		Request a list of brigades (GSBBrigadeList) having 
//		the specified filter criteria.
//		The search filter consists of a list of key/value pairs.
//		Refer to GSBSearchFilterKey for the list of keys. Values are strings.
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		filterList	: [in] The list of filter key/value pairs
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//		<p>
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_SearchCriteriaNotMet
//		<p>
//		This should only be used as a last resort.  If the Brigade ID is 
//		already known the gsbGetBrigadeById function should be used instead.
GSResult gsbSearchBrigades(GSBInstancePtr				theInstance,
                           GSBSearchFilterList          *filterList, 
                           GSBSearchBrigadesCallback    callback, 
                           void							*userData);


// gsbSearchPlayers
// Summary
//		Request a list of brigade members (GSBBrigadeMemberList) 
//		having the specified filter criteria.
//		The search filter consists of a list of key/value pairs.
//		Refer to GSBSearchFilterKey for the list of keys. Values are strings.
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		filterList	: [in] The list of filter key/value pairs
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//		<p>
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NotFound, 
//			GSBResultCode_SearchCriteriaNotMet
//		<p>
//		This should only be used as a last resort.  If the Brigade ID is 
//		already known the gsbGetBrigadeById function should be used instead.
GSResult gsbSearchPlayers(GSBInstancePtr theInstance, 
                          GSBSearchFilterList *filterList, 
                          GSBSearchPlayersCallback callback, 
                          void *userData);


// gsbSendMessageToBrigade
// Summary
//		An active brigade member can sends a message (maximum size 2KB)
//		to all active members on the same brigade
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		brigadeId	: [in] The brigade ID to send the message to
//		message		: [in] The message (formatted in Unicode) 
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound,
//			GSBResultCode_NoPermisionForAction
//
GSResult gsbSendMessageToBrigade(GSBInstancePtr theInstance,
							  gsi_u32        brigadeId, 
							  UCS2String     message,
							  GSBSendMessageToBrigadeCallback callback, 
							  void *userData);

// gsbUploadLogo
// Summary
//		An entitled active brigade member can change 
//		the brigade logo by uploading the image
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		logoFileName	: [in] The logo image filename 
//		brigadeId		: [in] The brigade ID
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound, 
//			GSBResultCode_BrigadeMemberNotFound, 
//			GSBResultCode_NoPermisionForAction
//
GSResult gsbUploadLogo(GSBInstancePtr theInstance, 
					   const gsi_char *logoFileName, 
					   gsi_u32 brigadeId, 
					   GSBUploadLogoCompleteCallback callback, 
					   void *userData);

// gsbDownloadLogo
// Summary
//		Allows entitled active brigade members to download their brigade logo
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		logoFileName	: [in] The logo image filename to save to 
//		logo			: [in] The logo location info
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound,
//			GSBResultCode_BrigadeMemberNotFound
//
GSResult gsbDownloadLogo(GSBInstancePtr theInstance, 
						 const gsi_char *logoFileName, 
						 GSBBrigadeLogo *logo,
						 GSBDownloadLogoCompleteCallback,
						 void *userData);

// gsbSendMessageToMember
// Summary
//		An active brigade member can send a message (maximum size 2KB)
//		to another active member on the same brigade
// Parameters
//		theInstance		: [in] The pointer to the Brigades SDK instance.
//		toProfileId		: [in] The receiver profile ID
//		message			: [in] The message (formatted in Unicode) 
//		callback		: [in] The developer callback 
//		userData		: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_NoPermisionForAction
//
GSResult gsbSendMessageToMember(GSBInstancePtr theInstance, 
                                gsi_u32        toProfileId,
                                UCS2String     message,
                                GSBSendMessageToMemberCallback callback, 
                                void *userData);

// gsbGetBrigadesByProfileId
// Summary
//		Retrieves all brigades for this game that a specified player belongs to.
//		In v1.0, a player can only belong to one brigade.
//		Future versions will allow belonging to multiple brigades.
//		In those versions, the first brigade 
//		will be the player's default brigade.
// Parameters
//		theInstance : [in] An instance of the SDK. 
//							Usually required for most functions.
//		profileId   : [in] The profile ID of the player
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound
//
GSResult gsbGetBrigadesByProfileId(GSBInstancePtr theInstance, 
                                   gsi_u32        profileId, 
                                   GSBGetBrigadesByProfileIdCallback callback, 
                                   void *userData);


// gsbGetBrigadeHistory
// Summary
//		Retrieve a list of brigade activities based on a player's access level
// Parameters
//		theInstance			: [in] The pointer to the Brigades SDK instance.
//		brigadeId			: [in] The ID of the brigade 
//		profileId			: [in] The ID of the player that
//									made the changes/invites
//		historyAccessLevel	: [in] The access level of the activity record
//		callback			: [in] The developer callback 
//		userData			: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_BrigadeNotFound,
//			GSBResultCode_NoPermisionForAction
//
//		If profileId is set to zero, it will do a search over all profiles.
//
//		If brigadeId is set to zero, no search data will be retrieved.
//
//		If the historyAccessLevel is above that of the requesting player,
//			the search will ignore the value and return only matching records
//			within the player's access level.
//
GSResult gsbGetBrigadeHistory(GSBInstancePtr				theInstance, 
                              gsi_u32						brigadeId, 
                              gsi_u32						profileId,
                              GSBBrigadeHistoryAccessLevel	historyAccessLevel,
                              GSBGetBrigadeHistoryCallback	callback, 
                              void							*userData);


// gsbGetBrigadeMatchHistory
// Summary
//		Retrieve a list of activities for a specific 
//		match for the member's current brigade
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		matchId		: [in] The ID of the match
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//
//		The resultSet in the callback can contain one of these values:
//			GSBResultCode_MatchIdNotFound,
//			GSBResultCode_NoPermisionForAction
//
//		If matchId is set to zero, no search data will be retrieved.
//
GSResult gsbGetBrigadeMatchHistory(GSBInstancePtr						theInstance, 
                                   gsi_u32								matchId, 
                                   GSBGetBrigadeMatchHistoryCallback	callback, 
                                   void									*userData);

// gsbUpdateMemberEmailAndNick
// Summary
//		This is a special API to trigger synchronize user's nick and email at the backend.
//      The Nick name and Email could be out of synch when changed outside of the 
//      game and this would update the brigades backend service with the new nickname and 
//      email address. It should be called whenever the brigade member logs in.   
// Parameters
//		theInstance	: [in] The pointer to the Brigades SDK instance.
//		callback	: [in] The developer callback 
//		userData	: [in] A pointer to the developer's data
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Every parameter is required except userData. 
//		The parameter theInstance needs to be initialized.
//		The callback will be called once the operation is complete.
//		<p>
//		This must be used to synchronize a user's nick information on the Brigades service,
//		if a player changes his profile nick via GP.  
GSResult gsbUpdateMemberEmailAndNick(
		GSBInstancePtr 	theInstance,  
		gsi_u32 		brigadeId, 
		GSBUpdateMemberEmailAndNickCallback callback, 
		void 			*userData);


///////////////////////////////////////////////////////////////////////////////
// Data helper functions
///////////////////////////////////////////////////////////////////////////////

// gsbCloneBrigadeMemberList
// Summary
//		Allocates memory and makes a deep copy of a list of brigade members
//		On error, memory allocated for copy is released and *destList = NULL
// Parameters
//		destList	: [out] The list of brigade members to copy to memory
//		srcList		: [in] The list of brigade members to copy from memory
// Returns
//		GSResult - 
//			GS_SUCCESS on success or 
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneBrigadeMemberList(GSBBrigadeMemberList **destList, GSBBrigadeMemberList *srcList);

// gsbCloneBrigadeMember
// Summary
//		Allocates memory and makes a deep copy of a brigade member
//		On error, memory allocated for copy is released and *destMember = NULL
// Parameters
//		destMember	: [out] The brigade member to copy to memory
//		srcMember	: [in] The brigade member to copy from memory
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneBrigadeMember(GSBBrigadeMember **destMember, GSBBrigadeMember *srcMember);

// gsbCloneBrigade
// Summary
//		Allocates memory and makes a deep copy of a brigade
//		On error, memory allocated for copy is released and *destBrigade = NULL
// Parameters
//		destBrigade	: [out] The brigade to copy to memory 
//		srcBrigade	: [in] The brigade to copy from memory
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneBrigade(GSBBrigade **destBrigade, GSBBrigade *srcBrigade);

// gsbCloneEntitlementIdList
// Summary
//		Allocates memory and makes a deep copy of a list of entitlement IDs
//		On error, memory allocated for copy is released and *destList = NULL
// Parameters
//		destList	: [out] The list of entitlement IDs to copy to memory 
//		srcList		: [in] The list of entitlement IDs to copy from memory 
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneEntitlementIdList(GSBEntitlementIdList **destList, GSBEntitlementIdList *srcList);

// gsbCloneEntitlementList
// Summary
//		Allocates memory and makes a deep copy of a list of entitlements
//		On error, memory allocated for copy is released and *destList = NULL
// Parameters
//		destList	: [out] The list of entitlements to copy to memory
//		srcList		: [in] The list of entitlements to copy from memory
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneEntitlementList(GSBEntitlementList **destList, GSBEntitlementList *srcList);

// gsbCloneEntitlement
// Summary
//		Allocates memory and makes a deep copy of an entitlement
//		On error, memory allocated for copy is released and
//		*destEntitlement = NULL
// Parameters
//		destEntitlement	: [out] The entitlement to copy to memory
//		srcEntitlement	: [in] The entitlement to copy from memory
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneEntitlement(GSBEntitlement *destEntitlement, GSBEntitlement *srcEntitlement);

// gsbCloneRole
// Summary
//		Allocates memory and makes a deep copy of a role.  
//		On error, memory allocated for copy is released and *destRole = NULL.
// Parameters
//		destEntitlement	: [out] The role to copy to memory
//		srcEntitlement	: [in] The role to copy from memory
// Returns
//		GSResult
//			GS_SUCCESS on success or
//			GSResultCode_OutOfMemory
//
GSResult gsbCloneRole(GSBRole **destRole, GSBRole *srcRole);

// gsbFreeBrigadeMemberList
// Summary
//		Deallocates memory for a list of brigade members (deep deallocation)
// Parameters
//		memberList	: [in] The list of brigade members to release from memory
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeBrigadeMemberList(GSBBrigadeMemberList *memberList);

// gsbFreeBrigadeMember
// Summary
//		Deallocates memory for a brigade member (deep deallocation)
// Parameters
//		member	: [in] The brigade member to release from memory
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeBrigadeMember(GSBBrigadeMember *member);

// gsbFreePendingActionsList
// Summary
//		Deallocates memory for a list of pending actions (deep deallocation)
// Parameters
//		actionList	: [in] The list of pending actions to release from memory.
// Returns
//		GSResult -
//			GS_SUCCESS on success.
//
GSResult gsbFreePendingActionsList(GSBBrigadePendingActionsList *actionList);

// gsbFreeRoleList
// Summary
//		Deallocates memory for a list of roles(deep deallocation)
// Parameters
//		roleList	: [in] The list of roles to release from memory
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeRoleList(GSBRoleList *roleList);

// gsbFreeRole
// Summary
//		Deallocates memory for a single role(deep deallocation)
// Parameters
//		role	: [in] The role to release from memory
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeRole(GSBRole *role);

// gsbFreeRoleIdList
// Summary
//		Deallocates memory for a list of roles ids(deep deallocation)
// Parameters
//		roleIdList	: [in] The list of roles ids to release from memeory
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeRoleIdList(GSBRoleIdList *roleIdList);


// gsbFreeBrigade
// Summary
//		Deallocates memory for a brigade(deep deallocation)
// Parameters
//		brigade	: [in] The brigade to release from memory.
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
// Remarks
//		Make sure you use this to when done using the GSBBrigade object.
//		Failure to use this may lead to memory leaks.
GSResult gsbFreeBrigade(GSBBrigade *brigade);

// gsbFreeBrigadeList
// Summary
//		Deallocates memory for a brigade list(deep deallocation)
// Parameters
//		brigadeList	: [in] The list of brigades to release from memory.
// Returns
//		GSResult -
//			GS_SUCCESS on success.
//
GSResult gsbFreeBrigadeList(GSBBrigadeList *brigadeList);

// gsbFreeEntitlementList
// Summary
//		Deallocates memory for a list of gsbFreeEntitlements(deep deallocation)
// Parameters
//		entitlementList	: [in] The list of entitlements to release from memory.
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeEntitlementList(GSBEntitlementList *entitlementList);

// gsbFreeEntitlementIdList
// Summary
//		Deallocates memory for a list of entitlement IDs(deep deallocation)
// Parameters
//		entitlementIdList	: [in] The list of entitlement IDs 
//									to release from memory.
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeEntitlementIdList(GSBEntitlementIdList *entitlementIdList);

// gsbFreeBrigadeHistoryList
// Summary
//		Deallocates memory for a list of history entries(deep deallocation)
// Parameters
//		historyList	: [in] The history list to release from memory.
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeBrigadeHistoryList(GSBBrigadeHistoryList *historyList);

// gsbFreeFilterList
// Summary
//		Deallocates memory for a list of filter entries(deep deallocation) 
//      allocated for the search functions.
// Parameters
//		filterList	: [in] The filter list to release from memory.
// Returns
//		GSResult - 
//			GS_SUCCESS on success.
//
GSResult gsbFreeFilterList(GSBSearchFilterList *filterList);

// NOTE: The following function should only be used for testing purposes!
GSResult gsbSetBaseServiceUrl(GSBInstancePtr theInstance, const char *baseServiceUrl);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__BRIGADES_H__
