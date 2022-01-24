///////////////////////////////////////////////////////////////////////////////
// File:	gpiPS3.c
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifdef _PS3

//INCLUDES
#include <stdlib.h>
#include <string.h>
#include "gpi.h"
#include <sysutil/sysutil_common.h>

//GLOBALS
uint8_t	gpi_np_pool[SCE_NP_MIN_POOL_SIZE];

//FUNCTIONS
void gpiNpManagerCallback(int incomingEvent,
						  int result,
						  void *arg)
{
    GPConnection * connection = (GPConnection*)arg;
    GPIConnection * iconnection = (GPIConnection*)*connection;
	GS_ASSERT(iconnection->npInitialized);
	if (!iconnection->npInitialized)
		return;

    // If game doesn't utilize NP, have OUR manager callback check for NP 
	// disconnect in-game.
    switch (incomingEvent)
    {
        case SCE_NP_MANAGER_STATUS_OFFLINE:
            // If we got online earlier, that means we got disconnected 
			// in-game, so mark for cleanup of NP.
            if (iconnection->npStatusRetrieved)
            {
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Network, GSIDebugLevel_HotError,
                    "gpiNpManagerCallback: NP Disconnected, marking NP for cleanup\n");
                iconnection->npDisconnected = gsi_true;
            }
            break;
        default:
            break; // Ignore everything else.
    }

}

void gpiNpCellSysutilCallback(uint64_t status,
							  uint64_t param,
							  void * userdata)
{
	GPIConnection *iconnection = (GPIConnection *)userdata;
	
	// We just handle the in-game xmb closed callback, triggering a re-sync for 
	// the buddy/block lists.
	switch(status)
	{
		case CELL_SYSUTIL_SYSTEM_MENU_CLOSE:
		{
			if (!iconnection->npPerformBlockSync)
				iconnection->npPerformBlockSync = gsi_true;
			if (!iconnection->npPerformBuddySync)
				iconnection->npPerformBuddySync = gsi_true;
		}
	}

}

int gpiNpBasicCallback(int incomingEvent, 
					   int retCode, 
					   uint32_t reqId, 
					   void *arg)
{
	GPIConnection *iconnection = (GPIConnection *)arg;
	int retValue;

	// We just handle friend or block list updates.
	switch (incomingEvent)
	{
		case SCE_NP_BASIC_EVENT_FRIEND_REMOVED:
		case SCE_NP_BASIC_EVENT_OFFLINE:	// These are called for
		case SCE_NP_BASIC_EVENT_PRESENCE:	// added friends.
		{
			if (!iconnection->npPerformBuddySync)
				iconnection->npPerformBuddySync = gsi_true;
			break;
		}
		case SCE_NP_BASIC_EVENT_END_OF_INITIAL_PRESENCE: // Tells us all NP friend updates have propagated down.
		{ 
			iconnection->npFriendListRetrieved = gsi_true;
			break;
		}
		case SCE_NP_BASIC_EVENT_ADD_BLOCKLIST_RESULT: 
		{
			iconnection->npBlockListRetrieved = gsi_true;
			if (!iconnection->npPerformBlockSync)
				iconnection->npPerformBlockSync = gsi_true;
			break;
		}
	}

	// Pass event on to developer registered callback.
	if (iconnection->npEventCallback.callback)
		retValue = iconnection->npEventCallback.callback(incomingEvent, retCode, reqId, iconnection->npEventCallback.userData);
	else // If developer didn't register callback, we are in charge of freeing the np basic event queue.
	{
		iconnection->npBasicEventsToGet++; // Keep track of the number of events to free.
	}
	
	return 0;	// Sony docs - "Be sure to return 0 when exiting from the handler".
}

