///////////////////////////////////////////////////////////////////////////////
// File:	gpiProfile.h
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GPIPROFILE_H_
#define _GPIPROFILE_H_

//INCLUDES
#include "gpi.h"

//DEFINES
#define GPI_SIG_LEN      33

//TYPES
// The status for a buddy profile.

// New Status Info.
typedef struct _GPIBuddyStatusInfo 
{
	int buddyIndex;
	GPEnum statusState;
	char *richStatus;
	char *gameType;
	char *gameVariant;
	char *gameMapName;
	unsigned int sessionFlags;
	unsigned int buddyIp;
	unsigned short buddyPort;
	unsigned int hostIp;
	unsigned int hostPrivateIp;
	unsigned short queryPort;
	unsigned short hostPort;
	GPEnum quietModeFlags;
	int productId;
	// New Status Info extended info Keys.
	DArray extendedInfoKeys;
} GPIBuddyStatusInfo;

// Old status. 
typedef struct
{
	int buddyIndex;
	GPEnum status;
	char * statusString;
	char * locationString;
	unsigned int ip;
	unsigned short port;
	GPEnum quietModeFlags;
} GPIBuddyStatus;

// Profile data.
typedef struct GPIProfile
{
	int profileId;
	int userId;
	GPIBuddyStatus * buddyStatus;
	GPIBuddyStatusInfo *buddyStatusInfo;
	GPIInfoCache * cache;
	char * authSig;
	int requestCount;
	char * peerSig;
    gsi_bool blocked;
    int blockIndex;
    gsi_bool buddyOrBlockCache;
	gsi_bool deletedLocally;
} GPIProfile;

// A list of profiles.
typedef struct
{
	HashTable profileTable;
	int num;
	int numBuddies;
    int numBlocked;
} GPIProfileList;

//FUNCTIONS
GPIBool
gpiInitProfiles(
  GPConnection * connection
);

GPIProfile *
gpiProfileListAdd(
  GPConnection * connection,
  int id
);

GPIBool
gpiGetProfile(
  GPConnection * connection,
  GPProfile profileid,
  GPIProfile ** pProfile
);

GPResult
gpiProcessNewProfile(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult
gpiNewProfile(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  GPEnum replace,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiProcessDeleteProfle
(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult gpiDeleteProfile(
    GPConnection * connection,
    GPCallback callback,
	void * param
);

void
gpiRemoveProfile(
  GPConnection * connection,
  GPIProfile * profile
);

void
gpiRemoveProfileByID(
  GPConnection * connection,
  int profileid
);

GPResult
gpiLoadDiskProfiles(
  GPConnection * connection
);

GPResult
gpiSaveDiskProfiles(
  GPConnection * connection
);

GPResult
gpiFindProfileByUser(
  GPConnection * connection,
  char nick[GP_NICK_LEN],
  char email[GP_EMAIL_LEN],
  GPIProfile ** profile
);

// return false to stop the mapping.
typedef GPIBool
(* gpiProfileMapFunc)(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
);

GPIBool
gpiProfileMap(
  GPConnection * connection,
  gpiProfileMapFunc func,
  void * data
);

GPIProfile *
gpiFindBuddy(
  GPConnection * connection,
  int buddyIndex
);

void gpiRemoveBuddyStatus(GPIBuddyStatus *buddyStatus);
void gpiRemoveBuddyStatusInfo(GPIBuddyStatusInfo *buddyStatusInfo);

GPIBool
gpiCanFreeProfile(
  GPIProfile * profile
);

void gpiSetInfoCacheFilename(
  const char filename[FILENAME_MAX + 1]
);

// BLOCK LIST
GPResult
gpiAddToBlockedList(
  GPConnection * connection,
  int profileid
);

GPResult
gpiRemoveFromBlockedList(
  GPConnection * connection,
  int profileid
);

GPIProfile *
gpiFindBlockedProfile(
  GPConnection * connection, 
  int blockIndex
);

GPResult
gpiProcessRecvBlockedList(
  GPConnection * connection,
  const char * input
);

#endif
