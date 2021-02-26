/*****************************************************************
*
* fileName:  zernike.h
*
* function:  计算zernike系数及拟合曲面
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     计算zernike系数及拟合曲面

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef ZERNIKE_H
#define ZERNIKE_H

#include "baseFunc.h"

extern cv::Mat zernikeConvertMatrix;             // 正交多项式转换矩阵
extern cv::Mat identityMatrix;                   // 单位阵
extern cv::Mat zernikeConvertMatrixInv;          // 正交多项式转换矩阵的逆矩阵
extern cv::Mat identityMatrixInv;                // 单位阵的逆矩阵

void calcConvertMatrix();

enum ZERNIKE_METHOD {
	ZERNIKE_METHOD_CIRCLE = 0,
	ZERNIKE_METHOD_ORTHO
};

// 移除泽尼克相关项的标志，true为移除
struct REMOVE_ZERNIKE_FLAGS {
	bool positionFlag = true;                    // 去掉偏移量的标志
	bool tiltFlag = true;                        // 去掉倾斜量的标志
	bool powerFlag = false;                      // 去掉power的标志
	bool astFlag = false;                        // 去掉ast的标志
	bool comaFlag = false;                       // 去掉coma的标志
	bool sphericalFlag = false;                  // s去掉pherical的标志
};

class Zernike {
public:
	cv::Mat coef_display_4;                      // 最终显示的zernike 4项系数结果
	cv::Mat coef_display_9;                      // 最终显示的zernike 9项系数结果 （term >= 9）
	cv::Mat coef_display_16;                     // 最终显示的zernike 16项系数结果 （term >= 16）
	cv::Mat coef_display_25;                     // 最终显示的zernike 25项系数结果 （term >= 25）
	cv::Mat coef_display_36;                     // 最终显示的zernike 36项系数结果 （term >= 36）
	cv::Mat coef_display_37;                     // 最终显示的zernike 37项系数结果 （term >= 37）

	cv::Mat coefficient_4;                       // 4项zernike系数(zernike power, zernike tilt, concavity, Rms(Power), seidel tilt, seidel tilt clock, Iso Sag, Parallel Theta)
	cv::Mat coefficient_9;                       // 9项zernike系数 （term >= 9）
	cv::Mat coefficient_16;                      // 16项zernike系数 （term >= 16）
	cv::Mat coefficient_25;                      // 25项zernike系数 （term >= 25）
	cv::Mat coefficient_36;                      // 36项zernike系数 （term >= 36）
	cv::Mat coefficient_37;                      // 37项zernike系数 （term >= 37）

	cv::Mat fitting_all;                         // zernike系数拟合结果(ttv, fringes, zernike residual)
	cv::Mat fitting_rem_zer;                     // 去除用户设置的移除项之后拟合的结果，后期计算pv，rms等参数时，使用的拟合结果
	cv::Mat fitting_r0_r1_r2;                    // 移除前3项的拟合结果，计算ISO的Rmst时使用（前3项是指4项拟合系数的前3项结果）
	cv::Mat fitting_r0_r1_r2_r3;                 // 移除前4项的拟合结果，计算ISO的Irr，Rmsa, Rmsi时使用（前3项是指4项拟合系数的前4项结果）
	cv::Mat fitting_z8_z15_z24_z35_z36;          // zernike系数中表示球差的项数拟合结果，计算ISO的Rsi,Rmsa时使用
	cv::Mat zernike_residual;                    // 残差

	int term;                                    // zernike项数
	cv::Mat zerConMat;                           // 计算zernike系数时，用到的转换矩阵（37*37）
	cv::Mat zerConMatInv;                        // zernike系数进行曲面拟合时，用到的转换矩阵的逆矩阵（37*37）
	cv::Mat zernikeValue;                        // 37项zernike多项式的值（37*1）

	bool removeResidualFlag = false;             // 移除残差的标志
	bool calcFitR012Flag = false;                // 计算fitting_r0_r1_r2的标志
	bool calcFitR0123Flag = false;               // 计算fitting_r0_r1_r2_r3的标志
	bool calcFitZ8toZ36Flag = false;             // 计算fitting_z8_z15_z24_z35_z36的标志
	ZERNIKE_METHOD zernikeMethod = ZERNIKE_METHOD_CIRCLE;// 计算zernike系数时，选择转换矩阵的方法
	REMOVE_ZERNIKE_FLAGS removeZernikeFlags;     // 移除zernike多项式的标志

public:
	Zernike(int inputTerm = 37) : term(inputTerm) {}

	/**
	 * @brief ZernikeProcess                     zernike处理函数
	 * @param ang                                相位
	 * @param mag                                幅度
	 * @param mask                               mask
	 * @param phase                              解包裹相位
	 * @return
	 */
	void ZernikeProcess(const cv::Mat& ang,
		const cv::Mat& mag,
		const cv::Mat& mask,
		const cv::Mat& phase);

	/**
	* @brief ZernikeConvert 32FC1                计算zernike转换矩阵
	* @param _ang                                相位kAng129 or kAng259
	* @param _mag                                幅值kMag129 or kMag259
	* @param _mask square matrix                 n*n的mask
	* @param terms                               zernike阶数
	* @return                                    转换矩阵，计算圆的zernike拟合系数时，该矩阵为单位阵
	*/
	cv::Mat ZernikeConvert();

	/**
	* @brief ZernikeA 32FC1                      得到37项zernike多项式
	* @param ang                                 相位
	* @param mag                                 幅值
	* @param sampling                            抽样点坐标
	* @return                                    terms项zernike多项式
	*/
	cv::Mat ZernikeA(const cv::Mat& ang,
		const cv::Mat& mag,
		const std::vector<cv::Point>& sampling,
		int terms);

	// 得到37项zernike多项式(验证速度较快)
	cv::Mat ZernikeA2(const cv::Mat &ang,
		const cv::Mat &mag,
		const std::vector<cv::Point> &sampling,
		int terms);

	// 得到37项zernike多项式(验证速度最快)
	cv::Mat ZernikeA1(const cv::Mat &ang,
		const cv::Mat &mag,
		const std::vector<cv::Point> &sampling,
		int terms);

	/**
	 * @brief ZernikeFitting                  利用zernike多项式拟合曲面
	 * @param coefForFitting                  zernike系数
	 * @param mask                            mask
	 * @param maskPoints                      mask中不为0的点集
	 * @return                                拟合曲面
	 */
	cv::Mat ZernikeFitting(const cv::Mat &coefForFitting,
		const cv::Mat &mask,
		const std::vector<cv::Point> &maskPoints);

	/**
	* @brief ItemMask                        设置各种去除项
	* @param remove_piston                   piston
	* @param remove_tilt                     tilt
	* @param remove_power                    power
	* @param remove_ast                      ast
	* @param remove_coma                     coma
	* @param remove_spherical                spherical
	* @param terms                           阶数
	* @return
	*/
	inline cv::Mat ItemMask(bool remove_piston = true,
		bool remove_tilt = true,
		bool remove_power = false,
		bool remove_ast = false,
		bool remove_coma = false,
		bool remove_spherical = false,
		int terms = 36) {
		cv::Mat m(terms, 1, CV_8UC1, cv::Scalar(255));
		if (remove_piston) { m.at<uchar>(0, 0) = 0; }
		if (remove_tilt) { m.at<uchar>(1, 0) = 0; m.at<uchar>(2, 0) = 0; }
		if (remove_power) { m.at<uchar>(3, 0) = 0; }
		if (remove_ast) { m.at<uchar>(4, 0) = 0; m.at<uchar>(5, 0) = 0; }
		if (remove_coma) { m.at<uchar>(6, 0) = 0; m.at<uchar>(7, 0) = 0; }
		if (remove_spherical) { m.at<uchar>(8, 0) = 0; }
		return m;
	}


