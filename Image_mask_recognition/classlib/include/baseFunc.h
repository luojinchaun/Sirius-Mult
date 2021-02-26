/*****************************************************************
*
* fileName:  baseFunc.h
*
* function:  存放基础函数
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     存放基础函数模块

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef BASEFUNC_H
#define BASEFUNC_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/highgui/highgui.hpp> 
#include <iostream>
#include <algorithm>
#include <vector>

#define SQRT_1  1.00000000f
#define SQRT_2  1.41421356f
#define SQRT_3  1.73205081f
#define SQRT_4  2.00000000f
#define SQRT_5  2.23606798f
#define SQRT_6  2.44948974f
#define SQRT_7  2.64575131f
#define SQRT_8  2.82842712f
#define SQRT_9  3.00000000f
#define SQRT_10  3.16227766f
#define SQRT_11  3.31662479f
#define SQRT_12  3.46410162f
#define SQRT_13  3.60555128f
#define SQRT_14  3.74165739f
#define SQRT_15  3.87298335f
#define SQRT_16  4.00000000f
#define SQRT_17  4.12310563f
#define SQRT_18  4.24264069f
#define SQRT_19  4.35889894f
#define SQRT_20  4.47213595f
#define SQRT_21  4.58257569f
#define SQRT_22  4.69041576f
#define SQRT_23  4.79583152f
#define SQRT_24  4.89897949f
#define SQRT_25  5.00000000f
#define SQRT_26  5.09901951f
#define SQRT_27  5.19615242f
#define SQRT_28  5.29150262f
#define SQRT_29  5.38516481f
#define SQRT_30  5.47722558f

#define Min(a, b) a < b ? a : b
#define Max(a, b) a > b ? a : b

#define RAD_TO_ANGLE  57.29577951f               // 弧度转化为角度的系数
#define PI            3.14159265358979f          // pi
#define PI_2          6.28318530717959f          // 2pi

// 打印函数
#define TURN_ON_DEBUG                            // 打印总开关
#ifdef TURN_ON_DEBUG
#define TURN_ON_DEBUG_VAR                    // 打印变量的开关
#define TURN_ON_DEBUG_STR                    // 打印字符串的开关
#define TURN_ON_DEBUG_DETAILS                // 打印变量的详细信息的开关
// 打印变量
#ifdef TURN_ON_DEBUG_VAR
	#define DEBUG(_PARAMS_) std::cout << "func: " << __FUNCTION__ << ", " << #_PARAMS_ << " = " << _PARAMS_<< std::endl;
#else
	#define DEBUG(_PARAMS_);
#endif
// 打印字符串
#ifdef TURN_ON_DEBUG_STR
	#define DEBUG_STR(_PARAMS_) std::cout << "algorithm: " << _PARAMS_ << std::endl;
#else
	#define DEBUG_STR(_PARAMS_);
#endif
// 打印变量的详细信息
#ifdef TURN_ON_DEBUG_DETAILS
	#define DEBUG_DETAILS(_PARAMS_) std::cout << "file: " << __FILE__ << ", line: " <<  __LINE__ << ", func: " << __FUNCTION__ << ", " << #_PARAMS_ << " = " << _PARAMS_<< std::endl;
#else
	#define DEBUG_DETAILS(_PARAMS_);
#endif
#else
	#define DEBUG(_PARAMS_);
	#define DEBUG_STR(_PARAMS_);
	#define DEBUG_DETAILS(_PARAMS_);
#endif

// 错误类型
enum ERROR_TYPE {
	ERROR_TYPE_NOT_ERROR = 0,
	ERROR_TYPE_NO_ENOUGH_IMAGES,
	ERROR_TYPE_NO_MASK,
	ERROR_TYPE_ROI_TOO_SMALL,
	ERROR_TYPE_PSI_ERROR,
	ERROR_TYPE_UNWRAP_ERROR,
	ERROR_TYPE_ZERNIKE_ERROR,
	ERROR_TYPE_CHECK,
	ERROR_TYPE_SPIKES_TOO_MUCH,
	ERROR_TYPE_NOT_AUTOMASK,
	ERROR_TYPE_PIC_SIZE,
	ERROR_TYPE_NO_RESULT
};

// check功能的判断阈值
struct CHECK_THR {
	int phaseShiftThr = 10;                       // 相移偏差角度阈值
	float stdPhaseHistThr = 0.01f;               // 相移角度直方图的标准差阈值
	float resPvThr = 0.095f;                     // 残差pv的阈值
	float resRmsThr = 0.018f;                    // 残差rms的阈值
};

// 基础函数
class BaseFunc {
public:
	/*--------------------- edgeDetector -------------------------*/
	/**
	 * @brief UnitCart                         产生指定范围内的指定数量点数，相邻数据跨度相同
	 * @param squaresize                       矩形的尺寸（正方形）
	 * @param x return CV_32FC1                列方向的向量
	 * @param y                                行方向的向量
	 */
	static void UnitCart(int squaresize, cv::Mat& x, cv::Mat& y) {
		CV_Assert(squaresize % 2 == 1);
		x.create(squaresize, squaresize, CV_32FC1);
		y.create(squaresize, squaresize, CV_32FC1);
		//设置边界
		x.col(0).setTo(-1.0);
		x.col(squaresize - 1).setTo(1.0f);
		y.row(0).setTo(1.0);
		y.row(squaresize - 1).setTo(-1.0f);

		float delta = 2.0f / (squaresize - 1.0f);  //两个元素的间隔

		//计算其他位置的值
		for (int i = 1; i < squaresize - 1; ++i) {
			x.col(i) = -1.0f + i * delta;
			y.row(i) = 1.0f - i * delta;
		}
	}

	/**
	 * @brief UnitPolar                        直角坐标转换为极坐标
	 * @param x                                UnitCart计算得到的x
	 * @param y                                UnitCart计算得到的y
	 * @param mag                              幅值
	 * @param ang                              角度
	 * @param indegree                         标志位（false：弧度，true:角度）
	 */
	static void UnitPolar(const cv::Mat& x, const cv::Mat& y,
		cv::Mat& mag, cv::Mat& ang, bool indegree = false) {
		cv::cartToPolar(x, y, mag, ang, indegree);   //直角坐标转换为极坐标
	}

	/**
	 * @brief UnitPolar                        直角坐标转换为极坐标
	 * @param squaresize                       矩形的尺寸（正方形）
	 * @param mag                              幅值
	 * @param ang                              角度
	 * @param indegree                         标志位（false：弧度，true:角度）
	 */
	static void UnitPolar(int squaresize, cv::Mat& mag,
		cv::Mat& ang, bool indegree = false) {
		cv::Mat x;
		cv::Mat y;
		UnitCart(squaresize, x, y);                //产生指定范围内的指定数量点数，相邻数据跨度相同
		cv::cartToPolar(x, y, mag, ang, indegree); //坐标转换
	}

	/**
	* @brief RectToChild                       获得矩形
	* @param r                                 矩形的尺寸（正方形）
	* @param roi                               幅值
	*/
	static cv::Rect RectToChild(const cv::Rect& r, const cv::Rect& roi) {
		return cv::Rect(r.x - roi.x, r.y - roi.y, r.width, r.height);
	}

	/**
	 * @brief UnitCircleSquare                 计算圆的外接矩形
	 * @param center                           UnitCircle 得到的 center
	 * @param radius                           UnitCircle 得到的 radius
	 * @return Rect                            外接矩形
	 * @note                                   不保证rect在mask大小内
	 */
	static cv::Rect UnitCircleSquare(const cv::Point& center, int& radius) {
		return cv::Rect(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1);
	}

	/**
	 * @brief UnitRectCircleSquare             计算矩形的最小包围圆的外接矩形
	 * @param center                           UnitCircle 得到的 center
	 * @param radius                           UnitCircle 得到的 radius
	 * @return Rect                            外接矩形以及圆心和半径
	 * @note                                   不保证rect在mask大小内
	 */
	static cv::Rect UnitRectCircleSquare(cv::Rect rect, cv::Point& center, int& radius) {
		center.x = (int)(rect.x + rect.width / 2.0f);
		center.y = (int)(rect.y + rect.height / 2.0f);
		radius = (int)(std::sqrt(rect.width * rect.width + rect.height * rect.height) / 2.0f);
		return cv::Rect(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1);
	}

	/**
	 * @brief UnitRotatedRectCircleSquare      计算旋转矩形的最小包围圆的外接矩形
	 * @param center                           UnitCircle 得到的 center
	 * @param radius                           UnitCircle 得到的 radius
	 * @return Rect                            外接矩形以及圆心和半径
	 * @note                                   不保证rotatedRect在mask大小内
	 */
	static cv::Rect UnitRotatedRectCircleSquare(
		cv::RotatedRect rotatedRect, cv::Point& center, int& radius) {
		center.x = (int)rotatedRect.center.x;
		center.y = (int)rotatedRect.center.y;
		radius = (int)(std::sqrtf(rotatedRect.size.width * rotatedRect.size.width
			+ rotatedRect.size.height * rotatedRect.size.height) / 2.0f);
		return cv::Rect(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1);
	}

	/**
	 * @brief UnitRotatedRectCircleSquare      计算旋转矩形的最小包围圆的外接矩形
	 * @param center                           UnitCircle 得到的 center
	 * @param radius                           UnitCircle 得到的 radius
	 * @param chamferRadius                    倒角矩形的倒角半径
	 * @return Rect                            外接矩形
	 * @note                                   不保证rotatedRect在mask大小内
	 */
	static cv::Rect UnitChamferRectCircleSquare(
		cv::RotatedRect rotatedRect, cv::Point& center,
		int& radius, float chamferRadius) {
		center.x = (int)rotatedRect.center.x;
		center.y = (int)rotatedRect.center.y;

		float newWidth = rotatedRect.size.width - chamferRadius * 2.0f;
		float newHeight = rotatedRect.size.height - chamferRadius * 2.0f;

		radius = (int)(std::sqrtf(newWidth * newWidth + newHeight * newHeight) / 2.0f + chamferRadius * 2.0f);
		return cv::Rect(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1);
	}

	/*--------------------- zernike -------------------------*/
	/**
	 * @brief GridSampling                     设置采样的位置点
	 * @param size                             重采样的范围尺寸
	 * @param rowinterval                      行间隔
	 * @param colinterval                      列间隔
	 * @return                                 重采样的mask
	 */
	static cv::Mat GridSampling(const cv::Size& size, int rowinterval, int colinterval) {
		cv::Mat dst(size, CV_8UC1, cv::Scalar(0));
		//设置采样的位置点
		for (int row = 0; row < dst.rows; row += rowinterval) {
			for (int col = 0; col < dst.cols; col += colinterval) {
				dst.at<uchar>(row, col) = 255;
			}
		}
		return dst;
	}

	//计算转换矩阵时的极坐标相位
	static cv::Mat InitialAng129() {
		cv::Mat ang;
		cv::Mat mag;
		UnitPolar(129, mag, ang);
		return ang;
	}

	//计算转换矩阵时的极坐标幅度
	static cv::Mat InitialMag129() {
		cv::Mat ang;
		cv::Mat mag;
		UnitPolar(129, mag, ang);
		return mag;
	}

	//计算转换矩阵时的极坐标相位
	static cv::Mat InitialAng259() {
		cv::Mat ang;
		cv::Mat mag;
		UnitPolar(259, mag, ang);
		return ang;
	}

	//计算转换矩阵时的极坐标幅度
	static cv::Mat InitialMag259() {
		cv::Mat ang;
		cv::Mat mag;
		UnitPolar(259, mag, ang);
		return mag;
	}

	/*--------------------- other functions -------------------------*/
	/**
	* @brief MorphOpenMask                     为消除孤立的小点，毛刺和小桥影响，对mask进行开运算
	* @param mask                              mask
	* @param shape                             形状
	* @param times                             迭代次数
	* @return
	*/
	static void MorphOpenMask(cv::Mat &mask, int shape, int size, int times)
	{
		//获得shape形，大小为size*size的模板
		cv::Mat element = cv::getStructuringElement(shape, cv::Size(size, size));
		//进行开运算，迭代times次
		morphologyEx(mask, mask, cv::MORPH_OPEN, element, cv::Point(-1, -1), times);
	}

	// 计算pv
	static float getPv(cv::Mat src) {
		double minv = 0;
		double maxv = 0;
		float pv = 0.0f;
		cv::minMaxIdx(src, &minv, &maxv, nullptr, nullptr, src == src);
		pv = (float)(maxv - minv);
		return pv;
	}

	// 计算rms
	static float getRms(cv::Mat src) {
		cv::Mat tmp;
		float rms = 0.0f;
		cv::pow(src, 2.0, tmp);
		float mean_tmp = (float)(cv::mean(tmp, tmp == tmp)[0]);     // 计算均值(nan != nan)
		rms = std::sqrt(mean_tmp);          //得到rms

		return rms;
	}

	// 计算标准差
	static  float getStdDev(std::vector<float> input) {
		float sum = 0.0f;
		std::for_each(std::begin(input), std::end(input), [&](const float d) {
			sum += d;
		});
		float mean = sum / input.size(); //均值
		float accum = 0.0f;
		std::for_each(std::begin(input), std::end(input), [&](const float d) {
			accum += (d - mean)*(d - mean);
		});
		float stdev = sqrt(accum / (input.size()-1)); 
		return stdev;
	}

	// 计算2sigma
	static float getTwoSigma(std::vector<float> input) {
		float std = getStdDev(input);
		return (2 * std);
	}

	/**
	* @brief CalcTheta                         计算x or y方向的角度
	* @param z1                                待测界面，mask1的4阶zernike系数的倾斜项（x:z2, y:z3）
	* @param z2                                参考界面，mask2的4阶zernike系数的倾斜项（x:z2, y:z3）
	* @param aperture1                         mask1的口径(mm)
	* @param aperture2                         mask2的口径(mm)
	* @param refractiveIndex                   折射率
	* @param testWaveLength                    测试波长（nm）
	* @return                                  x or y方向的角度(秒)
	*/
	static float CalcTheta(float z1, float z2, float aperture1,
		float aperture2, float refractiveIndex, float testWaveLength) {
		if (refractiveIndex == 1.0f || aperture1 == 0.0f || aperture2 == 0.0f) {
			return 0.0f;
		}
		float retTheta = 0.0f;
		float tmp = z1 / aperture1 / (refractiveIndex - 1.0f) - 2.0f * z2 / aperture2;
		// retTheta = 410.0544f * tmp / PI / 2.0f / (refractiveIndex - 1.0f); // testWaveLength == 632.8
		// retTheta = 3.6f * testWaveLength  * 0.18f * tmp / PI / 2.0f / (refractiveIndex - 1.0f);
		// retTheta = RAD_TO_ANGLE * testWaveLength  * 0.0036f * tmp / 2.0f / (refractiveIndex - 1.0f);
		retTheta = RAD_TO_ANGLE * testWaveLength  * 0.0036f * tmp / (refractiveIndex - 1.0f);
		return retTheta;
	} 

	/**
	* @brief CalcParallelDegree                计算平行度
	* @param zernikeCoef4_1                    待测界面，mask1的4阶zernike系数（x:z2, y:z3）
	* @param zernikeCoef4_2                    参考界面，mask2的4阶zernike系数（x:z2, y:z3）
	* @param aperture1                         mask1的口径(mm)
	* @param aperture2                         mask2的口径(mm)
	* @param refractiveIndex                   折射率
	* @param testWaveLength                    测试波长（nm）
	* @return                                  平行度（单位：秒）
	*/
	static float CalcParallelDegree(cv::Mat zernikeCoef4_1, cv::Mat zernikeCoef4_2,
		float aperture1, float aperture2, float refractiveIndex, float testWaveLength) {
		float retTheta = 0.0f;

		// 计算x or y方向的角度
		float theta_x = CalcTheta(zernikeCoef4_1.at<float>(1),
			zernikeCoef4_2.at<float>(1), aperture1, aperture2, refractiveIndex, testWaveLength);
		float theta_y = CalcTheta(zernikeCoef4_1.at<float>(2),
			zernikeCoef4_2.at<float>(2), aperture1, aperture2, refractiveIndex, testWaveLength);

		// 计算平行度theta
		retTheta = sqrtf(theta_x * theta_x + theta_y * theta_y);
		
		return retTheta;
	}

	/**
	* @brief detecThreeFlat                    三平面检测（a, b, c）
	* @param xSlice                            拟合曲面横坐标方向的切片(a+b, a+c, b+c, br/cr)@待测面+参考面，br即令b旋转180度，cr=c（参考面不变）
	* @param ySlice                            拟合曲面纵坐标方向的切片(a+b, a+c, b+c, br/cr)@待测面+参考面，br即令b旋转180度，cr=c（参考面不变）
	* @param pv                                三个平面的x,y方向上PV（顺序为：a,b,c，第一行x,第二行y）
	* @param rms                               三个平面的x,y方向上rms（顺序为：a,b,c，第一行x,第二行y）
	* @return                                  三个平面的计算结果(先x(abc)，后y(abc))
	*/
	static std::vector<cv::Mat> detecThreeFlat(std::vector<cv::Mat>xSlice,
		std::vector<cv::Mat>ySlice, float pv[2][3], float rms[2][3]) {
		CV_Assert(xSlice.size() == 4);
		CV_Assert(ySlice.size() == 4);

		std::vector<cv::Mat> result;

		cv::Mat tmp,rxa,rxb,rxc,rya,ryb,ryc;
		// 计算x方向的结果
		tmp = (xSlice[0] + xSlice[1] - xSlice[3]) / 2.0f;
		rxa = tmp.clone();
		result.emplace_back(rxa);
		pv[0][0] = getPv(rxa);
		rms[0][0] = getRms(rxa);

		tmp = (xSlice[0] + xSlice[3] - xSlice[1]) / 2.0f;
		rxb = tmp.clone();
		result.emplace_back(rxb);
		pv[0][1] = getPv(rxb);
		rms[0][1] = getRms(rxb);

		tmp = (xSlice[3] + xSlice[1] - xSlice[0]) / 2.0f;
		rxc = tmp.clone();
		result.emplace_back(rxc);
		pv[0][2] = getPv(rxc);
		rms[0][2] = getRms(rxc);

		// 计算y方向的结果
		tmp = (ySlice[0] + ySlice[1] - ySlice[2]) / 2.0f;
		rya = tmp.clone();
		result.emplace_back(rya);
		pv[1][0] = getPv(rya);
		rms[1][0] = getRms(rya);

		tmp = (ySlice[0] + ySlice[2] - ySlice[1]) / 2.0f;
		ryb = tmp.clone();
		result.emplace_back(ryb);
		pv[1][1] = getPv(ryb);
		rms[1][1] = getRms(ryb);

		tmp = (ySlice[2] + ySlice[1] - ySlice[0]) / 2.0f;
		ryc = tmp.clone();
		result.emplace_back(ryc);
		pv[1][2] = getPv(ryc);
		rms[1][2] = getRms(ryc);

		/*std::cout << "pv = ";
		for (int i = 0; i < 2; ++i) {
			std::cout << std::endl;
			for (int j = 0; j < 3; ++j) {
				std::cout << pv[i][j] << ", ";
			}
		}

		std::cout << std::endl << "rms = ";
		for (int i = 0; i < 2; ++i) {
			std::cout << std::endl;
			for (int j = 0; j < 3; ++j) {
				std::cout << rms[i][j] << ", ";
			}
		}*/

		return result;
	}
	
	/**
	* @brief Rotate                            图像旋转
	* @param srcImage                          待旋转图像
	* @param dstImage                          旋转后的图像
	* @param angle                             旋转角度（正数为逆时针，负数为顺时针）
	* @return 
	*/
	static void Rotate(const cv::Mat &srcImage, cv::Mat &dstImage, double angle)
	{
		cv::Point2f center(srcImage.cols / 2.0f, srcImage.rows / 2.0f);//中心
		cv::Mat M = cv::getRotationMatrix2D(center, angle, 1);//计算旋转的仿射变换矩阵 
		cv::warpAffine(srcImage, dstImage, M, cv::Size(srcImage.cols, srcImage.rows));//仿射变换  
	}

	/**
    * @brief Rotate2                           图像旋转
    * @param srcImage                          待旋转图像
    * @param dstImage                          旋转后的图像
    * @param angle                             旋转角度（正数为逆时针，负数为顺时针）
	* @param center                            旋转中心
    * @return
    */
	static void Rotate2(const cv::Mat &srcImage, cv::Mat &dstImage, double angle,cv::Point2f center)
	{
		cv::Mat M = cv::getRotationMatrix2D(center, angle, 1);//计算旋转的仿射变换矩阵 
		cv::warpAffine(srcImage, dstImage, M, cv::Size(srcImage.cols, srcImage.rows));//仿射变换  
	}
	
	/**
	* @brief analysisDoubleSpherSur            计算被测面和标准面的绝对误差（双球面绝对检测）
	* @param fittedSurface                     拟合曲面(M4:猫眼点, M0:共焦点0度, M1:90度, M2:180度, M3:270度)
	* @param pv                                pv(0:被测面, 1:标准面)
	* @param rms                               rms(0:被测面, 1:标准面)
	* @param testSurError                      被测面误差
	* @param standSurError                     标准面误差
	* @return                                  
	*/
	static void analysisDoubleSpherSur(std::vector<cv::Mat> fittedSurface,
		float pv[2], float rms[2], cv::Mat &testSurError, cv::Mat &standSurError) {
		CV_Assert(fittedSurface.size() == 5);

		cv::Mat ret;
		// 对猫眼点进行顺时针旋转
		cv::Mat M4_90, M4_180, M4_270;            // 顺时针旋转
		Rotate(fittedSurface[0], M4_90, -90);     // 顺时针旋转
		Rotate(fittedSurface[0], M4_180, -180);   // 顺时针旋转
		Rotate(fittedSurface[0], M4_270, -270);   // 顺时针旋转
		// 对共焦点进行逆时针旋转
		cv::Mat M1_90, M2_180, M3_270;            // 逆时针旋转
		Rotate(fittedSurface[2], M1_90, 90);      // 逆时针旋转
		Rotate(fittedSurface[3], M2_180, 180);    // 逆时针旋转
		Rotate(fittedSurface[4], M3_270, 270);    // 逆时针旋转

		// 计算被测面误差及其pv、rms
		testSurError = (fittedSurface[1] + M1_90 + M2_180 + M3_270
			- fittedSurface[0] - M4_90 - M4_180 - M4_270) / 4.0f;
		pv[0] = getPv(testSurError);
		rms[0] = getRms(testSurError);
		// 计算标准面误差及其pv、rms
		standSurError = fittedSurface[1] - testSurError;
		pv[1] = getPv(standSurError);
		rms[1] = getRms(standSurError);

		/*std::cout << "pv = ";
		for (int i = 0; i < 2; ++i) {
			std::cout << pv[i] << ", ";
		}

		std::cout << std::endl << "rms = ";
		for (int i = 0; i < 2; ++i) {
			std::cout << rms[i] << ", ";
		}
		std::cout << std::endl;*/

		return;
	}

	/**
	* @brief calcOpticalHomogeneity            计算光学均匀性
	* @param refractiveIndex                   折射率
	* @param thickness				           器件厚度
	* @param unitType                          单位
	* @param testWaveLength                    波长
	* @param fittedSurface                     拟合曲面(C,T,S1,S2)空腔，透过，前表面，后表面
	* @param pvh                               pvh(pv除以厚度)
	* @param rmsh                              rmsh(rms除以厚度)
	* @param pv                                pv
	* @param rms                               rms
	* @return
	*/
	static int calcOpticalHomogeneity(float refractiveIndex, float thickness, int unitType,
		float testWaveLength, std::vector<cv::Mat> &fittedSurface, float *pvh, float *rmsh,float *pv,float *rms,cv::Mat &result)
	{
	    if (fittedSurface.size() < 4) {
			return 1;                            //均匀性图组中图片个数不够
		}
		
		cv::Mat C = fittedSurface[0];
		cv::Mat T = fittedSurface[1];
		cv::Mat S1 = fittedSurface[2];
		cv::Mat S2 = fittedSurface[3];

		if (C.empty() || T.empty() || S1.empty() || S2.empty())
			return 2;                            //存在空图

		if (C.size() != T.size() || C.size() != S1.size() || C.size() != S2.size())
			return 3;                            //大小不一致

		cv::Mat deltaN = cv::Mat();
		deltaN = (refractiveIndex * (T - C) - (refractiveIndex - 1) * (S2 - S1));

		*pv = getPv(deltaN);
		*rms = getRms(deltaN);

		float coef = 1.0f;
		switch (unitType)
		{
		case 0:									 // UNIT_TYPE_WAVE, 以波长为单位
			coef = testWaveLength / 1000000.0f;
			break;
		case 1:									 // UNIT_TYPE_FR, 以Fr为单位
			coef = testWaveLength / 2000000.0f;
			break;
		case 2:                                  // UNIT_TYPE_NM, 以nm为单位
			coef = 1 / 1000000.0f;
			break;
		case 3:                                  // UNIT_TYPE_UM, 以um为单位
			coef = 1 / 1000.0f;;
			break;
		case 4:                                  // UNIT_TYPE_MM,以mm为单位:
			coef = 1.0f;
			break;
		default:
			coef = testWaveLength / 1000000.0f;
			break;
		}

		*pvh = *pv * coef / thickness;
		*rmsh = *rms * coef / thickness;
		deltaN.copyTo(result);
		return 0;                               //光学均匀性模块正常
	}

	//按第一行展开计算3*3|A|
	static float getA(float arcs[3][3], int n)
	{
		if (n == 1)
		{
			return arcs[0][0];
		}
		float ans = 0;
		float temp[3][3] = { 0.0 };
		int i, j, k;
		for (i = 0; i < n; ++i)
		{
			for (j = 0; j < n - 1; ++j)
			{
				for (k = 0; k < n - 1; ++k)
				{
					temp[j][k] = arcs[j + 1][(k >= i) ? k + 1 : k];

				}
			}

			float t = getA(temp, n - 1);

			if (i % 2 == 0)
			{
				ans += arcs[0][i] * t;
			}
			else
			{
				ans -= arcs[0][i] * t;
			}
		}
		return ans;
	}
	
	//计算每一行每一列的每个元素所对应的余子式，组成A*
	static void getAStart(float arcs[3][3], int n, float ans[3][3])
	{
		if (n == 1)
		{
			ans[0][0] = 1;
			return;
		}
		int i, j, k, t;
		float temp[3][3];
		for (i = 0; i < n; i++)
		{
			for (j = 0; j < n; j++)
			{
				for (k = 0; k < n - 1; k++)
				{
					for (t = 0; t < n - 1; t++)
					{
						temp[k][t] = arcs[k >= i ? k + 1 : k][t >= j ? t + 1 : t];
					}
				}

				ans[j][i] = getA(temp, n - 1);  //此处顺便进行了转置
				if ((i + j) % 2 == 1)
				{
					ans[j][i] = -ans[j][i];
				}
			}
		}
	}
	
	//得到给定矩阵src的逆矩阵保存到des中(3*3)。
	static void GetMatrixInverse(float src[3][3], int n, float des[3][3])
	{
		float value = getA(src, n);
		float t[3][3];

		getAStart(src, n, t);

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				des[i][j] = t[i][j] / value;
			}
		}
	}

	/**
	* @brief calculator_add                    相位计算器—加法
	* @param inImage1                          输入位相图1
	* @param inImage2                          输入位相图2
	* @return                                  结果图
	*/
	static cv::Mat calculator_add(const cv::Mat &inImage1,
		const cv::Mat &inImage2) {
		CV_Assert(inImage1.type() == inImage2.type());
		CV_Assert(inImage1.size() == inImage2.size());
		return inImage1 + inImage2;
	}

	/**
    * @brief calculator_sub                    相位计算器—减法
    * @param inImage1                          输入位相图1
	* @param inImage2                          输入位相图2
	* @return                                  结果图
	*/
	static cv::Mat calculator_sub(const cv::Mat &inImage1,
		const cv::Mat &inImage2) {
		CV_Assert(inImage1.type() == inImage2.type());
		CV_Assert(inImage1.size() == inImage2.size());
		return inImage1 - inImage2;
	}

	/**
	* @brief calculator_mul                    相位计算器—乘法
	* @param inImage                           输入位相图
	* @param scale                             乘法系数
	* @return                                  结果图
	*/
    static cv::Mat calculator_mul(const cv::Mat &inImage, const float scale) {
		return scale * inImage;
	}

	/**
	* @brief calculator_rev                    相位计算器—反转
	* @param inImage                           输入位相图
	* @return                                  结果图
	*/
	static cv::Mat calculator_rev(const cv::Mat &inImage) {
		return -1*inImage;
	}

	/**
	* @brief calculator_rotate                 相位计算器—旋转
	* @param inImage                           输入位相图
	* @param angle                             角度
	* @return                                  结果图
	*/
    static cv::Mat calculator_rotate(const cv::Mat &inImage, cv::Mat &mask,const float angle) {
		CV_Assert(inImage.type() == CV_32FC1);
		int left = 10000;
		int up = 10000;
		int right = 0;
		int down = 0;
		int row = inImage.rows;
		int col = inImage.cols;
		for (int i = 0; i < row; ++i)
		{
            uchar *temp = mask.ptr<uchar>(i);
			for (int j = 0; j < col; ++j)
			{
                if (temp[j]!=0)
				{
					if (i < up) up = i;
					if (i > down) down = i;
					if (j < left) left = j;
					if (j > right) right = j;
				}
			}
		}
		cv::Point2i center;
		center.x = (right + left) / 2;
		center.y = (up + down) / 2;
		cv::Mat out = cv::Mat();
		Rotate2(inImage,out,angle,center);
		return out;
	}
};

