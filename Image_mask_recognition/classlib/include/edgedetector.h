/*****************************************************************
*
* fileName:  edgedetector.h
*
* function:  对相位图进行轮廓检测
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     轮廓检测模块

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef DETECTOR_H
#define DETECTOR_H

#include "baseFunc.h"

// 掩膜的形状
enum MASK_SHAPE {
	MASK_SHAPE_MANUAL = 0,                       // 手动掩膜
	MASK_SHAPE_RECT,                             // 矩形
	MASK_SHAPE_ROTATED_RECT,                     // 有旋转角度的矩形
	MASK_SHAPE_ELLIPSE,                          // 椭圆
	MASK_SHAPE_SQUARE,                           // 正方形
	MASK_SHAPE_CIRCLE,                           // 圆
	MASK_SHAPE_ANNULUS,                          // 同心圆
	MASK_SHAPE_POLYGON,                          // 多边形
	MASK_SHAPE_PILL,                             // 圆端矩形，矩形加圆,(两条边被半圆取代，类似胶囊)
	MASK_SHAPE_RECT_CHAMFER,                     // 圆角矩形，四角为1/4圆
	MASK_SHAPE_CIRCLE_RECT,                      // 圆边矩形，矩形与圆的交集
	MASK_SHAPE_AUTO                              // 全自动识别
};

// 边缘检测时，输入的参数(cv::Size2f——8byte，MASK_SHAPE——4字节)
struct EDGE_DETEC_PARAMS {
	bool isUseScale = true;                      // 是否使用比例因子进行修改mask, true:使用，false:不使用
	float detecScale = 1.0f;                     // 矩形和圆检测时，缩放的比例因子
	float inputRadius = 0.0f;                    // 检测圆或者圆边矩形、圆角矩形时，手动输入的半径（像素个数）
	float preScale = 0.98f;                      // 预处理比例因子     
	MASK_SHAPE maskShape = MASK_SHAPE_MANUAL;    // 待检测孔的形状
	cv::Size2f rectSize = cv::Size2f(0, 0);      // 手动输入的图形的宽和高、长轴和短轴、边到圆心的距离等，（像素个数）
};

// 边缘检测类
class EdgeDetector {
public:
	bool isSingleHole = false;                   // 单孔标志，true：单孔，false:多孔
	int detecRadius = 0;                         // 待测物的最小包围圆的半径（像素个数）
	cv::Mat detecMask = cv::Mat();               // 待测物的mask
	cv::Mat edgeMask = cv::Mat();                // 待测物的mask边缘（处理前）
	cv::Rect detecSquare = cv::Rect(0, 0, 0, 0); // 待测物的最小包围圆的外接矩形
	cv::Point detecCenter = cv::Point(0, 0);     // 待测物的最小包围圆的圆心
	ERROR_TYPE errorType = ERROR_TYPE_NOT_ERROR; // 边缘检测时出现的错误
	EDGE_DETEC_PARAMS edgeDetecParams;           // 边缘检测时，输入的相关参数

public:
	/**
	 * @brief EdgeDetector                       初始化查找轮廓的相关参数
	 * @param inputParams                        边缘检测时输入的参数
	 */
	EdgeDetector(EDGE_DETEC_PARAMS inputParams) : edgeDetecParams(inputParams) { }

	/**
	 * @brief EdgeDetectorProcess                边缘检测，并找到mask的最小包围圆的外接矩形区域
	 * @param srcs  CV_8UC1 CV32FC1              图像
	 * @param mask                               生成的mask
	 * @note
	 */
	void EdgeDetectorProcess(const std::vector<cv::Mat> srcs,
		const cv::Mat& mask);

	/**
	 * @brief getMaskEdge                        获取掩膜的边缘（默认函数）
	 * @param retMask                            填充的掩膜
	 * @param maskEdge                           掩膜边缘
	 * @param roiMask                            掩膜
	 * @param roi                                位置信息
	 */
	void getMaskEdge(cv::Mat &retMask, cv::Mat &maskEdge, const cv::Mat roiMask, cv::Rect roi);

