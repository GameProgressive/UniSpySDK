///////////////////////////////////////////////////////////////////////////////
// File:	gsStringUtil.c
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.
// ------------------------------------
// Conversion Utility for ASCII, UTF8 and USC2 (Unicode) character sets
//
// See RFC2279 for reference

#include "gsCommon.h"
#include "gsStringUtil.h"

#ifdef __cplusplus
extern "C" {
#endif	

///////////////////////////////////////////////////////////////////////////////
// _ReadUCS2CharFromUTF8String
// Summary
//          Reads UCS2 character from UTF8String
// Parameters 
//			theUTF8String	: [in] UTF8String, doesn't need to be null terminated
//			theMaxBytes     : [in] Maximum number of bytes to read (not UTF8 characters)
//			theUCS2Char		: [out] The 2 byte UCS2 equivalent
// Return
//		0 if error
//		number of bytes read from theUTF8String
//
// Remarks
//		If theUTF8String is invalid, theUnicodeChar will be set to '?'
//		Function is designed for convenient parsing of UTF8 data streams
//		Because data is routed through an ASCII stream prior to this function being
//		called, embedded NULLs are stripped and hence, this function does not check for them
//		For example, the UTF-8 byte :1000 0000, would convert to a UCS2 NULL character
//		If this appeared in the middle of a stream, it could cause undesired operation
///////////////////////////////////////////////////////////////////////////////
static int _ReadUCS2CharFromUTF8String(const UTF8String theUTF8String, int theMaxBytes, UCS2Char* theUnicodeCharOut)
{
#ifndef _PS2
	GS_ASSERT(theUnicodeCharOut != NULL);
#endif

	if (theMaxBytes == 0)
	{
		// assert?
		*theUnicodeCharOut = (UCS2Char)REPLACE_INVALID_CHAR;
		return 0; // not enough data
	}

	// Check for normal ascii range (includes NULL terminator)
	if (UTF8_IS_SINGLE_BYTE(theUTF8String[0]))
	{
		// ASCII, just copy the value
		*theUnicodeCharOut = (UCS2Char)theUTF8String[0];
		return 1;
	}

	// Check for 2 byte UTF8
	else if (UTF8_IS_TWO_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 2)
		{
			*theUnicodeCharOut = (UCS2Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}

		// Make sure the second byte is valid 
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]))
		{
			// Construct 11 bit unicode character
			//		5 value bits from first UTF8Byte			(:000ABCDE)
			//		plus 6 value bits from the second UTF8Byte	(:00FGHIJK)
			//	Store as (:0000 0ABC DEFG HIJK)
			*theUnicodeCharOut  = (UCS2Char)(((theUTF8String[0] & UTF8_TWO_BYTE_MASK) << 6) +
							  	  ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK)));
			return 2;
		}
	}

	// Check for 3 byte UTF8
	else if (UTF8_IS_THREE_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 3)
		{
			*theUnicodeCharOut = (UCS2Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}

		// Make sure the second and third bytes are valid
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]) &&
			UTF8_IS_FOLLOW_BYTE(theUTF8String[2]))
		{
			// Construct 16 bit unicode character
			//		4 value bits from first UTF8Byte			(:0000ABCD)
			//		plus 6 value bits from the second UTF8Byte	(:00EFGHIJ)
			//		plus 6 value bits from the third  UTF8Byte	(:00KLMNOP)
			//	Store as (:ABCD EFGH IJKL MNOP)
			*theUnicodeCharOut  =	(UCS2Char)(((theUTF8String[0] & UTF8_THREE_BYTE_MASK) << 12) +
								((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK) << 6) +
								((theUTF8String[2] & UTF8_FOLLOW_BYTE_MASK)));
			return 3;	
		}
	}

	// Invalid character, replace with '?' and return false
	*theUnicodeCharOut = (UCS2Char)REPLACE_INVALID_CHAR;

	// The second byte could have been the start of a new valid UTF8 character
	// so we can only safely discard one invalid character
	return 1; 
}

