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

	//平滑预处理;
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

	//速度要求比较快时...
	if (bFastMode)
	{
		Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
		filter2D(img, img, img.depth(), kernel);
	}

	GetImgData(img);
}


/*
 红 bin[0]: 0-10; 
 橙 bin[1]: 11-25;
 黄 bin[2]: 26-34;
 绿 bin[3]: 37-77;
 青 bin[4]: 78-99;
 蓝 bin[5]: 100-124;
 紫 bin[6]: 125-155;
 红 bin[7]: 156-180;
*/
void CPaintImgDealer::QuantifyHue(struBin * pBin, vector<int> *pVecIndex){
	int nSize = m_nW * m_nH;
	int nHueBin[8] = { 0,10, 25, 34, 77, 99, 124, 155 };

	for (int i = 0; i < nSize; i++)
	{
		Vec3b v = m_pHsvData[i];
		uchar s = v[0];   //色度;

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

//对Saturation和Value两个分量进行同样的量化; bSat=true表示对饱和度量化,false表示对亮度量化; 
void CPaintImgDealer::QuantifySat_Val(struBin * pBin, vector<int> *pVecIndex,bool bSat){
	
	int nSize = m_nW * m_nH;
	for (int i = 0; i < nSize; i++)
	{
		Vec3b v = m_pHsvData[i];
		uchar s = v[1];   //饱和度;
		if (!bSat)
			s = v[2];     //亮度;

		//起始段<=40;
		if (s <= 40)
		{
			pBin[0].nCount++;
			pBin[0].dSum += s;
			pVecIndex[0].push_back(i);
		}
		//最后一段240-255;
		else if (s >= 240)
		{
			pBin[9].nCount++;
			pBin[9].dSum += s;
			pVecIndex[9].push_back(i);
		}
		else
		{
			int k = (s - 40) / 25 + 1;  //每段间隔25;
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

//将RGB数据的后两位去掉;
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
	/// 求 X方向梯度
	Sobel(gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	/// 求 Y方向梯度
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

	//腐蚀二值图：
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
	
	//对饱和度进行量化;
	QuantifySat_Val(pBin, pVecIndex,true);
	for (int i = 0; i < 10; i++)
	{
		vector<int> vec = pVecIndex[i];
		int nAvgSat = int(pBin[i].dSum / pBin[i].nCount);
		for (int kk = 0; kk < vec.size(); kk++)
		{
			int index = vec[kk];
			Vec3b v = m_pHsvData[index];
			v[1] = nAvgSat;  //将每个bin里面的元素的s分量，改为该bin里饱和度的平均值;

			int r = index / m_nW;
			int c = index % m_nW;

			m_pHsvData[index] = v;
			hsvImg.at<Vec3b>(r, c) = v;
		}
	}

	memset(pBin, 0, sizeof(struBin));
	for (int i = 0; i < 10; i++)
		pVecIndex[i].clear();
	
	//对亮度进行量化;
	QuantifySat_Val(pBin, pVecIndex, false);
	for (int i = 0; i < 10; i++)
	{
		vector<int> vec = pVecIndex[i];
		int nAvgSat = int(pBin[i].dSum / pBin[i].nCount);
		for (int kk = 0; kk < vec.size(); kk++)
		{
			int index = vec[kk];
			Vec3b v = m_pHsvData[index];
			v[2] = nAvgSat;  //将每个bin里面的元素的s分量，改为该bin里饱和度的平均值;

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

	/////////////////////////色度的量化////////////////////////
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
			v[0] = nAvgHue;  //将每个bin里面的元素的h分量，改为该bin里饱和度的平均值;

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

//查找属于同一区域的所有点。仅处理nIndex点的八连通区域的数据,基于输入的v值进行查找;
//nIndex: 要处理的点的坐标;  
//pVisitMap:本轮查找过程中用到的访问痕迹标记; 
//vecConn:返回的同区域标记的点的坐标集;
bool CPaintImgDealer::DealConnection_FindSameReg(int nIndex, bool * pVisitMap, vector<int> &vecConn, vector<int> &vecNeib) {

	int nRow, nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;

	int nCurIndex = m_pIndexMap[nIndex];

	for (int r = -1; r <= 1; r++)
		for (int c = -1; c <= 1; c++)
		{
			if (r == 0 && c == 0) continue; //自己不加入;

			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;

			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {

				//不属于同一个区域的;
				if (m_pIndexMap[nNewInd] != nCurIndex)
				{
					//记录相邻区域的indexmap值;
					vecNeib.push_back(m_pIndexMap[nNewInd]);
					continue;  
				}
				//已访问过的，不再访问,否则会死循环;
				if (pVisitMap[nNewInd]) 
					continue; 

				vecConn.push_back(nNewInd);
				pVisitMap[nNewInd] = true;

			}
		}

	return true;
}


//仅处理nIndex点的八连通区域的数据,基于输入的v值进行查找;
//nIndex: 要处理的点的坐标;  v: 基于v值进行相似像素的查找;
//pVisitMap:本轮查找过程中用到的访问痕迹标记; 
//vecConn:返回的区域内符合条件的点坐标;
//vecSum: 返回的区域内的所有点的RGB和，用于下一步的均值计算;
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
			if (r == 0 && c == 0) continue; //自己不加入;

			int nNewR = nRow + r;
			int nNewC = nCol + c;
			int nNewInd = nNewC + nNewR*m_nW;

			if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW) {

				if (m_pIndexMap[nNewInd])  continue;  //已经在全局区域访问过的，不再访问,否则会死循环;
				if (pVisitMap[nNewInd]) continue;  //本轮种子查找过程中已访问过的，不再访问,否则会死循环;

				Vec3b v1 = m_pData[nNewInd];
				if (IsEqual(v, v1)) {
					vecConn.push_back(nNewInd);
					pVisitMap[nNewInd] = true;

					//计算所有这些点的RGB总和;
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
			if (r == 0 && c == 0) continue; //自己不加入;

			int nNewInd = c + r*m_nW + nIndex;

			if (nNewInd>0 && nNewInd < m_nW*m_nH) {
				vecLoc.push_back(nNewInd);
			}
		}

	return GetMeanV(vecLoc);
}


//从nIndex开始，查找图像八连通区域中的种子;找到后压栈，并不停弹出再循环查找。
//基于已找到种子的均值，查找剩余的种子;
/*
nIndex: 像素所在的位置，即从哪个地方开始找;
vecLoc: 记录了所有和种子均有相同值得像素的坐标;
返回：  如果确认是种子,则返回true，否则为false;
*/
bool CPaintImgDealer::FindSeed_ByAvg(int nIndex, vector<int> &vecLoc) {

	if (!m_pData)
		return false;

	//初始化一个本轮种子查找的访问痕迹表; 该函数比较耗时;
	//memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	vector<int> vecSeedLoc;      //保存相同值的坐标，该栈不停的压入弹出，用于找到所有相近值的种子像素;
	Vec3b v = m_pData[nIndex];

	//v = CalcAvgValue(nIndex);

	vector<int> vecConn;  //联通区域的处理;
	Vec3i       vSum;     //返回当前某个像素八连通区域RGB值的总和,方便计算vecSeedLoc中所有像素的平均值;
	Vec3b       vAvg;     //当前vecSeedLoc中所有像素的平均值;
	Vec3d       vdSum;    //当前vecSeedLoc中所有像素的RGB值总和;
						  //如果不能在联通区域找到相同的点,直接返回;
	if (!DealConnection(nIndex, v, m_bVisit, vecConn, vSum))
		return false;

	//计算平均值,以及vdSum初始化值;
	for (int i = 0; i < 3; i++)
	{
		vAvg[i] = int(vSum[i] / vecConn.size() + 0.5);
		vdSum[i] = vSum[i];
	}

	//第一个种子找到的八连通区域;
	int ii = vecConn.size();
	for (int i = 0; i < ii; i++)
	{
		vecSeedLoc.push_back(vecConn[i]);
		m_bVisit[vecConn[i]] = true;
	}

	int nTotalNum = 0;
	while (!vecSeedLoc.empty())
	{
		//弹出一个，继续寻找;
		int ind = vecSeedLoc.back();
		vecSeedLoc.pop_back();

		nTotalNum++;
		vecLoc.push_back(ind);   //压入最终的栈;

		//不停找相同的种子;
		vecConn.clear();
		DealConnection(ind, vAvg, m_bVisit, vecConn, vSum);
		int k = vecConn.size();
		for (int i = 0; i < k; i++)
			vecSeedLoc.push_back(vecConn[i]);

		//更新种子平均值;
		/*for (int i = 0; i < 3; i++)
		{
			vdSum[i] += vSum[i];
			vAvg[i] = vdSum[i] / (vecSeedLoc.size() + nTotalNum) + 0.5;  //nTotalNum是被弹出的那些元素;
		}*/
	}

	//将当前的nIndex像素压入最终的栈;
	vecLoc.push_back(nIndex);

	//如果相同像素的点不够多,也不算找到;
	if (nTotalNum < m_nMinRegNum)
		return false;

	return true;
}


//获得种子区域像素平均值;
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

//bColorReassign表示是否要对颜色种类进行重新量化;
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
		//index从1开始。
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


//对于孤立的点n，根据这个点周边的种子区域进行投票,选择最接近的,并返回最接近的种子区域的index;
int CPaintImgDealer::FindSimilarSeedIndex(int n, int nRadius) {
	int nRow, nCol;
	nRow = n / m_nW;
	nCol = n % m_nW;

	vector<int> vecIndex;  //已有种子区域的index集合;
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

	//至少周边有1个像素时，才开始计算;
	if (vecIndex.size() <= 2 )
		return 0;

	//去除集合中的相同index
	sort(vecIndex.begin(), vecIndex.end());
	vecIndex.erase(unique(vecIndex.begin(), vecIndex.end()), vecIndex.end());

	//寻找最接近的;
	Vec3b v = m_pData[n];
	int   nMatchInd = 0;
	int s = vecIndex.size();
	for (int i = 0; i < s; i++)
	{
		double MinDist = 255 * 255 * 3 + 1;

		int ind = vecIndex[i];
		Vec3b regV = m_vecReg[ind - 1].v;  //index=1对应vector中第1个元素,但是下标0开始;
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

//处理所有index=0,未划归区域的
void CPaintImgDealer::DealResidual() {

	int * pTemp = new int[m_nH*m_nW];
	memcpy(pTemp, m_pIndexMap, sizeof(int)*m_nH*m_nW);

	vector<int> vecResidualPixel;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		if (m_pIndexMap[i] == 0)
			vecResidualPixel.push_back(i);
	}

	//所有未处理过的residualpixel集合;
	int nSize = vecResidualPixel.size();
	do 
	{
		for (vector<int>::iterator iter = vecResidualPixel.begin(); iter != vecResidualPixel.end();) {
			int i = *iter;
			//先完成外围的，再完成内部的相似点检测;
			int index = FindSimilarSeedIndex(i, 1);
			if (index) {
				pTemp[i] = index;
				//找到匹配的以后就删除;
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

//删除孤立点：四联通区域内，有三个颜色和自己不同;
//将孤立点的像素设为与其它三个相同;
void CPaintImgDealer::RemoveIsolatedPixel() {

	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int ind = m_pIndexMap[i];

		int iLeftInd = -1;
		int iRightInd = -1;
		int iUpInd = -1;
		int iDwInd = -1;

		//和当前像素不同颜色的，则放入vecNeib中;
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

		//如果剩下的颜色都相等,则认为当前像素为孤立点,直接设为与剩下颜色一样的值;
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


//生成所有index=指定值得区域图;
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

//更新区域信息;
//v: 当前新找到的区域平均值;  vecLoc: 当前该种子的所有点坐标; nNewIndex: 新找到的种子的区域编号;
//返回：如果是新的区域，则返回true,否则返回false,并将当前种子区域所属的index更新到nNewIndex变量返回;
bool CPaintImgDealer::UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex) {
	int  nInd;
	bool bRet = true;

	//已有保存的相同值的种子区域,区域编号用同一个的;
	if (CheckSameRegion(v, &nInd))
	{
		printf("equal to previous region:%d \n", nInd);
		nNewIndex = nInd;
		bRet = false;
	}
	//增加新区域;
	else
	{
		struRegionInfo ri;
		ri.nIndex = nNewIndex;
		ri.v = v;
		m_vecReg.push_back(ri);
	}

	//更新区域信息;
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


//看看已有的区域中，有没有相同值的，如果有，则返回该区域的index;
//如果没有，则更新IndexMap;
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


//在vecConn区域进行腐蚀;
void CPaintImgDealer::IndexMapErosion(vector<int> &vecConn) {
	bool * bObj = new bool[m_nH*m_nW];
	memset(bObj, 0, sizeof(bool)*m_nH*m_nW);

	//腐蚀后的结果保存在vecFinal中.首先拷贝一份原始的;
	vector<int> vecFinal;
	for (int i = 0; i < vecConn.size(); i++)
	{
		bObj[vecConn[i]] = 1;
		vecFinal.push_back(vecConn[i]);
	}

	//每个点进行腐蚀;
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
				if (r == 0 && c == 0) continue; //自己;

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
			vecFinal[i] = -1;  //做个要腐蚀掉的标记;
	}

	vecConn.clear();
	for (int i = 0; i < vecFinal.size(); i++)
	{
		if (vecFinal[i] != -1)
			vecConn.push_back(vecFinal[i]);
	}

	delete[] bObj;
}

//在index区域(保存在vecConn向量中)进行膨胀;
void CPaintImgDealer::IndexMapDilate(vector<int> &vecConn) {
	bool * bObj = new bool[m_nH*m_nW];
	memset(bObj, 0, sizeof(bool)*m_nH*m_nW);

	//膨胀后的结果保存在vecFinal中.首先拷贝一份原始的;
	vector<int> vecFinal;
	for (int i = 0; i < vecConn.size(); i++)
	{
		bObj[vecConn[i]] = 1;
		vecFinal.push_back(vecConn[i]);
	}

	//每个点进行膨胀;
	for (int i = 0; i < vecConn.size(); i++)
	{
		int nInd = vecConn[i];

		int nRow, nCol;
		nRow = nInd / m_nW;
		nCol = nInd % m_nW;
		for (int r = -1; r <= 1; r++)
			for (int c = -1; c <= 1; c++)
			{
				if (r == 0 && c == 0) continue; //自己;

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


//生成只有边界线的图像;bWhiteBG表示边界图背景是白色的;bThickBd表示边界是粗的，3个像素.如果是细的，则是单像素的;
void CPaintImgDealer::GenBorderImg(string strFile,bool bWhiteBG,bool bThickBd) {
	if (strFile.empty())
		return;

	Mat m = m_OriImg.clone();
	m.setTo(255);
	Vec3b v(0, 0, 0);

	//黑底;
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

		//右边;
		if (nCol + 1 < m_nW) {
			if (m_pIndexMap[i + 1] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}

		//下边;
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
			//膨胀
			dilate(m, m, kern);
			bitwise_not(m, m);
		}
		else
			//膨胀
			dilate(m, m, kern);
	}

	imwrite(strFile, m);
}

//输入为需要最终将颜色采样到指定的数量;
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

	map<int, int> indexMap;   //聚类前后index对应关系;
	centers.convertTo(centers, CV_8UC1);
	for (int i = 0; i < nColorSize; i++)
	{
		int clusterIdx = labels.at<int>(i);

		m_vecReg[i].v = centers.at<Vec3b>(clusterIdx);
		//m_vecReg中的index是从1开始的;0表示背景色;
		m_vecReg[i].nIndex = clusterIdx + 1;

		//原来的index i对应新的clusterIdx;
		indexMap.insert(make_pair(i, clusterIdx + 1));
	}

	//将每个像素的index 更新下;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int ind = m_pIndexMap[i];
		m_pIndexMap[i] = indexMap[ind - 1];
	}

	return true;
}

//查验区块状态，包括每个区块的个数,总的区块数等等;
//并将小于指定面积的区域，进行合并;
int CPaintImgDealer::CheckRegionStatus() {

	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	int nRegionCount = 0;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		if(m_bVisit[i])
			continue;

		int nCurIndex = m_pIndexMap[i];
		nRegionCount++;
		
		//临时队列;
		vector<int> vecTemp;
		vector<int> vecRegIndex;   //同一个区域的所有点坐标集合;
		vector<int> vecNeibIndex;  //该区域周围相邻的mapindex值;
		vecTemp.push_back(i);      //将自己压入;

		int nTotalNum = 0;
		while (!vecTemp.empty())
		{
			//弹出一个，继续寻找;
			int j = vecTemp.back();
			vecTemp.pop_back();

			vecRegIndex.push_back(j);

			DealConnection_FindSameReg(j, m_bVisit, vecTemp,vecNeibIndex);
		}

		//小于指定的区域数量;
		if (vecRegIndex.size() < m_nMinRegNum) {

			//去除集合中的相同index
			sort(vecNeibIndex.begin(), vecNeibIndex.end());
			vecNeibIndex.erase(unique(vecNeibIndex.begin(), vecNeibIndex.end()), vecNeibIndex.end());

			//查找颜色最相近的;
			Vec3b curV = m_vecReg[nCurIndex].v;
			double dMinDist = 1e+6;
			int   nNewIndex = nCurIndex;  //默认是最初的;
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
				//变黑;
				m_pIndexMap[vecRegIndex[ii]] = nNewIndex;
			}
		}

		vecNeibIndex.clear();
	}

	return nRegionCount;
}

//param是所有输入参数的结构体;
void  CPaintImgDealer::MainProc(struInParam param) {
	//使用的颜色空间;
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
		//进度...
		float f = i * 1.0 / nSize;
		(*(param.nProgress)) = int(f * 100 + 0.5);

		//已经访问过的,不再访问;
		if (m_pIndexMap[i]) continue;

		Vec3b v = m_pData[i];

		vecLoc.clear();
		if (FindSeed_ByAvg(i, vecLoc))
		{
			//close操作;
			IndexMapDilate(vecLoc);
			IndexMapErosion(vecLoc);

			v = GetMeanV(vecLoc);
			printf("Find Seed! Index=%d,Num=%d, Value=%d,%d,%d \n", nCurIndex, vecLoc.size(), v[2], v[1], v[0]);

			//更新区域信息;如果当前是新的区域，则更新index;
			//否则，返回与该区域类似的区域;
			int k = nCurIndex;
			if (UpdateRegionInfo(v, vecLoc, k))
				nCurIndex++;

			//生成index map;
			if (ENABLE_GENERATE_INDEXMAP) {
				char chFile[256] = { 0 };
				sprintf_s(chFile, "index_%d.jpg", k);
				string strFile(chFile);
				GenMapImageByIndex(strFile, k, Vec3b(255, 255, 255));
			}
		}

		//重新恢复访问记录的状态,保持m_bVisit变量在下一次访问前都置0;
		int s = vecLoc.size();
		for (int j = s - 1; j >= 0; j--)
			m_bVisit[vecLoc[j]] = false;
	}

	//GenMapImageByIndex("residual.jpg", 0, Vec3b(255, 255, 255));
	//GenMapImageByRegColor("regions.jpg");

	//保存未处理residual区域的图;
	string residualFile;
	int len = param.strBorderFile.length();
	residualFile = param.strBorderFile.substr(0, len - 4);
	residualFile = residualFile + "_residual.jpg";
	//GenMapImageByIndex(residualFile, 0, Vec3b(255, 255, 255));
	//处理未有归属区域的;
	DealResidual();
	//删除孤立像素点;
	RemoveIsolatedPixel();

	GenMapImageByRegColor(param.strColorFile);

	//颜色数量的压缩;压缩后重新生成;
	if (param.nFinalClrNum > 0){
		bool bRet = ColorReassign(param.nFinalClrNum);
		GenMapImageByRegColor(param.strColorFile,bRet);
	}

	GenBorderImg(param.strBorderFile,param.bWhiteBG,param.bThickBd);
}