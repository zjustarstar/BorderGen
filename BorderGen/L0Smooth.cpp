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

// �����ѭ��ƽ��
void CL0Smooth::circshift(cv::Mat &A, int shift_row, int shift_col)
{
	int row = A.rows, col = A.cols;

	// ���Ǹ����ͳ����߽�ֵ�����
	shift_row = (row + (shift_row % row)) % row;
	shift_col = (col + (shift_col % col)) % col;
	//printf("shift_row = %d, shift_col = %d\n", shift_row, shift_col);

	cv::Mat temp = A.clone();

	if (shift_row)
	{
		// temp[row - shift_row, row][*] -> A[0, shift_row][*]
		temp.rowRange(row - shift_row, row).copyTo(A.rowRange(0, shift_row));	// ���� row - shift_row �����ؾ��루�ƶ� shift_row �����أ�
																				// temp[0, row - shift_row][*] -> A[shift_row, row][*]
		temp.rowRange(0, row - shift_row).copyTo(A.rowRange(shift_row, row));	// ��ԭ�����ϲ���row - shift_row �����أ����� shift_row �����ؾ���
	}

	if (shift_col)
	{
		temp.colRange(col - shift_col, col).copyTo(A.colRange(0, shift_col));	// ����
		temp.colRange(0, col - shift_col).copyTo(A.colRange(shift_col, col));	// ��ԭ����������
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
%   PSF2OTF ������չ����ת��Ϊ�⴫�亯����OTF = PSF2OTF(PSF) �������չ���� (PSF) ����Ŀ��ٸ���Ҷ�任 (FFT)��
%	�������� PSF ƫ������Ӱ��Ĺ⴫�亯��(OTF)���С�Ĭ������£�OTF ����Ĵ�С�� PSF ������ͬ��
%   OTF = PSF2OTF(PSF,OUTSIZE) converts the PSF array into an OTF array of
%   specified size OUTSIZE. The OUTSIZE cannot be smaller than the PSF
%   array size in any dimension.
%
%	OTF = PSF2OTF(PSF,OUTSIZE )�� PSF ����ת��Ϊָ�� OUTSIZE ��С�� OTF ���顣���κ�ά���ϣ� OUTSIZE ������С�� PSF ����Ĵ�С��
%   To ensure that the OTF is not altered due to PSF off-centering, PSF2OTF
%   post-pads the PSF array (down or to the right) with zeros to match
%   dimensions specified in OUTSIZE, then circularly shifts the values of
%   the PSF array up (or to the left) until the central pixel reaches (1,1)
%   position.
%	ȷ�� OTF ������Ϊ PSF ��ƫ���������ı䣬PSF2OTF ���� PSF ���������� 0 (���»�����)��ƥ��ָ���� OUTSIZE �Ĵ�С��
%	Ȼ��ѭ����λ PSF �����ֵ(���������)ֱ���������شﵽ (1,1) ��λ�á�
%   Note that this function is used in image convolution/deconvolution
%   when the operations involve the FFT.
%
%   ע�⣬���漰 FFT ����ʱ���˺�������ͼ����/ȥ�����
*/
cv::Mat CL0Smooth::psf2otf(const cv::Mat &psf, const cv::Size &outSize)
{
	cv::Size psfSize = psf.size();
	cv::Mat new_psf = cv::Mat(outSize, CV_64FC2);// outSize ָ���Ĵ�С������������ͨ���� psf ֻ��һ��ͨ��
	new_psf.setTo(0);

	// ֻ��ֵ����һ��ͨ��
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

	// ѭ��ƽ�ƾ��󣨽����������ƶ������Ͻǣ�
	circshift(new_psf, -1 * int(floor(psfSize.height * .5f)), -1 * int(floor(psfSize.width * .5f)));	// (0, -1) (-1, 0)

	cv::Mat otf;
	// ��ɢ����Ҷ�任��ָ�����������ʽ��Ĭ����CCS��ʽ��
	cv::dft(new_psf, otf, cv::DFT_COMPLEX_OUTPUT);

	return otf;
}

cv::Mat CL0Smooth::L0Smoothing(cv::Mat &im8uc3, double lambda, double kappa) {
	// convert the image to double format
	int row = im8uc3.rows, col = im8uc3.cols;
	cv::Mat S;
	im8uc3.convertTo(S, CV_64FC3, 1. / 255.);
	// 2*2 �ľ����
	cv::Mat fx(1, 2, CV_64FC1);
	cv::Mat fy(2, 1, CV_64FC1);
	fx.at<double>(0) = 1; fx.at<double>(1) = -1;
	fy.at<double>(0) = 1; fy.at<double>(1) = -1;

	// ��һ���ռ����ɢ����ת��ΪƵ����Ĺ�ѧ���ݺ���
	cv::Size sizeI2D = im8uc3.size();
	cv::Mat otfFx = psf2otf(fx, sizeI2D);
	cv::Mat otfFy = psf2otf(fy, sizeI2D);

	// FNormin1 = fft2(S);
	cv::Mat FNormin1[3];	// ע�⣺DFT�Ժ�FNormalΪ����ͨ��
	cv::Mat single_channel[3];
	cv::split(S, single_channel);	// ����Ϊ����ͨ��
	for (int k = 0; k < 3; k++) {
		// ��ɢ����Ҷ�任��ָ�����������ʽ��Ĭ����CCS��ʽ��
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
		// F(1)+��(F(?x)?F(?x)+F(?y)?F(?y))
		cv::Mat Denormin = 1.0 + beta*Denormin2;

		// h-v subproblem
		// ����ͨ���� ?S/?x ?S/?y
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

				// (?S/?x)^2 + (?S/?y)^2  < ��/��
				if (val < lambda / beta) {
					dx[0].at<double>(i, j) = dx[1].at<double>(i, j) = dx[2].at<double>(i, j) = 0.0;
					dy[0].at<double>(i, j) = dy[1].at<double>(i, j) = dy[2].at<double>(i, j) = 0.0;
				}
			}
		}

		// S subproblem
		for (int k = 0; k < 3; k++) {
			// ���׵�
			cv::Mat shift_dx = dx[k].clone();
			circshift(shift_dx, 0, 1);
			cv::Mat ddx = shift_dx - dx[k];

			cv::Mat shift_dy = dy[k].clone();
			circshift(shift_dy, 1, 0);
			cv::Mat ddy = shift_dy - dy[k];

			cv::Mat Normin2 = ddx + ddy;
			cv::Mat FNormin2;
			// ��ɢ����Ҷ�任��ָ�����������ʽ��Ĭ����CCS��ʽ��
			cv::dft(Normin2, FNormin2, cv::DFT_COMPLEX_OUTPUT);

			// F(g)+��(F(?x)?F(h)+F(?y)?F(v))
			cv::Mat FS = FNormin1[k] + beta*FNormin2;

			// ���ĵĹ�ʽ(8)��F^-1�����е�����
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					FS.at<cv::Vec2d>(i, j)[0] /= Denormin.at<double>(i, j);
					FS.at<cv::Vec2d>(i, j)[1] /= Denormin.at<double>(i, j);
					//std::cout<< FS.at<cv::Vec2d>(i, j)[0] / Denormin.at<double>(i, j) <<std::endl;
				}
			}

			// ���ĵĹ�ʽ(8)������Ҷ��任
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
