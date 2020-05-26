
// BorderGenDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BorderGen.h"
#include "BorderGenDlg.h"
#include "afxdialogex.h"
#include "ImgQuantify.h"
#include "PaintImgDealer.h"
#include <io.h>


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
	int nMinRegNum = pDlg->m_nMinArea;

	//����һ��ͼ���ļ�;
	while (pDlg->m_vecFiles.size())
	{
		string strFile;
		pDlg->m_csFiles.Lock();
		strFile = pDlg->m_vecFiles.back();
		pDlg->m_vecFiles.pop_back();
		nFileId++;
		pDlg->m_csFiles.Unlock();

		//Ҫ�����ͼƬ��·��;
		string strBorder = pDlg->GetSaveFile("border",strFile);
		string strColor  = pDlg->GetSaveFile("color", strFile);
		//д��־;
		string s(pDlg->m_strFilePath.GetBuffer(0));
		pDlg->m_pStruProg[nFileId].strFile = strFile;
		strFile = s + "\\" + strFile;
		
		//���ɱ߽��ļ�;
		int * p = &(pDlg->m_pStruProg[nFileId].nProgess);
		Mat img = imread(strFile);

		//��ͼ�Ĵ���;
		if (pDlg->m_bSimpleVersion) {
			CImgQuantify iq(img);
			iq.setMinRegNum(nMinRegNum);
			iq.MainProc(strBorder, p);
		}
		//����ͼ�Ĵ���;
		else
		{
			CPaintImgDealer pid(img);
			struInParam ip;
			ip.strBorderFile = strBorder;
			//���Ҫ������ɫͼ;
			if (pDlg->m_bGenColorMap)
				ip.strColorFile = strColor;

			//���ò���;
			ip.nProgress = p;
			ip.nColorThre = pDlg->m_nColorDistThre;
			ip.bWhiteBG = pDlg->m_bWhiteBG;
			ip.bThickBd = pDlg->m_bThickBorder;
			ip.nMinAreaThre = pDlg->m_nMinArea;

			pid.MainProc(ip);
		}

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
	, m_nMinArea(100)
	, m_nThreadNum(4)
	, m_bGenColorMap(0)
	, m_nColorDistThre(20)
	, m_nFinalColorNum(0)
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
	DDX_Text(pDX, IDC_EDIT_PARAM_THREADNUM, m_nThreadNum);
	DDX_Text(pDX, IDC_EDIT_PARAM_REGIONMINSIZE, m_nMinArea);
	DDX_Text(pDX, IDC_EDIT_PARAM_COLORDIST, m_nColorDistThre);
	//DDX_Text(pDX, IDC_CHECK_GENCOLORMAP, m_bGenColorMap);
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
	ON_BN_CLICKED(IDC_RADIO_COMPLEXIMG, &CBorderGenDlg::OnBnClickedRadioCompleximg)
	ON_BN_CLICKED(IDC_RADIO_SIMPLEIMG, &CBorderGenDlg::OnBnClickedRadioSimpleimg)
	ON_BN_CLICKED(IDC_RADIO_BLACKBG, &CBorderGenDlg::OnBnClickedRadioBlackbg)
	ON_BN_CLICKED(IDC_RADIO_WHITEBG, &CBorderGenDlg::OnBnClickedRadioWhitebg)
	ON_BN_CLICKED(IDC_CHECK_GENCOLORMAP, &CBorderGenDlg::OnBnClickedCheckGencolormap)
	ON_BN_CLICKED(IDC_RADIO_THICK, &CBorderGenDlg::OnBnClickedRadioThick)
	ON_BN_CLICKED(IDC_RADIO_SLIM, &CBorderGenDlg::OnBnClickedRadioSlim)
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

	m_pStruProg      = NULL;
	m_bSimpleVersion = false;
	m_bWhiteBG       = true;
	m_bThickBorder   = false;
	m_bGenColorMap	 = true;

	CButton * pBtnSVersion = (CButton *)GetDlgItem(IDC_RADIO_SIMPLEIMG);
	CButton * pBtnCVersion = (CButton *)GetDlgItem(IDC_RADIO_COMPLEXIMG);
	CButton * pBtnBlackBG = (CButton *)GetDlgItem(IDC_RADIO_BLACKBG);
	CButton * pBtnWhiteBG = (CButton *)GetDlgItem(IDC_RADIO_WHITEBG);
	CButton * pBtnThickB  = (CButton *)GetDlgItem(IDC_RADIO_THICK);
	CButton * pBtnSlimB   = (CButton *)GetDlgItem(IDC_RADIO_SLIM);
	CButton * pGenColorButton = (CButton *)GetDlgItem(IDC_CHECK_GENCOLORMAP);
	pGenColorButton->SetCheck(1);
	pBtnCVersion->SetCheck(1);
	pBtnSVersion->SetCheck(0);
	pBtnWhiteBG->SetCheck(1);
	pBtnBlackBG->SetCheck(0);
	pBtnSlimB->SetCheck(1);
	pBtnThickB->SetCheck(0);

	m_ProgressBar.SetRange(0, 100);

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

//���ļ�����d:\\xxx.jpg��Ϊd:\\xxx_strExt\\.jpg;

string CBorderGenDlg::GetSaveFile(string strExt,string &strFile) {

	CString strBorderFolder = m_strFilePath;
	strBorderFolder = strBorderFolder + "\\" + BORDER_FOLDER;

	string strFileName="";
	if (!PathFileExists(strBorderFolder))
	{
		//ԭʼ�ļ�ͬ�ļ�����;
		strFileName.append(m_strFilePath.GetBuffer(0));
	}
	else {
		//�����½���border�ļ�����;
		strFileName.append(strBorderFolder.GetBuffer(0));
	}

	strFileName = strFileName + "\\" + strFile;

	//border�ļ�����;
	int nIndex = strFileName.rfind('.');
	string strBorder = strFileName.substr(0, nIndex);
	strBorder = strBorder + "_" + strExt;
	strBorder = strBorder.append(".jpg");

	return strBorder;
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
	{
		delete[] m_pStruProg;
		m_pStruProg = NULL;
	}

	m_pStruProg = new struProgress[nSize+1];  //�±�0����;

	//������ͼ���ļ�������vector��;
	m_vecFiles.clear();
	for (int i = 0; i < nSize; i++)
	{
		CString strFileName;
		m_lbFiles.GetText(i, strFileName);
		string s(strFileName.GetBuffer(0));
		m_vecFiles.push_back(s);
	}
}

//�ǲ��Ǻ�׺��_border���ļ�;
bool CBorderGenDlg::IsBorderFile(CString strFile)
{
	string strFileName;
	strFileName = strFile.GetBuffer(0);
	strFile.ReleaseBuffer();

	int nIndex = strFileName.rfind('.');
	string strBorder = strFileName.substr(0, nIndex);

	string strFormat = "_border";
	if (strBorder.length() <= strFormat.length())
		return false;

	string strExtractFormat = strBorder.substr(strBorder.length() - strFormat.length(), strFormat.length());
	if (strExtractFormat.compare(strFormat.c_str()) == 0)
		return true;
	else
		return false;
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

		//�����border�ļ�;
		if (!IsBorderFile(strFileName))
			m_lbFiles.AddString(strFileName);
	}
	findFile.Close();

	strDstFiles = strFilePath + "\\*.jpeg";
	bWorking = findFile.FindFile(strDstFiles);
	while (bWorking)
	{
		bWorking = findFile.FindNextFile();
		CString strFileName = findFile.GetFileName();

		if (!IsBorderFile(strFileName))
			m_lbFiles.AddString(strFileName);
	}
	findFile.Close();

	strDstFiles = strFilePath + "\\*.png";
	bWorking = findFile.FindFile(strDstFiles);
	while (bWorking)
	{
		bWorking = findFile.FindNextFile();
		CString strFileName = findFile.GetFileName();

		if (!IsBorderFile(strFileName))
			m_lbFiles.AddString(strFileName);
	}
	findFile.Close();
}

