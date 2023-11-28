///////////////////////////////////////////////////////////////////////////////
// File:	hashtable.c
// SDK:		GameSpy Common
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 1998-2009 GameSpy Industries, Inc.
// ------------------------------------
// See hashtable.h for function comments.
// Implementation is straight-forward, using a fixed dynamically allocated
// array for the buckets, and a DArray for each individual bucket.

#include <stdlib.h>
#include <string.h>
#include "darray.h"
#include "hashtable.h"
#include "gsAssert.h"

#ifdef _MFC_MEM_DEBUG
#define _CRTDBG_MAP_ALLOC 1
#include <crtdbg.h>
#endif


#ifdef _NO_NOPORT_H_
	#define gsimalloc malloc
	#define gsifree free
	#define gsirealloc realloc
	#include <assert.h>
#else
	#include "nonport.h" //for gsimalloc/realloc/free/assert
#endif


struct HashImplementation 
{
	DArray *buckets;
	int nbuckets;
	TableElementFreeFn freefn;
	TableHashFn hashfn;
	TableCompareFn compfn;
};

HashTable TableNew(int elemSize, int nBuckets, 
                   TableHashFn hashFn, TableCompareFn compFn, 
 					 TableElementFreeFn freeFn)
{
	return TableNew2(elemSize, nBuckets, 4, hashFn, compFn, freeFn);
}
HashTable TableNew2(int elemSize, int nBuckets, int nChains,
                   TableHashFn hashFn, TableCompareFn compFn, 
 					 TableElementFreeFn freeFn)
{
	HashTable table;
	int i;

	GS_ASSERT(hashFn != NULL);
	GS_ASSERT(compFn != NULL);
	GS_ASSERT(elemSize != 0);
	GS_ASSERT(nBuckets != 0);

	table = (HashTable)gsimalloc(sizeof(struct HashImplementation));
	GS_ASSERT(table != NULL);
	
	table->buckets = (DArray *)gsimalloc(nBuckets * sizeof(DArray));
	GS_ASSERT(table->buckets != 0);
	for (i = 0; i < nBuckets; i++) //ArrayNew will assert if allocation fails
		table->buckets[i] = ArrayNew(elemSize, nChains, freeFn);
	table->nbuckets = nBuckets;
	table->freefn = freeFn;
	table->compfn = compFn;
	table->hashfn = hashFn;

	return table;
}


void TableFree(HashTable table)
{
	int i;
	
	GS_ASSERT(table != NULL);

	if (NULL == table )
		return;
	
	for (i = 0 ; i < table->nbuckets ; i++)
		ArrayFree(table->buckets[i]);
	gsifree(table->buckets);
	gsifree(table);
}


int TableCount(HashTable table)
{
	int i, count = 0;
	
	GS_ASSERT(table != NULL);

	if (NULL == table )
		return count;

	for (i = 0 ; i < table->nbuckets ; i++)
		count += ArrayLength(table->buckets[i]);
	
	return count;
}


void TableEnter(HashTable table, const void *newElem)
{
	int hash, itempos;
	
	GS_ASSERT(table != NULL);

	if (NULL == table )
		return;

	hash = table->hashfn(newElem, table->nbuckets);
	itempos = ArraySearch(table->buckets[hash], newElem, table->compfn, 0,0);
	if (itempos == NOT_FOUND)
		ArrayAppend(table->buckets[hash], newElem);
	else
		ArrayReplaceAt(table->buckets[hash], newElem, itempos);
}

int TableRemove(HashTable table, const void *delElem)
{
	int hash, itempos;
	
	GS_ASSERT(table != NULL);

	if (NULL == table )
		return 0;

	hash = table->hashfn(delElem, table->nbuckets);
	itempos = ArraySearch(table->buckets[hash], delElem, table->compfn, 0,0);
	if (itempos == NOT_FOUND)
		return 0;
	else
		ArrayDeleteAt(table->buckets[hash], itempos);
	return 1;
}

void *TableLookup(HashTable table, const void *elemKey)
{
	int hash, itempos;
	
	GS_ASSERT(table != NULL);

	if (NULL == table )
		return NULL;

	hash = table->hashfn(elemKey, table->nbuckets);
	itempos = ArraySearch(table->buckets[hash], elemKey, table->compfn, 0,
						  0);
	if (itempos == NOT_FOUND)
		return NULL;
	else
		return ArrayNth(table->buckets[hash], itempos);
}


void TableMap(HashTable table, TableMapFn fn, void *clientData)
{
	int i;
	
	GS_ASSERT(table != NULL);
	GS_ASSERT(fn != NULL);

	if (NULL == table || NULL == fn)
		return;
	
	for (i = 0 ; i < table->nbuckets ; i++)
		ArrayMap(table->buckets[i], fn, clientData);
	
}

void TableMapSafe(HashTable table, TableMapFn fn, void *clientData)
{
	int i;
	
	GS_ASSERT(fn != NULL);
	
	for (i = 0 ; i < table->nbuckets ; i++)
		ArrayMapBackwards(table->buckets[i], fn, clientData);
	
}

void * TableMap2(HashTable table, TableMapFn2 fn, void *clientData)
{
	int i;
	void * pcurr;
	
	GS_ASSERT(fn != NULL);
	
	for (i = 0 ; i < table->nbuckets ; i++)
	{
		pcurr = ArrayMap2(table->buckets[i], fn, clientData);
		if(pcurr)
			return pcurr;
	}

	return NULL;
}

void * TableMapSafe2(HashTable table, TableMapFn2 fn, void *clientData)
{
	int i;
	void * pcurr;
	
	GS_ASSERT(fn != NULL);
	
	for (i = 0 ; i < table->nbuckets ; i++)
	{
		pcurr = ArrayMapBackwards2(table->buckets[i], fn, clientData);
		if(pcurr)
			return pcurr;
	}

	return NULL;
}

void TableClear(HashTable table)
{
	int i;

	for (i = 0 ; i < table->nbuckets ; i++)
		ArrayClear(table->buckets[i]);
}
