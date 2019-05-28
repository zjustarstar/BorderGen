#include "stdafx.h"
#include "ClosureDetector.h"
#include <math.h>
#include <opencv2/imgproc/types_c.h>


#define p1 SrcImage.at<uchar>(i, j)  
#define p2 SrcImage.at<uchar>(i - 1, j)  
#define p3 SrcImage.at<uchar>(i - 1, j + 1)  
#define p4 SrcImage.at<uchar>(i, j + 1)  
#define p5 SrcImage.at<uchar>(i + 1, j + 1)  
#define p6 SrcImage.at<uchar>(i + 1, j)  
#define p7 SrcImage.at<uchar>(i + 1, j - 1)  
#define p8 SrcImage.at<uchar>(i, j - 1)  
#define p9 SrcImage.at<uchar>(i - 1, j - 1)  
#define p10 p2

CClosureDetector::CClosureDetector()
{

}


CClosureDetector::~CClosureDetector()
{

}

bool CClosureDetector::GetThinImg(Mat img, Mat & thinImg)
{
	if (img.empty())
		return false;
	m_OriImg = img.clone();

	Mat grayImg, bimg;
	cvtColor(img, grayImg, CV_BGR2GRAY);

	threshold(grayImg, bimg, 128, 255, THRESH_BINARY);

	bitwise_not(bimg, bimg); //改为目标是255，背景是0;
	imwrite("bimg.jpg", bimg);
	bimg = bimg / 255; //输入的目标像素为1，背景像素为0;

	//骨骼化;
	FindSkeleton(bimg, thinImg);
	thinImg = thinImg * 255;

	//在骨骼化的基础上找不封闭的点;
	return FindNoClosure(thinImg, thinImg);
}

bool CClosureDetector::IsIsolatedPt(Mat InImage, Point pt)
{
	int radius = 10;  //在一定的区域查找;
	int y = pt.x;  //y坐标;
	int x = pt.y;  //x坐标;

	int t = ((y - radius) >= 0) ? (y - radius) : 0;
	int b = ((y + radius) <= (InImage.rows - 1)) ? (y + radius) : (InImage.rows - 1);
	int l = ((x - radius) >= 0) ? (x - radius) : 0;
	int r = ((x + radius) <= (InImage.cols - 1)) ? (x + radius) : (InImage.cols - 1);

	Rect rect;
	rect.x = l;
	rect.y = t;
	rect.width = r - l;
	rect.height = b - t;
	Mat SrcImage = InImage(rect);
	
	//如果能找到连接的点中有T字型的点，则说明不是一个真正的孤立点;
	Point newpt;  //在roi图像中的原pt坐标;
	newpt.x = radius;
	newpt.y = radius;

	vector<Point> vecObjPt;
	vecObjPt.push_back(newpt);

	int nw = SrcImage.cols;
	int nh = SrcImage.rows;
	int nSize = nw*nh;
	bool * bFlag = new bool[nSize];
	memset(bFlag, 0, nSize*sizeof(bool));

	bFlag[newpt.x + newpt.y * nw] = 1;

	while (!vecObjPt.empty())
	{
		Point p = vecObjPt.back();
		vecObjPt.pop_back();

		int i = p.x;
		int j = p.y;
		int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
		//T字型或者泛T字型的,直接return;
		if (n >= 3 * 255)
		{
			delete[] bFlag;
			return false;
		} 
		else
		{
			//如果当前点不是T型点,将其八连通邻域的目标点加入队列;
			for (int row = -1; row <= 1; row++)
				for (int col = -1; col <= 1; col++)
				{
					if (row==0 && col==0) continue; //自己不加入;
					
					int newi = i + row;
					int newj = j + col;
					//已经访问过的不访问;
					if (bFlag[newj + newi * nw]) continue;

					if (SrcImage.at<uchar>(newi, newj) == 255)
					{
						vecObjPt.push_back(Point(newi, newj));
						bFlag[newj + newi * nw] = 1;
					}
				}
		}

	}
	delete[] bFlag;
	return true;
}


////输入的目标像素为255，背景像素为0;
//SrcImage为输入的细化图;resImg为输出的找到了未封闭点的图;
bool   CClosureDetector::FindNoClosure(Mat SrcImage, Mat &resImg) {
	
	//cvtColor(SrcImage, resImg, CV_GRAY2BGR);

	int h = SrcImage.rows;
	int w = SrcImage.cols;

	bool bRes = false;

	//边界不访问;
	for (int i = 5; i < h-5; i++)
		for (int j = 5; j < w-5; j++)
	{
		//只针对目标像素;
		if (p1 != 255)
			continue;

		int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
		//孤立点;
		if (n <= 1 * 255)
		{
			Rect rect;
			rect.x = (j - 20) > 0 ? (j - 20) : 0;
			rect.y = (i - 20) > 0 ? (i - 20) : 0;
			rect.width = 40;
			rect.height = 40;

			Scalar s(0, 0, 255);
			if (IsIsolatedPt(SrcImage, cvPoint(i, j)))
			{
				rectangle(m_OriImg, rect, s, 2);
				bRes = true;
			}
		}
	}

	resImg = m_OriImg;

	return bRes;
}