///////////////////////////////////////////////////////////////////////////////
// _ReadUCS4CharFromUTF8String
// Summary
//          Reads UCS4 character from UTF8String
// Parameters 
//			theUTF8String	: [in] UTF8String, doesn't need to be null terminated
//			theMaxBytes     : [in] Maximum number of bytes to read (not UTF8 characters)
//			theUCS4Char		: [out] The 4 byte UCS4 equivalent
// Return
//		0 if error
//		number of bytes read from theUTF8String
//
// Remarks
//		If theUTF8String is invalid, theUnicodeChar will be set to '?'
//		Function is designed for convenient parsing of UTF8 data streams
//		Because data is routed through an ASCII stream prior to this function being
//		called, embedded NULLs are stripped and hence, this function does not check for them
//		For example, the UTF-8 byte :1000 0000, would convert to a UCS4 NULL character
//		If this appeared in the middle of a stream, it could cause undesired operation
///////////////////////////////////////////////////////////////////////////////
static int _ReadUCS4CharFromUTF8String(const UTF8String theUTF8String, int theMaxBytes, UCS4Char* theUnicodeCharOut)
{
#ifndef _PS2
	GS_ASSERT(theUnicodeCharOut != NULL);
#endif
	
	if (theMaxBytes == 0)
	{
		// assert?
		*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
		return 0; // not enough data
	}
	
	// Check for normal ascii range (includes NULL terminator)
	if (UTF8_IS_SINGLE_BYTE(theUTF8String[0]))
	{
		// ASCII, just copy the value
		*theUnicodeCharOut = (UCS4Char)theUTF8String[0];
		return 1;
	}
	
	// Check for 2 byte UTF8
	else if (UTF8_IS_TWO_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 2)
		{
			*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}
		
		// Make sure the second byte is valid 
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]))
		{
			// Construct 11 bit unicode character
			*theUnicodeCharOut  = (UCS4Char) (((theUTF8String[0] & UTF8_TWO_BYTE_MASK) << 6) +
											 ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK)));
			return 2;
		}
	}
	
	// Check for 3 byte UTF8
	else if (UTF8_IS_THREE_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 3)
		{
			*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}
		
		// Make sure the second and third bytes are valid
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]) && UTF8_IS_FOLLOW_BYTE(theUTF8String[2]))
		{
			*theUnicodeCharOut  =	(UCS4Char)(((theUTF8String[0] & UTF8_THREE_BYTE_MASK) << 12) +
											  ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK) << 6) +
											  ((theUTF8String[2] & UTF8_FOLLOW_BYTE_MASK)));
			return 3;	
		}
	}
	
	// Check for 4 byte UTF8
	else if (UTF8_IS_FOUR_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 4)
		{
			*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}
			
		// Make sure the second and third bytes are valid
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[2]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[3]))
		{
			*theUnicodeCharOut  =	(UCS4Char)(((theUTF8String[0] & UTF8_THREE_BYTE_MASK) << 18) +
											  ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK) << 12) +
											  ((theUTF8String[2] & UTF8_FOLLOW_BYTE_MASK) << 6) +
											  ((theUTF8String[3] & UTF8_FOLLOW_BYTE_MASK)));
			return 4;	
		}
	}

	// Check for 5 byte UTF8
	else if (UTF8_IS_FIVE_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 5)
		{
			*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}
			
		// Make sure the second and third bytes are valid
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[2]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[3]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[4]))
		{
			*theUnicodeCharOut  =	(UCS4Char)(((theUTF8String[0] & UTF8_THREE_BYTE_MASK) << 24) +
											  ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK) << 18) +
											  ((theUTF8String[2] & UTF8_FOLLOW_BYTE_MASK) << 12) +
											  ((theUTF8String[3] & UTF8_FOLLOW_BYTE_MASK) << 6) +
											  ((theUTF8String[4] & UTF8_FOLLOW_BYTE_MASK)));
			return 5;	
		}
	}
		
	// Check for 6 byte UTF8
	else if (UTF8_IS_SIX_BYTE(theUTF8String[0]))
	{
		if (theMaxBytes < 6)
		{
			*theUnicodeCharOut = (UCS2Char)REPLACE_INVALID_CHAR;
			return 0; // not enough data
		}
			
		// Make sure the second and third bytes are valid
		if (UTF8_IS_FOLLOW_BYTE(theUTF8String[1]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[2]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[3]) &&
			UTF8_IS_FOLLOW_BYTE(theUTF8String[4]) && 
			UTF8_IS_FOLLOW_BYTE(theUTF8String[5]))
		{
			*theUnicodeCharOut  =	(UCS4Char)(((theUTF8String[0] & UTF8_THREE_BYTE_MASK) << 30) +
											  ((theUTF8String[1] & UTF8_FOLLOW_BYTE_MASK) << 24) +
											  ((theUTF8String[2] & UTF8_FOLLOW_BYTE_MASK) << 18) +
											  ((theUTF8String[3] & UTF8_FOLLOW_BYTE_MASK) << 12) +
											  ((theUTF8String[4] & UTF8_FOLLOW_BYTE_MASK) << 6) +
											  ((theUTF8String[5] & UTF8_FOLLOW_BYTE_MASK)));
			return 6;	
		}
	}
		
	// Invalid character, replace with '?' and return false
	*theUnicodeCharOut = (UCS4Char)REPLACE_INVALID_CHAR;
	
	// The second byte could have been the start of a new valid UTF8 character
	// so we can only safely discard one invalid character
	return 1; 
}

	
///////////////////////////////////////////////////////////////////////////////
// UCS2CharToUTF8String
// Summary
//          Converts a UCS2 code point to a pre-allocated UTF8 strings
// Parameters 
//          theUCS2Char		: [in]  A 2 byte Unicode code point
//          theUTF8String	: [out] The 1-3 byte UTF8 equivalent
// Return
//		length of theUTF8String in bytes; from 1 to 3
//
// Remarks
//		Memory pre-allocated for theUTF8String needs to be at least 3 bytes
//		theUTF8String is NOT NULL terminated
///////////////////////////////////////////////////////////////////////////////	
int UCS2CharToUTF8String(UCS2Char theUCS2Char, UTF8String theUTF8String)
{
#ifndef _PS2
	GS_ASSERT(theUTF8String != NULL);
#endif

	// Screen out simple ascii (includes NULL terminator)
	if (theUCS2Char <= 0x7F)
	{
		// 0-7 bit unicode, copy stright over
		theUTF8String[0] = (char)(UTF8ByteType)theUCS2Char;
		return 1;
	}
	else if (theUCS2Char <= 0x07FF)
	{
		// 8-11 bits unicode, store as two byte UTF8
		// :00000ABC DEFGHIJK
		// :110ABCDE 10FGHIJK
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_TWO_BYTE_TAG | (theUCS2Char >> 6));				// Store the upper 5/11 bits as 0x110xxxxx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | (theUCS2Char & UTF8_FOLLOW_BYTE_MASK));	// Store the lower 6 bits as 0x10xxxxxx
		return 2;
	}
	else
	{
		// 12-16 bits unicode, store as three byte UTF8
		// :ABCDEFGH IJKLMNOP
		// :1110ABCD 10EFGHIJ 10KLMNOP
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_THREE_BYTE_TAG |  (theUCS2Char >> 12));					// Store the upper 4/16 bits as 0x1110xxxx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS2Char >> 6) & UTF8_FOLLOW_BYTE_MASK));	// Store the 5th-10th bits as 0x10xxxxxx
		theUTF8String[2] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS2Char) & UTF8_FOLLOW_BYTE_MASK));			// Store the last 6 bits as 0x10xxxxxx
		return 3;
	}
}