namespace calc {
	template<typename T, int num>
	/**
	 * @brief foreach_x
	 * @param srcs
	 * @param dst
	 * @param func
	 * @param mask
	 */
	void foreach_x(const std::vector<cv::Mat>& srcs,
		cv::Mat& dst,
		const std::function<T(const T*)>& func,
		const cv::Mat& mask = cv::Mat()) {
		CV_Assert(srcs.size() >= num);
		CV_Assert(srcs[0].elemSize1() == sizeof(T) && srcs[0].channels() == 1);
		CV_Assert(srcs[0].size() == dst.size());
		CV_Assert(srcs[0].type() == dst.type());
		CV_Assert(mask.empty() || srcs[0].size() == mask.size());
		CV_Assert(mask.type() == CV_8UC1);

		if (mask.empty()) {
			dst.forEach<T>([&srcs, &func](T& p, const int* position) {
				T vs[num];
				for (unsigned int i = 0; i != num; ++i) {
					vs[i] = srcs[i].at<T>(position[0], position[1]);
				}
				p = func(vs);
			});
		}
		else {
			dst.forEach<T>([&srcs, &func, &mask](T& p, const int* position) {
				if (mask.at<uchar>(position[0], position[1]) != 0) {
					T vs[num];
					for (unsigned int i = 0; i != num; ++i) {
						vs[i] = srcs[i].at<T>(position[0], position[1]);
					}
					p = func(vs);
				}
			});
		}
	}

