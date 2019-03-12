#include "stdafx.h"
#include "ImgQuantify.h"  

#define ENABLE_GENERATE_INDEXMAP    0   //�Ƿ�����indexmap

bool IsEqual(Vec3b v, Vec3b dst){
	bool n1 = abs(v[0] - dst[0]) < COLOR_SIMILAR_THRE;
	bool n2 = abs(v[1] - dst[1]) < COLOR_SIMILAR_THRE;
	bool n3 = abs(v[2] - dst[2]) < COLOR_SIMILAR_THRE;

	if (n1 && n2 && n3)
		return true;
	else
		return false;
}

CImgQuantify::CImgQuantify()
{
	m_pData = NULL;
	m_pIndexMap = NULL;
	m_bVisit = NULL;
}


CImgQuantify::~CImgQuantify()
{
	if (m_pData)
		delete[] m_pData;
	if (m_pIndexMap)
		delete[] m_pIndexMap;
	if (m_bVisit)
		delete[] m_bVisit;
}

void CImgQuantify::GetImgData(Mat img){

	int h = img.rows;
	int w = img.cols;
	int nChannel = img.channels();

	//Mat hsvImg;
	//cvtColor(img, hsvImg, CV_RGB2HSV);
	//imwrite("hsv.jpg", hsvImg);

	if (!m_pData)
		return;

	for (int r = 0; r < h; r++)
	{
		//uchar* pdata = img.ptr<uchar>(r);
		for (int c = 0; c < w; c++)
		{
			m_pData[c + r * w] = img.at<Vec3b>(r, c);
		}
	}
}

CImgQuantify::CImgQuantify(Mat img)
{
	if (img.empty())
		return;

	m_OriImg = img.clone();

	m_nH = img.rows;
	m_nW = img.cols;

	int nSize = m_nW * m_nH;
	m_pData = new Vec3b[nSize*sizeof(Vec3b)];
	GetImgData(img);

	m_pIndexMap = new unsigned int[nSize*sizeof(unsigned int)];
	m_bVisit = new bool[nSize * sizeof(bool)];

	memset(m_bVisit, 0, sizeof(bool)*nSize);
	memset(m_pIndexMap, 0, sizeof(unsigned int)*nSize);
}

//��vecConn������и�ʴ;
void CImgQuantify::IndexMapErosion(vector<int> &vecConn){
	bool * bObj = new bool[m_nH*m_nW];
	memset(bObj, 0, sizeof(bool)*m_nH*m_nW);

	//��ʴ��Ľ��������vecFinal��.���ȿ���һ��ԭʼ��;
	vector<int> vecFinal;
	for (int i = 0; i < vecConn.size(); i++)
	{
		bObj[vecConn[i]] = 1;
		vecFinal.push_back(vecConn[i]);
	}

	//ÿ������и�ʴ;
	for (int i = 0; i < vecConn.size(); i++)
	{
		int nInd = vecConn[i];

		int nRow, nCol;
		nRow = nInd / m_nW;
		nCol = nInd % m_nW;

		bool bNoObj = false;
		for (int r = -1; r <= 1; r++)
		{
			if (bNoObj) break;

			for (int c = -1; c <= 1; c++)
			{
				if (r == 0 && c == 0) continue; //�Լ�;

				int nNewR = nRow + r;
				int nNewC = nCol + c;
				int nNewInd = nNewC + nNewR*m_nW;
				if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){
					if (bObj[nNewInd] == 0)
					{
						bNoObj = true;
						break;
					}
				}
			}
		}

		if (bNoObj)
			vecFinal[i] = -1;  //����Ҫ��ʴ���ı��;
	}

	vecConn.clear();
	for (int i = 0; i < vecFinal.size(); i++)
	{
		if (vecFinal[i] != -1)
			vecConn.push_back(vecFinal[i]);
	}

	delete[] bObj;
}

