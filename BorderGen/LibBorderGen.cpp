#include "LibBorderGen.h"
#include "PaintImgDealer.h"
#include <opencv2\opencv.hpp>


CPaintImgDealer piDealer;

//��Ҫ���򡣸ó������init��inparamָ����strBorderFile��strColorFile
//������Ӧ���߿�ͼ����ɫͼ��
extern "C"  void main_proc() {
	piDealer.MainProc();
}

/*
   �������ܣ���ʼ���㷨��
   ���������chImgFileΪ�������ͼ��Ĵ�·���ļ���
             inParam��ʾ���ֲ���������μ�public_def.h�ļ���
   ���أ����strImgFileΪ�գ����߲������������᷵��false�����򷵻�true��
*/
extern "C"  bool init(char *  chImgFile, struInParam inParam) {
	Mat img = imread(string(chImgFile));
	return piDealer.init(img, inParam);
}

