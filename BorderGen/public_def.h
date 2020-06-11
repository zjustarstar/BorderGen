#pragma once

//不同颜色空间;
#define COLOR_RGB 0
#define COLOR_HSV 1
#define COLOR_LAB 2

struct struInParam {
	char * strBorderFile;   //生成的边界文件;
	char * strColorFile;    //生成的涂色文件;
	int  * nProgress;       //表示处理进度;

	bool   bFastMode;       //使用快速模式; 默认使用快速模式，非快速模式效果更好，但是速度更慢;
	int    nColorMode;      //使用的颜色空间，取值如上面定义的0-2
	bool   bWhiteBG;        //生成边界文件的背景，默认是白色的;
	bool   bThickBd;        //是否生成粗边界; （该参数线框生成中用处不大）
	int    nColorThre;      //颜色阈值;
	int    nMinAreaThre;    //最小区域阈值;
	int    nFinalClrNum;    //最终颜色数量;默认为0，表示不指定最终颜色数量，由程序自动生成。
							//如果大于0，则生成的填色图中的颜色数量由该值决定。
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
		nFinalClrNum = 0;  // 0表示不指定最终颜色数量;
	}
};