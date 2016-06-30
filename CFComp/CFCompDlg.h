
// CFCompDlg.h : header file
//

#pragma once
#include "afxwin.h"

// CCFCompDlg dialog
class CCFCompDlg : public CDialogEx
{
// Construction
public:
	CCFCompDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CFCOMP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	CString m_output;
	CString m_source;
};
