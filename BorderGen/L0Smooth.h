#pragma once
#include <opencv2\opencv.hpp>
using namespace std;
using namespace cv;

class CL0Smooth
{
public:
	CL0Smooth();
	virtual ~CL0Smooth();
	cv::Mat L0Smoothing(cv::Mat &im8uc3, double lambda = 2e-2, double kappa = 2.0);

private:
	void    circshift(cv::Mat &A, int shift_row, int shift_col);
	cv::Mat psf2otf(const cv::Mat &psf, const cv::Size &outSize);
};