//��index����(������vecConn������)��������;
void CImgQuantify::IndexMapDilate(vector<int> &vecConn){
	bool * bObj = new bool[m_nH*m_nW];
	memset(bObj, 0, sizeof(bool)*m_nH*m_nW);

	//���ͺ�Ľ��������vecFinal��.���ȿ���һ��ԭʼ��;
	vector<int> vecFinal;
	for (int i = 0; i < vecConn.size(); i++)
	{
		bObj[vecConn[i]] = 1;
		vecFinal.push_back(vecConn[i]);
	}
		
	//ÿ�����������;
	for (int i = 0; i < vecConn.size(); i++)
	{
		int nInd = vecConn[i];

		int nRow, nCol;
		nRow = nInd / m_nW;
		nCol = nInd % m_nW;
		for (int r = -1; r <= 1; r++)
		for (int c = -1; c <= 1; c++)
		{
			if (r == 0 && c == 0) continue; //�Լ�;

			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;
			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){
				if (bObj[nNewInd] == 0)
					vecFinal.push_back(nNewInd);
			}
		}
	}

	vecConn.clear();
	for (int i = 0; i < vecFinal.size(); i++)
		vecConn.push_back(vecFinal[i]);

	delete[] bObj;

}

//������nIndex��İ���ͨ���������;����nIndexλ�õ�Ԫ�ؽ��в���;
//nIndex: Ҫ����ĵ������; 
//pVisitMap:���ֲ��ҹ������õ��ķ��ʺۼ����; 
//vecConn:���ص������ڷ��������ĵ�����;
//b8: Ĭ�ϰ���ͨ����Ĵ���.false��Ϊ4��ͨ����;
bool CImgQuantify::DealConnection(int nIndex, bool * pVisitMap, vector<int> &vecConn, bool b8){

	int nRow, nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;

	Vec3b v = m_pData[nIndex];
	for (int r = -1; r <= 1; r++)
	for (int c = -1; c <= 1; c++)
	{
		if (r == 0 && c == 0) continue; //�Լ�������;

		int nNewR = nRow + r;
		int nNewC = nCol + c;
		int nNewInd = nNewC + nNewR*m_nW;

		if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){

			if (m_pIndexMap[nNewInd])  continue;  //�Ѿ���ȫ��������ʹ��ģ����ٷ���,�������ѭ��;
			if (pVisitMap[nNewInd]) continue;  //�������Ӳ��ҹ������ѷ��ʹ��ģ����ٷ���,�������ѭ��;

			Vec3b v1 = m_pData[nNewInd];
			if (IsEqual(v, v1)){
				vecConn.push_back(nNewInd);
				pVisitMap[nNewInd] = true;
			}
		}
	}

	return vecConn.size();
}

//������nIndex��İ���ͨ���������,���������vֵ���в���;
//nIndex: Ҫ����ĵ������;  v: ����vֵ�����������صĲ���;
//pVisitMap:���ֲ��ҹ������õ��ķ��ʺۼ����; 
//vecConn:���ص������ڷ��������ĵ�����;
//vecSum: ���ص������ڵ����е��RGB�ͣ�������һ���ľ�ֵ����;
bool CImgQuantify::DealConnection(int nIndex, Vec3b v, bool * pVisitMap, vector<int> &vecConn,Vec3i &vecSum){

	int nRow,nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;
	
	vecSum[0] = 0;
	vecSum[1] = 0;
	vecSum[2] = 0;

	for (int r = -1; r <= 1; r++)
	for (int c = -1; c <= 1; c++)
	{
		if (r==0 && c==0) continue; //�Լ�������;
		//if (r!=0 && c!=0) continue; //����ͨ;

		int nNewR = nRow + r;
		int nNewC = nCol + c;
		int nNewInd = nNewC + nNewR*m_nW;

		if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){

			if (m_pIndexMap[nNewInd])  continue;  //�Ѿ���ȫ��������ʹ��ģ����ٷ���,�������ѭ��;
			if (pVisitMap[nNewInd]) continue;  //�������Ӳ��ҹ������ѷ��ʹ��ģ����ٷ���,�������ѭ��;

			Vec3b v1 = m_pData[nNewInd];
			if (IsEqual(v, v1)){
				vecConn.push_back(nNewInd);
				pVisitMap[nNewInd] = true;

				//����������Щ���RGB�ܺ�;
				vecSum[0] += v1[0];
				vecSum[1] += v1[1];
				vecSum[2] += v1[2];
			}
		}
	}

	return vecConn.size();
}

