// MainDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BorderGen.h"
#include "MainDlg.h"

#include "afxdialogex.h"


// CMainDlg �Ի���

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_MAIN, pParent)
{

}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tabSet);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CMainDlg::OnSelchangeTab1)
END_MESSAGE_MAP()


// CMainDlg ��Ϣ�������


BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	InitTabCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CMainDlg::InitTabCtrl()
{
	m_tabSet.InsertItem(0, "�߽�����");
	m_tabSet.InsertItem(1, "�߽��ռ��");

	//��������;
	m_dlgBg.Create(IDD_BORDERGEN_DIALOG, &m_tabSet);
	m_dlgCd.Create(IDD_DIALOG_CLOSURE_DETECTOR, &m_tabSet);

	//�趨��Tab����ʾ�ķ�Χ
	CRect rc;
	m_tabSet.GetClientRect(rc);
	rc.top += 30;
	rc.bottom -= 4;
	rc.left += 8;
	rc.right -= 8;

	m_dlgBg.MoveWindow(&rc);
	m_dlgCd.MoveWindow(&rc);

	//��ʾ����;
	m_dlgBg.ShowWindow(SW_SHOW);
	m_dlgCd.ShowWindow(SW_HIDE);

	//��ǰ��ʾ�Ĵ���;
	m_pPage[1] = &m_dlgCd;
	m_pPage[0] = &m_dlgBg;

	m_iCurTab = 0;
}


void CMainDlg::OnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_pPage[m_iCurTab]->ShowWindow(SW_HIDE);
	m_iCurTab = m_tabSet.GetCurSel();
	m_pPage[m_iCurTab]->ShowWindow(SW_SHOW);

	*pResult = 0;
}
