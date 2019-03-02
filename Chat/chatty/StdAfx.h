///////////////////////////////////////////////////////////////////////////////
// File:	StdAfx.h
// SDK:		GameSpy Chat SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
// ------------------------------------
// Include file for standard system include files, or project specific 
// include files that are used frequently, but are changed infrequently.

#if !defined(AFX_STDAFX_H__4CAB521F_F4DE_4629_A857_08E96E0C89E8__INCLUDED_)
#define AFX_STDAFX_H__4CAB521F_F4DE_4629_A857_08E96E0C89E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0501
#define WINVER _WIN32_WINNT
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions
#include <Afxtempl.h>

#include "../chat.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4CAB521F_F4DE_4629_A857_08E96E0C89E8__INCLUDED_)
