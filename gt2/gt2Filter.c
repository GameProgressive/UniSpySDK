///////////////////////////////////////////////////////////////////////////////
// File:	gt2Filter.c
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "gt2Filter.h"
#include "gt2Callback.h"
#include "gt2Message.h"
#include "gt2Utility.h"

static int GS_STATIC_CALLBACK gti2SendFiltersCompare
(
	const void * elem1,
	const void * elem2
)
{
	gt2SendFilterCallback * callback1 = (gt2SendFilterCallback *)elem1;
	gt2SendFilterCallback * callback2 = (gt2SendFilterCallback *)elem2;

	if(*callback1 == *callback2)
		return 0;

	return 1;
}

static int GS_STATIC_CALLBACK gti2ReceiveFiltersCompare
(
	const void * elem1,
	const void * elem2
)
{
	gt2ReceiveFilterCallback * callback1 = (gt2ReceiveFilterCallback *)elem1;
	gt2ReceiveFilterCallback * callback2 = (gt2ReceiveFilterCallback *)elem2;

	if(*callback1 == *callback2)
		return 0;

	return 1;
}

GT2Bool gti2AddSendFilter(GT2Connection connection, gt2SendFilterCallback callback)
{
	// Check to see if we have a send filters list.
	if(!connection->sendFilters)
		return GT2False;

	// Add this callback to the list.
	ArrayAppend(connection->sendFilters, &callback);

	// Return GT2True if the filter was added.
	return (ArraySearch(connection->sendFilters, &callback, gti2SendFiltersCompare, 0, 0) != NOT_FOUND);
}

GT2Bool gti2AddReceiveFilter(GT2Connection connection, gt2ReceiveFilterCallback callback)
{
	// Check to see if we have a receive filters list.
	if(!connection->receiveFilters)
		return GT2False;

	// Add this callback to the list.
	ArrayAppend(connection->receiveFilters, &callback);

	// Return GT2True if the filter was added.
	return (ArraySearch(connection->receiveFilters, &callback, gti2ReceiveFiltersCompare, 0, 0) != NOT_FOUND);
}

void gti2RemoveSendFilter(GT2Connection connection, gt2SendFilterCallback callback)
{
	int index;

	// Check for no filters.
	if(!connection->sendFilters)
		return;

	// Check for removing all filters.
	if(!callback)
	{
		// Remove all of the filters.
		ArrayClear(connection->sendFilters);
		return;
	}

	// Find the filter.
	index = ArraySearch(connection->sendFilters, &callback, gti2SendFiltersCompare, 0, 0);
	if(index == NOT_FOUND)
		return;

	// Remove the filter.
	ArrayRemoveAt(connection->sendFilters, index);
}

void gti2RemoveReceiveFilter(GT2Connection connection, gt2ReceiveFilterCallback callback)
{
	int index;

	// Check for no filters.
	if(!connection->receiveFilters)
		return;

	// Check for removing all filters.
	if(!callback)
	{
		// Remove all of the filters.
		ArrayClear(connection->receiveFilters);
		return;
	}

	// Find the filter.
	index = ArraySearch(connection->receiveFilters, &callback, gti2ReceiveFiltersCompare, 0, 0);
	if(index == NOT_FOUND)
		return;

	// Remove the filter.
	ArrayRemoveAt(connection->receiveFilters, index);
}

GT2Bool gti2FilteredSend(GT2Connection connection, int filterID, const GT2Byte * message, int len, GT2Bool reliable)
{
	int num;

	// Make sure that we are connected.
	if(connection->state != GTI2Connected)
		return GT2True;

	// Check the message and len.
	gti2MessageCheck(&message, &len);

	// Get the number of filters.
	num = ArrayLength(connection->sendFilters);

	// Check that its a valid ID.
	if(filterID < 0)
		return GT2True;
	if(filterID >= num)
		return GT2True;

	// Is this filter the last one?
	if(filterID == (num - 1))
	{
		// Do the actual send.
		if(!gti2Send(connection, message, len, reliable))
			return GT2False;
	}
	else
	{
		// Filter it.
		if(!gti2SendFilterCallback(connection, ++filterID, message, len, reliable))
			return GT2False;
	}

	return GT2True;
}

GT2Bool gti2FilteredReceive(GT2Connection connection, int filterID, GT2Byte * message, int len, GT2Bool reliable)
{
	int num;

	// Make sure we're connected.
	if(connection->state != GTI2Connected)
		return GT2True;

	// Get the number of filters.
	num = ArrayLength(connection->receiveFilters);

	// Check that it's a valid ID.
	if(filterID < 0)
		return GT2True;
	if(filterID >= num)
		return GT2True;

	// Is this the last filter?
	if(filterID == (num - 1))
	{
		// Call the callback.
		if(!gti2ReceivedCallback(connection, message, len, reliable))
			return GT2False;
	}
	else
	{
		// Filter it.
		if(!gti2ReceiveFilterCallback(connection, ++filterID, message, len, reliable))
			return GT2False;
	}

	return GT2True;
}
