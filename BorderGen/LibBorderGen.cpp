#include "LibBorderGen.h"
#include "PaintImgDealer.h"
#include <opencv2\opencv.hpp>


CPaintImgDealer piDealer;

//主要程序。该程序根据init中inparam指定的strBorderFile和strColorFile
//生成相应的线框图和填色图。
extern "C"  void main_proc() {
	piDealer.MainProc();
}

/*
   函数功能：初始化算法。
   输入参数：chImgFile为待处理的图像的带路径文件名
             inParam表示各种参数。具体参见public_def.h文件。
   返回：如果strImgFile为空，或者参数不正常，会返回false，否则返回true。
*/
extern "C"  bool init(char *  chImgFile, struInParam inParam) {
	Mat img = imread(string(chImgFile));
	return piDealer.init(img, inParam);
}

