
// BorderGenDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

static int nFileId = 0;
UINT ThreadGenBorder(LPVOID pParam) {
	CBorderGenDlg * pDlg = (CBorderGenDlg *)pParam;

	//����һ��ͼ���ļ�;
	while (pDlg->m_vecFiles.size())
	{
		string strFile;
		pDlg->m_csFiles.Lock();
		strFile = pDlg->m_vecFiles.back();
		pDlg->m_vecFiles.pop_back();
		nFileId++;
		pDlg->m_csFiles.Unlock();

		//д��־;
		pDlg->m_pStruProg[nFileId].strFile = strFile;

		//border�ļ�����;
		int nIndex = strFile.rfind('.');
		string strBorder = strFile.substr(0, nIndex);
		strBorder = strBorder.append("_border.jpg");

		//���ɱ߽��ļ�;
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


// CBorderGenDlg �Ի���



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


// CBorderGenDlg ��Ϣ�������

BOOL CBorderGenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//
	SetTimer(1000, 1000, NULL);

	m_pStruProg = NULL;

	m_ProgressBar.SetRange(1, 100);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CBorderGenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CBorderGenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CBorderGenDlg::OnBnClickedButtonBrowser()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	BROWSEINFO bi;
	char Buffer[MAX_PATH];

	//��ʼ����ڲ���bi��ʼ
	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;//��ʼ���ƶ���rootĿ¼�ܲ����ף�
	bi.pszDisplayName = Buffer;//�˲�����ΪNULL������ʾ�Ի���
	bi.lpszTitle = "ѡ������ļ���";
	bi.lpfn = NULL;
	bi.ulFlags = BIF_EDITBOX;//�����ļ�

	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//������ʾѡ��Ի���
	if (pIDList)
	{
		SHGetPathFromIDList(pIDList, Buffer);
		//ȡ���ļ���·����Buffer��
		m_strFilePath.Format("%s", Buffer);
		m_editFilePath.SetWindowText(m_strFilePath);
	}

	ListAllFiles(m_strFilePath);

	int nSize = m_lbFiles.GetCount();
	CString strTile;
	strTile.Format("һ����%d���ļ�", nSize);
	m_sttTotal.SetWindowText(strTile);

	//������ļ���������Ϣ;
	if (m_pStruProg)
		delete[] m_pStruProg;
	else
		m_pStruProg = new struProgress[nSize+1];  //�±�0����;

	//������ͼ���ļ�������vector��;
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

	//��õ�ǰ�ļ����µ�����ͼ��;
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int nCurIndex = m_lbFiles.GetCurSel();
	if (-1 == nCurIndex)
	{
		return;
	}

	CString strFileName;
	m_lbFiles.GetText(nCurIndex, strFileName);
	strFileName = m_strFilePath + "\\" + strFileName;

	Mat img = imread(strFileName.GetBuffer(0));

	//���ŵ����ֻ��800;
	int nW = 480;
	int nH = img.cols*1.0 / nW / img.rows;
	namedWindow("ԭͼ");
	resizeWindow("ԭͼ", nW, nH);
	imshow("ԭͼ", img);
}


void CBorderGenDlg::OnBnClickedButtonBatchprocess()
{
	nFileId = 0;  

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	for (int i = 0; i < THREAD_NUM; i++)
		AfxBeginThread(ThreadGenBorder, (void *)this, THREAD_PRIORITY_HIGHEST);

	m_btnStart.EnableWindow(FALSE);
}


void CBorderGenDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	CDialogEx::OnTimer(nIDEvent);

	if (!m_pStruProg)
		return;

	CString strTotalText = "**************��ʼ����**************\n";
	int nFinished = 0;
	for (int i = 1; i <= m_lbFiles.GetCount(); i++){
		string strFile   = m_pStruProg[i].strFile;
		int    nProgress = m_pStruProg[i].nProgess;

		if (strFile.empty())
			continue;

		CString strText;
		if (nProgress <= 98)
			strText.Format("��ʼ�����ļ�:%s������Ϊ:%d%%\n", strFile.c_str(), m_pStruProg[i].nProgess);
		else
		{
			strText.Format("�ļ�%s�Ѿ��������\n", strFile.c_str());
			nFinished++;
		}
		strTotalText += strText;
	}

	m_reLog.SetWindowText(strTotalText);

	//���ý�����;
	if (nFinished>0){
		int nPos = nFinished * 100 / m_lbFiles.GetCount();
		m_ProgressBar.SetPos(nPos);

		CString strText;
		strText.Format("%d%%\n", nPos);
		m_sttProgress.SetWindowText(strText);

		//�����촦������;
		if (nPos>=99)
			m_btnStart.EnableWindow(TRUE);
	}
}


void CBorderGenDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	if (m_pStruProg)
		delete[] m_pStruProg;

	m_vecFiles.clear();
}
