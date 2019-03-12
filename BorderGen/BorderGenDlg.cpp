
// BorderGenDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BorderGen.h"
#include "BorderGenDlg.h"
#include "afxdialogex.h"
#include "ImgQuantify.h"

#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG  
	#pragma comment(lib,"opencv_world400d.lib")
#else  
	#pragma comment(lib,"opencv_world400.lib")
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

static int nFileId = 0;
UINT ThreadGenBorder(LPVOID pParam) {
	CBorderGenDlg * pDlg = (CBorderGenDlg *)pParam;

	//处理一个图像文件;
	while (pDlg->m_vecFiles.size())
	{
		string strFile;
		pDlg->m_csFiles.Lock();
		strFile = pDlg->m_vecFiles.back();
		pDlg->m_vecFiles.pop_back();
		nFileId++;
		pDlg->m_csFiles.Unlock();

		//写日志;
		pDlg->m_pStruProg[nFileId].strFile = strFile;

		//border文件命名;
		int nIndex = strFile.rfind('.');
		string strBorder = strFile.substr(0, nIndex);
		strBorder = strBorder.append("_border.jpg");

		//生成边界文件;
		int * p = &(pDlg->m_pStruProg[nFileId].nProgess);
		Mat img = imread(strFile);
		CImgQuantify iq(img);
		iq.MainProc(strBorder, p);
	}

	return 1;
}

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBorderGenDlg 对话框



CBorderGenDlg::CBorderGenDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BORDERGEN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBorderGenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FOLDERPATH, m_editFilePath);
	DDX_Control(pDX, IDC_LIST1, m_lbFiles);
	DDX_Control(pDX, IDC_STATIC_TOTALFILES, m_sttTotal);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_reLog);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_sttProgress);
	DDX_Control(pDX, IDC_BUTTON_BATCHPROCESS, m_btnStart);
}

BEGIN_MESSAGE_MAP(CBorderGenDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSER, &CBorderGenDlg::OnBnClickedButtonBrowser)
	ON_LBN_SELCHANGE(IDC_LIST1, &CBorderGenDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON_BATCHPROCESS, &CBorderGenDlg::OnBnClickedButtonBatchprocess)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CBorderGenDlg 消息处理程序

BOOL CBorderGenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//
	SetTimer(1000, 1000, NULL);

	m_pStruProg = NULL;

	m_ProgressBar.SetRange(1, 100);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CBorderGenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBorderGenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBorderGenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CBorderGenDlg::OnBnClickedButtonBrowser()
{
	// TODO: 在此添加控件通知处理程序代码

	BROWSEINFO bi;
	char Buffer[MAX_PATH];

	//初始化入口参数bi开始
	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;//初始化制定的root目录很不容易，
	bi.pszDisplayName = Buffer;//此参数如为NULL则不能显示对话框
	bi.lpszTitle = "选择测试文件夹";
	bi.lpfn = NULL;
	bi.ulFlags = BIF_EDITBOX;//包括文件

	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//调用显示选择对话框
	if (pIDList)
	{
		SHGetPathFromIDList(pIDList, Buffer);
		//取得文件夹路径到Buffer里
		m_strFilePath.Format("%s", Buffer);
		m_editFilePath.SetWindowText(m_strFilePath);
	}

	ListAllFiles(m_strFilePath);

	int nSize = m_lbFiles.GetCount();
	CString strTile;
	strTile.Format("一共有%d个文件", nSize);
	m_sttTotal.SetWindowText(strTile);

	//保存的文件及进度信息;
	if (m_pStruProg)
		delete[] m_pStruProg;
	else
		m_pStruProg = new struProgress[nSize+1];  //下标0不用;

	//将所有图像文件保存在vector中;
	m_vecFiles.clear();
	for (int i = 0; i < nSize; i++)
	{
		CString strFileName;
		m_lbFiles.GetText(i, strFileName);
		strFileName = m_strFilePath + "\\" + strFileName;
		string s(strFileName.GetBuffer(0));
		m_vecFiles.push_back(s);
	}

}


void CBorderGenDlg::ListAllFiles(CString strFilePath)
{
	m_lbFiles.ResetContent();

	//获得当前文件夹下的所有图像;
	CFileFind findFile;
	CString   strDstFiles;

	strDstFiles = strFilePath + "\\*.jpg";
	BOOL bWorking = findFile.FindFile(strDstFiles);

	while (bWorking)
	{
		bWorking = findFile.FindNextFile();
		CString strFileName = findFile.GetFileName();

		m_lbFiles.AddString(strFileName);
	}
	findFile.Close();

	strDstFiles = strFilePath + "\\*.jpeg";
	bWorking = findFile.FindFile(strDstFiles);
	while (bWorking)
	{
		bWorking = findFile.FindNextFile();
		CString strFileName = findFile.GetFileName();

		m_lbFiles.AddString(strFileName);
	}
	findFile.Close();

	strDstFiles = strFilePath + "\\*.png";
	bWorking = findFile.FindFile(strDstFiles);
	while (bWorking)
	{
		bWorking = findFile.FindNextFile();
		CString strFileName = findFile.GetFileName();

		m_lbFiles.AddString(strFileName);
	}
	findFile.Close();
}


void CBorderGenDlg::OnLbnSelchangeList1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCurIndex = m_lbFiles.GetCurSel();
	if (-1 == nCurIndex)
	{
		return;
	}

	CString strFileName;
	m_lbFiles.GetText(nCurIndex, strFileName);
	strFileName = m_strFilePath + "\\" + strFileName;

	Mat img = imread(strFileName.GetBuffer(0));

	//缩放到宽度只有800;
	int nW = 480;
	int nH = img.cols*1.0 / nW / img.rows;
	namedWindow("原图");
	resizeWindow("原图", nW, nH);
	imshow("原图", img);
}


void CBorderGenDlg::OnBnClickedButtonBatchprocess()
{
	nFileId = 0;  

	// TODO: 在此添加控件通知处理程序代码
	for (int i = 0; i < THREAD_NUM; i++)
		AfxBeginThread(ThreadGenBorder, (void *)this, THREAD_PRIORITY_HIGHEST);

	m_btnStart.EnableWindow(FALSE);
}


void CBorderGenDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	CDialogEx::OnTimer(nIDEvent);

	if (!m_pStruProg)
		return;

	CString strTotalText = "**************开始处理**************\n";
	int nFinished = 0;
	for (int i = 1; i <= m_lbFiles.GetCount(); i++){
		string strFile   = m_pStruProg[i].strFile;
		int    nProgress = m_pStruProg[i].nProgess;

		if (strFile.empty())
			continue;

		CString strText;
		if (nProgress <= 98)
			strText.Format("开始处理文件:%s，进度为:%d%%\n", strFile.c_str(), m_pStruProg[i].nProgess);
		else
		{
			strText.Format("文件%s已经处理完毕\n", strFile.c_str());
			nFinished++;
		}
		strTotalText += strText;
	}

	m_reLog.SetWindowText(strTotalText);

	//设置进度条;
	if (nFinished>0){
		int nPos = nFinished * 100 / m_lbFiles.GetCount();
		m_ProgressBar.SetPos(nPos);

		CString strText;
		strText.Format("%d%%\n", nPos);
		m_sttProgress.SetWindowText(strText);

		//几乎快处理完了;
		if (nPos>=99)
			m_btnStart.EnableWindow(TRUE);
	}
}


void CBorderGenDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	if (m_pStruProg)
		delete[] m_pStruProg;

	m_vecFiles.clear();
}