//��nIndex��ʼ������ͼ�����ͨ�����е�����;�ҵ���ѹջ������ͣ������ѭ�����ҡ�
//���ڵ�ǰ���ĵ�����ֵ��������������;
/*
nIndex: �������ڵ�λ�ã������ĸ��ط���ʼ��;
vecLoc: ��¼�����к����Ӿ�����ֵͬ�����ص�����;
b8:�Ƿ����ͨ�������;
���أ�  ���ȷ��������,�򷵻�true������Ϊfalse;
*/
bool CImgQuantify::FindSeed_ByCenter(int nIndex, vector<int> &vecLoc, bool b8)
{
	if (!m_pData)
		return false;

	//��ʼ��һ���������Ӳ��ҵķ��ʺۼ���;
	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	vector<int> vecSeedLoc;      //������ֵͬ�����꣬��ջ��ͣ��ѹ�뵯���������ҵ��������ֵ����������;
	Vec3b v = m_pData[nIndex];

	vector<int> vecConn;  //��ͨ����Ĵ���;
	//�����������ͨ�����ҵ���ͬ�ĵ�,ֱ�ӷ���;
	if (!DealConnection(nIndex, m_bVisit, vecConn, b8))
		return false;

	for (int i = 0; i < vecConn.size(); i++)
		vecSeedLoc.push_back(vecConn[i]);

	int nTotalNum = 0;
	while (!vecSeedLoc.empty())
	{
		//����һ��������Ѱ��;
		int ind = vecSeedLoc.back();
		vecSeedLoc.pop_back();

		nTotalNum++;
		vecLoc.push_back(ind);   //ѹ�����յ�ջ;

		//��ͣ����ͬ������;
		vecConn.clear();
		DealConnection(ind, m_bVisit, vecConn, b8);
		for (int i = 0; i < vecConn.size(); i++)
			vecSeedLoc.push_back(vecConn[i]);
	}

	//����ǰ��nIndex����ѹ�����յ�ջ;
	vecLoc.push_back(nIndex);

	//�����ͬ���صĵ㲻����,Ҳ�����ҵ�;
	if (nTotalNum < REG_AREA_THRE)
		return false;

	return true;
}

//��nIndex��ʼ������ͼ�����ͨ�����е�����;�ҵ���ѹջ������ͣ������ѭ�����ҡ�
//�������ҵ����ӵľ�ֵ������ʣ�������;
/*
   nIndex: �������ڵ�λ�ã������ĸ��ط���ʼ��;
   vecLoc: ��¼�����к����Ӿ�����ֵͬ�����ص�����;
   ���أ�  ���ȷ��������,�򷵻�true������Ϊfalse;
*/
bool CImgQuantify::FindSeed_ByAvg(int nIndex, vector<int> &vecLoc){

	if (!m_pData)
		return false;

	//��ʼ��һ���������Ӳ��ҵķ��ʺۼ���;
	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);
	
	vector<int> vecSeedLoc;      //������ֵͬ�����꣬��ջ��ͣ��ѹ�뵯���������ҵ��������ֵ����������;
	Vec3b v = m_pData[nIndex];

	vector<int> vecConn;  //��ͨ����Ĵ���;
	Vec3i       vSum;     //���ص�ǰĳ�����ذ���ͨ����RGBֵ���ܺ�,�������vecSeedLoc���������ص�ƽ��ֵ;
	Vec3b       vAvg;     //��ǰvecSeedLoc���������ص�ƽ��ֵ;
	Vec3d       vdSum;    //��ǰvecSeedLoc���������ص�RGBֵ�ܺ�;
	//�����������ͨ�����ҵ���ͬ�ĵ�,ֱ�ӷ���;
	if (!DealConnection(nIndex, v, m_bVisit, vecConn, vSum))
		return false;

	//����ƽ��ֵ,�Լ�vdSum��ʼ��ֵ;
	for (int i = 0; i < 3; i++)
	{
		vAvg[i]  = int(vSum[i] / vecConn.size() + 0.5);
		vdSum[i] = vSum[i];  
	}
	
	for (int i = 0; i < vecConn.size(); i++)
		vecSeedLoc.push_back(vecConn[i]);

	int nTotalNum = 0;
	while (!vecSeedLoc.empty())
	{
		//����һ��������Ѱ��;
		int ind = vecSeedLoc.back();
		vecSeedLoc.pop_back();

		nTotalNum++;
		vecLoc.push_back(ind);   //ѹ�����յ�ջ;

		//��ͣ����ͬ������;
		vecConn.clear();
		DealConnection(ind,vAvg, m_bVisit, vecConn,vSum);
		for (int i = 0; i < vecConn.size(); i++)
			vecSeedLoc.push_back(vecConn[i]);

		//��������ƽ��ֵ;
		for (int i = 0; i < 3; i++)
		{
			vdSum[i] += vSum[i];
			vAvg[i] = vdSum[i] / (vecSeedLoc.size() + nTotalNum) + 0.5;  //nTotalNum�Ǳ���������ЩԪ��;
		}
	}

	//����ǰ��nIndex����ѹ�����յ�ջ;
	vecLoc.push_back(nIndex);

	//�����ͬ���صĵ㲻����,Ҳ�����ҵ�;
	if (nTotalNum < REG_AREA_THRE)
		return false;

	return true;
}