///////////////////////////////////////////////////////////////////////////////
// UCS4CharToUTF8String
// Summary
//          Converts a UCS4 code point to a pre-allocated UTF8 strings
// Parameters 
//          theUCS4Char		: [in]  A 4 byte Unicode code point
//          theUTF8String	: [out] The 1-6 byte UTF8 equivalent
// Return
//		length of theUTF8String in bytes; from 1 to 6
//
// Remarks
//		Memory pre-allocated for theUTF8String needs to be at least 6 bytes
//		theUTF8String is NOT NULL terminated
///////////////////////////////////////////////////////////////////////////////	
int UCS4CharToUTF8String(UCS4Char theUCS4Char, UTF8String theUTF8String)
{
#ifndef _PS2
	GS_ASSERT(theUTF8String != NULL);
#endif
	
	// Screen out simple ascii (includes NULL terminator)
	if (theUCS4Char < 0x80)
	{
		// 0-7 bit unicode, copy stright over
		theUTF8String[0] = (char)(UTF8ByteType)theUCS4Char;
		return 1;
	}
	else if (theUCS4Char < 0x800)
	{
		// 8-11 bits unicode, store as two byte UTF8
		// :0000 0ABC DEFG HIJK
		// :110A BCDE 10FG HIJK
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_TWO_BYTE_TAG    | (theUCS4Char >> 6));						// Store the upper 5/11 bits as 0x110xxxxx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | (theUCS4Char & UTF8_FOLLOW_BYTE_MASK));	// Store the lower 6 bits as 0x10xxxxxx
		return 2;
	}
	else if (theUCS4Char < 0x10000)
	{
		// 12-16 bits unicode, store as three byte UTF8
		// :ABCD EFGH IJKL MNOP
		// :1110 ABCD 10EF GHIJ 10KL MNOP
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_THREE_BYTE_TAG  |  (theUCS4Char >> 12));							// Store the upper 4/16 bits (16-13) as 0x1110xxxx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 6) & UTF8_FOLLOW_BYTE_MASK));	// Store the (12-7) bits as 0x10xxxxxx
		theUTF8String[2] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char) & UTF8_FOLLOW_BYTE_MASK));		// Store the last 6 bits (6-1) as 0x10xxxxxx
		return 3;
	}
	else if (theUCS4Char < 0x200000)
	{
		// 17-21 bits unicode, store as four byte UTF8
		// :000A BCDE FGHI JKLM NOPQ RSTU
		// :1111 0ABC 10DE FGHI 10JK LMNO 10PQ RSTU
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_FOUR_BYTE_TAG   |  (theUCS4Char >> 18 & 0x7));						// Store the (21-19) bits as 0x11110xxx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 12) & UTF8_FOLLOW_BYTE_MASK));	// Store the (18-13) bits as 0x10xxxxxx
		theUTF8String[2] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 6) & UTF8_FOLLOW_BYTE_MASK));	// Store the (12-7) bits as 0x10xxxxxx
		theUTF8String[3] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char) & UTF8_FOLLOW_BYTE_MASK));		// Store the last 6 bits (6-1) as 0x10xxxxxx
		return 4;
	}
	else if (theUCS4Char < 0x4000000)
	{
		// 22-26 bits unicode, store as five byte UTF8
		// :00AB CDEF GHIJ KLMN OPQR STUV WXYZ
		// :1111 10AB 10CD EFGH 10IJ KLMN 10OP QRST 10UV WXYZ
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_FIVE_BYTE_TAG   |  (theUCS4Char >> 24 & 0x3));						// Store the (26-25) bits as 0x111100xx
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 18) & UTF8_FOLLOW_BYTE_MASK));	// Store the (24-19) bits as 0x10xxxxxx
		theUTF8String[2] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 12) & UTF8_FOLLOW_BYTE_MASK));	// Store the (18-13) bits as 0x10xxxxxx
		theUTF8String[3] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 6) & UTF8_FOLLOW_BYTE_MASK));	// Store the (12-7) bits as 0x10xxxxxx
		theUTF8String[4] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char) & UTF8_FOLLOW_BYTE_MASK));		// Store the last 6 bits (6-1) as 0x10xxxxxx
		return 5;
	}
	else
	{
		// 27-31 bits unicode, store as five byte UTF8
		// :0ABC DEFG HIJK LMNO PQRS TUVW XYZa bcde 
		// :1111 110A 10BC DEFG 10HI JKLM 10NO PQRS 10TU VWXY 10Za bcde
		theUTF8String[0] = (char)(UTF8ByteType)(UTF8_SIX_BYTE_TAG    |  (theUCS4Char >> 30 & 0x1));						// Store the 31 bit as 0x1111000x
		theUTF8String[1] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 24) & UTF8_FOLLOW_BYTE_MASK));	// Store the (30-25) bits as 0x10xxxxxx
		theUTF8String[2] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 18) & UTF8_FOLLOW_BYTE_MASK));	// Store the (24-19) bits as 0x10xxxxxx
		theUTF8String[3] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 12) & UTF8_FOLLOW_BYTE_MASK));	// Store the (18-13) bits as 0x10xxxxxx
		theUTF8String[4] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char >> 6) & UTF8_FOLLOW_BYTE_MASK));	// Store the (12-7) bits as 0x10xxxxxx
		theUTF8String[5] = (char)(UTF8ByteType)(UTF8_FOLLOW_BYTE_TAG | ((theUCS4Char) & UTF8_FOLLOW_BYTE_MASK));		// Store the last 6 bits (6-1) as 0x10xxxxxx
		return 6;
	}
	
}
	

///////////////////////////////////////////////////////////////////////////////
// AsciiToUTF8String
// Summary
//          Converts a NULL terminated ASCII string to a NULL terminated UTF8 string 
// Parameters
//          theAsciiString		: [in] A pointer to the UCS2String string
//          theUTF8String		: [out] A pointer to the NULL terminated UTF8 string
// Return
//		Number of characters in theUTF8String
//
// Remarks
//		Memory pre-allocated for theUTF8String needs to be as large as the theAsciiString
///////////////////////////////////////////////////////////////////////////////			
int AsciiToUTF8String(const char* theAsciiString, UTF8String theUTF8String)
{
	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theAsciiString == NULL)
	{
		*theUTF8String = 0;
		return 1;
	}
	else
	{
		// Copy the string, keeping track of length
		int aLength = 0;
		while (*theAsciiString != '\0')
		{
			*(theUTF8String++) = *(theAsciiString++);
			aLength++;
		} 

		// Append the null
		*theUTF8String = '\0';
		aLength++;

		return aLength;
	}
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToAsciiString
// Summary
//          Converts a NULL terminated UTF8 string to it's NULL terminated ASCII equivalent 
// Parameters
//          theUTF8String		: [in] A pointer to the NULL terminated UTF8 string
//          theAsciiString		: [out] A pointer to the ASCII string
// Return
//		Number of ASCII characters in theAsciiString
//
// Remarks
//		Memory pre-allocated for theAsciiString needs to be as large as the UTF8String
//		Any invalid UTF8 string will result in a '?' char
///////////////////////////////////////////////////////////////////////////////			
int UTF8ToAsciiString(const UTF8String theUTF8String, char* theAsciiString)
{
	// Strip non-ascii characters and replace with REPLACE_INVALID_CHAR
	const unsigned char* anInStream = (const unsigned char*)theUTF8String;
	int  aNumBytesWritten = 0;

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUTF8String == NULL)
	{
		*theAsciiString = 0;
		return 1;
	}

	// Keep extracting characters until we get a '\0'
	while (*anInStream != '\0')
	{
		if (UTF8_IS_SINGLE_BYTE(*anInStream))
			theAsciiString[aNumBytesWritten++] = (char)*anInStream;
		else
			theAsciiString[aNumBytesWritten++] = REPLACE_INVALID_CHAR;

		// move to next character
		anInStream++;
	}

	// Append the '\0'
	theAsciiString[aNumBytesWritten++] = '\0';
	return aNumBytesWritten;
}

///////////////////////////////////////////////////////////////////////////////
// UCS2ToUTF8String
// Summary
//			Convert a UCS2 (Unicode) string to its UTF8 equivalent
// Parameters 
//			theUCS2String	: [in] A pointer to NULL terminated UCS2 string
//          theUTF8String	: [out] A pointer to pre-allocated UTF8 string
// Return
//		length of theUTF8String and sets theUTF8String
//
// Remarks
//		Invalid UTF8 characters are replaced with '?'
//		Memory pre-allocated for theUTF8String needs to be up to 1.5 times the size of theUCS2String
//      This means that for each UCS2 character, up to 6 UTF8 characters may be generated
///////////////////////////////////////////////////////////////////////////////
int UCS2ToUTF8String(const UCS2String theUCS2String, UTF8String theUTF8String)
{
	int	aTotalBytesWritten	= 0;
	unsigned int	aUTF8CharLength		= 0;
	const UCS2Char*	anInStream			= theUCS2String;
	unsigned char*	anOutStream			= (unsigned char*)theUTF8String;

	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUCS2String == NULL)
	{
		*anOutStream = 0x00;
		return 1;
	}

	// Loop until we reach a NULL terminator
	while(*anInStream != 0)
	{
		aUTF8CharLength = (unsigned int) UCS2CharToUTF8String(*anInStream, (UTF8String)anOutStream);

		// Move out stream to next character position
		anOutStream += aUTF8CharLength;

		// Move to next UCS2 character
		anInStream++;

		// Record number of bytes written
		aTotalBytesWritten += aUTF8CharLength;
	}
	
	// Copy over the null terminator
	*anOutStream = '\0';
	aTotalBytesWritten++;

	return aTotalBytesWritten;
}

