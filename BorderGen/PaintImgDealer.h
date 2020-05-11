#pragma once

#include <opencv2\opencv.hpp>
#include "..\..\BorderGen\BorderGen\ImgQuantify.h"

using namespace std;
using namespace cv;

#define COLOR_DIST_THRE  20

typedef struct struBin{
	int    nCount;  //bin中元素个数;
	double dSum;    //bin中元素值总和;
	struBin(){
		nCount = 0;
		dSum = 0.0;
	}
};

typedef struct struInParam {
	string strBorderFile;   //生成的边界文件;
	string strColorFile;    //生成的涂色文件;
	int  * nProgress;       //表示进度;
	bool   bRGBData;        //使用的颜色空间;
	bool   bWhiteBG;        //生成边界文件的背景默认是白色的;
	bool   bThickBd;        //是否生成粗边界;
	struInParam() {
		strBorderFile = "";
		strColorFile = "";
		nProgress = NULL;
		bRGBData  = true;
		bWhiteBG  = true;
		bThickBd  = true;
	}
};

class CPaintImgDealer
{
public:
	CPaintImgDealer();
	CPaintImgDealer(Mat img);
	virtual ~CPaintImgDealer();
	void GetImgData(const Mat img);

	void setMinRegNum(int nNum);
	void PreProcess(Mat img);
	void QuantifySat_Val(struBin * pBin, vector<int> *pVecIndex, bool bSat);
	void QuantifyHue(struBin * pBin, vector<int> *pVecIndex);

	void MainProc(struInParam param);
	void MainProc_bySeg(Mat & resultImg);

	//seeds;
	bool DealConnection(int nIndex, Vec3b v, bool * pVisitMap, vector<int> &vecConn, Vec3i &vecSum);
	bool FindSeed_ByAvg(int nIndex, vector<int> &vecLoc);
	Vec3b GetMeanV(vector<int> vecLoc);
	bool IsEqual(Vec3b v, Vec3b dst);
	int FindSimilarSeedIndex(int n, int nRadius);

	//生成同值index区域的图;
	void GenMapImageByIndex(string strFile, int nIndex, Vec3b v);
	void GenMapImageByRegColor(string strFile);
	void GenBorderImg(string strFile, bool bWhiteBG=true, bool bThickBd=true);

	//regions;
	bool CheckSameRegion(Vec3b v, int * nInd);
	bool UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex);
	void DealResidual();

	//index map;
	void IndexMapErosion(vector<int> &vecConn);
	void IndexMapDilate(vector<int> &vecConn);

private:
	Mat     m_OriImg;
	int     m_nW, m_nH;      //图像宽高;
	bool    m_bUseRGB;       //是否使用RGB数据;
	Vec3b   * m_pBgrData;    //Rgb图像数据;
	Vec3b   * m_pHsvData;    //hsv图像数据;
	Vec3b   * m_pData;       //当前使用的数据;

	int   m_nMinRegNum;      //最小区域像素数;
	unsigned int   * m_pIndexMap;     //每个像素所属的区域index;    
	bool           * m_bVisit;        //是否被访问过的标记;
	vector<struRegionInfo> m_vecReg;  //所有量化颜色区域的信息;
};

