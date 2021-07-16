///////////////////////////////////////////////////////////////////////////////
// File:	md5.h
// SDK:		GameSpy Common
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
// Derived from the RSA Data Security, Inc. 
// MD5 Message-Digest Algorithm. Copyright (c) 1999-2009 GameSpy Industries, Inc.
// ------------------------------------
// Header file for md5.c.
// ------------------------------------
// Copyright (C) 1991, RSA Data Security, Inc. All rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.  
//                                                                  
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.  
//                                                                  
// These notices must be retained in any copies of any part of this
// documentation and/or software.  

#ifndef _MD5_H_
#define _MD5_H_

#ifdef __cplusplus
extern "C" {
#endif
	
// PROTOTYPES should be set to one if and only if the compiler supports
// function argument prototyping. The following makes PROTOTYPES default 
// to 0 if it has not already been defined with C compiler flags.

#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                           /* state (ABCD) */
  UINT4 count[2];                /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                                 /* input buffer */
} GSMD5_CTX;

void GSMD5Init PROTO_LIST ((GSMD5_CTX *));
void GSMD5Update PROTO_LIST ((GSMD5_CTX *, unsigned char *, unsigned int));
void GSMD5Final PROTO_LIST ((unsigned char [16], GSMD5_CTX *));
void GSMD5Print PROTO_LIST ((unsigned char [16], char[33]));
void GSMD5Digest PROTO_LIST ((unsigned char *, unsigned int, char[33]));
#ifdef __cplusplus
}
#endif

#endif
