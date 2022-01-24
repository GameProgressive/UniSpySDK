///////////////////////////////////////////////////////////////////////////////
// File:	gpiCallback.h
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef _GPICALLBACK_H_
#define _GPICALLBACK_H_

//INCLUDES
#include "gpi.h"

//DEFINES
// Unsolicited Callbacks.
enum GPICallbackId
{
	GPI_ERROR                      = GP_ERROR,
	GPI_RECV_BUDDY_REQUEST         = GP_RECV_BUDDY_REQUEST,
	GPI_RECV_BUDDY_STATUS          = GP_RECV_BUDDY_STATUS,
	GPI_RECV_BUDDY_MESSAGE         = GP_RECV_BUDDY_MESSAGE,
	GPI_RECV_BUDDY_UTM             = GP_RECV_BUDDY_UTM,
	GPI_RECV_GAME_INVITE           = GP_RECV_GAME_INVITE,
	GPI_TRANSFER_CALLBACK          = GP_TRANSFER_CALLBACK,
	GPI_RECV_BUDDY_AUTH            = GP_RECV_BUDDY_AUTH,
	GPI_RECV_BUDDY_REVOKE          = GP_RECV_BUDDY_REVOKE,
	GPI_NUM_CALLBACKS              
};

// Add type - not 0 only for a few.
enum GPIAddCallbackType
{
	GPI_ADD_NORMAL,
	GPI_ADD_ERROR,
	GPI_ADD_MESSAGE,
	GPI_ADD_NICKS,
	GPI_ADD_PMATCH,
	GPI_ADD_STATUS,
	GPI_ADD_BUDDDYREQUEST,
	GPI_ADD_TRANSFER_CALLBACK,
	GPI_ADD_REVERSE_BUDDIES,
	GPI_ADD_SUGGESTED_UNIQUE,
	GPI_ADD_BUDDYAUTH,
	GPI_ADD_BUDDYUTM,
	GPI_ADD_BUDDYREVOKE,
	GPI_ADD_REVERSE_BUDDIES_LIST,
	GPI_ADD_BUDDYKEYS,
	GPI_ADD_PROFILE_BUDDY_LIST,
	GPI_NUM_ADD_CALLBACK_TYPES
};


//TYPES
// A Callback.
typedef struct
{
  GPCallback callback;
  void * param;
} GPICallback;

// Data for a pending callback.
typedef struct GPICallbackData
{
	GPICallback callback;
	void * arg;
	int type;
	int operationID;
	struct GPICallbackData * pnext;
} GPICallbackData;

//FUNCTIONS
void
gpiCallErrorCallback(
  GPConnection * connection,
  GPResult result,
  GPEnum fatal
);

typedef struct GPIOperation_s *GPIOperation_st;

GPResult
gpiAddCallback(
  GPConnection * connection,
  GPICallback callback,
  void * arg,
  const struct GPIOperation_s * operation,
  int type
);

GPResult
gpiProcessCallbacks(
  GPConnection * connection,
  int blockingOperationID
);

#endif
