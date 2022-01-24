///////////////////////////////////////////////////////////////////////////////
// File:	gt2testDlg.cpp
// SDK:		GameSpy Transport 2 SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "stdafx.h"
#include "gt2test.h"
#include "gt2testDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CGt2testDlg * dlg;

/////////////////////////////////////////////////////////////////////////////
// CGt2testDlg dialog

CGt2testDlg::CGt2testDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGt2testDlg::IDD, pParent)
{
	m_delayedSends = NULL;
	m_delayedReceives = NULL;

	//{{AFX_DATA_INIT(CGt2testDlg)
	m_addressAliases = _T("");
	m_addressHostname = _T("");
	m_addressIPs = _T("");
	m_addressPort = _T("");
	m_addressString = _T("");
	m_outBufferSize = _T("");
	m_rejectReason = _T("");
	m_acceptMode = 0;
	m_alwaysThink = TRUE;
	m_blocking = FALSE;
	m_localAddress = _T("");
	m_remoteAddress = _T("");
	m_inBufferSize = _T("");
	m_listen = FALSE;
	m_connectionState = -1;
	m_reliable = FALSE;
	m_message = _T("");
	m_messages = _T("");
	m_receiveDelay = FALSE;
	m_receiveDelayValue = _T("0");
	m_receiveDrop = FALSE;
	m_receiveDropValue = _T("0");
	m_receiveROT13 = FALSE;
	m_sendDelay = FALSE;
	m_sendDelayValue = _T("0");
	m_sendDrop = FALSE;
	m_sendDropValue = _T("0");
	m_sendROT13 = FALSE;
	m_connectMessage = _T("");
	m_timeout = _T("5000");
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGt2testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGt2testDlg)
	DDX_Control(pDX, IDC_OUTGOING_BUFFER_SIZE, m_outgoingBufferSize);
	DDX_Control(pDX, IDC_OUTGOING_BUFFER_PROGRESS, m_outgoingBufferProgress);
	DDX_Control(pDX, IDC_INCOMING_BUFFER_SIZE, m_incomingBufferSize);
	DDX_Control(pDX, IDC_INCOMING_BUFFER_PROGRESS, m_incomingBufferProgress);
	DDX_Control(pDX, IDC_CONNECTIONS, m_connections);
	DDX_Control(pDX, IDC_ADDRESS_IP, m_addressIP);
	DDX_Text(pDX, IDC_ADDRESS_ALIASES, m_addressAliases);
	DDX_Text(pDX, IDC_ADDRESS_HOSTNAME, m_addressHostname);
	DDX_Text(pDX, IDC_ADDRESS_IPS, m_addressIPs);
	DDX_Text(pDX, IDC_ADDRESS_PORT, m_addressPort);
	DDX_Text(pDX, IDC_ADDRESS_STRING, m_addressString);
	DDX_Text(pDX, IDC_OUT_BUFFER_SIZE, m_outBufferSize);
	DDX_Text(pDX, IDC_REJECT_REASON, m_rejectReason);
	DDX_Radio(pDX, IDC_ACCEPT_ALL, m_acceptMode);
	DDX_Check(pDX, IDC_ALWAYS_THINK, m_alwaysThink);
	DDX_Check(pDX, IDC_BLOCKING, m_blocking);
	DDX_Text(pDX, IDC_LOCAL_ADDRESS, m_localAddress);
	DDX_Text(pDX, IDC_REMOTE_ADDRESS, m_remoteAddress);
	DDX_Text(pDX, IDC_IN_BUFFER_SIZE, m_inBufferSize);
	DDX_Check(pDX, IDC_LISTEN, m_listen);
	DDX_Radio(pDX, IDC_CONNECTING, m_connectionState);
	DDX_Check(pDX, IDC_RELIABLE, m_reliable);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
	DDX_Text(pDX, IDC_MESSAGES, m_messages);
	DDX_Check(pDX, IDC_RECEIVE_DELAY, m_receiveDelay);
	DDX_Text(pDX, IDC_RECEIVE_DELAY_VALUE, m_receiveDelayValue);
	DDX_Check(pDX, IDC_RECEIVE_DROP, m_receiveDrop);
	DDX_Text(pDX, IDC_RECEIVE_DROP_VALUE, m_receiveDropValue);
	DDX_Check(pDX, IDC_RECEIVE_ROT13, m_receiveROT13);
	DDX_Check(pDX, IDC_SEND_DELAY, m_sendDelay);
	DDX_Text(pDX, IDC_SEND_DELAY_VALUE, m_sendDelayValue);
	DDX_Check(pDX, IDC_SEND_DROP, m_sendDrop);
	DDX_Text(pDX, IDC_SEND_DROP_VALUE, m_sendDropValue);
	DDX_Check(pDX, IDC_SEND_ROT13, m_sendROT13);
	DDX_Text(pDX, IDC_CONNECT_MESSAGE, m_connectMessage);
	DDX_Text(pDX, IDC_TIMEOUT, m_timeout);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGt2testDlg, CDialog)
	//{{AFX_MSG_MAP(CGt2testDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADDRESS_TO, OnAddressTo)
	ON_BN_CLICKED(IDC_ADDRESS_FROM, OnAddressFrom)
	ON_BN_CLICKED(IDC_GET_IP_HOST_INFO, OnGetIpHostInfo)
	ON_BN_CLICKED(IDC_GET_STRING_HOST_INFO, OnGetStringHostInfo)
	ON_BN_CLICKED(IDC_CLOSE_SOCKET, OnCloseSocket)
	ON_BN_CLICKED(IDC_CREATE_SOCKET, OnCreateSocket)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_THINK, OnThink)
	ON_BN_CLICKED(IDC_LISTEN, OnListen)
	ON_BN_CLICKED(IDC_CLOSE_ALL_CONNECTIONS, OnCloseAllConnections)
	ON_BN_CLICKED(IDC_CLOSE_ALL_CONNECTIONS_HARD, OnCloseAllConnectionsHard)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_BN_CLICKED(IDC_PING, OnPing)
	ON_BN_CLICKED(IDC_CLOSE_CONNECTION, OnCloseConnection)
	ON_BN_CLICKED(IDC_CLOSE_CONNECTION_HARD, OnCloseConnectionHard)
	ON_CBN_SELCHANGE(IDC_CONNECTIONS, OnSelchangeConnections)
	ON_BN_CLICKED(IDC_SEND_ROT13, OnSendRot13)
	ON_BN_CLICKED(IDC_RECEIVE_ROT13, OnReceiveRot13)
	ON_BN_CLICKED(IDC_SEND_DROP, OnSendDrop)
	ON_BN_CLICKED(IDC_RECEIVE_DROP, OnReceiveDrop)
	ON_BN_CLICKED(IDC_SEND_DELAY, OnSendDelay)
	ON_BN_CLICKED(IDC_RECEIVE_DELAY, OnReceiveDelay)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_SEND_DROP_VALUE, OnChangeSendDropValue)
	ON_EN_CHANGE(IDC_SEND_DELAY_VALUE, OnChangeSendDelayValue)
	ON_EN_CHANGE(IDC_RECEIVE_DROP_VALUE, OnChangeReceiveDropValue)
	ON_EN_CHANGE(IDC_RECEIVE_DELAY_VALUE, OnChangeReceiveDelayValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGt2testDlg message handlers

static void DelayedMessageFree(void * elem)
{
	DelayedMessage * message = (DelayedMessage *)elem;

	free(message->message);
}

BOOL CGt2testDlg::OnInitDialog()
{
	// MFC stuff
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// So we can reference members from callbacks.
	dlg = this;

	// Disable the socket controls.
	EnableSocketControls(FALSE);

	// We initialize sockets ourselves because they might be needed for the 
	// address functions. This function is declared in nonport.h.
	SocketStartUp();

	// Setup the think timer.
	SetTimer(100, 20, NULL);

	// Not thinking yet.
	m_thinking = GT2False;

	// Create the delay arrays.
	m_delayedSends = ArrayNew(sizeof(DelayedMessage), 10, DelayedMessageFree);
	m_delayedReceives = ArrayNew(sizeof(DelayedMessage), 10, DelayedMessageFree);

	return TRUE;
}

void CGt2testDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// Free our arrays of delayed messages.
	if(m_delayedSends)
	{
		ArrayFree(m_delayedSends);
		m_delayedSends = NULL;
	}
	if(m_delayedReceives)
	{
		ArrayFree(m_delayedReceives);
		m_delayedReceives = NULL;
	}
}

