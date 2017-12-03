///////////////////////////////////////////////////////////////////////////////
// File:	gamespySake.cs
// SDK: 	GameSpy Sake Persistent Storage SDK C# Wrapper
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;
using Gamespy.Auth;

namespace Gamespy
{
    namespace Sake
    {
        class gamespySake
        {
            // Structures

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKERequest
            {
                public bool     mIsGameAuthenticated;
                [MarshalAs(UnmanagedType.LPStr)]
                public String   mGameName;
                public int      mGameId;
                [MarshalAs(UnmanagedType.LPStr)]
                public String   mSecretKey;
                public bool     mIsProfileAuthenticated;
                public int      mProfileId;
	            public gamespyAuth.GSLoginCertificate           mCertificate;
                public gamespyAuth.GSLoginCertificatePrivate    mPrivateData;
                public SAKEStartRequestResult mStartRequestResult;
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEBinaryData
            {
                public IntPtr   mValue;     // pointer to the data, should be byte array.
                public int      mLength;    // the number of bytes of data.
            };

            [StructLayout(LayoutKind.Explicit)]
            public struct SAKEValue
            {
                [FieldOffset(0)] public byte   mByte;
                [FieldOffset(0)] public Int16  mShort;
                [FieldOffset(0)] public Int32  mInt;
                [FieldOffset(0)] public float  mFloat;
                [FieldOffset(0)] public IntPtr mString; // Use PtrToStringAnsi if ASCII type, PtrToStringUni if Unicode Type string when Marshaling
                [FieldOffset(0)] public bool   mBoolean;
                [FieldOffset(0)] public Int32  mDateAndTime;
                [FieldOffset(0)] public SAKEBinaryData mBinaryData;
                [FieldOffset(0)] public Int64  mInt64;
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEField
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string           mName;	// the name used to identify the field.
                public SAKEFieldType    mType;	// The type of data stored in the field.
                public SAKEValue        mValue;	// The value that will be stored in the field.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEContentBuffer
            {
                public IntPtr mBuffer;      // Pointer to the memory location. Byte array
                public int    mLength;      // Size of buffer.
            };

            [StructLayout(LayoutKind.Explicit)]
            public struct SAKEContentStorage
            {
                [FieldOffset(0)] public IntPtr mMemory; // SAKEContentBuffer *:use when content is in memory
                [FieldOffset(0)] public IntPtr mFile;   // char * :file name (or path name) when content is on Disk
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEContentInfo
            {
                public Int32                    mFileid; // file id for the content stored at the backend
                public SAKEContentStorageType   mType;   // the location (disk or memory) of the content locally 
                public SAKEContentStorage       mStorage;// either memory and length or a file name
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEReportRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string   mTableId;	    // Points to the tableid of the table in which the record to be reported exists.
                public int      mRecordId;	    // The recordid of the record to report.
                public int      mReasonCode;    // A code for the report reason, for if the developer has a few predefined options
                [MarshalAs(UnmanagedType.LPStr)]
                public string   mReason;      // Description of reasons why the record is reported. Maximum 256 characters.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKECreateRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string mTableId;	        // Points to the tableid of the table in which the record will be created.
                public IntPtr mFields;		    // Points to an array of fields (SAKEField[]) which has the initial values for the new record's fields.
                public int    mNumFields;	    // Stores the number of fields in the mFields array.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKECreateRecordOutput
            {
                public int mRecordId;	        // The recordid for the newly created record.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEUpdateRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string   mTableId;	// Points to the tableid of the table in which 
							                // the record to be updated exists.
                public int      mRecordId;	// Identifies the record to be updated.
                public IntPtr   mFields;	// Points to an array of fields (SAKEField[]) which has the new 
							                // values for the record's fields.
                public int      mNumFields;	// Stores the number of fields in the mFields array.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEDeleteRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string mTableId;	    // Points to the tableid of the table in which the 
						                    // record to be deleted exists.
                public int mRecordId;	    // Identifies the record to be deleted.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKESearchForRecordsInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string mTableId;				// char *: Points to the tableid of the table to be searched.
                public IntPtr mFieldNames;		    // char     **:Points to an array of ASCII strings, each of which contains the name of 
										            // a field for which to return values. This list controls the values 
										            // which will be returned as part of the response.  The array can 
										            // contain just one field name, the names of all the fields in the table, 
										            // or any subset of the field names.
                public int    mNumFields;			// Stores the number of strings in the mFieldNames array.
                [MarshalAs(UnmanagedType.LPTStr)]
                public String mFilter;			    // gsi_char  *: SQL-like filter string which is used to search for records based on the 
										            // values in their fields. For example, to find everyone who has a score of 
										            // more than 50 use "score > 50", or to find everyone who has a name that 
										            // starts with an A use "name like A%".  Note that a field can be used 
										            // in the filter string even if it is not listed in the mFieldNames array, 
										            // and that file metadata fields can be used in a filter string.
	                                                // The length of this string has been tested to 12,000 characters without issue.
                [MarshalAs(UnmanagedType.LPStr)]
                public string mSort;				// char      *: SQL-like sort string which is used to sort the records which are found by 
										            // the search. To sort the results on a particular field, just pass in the 
										            // name of that field, and the results will be sorted from lowest to highest 
										            // based on that field. To make the sort descending instead of ascending 
										            // add " desc" after the name of the field. Note that a field can be used 
										            // in the sort string even if it is not listed in the mFieldNames array, 
										            // and that file metadata fields can be used in a sort.
                public int    mOffset;				// If not set to 0, then the Sake service will return records starting from the 
										            // given offset into the result set.
                public int    mMaxRecords;		    // Used to specify the maximum number of records to return for 
										            // a particular search.
                [MarshalAs(UnmanagedType.LPTStr)]
                public String mTargetRecordFilter;	// gsi_char  *:Used to specify a single record to return - when done in conjunction 
										            // with mSurroundingRecordsCount, this will return the "target" record plus 
										            // the surrounding records above and below this target record. Can also 
										            // be used to specify a "set" of target records to return, but when used 
										            // in this context the surrounding records count does not apply.
                public int mSurroundingRecordsCount;// Used in conjunction with mTargetRecordFilter - specifies the number 
										            // of records to return above and below the target record. (e.g. if = 5, 
										            // you will receive a maximum of 11 possible records, the target record + 5 
										            // above and 5 below).
                public IntPtr mOwnerIds;			// Specifies an array of ownerIds (int[] type) (profileid of record owner) to return 
										            // from the search.
                public int    mNumOwnerIds;			// Specifies the number of ids contained in the mOwnerIds array.
                public bool   mCacheFlag;			// Enables caching if set to true. Defaults to no caching if none 
										            // is specified. Please turn on caching for big, leaderboard-style queries.
										            // Please turn off caching for individual player queries.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKESearchForRecordsOutput
            {
                public int      mNumRecords;	// The number of records found.
                public IntPtr   mRecords;		// SAKEField **: Points an array of records, each of which is 
								                // represented as an array of fields.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetMyRecordsInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public String   mTableId;	    // char      *: Points to the tableid of the table from which to return records.
                public IntPtr   mFieldNames;    // char     **: Points to an array of ASCII strings, each of which contains the name of a
                public int      mNumFields;	    // Stores the number of strings in the mFieldNames array. This list 
							                    // controls the values which will be returned as part of the response.  
							                    // The array can contain just one field name, the names of all the 
							                    // fields in the table, or any subset of the field names.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetMyRecordsOutput
            {
                public int      mNumRecords;	// The number of records found.
                public IntPtr   mRecords;	    // SAKEField **: Points an array of records, each of which is 
								                // represented as an array of fields.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetSpecificRecordsInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string   mTableId;		// char      *: Points to the tableid of the table from which to get the records.
                public IntPtr   mRecordIds;	    // An array of recordids (of type int[]), each one identifying a record which is 
								                // to be returned.
                public int      mNumRecordIds;	// The number of recordids in the mRecordIds array.
                public IntPtr   mFieldNames;    // char     **: Points to an array of ASCII strings, each of which contains the name 
								                // of a field for which to return values.
                public int      mNumFields;	    // Stores the number of strings in the mFieldNames array. This list 
								                // controls the values which will be returned as part of the response. 
								                // The array can contain just one field name, the names of all the 
								                // fields in the table, or any subset of the field names.
            };
            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetSpecificRecordsOutput
            {
                public int mNumRecords;	    // The number of records found.
                public IntPtr mRecords;		// SAKEField ** Points an array of records, each of which is 
								            // represented as an array of fields.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRandomRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string mTableId;	    // char * Points to the tableid of the table to be searched.
                public IntPtr mFieldNames;  // char **  Points to an array of ASCII strings, each of which contains the name of a field for 
							                // which to return values. This list controls the values which will be returned 
							                // as part of the response.  The array can contain just one field name, the names 
							                // of all the fields in the table, or any subset of the field names.
                public int    mNumFields;	// Stores the number of strings in the mFieldNames array.
                [MarshalAs(UnmanagedType.LPTStr)]
                public String mFilter;		// gsi_char* SQL-like filter string which is used to filter which records are to be looked at 
							                // when choosing a random record. Note that if the search criteria is too specific 
							                // and no records are found, then the output will return no random record. Note that 
							                // a field can be used in the filter string even if it is not listed in the mFieldNames 
							                // array, and that file metadata fields can be used in a filter string.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRandomRecordOutput
            {
                public IntPtr mRecord;	// An array of fields (SAKEField[]) representing the random record.
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKERateRecordInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string   mTableId;	// char*  Points to the tableid of the table in which the record to be rated exists.
                public int      mRecordId;	// The recordid of the record to rate.
                public byte     mRating;	// The rating the user wants to give the record.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKERateRecordOutput
            {
                public int   mNumRatings;	// The number of ratings associated with this record.
                public float mAverageRating;// The average rating of this record.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRecordLimitInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public String mTableId;	// Points to the tableid of the table for which to check the limit.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRecordLimitOutput
            {
                public int mLimitPerOwner;	// Contains the maximum number of records that a profile 
						                    // can own in the table; corresponds to the limit per owner 
						                    // option that can be set using the Sake Administration 
						                    // website.
                public int mNumOwned;		// Contains the number of records that the local profile 
						                    // currently owns in the table.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRecordCountInput
            {
                [MarshalAs(UnmanagedType.LPStr)]
                public string mTableId;	    // Points to the tableid of the table to be searched.
                [MarshalAs(UnmanagedType.LPTStr)]
                public String mFilter;		// gsi_char *: SQL-like filter string which is used to filter which records 
							                // are to be looked at when getting the record count.
                public bool   mCacheFlag;	// Enables caching if set to true. Defaults to no caching 
							                // if none is specified.
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEGetRecordCountOutput
            {
                public int mCount;	// Contains the value of the record count. If no records 
						            // exist or the search criteria was too specific so that 
						            // no records were found, this value will be 0.
            } ;
            
            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEUploadContentInput
            {
                [MarshalAs(UnmanagedType.LPTStr)]
                public String remoteFileName;           // gsi_char * file name to saved be the server
                public IntPtr content;                  // SAKEContentInfo * pointer to the content info structure
                public bool   transferBlocking;         // true if request is a blocking call
                public SAKEUploadContentProgressCallback progressCallback; 
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SAKEDownloadContentInput
            {

                public IntPtr   content;         // SAKEContentInfo  *: pointer to content info structure
                public bool     transferBlocking;// true if request is a blocking call
                public SAKEDownloadContentProgressCallback progressCallback; // developer's callback to tack download progress
            } ;

            // Callbacks
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SAKERequestCallback
            (
                IntPtr sake,
                IntPtr request,             // pointer to SAKERequest
                SAKERequestResult result,
                IntPtr inputData,
                IntPtr outputData,
                IntPtr userData
            );
            
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SAKEUploadContentProgressCallback
            (
                IntPtr  sake, 
                UInt32  bytesTransfered, 
                UInt32  totalSize, 
                IntPtr  userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SAKEUploadContentCompletedCallback
            (
                IntPtr  sake, 
	            SAKERequestResult result,
	            Int32   fileId, 
	            SAKEFileResult    fileResult, 
	            IntPtr  userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SAKEDownloadContentProgressCallback
            (
                IntPtr  sake, 
                UInt32  bytesTransfered, 
                UInt32  totalSize, 
                IntPtr  userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SAKEDownloadContentCompletedCallback
            (
                IntPtr  sake,
                SAKERequestResult result,
                SAKEFileResult    fileResult,
                IntPtr  buffer,             // char *
                Int32   bufferLength,
                IntPtr  userData
            );

            // API functions
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SAKEStartupResult sakeStartup(ref IntPtr sakePtr);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void sakeShutdown(IntPtr sakePtr);
            
            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern void sakeSetGame
            (
                IntPtr  sake, 
                [MarshalAs(UnmanagedType.LPTStr)] String gameName, 
                Int32   gameId, 
                [MarshalAs(UnmanagedType.LPTStr)] String secretKey);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void sakeSetProfile
            ( 
                IntPtr sake, 
                Int32 profileId, 
                ref gamespyAuth.GSLoginCertificate certificate, 
                ref gamespyAuth.GSLoginCertificatePrivate privateData);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SAKEStartRequestResult sakeGetStartRequestResult(IntPtr sake);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeReportRecord
            (IntPtr sake, IntPtr /*SAKEReportRecordInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeCreateRecord
            (IntPtr sake, IntPtr /*SAKECreateRecordInput*/	input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeUpdateRecord
            (IntPtr sake, IntPtr /*SAKEUpdateRecordInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeDeleteRecord
            (IntPtr sake, IntPtr /*SAKEDeleteRecordInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeSearchForRecords
            (IntPtr sake, IntPtr /*SAKESearchForRecordsInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeGetMyRecords
            (IntPtr sake, IntPtr /*SAKEGetMyRecordsInput*/ input, SAKERequestCallback callback, IntPtr userData);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeGetSpecificRecords
            (IntPtr sake, IntPtr /*SAKEGetSpecificRecordsInput*/ input,SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeGetRandomRecord
            (IntPtr sake, IntPtr /*SAKEGetRandomRecordInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeRateRecord
            (IntPtr sake, IntPtr /*SAKERateRecordInput*/ input, SAKERequestCallback callback, IntPtr  userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/  sakeGetRecordLimit
            (IntPtr sake, IntPtr /*SAKEGetRecordLimitInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SAKERequest*/ sakeGetRecordCount
            (IntPtr sake, IntPtr /*SAKEGetRecordCountInput*/ input, SAKERequestCallback callback, IntPtr userData);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr sakeGetFieldByName // returns a pointer to SAKEField struct
            ([MarshalAs(UnmanagedType.LPStr)] String name, ref SAKEField[] fields, int numFields);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SAKEStartRequestResult sakeCancelRequest(IntPtr sake, SAKERequest request);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool sakeGetFileDownloadURL
            (IntPtr sake, int fileId, ref StringBuilder  url);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool sakeGetFileUploadURL
            (IntPtr sake, ref StringBuilder url/*[SAKE_MAX_URL_LENGTH]*/);
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SAKEStartRequestResult sakeUploadContent
            (IntPtr sake, IntPtr /*SAKEUploadContentInput*/ input, SAKEUploadContentCompletedCallback completedCallback, IntPtr userData); 

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SAKEStartRequestResult sakeDownloadContent
            (IntPtr sake, IntPtr /*SAKEDownloadContentInput*/ input, SAKEDownloadContentCompletedCallback completedCallback, IntPtr userData);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool sakeGetFileResultFromHeaders
            ([MarshalAs(UnmanagedType.LPStr)] StringBuilder headers, ref SAKEFileResult result);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl, CharSet=CharSet.Ansi)]
            public static extern bool sakeGetFileIdFromHeaders
            ([MarshalAs(UnmanagedType.LPStr)] StringBuilder headers, ref int fileId);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void sakeThink();

        }
    }
}
