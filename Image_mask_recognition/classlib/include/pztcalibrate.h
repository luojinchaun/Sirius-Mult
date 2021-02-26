/*****************************************************************
*
* fileName:  pztcalibrate.h
*
* function:  移相器标定算法
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     利用相位直方图计算相移

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef PZTCALIBRATE_H
#define PZTCALIBRATE_H

#include "baseFunc.h"

namespace calc {
	/**
	 * @brief PztHistCalibrate          相移校准        
	 * @param I0 32FC1                  相位图0
	 * @param I1                        相位图1
	 * @param I3                        相位图3
	 * @param I4                        相位图4
	 * @param roi                       相位图的所选区域
	 * @param hist CV_32SC1             相位直方图
	 * @param maxloc good if = 90       相移角度
	 * @note phase difference = 90 degree
	 */
	void PztHistCalibrate(const cv::Mat& I0,
		const cv::Mat& I1,
		const cv::Mat& I3,
		const cv::Mat& I4,
		const cv::Rect& roi,
		cv::Mat& hist,
		int* maxloc);

	/**
	 * @brief PztHistCalibrate          相移校准
	 * @param I0 32FC1                  相位图0
	 * @param I1                        相位图1
	 * @param I3                        相位图3
	 * @param I4                        相位图4
	 * @param hist CV_32SC1             相位直方图
	 * @param maxloc good if = 90       相移角度
	 * @note phase difference = 90 degree
	 */
	void PztHistCalibrate(const cv::Mat& I0,
		const cv::Mat& I1,
		const cv::Mat& I3,
		const cv::Mat& I4,
		cv::Mat& hist,
		int* maxloc);

	/**
	 * @brief PztMser                   计算相移角度直方图的标准差（标准差越大，说明相移越集中）
     * @param hist                      角度直方图
	 * @param I0                        图像0，计算mask使用
	 * @param roi                       用户选择的区域，计算mask使用
	 * @return                          标准差
	 * @note                            理论上只有90°不为0，其余皆为0
	 */
	float PztHistStd(cv::Mat& hist,
		const cv::Mat &I0,
		const cv::Rect &roi);

	/**
    * @brief PztMser                   计算相移角度直方图的标准差（标准差越大，说明相移越集中）
    * @param hist                      角度直方图
    * @param rows                      图像的rows
    * @param cols                      图像cols
    * @return                          标准差
    * @note  理论上只有90°不为0，其余皆为0
    */
	float PztHistStd(cv::Mat& hist,
		int rows,
		int cols);

	/**
	 * @brief PztMser                   计算相位图4与相位图0相减之后的均方根
	 * @param I0 32FC1                  相位图0
	 * @param I4                        相位图4
	 * @param roi                       相位图的所选区域
	 * @return                          均方根
	 * @note  理论上两幅图完全一样，    delta = 0
	 */
	float PztMser(const cv::Mat& I0,
		const cv::Mat& I4,
		const cv::Rect& roi);
} // namespace calc

#endif // PZTCALIBRATE_H