// MFC stuff
void CGt2testDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// MFC stuff
HCURSOR CGt2testDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/************
** GENERAL **
************/

// Enable or disable a control based on it's control ID.
#define ENABLE(control, enable) GetDlgItem(control)->EnableWindow(enable)

// Check if a delayed message should be passed on to it's filter function.
static void DelayCheck(DArray delayedMessages, unsigned long now, BOOL send)
{
	DelayedMessage * message;
	int num;
	int i;

	num = ArrayLength(delayedMessages);

	// Loop through the list backwards to allow for safe removals.
	for(i = (num - 1) ; i >= 0 ; i--)
	{
		message = (DelayedMessage *)ArrayNth(delayedMessages, i);

		// Check if the time passed since the start time is greater than the
		// delay time.
		if((now - message->startTime) > message->delayTime)
		{
			if(send)
				gt2FilteredSend(message->connection, message->filterID, message->message, message->len, message->reliable);
			else
				gt2FilteredReceive(message->connection, message->filterID, message->message, message->len, message->reliable);
			ArrayDeleteAt(delayedMessages, i);
		}
	}
}

static void RemoveDelayedMessages(DArray delayedMessages, GT2Connection connection)
{
	DelayedMessage * message;
	int num;
	int i;

	num = ArrayLength(delayedMessages);

	// Loop through the list backwards to allow for safe removals.
	for(i = (num - 1) ; i >= 0 ; i--)
	{
		message = (DelayedMessage *)ArrayNth(delayedMessages, i);

		// Remove it if the connection matches.
		if(message->connection == connection)
			ArrayDeleteAt(delayedMessages, i);
	}
}

