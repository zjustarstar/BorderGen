#pragma once
#include <opencv2\opencv.hpp>
using namespace std;
using namespace cv;

// CClosureDetDlg 对话框

class CClosureDetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClosureDetDlg)

public:
	CClosureDetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CClosureDetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CLOSURE_DETECTOR };
#endif

protected:
	CString   m_strFilePath;  //文件夹路径;
	CEdit     m_editFilePath; 
	CListBox  m_lbFiles;

	string    getSaveFileName(CString strFile);
	void      ListAllFiles(CString strFilePath);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedButtonBrowser();
	afx_msg void OnBnClickedButtonStartdetect();
	virtual BOOL OnInitDialog();
};
