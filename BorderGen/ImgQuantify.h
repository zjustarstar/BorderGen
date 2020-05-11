#pragma once

#include <opencv2\opencv.hpp>

#define COLOR_SIMILAR_THRE   5     //��ɫ���ƶ���ֵ;
//#define REG_AREA_THRE        100    //��С�����������ֵ;

using namespace std;
using namespace cv;


struct struRegionInfo{
	Vec3b v;        //rgbֵ;
	int   nIndex;   //����index;
};

class CImgQuantify
{
public:
	CImgQuantify();
	CImgQuantify(Mat img);
	virtual ~CImgQuantify();

	//��������;
	void setMinRegNum(int nNum);
	bool MainProc(string strBorderFile, int * nProgress, bool bWithBg=false);

private:
	//image
	void GetImgData(Mat img);
	//����ĳ�����������ͼ;
	void GenMapImageByLoc(string strFile, vector<int> vecLoc, Vec3b v);
	//����ֵͬindex�����ͼ;
	void GenMapImageByIndex(string strFile, int nIndex, Vec3b v);
	//�������յķ���ͼ
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
	Vec3b          * m_pData;        //ͼ������;
	int              m_nW,m_nH;      //ͼ����;
	int              m_nMinRegNum;   //ÿ������������������
	unsigned int   * m_pIndexMap;     //ÿ����������������index;    
	bool           * m_bVisit;        //�Ƿ񱻷��ʹ��ı��;
	vector<struRegionInfo> m_vecReg;  //����������ɫ�������Ϣ;
};