bool CBorderGenDlg::GenImgWithBorder(Mat img,Mat border,Mat & res) {

	res = img.clone();
	int w = img.cols;
	int h = img.rows;

	int bw = border.cols;
	int bh = border.rows;
	if ((bw != w) || (bh != h))
		return false;
	for (int r = 0; r < h; r++)
	{
		for (int c = 0; c < w; c++)
		{
			Vec3b v = border.at<Vec3b>(r, c);
			//����;
			if (v[0]<125 && v[1]<125 && v[2]<125)
				res.at<Vec3b>(r, c) = Vec3b(0, 0, 255);
		}
	}
	
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

	//���ŵ����ֻ��ԭ����һ���С;
	Mat resizedImg;
	resize(img, resizedImg, Size(img.cols/2, img.rows/2));
	namedWindow("ԭͼ");
	imshow("ԭͼ", resizedImg);

	//borderͼ;
	string strTemp;
	strTemp = strFileName.GetBuffer(0);
	int nIndex = strTemp.rfind('.');
	string strExt = strTemp.substr(nIndex, strTemp.length() - nIndex);
	string strBorderFile = strTemp.substr(0, nIndex);
	strBorderFile = strBorderFile.append("_border");
	strBorderFile = strBorderFile.append(strExt);

	//�����borderͼ����ʾ����ͼ;
	Mat borderImg = imread(strBorderFile);
	if (borderImg.empty()) {
		AfxMessageBox("δ�ҵ���Ӧ��_border�ļ�");
		return;
	}

	Mat resImg,resizeRes;
	GenImgWithBorder(img, borderImg, resImg);
	resize(resImg, resizeRes, Size(img.cols/2, img.rows/2));
	namedWindow("����ͼ");
	imshow("����ͼ", resizeRes);
}