	template<typename T, int num>
	/**
	 * @brief foreach_x
	 * @param srcs
	 * @param func
	 * @param initialvalue
	 * @param mask
	 * @return
	 */
	cv::Mat foreach_x(const std::vector<cv::Mat>& srcs,
		const std::function<T(const T*)>& func,
		T initialvalue = 0,
		const cv::Mat& mask = cv::Mat()) {
		cv::Mat dst(srcs[0].size(), srcs[0].type(), initialvalue);
		foreach_x<T, num>(srcs, dst, func, mask);
		return dst;
	}

	/**
	 * @brief  bitwise_and()同 &,  opencv自带的按位与的函数，
	 * @param  src1                输入图像1
	 * @param  src2                输入图像2
	 * @param  mask                掩膜
	 * @return dst                 输出结果即 dst = (src1 & src2) && mask != 0
	 */
	inline cv::Mat bitwise_and(cv::InputArray src1, cv::InputArray src2,
		cv::InputArray mask = cv::noArray()) {
		cv::Mat dst;
		cv::bitwise_and(src1, src2, dst, mask);
		return dst;
	}

	/**
	 * @brief  bitwise_or()同 |,   opencv自带的按位或的函数，
	 * @param  src1                输入图像1
	 * @param  src2                输入图像2
	 * @param  mask                掩膜
	 * @return dst                 输出结果即 dst = (src1 | src2) && mask != 0
	 */
	inline cv::Mat bitwise_or(cv::InputArray src1, cv::InputArray src2,
		cv::InputArray mask = cv::noArray()) {
		cv::Mat dst;
		cv::bitwise_or(src1, src2, dst, mask);
		return dst;
	}