///////////////////////////////////////////////////////////////////////////////
// UCS4ToUTF8String
// Summary
//			Convert a UCS4 (Unicode) string to its UTF8 equivalent
// Parameters 
//			theUCS4String	: [in] A pointer to NULL terminated UCS4 string
//          theUTF8String	: [out] A pointer to pre-allocated UTF8 string
// Return
//		length of theUTF8String and sets theUTF8String
//
// Remarks
//		Invalid UTF8 characters are replaced with '?'
//		Memory pre-allocated for theUTF8String needs to be up to 1.5 times the size of theUCS4String
//      This means that for each UCS4 character, up to 6 UTF8 characters may be generated
///////////////////////////////////////////////////////////////////////////////
int UCS4ToUTF8String(const UCS4String theUCS4String, UTF8String theUTF8String)
{
	int	aTotalBytesWritten	= 0;
	unsigned int	aUTF8CharLength		= 0;
	const UCS4Char*	anInStream			= theUCS4String;
	unsigned char*	anOutStream			= (unsigned char*)theUTF8String;
	
	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUCS4String == NULL)
	{
		*anOutStream = 0x00;
		return 1;
	}
	
	// Loop until we reach a NULL terminator
	while(*anInStream != 0)
	{
		aUTF8CharLength = (gsi_u32) UCS4CharToUTF8String(*anInStream, (UTF8String)anOutStream);
		
		// Move out stream to next character position
		anOutStream += aUTF8CharLength;
		
		// Move to next UCS4 character
		anInStream++;
		
		// Record number of bytes written
		aTotalBytesWritten += aUTF8CharLength;
	}
	
	// Copy over the null terminator
	*anOutStream = '\0';
	aTotalBytesWritten++;
	
	return aTotalBytesWritten;
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2String
// Summary
//			Convert a UTF8 string to it's UCS2 (Unicode) equivalent
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string
//			theUCS2String	: [out] A pointer to pre-allocated UCS2 string
// Return
//		length of theUCS2String and sets theUCS2String
//
// Remarks
//		Invalid UTF8 characters are replaced with '?'
//		Memory pre-allocated for theUCS2String needs to be as large as the UTF8String
///////////////////////////////////////////////////////////////////////////////
int UTF8ToUCS2String(const UTF8String theUTF8String, UCS2String theUCS2String)
{
	return UTF8ToUCS2StringLen(theUTF8String, theUCS2String, (int)strlen((char *)theUTF8String));
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4String
// Summary
//			Convert a UTF8 string to it's UCS4 (Unicode) equivalent
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string
//			theUCS4String	: [out] A pointer to pre-allocated UCS4 string
// Return
//		length of theUCS4String and sets theUCS4String
//
// Remarks
//		Invalid UTF8 characters are replaced with '?'
//		Memory pre-allocated for theUCS4String needs to be as large as the UTF8String
///////////////////////////////////////////////////////////////////////////////
int UTF8ToUCS4String(const UTF8String theUTF8String, UCS4String theUCS4String)
{
	return UTF8ToUCS4StringLen(theUTF8String, theUCS4String, (gsi_i32)strlen((char *)theUTF8String));
}
		

///////////////////////////////////////////////////////////////////////////////
// _UTF8ToUCS2ConversionLengthOnly
// Summary
//			Calculate the size needed to convert a UTF8String to a UCS2String
// Parameters 
//          theUTF8String		: [in] A pointer to NULL terminated UTF8 string
//			theUTF8StringLength	: [in] char length of UTF8 string
// Return
//		0 if error
//		Otherwise, length of the UC2 string that would be created
//
// Remarks
//		Invalid UTF8 characters are treated as 1 byte ('?')
///////////////////////////////////////////////////////////////////////////////
int _UTF8ToUCS2ConversionLengthOnly(const UTF8String theUTF8String, int theUTF8StringLength)
{
	int length = 0;
	const UTF8String theReadPos = theUTF8String;
	int bytesLeft = theUTF8StringLength;

	GS_ASSERT(theUTF8String != NULL);

	if (theUTF8String == NULL)
		return 0;

	while (*theReadPos != '\0' && bytesLeft > 0)
	{
		// Check for valid two byte string
		if (UTF8_IS_TWO_BYTE(theReadPos[0]) && 
			bytesLeft >= 2 &&
			UTF8_IS_FOLLOW_BYTE(theReadPos[1]))
		{
			theReadPos += 2;
			bytesLeft -= 2;
		}

		// Check for valid three byte string
		else if (UTF8_IS_THREE_BYTE(theReadPos[0]) && 
				 bytesLeft >= 3 &&
				 UTF8_IS_FOLLOW_BYTE(theReadPos[1]) &&
				 UTF8_IS_FOLLOW_BYTE(theReadPos[2]))
		{
			theReadPos += 3;
			bytesLeft -= 3;
		}
		// Anything else means one UTF8 character read from the buffer
		else
		{
			theReadPos++;
			bytesLeft--;
		}

		// Increment the length of the UCS2 string
		length++;
	}

	// don't count the null as a character, this conforms
	// with ANSI strlen functions
	return length;
}


///////////////////////////////////////////////////////////////////////////////
// _UTF8ToUCS4ConversionLengthOnly
// Summary
//			Calculate the size needed to convert a UTF8String to a UCS4String
// Parameters 
//          theUTF8String		: [in] A pointer to NULL terminated UTF8 string
//			theUTF8StringLength	: [in] char length of UTF8 string
// Return
//		0 if error
//		Otherwise, length of the UCS4 string that would be created
//
// Remarks
//		Invalid UTF8 characters are treated as 1 byte ('?')
///////////////////////////////////////////////////////////////////////////////
/*static int _UTF8ToUCS4ConversionLengthOnly(const UTF8String theUTF8String, int theUTF8StringLength)
{
	int length = 0;
	
	const UTF8String theReadPos = theUTF8String;
	int bytesLeft = theUTF8StringLength;
	
	GS_ASSERT(theUTF8String != NULL);

	if (theUTF8String == NULL)
		return 0;
	
	while (*theReadPos != '\0' && bytesLeft > 0)
	{
		// Check for valid one byte string
		if( UTF8_IS_SINGLE_BYTE(theReadPos[0]))
		{
			theReadPos++;
			bytesLeft--;
		}
				
		// Check for valid two byte string
		else if (UTF8_IS_TWO_BYTE(theReadPos[0]) && bytesLeft >= 2 && UTF8_IS_FOLLOW_BYTE(theReadPos[1]))
		{
			theReadPos += 2;
			bytesLeft -= 2;
		}
		
		// Check for valid three byte string
		else if (UTF8_IS_THREE_BYTE(theReadPos[0]) && bytesLeft >= 3 &&
				 UTF8_IS_FOLLOW_BYTE(theReadPos[1]) && UTF8_IS_FOLLOW_BYTE(theReadPos[2]))
		{
			theReadPos += 3;
			bytesLeft -= 3;
		}

		// Check for valid four byte string
		else if (UTF8_IS_FOUR_BYTE(theReadPos[0]) && bytesLeft >= 4 && UTF8_IS_FOLLOW_BYTE(theReadPos[1]) && 
				 UTF8_IS_FOLLOW_BYTE(theReadPos[2]) && UTF8_IS_FOLLOW_BYTE(theReadPos[3]))
		{
			theReadPos += 4;
			bytesLeft -= 4;
		}

		// Check for valid five byte string
		else if (UTF8_IS_FIVE_BYTE(theReadPos[0]) && bytesLeft >= 4 && UTF8_IS_FOLLOW_BYTE(theReadPos[1]) && 
				 UTF8_IS_FOLLOW_BYTE(theReadPos[2]) && UTF8_IS_FOLLOW_BYTE(theReadPos[3]) && UTF8_IS_FOLLOW_BYTE(theReadPos[4]))
		{
			theReadPos += 5;
			bytesLeft -= 5;
		}

		// Check for valid six byte string
		else if (UTF8_IS_SIX_BYTE(theReadPos[0]) && bytesLeft >= 4 && UTF8_IS_FOLLOW_BYTE(theReadPos[1]) && UTF8_IS_FOLLOW_BYTE(theReadPos[2]) &&
				 UTF8_IS_FOLLOW_BYTE(theReadPos[3]) && UTF8_IS_FOLLOW_BYTE(theReadPos[4]) && UTF8_IS_FOLLOW_BYTE(theReadPos[5]))
		{
			theReadPos += 6;
			bytesLeft -= 6;
		}

		// Anything else will become '?'
		else
		{
			theReadPos++;
			bytesLeft--;
		}
		
		// Increment the length of the UCS4 string
		length++;
	}
	
	// don't count the null as a character, this conforms
	// with ANSI strlen functions
	return length;
}*/

///////////////////////////////////////////////////////////////////////////////
// _UCS2ToUTF8ConversionLengthOnly
// Summary
//          Calculate the size needed to convert a UCS2String to a UTF8String
// Parameters 
//          theUCS2String	: [in] A pointer to NULL terminated UCS2 string 
// Return
//		0 if error
//		Otherwise, length of theUTF8String that would be created
//
///////////////////////////////////////////////////////////////////////////////
static int _UCS2ToUTF8ConversionLengthOnly(const UCS2String theUCS2String)
{
	int length = 0;
	const UCS2String theReadPos = theUCS2String;

	GS_ASSERT(theUCS2String != NULL);
	
	while (*theReadPos != 0)
	{
		// Values <= 0x7F are single byte ascii
		if (*theReadPos < 0x80)
			length++;
		// Values > 0x7F and <= 0x07FF are two bytes in UTF8
		else if (*theReadPos < 0x800) 
			length += 2;
		// Anything else is 3 bytes of UTF8
		else
			length += 3;

		// Set read pos to right spot (1 more UCS2 Character = 2 bytes)
		theReadPos++;
	}

	// don't count the null as a character, this conforms with ANSI strlen functions
	return length;
}

///////////////////////////////////////////////////////////////////////////////
// _UCS4ToUTF8ConversionLengthOnly
// Summary
//          Calculate the size needed to convert a UCS4String to a UTF8String
// Parameters 
//          theUCS2String	: [in] A pointer to NULL terminated UCS4 string 
// Return
//		0 if error
//		Otherwise, length of theUTF8String that would be created
//
///////////////////////////////////////////////////////////////////////////////
static int _UCS4ToUTF8ConversionLengthOnly(const UCS4String theUCS4String)
{
	int length = 0;
	const UCS4String theReadPos;
	
	GS_ASSERT(theUCS4String != NULL);
	
	for (theReadPos = theUCS4String; *theReadPos; theReadPos++)
	{
		if (*theReadPos < 0x80)				// 1 byte
			length++;						
		else if (*theReadPos < 0x800)		// 2 bytes
			length += 2;
		else if( *theReadPos < 0x10000 ) 	// 3 bytes
			length += 3;
		else if( *theReadPos < 0x200000 ) 	// 4 bytes
			length += 4;
		else if( *theReadPos < 0x4000000 ) 	// 5 bytes
			length +=5;
		else // if >= 0x4000000				   6 bytes
			length +=6;
	}

	// don't count the null as a character, this conforms with ANSI strlen functions
	return length;
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2StringAlloc
// Summary
//          Convert a NULL terminated UTF8String to a memory allocated UCS2String
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string 
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS2String based on the contents of theUTF8String
//
// Remarks
//		Memory for returned UCS2String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UCS2String UTF8ToUCS2StringAlloc(const UTF8String theUTF8String)
{
	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUTF8String == NULL)
		return NULL;

	return UTF8ToUCS2StringAllocLen(theUTF8String, (int)strlen((char *)theUTF8String));
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4StringAlloc
// Summary
//          Convert a NULL terminated UTF8String to a memory allocated UCS4String
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string 
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS4String based on the contents of theUTF8String
//
// Remarks
//		Memory for returned UCS4String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UCS4String UTF8ToUCS4StringAlloc(const UTF8String theUTF8String)
{
	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUTF8String == NULL)
		return NULL;

	return UTF8ToUCS4StringAllocLen(theUTF8String, (int)strlen((char *)theUTF8String));
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2StringAllocLen
// Summary
//          Convert a UTF8String of a given length to a memory allocated UCS2String
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string 
//          theNumStrings	: [in] Number of bytes in theUTF8String
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS2String based on the contents of theUTF8String
//
// Remarks
//		Memory for returned UCS2String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UCS2String UTF8ToUCS2StringAllocLen(const UTF8String theUTF8String, int theUTF8StringLength)
{
	int newLength;
	UCS2String aUCS2String;

	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUTF8String == NULL)
	{
		return NULL;
	}			
	// Find the length of the UCS2 string and allocate a block
	newLength = _UTF8ToUCS2ConversionLengthOnly(theUTF8String, theUTF8StringLength);
	aUCS2String = (UCS2String)gsimalloc(sizeof(UCS2Char)*(newLength + 1));
	
	// Do the conversion
	UTF8ToUCS2StringLen2(theUTF8String, theUTF8StringLength, aUCS2String, newLength);

	// Return the allocated string
	return aUCS2String;
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4StringAllocLen
// Summary
//          Convert a UTF8String of a given length to a memory allocated UCS4String
// Parameters 
//          theUTF8String	: [in] A pointer to NULL terminated UTF8 string 
//          theNumStrings	: [in] Number of bytes in theUTF8String
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS4String based on the contents of theUTF8String
//
// Remarks
//		Memory for returned UCS4String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UCS4String UTF8ToUCS4StringAllocLen(const UTF8String theUTF8String, int theUTF8StringLength)
{
	int newLength;
	UCS4String aUCS4String;

	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUTF8String == NULL)
		return NULL;
				
	// Find the length of the UCS2 string and allocate a block
	newLength = _UTF8ToUCS2ConversionLengthOnly(theUTF8String, theUTF8StringLength);
	aUCS4String = (UCS4String)gsimalloc(sizeof(UCS4Char)*(newLength + 1));

	// Do the conversion
	UTF8ToUCS4StringLen2(theUTF8String, theUTF8StringLength, aUCS4String, newLength);

	// Return the allocated string
	return aUCS4String;
}

///////////////////////////////////////////////////////////////////////////////
// UCS2ToUTF8StringAlloc
// Summary
//          Convert a UCS2String to a memory allocated UTF8String
// Parameters 
//          theUCS2String	: [in] A pointer to NULL terminated UCS2 string 
// Return
//		NULL if error
//		Otherwise, a newly allocated UTF8String based on the contents of theUCS2String
//
// Remarks
//		Memory for returned UTF8String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UTF8String UCS2ToUTF8StringAlloc(const UCS2String theUCS2String)
{
	int newLength;
	UTF8String aUTF8String;

	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUCS2String == NULL)
		return NULL;

	// Find the length of the UCS2 string and allocate a block
	newLength = _UCS2ToUTF8ConversionLengthOnly(theUCS2String);
	aUTF8String	= (UTF8String)gsimalloc(sizeof(char)*(newLength + 1));

	// Do the conversion
	UCS2ToUTF8String(theUCS2String, aUTF8String);

	// Return the allocated string
	return aUTF8String;
}


