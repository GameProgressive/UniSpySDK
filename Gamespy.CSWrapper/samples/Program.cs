///////////////////////////////////////////////////////////////////////////////
// File:	Program.cs
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.using System;

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
    class Program
    {

        public static void Main()
        {
            
            SakeAppProgram sakeapp = new SakeAppProgram();
            sakeapp.Run();

            AtlasAppProgram atlasapp = new AtlasAppProgram();
            atlasapp.Run();
            
            NatAppProgram   natnegapp = new NatAppProgram();
            //natnegapp.Run();  // This requires 2 applications running simultenously. 
            
            QR2AppProgram qr2app = new QR2AppProgram();
            qr2app.Run();
            
            ServerBrowserAppProgram serverBrowserApp = new ServerBrowserAppProgram();
            serverBrowserApp.Run();
        }
    }
}
