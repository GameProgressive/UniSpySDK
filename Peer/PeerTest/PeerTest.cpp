///////////////////////////////////////////////////////////////////////////////
// File:	PeerTest.cpp
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "stdafx.h"
#include "PeerTest.h"
#include "PeerTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPeerTestApp

BEGIN_MESSAGE_MAP(CPeerTestApp, CWinApp)
	//{{AFX_MSG_MAP(CPeerTestApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeerTestApp construction

CPeerTestApp::CPeerTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPeerTestApp object

CPeerTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPeerTestApp initialization

BOOL CPeerTestApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
*/
	CPeerTestDlg dlg2;
	m_pMainWnd = &dlg2;
	int nResponse = dlg2.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
