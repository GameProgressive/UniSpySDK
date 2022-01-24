///////////////////////////////////////////////////////////////////////////////
// File:	gsStringUtil.h
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__


// String utilities used by the SDKs

#ifdef __cplusplus
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _PS2
#define ALIGNED	__attribute__ ((aligned(16)))
#else
#define ALIGNED
#endif

#define UCS2Char        unsigned short
#define UCS2String      unsigned short*	// null terminated

#define UCS4Char        gsi_u32
#define UCS4String      gsi_u32*		// null terminated

#define UTF8ByteType    char			// For type casting
#define UTF8String      char*			// may not be NULL terminated when treated as a single character

// Call proper Unicode functions
#ifdef GSI_UNICODE	
	#ifdef _UNIX
		typedef UCS4String				UCSStringType;			
		#define UCSToAsciiString		UCS4ToAsciiString	// for unix/linux, unicode is 4 bytes
		#define AsciiToUCSString		AsciiToUCS4String
		#define UTF8ToUCSString			UTF8ToUCS4String
		#define UTF8ToUCSStringAlloc	UTF8ToUCS4StringAlloc
		#define UTF8ToUCSStringLen		UTF8ToUCS4StringLen
		#define UTF8ToUCSStringAllocLen UTF8ToUCS4StringAllocLen
		#define UTF8ToUCSStringLen2		UTF8ToUCS4StringLen2
		#define UCSToUTF8String			UCS4ToUTF8String
		#define UCSToUTF8StringAlloc	UCS4ToUTF8StringAlloc
		#define IS_ASCII_UNICODE(a) (a & 0xFFFFFF00) == 0
	#else
		typedef UCS2String				UCSStringType;
		#define UCSToAsciiString		UCS2ToAsciiString	// all other are 2 bytes
		#define AsciiToUCSString		AsciiToUCS2String
		#define UTF8ToUCSString			UTF8ToUCS2String
		#define UTF8ToUCSStringAlloc	UTF8ToUCS2StringAlloc
		#define UTF8ToUCSStringLen		UTF8ToUCS2StringLen
		#define UTF8ToUCSStringAllocLen UTF8ToUCS2StringAllocLen
		#define UTF8ToUCSStringLen2		UTF8ToUCS2StringLen2
		#define UCSToUTF8String			UCS2ToUTF8String
		#define UCSToUTF8StringAlloc	UCS2ToUTF8StringAlloc
		#define IS_ASCII_UNICODE(a) (a & 0xFF00) == 0
	#endif
#endif // functions 

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define UTF8_FOLLOW_BYTE_TAG	0x80    //:1000 0000    // Identifies non-leading bytes of UTF8String
#define UTF8_TWO_BYTE_TAG		0xC0	//:1100 0000    // Identifies start of Two-byte UTF8String
#define UTF8_THREE_BYTE_TAG		0xE0	//:1110 0000    // Identifies start of Three-byte UTF8String
#define UTF8_FOUR_BYTE_TAG		0xF0	//:1111 0000    // Identifies start of Four-byte UTF8String
#define UTF8_FIVE_BYTE_TAG		0xF8	//:1111 1000    // Identifies start of Five-byte UTF8String
#define UTF8_SIX_BYTE_TAG		0xFC	//:1111 1100    // Identifies start of Six-byte UTF8String

#define UTF8_FOLLOW_BYTE_MASK	0x3F    //:0011 1111    // The value bits in a follow byte
#define UTF8_TWO_BYTE_MASK		0x1F    //:0001 1111    // The value bits in a two byte tag
#define UTF8_THREE_BYTE_MASK	0x0F    //:0000 1111    // The value bits in a three byte tag
#define UTF8_FOUR_BYTE_MASK		0x0F    //:0000 0111    // The value bits in a four byte tag
#define UTF8_FIVE_BYTE_MASK		0x0F    //:0000 0011    // The value bits in a five byte tag
#define UTF8_SIX_BYTE_MASK		0x0F    //:0000 0001    // The value bits in a six byte tag

#define UTF8_IS_FOLLOW_BYTE(a)   (((UTF8ByteType)a & UTF8_TWO_BYTE_TAG)==UTF8_FOLLOW_BYTE_TAG)

//#define UTF8_IS_SINGLE_BYTE(a)	((UTF8ByteType)a < 0x80)	// 0-127
//#define UTF8_IS_TWO_BYTE(a)		(((UTF8ByteType)a & UTF8_THREE_BYTE_TAG) == UTF8_TWO_BYTE_TAG)
//#define UTF8_IS_THREE_BYTE(a)	(((UTF8ByteType)a & UTF8_FOUR_BYTE_TAG) == UTF8_THREE_BYTE_TAG)	
//#define UTF8_IS_FOUR_BYTE(a)	(((UTF8ByteType)a & UTF8_FIVE_BYTE_TAG) == UTF8_FOUR_BYTE_TAG)
//#define UTF8_IS_FIVE_BYTE(a)	(((UTF8ByteType)a & UTF8_SIX_BYTE_TAG) == UTF8_FIVE_BYTE_TAG)	
//#define UTF8_IS_SIX_BYTE(a)		(((UTF8ByteType)a & 0xFE) == UTF8_SIX_BYTE_TAG)