void CGt2testDlg::OnTimer(UINT nIDEvent) 
{
	// Check for our timer.
	if(nIDEvent == 100)
	{
		// We have a socket, "always think" is checked, and we're not already
		// thinking; the check for already thinking is because this can get 
		// called from inside a dialog box brought up inside a callback -- this
		// would be bad (recursive thinking).
		m_alwaysThink = (((CButton *)GetDlgItem(IDC_ALWAYS_THINK))->GetCheck() == BST_CHECKED);
		if(m_socket && m_alwaysThink && !m_thinking)
		{
			m_thinking = GT2True;
			gt2Think(m_socket);
			m_thinking = GT2False;
		}

		// Find the active connection, and check what the connection's state 
		// should be.
		GT2Connection connection = GetActiveConnection();
		int newState;
		if(connection)
			newState = gt2GetConnectionState(connection);
		else
			newState = -1;

		// If this differs from the previous connection state, make the change.
		if(newState != m_connectionState)
		{
			// If there's a state, check it.
			if(newState != -1)
				((CButton *)GetDlgItem(IDC_CONNECTING + newState))->SetCheck(BST_CHECKED);

			// If there's a previous state, uncheck it.
			if(m_connectionState != -1)
				((CButton *)GetDlgItem(IDC_CONNECTING + m_connectionState))->SetCheck(BST_UNCHECKED);

			// Save the new state.
			m_connectionState = newState;
		}

		// Update the buffer display.
		if(connection)
		{
			int freeSpace;
			int size;
			CString text;

			freeSpace = gt2GetIncomingBufferFreeSpace(connection);
			size = gt2GetIncomingBufferSize(connection);
			text.Format("%d", size);
			m_incomingBufferSize.SetWindowText(text);
			m_incomingBufferProgress.SetRange32(0, size);
			m_incomingBufferProgress.SetPos(size - freeSpace);

			freeSpace = gt2GetOutgoingBufferFreeSpace(connection);
			size = gt2GetOutgoingBufferSize(connection);
			text.Format("%d", size);
			m_outgoingBufferSize.SetWindowText(text);
			m_outgoingBufferProgress.SetRange32(0, size);
			m_outgoingBufferProgress.SetPos(size - freeSpace);
		}
		else
		{
			m_incomingBufferSize.SetWindowText("");
			m_incomingBufferProgress.SetPos(0);
			m_outgoingBufferSize.SetWindowText("");
			m_outgoingBufferProgress.SetPos(0);
		}

		// Check for delayed messages.
		unsigned long now = current_time();
		DelayCheck(m_delayedSends, now, TRUE);
		DelayCheck(m_delayedReceives, now, FALSE);
	}

	// MFC stuff.
	CDialog::OnTimer(nIDEvent);
}

BOOL CGt2testDlg::PreTranslateMessage(MSG* pMsg) 
{
	// Redirect enter.
	//////////////////
	if((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == 0x0D))
	{
		CWnd * pWnd = GetFocus();
		if(pWnd)
		{
			int id = pWnd->GetDlgCtrlID();

			if(id == IDC_LOCAL_ADDRESS)
			{
				OnCreateSocket();
				return TRUE;
			}
			if(id == IDC_REMOTE_ADDRESS)
			{
				OnConnect();
				return TRUE;
			}
			if(id == IDC_CONNECT_MESSAGE)
			{
				OnConnect();
				return TRUE;
			}
			if(id == IDC_MESSAGE)
			{
				OnSend();
				return TRUE;
			}
			if(id == IDC_ADDRESS_IP)
			{
				OnAddressTo();
				return TRUE;
			}
			if(id == IDC_ADDRESS_PORT)
			{
				OnAddressTo();
				return TRUE;
			}
			if(id == IDC_ADDRESS_STRING)
			{
				OnAddressFrom();
				return TRUE;
			}
		}
	}

	// MFC stuff.
	return CDialog::PreTranslateMessage(pMsg);
}

static const char * ResultToString(GT2Result result)
{
	switch(result)
	{
	case GT2Success:
		return "GT2Success";
	case GT2OutOfMemory:
		return "GT2OutOfMemory";
	case GT2Rejected:
		return "GT2Rejected";
	case GT2NetworkError:
		return "GT2NetworkError";
	case GT2AddressError:
		return "GT2AddressError";
	case GT2DuplicateAddress:
		return "GT2DuplicateAddress";
	case GT2TimedOut:
		return "GT2TimedOut";
	case GT2NegotiationError:
		return "GT2NegotiationError";
	}

	return "Unknown result";
}

// This is used to enable or disable socket controls when a socket is created 
// or destroyed.
void CGt2testDlg::EnableSocketControls(BOOL enable)
{
	if(!enable)
		m_socket = NULL;

	ENABLE(IDC_CREATE_SOCKET, !enable);
	ENABLE(IDC_CLOSE_SOCKET, enable);
	ENABLE(IDC_LOCAL_ADDRESS, !enable);
	ENABLE(IDC_IN_BUFFER_SIZE, !enable);
	ENABLE(IDC_OUT_BUFFER_SIZE, !enable);
	ENABLE(IDC_CONNECT, enable);
	ENABLE(IDC_REMOTE_ADDRESS, enable);
	ENABLE(IDC_BLOCKING, enable);
	ENABLE(IDC_CLOSE_ALL_CONNECTIONS, enable);
	ENABLE(IDC_CLOSE_ALL_CONNECTIONS_HARD, enable);
	ENABLE(IDC_THINK, enable);
	ENABLE(IDC_ALWAYS_THINK, enable);
	ENABLE(IDC_LISTEN, enable);
	ENABLE(IDC_CONNECT_MESSAGE, enable);
	ENABLE(IDC_TIMEOUT, enable);

	EnableListenControls(FALSE);
	EnableConnectionControls(FALSE);
}

// This is used to enable or disable the listen controls, depending on if the 
// socket is listening or not.
void CGt2testDlg::EnableListenControls(BOOL enable)
{
	ENABLE(IDC_ACCEPT_ALL, enable);
	ENABLE(IDC_REJECT_ALL, enable);
	ENABLE(IDC_PROMPT, enable);
	ENABLE(IDC_REJECT_REASON, enable);
}

