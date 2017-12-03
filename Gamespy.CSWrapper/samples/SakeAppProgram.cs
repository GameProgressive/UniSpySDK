///////////////////////////////////////////////////////////////////////////////
// File:	SakeAppProgram.cs
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
using Gamespy;
using Gamespy.Auth;
using Gamespy.Common;
using Gamespy.Http;
using Gamespy.Sake;
using Gamespy.Atlas;

namespace gamespySample
{
    class SakeAppProgram
    {
        private static string   _AccessKey = "39cf9714e24f421c8ca07b9bcb36c0f5";
        private static string   _SecretKey = "HA6zkS";
        private static Int32    _GameId = 0;
        private static Int32    _PartnerId = 0;
        private static Int32    _NameSpace = 1;
        private static bool     respReceived = false;
        private static string   _NickName = "saketest";
        private static string   _Email = "saketest@saketest.com";
        private static string   _Password = "saketest";
        private static string   _CdKey = "";
        private static gamespyAuth.GSLoginCertificate gameCertificate;
        private static gamespyAuth.GSLoginCertificatePrivate gamePrivateData;

        public static void loginCallback(GHTTPResult httpResult, IntPtr theResponse, IntPtr userData)
        {

            respReceived = true;

            if (httpResult != GHTTPResult.GHTTPSuccess)
            {
                Console.WriteLine("Failed on player login, HTTP error: {0}", httpResult);
                return;
            }

            gamespyAuth.WSLoginResponse resp = (gamespyAuth.WSLoginResponse)Marshal.PtrToStructure(theResponse, typeof(gamespyAuth.WSLoginResponse));

            if (resp.mLoginResult != WSLoginValue.WSLogin_Success)
            {
                Console.WriteLine("Failed on player login, Login result: {0}", resp.mLoginResult);
            }
            else
            {
                Console.WriteLine("Player {0} with profile Id {1} logged in.", resp.mCertificate.mUniqueNick, resp.mCertificate.mProfileId);
                gameCertificate = resp.mCertificate;
                gamePrivateData = resp.mPrivateData;
            }
        }

        /// Sake Application code

        // Sake related declarations 
        private static string _tableId = "test";
        private static IntPtr sake = new IntPtr();
        public int recordCount;
        public int recordId;
        public static gamespySake.SAKEGetMyRecordsInput getMyRecordsInput = new gamespySake.SAKEGetMyRecordsInput();
        public static gamespySake.SAKEGetMyRecordsOutput sakeResponse = new gamespySake.SAKEGetMyRecordsOutput();

        // Sake callbacks
        public static void storageGetRecordCountCallback
        (
            IntPtr sake,
            IntPtr request,             // pointer to SAKERequest
            SAKERequestResult result,
            IntPtr inputData,
            IntPtr outputData,
            IntPtr userData
        )
        {
            respReceived = true;

            // we are keeping the record count in userData for this callback. 
            // other applications might keep objects and etc.

            if (result == SAKERequestResult.SAKERequestResult_SUCCESS)
            {

                gamespySake.SAKEGetRecordCountOutput resp =
                    (gamespySake.SAKEGetRecordCountOutput)Marshal.PtrToStructure(outputData, typeof(gamespySake.SAKEGetRecordCountOutput));

                GCHandle handle = GCHandle.FromIntPtr(userData);
                SakeAppProgram myObject = (SakeAppProgram)handle.Target;
                myObject.recordCount = resp.mCount;
                handle.Free();

                Console.WriteLine("SUCCESS: Retrived Record Count {0}", resp.mCount);
            }
            else
            {
                Console.WriteLine("FAILURE: {0}", result);
            }
        }

        public static DateTime ConvertFromSeconds(int timestamp)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return origin.AddSeconds(timestamp);
        }

        private void WaitForResponse()
        {
            respReceived = false;
            while (respReceived == false)
            {
                gamespySake.sakeThink();
                Thread.Sleep(new TimeSpan(100000));
            }
        }
        
