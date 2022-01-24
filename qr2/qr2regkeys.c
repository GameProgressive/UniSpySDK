///////////////////////////////////////////////////////////////////////////////
// File:	qr2regkeys.c
// SDK:		GameSpy Query and Reporting 2 (QR2) SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "qr2regkeys.h"
#include "qr2.h"
#include "../common/gsStringUtil.h"
#include "../common/gsDebug.h"

#if defined(__MWERKS__) || defined(_PSP2) // CodeWarrior requires prototypes.
void qr2_register_keyW(int keyid, const unsigned short *key);
void qr2_register_keyA(int keyid, const char *key);
#endif

const char *qr2_registered_key_list[MAX_REGISTERED_KEYS] =
{
	"",				//0 is reserved.
	"hostname",		//1
	"gamename",		//2
	"gamever",		//3
	"hostport",		//4
	"mapname",		//5
	"gametype",		//6
	"gamevariant",	//7
	"numplayers",	//8
	"numteams",		//9
	"maxplayers",	//10
	"gamemode",		//11
	"teamplay",		//12
	"fraglimit",	//13
	"teamfraglimit",//14
	"timeelapsed",	//15
	"timelimit",	//16
	"roundtime",	//17
	"roundelapsed",	//18
	"password",		//19
	"groupid",		//20
	"player_",		//21
	"score_",		//22
	"skill_",		//23
	"ping_",		//24
	"team_",		//25
	"deaths_",		//26
	"pid_",			//27
	"team_t",		//28
	"score_t",		//29
	"nn_groupid",	//30

	// Query From Master Only keys.
	"country",		//31
	"region"		//32
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Keep a list of the unicode keys that we have allocated internally so that 
// we can free them when qr2 is shutdown.
typedef struct QR2KeyListNodeS
{
	char* mKeyData;
	struct QR2KeyListNodeS*	mNextKey;
} QR2KeyListNode;

typedef struct QR2KeyListS
{
	struct QR2KeyListNodeS* mHead;
} QR2KeyList;

static QR2KeyList qr2_internal_key_list = { NULL };

void qr2_internal_key_list_append(char* theKey)
{
	QR2KeyListNode* aNewNode;

	GS_ASSERT(theKey != NULL);

	// Initialize the new node.
	aNewNode = (QR2KeyListNode*)gsimalloc(sizeof(QR2KeyListNode));
	aNewNode->mKeyData = theKey;
	aNewNode->mNextKey = NULL;

	// Check for a NULL head.
	if (qr2_internal_key_list.mHead == NULL)
		qr2_internal_key_list.mHead = aNewNode;
	else
	{
		// Find the end of the list and append this node.
		QR2KeyListNode* aInsertPlace = qr2_internal_key_list.mHead;
		while(aInsertPlace->mNextKey != NULL)
			aInsertPlace = aInsertPlace->mNextKey;

		aInsertPlace->mNextKey = aNewNode;
	}
}

void qr2_internal_key_list_free()
{
	QR2KeyListNode* aNodeToFree;
	QR2KeyListNode* aNextNode;

	// Free the nodes.
	aNodeToFree = qr2_internal_key_list.mHead;
	while (aNodeToFree != NULL)
	{
		aNextNode = aNodeToFree->mNextKey;	// Get a ptr to the next node (or will be lost).
		gsifree(aNodeToFree->mKeyData);		// Free the string we allocated in qr2_register_keyW.
		gsifree(aNodeToFree);				// Free the current node.
		aNodeToFree = aNextNode;			// Set the current node to the next node.
	}
	
	// Initialize the list back to NULL.
	qr2_internal_key_list.mHead = NULL;
}

gsi_bool qr2_internal_is_master_only_key(const char * keyname)
{
	if (strcmp(keyname,qr2_registered_key_list[COUNTRY_KEY]) == 0 || 
		strcmp(keyname,qr2_registered_key_list[REGION_KEY]) == 0)
		return gsi_true;

	return gsi_false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void qr2_register_keyA(int keyid, const char *key)
{
	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_keyA()\r\n");

	// Verify the key range.
	if (keyid < NUM_RESERVED_KEYS || keyid > MAX_REGISTERED_KEYS)
	{
		gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_WarmError,
			"Attempted to register invalid key %d - %s\r\n", keyid, key);
		return;
	}

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_Comment,
		"Registered key %d - %s\r\n", keyid, key);

	qr2_registered_key_list[keyid] = key;
}

#if GSI_UNICODE
void qr2_register_keyW(int keyid, const gsi_char *key)
{
	char* key_A = NULL;

	gsDebugFormat(GSIDebugCat_QR2, GSIDebugType_Misc, GSIDebugLevel_StackTrace,
		"qr2_register_keyW()\r\n");

	// Create UTF8 copy.
	key_A = UCSToUTF8StringAlloc(key);

	// Register the ascii version.
	qr2_register_keyA(keyid, key_A);

	// Keep track of the unicode version that so we can delete it later.
	qr2_internal_key_list_append(key_A);
}
#endif