void CBorderGenDlg::OnBnClickedButtonBatchprocess()
{
	UpdateData();

	int aa = m_nMinArea;
	int bb = m_nColorDistThre;
	nFileId = 0;  

	CString strBorderFolder = m_strFilePath;
	strBorderFolder = strBorderFolder + "\\" + BORDER_FOLDER;
	if (!PathFileExists(strBorderFolder))
		CreateDirectory(strBorderFolder,NULL);

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

	CString strTotalText = "**************���[��ʼ����]��ť�����д���**************\n";
	int nFinished = 0;
	for (int i = 1; i <= m_lbFiles.GetCount(); i++){
		string strFile   = m_pStruProg[i].strFile;
		int    nProgress = m_pStruProg[i].nProgess;

		if (strFile.empty())
			continue;

		CString strText;
		if (nProgress == 0)
			strText.Format("��ʼԤ�����ļ�:%s.....\n", strFile.c_str());
		else if ((nProgress > 0) && (nProgress < 98))
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
		{
			m_btnStart.EnableWindow(TRUE);
			m_sttProgress.SetWindowText("0%");
			m_ProgressBar.SetPos(0);
		}
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


void CBorderGenDlg::OnBnClickedRadioCompleximg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bSimpleVersion = false;
}


void CBorderGenDlg::OnBnClickedRadioSimpleimg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bSimpleVersion = true;
}


void CBorderGenDlg::OnBnClickedRadioBlackbg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bWhiteBG = false;
}


void CBorderGenDlg::OnBnClickedRadioWhitebg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bWhiteBG = true;
}


void CBorderGenDlg::OnBnClickedCheckGencolormap()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CButton * pButton = (CButton *)GetDlgItem(IDC_CHECK_GENCOLORMAP);
	if (pButton->GetCheck())
		m_bGenColorMap = true;
	else
		m_bGenColorMap = false;
}


void CBorderGenDlg::OnBnClickedRadioThick()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bThickBorder = true;
}


void CBorderGenDlg::OnBnClickedRadioSlim()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bThickBorder = false;
}