// This is used to enable or disable the connection controls, depending on if 
// there's a connection or not.
void CGt2testDlg::EnableConnectionControls(BOOL enable)
{
	ENABLE(IDC_CONNECTIONS, enable);
	ENABLE(IDC_CONNECTING, enable);
	ENABLE(IDC_CONNECTED, enable);
	ENABLE(IDC_CLOSING, enable);
	ENABLE(IDC_CLOSED, enable);
	ENABLE(IDC_SEND, enable);
	ENABLE(IDC_MESSAGE, enable);
	ENABLE(IDC_RELIABLE, enable);
	ENABLE(IDC_PING, enable);
	ENABLE(IDC_CLOSE_CONNECTION, enable);
	ENABLE(IDC_CLOSE_CONNECTION_HARD, enable);
	ENABLE(IDC_SEND_ROT13, enable);
	ENABLE(IDC_RECEIVE_ROT13, enable);
	ENABLE(IDC_SEND_DROP, enable);
	ENABLE(IDC_RECEIVE_DROP, enable);
	ENABLE(IDC_SEND_DROP_VALUE, enable);
	ENABLE(IDC_RECEIVE_DROP_VALUE, enable);
	ENABLE(IDC_SEND_DELAY, enable);
	ENABLE(IDC_RECEIVE_DELAY, enable);
	ENABLE(IDC_SEND_DELAY_VALUE, enable);
	ENABLE(IDC_RECEIVE_DELAY_VALUE, enable);
}

// This gets the list index for a connection object.
int CGt2testDlg::GetConnectionIndex(GT2Connection connection)
{
	int nIndex;
	int num = m_connections.GetCount();

	for(nIndex = 0 ; nIndex < num ; nIndex++)
	{
		if(((ConnectionInfo *)m_connections.GetItemDataPtr(nIndex))->connection == connection)
			return nIndex;
	}

	return -1;
}

// This gets the connection for a given list index.
GT2Connection CGt2testDlg::GetConnection(int nIndex)
{
	if(nIndex == -1)
		return NULL;

	ConnectionInfo * info = (ConnectionInfo *)m_connections.GetItemDataPtr(nIndex);
	if(!info)
		return NULL;
	
	return info->connection;
}

// Get the connection info for a given list index.
ConnectionInfo * CGt2testDlg::GetConnectionInfo(int nIndex)
{
	if(nIndex == -1)
		return NULL;

	return (ConnectionInfo *)m_connections.GetItemDataPtr(nIndex);
}

ConnectionInfo * CGt2testDlg::GetConnectionInfo(GT2Connection connection)
{
	return GetConnectionInfo(GetConnectionIndex(connection));
}

// This adds a connection to the connection list.
void CGt2testDlg::AddConnection(GT2Connection connection)
{
	// The string to show in the list is its IP and port.
	CString address = gt2AddressToString(gt2GetRemoteIP(connection), gt2GetRemotePort(connection), NULL);

	// Add the string to the list.
	int nIndex = m_connections.AddString(address);
	if(nIndex == -1)
	{
		MessageBox("Error adding address to list!");
		return;
	}

	// Allocation the connection's info.
	ConnectionInfo * info = new ConnectionInfo;
	info->connection = connection;
	info->messages.Empty();
	info->sendROT13 = FALSE;
	info->receiveROT13 = FALSE;
	info->sendDrop = FALSE;
	info->receiveDrop = FALSE;
	info->sendDelay = FALSE;
	info->receiveDelay = FALSE;

	// Set the string's data pointer to point to its info.
	m_connections.SetItemDataPtr(nIndex, info);

	// We have a connection, so enable the controls.
	EnableConnectionControls();

	// Set this connection as the active connection.
	SetActiveConnection(connection);
}

// Sets a connection as active (or, if connection is NULL and nIndex is -1, 
// sets the list to no active connection).
void CGt2testDlg::SetActiveConnection(GT2Connection connection, int nIndex)
{
	// Make sure this connection is selected.
	m_connections.SetCurSel(nIndex);

	// Enable the connection controls depending on if this is a connection or 
	// not.
	EnableConnectionControls(connection != NULL);

	// Make sure the controls reflect the proper connection (or lack thereof).
	if(connection)
	{
		// Update the connection values with the saved values for this 
		// connection.
		ConnectionInfo * info = GetConnectionInfo(connection);
		m_messages = info->messages;
		m_sendROT13 = info->sendROT13;
		m_receiveROT13 = info->receiveROT13;
		m_sendDrop = info->sendDrop;
		m_receiveDrop = info->receiveDrop;
		m_sendDropValue = info->sendDropValue;
		m_receiveDropValue = info->receiveDropValue;
		m_sendDelay = info->sendDelay;
		m_receiveDelay = info->receiveDelay;
		m_sendDelayValue = info->sendDelayValue;
		m_receiveDelayValue = info->receiveDelayValue;
	}
	else
	{
		// Blank out all the connection values.
		m_messages = "";
		m_sendROT13 = FALSE;
		m_receiveROT13 = FALSE;
		m_sendDrop = FALSE;
		m_receiveDrop = FALSE;
		m_sendDropValue = "";
		m_receiveDropValue = "";
		m_sendDelay = FALSE;
		m_receiveDelay = FALSE;
		m_sendDelayValue = "";
		m_receiveDelayValue = "";
	}
}

// Set the active connection using a connection object.
void CGt2testDlg::SetActiveConnection(GT2Connection connection)
{
	int nIndex = GetConnectionIndex(connection);
	ASSERT(nIndex != -1);  // Tried to activate a bad connection.
	if(nIndex == -1)
		return;

	SetActiveConnection(connection, nIndex);
}

