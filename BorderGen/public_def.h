#pragma once

//��ͬ��ɫ�ռ�;
#define COLOR_RGB 0
#define COLOR_HSV 1
#define COLOR_LAB 2

struct struInParam {
	char * strBorderFile;   //���ɵı߽��ļ�;
	char * strColorFile;    //���ɵ�Ϳɫ�ļ�;
	int  * nProgress;       //��ʾ�������;

	bool   bFastMode;       //ʹ�ÿ���ģʽ; Ĭ��ʹ�ÿ���ģʽ���ǿ���ģʽЧ�����ã������ٶȸ���;
	int    nColorMode;      //ʹ�õ���ɫ�ռ䣬ȡֵ�����涨���0-2
	bool   bWhiteBG;        //���ɱ߽��ļ��ı�����Ĭ���ǰ�ɫ��;
	bool   bThickBd;        //�Ƿ����ɴֱ߽�; ���ò����߿��������ô�����
	int    nColorThre;      //��ɫ��ֵ;
	int    nMinAreaThre;    //��С������ֵ;
	int    nFinalClrNum;    //������ɫ����;Ĭ��Ϊ0����ʾ��ָ��������ɫ�������ɳ����Զ����ɡ�
							//�������0�������ɵ���ɫͼ�е���ɫ�����ɸ�ֵ������
	struInParam() {
		strBorderFile = "";
		strColorFile = "";
		nProgress = 0;
		nColorMode = COLOR_RGB;
		bFastMode = true; 
		bWhiteBG = true;
		bThickBd = true;
		nColorThre = 20;
		nMinAreaThre = 100;
		nFinalClrNum = 0;  // 0��ʾ��ָ��������ɫ����;
	}
};