private:
	/**
	* @brief GetAllContours                      两幅图像查找图像的轮廓
	* @param allcontours                         检测到的所有轮廓
	* @param _src0 CV_8UC1 CV32FC1               图像0
	* @param _src2 CV_8UC1 CV32FC1               图像2
	* @note  sub02 = abs(_src0 - _src2)
	*/
	void GetAllContours(std::vector<cv::Point>& allcontours,
		const cv::Mat& _src0,
		const cv::Mat& _src2);

	/**
	* @brief GetAllContours                      四幅图像查找图像的轮廓
	* @param allcontours                         检测到的所有轮廓
	* @param _src0 CV_8UC1 CV32FC1               图像0
	* @param _src1 CV_8UC1 CV32FC1               图像1
	* @param _src2 CV_8UC1 CV32FC1               图像2
	* @param _src3 CV_8UC1 CV32FC1               图像3
	* @note  add02_13 = (abs(_src0 - _src2) + abs(_src1 - _src3)) / 2
	*/
	void GetAllContours(std::vector<cv::Point>& allcontours,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	// 四幅图像查找图像的轮廓
	std::vector<std::vector<cv::Point>> GetAllContours2(std::vector<cv::Point> &allcontours,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	// 计算mask区域的最小包围圆
	void UnitCircle();

	/**
	 * @brief UnitCircle                         计算mask区域的最小包围圆
	 * @param center                             圆心
	 * @param radius                             半径
	 * @param mask                               mask
	 * @return none
	 * @note                                     circle(不保证圆在mask大小内)
	 */
	void UnitCircle(cv::Point& center,
		int& radius,
		const cv::Mat& mask);

	/**
	 * @brief DetectCircle                       检测圆，得到最小包围圆的圆心和半径
	 * @param center                             圆心
	 * @param radius                             半径
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 */
	bool DetectCircle(cv::Point2f& center,
		float& radius,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2 = cv::Mat(),
		const cv::Mat& _src3 = cv::Mat());

	/**
	 * @brief DetectAnnulus                      检测同心圆，得到最小包围圆的圆心、半径和mask
	 * @param center                             圆心
	 * @param radius                             半径
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 */
	bool DetectAnnulus(cv::Point2f &center,
		float &radius,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	/**
	 * @brief DetectPolygons                     检测多边形，并得到最小包围圆的圆心、半径和mask
	 * @param center                             圆心
	 * @param radius                             半径
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 */
	bool DetectPolygons(cv::Point2f &center,
		float &radius,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	/**
	 * @brief DetectRect                         计算轮廓的垂直边界最小矩形，矩形是与图像上下边界平行的，
	 * @param rect                               外接矩形
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 * @note                                     得到外接矩形的左上点坐标及矩形宽高
	 */
	bool DetectRect(cv::Rect& rect,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	/**
	 * @brief DetectRotatedRect                  计算包含点集最小面积的矩形，可以有偏转角度的，可以与图像的边界不平行
	 * @param rotatedrect                        外接矩形
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 * @note                                     得到外接矩形的中心点的坐标，宽和高，以及旋转角度
	 */
	bool DetectRotatedRect(cv::RotatedRect& rotatedrect,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	/**
	 * @brief DetectEllipse                      计算包含点集最小面积的矩形，可以有偏转角度的，可以与图像的边界不平行
	 * @param ellipse                            椭圆的参数
	 * @param _src0 CV_8UC1 CV32FC1              图像0
	 * @param _src1 CV_8UC1 CV32FC1              图像1
	 * @param _src2 CV_8UC1 CV32FC1              图像2
	 * @param _src3 CV_8UC1 CV32FC1              图像3
	 * @note                                     得到外接矩形的中心点的坐标，宽和高，以及旋转角度
	 */
	bool DetectEllipse(cv::RotatedRect& ellipse,
		const cv::Mat& _src0,
		const cv::Mat& _src1,
		const cv::Mat& _src2,
		const cv::Mat& _src3);

	// 根据用户设置的参数，修改mask的大小
	void changeMaskRect();                                    // 矩形
	void changeMaskRatatedRect(cv::RotatedRect& rotatedRect); // 旋转矩形
	void changeMaskCircle(float fRadius);                     // 圆
	void changeMaskEllipse(cv::RotatedRect& ellipse);         // 椭圆
	void changeMaskPill(cv::RotatedRect& pill);               // 胶囊
	void changeMaskChamfer(cv::RotatedRect& rotatedRect);     // 圆角矩形
	void changeMaskCircleRect(cv::RotatedRect& rotatedRect);  // 圆边矩形

}; // class 

#endif     // DETECTOR_H