// Set the active connection using a list index.
void CGt2testDlg::SetActiveConnection(int nIndex)
{
	if(nIndex != -1)
		SetActiveConnection(GetConnection(nIndex), nIndex);
	else
		SetActiveConnection(NULL, nIndex);
}

// Get the selected connection, or NULL if there is none.
GT2Connection CGt2testDlg::GetActiveConnection()
{
	if(!m_connections.IsWindowEnabled())
		return NULL;

	int nIndex = m_connections.GetCurSel();
	if(nIndex == -1)
		return NULL;

	return GetConnection(nIndex);
}

// Remove a connection from the list.
void CGt2testDlg::RemoveConnection(GT2Connection connection)
{
	// Check for any delayed messages that are supposed to use this 
	// connection.
	RemoveDelayedMessages(m_delayedSends, connection);
	RemoveDelayedMessages(m_delayedReceives, connection);

	// Get this connection's index.
	int nIndex = GetConnectionIndex(connection);
	if(nIndex == -1)
		return;

	// Check if this is the current selection.
	BOOL selected = (m_connections.GetCurSel() == nIndex);

	// Free its info.
	delete GetConnectionInfo(nIndex);

	// Delete it.
	m_connections.DeleteString(nIndex);

	UpdateData();

	// If it was selected, select something else.
	if(selected)
	{
		int count = m_connections.GetCount();
		if(count)
		{
			// If it was last, select the previous connection, otherwise select
			// the next one.
			if(nIndex == count)
				SetActiveConnection(nIndex - 1);
			else
				SetActiveConnection(nIndex);
		}
		else
		{
			// It was the only one, set to no selection.
			SetActiveConnection(-1);
		}
	}

	UpdateData(FALSE);
}

// Adds a string to a connection's message list and updates the displayed 
// messages list if this is the active connection.
void CGt2testDlg::AddMessageString(GT2Connection connection, const char *string)
{
	// Add this to the connection's info.
	ConnectionInfo * info = GetConnectionInfo(connection);
	if(info)
		info->messages += string;
	
	// Is this the active connection?
	if(GetActiveConnection() == connection)
	{
		// Update the messages list.
		UpdateData();
		m_messages += string;
		UpdateData(FALSE);

		// Scroll the messages to the bottom.
		CEdit * messagesCtrl = (CEdit *)GetDlgItem(IDC_MESSAGES);
		messagesCtrl->SetSel(m_messages.GetLength(), m_messages.GetLength());
	}
}

/***********
** SOCKET **
***********/

static void SocketErrorCallback(GT2Socket socket)
{
	dlg->EnableSocketControls(FALSE);

	dlg->MessageBox("Socket error");

	GSI_UNUSED(socket);
}

static void SendDumpCallback(GT2Socket socket, GT2Connection connection, unsigned int ip, unsigned short port, GT2Bool reset, const GT2Byte * message, int len)
{
	CString str;
	str.Format("SEND %p %p %s %d\n", socket, connection, gt2AddressToString(ip, port, NULL), len);
	OutputDebugString(str);

	GSI_UNUSED(message);
	GSI_UNUSED(reset);
}

static void ReceiveDumpCallback(GT2Socket socket, GT2Connection connection, unsigned int ip, unsigned short port, GT2Bool reset, const GT2Byte * message, int len)
{
	CString str;
	if(reset)
		str.Format("RECV %p %p %s RESET\n", socket, connection, gt2AddressToString(ip, port, NULL));
	else
		str.Format("RECV %p %p %s %d\n", socket, connection, gt2AddressToString(ip, port, NULL), len);
	OutputDebugString(str);

	GSI_UNUSED(message);
}

void CGt2testDlg::OnCreateSocket() 
{
	UpdateData();

	// Create the socket, using the info provided.
	GT2Result result = gt2CreateSocket(&m_socket, m_localAddress, atoi(m_outBufferSize), atoi(m_inBufferSize), SocketErrorCallback);
	if(result != GT2Success)
	{
		CString str;
		str.Format("gt2CreateSocket failed: %s", ResultToString(result));
		MessageBox(str);
		return;
	}

	// Set the dump callbacks.
	gt2SetSendDump(m_socket, SendDumpCallback);
	gt2SetReceiveDump(m_socket, ReceiveDumpCallback);

	// Set the config settings to their actual values.
	m_localAddress = gt2AddressToString(gt2GetLocalIP(m_socket), gt2GetLocalPort(m_socket), NULL);
	m_outBufferSize.Format("%d", atoi(m_outBufferSize));
	m_inBufferSize.Format("%d", atoi(m_inBufferSize));

	// Enable the socket controls.
	EnableSocketControls();

	UpdateData(FALSE);
}

void CGt2testDlg::OnCloseSocket() 
{
	// Close the socket.
	gt2CloseSocket(m_socket);

	// Disable the socket controls.
	EnableSocketControls(FALSE);
}

void CGt2testDlg::OnThink() 
{
	// Let the socket think, if it's not already thinking.
	if(m_thinking)
		return;
	m_thinking = GT2True;
	gt2Think(m_socket);
	m_thinking = GT2False;
}

