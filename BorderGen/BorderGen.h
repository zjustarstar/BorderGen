
// BorderGen.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBorderGenApp: 
// �йش����ʵ�֣������ BorderGen.cpp
//

class CBorderGenApp : public CWinApp
{
public:
	CBorderGenApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBorderGenApp theApp;