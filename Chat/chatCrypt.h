///////////////////////////////////////////////////////////////////////////////
// File:	chatCrypt.h
// SDK:		GameSpy Chat SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef _CHATCRYPT_H_
#define _CHATCRYPT_H_
#ifdef __cplusplus
extern "C" {
#endif

	
typedef struct _gs_crypt_key
{      
   unsigned char state[256];       
   unsigned char x;        
   unsigned char y;
} gs_crypt_key;


void gs_prepare_key(const unsigned char *key_data_ptr, int key_data_len, gs_crypt_key *key);
void gs_crypt(unsigned char *buffer_ptr, int buffer_len, gs_crypt_key *key);
void gs_xcode_buf(char *buf, int len, char *enckey);

#ifdef __cplusplus
}
#endif

#endif