static void ConnectAttemptCallback
(
	GT2Socket socket,
	GT2Connection connection,
	unsigned int ip,
	unsigned short port,
	int latency,
	GT2Byte * message,
	int len
)
{
	GT2Bool accept;

	dlg->UpdateData();
	
	// Check for an empty message.
	if(!message)
		message = (GT2Byte *)"";

	// Get a string address for the connector.
	CString address = gt2AddressToString(ip, port, NULL);

	// Check if we should accept, reject, or prompt.
	if(dlg->m_acceptMode == 0)
		accept = GT2True;
	else if(dlg->m_acceptMode == 1)
		accept = GT2False;
	else
	{
		// Ask the user, and set accept/reject based on their answer.
		CString str;
		str.Format("Accept a connection from %s (latency: %dms)?\n%s", (LPCSTR)address, latency, message);
		accept = (dlg->MessageBox(str, "Accept/Reject", MB_YESNO) == IDYES);
	}

	// Accept the connection?
	if(accept)
	{
		// Setup the connection callbacks.
		GT2ConnectionCallbacks callbacks;
		dlg->SetupConnectionCallbacks(callbacks);

		// Do the accept
		if(gt2Accept(connection, &callbacks))
		{
			// Add the connection to our list.
			dlg->AddConnection(connection);
		}
		else
		{
			// The other side has terminated the connection attempt before we 
			// accepted. Only warn about this if they specifically accepted it.
			if(dlg->m_acceptMode == 2)
				dlg->MessageBox("Connection already closed");
		}
	}
	else
	{
		// Reject them.
		gt2Reject(connection, (GT2Byte *)(LPCSTR)dlg->m_rejectReason, -1);
	}

	dlg->UpdateData(FALSE);

	GSI_UNUSED(len);
	GSI_UNUSED(socket);
}

void CGt2testDlg::OnListen() 
{
	UpdateData();

	// Check if we're supposed to start or stop listening.
	if(m_listen)
	{
		// Start listening.
		gt2Listen(m_socket, ConnectAttemptCallback);

		// Enable the listening controls.
		EnableListenControls();
	}
	else
	{
		// Stop listening.
		gt2Listen(m_socket, NULL);

		// Disable the listening controls.
		EnableListenControls(FALSE);
	}
}

void CGt2testDlg::OnConnect() 
{
	UpdateData();

	// Check the timeout, and show what value we're actually using.
	int timeout = atoi(m_timeout);
	if(timeout < 0)
		timeout = 0;
	m_timeout.Format("%d", timeout);

	// Setup the connection callbacks.
	GT2ConnectionCallbacks callbacks;
	dlg->SetupConnectionCallbacks(callbacks);

	// Start the connection attempt, using the provided values.
	GT2Connection connection;
	GT2Result result = gt2Connect(m_socket, &connection, m_remoteAddress, (GT2Byte *)(LPCSTR)m_connectMessage, -1, timeout, &callbacks, m_blocking);
	if(result != GT2Success)
	{
		CString str;
		str.Format("gt2Connect failed: %s", ResultToString(result));
		MessageBox(str);
		return;
	}

	// Add the connection to our list.
	AddConnection(connection);

	UpdateData(FALSE);
}

void CGt2testDlg::OnCloseAllConnections() 
{
	// Close all the connections.
	gt2CloseAllConnections(m_socket);
}

void CGt2testDlg::OnCloseAllConnectionsHard() 
{
	// Close all the connections without waiting for a response.
	gt2CloseAllConnectionsHard(m_socket);
}

/***************
** CONNECTION **
***************/

void ConnectedCallback
(
	GT2Connection connection,
	GT2Result result,
	GT2Byte * message,
	int len
)
{
	// Check if the attempt failed.
	if(result != GT2Success)
	{
		// Let the user know.
		CString str;
		str.Format("Failed to connect to server: %s", ResultToString(result));
		if(message && len)
		{
			str += "\n";
			str += (char *)message;
		}
		dlg->MessageBox(str);

		// Remove the connection from our list.
		dlg->RemoveConnection(connection);
	}
}

void ReceivedCallback
(
	GT2Connection connection,
	GT2Byte * message,
	int len,
	GT2Bool reliable
)
{
	// Empty messages are NULL.
	if(!message)
		message = (GT2Byte *)"";

	// Add the message to the messages list.
	CString str;
	str.Format("IN(%c): %s\xD\xA", reliable?'r':'u', (char *)message);
	dlg->AddMessageString(connection, str);

	GSI_UNUSED(len);
}

void ClosedCallback
(
	GT2Connection connection,
	GT2CloseReason reason
)
{
	// Show a message if it's an error.
	if(reason == GT2CommunicationError)
		dlg->MessageBox("Connection closed: Communication Error");
	else if(reason == GT2SocketError)
		dlg->MessageBox("Connection closed: Socket Error");
	else if(reason == GT2NotEnoughMemory)
		dlg->MessageBox("Connection closed: Not Enough Memory");

	// The connection is closed, so remove it from the list.
	dlg->RemoveConnection(connection);
}

void PingCallback
(
	GT2Connection connection,
	int latency
)
{
	// Add the ping to the messages list.
	CString str;
	str.Format("PING: %d\xD\xA", latency);
	dlg->AddMessageString(connection, str);
}

void CGt2testDlg::SetupConnectionCallbacks(GT2ConnectionCallbacks &callbacks)
{
	memset(&callbacks, 0, sizeof(GT2ConnectionCallbacks));
	callbacks.connected = ConnectedCallback;
	callbacks.received = ReceivedCallback;
	callbacks.closed = ClosedCallback;
	callbacks.ping = PingCallback;
}

