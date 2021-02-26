/*****************************************************************
*
* fileName:  psf.h
*
* function:  psf(point spread function)点扩散函数计算
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     计算psf点扩散函数

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef PSF_H
#define PSF_H

#include "baseFunc.h"

class Psf {
public:
	float strehl;
	cv::Mat psf;
	cv::Mat energy;
	cv::Mat mtf;

public:
	Psf() {}

	void psfProcess(cv::Mat fitting_rem_zer);

	inline cv::Mat fft2(const cv::Mat& m) {
		CV_Assert(m.type() == CV_32FC2);
		cv::Mat dst;
		cv::dft(m, dst);
		return dst;
	}

	inline cv::Mat abs_cx(const cv::Mat& m) {
		CV_Assert(m.type() == CV_32FC2);
		cv::Mat dst;
		cv::Mat channels[2];
		cv::split(m, channels);   //多通道图像的分离
		cv::magnitude(channels[0], channels[1], dst);
		return dst;
	}

private:
	/**
	 * @brief FftShift CV_32FC1 CV_32FC2
	 * @param src
	 * @return
	 */
	cv::Mat FftShift(const cv::Mat& src);

	/**
	 * @brief Psf
	 * @param fitting_rem_zer  (in test wavelength nm)
	 * @return
	 */
	void calcPsf(const cv::Mat& fitting_rem_zer);

	/**
	 * @brief StrehlRatio
	 * @param 
	 * @param 
	 */
	void StrehlRatio();

	/**
	 * @brief 计算能量集中度
	 * @param psf
	 * @param energy
	 */
	void calcEnergy();

	void calcmtf(const cv::Mat& psf);
};

#endif // PSF_H