GPResult gpiInitializeNpBasic(GPConnection * connection)
{
	SceNpCommunicationId emptyId;
	int ret = 0;
    GPIConnection * iconnection = (GPIConnection*)*connection;
	
	memset(&emptyId, 0, sizeof(emptyId));
	if (memcmp(&iconnection->npCommunicationId, &emptyId, sizeof(emptyId)) == 0)
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError, 
			"gpiInitializeNpBasic: Developer failed to initialize SceNpCommunicationId using gpSetNpCommunicationId!");
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "Developer failed to initialize SceNpCommunicationId using gpSetNpCommunicationId!");
	}

	GS_ASSERT(iconnection->npCellSysUtilSlotNum >= 0 || iconnection->npCellSysUtilSlotNum <= 3);
	if (iconnection->npCellSysUtilSlotNum < 0 || iconnection->npCellSysUtilSlotNum > 3)
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError, 
			"Developer failed to initialize Cell Sys Util Callback slot number using gpRegisterCellSysUtilCallback!");
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, 
			"Developer failed to initialize Cell Sys Util Callback slot number using gpRegisterCellSysUtilCallback!");
	}
	ret = cellSysutilRegisterCallback(iconnection->npCellSysUtilSlotNum, gpiNpCellSysutilCallback, iconnection);
	if (ret != CELL_OK)
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError, 
			"Failed to register CellSysUtil Callback, NP-BuddySync disabled!");
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, 
			"Failed to register CellSysUtil Callback, NP-BuddySync disabled!");
	}

	// Initial NP init - after this we wait for status to get to online.
	ret = sceNpInit(SCE_NP_MIN_POOL_SIZE, gpi_np_pool);

    if (ret == SCE_NP_ERROR_ALREADY_INITIALIZED)
    {
        // If already initialized - DO NOT terminate after sync (game might need it).
        iconnection->npBasicGameInitialized = gsi_true;
    }
	else if (ret < 0) 
	{
		// We don't need to clean up based on failing call to sceNpInit.
		iconnection->npBasicGameInitialized = gsi_true;
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "gpiInitializeNpBasic: sceNpInit() failed, NP-functionality disabled. ret = 0x%x\n", ret);	
        CallbackError(connection, GP_NETWORK_ERROR, GP_NETWORK, "sceNpInit() failed, NP-BuddySync disabled, please shutdown GP!");
	}
    else
    {
		// WE initialized NP, so destroy after complete.
		// The game in this case didn't do anything with NP.
        iconnection->npBasicGameInitialized = gsi_false; 

        // Initialize the NP manager callback to poll for offline status in case of in-game disconnect.
        ret = sceNpManagerRegisterCallback(gpiNpManagerCallback, connection);
        if (ret < 0) 
        {
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "gpiInitializeNpBasic: sceNpManagerRegisterCallback() failed, NP-functionality disabled. ret = 0x%x\n", ret);	
            CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "sceNpManagerRegisterCallback() failed, NP-BuddySync disabled!");
        }
    }

	iconnection->npInitialized = gsi_true;
	return GP_NO_ERROR;
}

// Freeing up transaction list darray.
void gpiNpTransactionListFree(void *element)
{
    npIdLookupTrans *aTrans = (npIdLookupTrans *)element;
    freeclear(aTrans->npIdForAdd);
}


GPResult gpiCheckNpStatus(GPConnection * connection)
{
	int ret = 0;
	int status = SCE_NP_MANAGER_STATUS_OFFLINE;
    SceNpId npId;
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Get NP status.
	ret = sceNpManagerGetStatus(&status);
	if (ret < 0) 
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"gpiCheckNpStatus: sceNpGetStatus() failed. ret = 0x%x\n", ret);	
	}
	gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
		"gpiCheckNpStatus: sceNpGetStatus - status = %d\n", status);	


	// If NP status != online after the timeout period, stop syncing.
	if (status != SCE_NP_MANAGER_STATUS_ONLINE && (current_time() - iconnection->loginTime > GPI_NP_STATUS_TIMEOUT))
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"gpiCheckNpStatus: NP Status not online - timed out\n");	
		
		// Flag to stop the sync process.
		iconnection->npPerformBuddySync = gsi_false;
        iconnection->npPerformBlockSync = gsi_false;

		//return GP_MISC_ERROR;
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "Unable to startup NP.");
	}

	// Once status is online, finish NP init.
	if (status == SCE_NP_MANAGER_STATUS_ONLINE)
	{
		iconnection->loginTime = current_time();

        // Note - we ignore error messages here - if something fails we really don't care.
        ret = sceNpBasicRegisterHandler(&iconnection->npCommunicationId, gpiNpBasicCallback, iconnection);
		if (ret < 0) 
	    {
		    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
			    "gpiCheckNpStatus: sceNpBasicRegisterHandler() failed. ret = 0x%x\n", ret);
	    }
        
        ret = sceNpLookupInit();
        if (ret == SCE_NP_COMMUNITY_ERROR_ALREADY_INITIALIZED)
        {
            // If already initialized - DO NOT terminate after GP destroy (game might need it).
            iconnection->npLookupGameInitialized = gsi_true;
        }
        else if (ret < 0) 
        {
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "gpiCheckNpStatus: sceNpLookupInit() failed. ret = 0x%x\n", ret);    
			iconnection->npLookupGameInitialized = gsi_true; // Tells you not to terminate NP after GP destroy.
			
			// Flag to stop the sync process.
			iconnection->npPerformBuddySync = gsi_false;
			iconnection->npPerformBlockSync = gsi_false;

			CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "Unable to startup NP.");
        }
        else
            iconnection->npLookupGameInitialized = gsi_false;

        // Regardless of game, create a title context id for GP to use for lookups.
        ret = sceNpManagerGetNpId(&npId);
        if (ret < 0) 
        {
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "gpiCheckNpStatus: sceNpManagerGetNpId() failed. ret = 0x%x\n", ret);  

			CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "Unable to startup NP.");
        }

        ret = sceNpLookupCreateTitleCtx(&iconnection->npCommunicationId, &npId);
        if (ret < 0) 
        {
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "gpiCheckNpStatus: sceNpLookupCreateTitleCtx() failed. ret = 0x%x\n", ret); 

			CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "Unable to startup NP.");
        }

        iconnection->npLookupTitleCtxId = ret;

		// Mark status retrieval completed.
		iconnection->npStatusRetrieved = gsi_true;
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
            "gpiCheckNpStatus: NP is now initialized with status.\n");	

        iconnection->npTransactionList = ArrayNew(sizeof(npIdLookupTrans), 1, gpiNpTransactionListFree);
        if (!iconnection->npTransactionList)
		{
			CallbackError(connection, GP_MEMORY_ERROR, GP_GENERAL, "Out of memory");
		}
	}

	return GP_NO_ERROR;
}