void CGt2testDlg::OnSend() 
{
	GT2Connection connection = GetActiveConnection();

	UpdateData();

	// Print this out before the send because filtering could change it.
	CString str;
	str.Format("OUT(%c): %s\xD\xA", m_reliable?'r':'u', (LPCSTR)m_message);
	AddMessageString(connection, str);

	// Do the send.
	gt2Send(connection, (const GT2Byte *)(LPCSTR)m_message, -1, m_reliable);
}

void CGt2testDlg::OnPing() 
{
	// Send the ping.
	gt2Ping(GetActiveConnection());
}

void CGt2testDlg::OnCloseConnection() 
{
	// Start closing the connection.
	gt2CloseConnection(GetActiveConnection());
}

void CGt2testDlg::OnCloseConnectionHard() 
{
	// Close the ocnnection without waiting for a response.
	gt2CloseConnectionHard(GetActiveConnection());
}

void CGt2testDlg::OnSelchangeConnections() 
{
	// Make sure the newly selected connection is activated.
	UpdateData();
	SetActiveConnection(m_connections.GetCurSel());
	UpdateData(FALSE);
}

// Does a ROT13 transformation on a string.
// ROT13 changes every letter to its "opposite" in the alphabet by adding 13
// to it (a->m, b->n, c->o, d->p, m->a, n->b, o->c, p->d, etc.) performing 
// ROT13 twice results in the original string, allowing the same filter to be 
// applied to both sending and receiving to end up with the original string. 
// Easy encryption.
static void ROT13(char * buffer, int len)
{
	int i;
	int c;
	int c2;

	for(i = 0 ; i < len ; i++)
	{
		c = buffer[i];

		// Only change letters.
		if(isalpha(c))
		{
			// Shift it by 13.
			c2 = c + 13;

			// If it went past the end of the alphabet, mod it.
			if((c2 - (isupper(c)?'A':'a')) >= 26)
				c2 -= 26;

			// Put it back in the string.
			buffer[i] = (char)c2;
		}
	}
}

static void SendFilterCallbackROT13(GT2Connection connection, int filterID, const GT2Byte * message, int len, GT2Bool reliable)
{
	char * msg;

	// We can override the const because we know we're the only filter,
	// it's our data we're sending, and we're not changing the length.
	msg = (char *)message;

	// ROT13 the string.
	ROT13(msg, len);

	// Pass it back to GT2.
	gt2FilteredSend(connection, filterID, (GT2Byte *)msg, len, reliable);
}

static void ReceiveFilterCallbackROT13(GT2Connection connection, int filterID, GT2Byte * message, int len, GT2Bool reliable)
{
	// ROT13 the string.
	ROT13((char *)message, len);

	// Pass it back to GT2.
	gt2FilteredReceive(connection, filterID, message, len, reliable);
}

// Returns true if a message should be dropped, based on the drop value.
static BOOL CheckDrop(int dropValue)
{
	return ((rand() % 100) < dropValue);
}

static void SendFilterCallbackDrop(GT2Connection connection, int filterID, const GT2Byte * message, int len, GT2Bool reliable)
{
	// If it's unreliable, check if it should be dropped.
	if(!reliable && CheckDrop(atoi(dlg->m_sendDropValue)))
		return;

	// Pass it back to GT2.
	gt2FilteredSend(connection, filterID, message, len, reliable);
}

static void ReceiveFilterCallbackDrop(GT2Connection connection, int filterID, GT2Byte * message, int len, GT2Bool reliable)
{
	// If it's unreliable, check if it should be dropped.
	if(!reliable && CheckDrop(atoi(dlg->m_receiveDropValue)))
		return;

	// Pass it back to GT2
	gt2FilteredReceive(connection, filterID, message, len, reliable);
}

static void AddDelayedMessage(DArray array, int delayValue, GT2Connection connection, int filterID, const GT2Byte * message, int len, GT2Bool reliable)
{
	// Don't allow negative delays!
	if(delayValue < 0)
		delayValue = 0;

	// Save off everything we need to remember about the message.
	DelayedMessage msg;
	memset(&msg, 0, sizeof(msg));
	msg.connection = connection;
	msg.filterID = filterID;
	msg.message = (GT2Byte *)malloc(len);
	ASSERT(msg.message);
	memcpy(msg.message, message, len);
	msg.len = len;
	msg.reliable = reliable;
	msg.startTime = current_time();
	msg.delayTime = delayValue;

	// Add it to the delay list.
	ArrayInsertAt(array, &msg, 0);
}

static void SendFilterCallbackDelay(GT2Connection connection, int filterID, const GT2Byte * message, int len, GT2Bool reliable)
{
	// Add it to the delayed sends list.
	AddDelayedMessage(dlg->m_delayedSends, atoi(dlg->m_sendDelayValue), connection, filterID, message, len, reliable);
}

static void ReceiveFilterCallbackDelay(GT2Connection connection, int filterID, GT2Byte * message, int len, GT2Bool reliable)
{
	// Add it to the delayed receives list.
	AddDelayedMessage(dlg->m_delayedReceives, atoi(dlg->m_receiveDelayValue), connection, filterID, message, len, reliable);
}

void CGt2testDlg::OnSendRot13() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_sendROT13)
		gt2AddSendFilter(connection, SendFilterCallbackROT13);
	else
		gt2RemoveSendFilter(connection, SendFilterCallbackROT13);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->sendROT13 = m_sendROT13;
}

