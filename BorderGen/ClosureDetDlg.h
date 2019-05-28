#pragma once
#include <opencv2\opencv.hpp>
using namespace std;
using namespace cv;

// CClosureDetDlg �Ի���

class CClosureDetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClosureDetDlg)

public:
	CClosureDetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CClosureDetDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CLOSURE_DETECTOR };
#endif

protected:
	CString   m_strFilePath;  //�ļ���·��;
	CEdit     m_editFilePath; 
	CListBox  m_lbFiles;

	string    getSaveFileName(CString strFile);
	void      ListAllFiles(CString strFilePath);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedButtonBrowser();
	afx_msg void OnBnClickedButtonStartdetect();
	virtual BOOL OnInitDialog();
};
