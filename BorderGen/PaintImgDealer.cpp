#include "stdafx.h"
#include "PaintImgDealer.h"
#include "L0Smooth.h"

#include <opencv2/imgproc/types_c.h>

#define ENABLE_GENERATE_INDEXMAP 0

CPaintImgDealer::CPaintImgDealer()
{
	m_pBgrData = NULL;
	m_pHsvData = NULL;

	m_nMinRegNum = 100;
}


CPaintImgDealer::~CPaintImgDealer()
{
	if (m_pHsvData)
		delete[] m_pHsvData;
	if (m_pBgrData)
		delete[] m_pBgrData;
	if (m_pIndexMap)
		delete[] m_pIndexMap;
	if (m_bVisit)
		delete[] m_bVisit;
}


CPaintImgDealer::CPaintImgDealer(Mat img,bool bFastMode)
{
	if (img.empty())
		return;

	//ƽ��Ԥ����;
	if (!bFastMode) {
		CL0Smooth ls;
		img = ls.L0Smoothing(img);
	}
	else
		GaussianBlur(img, img, cvSize(3, 3), 0);

	m_OriImg = img.clone();

	m_nH = img.rows;
	m_nW = img.cols;

	int nSize = m_nW * m_nH;
	m_pBgrData = new Vec3b[nSize*sizeof(Vec3b)];
	m_pHsvData = new Vec3b[nSize*sizeof(Vec3b)];

	m_pIndexMap = new unsigned int[nSize*sizeof(unsigned int)];
	m_bVisit = new bool[nSize * sizeof(bool)];

	memset(m_bVisit, 0, sizeof(bool)*nSize);
	memset(m_pIndexMap, 0, sizeof(unsigned int)*nSize);

	static int i = 0;
	char strName[256];
	sprintf_s(strName, "d:\\%d.jpg", i++);
	//imwrite(string(strName), dstImg);

	//�ٶ�Ҫ��ȽϿ�ʱ...
	if (bFastMode)
	{
		Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
		filter2D(img, img, img.depth(), kernel);
	}

	GetImgData(img);
}


/*
 �� bin[0]: 0-10; 
 �� bin[1]: 11-25;
 �� bin[2]: 26-34;
 �� bin[3]: 37-77;
 �� bin[4]: 78-99;
 �� bin[5]: 100-124;
 �� bin[6]: 125-155;
 �� bin[7]: 156-180;
*/
void CPaintImgDealer::QuantifyHue(struBin * pBin, vector<int> *pVecIndex){
	int nSize = m_nW * m_nH;
	int nHueBin[8] = { 0,10, 25, 34, 77, 99, 124, 155 };

	for (int i = 0; i < nSize; i++)
	{
		Vec3b v = m_pHsvData[i];
		uchar s = v[0];   //ɫ��;

		if (s >= 156)
		{
			pBin[7].nCount++;
			pBin[7].dSum += s;
			pVecIndex[7].push_back(i);
		}
		else
		{
			int k = 0;
			for (int j = 0; j < 7; j++)
			{
				if (s >nHueBin[j] && s <= nHueBin[j + 1])
				{
					k = j;
					break;
				}
			}

			pBin[k].nCount++;
			pBin[k].dSum += s;
			pVecIndex[k].push_back(i);
		}
	}
}

//��Saturation��Value������������ͬ��������; bSat=true��ʾ�Ա��Ͷ�����,false��ʾ����������; 
void CPaintImgDealer::QuantifySat_Val(struBin * pBin, vector<int> *pVecIndex,bool bSat){
	
	int nSize = m_nW * m_nH;
	for (int i = 0; i < nSize; i++)
	{
		Vec3b v = m_pHsvData[i];
		uchar s = v[1];   //���Ͷ�;
		if (!bSat)
			s = v[2];     //����;

		//��ʼ��<=40;
		if (s <= 40)
		{
			pBin[0].nCount++;
			pBin[0].dSum += s;
			pVecIndex[0].push_back(i);
		}
		//���һ��240-255;
		else if (s >= 240)
		{
			pBin[9].nCount++;
			pBin[9].dSum += s;
			pVecIndex[9].push_back(i);
		}
		else
		{
			int k = (s - 40) / 25 + 1;  //ÿ�μ��25;
			pBin[k].nCount++;
			pBin[k].dSum += s;
			pVecIndex[k].push_back(i);
		}
	}

}