///////////////////////////////////////////////////////////////////////////////
// UCS4ToUTF8StringAlloc
// Summary
//          Convert a UCS4String to a memory allocated UTF8String
// Parameters 
//          theUCS4String	: [in] A pointer to NULL terminated UCS4 string 
// Return
//		NULL if error
//		Otherwise, a newly allocated UTF8String based on the contents of theUCS4String
//
// Remarks
//		Memory for returned UTF8String must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////
UTF8String UCS4ToUTF8StringAlloc(const UCS4String theUCS4String)
{
	int newLength;
	UTF8String aUTF8String;

	// Allow for NULL here since SDKs allow for NULL string parameters
	if (theUCS4String == NULL)
		return NULL;

	// Find the length of the UCS4 string and allocate a block
	newLength = _UCS4ToUTF8ConversionLengthOnly(theUCS4String);
	aUTF8String	= (UTF8String)gsimalloc(sizeof(char)*(newLength + 1));

	// Do the conversion
	UCS4ToUTF8String(theUCS4String, aUTF8String);

	// Return the allocated string
	return aUTF8String;
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2StringArrayAlloc
// Summary
//          Convert an array of UTF8 strings to a memory allocated array of UCS2 strings
// Parameters 
//          theUTF8StringArray	: [in] A pointer to NULL terminated UTF8 strings 
//          theNumStrings		: [in] Number of strings in theUCS2StringArray
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS2String* based on the contents of theUTF8StringArray
//
// Remarks
//		Memory for returned UCS2String* must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////	
UCS2String* UTF8ToUCS2StringArrayAlloc(const UTF8String* theUTF8StringArray, int theNumStrings)
{
	UCS2String* aUCS2StringArray;
	int stringNum = 0;

	// Allow for NULL here since SDKs allow for NULL string arrays
	if(theUTF8StringArray == NULL || theNumStrings == 0)
		return NULL;

	aUCS2StringArray = (UCS2String*)gsimalloc(sizeof(UCS2String)*theNumStrings);

	while(stringNum < theNumStrings)
	{
		aUCS2StringArray[stringNum] = UTF8ToUCS2StringAlloc(theUTF8StringArray[stringNum]);
		stringNum++;
	}

	return aUCS2StringArray;
}


///////////////////////////////////////////////////////////////////////////////
// UCS2ToUTF8StringArrayAlloc
//
// Convert a UCS2StringArray to a UTF8StringArray, allocate space for the UTF8Strings
//
//  [in]	UCS2StringArray, array of NULL terminated UCS2Strings
//  [in]	theNumStrings, how many strings are in the array
//
//  returns the newly allocated UTF8StringArray
//
//	  Remarks:
//		The callee is responsible for freeing the allocated memory block
///////////////////////////////////////////////////////////////////////////////
UTF8String* UCS2ToUTF8StringArrayAlloc(const UCS2String* theUCS2StringArray, int theNumStrings)
{
	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUCS2StringArray == NULL || theNumStrings == 0)
		return NULL;
	else
	{
		UTF8String* aUTF8StringArray = (UTF8String*)gsimalloc(sizeof(UTF8String)*theNumStrings);

		int stringNum = 0;
		while(stringNum < theNumStrings)
		{
			aUTF8StringArray[stringNum] = UCS2ToUTF8StringAlloc(theUCS2StringArray[stringNum]);
			stringNum++;
		}

		return aUTF8StringArray;
	}
}


