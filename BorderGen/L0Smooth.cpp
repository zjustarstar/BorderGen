#include "stdafx.h"
#include "L0Smooth.h"
#include <math.h>

#define sqr(a)  ((a)*(a))

CL0Smooth::CL0Smooth()
{
}


CL0Smooth::~CL0Smooth()
{
}

// 矩阵的循环平移
void CL0Smooth::circshift(cv::Mat &A, int shift_row, int shift_col)
{
	int row = A.rows, col = A.cols;

	// 考虑负数和超出边界值的情况
	shift_row = (row + (shift_row % row)) % row;
	shift_col = (col + (shift_col % col)) % col;
	//printf("shift_row = %d, shift_col = %d\n", shift_row, shift_col);

	cv::Mat temp = A.clone();

	if (shift_row)
	{
		// temp[row - shift_row, row][*] -> A[0, shift_row][*]
		temp.rowRange(row - shift_row, row).copyTo(A.rowRange(0, shift_row));	// 上移 row - shift_row 个像素距离（移动 shift_row 个像素）
																				// temp[0, row - shift_row][*] -> A[shift_row, row][*]
		temp.rowRange(0, row - shift_row).copyTo(A.rowRange(shift_row, row));	// 将原来的上部（row - shift_row 个像素）下移 shift_row 个像素距离
	}

	if (shift_col)
	{
		temp.colRange(col - shift_col, col).copyTo(A.colRange(0, shift_col));	// 左移
		temp.colRange(0, col - shift_col).copyTo(A.colRange(shift_col, col));	// 将原来的左部右移
	}
	return;
}

/*
%   PSF2OTF Convert point-spread function to optical transfer function.
%   OTF = PSF2OTF(PSF) computes the Fast Fourier Transform (FFT) of the
%   point-spread function (PSF) array and creates the optical transfer
%   function (OTF) array that is not influenced by the PSF off-centering. By
%   default, the OTF array is the same size as the PSF array.
%
%   PSF2OTF 将点扩展函数转换为光传输函数。OTF = PSF2OTF(PSF) 计算点扩展函数 (PSF) 数组的快速傅里叶变换 (FFT)，
%	创建不受 PSF 偏离中心影响的光传输函数(OTF)阵列。默认情况下，OTF 数组的大小与 PSF 数组相同。
%   OTF = PSF2OTF(PSF,OUTSIZE) converts the PSF array into an OTF array of
%   specified size OUTSIZE. The OUTSIZE cannot be smaller than the PSF
%   array size in any dimension.
%
%	OTF = PSF2OTF(PSF,OUTSIZE )将 PSF 数组转换为指定 OUTSIZE 大小的 OTF 数组。在任何维度上， OUTSIZE 都不能小于 PSF 数组的大小。
%   To ensure that the OTF is not altered due to PSF off-centering, PSF2OTF
%   post-pads the PSF array (down or to the right) with zeros to match
%   dimensions specified in OUTSIZE, then circularly shifts the values of
%   the PSF array up (or to the left) until the central pixel reaches (1,1)
%   position.
%	确保 OTF 不会因为 PSF 的偏离中心所改变，PSF2OTF 会向 PSF 数组后向填充 0 (向下或向右)来匹配指定的 OUTSIZE 的大小。
%	然后循环移位 PSF 数组的值(向左或向上)直到中央像素达到 (1,1) 的位置。
%   Note that this function is used in image convolution/deconvolution
%   when the operations involve the FFT.
%
%   注意，当涉及 FFT 操作时，此函数用于图像卷积/去卷积。
*/
cv::Mat CL0Smooth::psf2otf(const cv::Mat &psf, const cv::Size &outSize)
{
	cv::Size psfSize = psf.size();
	cv::Mat new_psf = cv::Mat(outSize, CV_64FC2);// outSize 指定的大小，而且是两个通道， psf 只有一个通道
	new_psf.setTo(0);

	// 只赋值到第一个通道
	for (int i = 0; i < psfSize.height; i++)
	{
		for (int j = 0; j < psfSize.width; j++)
		{
			new_psf.at<cv::Vec2d>(i, j)[0] = psf.at<double>(i, j);	// (0, 0) (0, 1)/(1, 0)
		}
	}

	//  std::cout  << 
	//-1*int(floor(psfSize.height*0.5)) <<
	//", " <<
	//-1*int(floor(psfSize.width*0.5)) << 
	//std::endl;

	// 循环平移矩阵（将矩阵中心移动到左上角）
	circshift(new_psf, -1 * int(floor(psfSize.height * .5f)), -1 * int(floor(psfSize.width * .5f)));	// (0, -1) (-1, 0)

	cv::Mat otf;
	// 离散傅里叶变换，指定输出复数格式（默认是CCS格式）
	cv::dft(new_psf, otf, cv::DFT_COMPLEX_OUTPUT);

	return otf;
}