	/**
	 * @brief  bitwise_xor()同 ^,  opencv自带的按位异或的函数，0⊕0=0，1⊕0=1，0⊕1=1，1⊕1=0(同为0，异为1)
	 * @param  src1                输入图像1
	 * @param  src2                输入图像2
	 * @param  mask                掩膜
	 * @return dst                 输出结果即 dst = (src1 ^ src2) && mask != 0
	 */
	inline cv::Mat bitwise_xor(cv::InputArray src1, cv::InputArray src2,
		cv::InputArray mask = cv::noArray()) {
		cv::Mat dst;
		cv::bitwise_xor(src1, src2, dst, mask);
		return dst;
	}

	/**
	 * @brief  bitwise_not 同 ~,   opencv自带的非函数，
	 * @param  src                 输入图像
	 * @param  mask                掩膜
	 * @return dst                 输出结果即 dst = (~src) && mask != 0
	 */
	inline cv::Mat bitwise_not(cv::InputArray src,
		cv::InputArray mask = cv::noArray()) {
		cv::Mat dst;
		cv::bitwise_not(src, dst, mask);
		return dst;
	}

	/**
	 * @brief  sqrt,               计算矩阵中每个元素的平方根，
	 * @param  src                 输入图像
	 * @return dst                 输出结果即 dst = sqrt(src)
	 */
	inline cv::Mat sqrt(cv::InputArray src) {
		cv::Mat dst;
		cv::sqrt(src, dst);
		return dst;
	}