GPResult gpiDestroyNpBasic(GPConnection * connection)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;	

    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Network, GSIDebugLevel_Notice,
        "gpiDestroyNpBasic: Cleaning up and destroying NP (if appropriate for game)\n");

    // Do not destroy NpLookup or NpBasic if Game is using it.
    if (!iconnection->npLookupGameInitialized)
        sceNpLookupTerm(); // This will kill all title contexts created as well...

	sceNpBasicUnregisterHandler();

    if (!iconnection->npBasicGameInitialized)
    {
		sceNpManagerUnregisterCallback();

        sceNpTerm();
    }

    // Free up transaction list used for NP lookups.
    if (iconnection->npTransactionList)
        ArrayFree(iconnection->npTransactionList);

	// Reset all state variables.
	iconnection->npInitialized = gsi_false;
    iconnection->npStatusRetrieved = gsi_false;
	iconnection->npBasicGameInitialized = gsi_false;
	iconnection->npLookupGameInitialized = gsi_false;
	iconnection->npPerformBuddySync = gsi_false;
	iconnection->npPerformBlockSync = gsi_false;
	iconnection->npFriendListRetrieved = gsi_false;
	iconnection->npBlockListRetrieved = gsi_false;
	iconnection->npSyncLock = gsi_false;
    iconnection->npDisconnected = gsi_false;
	iconnection->npLookupTitleCtxId = 0;
	iconnection->npTransactionList = NULL;
	iconnection->loginTime = 0;
	iconnection->npBasicEventsToGet = 0;

	return GP_NO_ERROR;
}

