#pragma once

#include "public_def.h"
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;


struct _struRegionInfo {
	Vec3b v;        //rgbֵ;
	int   nIndex;   //����index;
};

typedef struct struBin{
	int    nCount;  //bin��Ԫ�ظ���;
	double dSum;    //bin��Ԫ��ֵ�ܺ�;
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

	//����ֵͬindex�����ͼ;
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
	int     m_nW, m_nH;      //ͼ����;
	Vec3b   * m_pBgrData;    //Rgbͼ������;
	Vec3b   * m_pHsvData;    //hsv����labͼ������;
	Vec3b   * m_pData;       //��ǰʹ�õ�����;

	unsigned int   * m_pIndexMap;     //ÿ����������������index;    
	bool           * m_bVisit;        //�Ƿ񱻷��ʹ��ı��;
	vector<_struRegionInfo> m_vecReg;  //����������ɫ�������Ϣ;
};