private:
	/**
	* @brief Calculates lower triangular matrix S, where A is a symmetrical matrix A=S*S'
	* @param A                                   正定的实对称阵
	* @param S                                   得到的下三角阵
	* @note  Cholesky分解
	*/
	void Cholf(const cv::Mat& A, cv::Mat& S);

	/**
	* @brief ZernikeB                            得到sampling处的数据,数据抽样
	* @param wave                                待抽样的数据，一般是解包裹数据
	* @param sampling                            抽样点处的坐标
	* @return                                    抽样后的数据
	*/
	cv::Mat ZernikeB(const cv::Mat& wave,
		const std::vector<cv::Point>& sampling);

	/**
	* @brief ZernikeCoefficient              求解正规方程A’A * dst = A'B中的dst,得到zernike系数
	* @param zernikeA                        zernike多项式
	* @param zernikeB                        抽样后的数据
	* @return                                zernike系数
	*/
	cv::Mat ZernikeCoefficient(const cv::Mat& zernikeA,
		const cv::Mat& zernikeB);

	/**
	 * @brief calcZernikeCoefficient          计算zernike系数
	 * @param ang                             相位
	 * @param mag                             幅度
	 * @param mask                            mask
	 * @param phase                           解包裹相位
	 * @return
	 */
	void calcZernikeCoefficient(const cv::Mat& ang,
		const cv::Mat& mag,
		const cv::Mat& mask,
		const cv::Mat& phase);

};

#endif // ZERNIKE_H


