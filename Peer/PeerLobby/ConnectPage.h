///////////////////////////////////////////////////////////////////////////////
// File:	ConnectPage.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#if !defined(AFX_CONNECTPAGE_H__70C3619F_ED14_49F8_9155_9F96147FF4C2__INCLUDED_)
#define AFX_CONNECTPAGE_H__70C3619F_ED14_49F8_9155_9F96147FF4C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CConnectPage dialog

class CConnectPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConnectPage)

// Construction
public:
	CConnectPage();
	~CConnectPage();

// Dialog Data
	//{{AFX_DATA(CConnectPage)
	enum { IDD = IDD_CONNECT_PAGE };
	CString	m_nick;
	CString	m_title;
	BOOL	m_groupRooms;
	CString	m_key;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConnectPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConnectPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

extern CConnectPage * ConnectPage;

#endif // !defined(AFX_CONNECTPAGE_H__70C3619F_ED14_49F8_9155_9F96147FF4C2__INCLUDED_)