//�������е������У���û����ֵͬ�ģ�����У��򷵻ظ������index;
//���û�У������IndexMap;
bool CImgQuantify::CheckSameRegion(Vec3b v, int * nInd){
	for (int i = 0; i < m_vecReg.size(); i++)
	{
		Vec3b v1 = m_vecReg[i].v;
		if (IsEqual(v,v1))
		{
			(*nInd) = m_vecReg[i].nIndex;
			return true;
		}
	}

	return false;
}

//����������Ϣ;
//v: ��ǰ���ҵ�������ƽ��ֵ;  vecLoc: ��ǰ�����ӵ����е�����; nNewIndex: ���ҵ������ӵ�������;
//���أ�������µ������򷵻�true,���򷵻�false,������ǰ��������������index���µ�nNewIndex��������;
bool CImgQuantify::UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex){
	int  nInd;
	bool bRet = true;

	//���б������ֵͬ����������,��������ͬһ����;
	if (CheckSameRegion(v, &nInd))
	{
		printf("equal to previous region:%d \n", nInd);
		nNewIndex = nInd;
		bRet = false;
	}
	//����������;
	else
	{
		struRegionInfo ri;
		ri.nIndex = nNewIndex;
		ri.v = v;
		m_vecReg.push_back(ri);
	}
	
	//����������Ϣ;
	for (int i = 0; i < vecLoc.size(); i++)
	{
		int k = vecLoc[i];
		m_pIndexMap[k] = nNewIndex;
	}

	return bRet;
}

//��������index=ָ��ֵ������ͼ;
void CImgQuantify::GenMapImageByIndex(string strFile, int nIndex, Vec3b v){
	Mat m = m_OriImg.clone();
	m.setTo(0);
	for (int i = 0; i < m_nH*m_nW; i++)
	{
		int ind = m_pIndexMap[i];

		if (ind != nIndex ) continue;

		int nRow, nCol;
		nRow = i / m_nW;
		nCol = i % m_nW;
		m.at<Vec3b>(nRow, nCol) = v;
	}
	imwrite(strFile, m);
}

//�����������꣬����һ��ͼ;
void CImgQuantify::GenMapImageByLoc(string strFile, vector<int> vecLoc, Vec3b v){
	Mat m = m_OriImg.clone();
	m.setTo(0);
	for (int i = 0; i < vecLoc.size(); i++)
	{
		int ind = vecLoc[i];
		int nRow, nCol;
		nRow = ind / m_nW;
		nCol = ind % m_nW;
		m.at<Vec3b>(nRow, nCol) = v;
	}
	imwrite(strFile, m);
}

