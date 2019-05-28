#pragma once
#include "afxcmn.h"
#include "ClosureDetDlg.h"
#include "BorderGenDlg.h"

// CMainDlg 对话框

class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MAIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void InitTabCtrl();

	CDialog * m_pPage[2];
	CClosureDetDlg m_dlgCd;
	CBorderGenDlg  m_dlgBg;
	CTabCtrl       m_tabSet;
	int            m_iCurTab;
	afx_msg void OnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
