
// BorderGenDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <vector>
#include <time.h>
#include <string>
#include "afxcmn.h"
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

#define BORDER_FOLDER "border_result"

struct struProgress {
	string strFile;   //��ǰ������ļ�;
	int    nProgess;  //����;
	struProgress() {
		strFile = "";
		nProgess = 0;
	}
};

// CBorderGenDlg �Ի���
class CBorderGenDlg : public CDialogEx
{
// ����
public:
	CBorderGenDlg(CWnd* pParent = NULL);	// ��׼���캯��

	CString              m_strFilePath;  //�ļ���·��;
	std::vector<string>  m_vecFiles;     //���ڶ��̴߳���,���������Ϳ���;
	std::vector<string>  m_vecOriFiles;  //������ԭʼ�ļ�����Ϣ;
	CCriticalSection     m_csFiles;  //����vecFiles�Ĺ������;
	struProgress       * m_pStruProg; //�洢ÿ���̴߳�����ļ���������Ϣ;��1��ʼ������0
	bool                 m_bSimpleVersion;   //���ַ���
	int                  m_nColorMode;       //ѡ�õ���ɫ�ռ�;
	bool                 m_bWhiteBG;         //����ͼ�ı�����ɫ;
	bool                 m_bThickBorder;     //�ֱ߽�;
	int					 m_nMinArea;
	bool                 m_bGenColorMap;
	time_t               m_tStart, m_tEnd;   //��¼����ʱ��
	bool                 m_bFinish;          //�����Ƿ����н���;
	string  GetSaveFile(string strExt, int nColorMode, string &strFile);

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BORDERGEN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
private:

	CWinThread   * m_pDealThread;
	void ListAllFiles(CString strFilePath);
	bool IsBorderFile(CString strFileName);
	bool GenImgWithBorder(Mat img, Mat border, Mat & res);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBrowser();
	CEdit m_editFilePath;
	CListBox m_lbFiles;
	CStatic m_sttTotal;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedButtonBatchprocess();
	CRichEditCtrl m_reLog;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CProgressCtrl m_ProgressBar;
	CStatic m_sttProgress;
	CButton m_btnStart;

	// �������߳���
	short m_nThreadNum;
	afx_msg void OnBnClickedRadioCompleximg();
	afx_msg void OnBnClickedRadioSimpleimg();
	afx_msg void OnBnClickedRadioBlackbg();
	afx_msg void OnBnClickedRadioWhitebg();
	afx_msg void OnBnClickedCheckGencolormap();
	afx_msg void OnBnClickedRadioThick();
	afx_msg void OnBnClickedRadioSlim();
	// ��ɫ֮��ľ���
	int m_nColorDistThre;
	// �������ɵ���ɫ����
	short m_nFinalColorNum;
	afx_msg void OnBnClickedCheckSpecColornum();
	// ����ģʽ�µ��㷨ʵ��
	bool m_bFastMode;
	afx_msg void OnBnClickedCheckFastmode();
	afx_msg void OnBnClickedRadioColorRgb();
	afx_msg void OnBnClickedRadioColorHsv();
	afx_msg void OnBnClickedRadioColorLab();
};
