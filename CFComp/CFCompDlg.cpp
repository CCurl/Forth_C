
// CFCompDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CFComp.h"
#include "CFCompDlg.h"
#include "afxdialogex.h"

#include "CFCompiler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CCFCompDlg dialog

CCFCompDlg::CCFCompDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCFCompDlg::IDD, pParent)
	, m_output(_T(".\\image.txt"))
	, m_source(_T(".\\source.txt"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCFCompDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OUTPUT, m_output);
	DDX_Text(pDX, IDC_SOURCE, m_source);
}

BEGIN_MESSAGE_MAP(CCFCompDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCFCompDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCFCompDlg message handlers

BOOL CCFCompDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	OnBnClickedOk();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCFCompDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCFCompDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCFCompDlg::OnBnClickedOk()
{
	CCFCompiler c;
	c.Compile(m_source, m_output);
	CDialog::OnOK();
}