void CPaintImgDealer::GetImgData(const Mat img){
	int h = img.rows;
	int w = img.cols;
	int nChannel = img.channels();

	Mat hsvImg;
	cvtColor(img, hsvImg, CV_BGR2HSV);

	if (!m_pBgrData || !m_pHsvData)
		return;

	for (int r = 0; r < h; r++)
	{
		//uchar* pdata = img.ptr<uchar>(r);
		for (int c = 0; c < w; c++)
		{
			m_pBgrData[c + r * w] = img.at<Vec3b>(r, c);
			m_pHsvData[c + r * w] = hsvImg.at<Vec3b>(r, c);
		}
	}
}

//��RGB���ݵĺ���λȥ��;
void CPaintImgDealer::PreProcess(Mat img){

	int h = img.rows;
	int w = img.cols;

	for (int r = 0; r < h; r++)
	{
		for (int c = 0; c < w; c++)
		{
			Vec3b v = img.at<Vec3b>(r, c);

			Vec3b v2;
			for (int i = 0; i < 3; i++)
				v2[i] = (v[i] >> 2) << 2;
			img.at<Vec3b>(r, c) = v2;
		}
	}
}

void CPaintImgDealer::MainProc_bySeg(Mat & resultImg) {
	Mat m;

	//m = m_OriImg;
	GaussianBlur(m_OriImg, m, cvSize(3, 3), 0);

	//namedWindow("blur");
	//imshow("blur", m);

	Mat gray;
	cvtColor(m, gray, CV_BGR2GRAY);

	Mat grad_x, grad_y,grad;
	Mat abs_grad_x, abs_grad_y;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	/// �� X�����ݶ�
	Sobel(gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	/// �� Y�����ݶ�
	Sobel(gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);

	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

	//namedWindow("sobel");
	//imshow("sobel", grad);
	//imwrite("d:\\grad.jpg", grad);

	Mat bimg1,bimg2;
	threshold(grad, bimg1, 0, 255, THRESH_OTSU);
	namedWindow("otsu");
	imshow("otsu", bimg1);
	imwrite("ostu.jpg", bimg1);

	//��ʴ��ֵͼ��
	Mat kern = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	erode(bimg1, bimg1, kern);
	dilate(bimg1, bimg1, kern);
	imwrite("close.jpg", bimg1);

	resultImg = grad;
}

/*
void CPaintImgDealer::MainProc(Mat & resultImg){
	struBin * pBin = new struBin[10];
	vector<int> *pVecIndex = new vector<int>[10];

	Mat hsvImg;
	cvtColor(m_OriImg, hsvImg, CV_BGR2HSV);
	
	//�Ա��ͶȽ�������;
	QuantifySat_Val(pBin, pVecIndex,true);
	for (int i = 0; i < 10; i++)
	{
		vector<int> vec = pVecIndex[i];
		int nAvgSat = int(pBin[i].dSum / pBin[i].nCount);
		for (int kk = 0; kk < vec.size(); kk++)
		{
			int index = vec[kk];
			Vec3b v = m_pHsvData[index];
			v[1] = nAvgSat;  //��ÿ��bin�����Ԫ�ص�s��������Ϊ��bin�ﱥ�Ͷȵ�ƽ��ֵ;

			int r = index / m_nW;
			int c = index % m_nW;

			m_pHsvData[index] = v;
			hsvImg.at<Vec3b>(r, c) = v;
		}
	}

	memset(pBin, 0, sizeof(struBin));
	for (int i = 0; i < 10; i++)
		pVecIndex[i].clear();
	
	//�����Ƚ�������;
	QuantifySat_Val(pBin, pVecIndex, false);
	for (int i = 0; i < 10; i++)
	{
		vector<int> vec = pVecIndex[i];
		int nAvgSat = int(pBin[i].dSum / pBin[i].nCount);
		for (int kk = 0; kk < vec.size(); kk++)
		{
			int index = vec[kk];
			Vec3b v = m_pHsvData[index];
			v[2] = nAvgSat;  //��ÿ��bin�����Ԫ�ص�s��������Ϊ��bin�ﱥ�Ͷȵ�ƽ��ֵ;

			int r = index / m_nW;
			int c = index % m_nW;

			m_pHsvData[index] = v;
			hsvImg.at<Vec3b>(r, c) = v;
		}
	}

	Mat m1,m1g;
	cvtColor(hsvImg, m1, CV_HSV2BGR);
	imwrite("d:\\quan1.jpg", m1);

	//namedWindow("new1");
	//imshow("new1", hsvImg);

	/////////////////////////ɫ�ȵ�����////////////////////////
	struBin * pHueBin = new struBin[8];
	vector<int> *pVecHueIndex = new vector<int>[8];
	QuantifyHue(pHueBin, pVecHueIndex);
	for (int i = 0; i < 8; i++)
	{
		vector<int> vec = pVecHueIndex[i];
		int nAvgHue = int(pHueBin[i].dSum / pHueBin[i].nCount);
		for (int kk = 0; kk < vec.size(); kk++)
		{
			int index = vec[kk];
			Vec3b v = m_pHsvData[index];
			v[0] = nAvgHue;  //��ÿ��bin�����Ԫ�ص�h��������Ϊ��bin�ﱥ�Ͷȵ�ƽ��ֵ;

			int r = index / m_nW;
			int c = index % m_nW;
			hsvImg.at<Vec3b>(r, c) = v;
		}
	}

	Mat m2,m2g;
	cvtColor(hsvImg, m2, CV_HSV2BGR);
	imwrite("d:\\quan2.jpg", m2);
	resultImg = m2.clone();

	//namedWindow("new2");
	//imshow("new2", hsvImg);

	delete[] pBin;
	for (int i = 0; i < 10; i++)
		pVecIndex[i].clear();
	delete[] pVecIndex;
	
	delete[] pHueBin;
	for (int i = 0; i < 8; i++)
		pVecHueIndex[i].clear();
	delete[] pVecHueIndex;
}
*/

//��������ͬһ��������е㡣������nIndex��İ���ͨ���������,���������vֵ���в���;
//nIndex: Ҫ����ĵ������;  
//pVisitMap:���ֲ��ҹ������õ��ķ��ʺۼ����; 
//vecConn:���ص�ͬ�����ǵĵ�����꼯;
bool CPaintImgDealer::DealConnection_FindSameReg(int nIndex, bool * pVisitMap, vector<int> &vecConn, vector<int> &vecNeib) {

	int nRow, nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;

	int nCurIndex = m_pIndexMap[nIndex];

	for (int r = -1; r <= 1; r++)
		for (int c = -1; c <= 1; c++)
		{
			if (r == 0 && c == 0) continue; //�Լ�������;

			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;

			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {

				//������ͬһ�������;
				if (m_pIndexMap[nNewInd] != nCurIndex)
				{
					//��¼���������indexmapֵ;
					vecNeib.push_back(m_pIndexMap[nNewInd]);
					continue;  
				}
				//�ѷ��ʹ��ģ����ٷ���,�������ѭ��;
				if (pVisitMap[nNewInd]) 
					continue; 

				vecConn.push_back(nNewInd);
				pVisitMap[nNewInd] = true;

			}
		}

	return true;
}


//������nIndex��İ���ͨ���������,���������vֵ���в���;
//nIndex: Ҫ����ĵ������;  v: ����vֵ�����������صĲ���;
//pVisitMap:���ֲ��ҹ������õ��ķ��ʺۼ����; 
//vecConn:���ص������ڷ��������ĵ�����;
//vecSum: ���ص������ڵ����е��RGB�ͣ�������һ���ľ�ֵ����;
bool CPaintImgDealer::DealConnection(int nIndex, Vec3b v, bool * pVisitMap, vector<int> &vecConn, Vec3i &vecSum) {

	int nRow, nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;

	vecSum[0] = 0;
	vecSum[1] = 0;
	vecSum[2] = 0;

	for (int r = -1; r <= 1; r++)
		for (int c = -1; c <= 1; c++)
		{
			if (r == 0 && c == 0) continue; //�Լ�������;

			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;

			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {

				if (m_pIndexMap[nNewInd])  continue;  //�Ѿ���ȫ��������ʹ��ģ����ٷ���,�������ѭ��;
				if (pVisitMap[nNewInd]) continue;  //�������Ӳ��ҹ������ѷ��ʹ��ģ����ٷ���,�������ѭ��;

				Vec3b v1 = m_pData[nNewInd];
				if (IsEqual(v, v1)) {
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

Vec3b  CPaintImgDealer::CalcAvgValue(int nIndex) {
	vector<int> vecLoc;

	for (int r = -1; r <= 1; r++)
		for (int c = -1; c <= 1; c++)
		{
			if (r == 0 && c == 0) continue; //�Լ�������;

			int nNewInd = c + r*m_nW + nIndex;

			if (nNewInd>0 && nNewInd < m_nW*m_nH) {
				vecLoc.push_back(nNewInd);
			}
		}

	return GetMeanV(vecLoc);
}


//��nIndex��ʼ������ͼ�����ͨ�����е�����;�ҵ���ѹջ������ͣ������ѭ�����ҡ�
//�������ҵ����ӵľ�ֵ������ʣ�������;
/*
nIndex: �������ڵ�λ�ã������ĸ��ط���ʼ��;
vecLoc: ��¼�����к����Ӿ�����ֵͬ�����ص�����;
���أ�  ���ȷ��������,�򷵻�true������Ϊfalse;
*/
bool CPaintImgDealer::FindSeed_ByAvg(int nIndex, vector<int> &vecLoc) {

	if (!m_pData)
		return false;

	//��ʼ��һ���������Ӳ��ҵķ��ʺۼ���; �ú����ȽϺ�ʱ;
	//memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	vector<int> vecSeedLoc;      //������ֵͬ�����꣬��ջ��ͣ��ѹ�뵯���������ҵ��������ֵ����������;
	Vec3b v = m_pData[nIndex];

	//v = CalcAvgValue(nIndex);

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
		vAvg[i] = int(vSum[i] / vecConn.size() + 0.5);
		vdSum[i] = vSum[i];
	}

	//��һ�������ҵ��İ���ͨ����;
	int ii = vecConn.size();
	for (int i = 0; i < ii; i++)
	{
		vecSeedLoc.push_back(vecConn[i]);
		m_bVisit[vecConn[i]] = true;
	}

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
		DealConnection(ind, vAvg, m_bVisit, vecConn, vSum);
		int k = vecConn.size();
		for (int i = 0; i < k; i++)
			vecSeedLoc.push_back(vecConn[i]);

		//��������ƽ��ֵ;
		/*for (int i = 0; i < 3; i++)
		{
			vdSum[i] += vSum[i];
			vAvg[i] = vdSum[i] / (vecSeedLoc.size() + nTotalNum) + 0.5;  //nTotalNum�Ǳ���������ЩԪ��;
		}*/
	}

	//����ǰ��nIndex����ѹ�����յ�ջ;
	vecLoc.push_back(nIndex);

	//�����ͬ���صĵ㲻����,Ҳ�����ҵ�;
	if (nTotalNum < m_nMinRegNum)
		return false;

	return true;
}


//���������������ƽ��ֵ;
Vec3b CPaintImgDealer::GetMeanV(vector<int> vecLoc) {
	double d1 = 0, d2 = 0, d3 = 0;
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

//bColorReassign��ʾ�Ƿ�Ҫ����ɫ���������������;
void CPaintImgDealer::GenMapImageByRegColor(string strFile, bool bColorReassign) {
	if (strFile.empty())
		return;

	int nRegCnt = 0;
	nRegCnt = CheckRegionStatus();

	Mat m = m_OriImg.clone();
	m.setTo(0);
	for (int i = 0; i < m_nH*m_nW; i++)
	{
		int ind = m_pIndexMap[i];

		Vec3b v(0,0,0);
		//index��1��ʼ��
		for (int j = 0; j < m_vecReg.size(); j++)
		{
			if (ind == m_vecReg[j].nIndex)
			{
				v = m_vecReg[j].v;
				break;
			}
		}

		int nRow, nCol;
		nRow = i / m_nW;
		nCol = i % m_nW;
		m.at<Vec3b>(nRow, nCol) = v;
	}

	if (!m_bUseRGB)
		cvtColor(m, m, CV_HSV2BGR);

	char strNewFile_png[256];
	int nColorSize = m_vecReg.size();
	if (bColorReassign)
		nColorSize = m_nFinalColorNum;
	string file = strFile.substr(0, strFile.length() - 4);
	sprintf_s(strNewFile_png, "%s_%d_%d.png", file.c_str(), nColorSize, nRegCnt);
	imwrite(strNewFile_png, m);
}


//���ڹ����ĵ�n������������ܱߵ������������ͶƱ,ѡ����ӽ���,��������ӽ������������index;
int CPaintImgDealer::FindSimilarSeedIndex(int n, int nRadius) {
	int nRow, nCol;
	nRow = n / m_nW;
	nCol = n % m_nW;

	vector<int> vecIndex;  //�������������index����;
	for (int r = -nRadius; r <= nRadius; r++)
		for (int c = -nRadius; c <= nRadius; c++)
		{
			if (c == 0 && r == 0) continue;
			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;

			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {
				if (m_pIndexMap[nNewInd])
					vecIndex.push_back(m_pIndexMap[nNewInd]);
			}
		}

	//�����ܱ���1������ʱ���ſ�ʼ����;
	if (vecIndex.size() <= 2 )
		return 0;

	//ȥ�������е���ͬindex
	sort(vecIndex.begin(), vecIndex.end());
	vecIndex.erase(unique(vecIndex.begin(), vecIndex.end()), vecIndex.end());

	//Ѱ����ӽ���;
	Vec3b v = m_pData[n];
	int   nMatchInd = 0;
	int s = vecIndex.size();
	for (int i = 0; i < s; i++)
	{
		double MinDist = 255 * 255 * 3 + 1;

		int ind = vecIndex[i];
		Vec3b regV = m_vecReg[ind - 1].v;  //index=1��Ӧvector�е�1��Ԫ��,�����±�0��ʼ;
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
void CPaintImgDealer::DealResidual() {

	int * pTemp = new int[m_nH*m_nW];
	memcpy(pTemp, m_pIndexMap, sizeof(int)*m_nH*m_nW);

	vector<int> vecResidualPixel;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		if (m_pIndexMap[i] == 0)
			vecResidualPixel.push_back(i);
	}

	//����δ�������residualpixel����;
	int nSize = vecResidualPixel.size();
	do 
	{
		for (vector<int>::iterator iter = vecResidualPixel.begin(); iter != vecResidualPixel.end();) {
			int i = *iter;
			//�������Χ�ģ�������ڲ������Ƶ���;
			int index = FindSimilarSeedIndex(i, 1);
			if (index) {
				pTemp[i] = index;
				//�ҵ�ƥ����Ժ��ɾ��;
				iter = vecResidualPixel.erase(iter);
			}
			else
				iter++;
		}

		memcpy(m_pIndexMap, pTemp, sizeof(int)*m_nH*m_nW);
		nSize = vecResidualPixel.size();

	} while (nSize != 0);

	delete[] pTemp;
}

//ɾ�������㣺����ͨ�����ڣ���������ɫ���Լ���ͬ;
//���������������Ϊ������������ͬ;
void CPaintImgDealer::RemoveIsolatedPixel() {

	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int ind = m_pIndexMap[i];

		int iLeftInd = -1;
		int iRightInd = -1;
		int iUpInd = -1;
		int iDwInd = -1;

		//�͵�ǰ���ز�ͬ��ɫ�ģ������vecNeib��;
		vector<int> vecNeib;
		if (i - 1 >= 0){
			iLeftInd = m_pIndexMap[i - 1];
			if (iLeftInd != ind)
				vecNeib.push_back(iLeftInd);
		}
		if (i + 1 < m_nW*m_nH){
			iRightInd = m_pIndexMap[i + 1];
			if (iRightInd != ind)
				vecNeib.push_back(iRightInd);
		}
		if (i - m_nW >= 0){
			iUpInd = m_pIndexMap[i - m_nW];
			if (iUpInd != ind)
				vecNeib.push_back(iUpInd);
		}
		if (i + m_nW < m_nW*m_nH){
			iDwInd = m_pIndexMap[i + m_nW];
			if (iDwInd != ind)
				vecNeib.push_back(iDwInd);
		}

		//���ʣ�µ���ɫ�����,����Ϊ��ǰ����Ϊ������,ֱ����Ϊ��ʣ����ɫһ����ֵ;
		if (vecNeib.size() >= 3) {
			bool bFlag = true;
			int v = vecNeib[0];
			for (int i = 0; i < vecNeib.size(); i++)
			{
				if (vecNeib[i] != v){
					bFlag = false;
					break;
				}
					
			}
			if (bFlag)
				m_pIndexMap[i] = v;
		}
			
	}
}


//��������index=ָ��ֵ������ͼ;
void CPaintImgDealer::GenMapImageByIndex(string strFile, int nIndex, Vec3b v) {
	Mat m = m_OriImg.clone();
	m.setTo(0);
	for (int i = 0; i < m_nH*m_nW; i++)
	{
		int ind = m_pIndexMap[i];

		if (ind != nIndex) continue;

		int nRow, nCol;
		nRow = i / m_nW;
		nCol = i % m_nW;
		m.at<Vec3b>(nRow, nCol) = v;
	}
	imwrite(strFile, m);
}

//����������Ϣ;
//v: ��ǰ���ҵ�������ƽ��ֵ;  vecLoc: ��ǰ�����ӵ����е�����; nNewIndex: ���ҵ������ӵ�������;
//���أ�������µ������򷵻�true,���򷵻�false,������ǰ��������������index���µ�nNewIndex��������;
bool CPaintImgDealer::UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex) {
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
	int s = vecLoc.size();
	for (int i = 0; i < s; i++)
	{
		int k = vecLoc[i];
		m_pIndexMap[k] = nNewIndex;
	}

	return bRet;
}

bool CPaintImgDealer::IsEqual(Vec3b v, Vec3b dst) {

	bool n1 = abs(v[0] - dst[0]) < m_nColorDistThre;
	bool n2 = abs(v[1] - dst[1]) < m_nColorDistThre;
	bool n3 = abs(v[2] - dst[2]) < m_nColorDistThre;

	if (n1 && n2 && n3)
		return true;
	else
		return false;
	
}


//�������е������У���û����ֵͬ�ģ�����У��򷵻ظ������index;
//���û�У������IndexMap;
bool CPaintImgDealer::CheckSameRegion(Vec3b v, int * nInd) {
	for (int i = 0; i < m_vecReg.size(); i++)
	{
		Vec3b v1 = m_vecReg[i].v;
		if (IsEqual(v, v1))
		{
			(*nInd) = m_vecReg[i].nIndex;
			return true;
		}
	}

	return false;
}


//��vecConn������и�ʴ;
void CPaintImgDealer::IndexMapErosion(vector<int> &vecConn) {
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
				if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {
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
void CPaintImgDealer::IndexMapDilate(vector<int> &vecConn) {
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
				if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {
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


//����ֻ�б߽��ߵ�ͼ��;bWhiteBG��ʾ�߽�ͼ�����ǰ�ɫ��;bThickBd��ʾ�߽��Ǵֵģ�3������.�����ϸ�ģ����ǵ����ص�;
void CPaintImgDealer::GenBorderImg(string strFile,bool bWhiteBG,bool bThickBd) {
	if (strFile.empty())
		return;

	Mat m = m_OriImg.clone();
	m.setTo(255);
	Vec3b v(0, 0, 0);

	//�ڵ�;
	if (!bWhiteBG) {
		m.setTo(0);
		v = Vec3b(255, 255, 255);
	}

	int nRow, nCol;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		nRow = i / m_nW;
		nCol = i % m_nW;
		int nCurInd = m_pIndexMap[i];

		//�ұ�;
		if (nCol + 1 < m_nW) {
			if (m_pIndexMap[i + 1] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}

		//�±�;
		if (nRow + 1 < m_nH) {
			if (m_pIndexMap[i + m_nW] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}
	}

	if (bThickBd){
		Mat kern = getStructuringElement(MORPH_RECT, Size(3, 3));
		Mat dilate_img;
		if (bWhiteBG) {
			bitwise_not(m, m);
			//����
			dilate(m, m, kern);
			bitwise_not(m, m);
		}
		else
			//����
			dilate(m, m, kern);
	}

	imwrite(strFile, m);
}

//����Ϊ��Ҫ���ս���ɫ������ָ��������;
bool  CPaintImgDealer::ColorReassign(int nNewColorNum) {
	int nColorSize = m_vecReg.size();
	if (nColorSize <= nNewColorNum)
		return false;

	Mat data(nColorSize,3,CV_8UC1);
	Mat hsvdata;
	for (int i = 0; i < nColorSize; i++) {
		Vec3b point = m_vecReg[i].v;
		for (int j = 0; j < 3; j++)
			data.at<uchar>(i, j) = point[j];
	}
	
	data.convertTo(data, CV_32FC1);
	
	const cv::TermCriteria term_criteria = cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 0.01);
	cv::Mat labels, centers;

	kmeans(data, nNewColorNum, labels, term_criteria, 100, cv::KMEANS_RANDOM_CENTERS, centers);

	map<int, int> indexMap;   //����ǰ��index��Ӧ��ϵ;
	centers.convertTo(centers, CV_8UC1);
	for (int i = 0; i < nColorSize; i++)
	{
		int clusterIdx = labels.at<int>(i);

		m_vecReg[i].v = centers.at<Vec3b>(clusterIdx);
		//m_vecReg�е�index�Ǵ�1��ʼ��;0��ʾ����ɫ;
		m_vecReg[i].nIndex = clusterIdx + 1;

		//ԭ����index i��Ӧ�µ�clusterIdx;
		indexMap.insert(make_pair(i, clusterIdx + 1));
	}

	//��ÿ�����ص�index ������;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int ind = m_pIndexMap[i];
		m_pIndexMap[i] = indexMap[ind - 1];
	}

	return true;
}

//��������״̬������ÿ������ĸ���,�ܵ��������ȵ�;
//����С��ָ����������򣬽��кϲ�;
int CPaintImgDealer::CheckRegionStatus() {

	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	int nRegionCount = 0;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		if(m_bVisit[i])
			continue;

		int nCurIndex = m_pIndexMap[i];
		nRegionCount++;
		
		//��ʱ����;
		vector<int> vecTemp;
		vector<int> vecRegIndex;   //ͬһ����������е����꼯��;
		vector<int> vecNeibIndex;  //��������Χ���ڵ�mapindexֵ;
		vecTemp.push_back(i);      //���Լ�ѹ��;

		int nTotalNum = 0;
		while (!vecTemp.empty())
		{
			//����һ��������Ѱ��;
			int j = vecTemp.back();
			vecTemp.pop_back();

			vecRegIndex.push_back(j);

			DealConnection_FindSameReg(j, m_bVisit, vecTemp,vecNeibIndex);
		}

		//С��ָ������������;
		if (vecRegIndex.size() < m_nMinRegNum) {

			//ȥ�������е���ͬindex
			sort(vecNeibIndex.begin(), vecNeibIndex.end());
			vecNeibIndex.erase(unique(vecNeibIndex.begin(), vecNeibIndex.end()), vecNeibIndex.end());

			//������ɫ�������;
			Vec3b curV = m_vecReg[nCurIndex].v;
			double dMinDist = 1e+6;
			int   nNewIndex = nCurIndex;  //Ĭ���������;
			for (int m = 0; m < vecNeibIndex.size(); m++)
			{
				Vec3b t = m_vecReg[vecNeibIndex[m]].v;
				double dTemp = (curV[0] - t[0]) * (curV[0] - t[0]);
				dTemp += (curV[1] - t[1]) * (curV[1] - t[1]);
				dTemp += (curV[2] - t[2]) * (curV[2] - t[2]);

				if (dTemp < dMinDist) {
					dMinDist = dTemp;
					nNewIndex = vecNeibIndex[m];
				}
			}

			for (int ii = 0; ii < vecRegIndex.size(); ii++)
			{
				//���;
				m_pIndexMap[vecRegIndex[ii]] = nNewIndex;
			}
		}

		vecNeibIndex.clear();
	}

	return nRegionCount;
}

//param��������������Ľṹ��;
void  CPaintImgDealer::MainProc(struInParam param) {
	//ʹ�õ���ɫ�ռ�;
	if (param.bRGBData)
		m_pData = m_pBgrData;
	else
		m_pData = m_pHsvData;

	m_bUseRGB = param.bRGBData;
	m_nColorDistThre = param.nColorThre;
	m_nMinRegNum = param.nMinAreaThre;
	m_nFinalColorNum = param.nFinalClrNum;

	vector<int> vecLoc;
	int nCurIndex = 1;
	int nSize = m_nW * m_nH;

	for (int i = 0; i < nSize; i++)
	{
		//����...
		float f = i * 1.0 / nSize;
		(*(param.nProgress)) = int(f * 100 + 0.5);

		//�Ѿ����ʹ���,���ٷ���;
		if (m_pIndexMap[i]) continue;

		Vec3b v = m_pData[i];

		vecLoc.clear();
		if (FindSeed_ByAvg(i, vecLoc))
		{
			//close����;
			IndexMapDilate(vecLoc);
			IndexMapErosion(vecLoc);

			v = GetMeanV(vecLoc);
			printf("Find Seed! Index=%d,Num=%d, Value=%d,%d,%d \n", nCurIndex, vecLoc.size(), v[2], v[1], v[0]);

			//����������Ϣ;�����ǰ���µ����������index;
			//���򣬷�������������Ƶ�����;
			int k = nCurIndex;
			if (UpdateRegionInfo(v, vecLoc, k))
				nCurIndex++;

			//����index map;
			if (ENABLE_GENERATE_INDEXMAP) {
				char chFile[256] = { 0 };
				sprintf_s(chFile, "index_%d.jpg", k);
				string strFile(chFile);
				GenMapImageByIndex(strFile, k, Vec3b(255, 255, 255));
			}
		}

		//���»ָ����ʼ�¼��״̬,����m_bVisit��������һ�η���ǰ����0;
		int s = vecLoc.size();
		for (int j = s - 1; j >= 0; j--)
			m_bVisit[vecLoc[j]] = false;
	}

	//GenMapImageByIndex("residual.jpg", 0, Vec3b(255, 255, 255));
	//GenMapImageByRegColor("regions.jpg");

	//����δ����residual�����ͼ;
	string residualFile;
	int len = param.strBorderFile.length();
	residualFile = param.strBorderFile.substr(0, len - 4);
	residualFile = residualFile + "_residual.jpg";
	//GenMapImageByIndex(residualFile, 0, Vec3b(255, 255, 255));
	//����δ�й��������;
	DealResidual();
	//ɾ���������ص�;
	RemoveIsolatedPixel();

	GenMapImageByRegColor(param.strColorFile);

	//��ɫ������ѹ��;ѹ������������;
	if (param.nFinalClrNum > 0){
		bool bRet = ColorReassign(param.nFinalClrNum);
		GenMapImageByRegColor(param.strColorFile,bRet);
	}

	GenBorderImg(param.strBorderFile,param.bWhiteBG,param.bThickBd);
}