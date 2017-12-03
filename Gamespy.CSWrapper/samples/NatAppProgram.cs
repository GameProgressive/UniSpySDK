///////////////////////////////////////////////////////////////////////////////
// File:	NatAppProgram.cs
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
using Gamespy.Common;
using Gamespy.NatNegoation;
using System.Net.Sockets;
using System.Net;

namespace gamespySample
{
    class NatAppProgram
    {
        public static IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Any, 0); 

        public static void tryReadFrom(Socket sock)
        {
            System.Collections.IList readList = new List<Socket> ();
            readList.Add(sock); 
            Socket.Select(readList, null, null, 0);
            EndPoint fromEndPoint = (EndPoint) new IPEndPoint(IPAddress.Any, 0);
            
            for  (int i = 0; i<readList.Count; i++)
            {
                int length = 0;
                byte[] mybuffer = new byte[255];
                try 
                {
                    length = sock.ReceiveFrom(mybuffer, ref fromEndPoint);
                }
                catch (Exception e) //(SocketException e) 
                {
                    Console.WriteLine("Exception: Problem connecting to host");
                    return;
                }
                Console.WriteLine("Local End Point {0}:{1} length {2}", IPAddress.Parse(((IPEndPoint)sock.LocalEndPoint).Address.ToString()), ((IPEndPoint)sock.LocalEndPoint).Port.ToString(), length);
                Console.WriteLine("Remote end point {0}:{1}", IPAddress.Parse(((IPEndPoint)fromEndPoint).Address.ToString()), ((IPEndPoint)fromEndPoint).Port.ToString());
                
                if (length> 0) 
                {
                    int j = 0;
                    for (; j<gamespyNatNeg.NATNEG_MAGIC_LEN &&( mybuffer[j] == gamespyNatNeg.nnMagic[j]); j++);
                    if (j == gamespyNatNeg.NATNEG_MAGIC_LEN)
                    {
                        gamespySocketIF.sockaddr_in saddr = new gamespySocketIF.sockaddr_in();
                        saddr.sin_addr.S_addr = (UInt32)((IPEndPoint)fromEndPoint).Address.Address;
                        saddr.sin_family = (short) ((IPEndPoint)fromEndPoint).AddressFamily;

                        // IMPORTANT: Port has to be in network byte order.
                        // we need to convert the network order since sockaddr_in expects in that order.
                        short portNo = (short)(((IPEndPoint)fromEndPoint).Port);
                        saddr.sin_port = (ushort)IPAddress.HostToNetworkOrder(portNo); 

                        Console.WriteLine("Received NAT Magic Bytes from {0}:{1}", ((IPEndPoint)fromEndPoint).Port,(System.Net.IPAddress.HostToNetworkOrder(((IPEndPoint)fromEndPoint).Port)));
                        
                        // Marshal sockaddr_in
                        IntPtr remoteSockAddr = Marshal.AllocHGlobal(Marshal.SizeOf(saddr)); 
                        Marshal.StructureToPtr(saddr, remoteSockAddr,false);
                        IntPtr mydata = Marshal.AllocHGlobal(length+1);
                        Marshal.Copy(mybuffer, 0, mydata, (length));
                        
                        gamespyNatNeg.NNProcessData(mydata, length, remoteSockAddr);
                    }
                    else
                    {
                        StringBuilder recvStr = new StringBuilder();
                        ASCIIEncoding asciiString = new ASCIIEncoding();
                        recvStr.Append(asciiString.GetChars(mybuffer));
                        Console.WriteLine("Got Data ({0}:{1}) : {2}", 
                                          IPAddress.Parse(((IPEndPoint)fromEndPoint).Address.ToString()), 
                                          ((IPEndPoint)fromEndPoint).Port.ToString(),
                                          recvStr);
                                         
                    }
                }
            } 
        }
        
        public static Boolean connected = false;
        
        public static void nr(bool success, gamespyNatNeg.NAT nat)
        {
            
            Console.WriteLine("NAT Type: {0}", nat.natType);

            if (success == false)
            {
                Console.WriteLine("Failed to fully detect the NAT.  Please try the detection again.");
                return;
            }

            if (nat.natType !=  gamespyNatNeg.NatType.no_nat && 
                nat.natType !=  gamespyNatNeg.NatType.firewall_only)
            {
                StringBuilder msg = new StringBuilder("The detected NAT is using ");
                switch (nat.mappingScheme)
                {
                    case gamespyNatNeg.NatMappingScheme.private_as_public:
                        msg.Append(" the private port as the public port.");
                        break;
                    case gamespyNatNeg.NatMappingScheme.consistent_port:
                        msg.Append(" the same public port for all requests from the same private port.");
                        break;
                    case gamespyNatNeg.NatMappingScheme.incremental:
                        msg.Append(" an incremental port mapping scheme.");
                        break;
                    case gamespyNatNeg.NatMappingScheme.mixed:
                        msg.Append(" a mixed port mapping scheme.");
                        break;
                    case gamespyNatNeg.NatMappingScheme.unrecognized:
                    default:
                        msg.Append(" an unrecognized port mapping scheme.");
                        break;
                }
                Console.WriteLine(msg);
            }
        }
        public static void natnegProgressCallback(NatNegotiateState state, IntPtr userdata)
        {
            StringBuilder msg = new StringBuilder("Received State Update: ");

            switch (state)
           {
                case NatNegotiateState.ns_initack:
                    msg.Append("ns_initack, Init Packets Acknowledged.");
                    break;
                case NatNegotiateState.ns_connectping:
                    msg.Append("ns_connectping, Starting connection and pinging other machine.\n");
                    break;
                default:
                    break;
            }
            Console.WriteLine(msg);
        }
    
