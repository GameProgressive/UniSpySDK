///////////////////////////////////////////////////////////////////////////////
// File:	gamespyAuth.cs
// SDK:		GameSpy Authentication Service SDK C# Wrapper
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


namespace Gamespy
{

    namespace Auth
    {   
        
        class gamespyAuth
        {
            /// <summary>
            ///  Auth service declarations
            /// </summary>
            /// 
            public const int GS_LARGEINT_DIGIT_SIZE_BYTES = sizeof(UInt32);
            public const int GS_LARGEINT_DIGIT_SIZE_BITS = GS_LARGEINT_DIGIT_SIZE_BYTES * 8;
            public const int GS_LARGEINT_MAX_DIGITS = 2048 / GS_LARGEINT_DIGIT_SIZE_BITS;

            public const int WS_LOGIN_NICK_LEN         =30+1;
            public const int WS_LOGIN_EMAIL_LEN        =50+1;
            public const int WS_LOGIN_PASSWORD_LEN     =30+1;
            public const int WS_LOGIN_UNIQUENICK_LEN   =20+1;
            public const int WS_LOGIN_CDKEY_LEN        =64+1;
            public const int WS_LOGIN_TIMESTAMP_LEN    =64+1;
            public const int WS_LOGIN_KEYHASH_LEN      =33;
            public const int GS_CRYPT_RSA_BYTESIZE     =128 ; 

            [StructLayout(LayoutKind.Sequential)]
            public struct IntArray
            {
                public UInt32 length;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = GS_LARGEINT_MAX_DIGITS)]
                public UInt32[] intArray;
            }
            
            // The following is the structures being used
            [StructLayout(LayoutKind.Sequential)]
            public struct gsCryptRSAKey
            {
                public IntArray modulus;
                public IntArray exponent;
            }

            [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode) ]
            public struct GSLoginCertificate
            {
                public bool mIsValid;
                public UInt32 mLength;
                public UInt32 mVersion;
                public UInt32 mPartnerCode; // aka Account space
                public UInt32 mNamespaceId;
                public UInt32 mUserId;
                public UInt32 mProfileId;
                public UInt32 mExpireTime;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = WS_LOGIN_NICK_LEN)]
                public char[] mProfileNick;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = WS_LOGIN_UNIQUENICK_LEN )]
                public char[] mUniqueNick;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = WS_LOGIN_KEYHASH_LEN)]
                public char[] mCdKeyHash;       // hexstr - bigendian
                public gsCryptRSAKey mPeerPublicKey;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = GS_CRYPT_RSA_BYTESIZE)]
                public byte[] mSignature;       // binary - bigendian
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = GS_CRYPT_RSA_BYTESIZE)]
                public byte[] mServerData;      // binary - bigendian
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = WS_LOGIN_TIMESTAMP_LEN)]
                public char[] mTimestamp;
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct GSLoginCertificatePrivate
            {
                public gsCryptRSAKey mPeerPrivateKey;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
                public byte[] mKeyHash;
            };

            [StructLayout(LayoutKind.Sequential)]
            public struct WSLoginResponse
            {
                public WSLoginValue mLoginResult;
                public WSLoginValue mResponseCode;
                public GSLoginCertificate mCertificate;
                public GSLoginCertificatePrivate mPrivateData;
                public IntPtr mUserData;
            }

            //The callback function type declaration
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void WSLoginCallback(GHTTPResult httpResult, IntPtr theResponse, IntPtr userData);

            // API functions
            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern void wsSetGameCredentials(byte[] accessKey, Int32 gameId, byte[] secretKey);

            [DllImport("gamespy.dll", CharSet= CharSet.Unicode, CallingConvention=CallingConvention.Cdecl)]
            public static extern void wsLoginProfile
            (
                Int32 gameId, 
                Int32 partnerCode, 
                Int32 namespaceId,
                String profileNick,
                String email, 
                String  password,
                String cdkeyhash,
                WSLoginCallback callback, 
                IntPtr userData);
        }
    }
}
