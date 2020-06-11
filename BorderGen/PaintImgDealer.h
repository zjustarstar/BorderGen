#pragma once

#include "public_def.h"
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;


struct _struRegionInfo {
	Vec3b v;        //rgb值;
	int   nIndex;   //区域index;
};

typedef struct struBin{
	int    nCount;  //bin中元素个数;
	double dSum;    //bin中元素值总和;
	struBin(){
		nCount = 0;
		dSum = 0.0;
	}
};

class CPaintImgDealer
{
public:
	CPaintImgDealer();
	CPaintImgDealer(Mat img, struInParam param);
	bool init(Mat img, struInParam param);
	virtual ~CPaintImgDealer();
	void GetImgData(const Mat img);

	void PreProcess(Mat img);
	void QuantifySat_Val(struBin * pBin, vector<int> *pVecIndex, bool bSat);
	void QuantifyHue(struBin * pBin, vector<int> *pVecIndex);

	void MainProc();

	//seeds;
	bool DealConnection(int nIndex, Vec3b v, bool * pVisitMap, vector<int> &vecConn, Vec3i &vecSum);
	bool DealConnection_FindSameReg(int nIndex, bool * pVisitMap, vector<int> &vecConn, vector<int> &vecNeib);
	bool FindSeed_ByAvg(int nIndex, vector<int> &vecLoc);
	Vec3b GetMeanV(vector<int> vecLoc);
	bool IsEqual(Vec3b v, Vec3b dst);
	int FindSimilarSeedIndex(int n, int nRadius);

	//生成同值index区域的图;
	void GenMapImageByIndex(string strFile, int nIndex, Vec3b v);
	void GenMapImageByRegColor(string strFile, bool bColorReassign=false);
	void GenBorderImg(string strFile, bool bWhiteBG=true, bool bThickBd=true);

	//regions;
	bool CheckSameRegion(Vec3b v, int * nInd);
	bool UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex);
	void DealResidual();
	void RemoveIsolatedPixel();
	bool ColorReassign(int nNewColorNum);
	int  CheckRegionStatus();

	//index map;
	void IndexMapErosion(vector<int> &vecConn);
	void IndexMapDilate(vector<int> &vecConn);

	Vec3b  CalcAvgValue(int nIndex);

private:
	struInParam m_param;
	Mat     m_OriImg;
	int     m_nW, m_nH;      //图像宽高;
	Vec3b   * m_pBgrData;    //Rgb图像数据;
	Vec3b   * m_pHsvData;    //hsv或者lab图像数据;
	Vec3b   * m_pData;       //当前使用的数据;

	unsigned int   * m_pIndexMap;     //每个像素所属的区域index;    
	bool           * m_bVisit;        //是否被访问过的标记;
	vector<_struRegionInfo> m_vecReg;  //所有量化颜色区域的信息;
};