GPResult gpiSyncNpBuddies(GPConnection * connection)
{
	int ret; 
	SceNpId npId;	//Buffer to store friend list entry's NP ID.
	gsi_u32 i, j, count = 0;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPResult result = GP_NO_ERROR;

	// Flag sync as complete so we don't do it more than once per login.
	iconnection->npPerformBuddySync = gsi_false;

	// Get buddy count.
	ret = sceNpBasicGetFriendListEntryCount(&count);
	if ( ret < 0 ) 
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"PS3BuddySync: Failed to get NP friend list count\n");
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
	}

	// Loop through each np friend, check for existence of GSID account.
	for (i = 0; i < count; i++) 
	{
		gsi_bool friendFound = gsi_false;
		memset(&npId, 0, sizeof(npId));
		ret = sceNpBasicGetFriendListEntry(i, &npId);
		if (ret < 0) 
		{
			gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
				"PS3BuddySync: Failed to get NP friend entry #%d\n", i);

			// After an NP friend 'remove', the count set by 
			// sceNpBasicGetFriendListEntryCount does not seem to decrement
			// accordingly, which results in 
			// SCE_NP_BASIC_ERROR_INVALID_ARGUMENT for invalid index passed to 
			// sceNpBasicGetFriendListEntry. So if this is the case we just 
			// continue instead of erroring out.
			if (ret != SCE_NP_BASIC_ERROR_INVALID_ARGUMENT)
			{
				CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
			}
			continue;
		}

		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
			"PS3BuddySync: NP friend entry #%d, npid = %s. Queueing Search.\n", i, npId.handle.data);
		
		// loop through GP buddy list and compare current np handle to each 
		// uniquenick; *if* it does not match any buddies, then proceed with 
		// profile search.
		for (j = 0; j < iconnection->profileList.numBuddies; j++)
		{
			GPIProfile *aProfile = NULL;
			GPGetInfoResponseArg infoResponse;
#ifdef GSI_UNICODE
			char uniquenickChar[GP_UNIQUENICK_LEN]; // Used for converting unicode uniquenick to ascii string.
#endif
			aProfile = gpiFindBuddy(connection, j);
			if (aProfile == NULL)
			{
				// This shouldn't happen because we always have the buddy list 
				// before we're done connecting. If it does occur we will just 
				// skip out of the 'for' loop and go straight to the profile 
				// search.
				break;
			}

			// We use a local check here to minimize backend traffic.
			result = gpiGetInfoNoWait(connection, aProfile->profileId, &infoResponse);
			if (result != GP_NO_ERROR)
			{
				continue; // This will expectedly error if the cache has not yet been written.
			}
#ifdef GSI_UNICODE
			// since strncmp needs string arguments
			UCSToAsciiString(infoResponse.uniquenick, uniquenickChar);
			if (strncmp(npId.handle.data, uniquenickChar, sizeof(npId.handle.data)) == 0)
#else
			if (strncmp(npId.handle.data, infoResponse.uniquenick, sizeof(npId.handle.data)) == 0)
#endif
			{
				friendFound = gsi_true; // We already have this friend in our GP buddy list.
				break;
			}
		}

		// Only run profile search if we don't already have this person in our
		// buddy list.
		if (!friendFound) 
		{
			result = gpiProfileSearchUniquenick(connection, npId.handle.data, &iconnection->namespaceID, 
				1, GP_NON_BLOCKING, (GPCallback)gpiSyncNpBuddiesCallback, NULL);
			if (result != GP_NO_ERROR)
			{
				CallbackError(connection, result, GP_GENERAL, "There was an error syncing NP buddies.");
			}
		}
	}

	// loop through GP buddy list and check each in the NP list (deleting those
	// who are not an NP friend).
	for (i = 0; i < iconnection->profileList.numBuddies; i++)
	{
		GPIProfile *aProfile = NULL;
		aProfile = gpiFindBuddy(connection, i);
		if (aProfile == NULL)
		{
			// This shouldn't happen because we always have the buddy list before
			// we're done connecting.
			CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
		}
		
		// The getinfo callback (gpiSyncNpDeletedBuddiesCallback) will delete 
		// GP buddies who are not NP friends.
		result = gpiGetInfo(connection, aProfile->profileId, GP_CHECK_CACHE, GP_NON_BLOCKING, (GPCallback)gpiSyncNpDeletedBuddiesCallback, NULL);
		if (result != GP_NO_ERROR)
		{
			CallbackError(connection, result, GP_GENERAL, "There was an error syncing NP buddies.");
		}
	}
	
	return result;
}

// Called for NP friends, here we make sure they are also added to our GP buddy
// list.
void gpiSyncNpBuddiesCallback(GPConnection * pconnection,
							  GPProfileSearchResponseArg * arg,
							  void * param)
{
	if(arg->result == GP_NO_ERROR)
	{
		if(arg->numMatches == 1)
		{
            // Check if already a buddy.
		    if (!gpIsBuddy(pconnection, arg->matches[0].profile))
            {
				GPResult result;
				GPIProfile *profileInfo;
				gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
					"PS3BuddySync: NP Buddy \"%s\" found in namespace %d. Sending Request.\n", 
					arg->matches[0].uniquenick, arg->matches[0].namespaceID);

                // Add this user to our buddy list locally.
				profileInfo = gpiProfileListAdd(pconnection, arg->matches[0].profile);
				if (!profileInfo)
				{
					CallbackErrorNoReturn(pconnection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
					return;
				}

				// Add this user to our buddy list symmetrically via the 
				// backend; with this function we will not have to wait for the
				// other player to accept the request.
				result = gpiSendAddBuddyRequest(pconnection, arg->matches[0].profile, "PS3 Buddy Sync", GPITrue);
				if (result != GP_NO_ERROR)
				{
					CallbackErrorNoReturn(pconnection, result, GP_GENERAL, "There was an error syncing NP buddies.");
					return;
				}
			}
			else
				gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
					"PS3BuddySync: \"%s\" is already a buddy\n", arg->matches[0].uniquenick);
		}
		else
			gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
				"PS3BuddySync: No suitable match found\n");
	}
	else 
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
			"PS3BuddySync: Buddy Search FAILED!\n");
		
		CallbackErrorNoReturn(pconnection, arg->result, GP_GENERAL, "There was an error syncing NP buddies.");
	}

	GSI_UNUSED(pconnection);
	GSI_UNUSED(param);
}