	/**
	 * @brief  pow,                对矩阵逐元素取power次幂,若power为整数，直接进行幂运算；
	 *                             否则，先计算原矩阵的绝对值，再进行power次幂
	 * @param  src                 输入图像
	 * @param  power               power次幂
	 * @return dst                 输出结果即 dst = (src)^power
	 */
	inline cv::Mat pow(cv::InputArray src, double power) {
		cv::Mat dst;
		cv::pow(src, power, dst);
		return dst;
	}

	/**
	 * @brief  exp,                求src中所有元素的指数，并将结果放在dst中
	 * @param  src                 输入图像
	 * @return dst                 输出结果,即 dst = e^src
	 */
	inline cv::Mat exp(cv::InputArray src) {
		cv::Mat dst;
		cv::exp(src, dst);
		return dst;
	}

	/**
	 * @brief  sinf,               求src中所有元素的正弦，并将结果放在dst中
	 * @param  src                 输入图像
	 * @return dst                 输出结果,即 dst = sin(src)
	 */
	inline cv::Mat sinf(const cv::Mat& src) {
		CV_Assert(src.type() == CV_32FC1);
		cv::Mat dst(src.size(), src.type());

		int cols = src.cols;
		int rows = src.rows;

		//返回bool值，判断存储是否连续。
		if (src.isContinuous() && dst.isContinuous())
		{
			cols *= rows;
			rows = 1;
		}
		//计算每个元素的sin()
		for (int i = 0; i < rows; i++)
		{
			const float* srci = src.ptr<float>(i);
			float* dsti = dst.ptr<float>(i);
			for (int j = 0; j < cols; j++) {
				dsti[j] = std::sinf(srci[j]);
			}
		}
		return dst;
	}

