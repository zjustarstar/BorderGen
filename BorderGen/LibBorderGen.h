/*
	Author: 徐舒畅 博士
	Time  : 2020/06/08
	Dept  : 乐信产学研
	Ver   : V1.1
	Desc  : 该文档用于根据输入的图像文件，生成对应的线框图和着色图。
*/

#include "public_def.h"


/*
	主要处理函数。需要先调用init。
*/
extern "C"  void main_proc();

/*
	初始化函数。
*/
extern "C"  bool init(char * chImgFile, struInParam inParam);