        public static void natnegCompleteCallback
        (
            NatNegotiateResult result, 
            IntPtr gamesocket,
            IntPtr remoteaddr, /*struct sockaddr_in * */
            IntPtr  userdata)
        {	       
		    switch(result)
		    {
		        case NatNegotiateResult.nr_success:
		            Console.WriteLine("NatNeg Result: Nat Negoation Success!");
		            break;
                case NatNegotiateResult.nr_deadbeatpartner:
			        Console.WriteLine("NatNeg Result: nr_deadbeatpartner, The other machine isn't responding.");
                    return;
                case NatNegotiateResult.nr_inittimeout:
                    Console.WriteLine("NatNeg Result: nr_inittimeout, The NAT server could not be contacted.");
                    return;
                case NatNegotiateResult.nr_pingtimeout:
                    Console.WriteLine("NatNeg Result: nr_pingtimeout, The other machine could not be contacted.");
                    return;
		        default:
		            Console.WriteLine("NatNeg Result: Nat Negoation Unknown error! %d", result);
                    return;
		    }
    		
            if (result != NatNegotiateResult.nr_success)
	        {
	            return;
	        }
	    
	        if (gamesocket != IntPtr.Zero)
	        {                
		        Console.WriteLine("NatNeg Complete: Local game socket: {0}", gamesocket.ToString());
	        }
    
	        if (remoteaddr != IntPtr.Zero)
	        {
                gamespySocketIF.sockaddr_in remoteSockaddr = new gamespySocketIF.sockaddr_in();
                remoteSockaddr = (gamespySocketIF.sockaddr_in) Marshal.PtrToStructure(remoteaddr, remoteSockaddr.GetType());
        
                // IMPORTANT: remote sock port is in the network order. Convert to host order. 
                // Unlike C, in C# the IP Address and Port conversion is transparent, done in the .Net Sockets 
                ushort portno = (ushort)(IPAddress.NetworkToHostOrder((short)remoteSockaddr.sin_port));
                IPAddress rIpAddr = new IPAddress(remoteSockaddr.sin_addr.S_addr);
                Console.WriteLine("NatNeg Got connected, Remote Address = {0} {1}:{2}",
                                  rIpAddr.AddressFamily, 
                                  rIpAddr.ToString(),
                                  portno);
                remoteEndPoint.Address = rIpAddr; 
                remoteEndPoint.Port    = portno; 
                connected = true;

	        }
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
        
        public static gamespyNatNeg.NatDetectionResultsFunc natDetectCallback = new gamespyNatNeg.NatDetectionResultsFunc(nr);
        public static gamespyNatNeg.NegotiateProgressFunc natProgressCallback = new gamespyNatNeg.NegotiateProgressFunc(natnegProgressCallback);
        public static gamespyNatNeg.NegotiateCompletedFunc natCompleteCallback = new gamespyNatNeg.NegotiateCompletedFunc(natnegCompleteCallback);
        public static Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

        public void Run()
        {
            
            string gamename = "gmtest";
            Console.WriteLine("\n------------ Natneg C# sample ------------------------\n");

            if (CheckServices(gamename) != GSIACResult.GSIACAvailable)
            {
                Console.WriteLine("{0}: Online services are unavailable now.", gamename);
                return;
            }
            Console.WriteLine("Please enter the test index 0 or 1");
            int index = Console.ReadLine().Equals("0")?0:1;
#if GSI_COMMON_DEBUG
            StringBuilder debugFileName = new StringBuilder("Debug_NatNeg_");
            debugFileName.Append(index);
            debugFileName.Append(".log");

            gamespyCommonDebug.gsOpenDebugFile(debugFileName.ToString());
            gamespyCommonDebug.gsSetDebugLevel(GSIDebugCategory.GSIDebugCat_All, GSIDebugType.GSIDebugType_All, GSIDebugLevel.Hardcore);
#endif
            
            NatNegotiateError error = gamespyNatNeg.NNStartNatDetection(natDetectCallback);
            // Create and bind socket to the local IP
            IPAddress localIPAddr = Dns.Resolve(Dns.GetHostName()).AddressList[0];
            Console.WriteLine("My local IpAddress is :" + localIPAddr.ToString ());
            EndPoint localEndPoint = new IPEndPoint(localIPAddr, 0);
            
            sock.Bind(localEndPoint);
            Console.WriteLine("Local Endpoint {0}:{1}", ((IPEndPoint)sock.LocalEndPoint).Address.ToString(), ((IPEndPoint)sock.LocalEndPoint).Port.ToString());
          
            error = gamespyNatNeg.NNBeginNegotiationWithSocket(sock.Handle, 666, index, natProgressCallback, natCompleteCallback, IntPtr.Zero);
          
            //EndPoint remoteEndPoint 
            double startTimeInMs = DateTime.Now.TimeOfDay.TotalMilliseconds;
            double currentTimeInMs = DateTime.Now.TimeOfDay.TotalMilliseconds;
            double sendTime = 0;  
            while ((DateTime.Now.TimeOfDay.TotalMilliseconds - startTimeInMs) < 60000)
            {
                gamespyNatNeg.NNThink();
                if (connected)
                {
                    if ((DateTime.Now.TimeOfDay.TotalMilliseconds - sendTime)> 2000)
                    {
                        byte[] sendBuffer = ASCIIEncoding.ASCII.GetBytes("Hello There!");
                        sock.SendTo(sendBuffer, (EndPoint) remoteEndPoint);
                        sendTime = DateTime.Now.TimeOfDay.TotalMilliseconds;
                    }
                }
                tryReadFrom(sock);
                Thread.Sleep(100);
            }
            sock.Shutdown(SocketShutdown.Both);
            sock.Close();
            gamespyNatNeg.NNFreeNegotiateList();
        }
    }
}