	/**
	 * @brief  cosf,               求src中所有元素的余弦，并将结果放在dst中
	 * @param  src                 输入图像
	 * @return dst                 输出结果,即 dst = cos(src)
	 */
	inline cv::Mat cosf(const cv::Mat& src) {
		CV_Assert(src.type() == CV_32FC1);
		cv::Mat dst(src.size(), src.type());

		int cols = src.cols;
		int rows = src.rows;

		//返回bool值，判断存储是否连续。
		if (src.isContinuous() && dst.isContinuous())
		{
			cols *= rows;
			rows = 1;
		}
		//计算每个元素的cos()
		for (int i = 0; i < rows; i++)
		{
			const float* srci = src.ptr<float>(i);
			float* dsti = dst.ptr<float>(i);
			for (int j = 0; j < cols; j++) {
				dsti[j] = std::cosf(srci[j]);
			}
		}
		return dst;
	}
	
	/**
	 * @brief  expf_cx,            求exp(src)(两通道)，第一通道为实部，第二部分为虚部
	 * @param  src                 输入图像
	 * @return dst                 输出结果,即 dst = exp(src[0] + j * src[1])
	 */
	inline cv::Mat expf_cx(const cv::Mat& src) {
		CV_Assert(src.type() == CV_32FC2);
		cv::Mat dst(src.size(), src.type(), 0.0);

		int cols = src.cols;
		int rows = src.rows;

		if (src.isContinuous() && dst.isContinuous())
		{
			cols *= rows;
			rows = 1;
		}
		for (int i = 0; i < rows; i++)
		{
			const cv::Vec2f* srci = src.ptr<cv::Vec2f>(i);
			cv::Vec2f* dsti = dst.ptr<cv::Vec2f>(i);
			for (int j = 0; j < cols; j++) {
				std::complex<float> v(srci[j][0], srci[j][1]);
				std::complex<float> r = std::exp(v);
				dsti[j] = cv::Vec2f(r.real(), r.imag());
			}
		}
		return dst;
	}

	/**
	 * @brief  SetToNan,           将src中的非mask区域设置为NaN
	 * @param  src                 输入图像
	 * @return mask                mask
	 */
	inline void SetToNan(cv::Mat& src, const cv::Mat& mask) {
		CV_Assert(src.type() == CV_32FC1);
		cv::Mat _nan(src.size(), src.type(), nan(""));
		_nan.setTo(0, ~mask);
		src = src + _nan;
	}

	/**
	 * @brief  find,               查找满足条件的索引，并将结果放在idx中
	 * @param  condition           输入条件
	 * @return idx                 输出结果,
	 */
	inline void find(const cv::Mat& condition,
		std::vector<cv::Point>& idx) {
		cv::findNonZero(condition, idx);  //Returns the list of locations of non-zero pixels
	}

	/**
	 * @brief  find_nan,           查找所有为NaN的元素，并将其索引放在idx中，(NaN != NaN) = true
	 * @param  src                 输入数据
	 * @return idx                 输出结果,
	 */
	inline void find_nan(const cv::Mat& src,
		std::vector<cv::Point>& idx) {
		cv::findNonZero(~(src == src), idx);  //Returns the list of locations of non-zero pixels
	}

	/**
	 * @brief  find_notnan,        查找所有不为NaN的元素，并将其索引放在idx中，(NaN == NaN) = false
	 * @param  src                 输入数据
	 * @return idx                 输出结果,
	 */
	inline void find_notnan(const cv::Mat& src,
		std::vector<cv::Point>& idx) {
		cv::findNonZero(src == src, idx);  //Returns the list of locations of non-zero pixels
	}

	template <typename T>
	/**
	 * @brief  get in a col         将src中，idx区域转化为一个列矩阵
	 * @param  src                  输入数据
	 * @param  idx                  待处理区域的索引
	 * @return dst                  列矩阵
	 */
	inline cv::Mat get(const cv::Mat& src,
		const std::vector<cv::Point>& idx) {
		CV_Assert(cv::DataType<T>::type == src.type());
		int num = (int)idx.size();
		cv::Mat dst(num, 1, src.type());

		/* pragma omp parallel for 是OpenMP中的一个指令，
		表示接下来的for循环将被多线程执行，另外每次循环之间不能有关系 */
#pragma omp parallel for
		for (int i = 0; i < num; ++i) {
				dst.at<T>(i, 0) = src.at<T>(idx[i]);
		}
		return dst;
	}

	/**
	 * @brief  get_notnan in a col  将所有不为NaN的元素转化为一个列向量
	 * @param  src CV_32FC1         输入数据
	 * @return dst                  列向量
	 */
	inline cv::Mat get_notnan(const cv::Mat& src) {
		CV_Assert(src.type() == CV_32FC1);
		std::vector<cv::Point> idx;

		//查找所有不为NaN的元素，并将其索引放在idx中
		find_notnan(src, idx);
		cv::Mat dst((int)(idx.size()), 1, src.type());
		for (int i = 0; i != idx.size(); ++i) {
			dst.at<float>(i, 0) = src.at<float>(idx[i]); //转化为列向量
		}
		return dst;
	}

	// 画直方图
	inline void drawHistImg(cv::Mat &hist)
	{
		cv::Mat histImage = cv::Mat::zeros(540, 720, CV_8UC1);
		const int bins = 180;
		double maxValue;
		cv::Point2i maxLoc;
		cv::minMaxLoc(hist, 0, &maxValue, 0, &maxLoc);
		int scale = 4;
		int histHeight = 540;

		/*std::string name_ang = std::to_string(maxLoc.y);
		std::string name_unit = "度";
		std::string name = "最大值为：";
		name.append(name_ang);
		name.append(name_unit);*/

		for (int i = 0; i < bins; i++)
		{
			float binValue = (float)(hist.at<int>(i));
			int height = cvRound(binValue * histHeight / maxValue);
			cv::rectangle(histImage, cv::Point(i * scale, histHeight),
				cv::Point((i + 1) * scale, histHeight - height), cv::Scalar(255), -1);
			
		}
		cv::rectangle(histImage, cv::Point(90 * scale, histHeight),
			cv::Point((90 + 1) * scale, 0), cv::Scalar(150), -1);
		cv::imshow("hist", histImage);
		cv::waitKey(1000);
	}
	