///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4StringArrayAlloc
// Summary
//          Convert an array of UTF8 strings to a memory allocated array of UCS4 strings
// Parameters 
//          theUTF8StringArray	: [in] A pointer to NULL terminated UTF8 strings 
//          theNumStrings		: [in] Number of strings in theUCS4StringArray
// Return
//		NULL if error
//		Otherwise, a newly allocated UCS4String* based on the contents of theUTF8StringArray
//
// Remarks
//		Memory for returned UCS4String* must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////	
UCS4String* UTF8ToUCS4StringArrayAlloc(const UTF8String* theUTF8StringArray, int theNumStrings)
{
	int stringNum = 0;
	UCS4String* aUCS4StringArray;

	// Allow for NULL here since SDKs allow for NULL string arrays
	if(theUTF8StringArray == NULL || theNumStrings == 0)
		return NULL;

	aUCS4StringArray = (UCS4String*)gsimalloc(sizeof(UCS4String)*theNumStrings);
	
	while(stringNum < theNumStrings)
	{
		aUCS4StringArray[stringNum] = UTF8ToUCS4StringAlloc(theUTF8StringArray[stringNum]);
		stringNum++;
	}

	return aUCS4StringArray;
}

///////////////////////////////////////////////////////////////////////////////
// UCS4ToUTF8StringArrayAlloc
// Summary
//          Convert a NULL terminated array of UCS4 strings to a memory allocated array of UTF8 strings
// Parameters 
//          theUCS4StringArray	: [in]  A pointer to UCS4String 
//          theNumStrings		: [in] Number of strings in theUCS4StringArray
// Return
//		NULL if error
//		Otherwise, a newly allocated UTF8String* based on the contents of UCS4StringArray
//
// Remarks
//		Memory for returned UTF8String* must be freed by caller when appropriate
///////////////////////////////////////////////////////////////////////////////	
UTF8String* UCS4ToUTF8StringArrayAlloc(const UCS4String* theUCS4StringArray, int theNumStrings)
{
	UTF8String* aUTF8StringArray;
	int stringNum = 0;

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUCS4StringArray == NULL || theNumStrings == 0)
		return NULL;

	aUTF8StringArray = (UTF8String*)gsimalloc( sizeof(UTF8String) * theNumStrings );

	while(stringNum < theNumStrings)
	{
		aUTF8StringArray[stringNum] = UCS4ToUTF8StringAlloc(theUCS4StringArray[stringNum]);
		stringNum++;
	}

	return aUTF8StringArray;
}


