///////////////////////////////////////////////////////////////////////////////
// File:	NATify.h
// SDK:		GameSpy NAT Negotiation SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.


#if !defined(AFX_NATIFY_H__B8FF4369_8789_4674_8569_3D52CE8CA890__INCLUDED_)
#define AFX_NATIFY_H__B8FF4369_8789_4674_8569_3D52CE8CA890__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define NATIFY_COOKIE		777
#define NATIFY_TIMEOUT		10000
#define NATIFY_STATUS_STEPS (NATIFY_TIMEOUT / 1000) + 7

typedef enum { packet_map1a, packet_map2, packet_map3, packet_map1b, NUM_PACKETS } NatifyPacket;
typedef enum { no_nat, firewall_only, full_cone, restricted_cone, port_restricted_cone, symmetric, unknown, NUM_NAT_TYPES } NatType;
typedef enum { promiscuous, not_promiscuous, port_promiscuous, ip_promiscuous, promiscuity_not_applicable, NUM_PROMISCUITY_TYPES } NatPromiscuity;
typedef enum { unrecognized, private_as_public, consistent_port, incremental, mixed, NUM_MAPPING_SCHEMES } NatMappingScheme;

//////////////////////////////////////////////////////////////
// AddressMapping
// Summary
//		Internal and external address pairing for an observed network address 
//		translation.
// See Also
//		NAT
typedef struct _AddressMapping {
	unsigned int privateIp;		// Internal IP address.
	unsigned short privatePort;	// Internal port number.
	unsigned int publicIp;		// External IP address.
	unsigned short publicPort;	// External port number.
} AddressMapping;

//////////////////////////////////////////////////////////////
// NAT
// Summary
//		The result of a NAT detection.  
//		Upon successful completion of a detection, this will contain as many 
//		properties of the NAT as could be determined.
// See Also
//		NNStartNatDetection, NatDetectionResultsFunc, NatType, NatMappingScheme, NatPromiscuity, AddressMapping
typedef struct _NAT {
	char brand[32];					// NAT device brand/vendor (not currently used).
	char model[32];					// NAT device model name/number (not currently used).
	char firmware[64];				// NAT device brand/vendor (not currently used).
	gsi_bool ipRestricted;			// gsi_true if the NAT drops packets from unsolicited IP addresses.
	gsi_bool portRestricted;		// gsi_true if the NAT drops packets from unsolicited ports.
	NatPromiscuity promiscuity;		// The type of promiscuity the NAT allows.
	NatType natType;				// The type of NAT as defined by RFC2663.
	NatMappingScheme mappingScheme;	// The type of port mapping/allocation scheme used by the NAT.
	AddressMapping mappings[4];		// Port mappings observed during the detection process.
	gsi_bool qr2Compatible;			// gsi_true if the NAT is compatible with QR2.
} NAT;

int DiscoverReachability(SOCKET sock, unsigned int ip, unsigned short port, int portType);
int DiscoverMapping(SOCKET sock, unsigned int ip, unsigned short port, int portType, int id);
int NatifyThink(SOCKET sock, NAT * nat);
gsi_bool DetermineNatType(NAT * nat);
void OutputMapping(const AddressMapping * theMap);


#endif // !defined(AFX_NATIFY_H__B8FF4369_8789_4674_8569_3D52CE8CA890__INCLUDED_)
