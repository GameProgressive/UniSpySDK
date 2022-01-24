///////////////////////////////////////////////////////////////////////////////
// File:	gpiUnique.h
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GPIUNIQUE_H_
#define _GPIUNIQUE_H_

//INCLUDES
#include "gpi.h"

//FUNCTIONS
GPResult gpiRegisterUniqueNick(
  GPConnection * connection,
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char cdkey[GP_CDKEY_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiProcessRegisterUniqueNick(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

// Separated registration of unique nick and cdkey.
GPResult gpiRegisterCdKey(
  GPConnection * connection,
  const char cdkey[GP_CDKEY_LEN],
  int gameId,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiProcessRegisterCdKey(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

#endif
