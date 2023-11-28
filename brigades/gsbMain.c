///////////////////////////////////////////////////////////////////////////////
// File:	gsbMain.c
// SDK:		GameSpy Brigades SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited. 


///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "brigades.h"
#include "gsbMain.h"
#include "gsbServices.h"
#include "gsbUtil.h"
#include "../common/gsAvailable.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbInitialize(GSBInstancePtr           *theInstanceOut,
                       gsi_u32                  theGameId,
                       const GSLoginCertificate *theCertificate, 
                       const GSLoginPrivateData *thePrivateData)
{
    GSBInternalInstance *instance = NULL;

	GSB_ASSERT_CHECK_PARAM(theInstanceOut, "gsbInitialize: theInstanceOut is NULL.");
	GSB_ASSERT_CHECK_PARAM(theCertificate, "gsbInitialize: theCertificate is NULL.");
	GSB_ASSERT_CHECK_PARAM(thePrivateData, "gsbInitialize: thePrivateData is NULL.");
    GSB_CHECKAVAIL();

	GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_Debug, "Creating Brigades SDK instance");

    instance = (GSBInternalInstance*)gsimalloc(sizeof(GSBInternalInstance));
    if (instance == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate instance");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	memset(instance, 0, sizeof(GSBInternalInstance));
    instance->mGameId = theGameId;
	memcpy(&instance->mCertificate, theCertificate, sizeof(GSLoginCertificate));
    memcpy(&instance->mPrivateData, thePrivateData, sizeof(GSLoginPrivateData));
	
    *theInstanceOut = instance;

    return GS_SUCCESS;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbShutdown(GSBInstancePtr theInstance)
{
	GSBInternalInstance * instance = (GSBInternalInstance*)theInstance;
	
    GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");

	GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_Debug, "Destroying brigades interface");

    gsifree(instance);

    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbSaveBrigade(GSBInstancePtr			theInstance,
						GSBBrigade				*theBrigade, 
						GSBSaveBrigadeCallback	callback, 
						void					*userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;
	
	GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(theBrigade, "NULL GSBrigade input.");
    GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
	result = gsbiServiceUpdateBrigade(instance, theBrigade, callback, userData);
	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbDisbandBrigade(GSBInstancePtr				theInstance,
						   gsi_u32						brigadeId,
						   GSBDisbandBrigadeCallback	callback, 
                           void							*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbDisbandBrigade: NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbDisbandBrigade: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
	result = gsbiServiceDisbandBrigade(instance, brigadeId, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetBrigadeById(GSBInstancePtr				theInstance,
                           gsi_u32						brigadeId, 
                           GSBGetBrigadeByIdCallback	callback, 
                           void							*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbGetBrigadeById: NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(brigadeId != 0, "gsbGetBrigadeById: brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbGetBrigadeById: NULL callback input.");
    GSB_CHECKAVAIL();

	
    // do the web service call
    result = gsbiServiceGetBrigadeById(instance, brigadeId, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called by entitled role member to invite a player to be a role member
GSResult gsbInviteToBrigade(GSBInstancePtr              theInstance,
							    gsi_u32					brigadeId, 
                                gsi_u32					profileIdToInvite, 
                                GSBPerformBrigadeMemberActionCallback callback, 
                                void                    *userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbInviteToBrigade: NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbInviteToBrigade: brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(profileIdToInvite!=0, "gsbInviteToBrigade: profileIdToInvite = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbInviteToBrigade: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
	//instance->mBrigadeId=brigadeId;
    result = gsbiServiceInviteToBrigade(instance, brigadeId, profileIdToInvite, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called by entitled role member to cancel an invitation
GSResult gsbRescindInvite(GSBInstancePtr            theInstance,
							gsi_u32					brigadeId, 
							gsi_u32					profileIdToRescindInvite, 
							GSBPerformBrigadeMemberActionCallback callback, 
							void                    *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbRescindInvite: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbRescindInvite: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(profileIdToRescindInvite!=0, "gsbRescindInvite: profileIdToRescindInvite = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbRescindInvite: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	//instance->mBrigadeId=brigadeId;
	result = gsbiServiceRescindInvite(instance, brigadeId, profileIdToRescindInvite, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called by player to acceptJoin/decline the invitation to a role
GSResult gsbAnswerInvite(GSBInstancePtr				theInstance,
						    gsi_u32					brigadeId,
							gsi_bool				acceptInvite,
							GSBPerformBrigadeMemberActionCallback callback, 
							void					*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbAnswerInvite: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbAnswerInvite: brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbAnswerInvite: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceAnswerInvite(instance, brigadeId, acceptInvite, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbJoinBrigade(GSBInstancePtr					theInstance,
						gsi_u32							brigadeId, 
						GSBPerformBrigadeMemberActionCallback callback, 
						void							*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbJoinBrigade: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbJoinBrigade: brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbJoinBrigade: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceJoinBrigade(instance, brigadeId, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called by entitled role member to accept/decline the player requesting to join role
GSResult gsbAnswerJoin(GSBInstancePtr           theInstance,
					   gsi_u32					brigadeId,
					   gsi_u32					playerProfileId,
					   gsi_bool					acceptJoin,
					   GSBPerformBrigadeMemberActionCallback    callback, 
					   void                     *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbAnswerJoin: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbAnswerJoin: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(playerProfileId!=0, "gsbAnswerJoin: playerProfileId = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbAnswerJoin: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceAnswerJoin(instance, brigadeId, playerProfileId, acceptJoin, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbRemoveBrigadeMember(GSBInstancePtr		theInstance,
						 gsi_u32					brigadeId,
						 gsi_u32					profileIdToRemove,
						 GSBPerformBrigadeMemberActionCallback	callback, 
						 void						*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbRemoveBrigadeMember: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbRemoveBrigadeMember: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(profileIdToRemove!=0, "gsbRemoveBrigadeMember: profileIdToRemove = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbRemoveBrigadeMember: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceRemoveBrigadeMember(instance, brigadeId, profileIdToRemove, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbBanMember(GSBInstancePtr                    theInstance,
					  gsi_u32							brigadeId,
					  gsi_u32							profileIdToBan, 
					  GSBPerformBrigadeMemberActionCallback  callback, 
					  void								*userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbBanMember: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbBanMember: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(profileIdToBan!=0, "gsbBanMember: profileIdToBan = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbBanMember: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceBanMember(instance, brigadeId, profileIdToBan, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbLeaveBrigade(GSBInstancePtr					theInstance,
						 gsi_u32						brigadeId, 
						 GSBPerformBrigadeMemberActionCallback  callback, 
						 void							*userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbLeaveBrigade: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbLeaveBrigade: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbLeaveBrigade: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceLeaveBrigade(instance, brigadeId, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbPromoteToLeader(GSBInstancePtr				theInstance,
                            gsi_u32						brigadeId, 
                            gsi_u32                     memberId, 
                            GSBPromoteToLeaderCallback  callback,
                            void						*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(memberId!=0, "memberId = 0.");

    GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServicePromoteToLeader(instance, brigadeId, memberId, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// gsbGetEntitlementList
// Summary
//		Requests the list of entitlements available for the current game
// Parameters
//		theInstance : [in] An instance of the SDK. Usually required for most functions.
//		callback    : [in] The developer callback 
//		userData    : [in] A pointer to the developer's data
// Return
//		GSResult
//			GSBResultCode_NotFound
// Remarks
//		Every parameter is required except userData. The parameter theInstance needs to be initialized.
//      The callback will be called once the operation is complete.  
//		The resultSet in the callback can contain one of these values:
//		
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetEntitlementList( GSBInstancePtr         theInstance,
								GSBGetEntitlementListCallback callback, 
								void                   *userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "gsbGetEntitlementList: NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbGetEntitlementList: NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceGetEntitlementList(instance, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetEntitlementIdListByRoleId(GSBInstancePtr	theInstance,
										   gsi_u32			brigadeId,
										   gsi_u32			roleId,
										   GSBGetEntitlementIdListByRoleIdCallback   callback, 
										   void				*userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbGetEntitlementIdListByRoleId: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId != 0, "brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(roleId != 0, "roleId = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbGetEntitlementIdListByRoleId: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceGetEntitlementIdListByRoleId(instance, brigadeId, roleId, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetRoleIdListByEntitlementId(GSBInstancePtr               theInstance,
										   gsi_u32                             brigadeId,
										   gsi_u32                             entitlementId,
										   GSBGetRoleIdListByEntitlementIdCallback   callback, 
										   void                                *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbGetRoleIdListByEntitlementId: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId != 0, "brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(entitlementId != 0, "entitlementId = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbGetRoleIdListByEntitlementId: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceGetRoleIdListByEntitlementId(instance, brigadeId, entitlementId, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetMyPendingInvitesAndJoins(GSBInstancePtr                   theInstance,
									  GSBGetPendingInvitesAndJoinsCallback   callback, 
									  void                             *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbGetMyPendingInvitesAndJoins: NULL instance input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceGetPendingInvitesAndJoins(instance, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetRoleList( GSBInstancePtr         theInstance,
						 gsi_u32				brigadeId,
                         GSBGetRoleListCallback callback, 
                         void                   *userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(brigadeId != 0, "brigadeId = 0.");
    GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceGetRoleList(instance, brigadeId, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "gsbiServiceGetRoleList failed to complete");
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetBrigadeMemberList( GSBInstancePtr					theInstance,
                                  gsi_u32							brigadeId, 
								  gsi_u32							statuses,
                                  GSBGetBrigadeMemberListCallback	callback, 
                                  void								*userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(brigadeId != 0, "brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(statuses < GSBBrigadeMemberStatus_Max, "invalid statuses.");
    GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceGetBrigadeMemberList(instance, brigadeId, statuses, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbUpdateBrigadeMember(GSBInstancePtr            theInstance,
                          GSBBrigadeMember                *theMember, 
                          GSBUpdateBrigadeMemberCallback  callback, 
                          void                      *userData)
{
    GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
    GSResult result = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(theMember, "NULL GSBrigadeMember input.");
    GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

    // do the web service call
    result = gsbiServiceUpdateBrigadeMember(instance, theMember, callback, userData);

    GSI_UNUSED(instance);
    GSI_UNUSED(userData);
    GSI_UNUSED(callback);

    if (GS_FAILED(result))
    {
		GSB_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "gsbiServiceUpdateBrigadeMember failed to complete");
        return result;
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCreateRole(GSBInstancePtr         theInstance,
                       GSBRole                *theRole,
					   GSBEntitlementIdList	  *entitlementIdList,
                       GSBUpdateRoleCallback  callback, 
					   void                   *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;
	
    GSB_ASSERT_CHECK_PARAM(instance, "gsbCreateRole: NULL instance input.");
    GSB_ASSERT_CHECK_PARAM(theRole, "gsbCreateRole: NULL GSBRole input.");
	GSB_ASSERT_CHECK_PARAM(entitlementIdList, "gsbCreateRole: NULL GSBEntitlementList input.");

	GSB_ASSERT_CHECK_PARAM(theRole->mBrigadeId!=0, "gsbCreateRole: brigadeId in GSBRole = 0.");
	GSB_ASSERT_CHECK_PARAM(theRole->mRoleName!=NULL, "gsbCreateRole: NULL roleName in GSBRole.");
    GSB_ASSERT_CHECK_PARAM(callback, "gsbCreateRole: NULL callback input.");
    GSB_CHECKAVAIL();

	// ensure roleId = 0 when creating new role
	theRole->mRoleId = 0;

    // do the web service call
	result = gsbiServiceUpdateRole(instance, theRole, entitlementIdList, callback, userData);


	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbUpdateRole(GSBInstancePtr         theInstance,
					   GSBRole                *theRole,
					   GSBEntitlementIdList	  *entitlementIdList,
					   GSBUpdateRoleCallback  callback, 
					   void                   *userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbUpdateRole: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(theRole, "gsbUpdateRole: NULL GSBRole input.");
	GSB_ASSERT_CHECK_PARAM(entitlementIdList, "gsbUpdateRole: NULL GSBEntitlementIdList input.");

	GSB_ASSERT_CHECK_PARAM(theRole->mBrigadeId!=0, "gsbUpdateRole: brigadeId in GSBRole = 0.");
	GSB_ASSERT_CHECK_PARAM(theRole->mRoleId!=0, "gsbUpdateRole: roleId in GSBRole = 0.");
	GSB_ASSERT_CHECK_PARAM(theRole->mRoleName!=NULL, "gsbUpdateRole: NULL roleName in GSBRole.");
    GSB_ASSERT_CHECK_PARAM(entitlementIdList->mEntitlementIds != NULL,"gsbUpdateRole: NULL EntitlemetID in the entitlemetIdList."); 
	GSB_ASSERT_CHECK_PARAM(callback, "gsbUpdateRole: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceUpdateRole(instance, theRole, entitlementIdList, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbRemoveRole(GSBInstancePtr			theInstance,
					   gsi_u32					brigadeId,
					   gsi_u32					roleId, 
					   GSBRemoveRoleCallback	callback, 
					   void						*userData)
{
	GSBInternalInstance * instance   = (GSBInternalInstance*)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "gsbRemoveRole: NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId!=0, "gsbRemoveRole: brigadeId = 0.");
	GSB_ASSERT_CHECK_PARAM(roleId!=0, "gsbRemoveRole: roleId  = 0.");
	GSB_ASSERT_CHECK_PARAM(callback, "gsbRemoveRole: NULL callback input.");
	GSB_CHECKAVAIL();

	// do the web service call
	result = gsbiServiceRemoveRole(instance, brigadeId, roleId, callback, userData);

	GSI_UNUSED(instance);
	GSI_UNUSED(userData);
	GSI_UNUSED(callback);

	if (GS_FAILED(result))
	{
		// TODO: better error handling/reporting
		return result;
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbUploadLogo(GSBInstancePtr					theInstance, 
					   const gsi_char					*logoFileName, 
					   gsi_u32							brigadeId, 
					   GSBUploadLogoCompleteCallback	callback, 
					   void								*userData)
{
	GSBInternalInstance *instance = (GSBInternalInstance *)theInstance;
	GSResult result = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(logoFileName, "NULL logoFileName input.");
	GSB_ASSERT_CHECK_PARAM(brigadeId, "Zero brigadeid input.");
	GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

	result = gsbiStartUploadRequest(instance, logoFileName, brigadeId, callback, userData);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbDownloadLogo(GSBInstancePtr						theInstance, 
						 const gsi_char						*logoFileName,
						 GSBBrigadeLogo						*logo,
						 GSBDownloadLogoCompleteCallback	callback,
						 void								*userData)
{
	GSBInternalInstance *instance = (GSBInternalInstance *)theInstance;

	GSB_ASSERT_CHECK_PARAM(instance, "NULL instance input.");
	GSB_ASSERT_CHECK_PARAM(logo, "NULL logo input.");
	GSB_ASSERT_CHECK_PARAM(callback, "NULL callback input.");
    GSB_CHECKAVAIL();

	return gsbiStartDownloadRequest(instance, logoFileName, logo, callback, userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneBrigadeMemberList(GSBBrigadeMemberList  **destList, GSBBrigadeMemberList  *srcList)
{
	GSBBrigadeMemberList *memberList;
	gsi_u32 i, j;
	GSResult result = GS_SUCCESS;

	*destList = NULL;

	GSB_ASSERT_CHECK_PARAM(destList, "NULL destList output");
	GSB_ASSERT_CHECK_PARAM(srcList, "NULL srcList output");

	// allocate a GSBBrigadeMemberList
	memberList = (GSBBrigadeMemberList *)gsimalloc(sizeof(GSBBrigadeMemberList));
	if (memberList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"Failed on gsimalloc for GSBBrigadeMemberList");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	// set the count for the above
	memberList->mCount=srcList->mCount;

	// allocate GSBBrigadeMember * count from above
	memberList->mBrigadeMembers = gsimalloc ( sizeof( GSBBrigadeMember) * memberList->mCount );
	if (memberList->mBrigadeMembers == NULL)
	{
		gsifree(memberList);

		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"Failed on gsimalloc of %d GSBBrigadeMember(s)", memberList->mCount);
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	// go through the list
	//	and make a deep copy of all pointers, numbers
	//  and make a deep copy of the strings via strdup or 

	for (i=0; i<srcList->mCount; i++) 
	{
		result = gsbiCloneBrigadeMemberContents(&memberList->mBrigadeMembers[i],&srcList->mBrigadeMembers[i]);
		if( result != GS_SUCCESS ) break;
	}

	// if error during copy, gsifree allocated memory so far
	if( result != GS_SUCCESS )
	{
		*destList = NULL;

		for(j=0; j < i; j++) 
		{
			gsbFreeBrigadeMember( &memberList->mBrigadeMembers[j]);
		}

		gsifree(memberList->mBrigadeMembers);
		gsifree(memberList);

		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"Failed on gsimalloc");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	*destList=memberList;
	return GS_SUCCESS;
}


GSResult gsbCloneBrigadeMember(GSBBrigadeMember **destMember, GSBBrigadeMember *srcMember)
{
	GSResult result;

	GSB_ASSERT_CHECK_PARAM(destMember, "NULL destMember output");
	GSB_ASSERT_CHECK_PARAM(srcMember, "NULL srcMember input");

	*destMember = (GSBBrigadeMember *)gsimalloc(sizeof(GSBBrigadeMember));
	if (*destMember == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate GSBBrigadeMember");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	result = gsbiCloneBrigadeMemberContents(*destMember, srcMember);

	if(result != GS_SUCCESS)
	{
		gsifree(*destMember);

		*destMember = NULL;

		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate GSBBrigadeMember");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeBrigadeMember(GSBBrigadeMember *member)
{
    if (member)
    {
        gsbiFreeBrigadeMember(member);
    }
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeBrigadeMemberList(GSBBrigadeMemberList *memberList)
{
    if (memberList)
    {
        gsbiFreeBrigadeMemberList(memberList);
    }

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreePendingActionsList(GSBBrigadePendingActionsList *actionList)
{
    if (actionList)
    {
        gsbiFreePendingActionsList(actionList);
	}
	return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeRoleList(GSBRoleList *roleList)
{
	if (roleList)
	{
		gsbiFreeRoleList(roleList);
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeRole(GSBRole *role)
{
    if (role)
    {
        gsbiFreeRole(role);
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneRole(GSBRole **destRole, GSBRole *srcRole)
{
    GSBRole *role = NULL;

    GSB_ASSERT_CHECK_PARAM(destRole, "NULL destRole output");
    GSB_ASSERT_CHECK_PARAM(srcRole, "NULL srcRole input");

    role = (GSBRole *)gsimalloc(sizeof(GSBRole));
    if (role == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBBrigade");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    memset(role, 0, sizeof(GSBRole));

    role->mBrigadeId = srcRole->mBrigadeId;
    role->mIsDefault = srcRole->mIsDefault;
    role->mIsGameRole = srcRole->mIsGameRole;
    role->mRoleId = srcRole->mRoleId;
    
    role->mRoleName = goawstrdup(srcRole->mRoleName);
    *destRole = role;
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeBrigadeHistoryList(GSBBrigadeHistoryList *historyList)
{
    if (historyList)
    {    
        gsbiFreeBrigadeHistoryList(historyList);
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeBrigade(GSBBrigade *brigade)
{
	if (brigade)
	{
		gsbiFreeBrigadeContents(brigade);
        gsifree(brigade);
	}
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeBrigadeList(GSBBrigadeList *brigadeList)
{
    if (brigadeList)
    {
        gsbiFreeBrigadeList(brigadeList);
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneBrigade(GSBBrigade **destBrigade, GSBBrigade *srcBrigade)
{
    GSBBrigade *brigade = NULL;

    GSB_ASSERT_CHECK_PARAM(destBrigade, "NULL destBrigade output");
    GSB_ASSERT_CHECK_PARAM(srcBrigade, "NULL srcBrigade input");
    
    brigade = (GSBBrigade *)gsimalloc(sizeof(GSBBrigade));
    if (brigade == NULL)
    {
        GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBBrigade");
        return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    memset(brigade, 0, sizeof(GSBBrigade));

    brigade->mBrigadeId = srcBrigade->mBrigadeId;
    brigade->mCreatorProfileId = srcBrigade->mCreatorProfileId;
    brigade->mDisbandDate = srcBrigade->mDisbandDate;
    brigade->mDisbanded = srcBrigade->mDisbanded;
    brigade->mGameId = srcBrigade->mGameId;
    brigade->mLeaderProfileId = srcBrigade->mLeaderProfileId;
	brigade->mRecruitingType = srcBrigade->mRecruitingType;

    gsbiCloneBrigadeLogoList(&brigade->mLogoList, &srcBrigade->mLogoList);
    brigade->mMessageOfTheDay = goawstrdup(srcBrigade->mMessageOfTheDay);
    brigade->mName = goawstrdup(srcBrigade->mName);    
    brigade->mTag = goawstrdup(srcBrigade->mTag);
    brigade->mUrl = goawstrdup(srcBrigade->mUrl);

    *destBrigade = brigade;
    return GS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbSetBaseServiceUrl(GSBInstancePtr theInstance, const char *baseServiceUrl)
{
	size_t baseServiceUrlLen;
	GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;

	GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
	GSB_ASSERT_CHECK_PARAM(baseServiceUrl, "serviceUrl is NULL.");
	baseServiceUrlLen = strlen(baseServiceUrl);
	GSB_ASSERT_CHECK_PARAM(baseServiceUrlLen > 0, "serviceUrl cannot be empty");
	GSB_ASSERT_CHECK_PARAM(baseServiceUrlLen < GSB_SERVICE_MAX_URL_BASE_LEN, "serviceUrl length is longer than %d, actual: %d", GSB_SERVICE_MAX_URL_BASE_LEN, baseServiceUrlLen);

	gsiSafeStrcpyA(instance->mBaseServiceURL, baseServiceUrl, strlen(baseServiceUrl) + 1);
	return GS_SUCCESS;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbSearchBrigades(GSBInstancePtr				theInstance,
						   GSBSearchFilterList			*filterList, 
						   GSBSearchBrigadesCallback	callback, 
						   void							*userData)
{
	GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
	GSResult            result    = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
	GSB_ASSERT_CHECK_PARAM(filterList, "filter is NULL.");
	GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

	// validate the filter here prior to sending to the service function
	result = gsbiValidateBrigadeSearchFilter(filterList);
	if (GS_SUCCEEDED(result))
	{
		result = gsbiServiceSearchBrigades(instance, filterList, callback, userData);
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeFilterList(GSBSearchFilterList *filterList)
{
    if (filterList)
    {
        gsbiFreeFilterList(filterList);
    }
    return GS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbSearchPlayers(GSBInstancePtr				theInstance,
						  GSBSearchFilterList			*filterList, 
						  GSBSearchPlayersCallback		callback, 
						  void							*userData)
{
	GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
	GSResult            result    = GS_SUCCESS;

	GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
	GSB_ASSERT_CHECK_PARAM(filterList, "filter is NULL.");
	GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

	// validate the filter here prior to sending to the service function
	result = gsbiValidateMemberSearchFilter(filterList);
	if (GS_SUCCEEDED(result))
	{
		result = gsbiServiceSearchPlayers(instance, filterList, callback, userData);
	}

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbSendMessageToBrigade(GSBInstancePtr	theInstance, 
                            gsi_u32				brigadeId, 
                            UCS2String			message,
                            GSBSendMessageToBrigadeCallback callback, 
                            void				*userData)
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult	result    = GS_SUCCESS;
    gsi_u32		messageSize = (gsi_u32)(wcslen(message) * sizeof(UCS2Char));

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(brigadeId,   "Invalid Brigade Id");
    GSB_ASSERT_CHECK_PARAM(message,     "Invalid Message");
    GSB_ASSERT_CHECK_PARAM( (messageSize <= GSB_SEND_MESSAGE_MAX_LEN), "Message too long");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

    // validate the filter here prior to sending to the service function
    result = gsbiServiceSendMessageToBrigade(instance, brigadeId, message, callback, userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GSResult gsbSendMessageToMember(GSBInstancePtr	theInstance, 
                                gsi_u32			toProfileId,
                                UCS2String		message,
                                GSBSendMessageToMemberCallback callback, 
                                void			*userData)
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult            result    = GS_SUCCESS;
    gsi_u32                 messageSize = 0;

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(toProfileId,   "Receiver Profile Id is invalid");
    GSB_ASSERT_CHECK_PARAM(message,     "Invalid Message");
    GSB_CHECKAVAIL();

    messageSize = (gsi_u32) (wcslen((const wchar_t*)message) * sizeof(UCS2Char));
    GSB_ASSERT_CHECK_PARAM( (messageSize <= GSB_SEND_MESSAGE_MAX_LEN), "Message too long");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    
    // validate the filter here prior to sending to the service function
    result = gsbiServiceSendMessageToMember(instance, toProfileId, message, callback, userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetBrigadesByProfileId(GSBInstancePtr	theInstance, 
                                   gsi_u32			profileId, 
                                   GSBGetBrigadesByProfileIdCallback callback, 
                                   void				*userData)
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult            result    = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(profileId,"Origination Profile Id is invalid");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

    // validate the filter here prior to sending to the service function
    result = gsbiServiceGetBrigadesByProfileId( instance,  profileId,  callback,  userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetBrigadeHistory(GSBInstancePtr				theInstance, 
                              gsi_u32						brigadeId, 
                              gsi_u32						profileId,
                              GSBBrigadeHistoryAccessLevel	historyAccessLevel,
                              GSBGetBrigadeHistoryCallback	callback, 
                              void							*userData)
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult            result    = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(brigadeId,"Origination Profile Id is invalid");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

    // validate the filter here prior to sending to the service function
    result = gsbiServiceGetBrigadeHistory( instance,  brigadeId, profileId,  historyAccessLevel, callback,  userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbGetBrigadeMatchHistory(GSBInstancePtr	theInstance, 
                              gsi_u32				matchId, 
                              GSBGetBrigadeMatchHistoryCallback callback, 
                              void					*userData)
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult            result    = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(matchId,"Origination Profile Id is invalid");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

    // validate the filter here prior to sending to the service function
    result = gsbiServiceGetBrigadeMatchHistory( instance,  matchId, callback,  userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbUpdateMemberEmailAndNick( GSBInstancePtr theInstance, gsi_u32 brigadeId, GSBUpdateMemberEmailAndNickCallback callback, void *userData )
{
    GSBInternalInstance *instance = (GSBInternalInstance*)theInstance;
    GSResult            result    = GS_SUCCESS;

    GSB_ASSERT_CHECK_PARAM(instance, "theInstance is NULL.");
    GSB_ASSERT_CHECK_PARAM(callback, "callback is NULL.");
    GSB_CHECKAVAIL();

    // validate the filter here prior to sending to the service function
    result = gsbiServiceUpdateMemberEmailAndNick( instance, brigadeId, callback,  userData);

    GSI_UNUSED(userData);

    if (GS_FAILED(result))
    {
        // TODO: better error handling/reporting
        return result;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneEntitlementList(GSBEntitlementList **destList, 
                                 GSBEntitlementList *srcList)
{
	GSResult result = GS_SUCCESS;

    GSBEntitlementList *entitlementList;
    gsi_u32 i;

	GSB_ASSERT_CHECK_PARAM(destList, "gsbCloneEntitlementList: NULL destList output");
    GSB_ASSERT_CHECK_PARAM(srcList, "gsbCloneEntitlementList: NULL srcList output");

	*destList = NULL;

    // allocate a GSBEntitlementList
    entitlementList = (GSBEntitlementList *)gsimalloc(sizeof(GSBEntitlementList));
	if(entitlementList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBEntitlementList");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    // set the count
    entitlementList->mCount = srcList->mCount;

    // allocate GSBEntitlement * count
    entitlementList->mEntitlements = (GSBEntitlement *) gsimalloc ( sizeof( GSBEntitlement) * entitlementList->mCount );
	if(entitlementList->mEntitlements == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBEntitlements");

		gsifree(entitlementList);
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

    // go through the list
    //	and make a deep copy of all pointers, numbers
    //  and make a deep copy of the strings via strdup or 

    for (i=0; i < srcList->mCount; i++) 
    {
		result = gsbCloneEntitlement(&entitlementList->mEntitlements[i], &srcList->mEntitlements[i]);
        if (result != GS_SUCCESS)
			break;
    }

	// cleanup on error
	if(result != GS_SUCCESS)
	{
		entitlementList->mCount = i;
		gsbiFreeEntitlementList(entitlementList);
		
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBEntitlements");
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	*destList=entitlementList;

    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneEntitlement(GSBEntitlement *destEntitlement, GSBEntitlement *srcEntitlement)
{
    GSB_ASSERT_CHECK_PARAM(destEntitlement, "gsbCloneEntitlement: NULL destEntitlement output");
    GSB_ASSERT_CHECK_PARAM(srcEntitlement, "gsbCloneEntitlement: NULL srcEntitlement input");

    return gsbiCloneEntitlement(destEntitlement, srcEntitlement);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeEntitlementList(GSBEntitlementList *entitlementList)
{
    //GSB_ASSERT_CHECK_PARAM(entitlementList, "gsbFreeEntitlementList: NULL member input");
    if (entitlementList)
    {
        gsbiFreeEntitlementList(entitlementList);
    }
    return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbCloneEntitlementIdList(GSBEntitlementIdList **destList, GSBEntitlementIdList *srcList)
{
	GSBEntitlementIdList *entitlementIdList;
	gsi_u32 i;

	GSB_ASSERT_CHECK_PARAM(destList, "gsbCloneEntitlementIdList: NULL destList output");
	GSB_ASSERT_CHECK_PARAM(srcList, "gsbCloneEntitlementIdList: NULL srcList output");

	*destList = NULL;

	// allocate a GSBEntitlementIdList
	entitlementIdList = (GSBEntitlementIdList *)gsimalloc(sizeof(GSBEntitlementIdList));
	if(entitlementIdList == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBEntitlementIdList");

		gsifree(entitlementIdList);
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	// set the count 
	entitlementIdList->mCount = srcList->mCount;

	// allocate gsi_u32 * count 
	entitlementIdList->mEntitlementIds = (gsi_u32 *) gsimalloc ( sizeof( gsi_u32) * entitlementIdList->mCount );
	if(entitlementIdList->mEntitlementIds == NULL)
	{
		GSB_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "Failed to allocate memory for GSBEntitlementIds");

		gsifree(entitlementIdList);
		return GSB_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
	}

	// go through the list and set IDs
	for (i=0; i < srcList->mCount; i++) 
	{
		entitlementIdList->mEntitlementIds[i] = srcList->mEntitlementIds[i];
	}

	*destList=entitlementIdList;

	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeEntitlementIdList(GSBEntitlementIdList *entitlementIdList)
{
    if (entitlementIdList)
    {
        gsbiFreeEntitlementIdList(entitlementIdList);
    }	
	return GS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult gsbFreeRoleIdList(GSBRoleIdList *roleIdList)
{
    if (roleIdList)
    {
        gsbiFreeRoleIdList(roleIdList);
    }	

    return GS_SUCCESS;
}
