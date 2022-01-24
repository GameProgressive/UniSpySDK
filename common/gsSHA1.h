///////////////////////////////////////////////////////////////////////////////
// File:	gsSHA1.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ------------------------------------
// This is the header file for code which implements the Secure
// Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
// April 17, 1995.
//
// Many of the variable names in this code, especially the
// single character names, were used because those were the names
// used in the publication.
//
// Please read the file sha1.c for more information.

#ifndef _SHA1_H_
#define _SHA1_H_

//#include <stdint.h>
#include "gsCommon.h"

/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typdef the following:
 *    name              meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 *
 */
// WD : THOMAS : These are already defined in Android:
#ifndef ANDROID
#if !(defined(_PS3) || defined(_UNIX) || defined(_MACOSX))
	// these common types are defined in sony libs
	typedef gsi_u32 uint32_t;
	typedef gsi_u8  uint8_t;
#endif

typedef gsi_i16 int_least16_t;
#endif

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif
#define GSSHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct GSSHA1Context
{
    uint32_t Intermediate_Hash[GSSHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} GSSHA1Context;

/*
 *  Function Prototypes
 */

#if defined(__cplusplus)
extern "C"
{
#endif


int GSSHA1Reset(  GSSHA1Context *);
int GSSHA1Input(  GSSHA1Context *,
                const uint8_t *,
                unsigned int);
int GSSHA1Result( GSSHA1Context *,
                uint8_t Message_Digest[GSSHA1HashSize]);


#if defined(__cplusplus)
}
#endif // extern "C"


#endif
