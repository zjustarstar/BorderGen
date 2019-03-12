#include "stdafx.h"
#include "ImgQuantify.h"  

#define ENABLE_GENERATE_INDEXMAP    0   //是否生成indexmap

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

//在vecConn区域进行腐蚀;
void CImgQuantify::IndexMapErosion(vector<int> &vecConn){
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
void CImgQuantify::IndexMapDilate(vector<int> &vecConn){
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

//仅处理nIndex点的八连通区域的数据;基于nIndex位置的元素进行查找;
//nIndex: 要处理的点的坐标; 
//pVisitMap:本轮查找过程中用到的访问痕迹标记; 
//vecConn:返回的区域内符合条件的点坐标;
//b8: 默认八连通领域的处理.false则为4联通领域;
bool CImgQuantify::DealConnection(int nIndex, bool * pVisitMap, vector<int> &vecConn, bool b8){

	int nRow, nCol;
	nRow = nIndex / m_nW;
	nCol = nIndex % m_nW;

	Vec3b v = m_pData[nIndex];
	for (int r = -1; r <= 1; r++)
	for (int c = -1; c <= 1; c++)
	{
		if (r == 0 && c == 0) continue; //自己不加入;

		int nNewR = nRow + r;
		int nNewC = nCol + c;
		int nNewInd = nNewC + nNewR*m_nW;

		if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){

			if (m_pIndexMap[nNewInd])  continue;  //已经在全局区域访问过的，不再访问,否则会死循环;
			if (pVisitMap[nNewInd]) continue;  //本轮种子查找过程中已访问过的，不再访问,否则会死循环;

			Vec3b v1 = m_pData[nNewInd];
			if (IsEqual(v, v1)){
				vecConn.push_back(nNewInd);
				pVisitMap[nNewInd] = true;
			}
		}
	}

	return vecConn.size();
}

//仅处理nIndex点的八连通区域的数据,基于输入的v值进行查找;
//nIndex: 要处理的点的坐标;  v: 基于v值进行相似像素的查找;
//pVisitMap:本轮查找过程中用到的访问痕迹标记; 
//vecConn:返回的区域内符合条件的点坐标;
//vecSum: 返回的区域内的所有点的RGB和，用于下一步的均值计算;
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
		if (r==0 && c==0) continue; //自己不加入;
		//if (r!=0 && c!=0) continue; //四联通;

		int nNewR = nRow + r;
		int nNewC = nCol + c;
		int nNewInd = nNewC + nNewR*m_nW;

		if (nNewR >= 0 && nNewR < m_nH && nNewC >= 0 && nNewC < m_nW){

			if (m_pIndexMap[nNewInd])  continue;  //已经在全局区域访问过的，不再访问,否则会死循环;
			if (pVisitMap[nNewInd]) continue;  //本轮种子查找过程中已访问过的，不再访问,否则会死循环;

			Vec3b v1 = m_pData[nNewInd];
			if (IsEqual(v, v1)){
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

//从nIndex开始，查找图像八连通区域中的种子;找到后压栈，并不停弹出再循环查找。
//基于当前中心点像素值，查找其它种子;
/*
nIndex: 像素所在的位置，即从哪个地方开始找;
vecLoc: 记录了所有和种子均有相同值得像素的坐标;
b8:是否八连通区域查找;
返回：  如果确认是种子,则返回true，否则为false;
*/
bool CImgQuantify::FindSeed_ByCenter(int nIndex, vector<int> &vecLoc, bool b8)
{
	if (!m_pData)
		return false;

	//初始化一个本轮种子查找的访问痕迹表;
	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);

	vector<int> vecSeedLoc;      //保存相同值的坐标，该栈不停的压入弹出，用于找到所有相近值的种子像素;
	Vec3b v = m_pData[nIndex];

	vector<int> vecConn;  //联通区域的处理;
	//如果不能在联通区域找到相同的点,直接返回;
	if (!DealConnection(nIndex, m_bVisit, vecConn, b8))
		return false;

	for (int i = 0; i < vecConn.size(); i++)
		vecSeedLoc.push_back(vecConn[i]);

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
		DealConnection(ind, m_bVisit, vecConn, b8);
		for (int i = 0; i < vecConn.size(); i++)
			vecSeedLoc.push_back(vecConn[i]);
	}

	//将当前的nIndex像素压入最终的栈;
	vecLoc.push_back(nIndex);

	//如果相同像素的点不够多,也不算找到;
	if (nTotalNum < REG_AREA_THRE)
		return false;

	return true;
}

//从nIndex开始，查找图像八连通区域中的种子;找到后压栈，并不停弹出再循环查找。
//基于已找到种子的均值，查找剩余的种子;
/*
   nIndex: 像素所在的位置，即从哪个地方开始找;
   vecLoc: 记录了所有和种子均有相同值得像素的坐标;
   返回：  如果确认是种子,则返回true，否则为false;
*/
bool CImgQuantify::FindSeed_ByAvg(int nIndex, vector<int> &vecLoc){

	if (!m_pData)
		return false;

	//初始化一个本轮种子查找的访问痕迹表;
	memset(m_bVisit, 0, sizeof(bool)*m_nH*m_nW);
	
	vector<int> vecSeedLoc;      //保存相同值的坐标，该栈不停的压入弹出，用于找到所有相近值的种子像素;
	Vec3b v = m_pData[nIndex];

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
		vAvg[i]  = int(vSum[i] / vecConn.size() + 0.5);
		vdSum[i] = vSum[i];  
	}
	
	for (int i = 0; i < vecConn.size(); i++)
		vecSeedLoc.push_back(vecConn[i]);

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
		DealConnection(ind,vAvg, m_bVisit, vecConn,vSum);
		for (int i = 0; i < vecConn.size(); i++)
			vecSeedLoc.push_back(vecConn[i]);

		//更新种子平均值;
		for (int i = 0; i < 3; i++)
		{
			vdSum[i] += vSum[i];
			vAvg[i] = vdSum[i] / (vecSeedLoc.size() + nTotalNum) + 0.5;  //nTotalNum是被弹出的那些元素;
		}
	}

	//将当前的nIndex像素压入最终的栈;
	vecLoc.push_back(nIndex);

	//如果相同像素的点不够多,也不算找到;
	if (nTotalNum < REG_AREA_THRE)
		return false;

	return true;
}