// Called for GP buddies, here we make sure they are deleted if not in the NP 
// friend list.
void gpiSyncNpDeletedBuddiesCallback(GPConnection *pConnection, GPGetInfoResponseArg *arg, void *param)
{
	if (arg->result == GP_NO_ERROR)
	{
		SceNpId npId;
		int ret;  
		unsigned int count, i;
		GPIBool buddyExists = GPIFalse;
		GPIConnection * iconnection = (GPIConnection*)*pConnection;

		if (!iconnection->npSyncEnabled)
		{
			// Sync was disabled mid-sync, we will set the sync flag true so it
			// will trigger once sync is enabled.
			iconnection->npPerformBuddySync = GPITrue;
			return;
		}

		// Get NP friend count.
		ret = sceNpBasicGetFriendListEntryCount(&count);
		if ( ret < 0 ) 
		{
			CallbackErrorNoReturn(pConnection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
			return;
		}

		// Loop through each NP friend checking if it is the GP buddy passed to
		// this callback.
		for (i = 0; i < count; i++) 
		{
#ifdef GSI_UNICODE
			char uniquenickChar[GP_UNIQUENICK_LEN]; // Used for converting unicode uniquenick to ascii string.
#endif
			memset(&npId, 0, sizeof(npId));
			ret = sceNpBasicGetFriendListEntry(i, &npId);
			if (ret < 0) 
			{
				gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
					"PS3BuddySync: Failed to get NP friend entry #%d\n", i);
				
				// After an NP friend 'remove', the count set by 
				// sceNpBasicGetFriendListEntryCount does not seem to decrement
				// accordingly, which results in 
				// SCE_NP_BASIC_ERROR_INVALID_ARGUMENT for invalid index passed 
				// to sceNpBasicGetFriendListEntry. So if this is the case we 
				// just continue instead of erroring out.
				if (ret != SCE_NP_BASIC_ERROR_INVALID_ARGUMENT)
				{
					CallbackErrorNoReturn(pConnection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP buddies.");
					return;
				}
				continue;
			}

#ifdef GSI_UNICODE
			// Since strncmp needs string arguments.
			UCSToAsciiString(arg->uniquenick, uniquenickChar);
			if (strncmp(npId.handle.data, uniquenickChar, sizeof(npId.handle.data)) == 0)
#else
			if (strncmp(npId.handle.data, arg->uniquenick, sizeof(npId.handle.data)) == 0)
#endif
			{
				buddyExists = GPITrue; // This GP buddy is also an NP friend.
				break;
			}
		}

		if (!buddyExists) // Delete GP buddy since he is not an NP friend.
		{
			GPICallback callback;
			time_t date;

			gpiDeleteBuddy(pConnection, arg->profile, GPITrue);

			// Call buddy revoke callback to alert developer of buddy removal.
			date = time(NULL);
			callback = iconnection->callbacks[GPI_RECV_BUDDY_REVOKE];
			if(callback.callback != NULL)
			{
				GPRecvBuddyRevokeArg * revokeArg;
				revokeArg = (GPRecvBuddyRevokeArg *)gsimalloc(sizeof(GPRecvBuddyRevokeArg));

				if (revokeArg == NULL)
				{
					gpiSetErrorString(pConnection, "Out of memory.");
					return;
				}
				revokeArg->profile = arg->profile;
				revokeArg->date = (unsigned int)date;

				gpiAddCallback(pConnection, callback, revokeArg, NULL, GPI_ADD_BUDDYREVOKE);
			}
		}
	}
	GSI_UNUSED(pConnection);
	GSI_UNUSED(param);
}

GPResult gpiSyncNpBlockList(GPConnection * connection)
{
    int ret; 
    SceNpId npId;	//Buffer to store block list entry's NP ID.
    gsi_u32 i, j, count = 0;
    GPIConnection * iconnection = (GPIConnection*)*connection;
	GPResult result = GP_NO_ERROR;

    // Flag sync as complete so we don't do it more than once per login.
    iconnection->npPerformBlockSync = gsi_false;

    // Get NP block list count.
    ret = sceNpBasicGetBlockListEntryCount(&count);
    if ( ret < 0 ) 
    {
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "PS3BlockSync: Failed to get NP block list count\n");
		CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP block list.");
    }

    // Loop through each entry, check for existence of GSID account.
    for (i = 0; i < count; i++) 
    {
		gsi_bool blockFound = gsi_false;
        memset(&npId, 0x00, sizeof(npId));
        ret = sceNpBasicGetBlockListEntry(i, &npId);
        if (ret < 0) 
        {
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "PS3BlockSync: Failed to get NP block entry #%d\n", i);
            CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP block list.");
        }

        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
            "PS3BlockSync: NP block entry #%d, npid = %s. Queueing Search.\n", i, npId.handle.data);

		// Loop through GP block list and compare current np handle to each
		// uniquenick *if* it does not match any GP blocks, then proceed with
		// profile search.
		for (j = 0; j < iconnection->profileList.numBlocked; j++)
		{
			GPIProfile *aProfile = NULL;
			GPGetInfoResponseArg infoResponse;
#ifdef GSI_UNICODE
			char uniquenickChar[GP_UNIQUENICK_LEN]; // Used for converting unicode uniquenick to ascii string.
#endif
			aProfile = gpiFindBlockedProfile(connection, j);
			if (aProfile == NULL)
			{
				// This shouldn't happen because we always have the block list
				// before we're done connecting. If it does occur we will just 
				// skip out of the 'for' loop and go straight to the profile 
				// search.
				break;
			}

			// Check user info locally to avoid backend traffic.
			result = gpiGetInfoNoWait(connection, aProfile->profileId, &infoResponse);
			if (result != GP_NO_ERROR)
			{
				continue; // This will expectedly error if the cache has not yet been written.
			}
#ifdef GSI_UNICODE
			// Since strncmp needs string arguments.
			UCSToAsciiString(infoResponse.uniquenick, uniquenickChar);
			if (strncmp(npId.handle.data, uniquenickChar, sizeof(npId.handle.data)) == 0)
#else
			if (strncmp(npId.handle.data, infoResponse.uniquenick, sizeof(npId.handle.data)) == 0)
#endif
			{
				blockFound = gsi_true; // This NP block member is already in our GP block list.
				break;
			}
		}

		// Only run profile search if we don't already have this person in our
		// block list.
		if (!blockFound) 
		{
			result = gpiProfileSearchUniquenick(connection, npId.handle.data, &iconnection->namespaceID, 
				1, GP_NON_BLOCKING, (GPCallback)gpiSyncNpBlockListCallback, NULL);
			if (result != GP_NO_ERROR)
			{
				CallbackError(connection, result, GP_GENERAL, "There was an error syncing NP block list.");
			}
		}
    }

	// Loop through GP block list and check each in the NP list (deleting those 
	// who are not blocked in NP).
	for (i = 0; i < iconnection->profileList.numBlocked; i++)
	{
		GPIProfile *aProfile = NULL;
		aProfile = gpiFindBlockedProfile(connection, i);
		if (aProfile == NULL)
		{
			// This shouldn't happen because we always have the block list 
			// before we're done connecting.
			CallbackError(connection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP block list.");
		}

		// The getinfo callback (gpiSyncNpDeletedBlocksCallback) will remove 
		// GP blocked members who are not blocked in NP.
		result = gpiGetInfo(connection, aProfile->profileId, GP_CHECK_CACHE, GP_NON_BLOCKING, (GPCallback)gpiSyncNpDeletedBlocksCallback, NULL);
		if (result != GP_NO_ERROR)
		{
			CallbackError(connection, result, GP_GENERAL, "There was an error syncing NP block list.");
		}
	}

    return result;
}