//���ڹ����ĵ�n������������ܱߵ������������ͶƱ,ѡ����ӽ���,��������ӽ������������index;
int CImgQuantify::FindSimilarSeedIndex(int n,int nRadius){
	int nRow, nCol;
	nRow = n / m_nW;
	nCol = n % m_nW;
	
	vector<int> vecIndex;  //�������������index����;
	for (int r = -nRadius; r <= nRadius; r++)
	for (int c = -nRadius; c <= nRadius; c++)
	{
		if (c==0 && r==0) continue;
		int nNewR = nRow + r;
		int nNewC = nCol + c;
		int nNewInd = nNewC + nNewR*m_nW;

		if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){
			if (m_pIndexMap[nNewInd])
				vecIndex.push_back(m_pIndexMap[nNewInd]);
		}
	}

	if (vecIndex.size() == 0)
		return 0;

	//ȥ�������е���ͬindex
	sort(vecIndex.begin(), vecIndex.end());
	vecIndex.erase(unique(vecIndex.begin(), vecIndex.end()), vecIndex.end());

	//Ѱ����ӽ���;
	Vec3b v = m_pData[n];
	int   nMatchInd = 0;
	for (int i = 0; i < vecIndex.size(); i++)
	{
		double MinDist = 255*255*3+1;
	
		int ind = vecIndex[i];
		Vec3b regV = m_vecReg[ind-1].v;  //index=1��Ӧvector�е�1��Ԫ��,�����±�0��ʼ;
		double d = (v[0] - regV[0]) * (v[0] - regV[0]);
		d += (v[1] - regV[1]) * (v[1] - regV[1]);
		d += (v[2] - regV[2]) * (v[2] - regV[2]);

		if (d < MinDist)
		{
			MinDist = d;
			nMatchInd = ind;
		}
	}

	return nMatchInd;
}

//��������index=0,δ���������
void CImgQuantify::DealResidual(){

	int * pTemp = new int[m_nH*m_nW];
	memcpy(pTemp, m_pIndexMap, sizeof(int)*m_nH*m_nW);

	bool bStillNoMatch = false;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int index = m_pIndexMap[i];

		if (index != 0)
			continue;

		//��ʾ�����ĵ㣬�������������;���Ǳ��δ���������ʹ����pTemp����Ӱ�����Ԫ�صĴ���
		index = FindSimilarSeedIndex(i,2);
		if (index == 0)
			bStillNoMatch = true;
		pTemp[i] = index;
	}

	memcpy(m_pIndexMap, pTemp,sizeof(int)*m_nH*m_nW);

	//���й����ĵ㣬�ٴδ���.��δ�����ǰ�δ�������Ӱ�죬�����ܱ�֤���е㶼��������;
	if (bStillNoMatch)
	{
		for (int i = 0; i < m_nW * m_nH; i++)
		{
			int index = m_pIndexMap[i];

			if (index != 0)
				continue;

			//��ʾ�����ĵ㣬�������������,�˴δ���Ϊ1;
			index = FindSimilarSeedIndex(i, 1);
			m_pIndexMap[i] = index;
		}
	}

	delete[] pTemp;
}

