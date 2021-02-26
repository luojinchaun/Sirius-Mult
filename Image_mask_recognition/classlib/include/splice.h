/*****************************************************************
*
* fileName:  splice.h
*
* function:  子孔径拼接
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     子孔径拼接

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef SPLICE_H
#define SPLICE_H

#include "baseFunc.h"

class Splice {
public:
	float px_1mm;
	std::vector<cv::Point> delta;
	std::vector<cv::Mat> phase;
	cv::Mat splicedPhase;

public:
	Splice(float input_px_1mm): px_1mm(input_px_1mm){}

	/**
	 * @brief SpliceProcess                      子孔径拼接功能接口
	 * @param files                              asc文件
	 */
	void SpliceProcess(const std::vector<std::string> &files);
	
private:
	/**
	 * @brief toDelta                            调整偏移量，从0开始
	 * @param delta                              偏移量
	 */
	void toDelta();
	
	/**
	 * @brief getTopLeft                         得到统一坐标系中的左上角的点的坐标
	 * @param delta                              偏移量
	 * @param topleft                            左上角的点的坐标
	 */
	void getTopLeft(std::vector<cv::Point>&topleft);
	
	/**
	 * @brief getSize                            获取拼接后的图像大小
	 * @param size0                              解包裹数据的大小
	 * @param size                               输出的最终拼接后的图像大小
	 */
	void getSize(const std::vector<cv::Point>& topleft,
		cv::Size size0, cv::Size& size);
	
	/**
	 * @brief extendSize                         将解包裹数据展宽
	 * @param phase                              解包裹数据
	 * @param topleft                            各个解包裹数据在展宽图像中的左上角的坐标
	 * @param size                               展宽后的大小
	 * @param extendphase                        展宽后的数据
	 */
	void extendSize(const std::vector<cv::Point>& topleft,
		cv::Size size,
		std::vector<cv::Mat>& extendphase);
	
	/**
	 * @brief splice22                           图像拼接
	 * @param extendphase1                       展宽图1，也是输出的拼接结果
	 * @param extendphase2                       展宽图2
	 */
	void splice22(cv::Mat& extendphase1,
		const cv::Mat& extendphase2);
	
	/**
	 * @brief splice22_2                         图像拼接,速度更快
	 * @param extendphase1                       展宽图1，也是输出的拼接结果
	 * @param extendphase2                       展宽图2
	 */
	void splice22_2(cv::Mat& extendphase1,
		const cv::Mat& extendphase2);

	/*
	 * @brief getNonZero                         查找不为0的点的坐标
	 * @param mask                               查找的mask
	 * @param Coordinate                         存储点的坐标
	 */
	int getNonZero(cv::Mat mask,
		int **Coordinate);
	
	/**
	 * @brief spliceSerial                       拼接多幅图像
	 * @param phase                              解包裹数据
	 * @param delta                              偏移量
	 * @param splicedphase                       拼接图像
	 */
	void spliceSerial();
	
	/**
	 * @brief readSerialFiles                    读取多个asc文件
	 * @param files                              待读取的文件
	 * @param px_1mm                             每mm的像素个数，分辨率
	 * @param phase                              解包裹数据
	 * @param delta                              偏移量
	 */
	void readSerialFiles(const std::vector<std::string>& files);

};

#endif