// Called for NP blocked members, here we make sure they are also added to our 
// GP block list.
void gpiSyncNpBlockListCallback(GPConnection * pconnection, 
								GPProfileSearchResponseArg * arg, 
								void * param)
{
	GPIProfile * pProfile;
    GPIConnection * iconnection = (GPIConnection*)*pconnection;

    if(arg->result == GP_NO_ERROR)
    {
        if(arg->numMatches == 1)
        {
            // Check if already blocked in GP.
            if(!gpiGetProfile(pconnection, arg->matches[0].profile, &pProfile) || !pProfile->blocked)
            {
				GPResult result;
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
                    "PS3BlockSync: NP Block Entry \"%s\" found in namespace %d. Adding to BlockedList.\n", 
                    arg->matches[0].uniquenick, arg->matches[0].namespaceID);

                // Add to GP Blocked List - set lock to make sure we dont try
				// to add to NP list.
                iconnection->npSyncLock = gsi_true;
                result = gpiAddToBlockedList(pconnection, arg->matches[0].profile);
                iconnection->npSyncLock = gsi_false;

				if (result != GP_NO_ERROR)
				{
					CallbackErrorNoReturn(pconnection, result, GP_GENERAL, "There was an error syncing NP block list.");
					return;
				}
            }
            else
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
                    "PS3BlockSync: \"%s\" is already blocked\n", arg->matches[0].uniquenick);
        }
        else
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
                "PS3BlockSync: No suitable match found\n");
    }
    else 
	{
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
        "PS3BlockSync: Block Entry Search FAILED!\n");

		CallbackErrorNoReturn(pconnection, arg->result, GP_GENERAL, "There was an error syncing NP block list.");
		return;
	}

    GSI_UNUSED(param);
}

