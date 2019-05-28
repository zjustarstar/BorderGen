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

	bitwise_not(bimg, bimg); //��ΪĿ����255��������0;
	imwrite("bimg.jpg", bimg);
	bimg = bimg / 255; //�����Ŀ������Ϊ1����������Ϊ0;

	//������;
	FindSkeleton(bimg, thinImg);
	thinImg = thinImg * 255;

	//�ڹ������Ļ������Ҳ���յĵ�;
	return FindNoClosure(thinImg, thinImg);
}

bool CClosureDetector::IsIsolatedPt(Mat InImage, Point pt)
{
	int radius = 10;  //��һ�����������;
	int y = pt.x;  //y����;
	int x = pt.y;  //x����;

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
	
	//������ҵ����ӵĵ�����T���͵ĵ㣬��˵������һ�������Ĺ�����;
	Point newpt;  //��roiͼ���е�ԭpt����;
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
		//T���ͻ��߷�T���͵�,ֱ��return;
		if (n >= 3 * 255)
		{
			delete[] bFlag;
			return false;
		} 
		else
		{
			//�����ǰ�㲻��T�͵�,�������ͨ�����Ŀ���������;
			for (int row = -1; row <= 1; row++)
				for (int col = -1; col <= 1; col++)
				{
					if (row==0 && col==0) continue; //�Լ�������;
					
					int newi = i + row;
					int newj = j + col;
					//�Ѿ����ʹ��Ĳ�����;
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


////�����Ŀ������Ϊ255����������Ϊ0;
//SrcImageΪ�����ϸ��ͼ;resImgΪ������ҵ���δ��յ��ͼ;
bool   CClosureDetector::FindNoClosure(Mat SrcImage, Mat &resImg) {
	
	//cvtColor(SrcImage, resImg, CV_GRAY2BGR);

	int h = SrcImage.rows;
	int w = SrcImage.cols;

	bool bRes = false;

	//�߽粻����;
	for (int i = 5; i < h-5; i++)
		for (int j = 5; j < w-5; j++)
	{
		//ֻ���Ŀ������;
		if (p1 != 255)
			continue;

		int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
		//������;
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

void CClosureDetector::FindSkeleton(Mat SrcImage, Mat &DstImage)//�����Ŀ������Ϊ1����������Ϊ0  
{
	/*0���ȶ�ͼ��ı�Ե���д���,����ͼƬ�ı�Ե�ԹǼ���ȡ��ɵ�Ӱ�졪�����ͼƬ�ı߶�Ϊ0��û�б�Ҫ*/
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
	/*0:��¼���з���������*/
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
		flag = 0;//��ձ�־λ  
				 /*1��������ص��Ƿ�����Zhang����ϸ��������1���ĸ�����*/
		vector<Point> erase, reserve;
		for (int k = 0; k < target.size(); k++)//ֻ����Ѿ���¼�ĵ�  
		{
			int i = target[k].x, j = target[k].y;
			//�������a 2<=N(p1)<=6  
			int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			if (n < 2 || n > 6)//����������a���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������b T(p1)=1  
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
			if (t != 1)//����������b���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������c p2*p4*p6=0  
			if (p2 * p4 * p6 != 0)//����������c���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������d  p4*p6*p8=0  
			if (p4 * p6 * p8 != 0)//����������d���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�Ѿ����������е��������õ���Ա�ɾ��,ʹ��vector��¼�µ�ǰҪɾ���ĵ������  
			erase.push_back(Point(i, j));//��ʵ�����Ƿ��ģ��ں��������·��˻���  
			flag = 1;//��1��־λ����ʾ�й��޸�  
		}
		/*2:ɾ�����б�ǵ�*/
		for (int i = 0; i < erase.size(); i++)
		{
			SrcImage.at<uchar>(erase[i].x, erase[i].y) = 0;
		}
		erase.clear();
		target.clear();
		target = reserve;//���µ�ǰ�ķ��������  
		reserve.clear();
		/*3:������ص��Ƿ�����Zhang����ϸ��������2���ĸ�����*/
		for (int k = 0; k < target.size(); k++)
		{
			int i = target[k].x, j = target[k].y;
			//�������a 2<=N(p1)<=6  
			int n = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			if (n < 2 || n > 6)//����������a���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������b T(p1)=1  
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
			if (t != 1)//����������b���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������c p2*p4*p8=0  
			if (p2 * p4 * p8 != 0)//����������c���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�������d  p2*p6*p8=0  
			if (p2 * p6 * p8 != 0)//����������d���õ㲻��ɾ����ֱ�Ӽ����һ����  
			{
				reserve.push_back(Point(i, j));
				continue;
			}
			//�Ѿ����������е��������õ���Ա�ɾ����ʹ��vector��¼�µ�ǰҪɾ���ĵ������  
			erase.push_back(Point(i, j));
			flag = 1;//��1��־λ����ʾ�й��޸�  
		}
		/*4:ɾ�����б�ǵ�*/
		for (int i = 0; i < erase.size(); i++)
		{
			SrcImage.at<uchar>(erase[i].x, erase[i].y) = 0;
		}
		erase.clear();
		target.clear();
		target = reserve;//���µ�ǰ�ķ��������  
		reserve.clear();
	}
	DstImage = SrcImage;

}