#define UTF8_IS_SINGLE_BYTE(a)	((UTF8ByteType)a & 0x80 ) == 0	// 0-127
#define UTF8_IS_TWO_BYTE(a)		((UTF8ByteType)a & UTF8_THREE_BYTE_TAG) == UTF8_TWO_BYTE_TAG
#define UTF8_IS_THREE_BYTE(a)	((UTF8ByteType)a & UTF8_FOUR_BYTE_TAG) == UTF8_THREE_BYTE_TAG	
#define UTF8_IS_FOUR_BYTE(a)	((UTF8ByteType)a & UTF8_FIVE_BYTE_TAG) == UTF8_FOUR_BYTE_TAG
#define UTF8_IS_FIVE_BYTE(a)	((UTF8ByteType)a & UTF8_SIX_BYTE_TAG) == UTF8_FIVE_BYTE_TAG	
#define UTF8_IS_SIX_BYTE(a)		((UTF8ByteType)a & 0xFE) == UTF8_SIX_BYTE_TAG


#define	REPLACE_INVALID_CHAR     '?' 	// Replace invalid UTF8 chars with this

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Prototypes
//		'_' denotes internal use functions
//int _ReadUCS2CharFromUTF8String (const UTF8String  theUTF8String, int theUTF8ByteLength, UCS2Char* theUnicodeChar);

//int _UCS2ToUTF8ConversionLengthOnly (const UCS2String theUCS2String);
int _UTF8ToUCS2ConversionLengthOnly (const UTF8String theUTF8String, int theUTF8StringLength);

//int _ReadUCS4CharFromUTF8String (const UTF8String  theUTF8String, int theUTF8ByteLength, UCS2Char* theUnicodeChar);
//int _UCS4ToUTF8ConversionLengthOnly (const UCS4String theUCS4String);
//int _UTF8ToUCS4ConversionLengthOnly (const UTF8String theUTF8String, int theUTF8StringLength);
	
// Convert string types
int UCS2CharToUTF8String(UCS2Char theUCS2Char, UTF8String theUTF8String);
int UCS4CharToUTF8String(UCS4Char theUCS4Char, UTF8String theUTF8String);

int AsciiToUTF8String(const char*      theAsciiString, UTF8String theUTF8String );
int UTF8ToAsciiString(const UTF8String theUTF8String,  char*      theAsciiString);

int UCS2ToUTF8String (const UCS2String theUCS2String,  UTF8String theUTF8String );
int UCS4ToUTF8String (const UCS4String theUCS4String,  UTF8String theUTF8String );

int UTF8ToUCS2String (const UTF8String theUTF8String,  UCS2String theUCS2String );
int UTF8ToUCS4String (const UTF8String theUTF8String,  UCS4String theUCS4String );

int UCS2ToAsciiString(const UCS2String theUCS2String,  char*      theAsciiString);
int UCS4ToAsciiString(const UCS4String theUCS4String,  char*      theAsciiString);

int AsciiToUCS2String(const char*      theAsciiString, UCS2String theUCS2String );
int AsciiToUCS4String(const char*      theAsciiString, UCS4String theUCS4String );
	
// Convert with maximum buffer length
// similar to strncpy

int UTF8ToUCS2StringLen(const UTF8String theUTF8String, UCS2String theUCS2String, int theMaxUCS2Length);
int UTF8ToUCS4StringLen(const UTF8String theUTF8String, UCS4String theUCS4String, int theMaxUCS4Length);

int UTF8ToUCS2StringLen2(const UTF8String theUTF8String, int theUTF8Length, UCS2String theUCS2String, int theMaxUCS2Length);
int UTF8ToUCS4StringLen2(const UTF8String theUTF8String, int theUTF8Length, UCS4String theUCS4String, int theMaxUCS4Length);
	
// Convert a string, allocate space for the new string
UTF8String UCS2ToUTF8StringAlloc(const UCS2String theUCS2String);
UTF8String UCS4ToUTF8StringAlloc(const UCS4String theUCS4String);

UCS2String UTF8ToUCS2StringAlloc(const UTF8String theUTF8String);
UCS4String UTF8ToUCS4StringAlloc(const UTF8String theUTF8String);

UCS2String UTF8ToUCS2StringAllocLen(const UTF8String theUTF8String, int theUTF8Length);
UCS4String UTF8ToUCS4StringAllocLen(const UTF8String theUTF8String, int theUTF8Length);
	
// Convert an array of strings, allocate space for the new strings
UTF8String* UCS2ToUTF8StringArrayAlloc(const UCS2String* theUCS2StringArray, int theNumStrings);
UTF8String* UCS4ToUTF8StringArrayAlloc(const UCS4String* theUCS4StringArray, int theNumStrings);

UCS2String* UTF8ToUCS2StringArrayAlloc(const UTF8String* theUTF8StringArray, int theNumStrings);
UCS4String* UTF8ToUCS4StringArrayAlloc(const UTF8String* theUTF8StringArray, int theNumStrings);

// Safely copy string "src" into destination "dest", never writing more than
// "size" characters. The destination string will always be NUL-terminated.
// For example, if "size" is 64 and "src" points to a string longer than 64
// characters, the first 63 bytes will be copied to "dst" - followed by a NUL byte.
size_t gsiSafeStrcpyA(char *dst, const char *src, size_t size);
size_t gsiSafeStrcpyW(wchar_t *dst, const wchar_t *src, size_t size);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} // extern "C"
#endif

#endif // __STRINGUTIL_H__