// Called for GP blocked members, here we make sure they are removed if not in 
// the NP block list.
void gpiSyncNpDeletedBlocksCallback(GPConnection *pConnection, GPGetInfoResponseArg *arg, void *param)
{
	if (arg->result == GP_NO_ERROR)
	{
		SceNpId npId;
		int ret;  
		unsigned int count, i;
		GPIBool blockExists = GPIFalse;
#ifdef GSI_UNICODE
		char uniquenickChar[GP_UNIQUENICK_LEN]; // Used for converting unicode uniquenick to ascii string.
#endif
		GPIConnection * iconnection = (GPIConnection*)*pConnection;

		if (!iconnection->npSyncEnabled)
		{
			// Sync was disabled mid-sync, we will set the sync flag true so it 
			// will trigger once sync is enabled.
			iconnection->npPerformBlockSync = GPITrue;
			return;
		}

		// Get NP block list count.
		ret = sceNpBasicGetBlockListEntryCount(&count);
		if ( ret < 0 ) 
		{
			CallbackErrorNoReturn(pConnection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP block list.");
			return;
		}

		// Loop through each NP blocked member checking if it is the GP blocked 
		// member passed to this callback.
		for (i = 0; i < count; i++) 
		{
			memset(&npId, 0, sizeof(npId));
			ret = sceNpBasicGetBlockListEntry(i, &npId);
			if (ret < 0) 
			{
				gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
					"PS3BuddySync: Failed to get NP block entry #%d\n", i);

				// After an NP friend 'remove', the count set by 
				// sceNpBasicGetFriendListEntryCount does not seem to decrement
				// accordingly, which results in 
				// SCE_NP_BASIC_ERROR_INVALID_ARGUMENT for invalid index passed 
				// to sceNpBasicGetFriendListEntry. So if this is the case for 
				// blocks too we just continue instead of erroring out.
				if (ret != SCE_NP_BASIC_ERROR_INVALID_ARGUMENT)
				{
					CallbackErrorNoReturn(pConnection, GP_MISC_ERROR, GP_GENERAL, "There was an error syncing NP block list.");
					return;
				}
				continue;
			}

#ifdef GSI_UNICODE
			// Since strncmp needs string arguments.
			UCSToAsciiString(arg->uniquenick, uniquenickChar);
			if (strncmp(npId.handle.data, uniquenickChar, sizeof(npId.handle.data)) == 0)
#else
			if (strncmp(npId.handle.data, arg->uniquenick, sizeof(npId.handle.data)) == 0)
#endif
			{
				blockExists = GPITrue; // This GP buddy is also an NP friend.
				break;
			}
		}

		if (!blockExists) // Delete GP block member since he is not a in the NP block list.
		{
			gpiRemoveFromBlockedList(pConnection, arg->profile);
		}
	}
	GSI_UNUSED(pConnection);
	GSI_UNUSED(param);
}

GPResult gpiAddToNpBlockList(GPConnection * connection, 
							 int profileid)
{
    GPIConnection * iconnection = (GPIConnection*)*connection;
    // TODO: consider developer method for cache input in order to check HDD cache?

    // If NP status not resolved, don't bother with lookup.
    if (!iconnection->npTransactionList || iconnection->npLookupTitleCtxId < 0)
    {
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "PS3AddToNpBlockList: Cancelling add - NP status not yet resolved.\n");        
        return GP_NO_ERROR;
    }

    // Do an info lookup to find out if this player has an NP account.
    gpiGetInfo(connection, profileid, GP_CHECK_CACHE, GP_NON_BLOCKING, 
        (GPCallback)gpiAddToNpBlockListInfoCallback, NULL);

    return GP_NO_ERROR;
}

void gpiAddToNpBlockListInfoCallback(GPConnection * pconnection, 
									 GPGetInfoResponseArg * arg, 
									 void * param)
{
    SceNpOnlineId onlineId;
    int ret;
    npIdLookupTrans transaction;
    GPIConnection * iconnection = (GPIConnection*)*pconnection;
#ifdef GSI_UNICODE
    char asciiUniquenick[GP_UNIQUENICK_LEN];
#endif

    if(arg->result == GP_NO_ERROR)
    {
        // Make sure its a PS3 uniquenick (e.g. we have the uniquenick).
        if (_tcslen(arg->uniquenick) != 0)
        {
            memset(&onlineId, 0, sizeof(onlineId));

#ifdef GSI_UNICODE
            UCS2ToAsciiString(arg->uniquenick, (char*)asciiUniquenick);
            strncpy(onlineId.data, asciiUniquenick, SCE_NET_NP_ONLINEID_MAX_LENGTH);
#else
            strncpy(onlineId.data, arg->uniquenick, SCE_NET_NP_ONLINEID_MAX_LENGTH);
#endif

            if (ArrayLength(iconnection->npTransactionList) < GPI_NP_NUM_TRANSACTIONS)
            {
                ret = sceNpLookupCreateTransactionCtx(iconnection->npLookupTitleCtxId);
                if (ret < 0)
                {
                    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                        "PS3AddToNpBlockList: sceNpLookupCreateTransactionCtx() failed. ret = 0x%x\n", ret);  
                }
                else
                {
                    transaction.npIdForAdd = (SceNpId*)gsimalloc(sizeof(SceNpId));
                    if(transaction.npIdForAdd == NULL)
                    {
                        sceNpLookupDestroyTransactionCtx(ret);
                        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                            "PS3AddToNpBlockList: Out of memory.\n");  
                        return;
                    }
                    transaction.npTransId = ret;
                    transaction.npLookupDone = gsi_false;
                    ArrayAppend(iconnection->npTransactionList, &transaction);

                    // Perform NP lookup to get the NpId.
                    ret = sceNpLookupNpIdAsync(transaction.npTransId, &onlineId, 
                        transaction.npIdForAdd, 0, NULL);
                    if (ret < 0) 
                    {
                        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                            "PS3AddToNpBlockList: sceNpLookupNpIdAsync() failed. ret = 0x%x\n", ret);  
                    } 
                }
            }
            else
            {
                // Can only have a max of 32 simultaneous transactions (based on PS3 lib).
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
                    "PS3AddToNpBlockList: Transactions limit reached for np lookups\n");  
            }
        }
        else
            gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                "PS3AddToNpBlockList: Profile [%d] does not have a uniquenick in namespace %d!\n",
                arg->profile, iconnection->namespaceID);
    }
    else
        gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "PS3AddToNpBlockList: Player Info lookup FAILED!\n");

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