        public static void storageGetMyRecordsCallback
        (
            IntPtr sake,
            IntPtr request,             // pointer to SAKERequest
            SAKERequestResult result,
            IntPtr inputData,
            IntPtr outputData,
            IntPtr userData
        )
        {
            respReceived = true;

            // we are keeping the record count in userData for this callback. 
            // other applications might keep objects and etc.

            if (result == SAKERequestResult.SAKERequestResult_SUCCESS)
            {
                gamespySake.SAKEGetMyRecordsOutput resp =
                    (gamespySake.SAKEGetMyRecordsOutput)Marshal.PtrToStructure(outputData, typeof(gamespySake.SAKEGetMyRecordsOutput));

                //gamespySake.SAKEGetMyRecordsInput  req  = 
                //  (gamespySake.SAKEGetMyRecordsInput)Marshal.PtrToStructure(inputData, typeof(gamespySake.SAKEGetMyRecordsInput));

                //User Data Reference Pointer
                GCHandle handle = GCHandle.FromIntPtr(userData);
                SakeAppProgram myObject = (SakeAppProgram)handle.Target;

                //Process the received Data
                IntPtr[] pRec = new IntPtr[resp.mNumRecords];
                Marshal.Copy(resp.mRecords, pRec, 0, resp.mNumRecords);

                for (int i = 0; i < resp.mNumRecords; i++)
                {
                    // Process a Record
                    Console.WriteLine("---------Record[{0}]--------- ", i);
                    for (int j = 0; j < getMyRecordsInput.mNumFields; j++)
                    {
                        // Each record is block of memory which contains the fields (Name,Value,Type) 
                        gamespySake.SAKEField aSakefield = new gamespySake.SAKEField();
                        IntPtr pSakeField = new IntPtr(pRec[i].ToInt32() + j * Marshal.SizeOf(aSakefield));
                        aSakefield = (gamespySake.SAKEField)Marshal.PtrToStructure(pSakeField, typeof(gamespySake.SAKEField)); ;

                        switch (aSakefield.mType)
                        {
                            case SAKEFieldType.SAKEFieldType_ASCII_STRING:
                                String myAsciiString = Marshal.PtrToStringAnsi(aSakefield.mValue.mString);
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, myAsciiString);
                                break;

                            case SAKEFieldType.SAKEFieldType_UNICODE_STRING:
                                String myUnicodeString = Marshal.PtrToStringUni(aSakefield.mValue.mString);
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, myUnicodeString);
                                break;
                            case SAKEFieldType.SAKEFieldType_SHORT:
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mShort);
                                break;
                            case SAKEFieldType.SAKEFieldType_INT:
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mInt);
                                break;
                            case SAKEFieldType.SAKEFieldType_FLOAT:
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mFloat);
                                break;
                            case SAKEFieldType.SAKEFieldType_BYTE:
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mByte);
                                break;
                            case SAKEFieldType.SAKEFieldType_BINARY_DATA:
                                Console.Write("{0}[{1}]=", aSakefield.mName, aSakefield.mType);
                                if (aSakefield.mValue.mBinaryData.mValue != IntPtr.Zero)
                                {
                                    IntPtr pBinaryData = aSakefield.mValue.mBinaryData.mValue;
                                    byte[] binaryData = new byte[aSakefield.mValue.mBinaryData.mLength];
                                    Marshal.Copy(pBinaryData, binaryData, 0, binaryData.Length);
                                    for (int k = 0; k < binaryData.Length; k++) Console.Write(" {0} ", binaryData[k]);
                                    Console.WriteLine("\n");
                                }
                                else Console.WriteLine("Null\n");
                                break;
                            case SAKEFieldType.SAKEFieldType_BOOLEAN:
                                Console.WriteLine("{0}[{1}]={2} \n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mBoolean);
                                break;
                            case SAKEFieldType.SAKEFieldType_DATE_AND_TIME:
                                Console.WriteLine("{0}[{1}]={2}\n", aSakefield.mName, aSakefield.mType, ConvertFromSeconds(aSakefield.mValue.mDateAndTime).ToString());
                                break;
                            case SAKEFieldType.SAKEFieldType_INT64:
                                Console.WriteLine("{0}[{1}]={2} \n", aSakefield.mName, aSakefield.mType, aSakefield.mValue.mInt64);
                                break;
                            default:
                                break;

                        }
                    }
                }
                Console.WriteLine("SUCCESS: Retrived {0} records\n", resp.mNumRecords);
            }
            else
            {
                Console.WriteLine("FAILURE: {0}\n", result);
            }

        }
        public static gamespySake.SAKERequestCallback getRecordCountCallback = new gamespySake.SAKERequestCallback(storageGetRecordCountCallback);

        // Sake APP

        public int StorageGetRecordCount(string tableName, StringBuilder filter, bool cache)
        {
            gamespySake.SAKEGetRecordCountInput recordCountInput = new gamespySake.SAKEGetRecordCountInput();
            recordCountInput.mTableId = tableName;
            recordCountInput.mCacheFlag = true;

            //gamespySake.SAKERequestCallback getRecordCountCallback = new gamespySake.SAKERequestCallback(storageGetRecordCountCallback);

            // Convert this pointer to IntPtr. This is needed for marshalling between C# and gamespy.dll
            GCHandle hUserData = GCHandle.Alloc(this, GCHandleType.Normal);
            IntPtr userData = GCHandle.ToIntPtr(hUserData);

            IntPtr pInput = Marshal.AllocHGlobal(Marshal.SizeOf(recordCountInput));
            Marshal.StructureToPtr(recordCountInput, pInput, false);
            gamespySake.sakeGetRecordCount(sake, pInput, getRecordCountCallback, userData);

            WaitForResponse();
            
            hUserData.Free();
            Console.WriteLine("{0} table has {1} records\n", tableName, this.recordCount);
            return this.recordCount;
        }
        public static gamespySake.SAKERequestCallback getMyRecordsCallback = new gamespySake.SAKERequestCallback(storageGetMyRecordsCallback);

        public void StorageGetMyRecords(gamespySake.SAKEGetMyRecordsInput input)
        {
            SAKEStartRequestResult startRequestResult;
            // Convert this pointer to IntPtr. This is needed for marshalling between C# and gamespy.dll
            GCHandle hUserData = GCHandle.Alloc(this, GCHandleType.Normal);
            IntPtr userData = GCHandle.ToIntPtr(hUserData);

            IntPtr pInput = Marshal.AllocHGlobal(Marshal.SizeOf(input));
            Marshal.StructureToPtr(input, pInput, false);

            IntPtr request = gamespySake.sakeGetMyRecords(sake, pInput, getMyRecordsCallback, userData);
            if (request == IntPtr.Zero)
            {
                startRequestResult = gamespySake.sakeGetStartRequestResult(sake);
                Console.WriteLine("FAILURE: GetMyRecords {0} ", startRequestResult);
            }
            else
            {
                WaitForResponse();
                
                // retrieve the response
                Console.WriteLine("GetMyRecords completed");
            }
            hUserData.Free();
        }

        // Sake callbacks
        public static void storageCreateRecordCallback
        (
            IntPtr sake,
            IntPtr request,             // pointer to SAKERequest
            SAKERequestResult result,
            IntPtr inputData,
            IntPtr outputData,
            IntPtr userData
        )
        {
            respReceived = true;

            // we are keeping the record count in userData for this callback. 
            // other applications might keep objects and etc.

            if (result == SAKERequestResult.SAKERequestResult_SUCCESS)
            {

                gamespySake.SAKECreateRecordOutput resp =
                    (gamespySake.SAKECreateRecordOutput)Marshal.PtrToStructure(outputData, typeof(gamespySake.SAKECreateRecordOutput));

                GCHandle handle = GCHandle.FromIntPtr(userData);
                SakeAppProgram myObject = (SakeAppProgram)handle.Target;
                myObject.recordId = resp.mRecordId;
                handle.Free();

                Console.WriteLine("SUCCESS:Record Id = {0}", resp.mRecordId);
            }
            else
            {
                Console.WriteLine("FAILURE: {0}", result);
            }
        }

        public static gamespySake.SAKERequestCallback createRecordCallback = new gamespySake.SAKERequestCallback(storageCreateRecordCallback);

        public void StorageCreateRecord(gamespySake.SAKECreateRecordInput input)
        {
            SAKEStartRequestResult startRequestResult;
            // Convert this pointer to IntPtr. This is needed for marshalling between C# and gamespy.dll
            GCHandle hUserData = GCHandle.Alloc(this, GCHandleType.Normal);
            IntPtr userData = GCHandle.ToIntPtr(hUserData);

            IntPtr pInput = Marshal.AllocHGlobal(Marshal.SizeOf(input));
            Marshal.StructureToPtr(input, pInput, false);

            IntPtr request = gamespySake.sakeCreateRecord(sake, pInput, createRecordCallback, userData);
            if (request == IntPtr.Zero)
            {
                startRequestResult = gamespySake.sakeGetStartRequestResult(sake);
                Console.WriteLine("FAILURE: CreateRecord {0} ", startRequestResult);
            }
            else
            {
                WaitForResponse();
                
                // retrieve the response
                Console.WriteLine("CreateRecord completed");
            }
            hUserData.Free();
        }


        // Application methods
        public static bool Initialize()
        {
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            byte[] accessKey = encoding.GetBytes("39cf9714e24f421c8ca07b9bcb36c0f5"/*_AccessKey*/);
            byte[] secretKey = encoding.GetBytes("HA6zkS"/*_SecretKey*/);

            // Now initialize the common core
            gamespyCommon.gsCoreInitialize();
            gamespyAuth.wsSetGameCredentials(accessKey, _GameId, secretKey);
            gamespyAuth.WSLoginCallback myLoginCallback = new gamespyAuth.WSLoginCallback(loginCallback);
            gamespyAuth.wsLoginProfile(0, 0, 1, _NickName, _Email, _Password, _CdKey, myLoginCallback, IntPtr.Zero);

            while (respReceived == false)
            {
                gamespyCommon.gsCoreThink(0);
                Thread.Sleep(new TimeSpan(100000));
            }

            // Now Initialize SAKE
            SAKEStartupResult sakeResult = gamespySake.sakeStartup(ref sake);
            Console.WriteLine("Game authenticating with Sake");
            gamespySake.sakeSetGame(sake, "gmtest", 0, _SecretKey);
            gamespySake.sakeSetProfile(sake, (int)gameCertificate.mProfileId, ref gameCertificate, ref gamePrivateData);
            return true;
        }

        public static bool Terminate()
        {
            if (sake != IntPtr.Zero) gamespySake.sakeShutdown(sake);
            gamespyHttp.ghttpCleanup();
            gamespyCommon.gsCoreShutdown();
            // Wait for gsCore to shutdown. This is needed to run the next sample app.
            while (gamespyCommon.gsCoreIsShutdown() != GSCoreState.GSCoreState_SHUTDOWN_COMPLETE)
            {
                gamespyCommon.gsCoreThink(0);
                if (gamespyCommon.gsCoreIsShutdown() != GSCoreState.GSCoreState_SHUTDOWN_COMPLETE)
                    Thread.Sleep(new TimeSpan(100000));
            }

            return true;
        }

        public GSIACResult CheckServices(string gamename)
        {
            GSIACResult aResult;

            gamespyCommon.GSIStartAvailableCheck(gamename);

            // Continue processing while the check is in progress
            do
            {
                aResult = gamespyCommon.GSIAvailableCheckThink();
                Thread.Sleep(new TimeSpan(100000));
            } while (aResult == GSIACResult.GSIACWaiting);

            // Check the result
            switch (aResult)
            {
                case GSIACResult.GSIACAvailable:
                    Console.WriteLine("{0}: Online Services are available.", gamename);
                    break;
                case GSIACResult.GSIACUnavailable:
                    Console.WriteLine("{0}: Online services are unavailable.", gamename);
                    break;
                case GSIACResult.GSIACTemporarilyUnavailable:
                    Console.WriteLine("{0}: Online services are temporarily unavailable.", gamename);
                    break;
                default:
                    break;
            };

            return aResult;
        }

        public void Run()
        {
            string gamename = "gmtest";
#if GSI_COMMON_DEBUG
            gamespyCommonDebug.gsOpenDebugFile("Debug_Sake.log");
            gamespyCommonDebug.gsSetDebugLevel(GSIDebugCategory.GSIDebugCat_All, GSIDebugType.GSIDebugType_All, GSIDebugLevel.Hardcore);
#endif
            Console.WriteLine("\n------------ SAKE C# sample ------------------------\n");

            if (CheckServices(gamename) != GSIACResult.GSIACAvailable)
            {
                Console.WriteLine("{0}: Online services are unavailable now.", gamename);
                return;
            }

            if (Initialize())
            {
                Console.WriteLine("{0}: Initialized.\n", gamename);
            }
            else
            {

                Terminate();
                Console.WriteLine("\n------------ SAKE C# Sample Terminated with Failure ------------------------\n");
                Console.WriteLine("Press AnyKey Continue....");
                Console.ReadLine();
                return;
            }

            // sakeGetRecordCount example
            Console.WriteLine("---------- {0} table : GetRecordCount \n", _tableId);

            StorageGetRecordCount(_tableId, null, true);

            // sakeGetMyRecords example
            Console.WriteLine("--------- {0} table : GetMyRecords\n", _tableId);

            // Initialize sample data

            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();

            string[] fields = new string[12]{    "recordid", 
                                    "ownerid", 
                                    "MyByte", 
                                    "MyShort",
	                                "MyInt", 
                                    "MyFloat", 
                                    "MyAsciiString",
	                                "MyUnicodeString", 
                                    "MyBoolean", 
                                    "MyDateAndTime",
	                                "MyBinaryData", 
                                    "MyFileID"};

            //gamespySake.SAKEGetMyRecordsInput getMyRecordsInput = new gamespySake.SAKEGetMyRecordsInput();

            getMyRecordsInput.mTableId = _tableId;
            getMyRecordsInput.mNumFields = 12;

            getMyRecordsInput.mFieldNames = Marshal.AllocHGlobal(getMyRecordsInput.mNumFields * Marshal.SizeOf(typeof(IntPtr)));

            IntPtr[] pFieldNames = new IntPtr[getMyRecordsInput.mNumFields];
            for (int i = 0; i < getMyRecordsInput.mNumFields; i++)
            {
                pFieldNames[i] = Marshal.StringToHGlobalAnsi(fields[i]);
            }
            Marshal.Copy(pFieldNames, 0, getMyRecordsInput.mFieldNames, getMyRecordsInput.mNumFields);

            GCHandle hInput = GCHandle.Alloc(getMyRecordsInput);

            StorageGetMyRecords(getMyRecordsInput);

            hInput.Free();

            //
            // sakeCreateRecord example
            Console.WriteLine("--------- {0} table : CreateRecord\n", _tableId);

            gamespySake.SAKECreateRecordInput createRecordInput = new gamespySake.SAKECreateRecordInput();
            gamespySake.SAKEField[] field = new gamespySake.SAKEField[9]; // ownerid, recordid and dateandtime is auto created, do not initialize them
            
            // Initialize the field[] array
            int j=0;
            field[j].mName = "MyByte";
            field[j].mType = SAKEFieldType.SAKEFieldType_BYTE;
            field[j].mValue.mByte = 1;
            
            j++;
            field[j].mName = "MyShort";
            field[j].mType = SAKEFieldType.SAKEFieldType_SHORT;
            field[j].mValue.mShort = 2;
            
            j++;
            field[j].mName = "MyInt";
            field[j].mType = SAKEFieldType.SAKEFieldType_INT;
            field[j].mValue.mInt = 3;

            j++;
            field[j].mName = "MyFloat";
            field[j].mType = SAKEFieldType.SAKEFieldType_FLOAT;
            field[j].mValue.mFloat = 4.0F;

            j++;
            field[j].mName = "MyAsciiString";
            field[j].mType = SAKEFieldType.SAKEFieldType_ASCII_STRING;
            field[j].mValue.mString = new IntPtr();
            field[j].mValue.mString = Marshal.StringToHGlobalAnsi("ASCII string");

            j++;
            field[j].mName = "MyUnicodeString";
            field[j].mType = SAKEFieldType.SAKEFieldType_UNICODE_STRING;
            field[j].mValue.mString = new IntPtr();
            field[j].mValue.mString = Marshal.StringToHGlobalUni("UNICODE string");
            
            j++;
            field[j].mName = "MyBoolean";
            field[j].mType = SAKEFieldType.SAKEFieldType_BOOLEAN;
            field[j].mValue.mBoolean = true;
            
            j++;
            field[j].mName = "MyBinaryData";
            field[j].mType = SAKEFieldType.SAKEFieldType_BINARY_DATA;
            field[j].mValue.mBinaryData = new gamespySake.SAKEBinaryData();
            field[j].mValue.mBinaryData.mLength = 10;
            field[j].mValue.mBinaryData.mValue = Marshal.AllocHGlobal(field[j].mValue.mBinaryData.mLength);
            byte[] binaryData = new byte[10]  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            Marshal.Copy(binaryData,0,field[j].mValue.mBinaryData.mValue,binaryData.Length);
            
            j++;
            field[j].mName = "MyFileID";
            field[j].mType = SAKEFieldType.SAKEFieldType_INT;
            field[j].mValue.mInt = 55555;
            
            //Correct way of Marshaling SAKEField array into IntPtr
            
            createRecordInput.mTableId   = _tableId;
            createRecordInput.mNumFields = field.Length; // number of elements in the field[] array.
            
            // Sake expects all the fields in a continous block of memory.
            createRecordInput.mFields    = Marshal.AllocHGlobal(createRecordInput.mNumFields * Marshal.SizeOf(typeof(gamespySake.SAKEField)));

            for (int i = 0; i < createRecordInput.mNumFields; i++)
            {
                // Obtain the pointer to the specific element in the Unmanaged Memory
                IntPtr pField = new IntPtr(createRecordInput.mFields.ToInt32()+i*Marshal.SizeOf(typeof(gamespySake.SAKEField)));
                
                //Marshal filed[i] to Unmanaged memory
                Marshal.StructureToPtr(field[i], pField, false); // must delete field[] after the callback returns
            }
            
            // Want to make sure createRecordInput does not get garbage collected.
            GCHandle hCreateInput = GCHandle.Alloc(createRecordInput);

            StorageCreateRecord(createRecordInput);

            hCreateInput.Free();
            //
            Terminate();

            Console.WriteLine("\n------------ SAKE C# Sample Completed------------------------\n");
            Console.WriteLine("Press Enter Continue....");
            Console.ReadLine();

        }
    }
}