///////////////////////////////////////////////////////////////////////////////
// UCS2ToAsciiString
// Summary
//          Converts a NULL terminated a Unicode (2 byte char) string
//			to a NULL terminated ASCII string  
// Parameters 
//          theUCS2String		: [in]  A pointer to the UCS2String string
//          theAsciiString		: [out] A pointer to the ASCII string
// Return
//		Number of ASCII characters in theAsciiString
//
// Remarks
//		Invalid ASCII characters are truncated
//		Memory for theAsciiString must be allocated with its size 
//		being 1/4 the size of theUCS2String
///////////////////////////////////////////////////////////////////////////////	
int UCS2ToAsciiString(const UCS2String theUCS2String, char* theAsciiString)
{
	int length = 0;
	const UCS2String aReadPos = theUCS2String;
	char* aWritePos = theAsciiString;

	GS_ASSERT(theAsciiString != NULL);

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUCS2String == NULL)
	{
		*theAsciiString = '\0';
		return 1;
	}

	// Convert each character until a '\0' is reached
	while(*aReadPos != '\0')
	{
		(*aWritePos++) = (char)(0x00FF & (*aReadPos++));
		length++;
	}

	// append the NULL
	*aWritePos = '\0';
	length++;

	return length;
}

///////////////////////////////////////////////////////////////////////////////
// UCS4ToAsciiString
// Summary
//          Converts a NULL terminated a Unicode (4 byte char) string
//			to a NULL terminated ASCII string  
// Parameters 
//          theUCS4String		: [in]  A pointer to the UCS4String string
//          theAsciiString		: [out] A pointer to the ASCII string
// Return
//		Number of ASCII characters in theAsciiString
//
// Remarks
//		Invalid ASCII characters are truncated
//		Memory for theAsciiString must be allocated with its size 
//		being 1/4 the size of theUCS4String
///////////////////////////////////////////////////////////////////////////////	
int UCS4ToAsciiString(const UCS4String theUCS4String, char* theAsciiString)
{
	int length = 0;
	const UCS4String aReadPos = theUCS4String;
	char* aWritePos = theAsciiString;

	GS_ASSERT(theAsciiString != NULL);

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUCS4String == NULL)
	{
		*theAsciiString = '\0';
		return 1;
	}

	// Convert each character until a '\0' is reached
	while(*aReadPos != '\0')
	{
		(*aWritePos++) = (char)(0x0000007F & (*aReadPos++));
		length++;
	}

	// append the NULL
	*aWritePos = '\0';
	length++;

	return length;
}
				

///////////////////////////////////////////////////////////////////////////////
// AsciiToUCS2String
// Summary
//          Converts a NULL terminated ASCII string into a Unicode (2 byte char) string 
// Parameters
//          theAsciiString		: [in] A pointer to the ASCII string
//          theUCS2String		: [out] A pointer to the UCS2String string
// Return
//		Number of Unicode characters in theUCS2String
//
// Remarks
//		theUCS2String is NULL terminated
//		Memory for theUCS2String must be allocated with its size 
//		being 2 times the size of theAsciiString
///////////////////////////////////////////////////////////////////////////////
int AsciiToUCS2String(const char* theAsciiString, UCS2String theUCS2String)
{
	int			length		= 0;
	const char* aReadPos	= theAsciiString;
	UCS2String  aWritePos	= theUCS2String;

	GS_ASSERT(theUCS2String != NULL);

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theAsciiString == NULL)
	{
		*theUCS2String = 0;
		return 1;
	}

	// Convert each character until a '\0' is reached
	while(*aReadPos != '\0')
	{
//		(*aWritePos++) = (UCS2Char)(0x00FF & (*aReadPos++)); // copy and strip extra byte
		(*aWritePos++) = (UCS2Char) *aReadPos; // copy and strip extra byte
		aReadPos++;
		length++;
	}

	// append a NULL terminator to the UCS2String
	*aWritePos = '\0';
	length++;

	return length;
}

///////////////////////////////////////////////////////////////////////////////
// AsciiToUCS4String
// Summary
//          Converts a NULL terminated ASCII string into a Unicode (4 byte char) string 
// Parameters
//          theAsciiString		: [in] A pointer to the ASCII string
//          theUCS4String		: [out] A pointer to the UCS4String string
// Return
//		Number of Unicode characters in theUCS4String
//
// Remarks
//		theUCS4String is NULL terminated
//		Memory for theUCS4String must be allocated with its size 
//		being 4 times the size of theAsciiString
///////////////////////////////////////////////////////////////////////////////		
int AsciiToUCS4String(const char* theAsciiString, UCS4String theUCS4String)
{
	int			length		= 0;
	const char* aReadPos	= theAsciiString;
	UCS4String  aWritePos	= theUCS4String;

	GS_ASSERT(theUCS4String != NULL);

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theAsciiString == NULL)
	{
		*theUCS4String = 0;
		return 1;
	}

	// Convert each character until a '\0' is reached
	while(*aReadPos != '\0')
	{
		(*aWritePos++) = (UCS4Char) *aReadPos; // copy and strip extra bytes
		aReadPos++;
		length++;
	}

	// append a NULL terminator to the UCS4String
	*aWritePos = '\0';
	length++;

	return length;
}
				

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2StringLen
// Summary
//          Converts a NULL terminated UTF8 string into a Unicode (2 byte char) string 
//			with up to "theMaxUCS2Length" Unicode characters
// Parameters
//          theUTF8String		: [in] A pointer to the UTF8 string
//          theUCS2String		: [out] A pointer to the UCS2String string
//			theMaxUCS2Length	: [in] maximum number of UCS2 characters to convert to
// Return
//		Number of Unicode characters in theUCS2String
//
// Remarks
//		The character length of theUCS2String will not exceed theMaxUCS2Length
//		theUCS2String is NULL terminated
//		Any invalid UTF8 string will result in a NULL unicode string
///////////////////////////////////////////////////////////////////////////////				
int UTF8ToUCS2StringLen(const UTF8String theUTF8String, UCS2String theUCS2String, int theMaxUCS2Length)
{
	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUTF8String == NULL)
	{
		*theUCS2String = 0;
		return 1;
	}
				
	return UTF8ToUCS2StringLen2(theUTF8String, (int)strlen((char *)theUTF8String), theUCS2String, theMaxUCS2Length);
}