cv::Mat CL0Smooth::L0Smoothing(cv::Mat &im8uc3, double lambda, double kappa) {
	// convert the image to double format
	int row = im8uc3.rows, col = im8uc3.cols;
	cv::Mat S;
	im8uc3.convertTo(S, CV_64FC3, 1. / 255.);
	// 2*2 的卷积核
	cv::Mat fx(1, 2, CV_64FC1);
	cv::Mat fy(2, 1, CV_64FC1);
	fx.at<double>(0) = 1; fx.at<double>(1) = -1;
	fy.at<double>(0) = 1; fy.at<double>(1) = -1;

	// 把一个空间点扩散函数转换为频谱面的光学传递函数
	cv::Size sizeI2D = im8uc3.size();
	cv::Mat otfFx = psf2otf(fx, sizeI2D);
	cv::Mat otfFy = psf2otf(fy, sizeI2D);

	// FNormin1 = fft2(S);
	cv::Mat FNormin1[3];	// 注意：DFT以后，FNormal为两个通道
	cv::Mat single_channel[3];
	cv::split(S, single_channel);	// 分裂为三个通道
	for (int k = 0; k < 3; k++) {
		// 离散傅里叶变换，指定输出复数格式（默认是CCS格式）
		cv::dft(single_channel[k], FNormin1[k], cv::DFT_COMPLEX_OUTPUT);
	}

	// F(?x)?F(?x)+F(?y)?F(?y);
	cv::Mat Denormin2(row, col, CV_64FC1);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			cv::Vec2d &c1 = otfFx.at<cv::Vec2d>(i, j);
			cv::Vec2d &c2 = otfFy.at<cv::Vec2d>(i, j);
			// 0: Real, 1: Image
			Denormin2.at<double>(i, j) = (c1[0]*c1[0]) + (c1[1]* c1[1]) + (c2[0]* c2[0]) + (c2[1]* c2[1]);
		}
	}

	double beta = 2.0*lambda;
	double betamax = 1e5;

	while (beta < betamax) {
		// F(1)+β(F(?x)?F(?x)+F(?y)?F(?y))
		cv::Mat Denormin = 1.0 + beta*Denormin2;

		// h-v subproblem
		// 三个通道的 ?S/?x ?S/?y
		cv::Mat dx[3], dy[3];
		for (int k = 0; k < 3; k++) {
			cv::Mat shifted_x = single_channel[k].clone();
			circshift(shifted_x, 0, -1);
			dx[k] = shifted_x - single_channel[k];

			cv::Mat shifted_y = single_channel[k].clone();
			circshift(shifted_y, -1, 0);
			dy[k] = shifted_y - single_channel[k];
		}
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				// (?S/?x)^2 + (?S/?y)^2
				double val =
					sqr(dx[0].at<double>(i, j)) + sqr(dy[0].at<double>(i, j)) +
					sqr(dx[1].at<double>(i, j)) + sqr(dy[1].at<double>(i, j)) +
					sqr(dx[2].at<double>(i, j)) + sqr(dy[2].at<double>(i, j));

				// (?S/?x)^2 + (?S/?y)^2  < λ/β
				if (val < lambda / beta) {
					dx[0].at<double>(i, j) = dx[1].at<double>(i, j) = dx[2].at<double>(i, j) = 0.0;
					dy[0].at<double>(i, j) = dy[1].at<double>(i, j) = dy[2].at<double>(i, j) = 0.0;
				}
			}
		}

		// S subproblem
		for (int k = 0; k < 3; k++) {
			// 二阶导
			cv::Mat shift_dx = dx[k].clone();
			circshift(shift_dx, 0, 1);
			cv::Mat ddx = shift_dx - dx[k];

			cv::Mat shift_dy = dy[k].clone();
			circshift(shift_dy, 1, 0);
			cv::Mat ddy = shift_dy - dy[k];

			cv::Mat Normin2 = ddx + ddy;
			cv::Mat FNormin2;
			// 离散傅里叶变换，指定输出复数格式（默认是CCS格式）
			cv::dft(Normin2, FNormin2, cv::DFT_COMPLEX_OUTPUT);

			// F(g)+β(F(?x)?F(h)+F(?y)?F(v))
			cv::Mat FS = FNormin1[k] + beta*FNormin2;

			// 论文的公式(8)：F^-1括号中的内容
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					FS.at<cv::Vec2d>(i, j)[0] /= Denormin.at<double>(i, j);
					FS.at<cv::Vec2d>(i, j)[1] /= Denormin.at<double>(i, j);
					//std::cout<< FS.at<cv::Vec2d>(i, j)[0] / Denormin.at<double>(i, j) <<std::endl;
				}
			}

			// 论文的公式(8)：傅里叶逆变换
			cv::Mat ifft;
			cv::idft(FS, ifft, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					single_channel[k].at<double>(i, j) = ifft.at<cv::Vec2d>(i, j)[0];
					//std::cout<< ifft.at<cv::Vec2d>(i, j)[0] <<std::endl;
				}
			}
		}
		beta *= kappa;
		std::cout << '.';
	}
	for (int i = 0; i < 3; i++)
		single_channel[i].convertTo(single_channel[i], CV_8UC1, 255);
	
	cv::merge(single_channel, 3, S);
	return S;
}