	// 画圆
	inline void DrawCircle(cv::Mat mask,
		const cv::Point2i &center, int radius, 
		const cv::Scalar &color, int lineType)
	{
		cv::circle(mask, center, radius, color, lineType);
	}
	
	// 画矩形
	inline void DrawRect(cv::Mat mask,
		const cv::Rect &rect, const cv::Scalar &color, int lineType)
	{
		cv::rectangle(mask, rect, color, lineType);
	}
	
	inline void DrawRect(cv::Mat mask,
		const cv::Rect &rect, int newwidth,
		int newheight, const cv::Scalar &color, int lineType)
	{
		cv::Rect newrect(0, 0, newwidth, newheight);
		newrect.x = rect.x + (rect.width - newwidth) / 2;
		newrect.y = rect.y + (rect.height - newheight) / 2;
		DrawRect(mask, newrect, color, lineType);
	}
	
	// 画旋转矩形
	inline void DrawRotatedRect(cv::Mat mask,
		const cv::RotatedRect &rotatedrect,
		const cv::Scalar &color, int lineType)
	{
		cv::Point2f ps[4];
		rotatedrect.points(ps);

		std::vector<std::vector<cv::Point>> tmpContours;    // 创建一个InputArrayOfArrays 类型的点集
		std::vector<cv::Point> contours;
		for (int i = 0; i != 4; ++i) {
			contours.emplace_back(cv::Point2i(ps[i]));
		}
		tmpContours.insert(tmpContours.end(), contours);
		drawContours(mask, tmpContours, 0, color, lineType);  // 填充mask
	}
	
	inline void DrawRotatedRect(cv::Mat mask,
		const cv::RotatedRect &rotatedrect, float newlong, 
		float newshort, const cv::Scalar &color, int lineType)
	{
		cv::RotatedRect newrotatedrect = rotatedrect;
		if (newrotatedrect.size.width > newrotatedrect.size.height) {
			newrotatedrect.size = cv::Size2f(newlong, newshort);
		}
		else {
			newrotatedrect.size = cv::Size2f(newshort, newlong);
		}
		DrawRotatedRect(mask, newrotatedrect, color, lineType);
	}
	
	// 画椭圆
	inline void DrawEllipse(cv::Mat mask,
		const cv::RotatedRect &ellipse, 
		const cv::Scalar &color, int lineType)
	{
		cv::ellipse(mask, ellipse, color, lineType);
	}
	
	inline void DrawEllipse(cv::Mat mask,
		const cv::RotatedRect &ellipse, float newlong,
		float newshort, const cv::Scalar &color, int lineType)
	{
		cv::RotatedRect newellipse = ellipse;
		if (newellipse.size.width > ellipse.size.height) {
			newellipse.size = cv::Size2f(newlong, newshort);
		}
		else {
			newellipse.size = cv::Size2f(newshort, newlong);
		}
		DrawEllipse(mask, newellipse, color, lineType);
	}
	
	// 画矩形与圆
	inline void DrawPill(cv::Mat mask,
		const cv::RotatedRect &rotatedrect,
		const cv::Scalar &color, bool s, int lineType)
	{
		cv::RotatedRect rect = rotatedrect;
		float r = rect.size.height / 2.0f;
		if (rect.size.width > rect.size.height) {
			rect.size.width -= rect.size.height;
		}
		else {
			rect.size.height -= rect.size.width;
			r = rect.size.width / 2.0f;
		}
		cv::Point2f ps[4];
		rect.points(ps);
		// rotatedrect.points(ps);

		std::vector<std::vector<cv::Point>> tmpContours;
		std::vector<cv::Point> contours;
		for (int i = 0; i != 4; ++i) {
			contours.emplace_back(cv::Point2i(ps[i]));
		}
		tmpContours.insert(tmpContours.end(), contours);
		drawContours(mask, tmpContours, 0, color, lineType);  // 填充mask

		// 计算常长短轴
		float a = rotatedrect.size.width;
		float b = rotatedrect.size.height;

		int point01_x = (int)((ps[0].x + ps[1].x) / 2.0f);
		int point01_y = (int)((ps[0].y + ps[1].y) / 2.0f);
		int point03_x = (int)((ps[0].x + ps[3].x) / 2.0f);
		int point03_y = (int)((ps[0].y + ps[3].y) / 2.0f);
		int point12_x = (int)((ps[1].x + ps[2].x) / 2.0f);
		int point12_y = (int)((ps[1].y + ps[2].y) / 2.0f);
		int point23_x = (int)((ps[2].x + ps[3].x) / 2.0f);
		int point23_y = (int)((ps[2].y + ps[3].y) / 2.0f);

		if (s) {
			// float r = a > b ? (b / 2.0f) : (a / 2.0f);
			cv::Point c0 = a < b ? cv::Point(point12_x, point12_y) : cv::Point(point23_x, point23_y);
			cv::Point c1 = a < b ? cv::Point(point03_x, point03_y) : cv::Point(point01_x, point01_y);

			// 长轴两端以填充的方式画圆，直径等于短轴
			cv::circle(mask, c0, (int)r, color, lineType);
			cv::circle(mask, c1, (int)r, color, lineType);
		}
		else {
			// float r = a < b ? (b / 2.0f) : (a / 2.0f);
			cv::Point c0 = a > b ? cv::Point(point12_x, point12_y) : cv::Point(point23_x, point23_y);
			cv::Point c1 = a > b ? cv::Point(point03_x, point03_y) : cv::Point(point01_x, point01_y);

			// 短轴两端以填充的方式画圆，直径等于长轴
			cv::circle(mask, c0, (int)r, color, lineType);
			cv::circle(mask, c1, (int)r, color, lineType);
		}
	}
	
	inline void DrawPill(cv::Mat mask,
		const cv::RotatedRect &rotatedrect, float newlong, 
		float newshort, const cv::Scalar &color, bool s, int lineType)
	{
		cv::RotatedRect newrotatedrect = rotatedrect;
		if (newrotatedrect.size.width > newrotatedrect.size.height) {
			newrotatedrect.size = cv::Size2f(newlong, newshort);
		}
		else {
			newrotatedrect.size = cv::Size2f(newshort, newlong);
		}
		DrawPill(mask, newrotatedrect, color, s, lineType);  //画矩形与圆
	}
	
	// 画圆角的矩形
	inline void DrawRotatedRectChamfer(cv::Mat &mask, 
		const cv::RotatedRect &rotatedrect, float newlong, 
		float newshort, int radius, const cv::Scalar &color, int lineType)
	{
		cv::RotatedRect newrotatedrect = rotatedrect;
		if (newrotatedrect.size.width > newrotatedrect.size.height) {
			newrotatedrect.size = cv::Size2f(newlong, newshort);
		}
		else {
			newrotatedrect.size = cv::Size2f(newshort, newlong);
		}

		//画宽缩减后的矩形
		cv::RotatedRect r1 = newrotatedrect;
		r1.size.width = r1.size.width - 2 * radius;
		DrawRotatedRect(mask, r1, color, lineType);

		//画高缩减后的矩形
		cv::RotatedRect r2 = newrotatedrect;
		r2.size.height = r2.size.height - 2 * radius;
		DrawRotatedRect(mask, r2, color, lineType);

		//画四个角的圆
		cv::RotatedRect r3 = r2;
		r3.size.width = r1.size.width;

		cv::Point2f ps[4];
		r3.points(ps);

		for (int i = 0; i != 4; ++i) {
			cv::circle(mask, ps[i], radius, color, lineType);
		}
	}
	
