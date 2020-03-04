///////////////////////////////////////////////////////////////////////////////
// File:	sbmfcsample.h
// SDK:		GameSpy Server Browsing SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
// ------------------------------------
// Main header file for the SBMFCSAMPLE application.

#if !defined(AFX_SBMFCSAMPLE_H__F2EE1BD5_6089_4F00_9E83_2D0A7C9AA592__INCLUDED_)
#define AFX_SBMFCSAMPLE_H__F2EE1BD5_6089_4F00_9E83_2D0A7C9AA592__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSbmfcsampleApp:
// See sbmfcsample.cpp for the implementation of this class
//

class CSbmfcsampleApp : public CWinApp
{
public:
	CSbmfcsampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSbmfcsampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSbmfcsampleApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SBMFCSAMPLE_H__F2EE1BD5_6089_4F00_9E83_2D0A7C9AA592__INCLUDED_)