//看看已有的区域中，有没有相同值的，如果有，则返回该区域的index;
//如果没有，则更新IndexMap;
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

//更新区域信息;
//v: 当前新找到的区域平均值;  vecLoc: 当前该种子的所有点坐标; nNewIndex: 新找到的种子的区域编号;
//返回：如果是新的区域，则返回true,否则返回false,并将当前种子区域所属的index更新到nNewIndex变量返回;
bool CImgQuantify::UpdateRegionInfo(Vec3b v, vector<int> vecLoc, int &nNewIndex){
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
	for (int i = 0; i < vecLoc.size(); i++)
	{
		int k = vecLoc[i];
		m_pIndexMap[k] = nNewIndex;
	}

	return bRet;
}

//生成所有index=指定值得区域图;
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

//根据区域坐标，生成一张图;
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

//对于孤立的点n，根据这个点周边的种子区域进行投票,选择最接近的,并返回最接近的种子区域的index;
int CImgQuantify::FindSimilarSeedIndex(int n,int nRadius){
	int nRow, nCol;
	nRow = n / m_nW;
	nCol = n % m_nW;
	
	vector<int> vecIndex;  //已有种子区域的index集合;
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

	//去除集合中的相同index
	sort(vecIndex.begin(), vecIndex.end());
	vecIndex.erase(unique(vecIndex.begin(), vecIndex.end()), vecIndex.end());

	//寻找最接近的;
	Vec3b v = m_pData[n];
	int   nMatchInd = 0;
	for (int i = 0; i < vecIndex.size(); i++)
	{
		double MinDist = 255*255*3+1;
	
		int ind = vecIndex[i];
		Vec3b regV = m_vecReg[ind-1].v;  //index=1对应vector中第1个元素,但是下标0开始;
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
void CImgQuantify::DealResidual(){

	int * pTemp = new int[m_nH*m_nW];
	memcpy(pTemp, m_pIndexMap, sizeof(int)*m_nH*m_nW);

	bool bStillNoMatch = false;
	for (int i = 0; i < m_nW * m_nH; i++)
	{
		int index = m_pIndexMap[i];

		if (index != 0)
			continue;

		//表示孤立的点，找最相近的种子;但是本次处理结果由于使用了pTemp，不影响后续元素的处理
		index = FindSimilarSeedIndex(i,2);
		if (index == 0)
			bStillNoMatch = true;
		pTemp[i] = index;
	}

	memcpy(m_pIndexMap, pTemp,sizeof(int)*m_nH*m_nW);

	//还有孤立的点，再次处理.这次处理，受前次处理结果的影响，但是能保证所有点都被处理完;
	if (bStillNoMatch)
	{
		for (int i = 0; i < m_nW * m_nH; i++)
		{
			int index = m_pIndexMap[i];

			if (index != 0)
				continue;

			//表示孤立的点，找最相近的种子,此次窗口为1;
			index = FindSimilarSeedIndex(i, 1);
			m_pIndexMap[i] = index;
		}
	}

	delete[] pTemp;
}

//生成只有边界线的图像;
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

		//右边;
		if (nCol + 1 < m_nW){
			if (m_pIndexMap[i+1] != nCurInd)
				m.at<Vec3b>(nRow, nCol) = v;
		}

		//下边;
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
		
		//找该index对应的区域的值;
		for (int j = 0; j < m_vecReg.size(); j++)
		{
			if (index == m_vecReg[j].nIndex)
			{
				v = m_vecReg[j].v;
				break;
			}
		}

		//改变下颜色;
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

//获得种子区域像素平均值;
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

//将边界图保存在strBorderFile文件中,nProgess表示进度;
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
			//close操作;
			IndexMapDilate(vecLoc);
			IndexMapErosion(vecLoc);

			v = GetMeanV(vecLoc);
			printf("Find Seed! Index=%d,Num=%d, Value=%d,%d,%d \n", nCurIndex, vecLoc.size(), v[2], v[1], v[0]);			

			//填写访问记录;
			for (int j = 0; j < vecLoc.size(); j++)
				m_bVisit[vecLoc[j]] = true;
			
			//更新区域信息;如果当前是新的区域，则更新index;
			//否则，返回与该区域类似的区域;
			int k = nCurIndex;
			if (UpdateRegionInfo(v, vecLoc, k))
				nCurIndex++;

			//生成index map;
			if (ENABLE_GENERATE_INDEXMAP){
				char chFile[256] = { 0 };
				sprintf_s(chFile, "index_%d.jpg", k);
				string strFile(chFile);
				GenMapImageByIndex(strFile, k, Vec3b(255, 255, 255));
			}
		}
	}

	//保存未处理residual区域的图;
	//GenMapImageByIndex("residual.jpg", 0, Vec3b(255, 255, 255));
	//处理未有归属区域的;
	DealResidual();

	printf("*****************************regin info***************************\n");
	for (int i = 0; i < m_vecReg.size(); i++)
	{
		Vec3b v = m_vecReg[i].v;
		printf("%d---%d,%d,%d \n", m_vecReg[i].nIndex, v[2], v[1], v[0]);

		//生成处理完未归属区域的点以后的最终区域图;
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
