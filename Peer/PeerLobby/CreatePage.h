///////////////////////////////////////////////////////////////////////////////
// File:	CreatePage.h
// SDK:		GameSpy Peer SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#if !defined(AFX_CREATEPAGE_H__4FC1C8CA_9B4C_47F6_B226_C211DC74D504__INCLUDED_)
#define AFX_CREATEPAGE_H__4FC1C8CA_9B4C_47F6_B226_C211DC74D504__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCreatePage dialog

class CCreatePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CCreatePage)

// Construction
public:
	CCreatePage();
	~CCreatePage();

// Dialog Data
	//{{AFX_DATA(CCreatePage)
	enum { IDD = IDD_CREATE_PAGE };
	CString	m_name;
	int		m_maxPlayers;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCreatePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCreatePage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

extern CCreatePage * CreatePage;

#endif // !defined(AFX_CREATEPAGE_H__4FC1C8CA_9B4C_47F6_B226_C211DC74D504__INCLUDED_)