	inline void DrawRotatedRectChamfer(cv::Mat &mask,
		const cv::RotatedRect &rotatedrect, float radius,
		const cv::Scalar &color, int lineType)
	{
		cv::RotatedRect newrotatedrect = rotatedrect;

		//画宽缩减后的矩形
		cv::RotatedRect r1 = newrotatedrect;
		r1.size.width = r1.size.width - 2 * radius;
		DrawRotatedRect(mask, r1, color, lineType);

		//画高缩减后的矩形
		cv::RotatedRect r2 = newrotatedrect;
		r2.size.height = r2.size.height - 2 * radius;
		DrawRotatedRect(mask, r2, color, lineType);

		//画四个角的圆
		cv::RotatedRect r3 = r2;
		r3.size.width = r1.size.width;

		cv::Point2f ps[4];
		r3.points(ps);

		for (int i = 0; i != 4; ++i) {
			cv::circle(mask, ps[i], (int)radius, color, lineType);
		}
	}

	// 画多边形
	inline void DrawPolygon(cv::Mat& mask,
		const std::vector<cv::Point> &contours, 
		const cv::Scalar &color, int lineType) {
		if (contours.empty()) {
			return;
		}

		std::vector<std::vector<cv::Point>> tmpContours;    // 创建一个InputArrayOfArrays 类型的点集
		tmpContours.insert(tmpContours.end(), contours);
		drawContours(mask, tmpContours, 0, color, lineType);  // 填充mask
	}
	
	inline void DrawPolygon(cv::Mat& mask,
		const std::vector<std::vector<cv::Point>> &contours, const cv::Scalar &color, int lineType) {
		drawContours(mask, contours, 0, color, lineType);  // 填充mask
	}

	// 画Circle Rectangle
	inline void DrawCircleRect(cv::Mat& mask,
		const cv::RotatedRect &rotatedrect, const cv::Point2i& center,
		int radius, const cv::Scalar &color, int lineType) {
		cv::Mat maskCircle = mask.clone();
		cv::Mat maskRect = mask.clone();
		cv::circle(maskCircle, center, radius, color, lineType);
		DrawRotatedRect(maskRect, rotatedrect, color, lineType);
		mask = maskRect & maskCircle;
	}

	/**
	 * @brief  ReadAsc              读取asc文件
	 * @param  file                 输入asc数据
	 * @param  images               原始干涉图
	 * @param  phase                解包裹数据
	 */
	inline void ReadAsc(const std::string& file,
		std::vector<cv::Mat>& images,
		cv::Mat& phase) {
		std::ifstream in(file);
		std::string str;

		getline(in, str);
		getline(in, str);

		int intensoriginx = -1;
		int intensoriginy = -1;
		int intenswidth = -1;
		int intensheight = -1;
		int nbuckets = -1;
		int intensrange = -1;

		in >> intensoriginx
			>> intensoriginy
			>> intenswidth
			>> intensheight
			>> nbuckets
			>> intensrange;

		int phaseoriginx = -1;
		int phaseoriginy = -1;
		int phasewidth = -1;
		int phaseheight = -1;

		in >> phaseoriginx
			>> phaseoriginy
			>> phasewidth
			>> phaseheight;

		getline(in, str);
		getline(in, str);
		getline(in, str);
		getline(in, str);

		int source = -1;
		float intfscalefactor = -1.;
		float wavelengthin = -1.;
		float nunericaperture = -1.;
		float obliquityfactor = -1.;
		float magnification = -1.;
		float camerares = -1.;                            // 相机分辨率 m/pix
		int timestamp = -1;

		in >> source
			>> intfscalefactor
			>> wavelengthin
			>> nunericaperture
			>> obliquityfactor
			>> magnification
			>> camerares
			>> timestamp;

		getline(in, str);
		getline(in, str);
		getline(in, str);

		int phaseres = -1;
		int phaseavgs = -1;
		int minimumareasize = -1;
		int disconaction = -1;
		float disconfilter = -1;
		int connectionorder = -1;
		int removetiltbias = -1;
		int datasign = -1;
		int codevtype = -1;

		in >> phaseres
			>> phaseavgs
			>> minimumareasize
			>> disconaction
			>> disconfilter
			>> connectionorder
			>> removetiltbias
			>> datasign
			>> codevtype;

		getline(in, str);
		getline(in, str);
		getline(in, str);
		getline(in, str);
		getline(in, str);

		const int intensinvalidvalue = 65535;

		images.clear();
		int tmp;
		for (int k = 0; k < nbuckets; ++k) {
			cv::Mat img(intensheight, intenswidth, CV_32FC1);
			for (int i = 0; i < intensheight; ++i) {
				for (int j = 0; j < intenswidth; ++j) {
					in >> tmp;
					if (tmp >= intensinvalidvalue) {
						img.at<float>(i, j) = 0.;
					}
					else {
						img.at<float>(i, j) = tmp * 255.f / intensrange;
					}
				}
			}
			images.emplace_back(img);
		}

		getline(in, str);
		getline(in, str);

		const int phaseinvalidvalue = 2147483640;
		int R = 32768;
		if (phaseres == 0) {
			R = 4096;
		}
		else if (phaseres == 1) {
			R = 32768;
		}

		phase = cv::Mat(phaseheight, phasewidth, CV_32FC1);
		for (int i = 0; i < phaseheight; ++i) {
			for (int j = 0; j < phasewidth; ++j) {
				in >> tmp;
				if (tmp >= phaseinvalidvalue) {
					phase.at<float>(i, j) = nanf("");
				}
				else {
					phase.at<float>(i, j) = tmp * (intfscalefactor * obliquityfactor) / R;
				}
			}
		}

		getline(in, str);
		getline(in, str);

		in.close();
	} // end ReadAsc

	inline void WriteCsv2(std::string file, const cv::Mat& m) {
		std::ofstream f(file);
		f << cv::format(m, cv::Formatter::FMT_CSV);
		f.close();
	}

	//删除面积过小的连通域
	inline void bwareaopen(cv::Mat src, cv::Mat &dst, double min_area) {
		dst = src.clone();
		std::vector<std::vector<cv::Point> > 	contours;
		std::vector<cv::Vec4i> 			hierarchy;
		cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
		if (!contours.empty() && !hierarchy.empty()) {
			std::vector<std::vector<cv::Point> >::const_iterator itc = contours.begin();
			while (itc != contours.end()) {
				cv::Rect rect = cv::boundingRect(cv::Mat(*itc));
				double area = contourArea(*itc);
				if (area < min_area) {
					for (int i = rect.y; i < rect.y + rect.height; i++) {
						uchar *output_data = dst.ptr<uchar>(i);
						for (int j = rect.x; j < rect.x + rect.width; j++) {
							if (output_data[j] == 255) {
								output_data[j] = 0;
							}
						}
					}
				}
				itc++;
			}
		}
	}
} // end calc

#endif
