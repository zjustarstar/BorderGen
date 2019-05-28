// ClosureDetDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BorderGen.h"
#include "ClosureDetDlg.h"
#include "afxdialogex.h"
#include <io.h>
#include "ClosureDetector.h"

#define RESULT_FOLDER "ClosureDet_Result"

using namespace std;

// CClosureDetDlg �Ի���

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


// CClosureDetDlg ��Ϣ�������

void CClosureDetDlg::ListAllFiles(CString strFilePath)
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

void CClosureDetDlg::OnBnClickedButtonBrowser()
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

}

string CClosureDetDlg::getSaveFileName(CString strFile) {
	//�����µ�·��;
	CString strResultFolder = m_strFilePath;
	strResultFolder = strResultFolder + "\\" + RESULT_FOLDER;

	string strFileName = "";
	if (!PathFileExists(strResultFolder))
	{
		//ԭʼ�ļ�ͬ�ļ�����;
		strFileName.append(m_strFilePath.GetBuffer(0));
	}
	else {
		//�����½���border�ļ�����;
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

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int nSize = m_lbFiles.GetCount();
	CClosureDetector ccd;
	for (int i = 0; i < nSize; i++)
	{
		CString strFile;
		m_lbFiles.GetText(i, strFile);
		string strFileName = getSaveFileName(strFile);  //�����ͼƬ(��·��);

		//��ʼ���;
		strFile = m_strFilePath + "\\" + strFile;
		Mat img = imread(strFile.GetBuffer(0));

		//�����δ��յ������򱣴�;
		Mat resImg;
		if (ccd.GetThinImg(img, resImg))
		{
			imwrite(strFileName, resImg);
		}
	}

	AfxMessageBox("�������!");
	
}


BOOL CClosureDetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CButton * pBtnBlackBG = (CButton *)GetDlgItem(IDC_RADIO_BLACKBG);
	CButton * pBtnWhiteBG = (CButton *)GetDlgItem(IDC_RADIO_WHITEBG);
	pBtnBlackBG->SetCheck(0);
	pBtnWhiteBG->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
