///////////////////////////////////////////////////////////////////////////////
// File:	gpiInfo.h
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GPIINFO_H_
#define _GPIINFO_H_

//INCLUDES
#include "gpi.h"

//TYPES
// Profile info cache.
typedef struct
{
  char * nick;
  char * uniquenick;
  char * email;
  char * firstname;
  char * lastname;
  char * homepage;
  int icquin;
  char zipcode[GP_ZIPCODE_LEN];
  char countrycode[GP_COUNTRYCODE_LEN];
  float longitude; // negative is west, positive is east.  (0, 0) means unknown.
  float latitude;  // negative is south, positive is north.  (0, 0) means unknown.
  char place[GP_PLACE_LEN];  // e.g., "USA|California|Irvine", "South Korea|Seoul", "Turkey"
  int birthday;
  int birthmonth;
  int birthyear;
  GPEnum sex;
  int publicmask;
  char * aimname;
  int pic;
  int occupationid;
  int industryid;
  int incomeid;
  int marriedid;
  int childcount;
  int interests1;
  int ownership1;
  int conntypeid;
} GPIInfoCache;

//FUNCTIONS
GPResult
gpiSetInfoi(
  GPConnection * connection, 
  GPEnum info, 
  int value
);

GPResult
gpiSetInfos(
  GPConnection * connection, 
  GPEnum info, 
  const char * value
);

GPResult
gpiSetInfod(
  GPConnection * connection, 
  GPEnum info, 
  int day,
  int month,
  int year
);

GPResult
gpiSetInfoMask(
  GPConnection * connection, 
  GPEnum mask
);

void
gpiInfoCacheToArg(
  const GPIInfoCache * cache,
  GPGetInfoResponseArg * arg
);

GPResult
gpiGetInfo(
  GPConnection * connection,
  GPProfile profile, 
  GPEnum checkCache,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiGetInfoNoWait(
  GPConnection * connection,
  GPProfile profile,
  GPGetInfoResponseArg * arg
);

GPResult
gpiProcessGetInfo(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult
gpiSendGetInfo(
  GPConnection * connection,
  int profileid,
  int operationid
);

GPResult
gpiAddLocalInfo(
  GPConnection * connection,
  GPIBuffer * buffer
);

typedef struct GPIProfile *pGPIProfile;

GPIBool
gpiSetInfoCache(
  GPConnection * connection,
  pGPIProfile profile,
  const GPIInfoCache * cache
);

void
gpiFreeInfoCache(
  pGPIProfile profile
);

#endif