void CClosureDetector::FindSkeleton(Mat SrcImage, Mat &DstImage)//输入的目标像素为1，背景像素为0  
{
	/*0：先对图像的边缘进行处理,避免图片的边缘对骨架提取造成的影响——如果图片四边都为0则没有必要*/
	for (int j = 0; j < SrcImage.cols; j++)
	{
		SrcImage.at<uchar>(0, j) = 0;
		SrcImage.at<uchar>(SrcImage.rows - 1, j) = 0;
	}
	for (int i = 0; i < SrcImage.rows; i++)
	{
		SrcImage.at<uchar>(i, 0) = 0;
		SrcImage.at<uchar>(i, SrcImage.cols - 1) = 0;
	}
	/*0:记录所有非零点的坐标*/
	vector<Point>target;
	for (int i = 1; i < SrcImage.rows - 1; i++)
	{
		for (int j = 1; j < SrcImage.cols - 1; j++)
		{
			if (SrcImage.at<uchar>(i, j) == 1)
			{
				target.push_back(Point(i, j));
			}
		}
	}
	//--------------------------Zhang----------------------------------  
	int flag = 1;
	while (flag)
	{
		flag = 0;//清空标志位  
				 /*1：检测像素点是否满足Zhang快速细化法步骤1的四个条件*/
		vector<Point> erase, reserve;
		for (int k = 0; k < target.size(); k++)//只检测已经记录的点  
		{
			int i = target[k].x, j = target[k].y;
			//检测条件a 2<=N(p1)<=6  
			int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			if (n < 2 || n > 6)//不满足条件a，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件b T(p1)=1  
			int t = 0;
			if (p2 == 0 && p3 == 1)
				t++;
			if (p3 == 0 && p4 == 1)
				t++;
			if (p4 == 0 && p5 == 1)
				t++;
			if (p5 == 0 && p6 == 1)
				t++;
			if (p6 == 0 && p7 == 1)
				t++;
			if (p7 == 0 && p8 == 1)
				t++;
			if (p8 == 0 && p9 == 1)
				t++;
			if (p9 == 0 && p10 == 1)
				t++;
			if (t != 1)//不满足条件b，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件c p2*p4*p6=0  
			if (p2 * p4 * p6 != 0)//不满足条件c，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件d  p4*p6*p8=0  
			if (p4 * p6 * p8 != 0)//不满足条件d，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//已经满足了所有的条件，该点可以被删除,使用vector记录下当前要删除的点的坐标  
			erase.push_back(Point(i, j));//其实这里是反的，在后面又重新反了回来  
			flag = 1;//置1标志位，表示有过修改  
		}
		/*2:删除所有标记点*/
		for (int i = 0; i < erase.size(); i++)
		{
			SrcImage.at<uchar>(erase[i].x, erase[i].y) = 0;
		}
		erase.clear();
		target.clear();
		target = reserve;//更新当前的非零点坐标  
		reserve.clear();
		/*3:检测像素点是否满足Zhang快速细化法步骤2的四个条件*/
		for (int k = 0; k < target.size(); k++)
		{
			int i = target[k].x, j = target[k].y;
			//检测条件a 2<=N(p1)<=6  
			int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			if (n < 2 || n > 6)//不满足条件a，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件b T(p1)=1  
			int t = 0;
			if (p2 == 0 && p3 == 1)
				t++;
			if (p3 == 0 && p4 == 1)
				t++;
			if (p4 == 0 && p5 == 1)
				t++;
			if (p5 == 0 && p6 == 1)
				t++;
			if (p6 == 0 && p7 == 1)
				t++;
			if (p7 == 0 && p8 == 1)
				t++;
			if (p8 == 0 && p9 == 1)
				t++;
			if (p9 == 0 && p10 == 1)
				t++;
			if (t != 1)//不满足条件b，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件c p2*p4*p8=0  
			if (p2 * p4 * p8 != 0)//不满足条件c，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//检测条件d  p2*p6*p8=0  
			if (p2 * p6 * p8 != 0)//不满足条件d，该点不能删除，直接检测下一个点  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//已经满足了所有的条件，该点可以被删除，使用vector记录下当前要删除的点的坐标  
			erase.push_back(Point(i, j));
			flag = 1;//置1标志位，表示有过修改  
		}
		/*4:删除所有标记点*/
		for (int i = 0; i < erase.size(); i++)
		{
			SrcImage.at<uchar>(erase[i].x, erase[i].y) = 0;
		}
		erase.clear();
		target.clear();
		target = reserve;//更新当前的非零点坐标  
		reserve.clear();
	}
	DstImage = SrcImage;

}