GPResult gpiProcessNp(GPConnection * connection)
{
    int i, ret=0;
    GPIConnection * iconnection = (GPIConnection*)*connection;
    npIdLookupTrans * transaction;

    // Check for offline status disconnected trigger.
    if(iconnection->npDisconnected)
    {
        gpiDestroyNpBasic(connection);
        return GP_NO_ERROR;
    }
    
	// Call this every 'frame' since we need to process sysutil for the async 
	// lookups - NP manager status callback/NP basic callback/close of in-game 
	// XMB callback.
	cellSysutilCheckCallback();

	if (iconnection->npBasicEventsToGet > 0)
	{
		SceNpUserInfo user;	// Buffer to store contact's NP user info.
		uint8_t buffer[SCE_NP_BASIC_MAX_MESSAGE_SIZE]; // Buffer to store incoming data.

		int ret;
		int event;
		size_t size;
		size = sizeof(buffer);

		ret = sceNpBasicGetEvent(&event, &user, buffer, &size);
		iconnection->npBasicEventsToGet--;
		if ( ret < 0 ) {
			if (ret == SCE_NP_BASIC_ERROR_NO_EVENT) // The 'no event' case is not a problem.
			{
				iconnection->npBasicEventsToGet = 0;	
			}
			else
			{
				gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
					"sceNpBasicGetEvent: failed for reason other than 'no event' case\n");
			}
		}
	}

    // Check for uninitialized transaction darray.
    if (!iconnection->npTransactionList)
        return GP_NO_ERROR;

    // Loop through all current transactions, check if complete.
    for (i=0; i < ArrayLength(iconnection->npTransactionList); i++)
    {
        // Grab next transaction in the list.
        transaction = (npIdLookupTrans *)ArrayNth(iconnection->npTransactionList, i);

        if (!transaction->npLookupDone)
        {
            if (sceNpLookupPollAsync(transaction->npTransId, &ret)==0)
                transaction->npLookupDone = gsi_true;
        }
        else
        {
            if (ret<0)
            {
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                    "PS3AddToNpBlockList: sceNpLookupWaitAsync. ret = 0x%x\n", ret);
                if (ret == (int)SCE_NP_COMMUNITY_SERVER_ERROR_NO_SUCH_USER_NPID)
                {
                    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                        "PS3AddToNpBlockList: Player '%s' is not an NP user.\n", 
                        transaction->npIdForAdd->handle.data);
                }
            }
            else
            {
                // Found an NpId, try to add.            
                ret = sceNpBasicAddBlockListEntry(transaction->npIdForAdd);
                if (ret == (int)SCE_NP_BASIC_ERROR_BUSY)
                {
                    // Oh nice, NP is too busy to help us.... keep on trying.                        
                    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
                        "PS3AddToNpBlockList: SCE_NP_BASIC_ERROR_BUSY. continue trying to add to NP\n"); 
                    return GP_NO_ERROR;
                }
                else if ( ret < 0 ) 
                {
                    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                        "PS3AddToNpBlockList: sceNpBasicAddBlockListEntry() failed. ret = 0x%x\n", ret); 
                }                
                else
                {
                    gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_Comment,
                        "PS3AddToNpBlockList: Player '%s' added to NP Block list.\n", 
                        transaction->npIdForAdd->handle.data); 
                }
            }

            ret = sceNpLookupDestroyTransactionCtx(transaction->npTransId);
            if (ret < 0)
            {
                gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Misc, GSIDebugLevel_HotError,
                    "PS3AddToNpBlockList: sceNpLookupDestroyTransactionCtx() failed. ret = 0x%x\n", ret); 
            }

            // Delete Transaction when its complete.
            ArrayDeleteAt(iconnection->npTransactionList, i);
        }
    }

    return GP_NO_ERROR;
}

#endif
