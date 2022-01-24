///////////////////////////////////////////////////////////////////////////////
// File:	gt2test.h
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#if !defined(AFX_GT2TEST_H__BDC522B0_AA93_4D1B_8271_A3CFD716E950__INCLUDED_)
#define AFX_GT2TEST_H__BDC522B0_AA93_4D1B_8271_A3CFD716E950__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGt2testApp:
// See gt2test.cpp for the implementation of this class
//

class CGt2testApp : public CWinApp
{
public:
	CGt2testApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGt2testApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGt2testApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before 
// the previous line.

#endif // !defined(AFX_GT2TEST_H__BDC522B0_AA93_4D1B_8271_A3CFD716E950__INCLUDED_)
