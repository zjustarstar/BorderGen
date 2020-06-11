
// BorderGenDlg.h : 头文件
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
	string strFile;   //当前处理的文件;
	int    nProgess;  //进度;
	struProgress() {
		strFile = "";
		nProgess = 0;
	}
};

// CBorderGenDlg 对话框
class CBorderGenDlg : public CDialogEx
{
// 构造
public:
	CBorderGenDlg(CWnd* pParent = NULL);	// 标准构造函数

	CString              m_strFilePath;  //文件夹路径;
	std::vector<string>  m_vecFiles;     //用于多线程处理,处理结束后就空了;
	std::vector<string>  m_vecOriFiles;  //保存了原始文件的信息;
	CCriticalSection     m_csFiles;  //用于vecFiles的共享访问;
	struProgress       * m_pStruProg; //存储每个线程处理的文件及进度信息;从1开始，不是0
	bool                 m_bSimpleVersion;   //哪种方法
	int                  m_nColorMode;       //选用的颜色空间;
	bool                 m_bWhiteBG;         //生成图的背景颜色;
	bool                 m_bThickBorder;     //粗边界;
	int					 m_nMinArea;
	bool                 m_bGenColorMap;
	time_t               m_tStart, m_tEnd;   //记录运行时间
	bool                 m_bFinish;          //程序是否运行结束;
	string  GetSaveFile(string strExt, int nColorMode, string &strFile);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BORDERGEN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private:

	CWinThread   * m_pDealThread;
	void ListAllFiles(CString strFilePath);
	bool IsBorderFile(CString strFileName);
	bool GenImgWithBorder(Mat img, Mat border, Mat & res);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

	// 启动的线程数
	short m_nThreadNum;
	afx_msg void OnBnClickedRadioCompleximg();
	afx_msg void OnBnClickedRadioSimpleimg();
	afx_msg void OnBnClickedRadioBlackbg();
	afx_msg void OnBnClickedRadioWhitebg();
	afx_msg void OnBnClickedCheckGencolormap();
	afx_msg void OnBnClickedRadioThick();
	afx_msg void OnBnClickedRadioSlim();
	// 颜色之间的距离
	int m_nColorDistThre;
	// 最终生成的颜色数量
	short m_nFinalColorNum;
	afx_msg void OnBnClickedCheckSpecColornum();
	// 快速模式下的算法实现
	bool m_bFastMode;
	afx_msg void OnBnClickedCheckFastmode();
	afx_msg void OnBnClickedRadioColorRgb();
	afx_msg void OnBnClickedRadioColorHsv();
	afx_msg void OnBnClickedRadioColorLab();
};