///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4StringLen
// Summary
//          Converts a NULL terminated UTF8 string into a Unicode (4 byte char) string 
//			with up to "theMaxUCS4Length" Unicode characters
// Parameters
//          theUTF8String		: [in] A pointer to the UTF8 string
//          theUCS4String		: [out] A pointer to the UCS4String string
//			theMaxUCS4Length	: [in] maximum number of UCS4 characters to convert to
// Return
//		Number of Unicode characters in theUCS4String
//
// Remarks
//		The character length of theUCS4String will not exceed theMaxUCS4Length
//		theUCS4String is NULL terminated
//		Any invalid UTF8 string will result in a NULL unicode string
///////////////////////////////////////////////////////////////////////////////				
int UTF8ToUCS4StringLen(const UTF8String theUTF8String, UCS4String theUCS4String, int theMaxUCS4Length)
{
	// Allow for NULL here since SDKs allow for NULL string
	if (theUTF8String == NULL)
	{
		*theUCS4String = 0;
		return 1;
	}
				
	return UTF8ToUCS4StringLen2(theUTF8String, (int)strlen((char *)theUTF8String), theUCS4String, theMaxUCS4Length);
}
	
///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS2StringLen2
// Summary
//          Converts a UTF8 string of length theUTF8Length into a Unicode (2 byte char) string 
//			with up to "theMaxUCS2Length" Unicode characters
// Parameters
//          theUTF8String		: [in] A pointer to the UTF8 string
//          theUTF8Length		: [in]  length of theUTF8String in bytes
//          theUCS2String		: [out] A pointer to the UCS2String string
//			theMaxUCS2Length	: [in] maximum number of UCS2 characters to convert to
// Return
//		Number of Unicode characters in theUCS2String
//
// Remarks
//		theUTF8String doesn't need to be null terminated
//		The character length of theUCS2String will not exceed theMaxUCS2Length
//		theUCS2String is NULL terminated
//		Any invalid UTF8 string will result in a NULL unicode string
///////////////////////////////////////////////////////////////////////////////				
int UTF8ToUCS2StringLen2(const UTF8String theUTF8String, int theUTF8Length, UCS2String theUCS2String, int theMaxUCS2Length)
{
	int aNumCharsWritten	= 0;
	int aNumBytesRead		= 0;
	int aTotalBytesRead     = 0;
	const unsigned char* anInStream	= (const unsigned char*)theUTF8String;
	UCS2Char*            anOutStream= theUCS2String;

	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUTF8String == NULL)
	{
		*anOutStream = 0;
		return 1;
	}

	// Loop until we find the NULL terminator
	while (*anInStream != '\0' && 
		(theMaxUCS2Length+1) > aNumCharsWritten &&
		theUTF8Length > aTotalBytesRead)
	{
		// Convert one character
		aNumBytesRead = _ReadUCS2CharFromUTF8String((UTF8String)anInStream, theUTF8Length-aTotalBytesRead, anOutStream);
		if (aNumBytesRead == 0)
		{
			// Error, read past end of buffer
			theUCS2String[0] = 0;
			return 0;
		}
		aTotalBytesRead += aNumBytesRead;

		// Move InStream position to new data
		anInStream += aNumBytesRead;

		// Keep track of characters written
		aNumCharsWritten++;

		// Move OutStream to next write position
		anOutStream++;
	}

	// NULL terminate the UCS2String
	*anOutStream = 0;
	aNumCharsWritten++;

	return aNumCharsWritten;
}
	
///////////////////////////////////////////////////////////////////////////////
// UTF8ToUCS4StringLen2
// Summary
//          Converts a UTF8 string of length theUTF8Length into a Unicode (4 byte char) string 
//			with up to "theMaxUCS4Length" Unicode characters
// Parameters
//          theUTF8String		: [in] A pointer to the UTF8 string
//          theUTF8Length		: [in]  length of theUTF8String in bytes
//          theUCS4String		: [out] A pointer to the UCS4String string
//			theMaxUCS4Length	: [in] maximum number of UCS4 characters to convert to
// Return
//		Number of Unicode characters in theUCS4String
//
// Remarks
//		theUTF8String doesn't need to be null terminated
//		The character length of theUCS4String will not exceed theMaxUCS4Length
//		theUCS4String is NULL terminated
//		Any invalid UTF8 string will result in a NULL unicode string
///////////////////////////////////////////////////////////////////////////////
int UTF8ToUCS4StringLen2(const UTF8String theUTF8String, int theUTF8Length, UCS4String theUCS4String, int theMaxUCS4Length)
{
	const UTF8String anInStream	 = theUTF8String;

	int			aNumCharsWritten	= 0;
	int			aNumBytesRead		= 0;
	int			aTotalBytesRead     = 0;
	UCS4String	anOutStream			= theUCS4String;
	
	// Allow for NULL here since SDKs allow for NULL string arrays
	if (theUTF8String == NULL)
	{
		*anOutStream = 0;
		return 1;
	}
	
	// Loop until we find the NULL terminator
	while (*anInStream != '\0' && (theMaxUCS4Length+1) > aNumCharsWritten && theUTF8Length > aTotalBytesRead)
	{
		// Convert one Unicode code point character
		aNumBytesRead = _ReadUCS4CharFromUTF8String(anInStream, theUTF8Length - aTotalBytesRead, anOutStream);
		if (aNumBytesRead == 0)
		{
			// Error, return empty Unicode string
			theUCS4String[0] = 0;
			return 0;
		}
				
		aTotalBytesRead += aNumBytesRead;
		
		// Move InStream position to new data
		anInStream += aNumBytesRead;
		
		// Keep track of characters written
		aNumCharsWritten++;
		
		// Move OutStream to next write position
		anOutStream++;
	}
	
	// NULL terminate the UCS4String
	*anOutStream = 0;
	aNumCharsWritten++;
	
	return aNumCharsWritten;
}
	
///////////////////////////////////////////////////////////////////////////////
// gsiSafeStrcpyA
// Summary
//          Copies up to "size" characters of a char string "src" into "dst" 
// Parameters
//          dst		: [out] A pointer to the copied string
//          src		: [in]  A pointer to the string to copy
//          size    : [in] The maximum number of char characters to copy
// Return
//		Number of char characters copied
// Remarks
//		
///////////////////////////////////////////////////////////////////////////////
size_t
gsiSafeStrcpyA(char *dst, const char *src, size_t size)
{
	size_t len;

	GS_ASSERT(dst != NULL);
	GS_ASSERT(src != NULL);
	GS_ASSERT(size > 0);

	len = strlen(src);
	if (len > size - 1)
		len = size - 1;
				
	memcpy(dst, src, len);
	dst[len] = '\0';
				
	return len;
}

///////////////////////////////////////////////////////////////////////////////
// gsiSafeStrcpyW
// Summary
//          Copies up to "size" characters of a wchar_t string "src" into "dst" 
// Parameters
//          dst		: [out] A pointer to the copied string
//          src		: [in]  A pointer to the string to copy
//          size    : [in] The maximum number of wchar_t characters to copy
// Return
//		Number of wchar_t characters copied
///////////////////////////////////////////////////////////////////////////////
size_t gsiSafeStrcpyW(wchar_t *dst, const wchar_t *src, size_t size)
{
	size_t len;

	GS_ASSERT(dst != NULL);
	GS_ASSERT(src != NULL);
	GS_ASSERT(size > 0);

	len = wcslen(src);
				
	if (len > size - 1)
		len = size - 1;
				
	memcpy(dst, src, len * sizeof(wchar_t));
	dst[len] = '\0';
				
	return len;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} //extern "C"
#endif	