//����ֻ�б߽��ߵ�ͼ��;
void CImgQuantify::GenBorderImg(string strFile){
	Mat m = m_OriImg.clone();
	m.setTo(0);

	Vec3b v(255, 255, 255);
	int nRow, nCol;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		nRow = i / m_nW;
		nCol = i % m_nW;
		int nCurInd = m_pIndexMap[i];

		//�ұ�;
		if (nCol + 1 < m_nW){
			if (m_pIndexMap[i+1] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}

		//�±�;
		if (nRow + 1 < m_nH){
			if (m_pIndexMap[i+m_nW] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}	
	}

	imwrite(strFile, m);
}

void CImgQuantify::GenFinalMap(string strFile){

	Mat m = m_OriImg.clone();
	m.setTo(0);

	int nRow, nCol;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		nRow = i / m_nW;
		nCol = i % m_nW;
		int index = m_pIndexMap[i];

		if (index == 0)
			continue;

		Vec3b v(0, 0, 0);
		
		//�Ҹ�index��Ӧ�������ֵ;
		for (int j = 0; j < m_vecReg.size(); j++)
		{
			if (index == m_vecReg[j].nIndex)
			{
				v = m_vecReg[j].v;
				break;
			}
		}

		//�ı�����ɫ;
		v[0] = (v[0] - 60) % 255;
		v[1] = (v[1] + 60) % 255;
		v[2] = (v[2] / 2) % 255;

		m.at<Vec3b>(nRow, nCol) = v;
	}

	cv::imwrite(strFile, m);

	//namedWindow("finalimg", 0);
	//resizeWindow("finalimg", 640, 640);
	//imshow("finalimg", m);
}

//���������������ƽ��ֵ;
Vec3b CImgQuantify::GetMeanV(vector<int> vecLoc){
	double d1=0, d2=0, d3=0;
	int nSize = vecLoc.size();
	for (int i = 0; i < nSize; i++)
	{
		Vec3b v;
		v = m_pData[vecLoc[i]];
		d1 += v[0];
		d2 += v[1];
		d3 += v[2];
	}
	Vec3b vv;
	vv[0] = int(d1 / nSize + 0.5);
	vv[1] = int(d2 / nSize + 0.5);
	vv[2] = int(d3 / nSize + 0.5);

	return vv;
}

//���߽�ͼ������strBorderFile�ļ���,nProgess��ʾ����;
bool CImgQuantify::MainProc(string strBorderFile,int * nProgress){
	vector<int> vecLoc;
	int nCurIndex = 1;

	int nSize = m_nW * m_nH;
	//nSize = 1;
	for (int i = 0; i < nSize; i++)
	{
		if (m_bVisit[i]) continue;

		Vec3b v = m_pData[i];

		float f = i * 1.0 / nSize;
		(*nProgress) = int(f * 100);

		vecLoc.clear();
		if (FindSeed_ByAvg(i,vecLoc))
		{
			//close����;
			IndexMapDilate(vecLoc);
			IndexMapErosion(vecLoc);

			v = GetMeanV(vecLoc);
			printf("Find Seed! Index=%d,Num=%d, Value=%d,%d,%d \n", nCurIndex, vecLoc.size(), v[2], v[1], v[0]);			

			//��д���ʼ�¼;
			for (int j = 0; j < vecLoc.size(); j++)
				m_bVisit[vecLoc[j]] = true;
			
			//����������Ϣ;�����ǰ���µ����������index;
			//���򣬷�������������Ƶ�����;
			int k = nCurIndex;
			if (UpdateRegionInfo(v, vecLoc, k))
				nCurIndex++;

			//����index map;
			if (ENABLE_GENERATE_INDEXMAP){
				char chFile[256] = { 0 };
				sprintf_s(chFile, "index_%d.jpg", k);
				string strFile(chFile);
				GenMapImageByIndex(strFile, k, Vec3b(255, 255, 255));
			}
		}
	}

	//����δ����residual�����ͼ;
	//GenMapImageByIndex("residual.jpg", 0, Vec3b(255, 255, 255));
	//����δ�й��������;
	DealResidual();

	printf("*****************************regin info***************************\n");
	for (int i = 0; i < m_vecReg.size(); i++)
	{
		Vec3b v = m_vecReg[i].v;
		printf("%d---%d,%d,%d \n", m_vecReg[i].nIndex, v[2], v[1], v[0]);

		//���ɴ�����δ��������ĵ��Ժ����������ͼ;
		if (ENABLE_GENERATE_INDEXMAP){
			char chFile[256] = { 0 };
			sprintf_s(chFile, "index_%d_final.jpg", i + 1);
			string strFile(chFile);
			GenMapImageByIndex(strFile, i + 1, Vec3b(255, 255, 255));
		}
	}

	//GenFinalMap("final.jpg");
	GenBorderImg(strBorderFile);

	printf("done!");

	return true;
}