void CGt2testDlg::OnReceiveRot13() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_receiveROT13)
		gt2AddReceiveFilter(connection, ReceiveFilterCallbackROT13);
	else
		gt2RemoveReceiveFilter(connection, ReceiveFilterCallbackROT13);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->receiveROT13 = m_receiveROT13;
}

void CGt2testDlg::OnSendDrop() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_sendDrop)
		gt2AddSendFilter(connection, SendFilterCallbackDrop);
	else
		gt2RemoveSendFilter(connection, SendFilterCallbackDrop);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->sendDrop = m_sendDrop;
}

void CGt2testDlg::OnReceiveDrop() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_receiveDrop)
		gt2AddReceiveFilter(connection, ReceiveFilterCallbackDrop);
	else
		gt2RemoveReceiveFilter(connection, ReceiveFilterCallbackDrop);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->receiveDrop = m_receiveDrop;
}

void CGt2testDlg::OnSendDelay() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_sendDelay)
		gt2AddSendFilter(connection, SendFilterCallbackDelay);
	else
		gt2RemoveSendFilter(connection, SendFilterCallbackDelay);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->sendDelay = m_sendDelay;
}

void CGt2testDlg::OnReceiveDelay() 
{
	UpdateData();

	GT2Connection connection = GetActiveConnection();

	// Add or remove the filter based on the check box.
	if(m_receiveDelay)
		gt2AddReceiveFilter(connection, ReceiveFilterCallbackDelay);
	else
		gt2RemoveReceiveFilter(connection, ReceiveFilterCallbackDelay);

	// Save the setting with the connection's info.
	GetConnectionInfo(connection)->receiveDelay = m_receiveDelay;
}

void CGt2testDlg::OnChangeSendDropValue() 
{
	UpdateData();

	// Save the setting with the connection's info.
	GetConnectionInfo(GetActiveConnection())->sendDropValue = m_sendDropValue;
}

void CGt2testDlg::OnChangeReceiveDropValue() 
{
	UpdateData();

	// Save the setting with the connection's info.
	GetConnectionInfo(GetActiveConnection())->receiveDropValue = m_receiveDropValue;
}

void CGt2testDlg::OnChangeSendDelayValue() 
{
	UpdateData();

	// Save the setting with the connection's info.
	GetConnectionInfo(GetActiveConnection())->sendDelayValue = m_sendDelayValue;
}

void CGt2testDlg::OnChangeReceiveDelayValue() 
{
	UpdateData();

	// Save the setting with the connection's info.
	GetConnectionInfo(GetActiveConnection())->receiveDelayValue = m_receiveDelayValue;
}

/************
** ADDRESS **
************/

void CGt2testDlg::OnAddressTo() 
{
	UpdateData();

	// Get the IP and port.
	DWORD IP;
	unsigned int port;
	if(m_addressIP.GetAddress(IP) != 4)
	{
		MessageBox("No IP address");
		return;
	}
	IP = gt2HostToNetworkInt(IP);
	port = atoi(m_addressPort);

	// Convert the ip and port to a string.
	m_addressString = gt2AddressToString(IP, (unsigned short)port, NULL);

	UpdateData(FALSE);
}

void CGt2testDlg::OnAddressFrom() 
{
	UpdateData();

	// Convert the string to an ip and port.
	unsigned int IP;
	unsigned short port;
	if(!gt2StringToAddress(m_addressString, &IP, &port))
	{
		MessageBox("Unable to convert the string to an address");
		return;
	}

	// Set the ip and port.
	m_addressIP.SetAddress((DWORD)gt2NetworkToHostInt(IP));
	m_addressPort.Format("%d", port);

	UpdateData(FALSE);
}

// Updates dialog based on returned host info.
void CGt2testDlg::HandleHostInfo(const char * hostname, char ** aliases, unsigned int ** ips)
{
	int count;

	// Set the hostname.
	m_addressHostname = hostname;

	// Fill in the aliases.
	m_addressAliases.Empty();
	for(count = 0 ; aliases[count] ; count++)
	{
		m_addressAliases += aliases[count];
		m_addressAliases += "   ";
	}

	// Fill in the ips.
	m_addressIPs.Empty();
	for(count = 0 ; ips[count] ; count++)
	{
		m_addressIPs += gt2AddressToString(*(ips[count]), 0, NULL);
		m_addressIPs += "   ";
	}
}

void CGt2testDlg::OnGetIpHostInfo() 
{
	UpdateData();

	// Get the ip.
	DWORD IP;
	if(m_addressIP.GetAddress(IP) != 4)
	{
		MessageBox("No IP address");
		return;
	}
	IP = gt2HostToNetworkInt(IP);

	// Get the info for the ip.
	const char * hostname;
	char ** aliases;
	unsigned int ** ips;
	hostname = gt2IPToHostInfo(IP, &aliases, &ips);
	if(!hostname)
	{
		MessageBox("Unable to convert the IP to host info");
		return;
	}

	// Handle the info.
	HandleHostInfo(hostname, aliases, ips);

	UpdateData(FALSE);
}

void CGt2testDlg::OnGetStringHostInfo() 
{
	UpdateData();

	// Get the info for the string.
	const char * hostname;
	char ** aliases;
	unsigned int ** ips;
	hostname = gt2StringToHostInfo(m_addressString, &aliases, &ips);
	if(!hostname)
	{
		CString str = "Unable to convert the string to host info";
		if(strchr(m_addressString, ':'))
			str += "\n(Remove the port number)";
		MessageBox(str);
		return;
	}

	// Handle the info.
	HandleHostInfo(hostname, aliases, ips);

	UpdateData(FALSE);
}
