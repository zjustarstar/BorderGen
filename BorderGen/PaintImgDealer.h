#pragma once

#include <opencv2\opencv.hpp>
#include "..\..\BorderGen\BorderGen\ImgQuantify.h"

using namespace std;
using namespace cv;

#define COLOR_DIST_THRE  20

typedef struct struBin{
	int    nCount;  //bin��Ԫ�ظ���;
	double dSum;    //bin��Ԫ��ֵ�ܺ�;
	struBin(){
		nCount = 0;
		dSum = 0.0;
	}
};

typedef struct struInParam {
	string strBorderFile;   //���ɵı߽��ļ�;
	string strColorFile;    //���ɵ�Ϳɫ�ļ�;
	int  * nProgress;       //��ʾ����;
	bool   bRGBData;        //ʹ�õ���ɫ�ռ�;
	bool   bWhiteBG;        //���ɱ߽��ļ��ı���Ĭ���ǰ�ɫ��;
	bool   bThickBd;        //�Ƿ����ɴֱ߽�;
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

	//����ֵͬindex�����ͼ;
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
	int     m_nW, m_nH;      //ͼ����;
	bool    m_bUseRGB;       //�Ƿ�ʹ��RGB����;
	Vec3b   * m_pBgrData;    //Rgbͼ������;
	Vec3b   * m_pHsvData;    //hsvͼ������;
	Vec3b   * m_pData;       //��ǰʹ�õ�����;

	int   m_nMinRegNum;      //��С����������;
	unsigned int   * m_pIndexMap;     //ÿ����������������index;    
	bool           * m_bVisit;        //�Ƿ񱻷��ʹ��ı��;
	vector<struRegionInfo> m_vecReg;  //����������ɫ�������Ϣ;
};

