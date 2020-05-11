#pragma once

#include <opencv2\opencv.hpp>

#define COLOR_SIMILAR_THRE   5     //颜色相似度阈值;
//#define REG_AREA_THRE        100    //最小种子区域的阈值;

using namespace std;
using namespace cv;


struct struRegionInfo{
	Vec3b v;        //rgb值;
	int   nIndex;   //区域index;
};

class CImgQuantify
{
public:
	CImgQuantify();
	CImgQuantify(Mat img);
	virtual ~CImgQuantify();

	//参数设置;
	void setMinRegNum(int nNum);
	bool MainProc(string strBorderFile, int * nProgress, bool bWithBg=false);

private:
	//image
	void GetImgData(Mat img);
	//生成某个种子区域的图;
	void GenMapImageByLoc(string strFile, vector<int> vecLoc, Vec3b v);
	//生成同值index区域的图;
	void GenMapImageByIndex(string strFile, int nIndex, Vec3b v);
	//生成最终的分区图
	void GenFinalMap(string strFile);
	void GenBorderImg(string strFile, bool bWithBg=false);

	//seeds;
	bool DealConnection(int nIndex, bool * pVisitMap, vector<int> &vecConn, bool b8=true);
	bool DealConnection(int nIndex, Vec3b v, bool * pVisitMap, vector<int> &vecConn, Vec3i &vecSum);
	bool FindSeed_ByAvg(int nIndex, vector<int> &vecLoc);
	bool FindSeed_ByCenter(int nIndex, vector<int> &vecLoc, bool b8 = true);
	Vec3b GetMeanV(vector<int> vecLoc);
	int  FindSimilarSeedIndex(int n, int nRadius);
	void DealResidual();

	//index map;
	void IndexMapErosion(vector<int> &vecConn);
	void IndexMapDilate(vector<int> &vecConn);

	//regions;
	bool CheckSameRegion(Vec3b v, int * nInd);
	bool UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex);

private:
	Mat              m_OriImg;
	Vec3b          * m_pData;        //图像数据;
	int              m_nW,m_nH;      //图像宽高;
	int              m_nMinRegNum;   //每个区域最少像素数：
	unsigned int   * m_pIndexMap;     //每个像素所属的区域index;    
	bool           * m_bVisit;        //是否被访问过的标记;
	vector<struRegionInfo> m_vecReg;  //所有量化颜色区域的信息;
};

