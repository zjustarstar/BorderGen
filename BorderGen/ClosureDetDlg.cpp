// ClosureDetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BorderGen.h"
#include "ClosureDetDlg.h"
#include "afxdialogex.h"
#include <io.h>
#include "ClosureDetector.h"

#define RESULT_FOLDER "ClosureDet_Result"

using namespace std;

// CClosureDetDlg 对话框

IMPLEMENT_DYNAMIC(CClosureDetDlg, CDialogEx)

CClosureDetDlg::CClosureDetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CLOSURE_DETECTOR, pParent)
{

}

CClosureDetDlg::~CClosureDetDlg()
{
}

void CClosureDetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILEPATH, m_editFilePath);
	DDX_Control(pDX, IDC_LIST_FILES, m_lbFiles);
}


BEGIN_MESSAGE_MAP(CClosureDetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_BROWSER, &CClosureDetDlg::OnBnClickedButtonBrowser)
	ON_BN_CLICKED(IDC_BUTTON_STARTDETECT, &CClosureDetDlg::OnBnClickedButtonStartdetect)
END_MESSAGE_MAP()


// CClosureDetDlg 消息处理程序

void CClosureDetDlg::ListAllFiles(CString strFilePath)
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

void CClosureDetDlg::OnBnClickedButtonBrowser()
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

}

string CClosureDetDlg::getSaveFileName(CString strFile) {
	//创建新的路径;
	CString strResultFolder = m_strFilePath;
	strResultFolder = strResultFolder + "\\" + RESULT_FOLDER;

	string strFileName = "";
	if (!PathFileExists(strResultFolder))
	{
		//原始文件同文件夹下;
		strFileName.append(m_strFilePath.GetBuffer(0));
	}
	else {
		//放在新建的border文件夹下;
		strFileName.append(strResultFolder.GetBuffer(0));
	}

	string strTemp;
	strTemp = strFile.GetBuffer(0);
	strFileName = strFileName + "\\" + strTemp;

	return strFileName;
}

void CClosureDetDlg::OnBnClickedButtonStartdetect()
{
	CString strBorderFolder = m_strFilePath;
	strBorderFolder = strBorderFolder + "\\" + RESULT_FOLDER;
	if (!PathFileExists(strBorderFolder))
		CreateDirectory(strBorderFolder, NULL);

	// TODO: 在此添加控件通知处理程序代码
	int nSize = m_lbFiles.GetCount();
	CClosureDetector ccd;
	for (int i = 0; i < nSize; i++)
	{
		CString strFile;
		m_lbFiles.GetText(i, strFile);
		string strFileName = getSaveFileName(strFile);  //保存的图片(带路径);

		//开始检测;
		strFile = m_strFilePath + "\\" + strFile;
		Mat img = imread(strFile.GetBuffer(0));

		//如果有未封闭的区域，则保存;
		Mat resImg;
		if (ccd.GetThinImg(img, resImg))
		{
			imwrite(strFileName, resImg);
		}
	}

	AfxMessageBox("处理完成!");
	
}


BOOL CClosureDetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CButton * pBtnBlackBG = (CButton *)GetDlgItem(IDC_RADIO_BLACKBG);
	CButton * pBtnWhiteBG = (CButton *)GetDlgItem(IDC_RADIO_WHITEBG);
	pBtnBlackBG->SetCheck(0);
	pBtnWhiteBG->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
