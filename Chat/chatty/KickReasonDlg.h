///////////////////////////////////////////////////////////////////////////////
// File:	KickReasonDlg.h
// SDK:		GameSpy Chat SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#if !defined(AFX_KICKREASONDLG_H__25C49432_E9A9_4BEF_8C47_31F8DF889DF0__INCLUDED_)
#define AFX_KICKREASONDLG_H__25C49432_E9A9_4BEF_8C47_31F8DF889DF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CKickReasonDlg dialog

class CKickReasonDlg : public CDialog
{
// Construction
public:
	CKickReasonDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKickReasonDlg)
	enum { IDD = IDD_KICK_REASON };
	CString	m_reason;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKickReasonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKickReasonDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KICKREASONDLG_H__25C49432_E9A9_4BEF_8C47_31F8DF889DF0__INCLUDED_)
