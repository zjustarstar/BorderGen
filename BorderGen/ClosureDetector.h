#pragma once
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

class CClosureDetector
{
public:
	CClosureDetector();
	virtual ~CClosureDetector();
	bool   GetThinImg(Mat img, Mat &thinImg);

private:
	Mat    m_OriImg;  
	bool   IsIsolatedPt(Mat SrcImage, Point pt);
	void   FindSkeleton(Mat SrcImage, Mat &DstImage);//�����Ŀ������Ϊ1����������Ϊ0
	bool   FindNoClosure(Mat img, Mat &resImg);
};

