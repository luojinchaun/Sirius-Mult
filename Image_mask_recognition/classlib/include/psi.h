/*****************************************************************
*
* fileName:  psi.h
*
* function:  (Phase shift interferometry)计算干涉图中各点的相位
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     计算相位

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef PSI_H
#define PSI_H

#include <omp.h>
#include "baseFunc.h"

#define PHASE_SHIFT_THR      10                     // 相移10°，认为已经对测量结果有较大影响了
#define IMG_0_SUB_4_STD_THR  30                     // 30已经是相对较大的结果了，相移准确无干扰时，一般都在10左右
#define STD_RATIO_THR        0.2                    // 标准的正弦函数，相移90°，偏差10°时，标准差比值为[0.9091, 1.1]

#define DISTINGUISHING_THR   160000                 // shift_std，与mask相关，mask越大，std相对较小
#define PHASE_SHIFT_STD_THR1 0.05                   // 相移准确且无干扰时，一般都在0.07左右，有干扰时，一般在0.03以下
#define PHASE_SHIFT_STD_THR2 0.04                   // 相移准确且无干扰时，一般都在0.06左右，有干扰时，一般在0.02以下

#define AIA_THR              0.001

// 相移方法
enum PSI_METHOD{
	PSI_METHOD_BUCKET4A_P = 0,
	PSI_METHOD_BUCKETB5A_H_P,
	PSI_METHOD_BUCKET5B_CS_P,
	PSI_METHOD_BUCKET9A_CS_P,
	PSI_METHOD_BUCKET9B_CS_P,
	PSI_METHOD_OPT_SEQUENCE
};

class Psi {
public:
	bool checkFalg;                                 // 是否进行相移检查
	bool isSingleHole;                              // 单孔标志，true：单孔，false:多孔
	cv::Mat phaseModulation;                        // 信号调制
	cv::Mat phase;                                  // 相移
	ERROR_TYPE errorType;                           // 错误类型
	PSI_METHOD psiMethod;                           // psi方法
	CHECK_THR checkThr;                             // check功能的阈值

public:
	Psi(bool isCheck, PSI_METHOD method, bool singleHoleFlag, CHECK_THR thr)
		: checkFalg(isCheck), psiMethod(method), isSingleHole(singleHoleFlag), checkThr(thr) {}
	
	/**
	* @brief PsiProcess        psi算法接口，计算包裹相位与相位调制度 gama = I'' / I'
	* @param srcs              输入的图像
	* @return
	*/
	void PsiProcess(std::vector<cv::Mat> srcs, cv::Mat mask);

	/**
	* @brief psiPretreatment   相移预处理(优选图像，检查相移是否准确)
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @return                  优选后的图像
	*/
	std::vector<cv::Mat> psiPretreatment(const std::vector<cv::Mat> srcs, cv::Mat mask);

	/**
	 * @brief ImgNum 移相算法所需的图片数
	 * @return
	 */
	inline unsigned int imgNum() {
		switch (psiMethod) {
		case PSI_METHOD_BUCKET4A_P:  return 4;
		case PSI_METHOD_BUCKETB5A_H_P:
		case PSI_METHOD_BUCKET5B_CS_P: return 5;
		case PSI_METHOD_BUCKET9A_CS_P:
		case PSI_METHOD_BUCKET9B_CS_P: return 9;
		case PSI_METHOD_OPT_SEQUENCE: return 5;
		default:
			return 0;
		}
	}

private:
	/**
	* @brief AIA               相移迭代算法
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @param iterNum           迭代次数
	* @return                  
	*/
	void AIA(std::vector<cv::Mat> srcs, cv::Mat mask, int iterNum);
	
	/**
	* @brief Todelta           由相位分布求解移相量
	* @param delta             移项角度
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @param a                 调制因子
	* @param b                 调制因子
	* @return
	*/
	void Todelta(float *delta, std::vector<cv::Mat> srcs, cv::Mat mask, float *a, float *b);
	
	// 由相位分布求解移相量 (速度较快)
	void Todelta2(float *delta, std::vector<cv::Mat> srcs, cv::Mat mask, float *a, float *b);

	/**
	* @brief Todelta           由移相量求解相位分布
	* @param delta             移项角度
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @param a                 调制因子
	* @param b                 调制因子
	* @return
	*/
	void Tophase(float *delta, std::vector<cv::Mat> srcs, cv::Mat mask, float *a, float *b);
	
	// 由移相量求解相位分布 (速度较快)
	void Tophase2(float *delta, std::vector<cv::Mat> srcs, cv::Mat mask, float *a, float *b);

	// 由移相量求解相位分布 (速度最快)
	void Tophase3(float *delta, std::vector<cv::Mat> srcs, cv::Mat mask, float *a, float *b);

	/**
	* @brief findGoodPics      找到相移较好的图像
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @return                  相移较好的干涉图
	*/
	std::vector<cv::Mat> findGoodPics(std::vector<cv::Mat> srcs, cv::Mat mask);

	/**
	* @brief calcShiftAndStd   防震算法，计算偏移角度与角度直方图的标准差
	* @param srcs0             输入的图像0
	* @param srcs1             输入的图像1
	* @param srcs3             输入的图像3
	* @param srcs4             输入的图像4
	* @param mask              输入的mask
	* @param data              输出的各个初始相位下的相移及标准差
	* @param idx               最优的一组干涉图的起始索引
	* @return                  
	*/
	void calcShiftAndStd(cv::Mat srcs0,
		cv::Mat srcs1,
		cv::Mat srcs3,
		cv::Mat srcs4,
		cv::Mat mask,
		float data[2][5],
		int idx);

	/**
	* @brief checkPsi          检查相移是否准确
	* @param srcs              输入的图像
	* @param mask              输入的mask
	* @return                  
	*/
	void checkPsi(const std::vector<cv::Mat> srcs, cv::Mat mask);

	/**
	 * @brief Bucket4A_P
	 * @note Phase Shift: 90; Purpose: General purposes.
	 * @param I CV_32FC1
	 * @return
	 */
	cv::Mat Bucket4A_P(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket4A_M    得到整个干涉图的数据调制系数gama = I'' / I'
	 * @param I
	 * @return
	 */
	cv::Mat Bucket4A_M(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket5A_H_P
	 * @note Phase Shift: 90; Purpose: Phase shift error correction.
	 * @param I
	 * @return
	 */
	cv::Mat Bucket5A_H_P(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket5A_H_M   得到整个干涉图的数据调制系数gama = I'' / I'
	 * @param I
	 * @return
	 */
	cv::Mat Bucket5A_H_M(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket5B_CS_P
	 * @note Phase Shift: 90; Purpose: Enhanced phase shift error correction.
	 * @param I
	 * @return
	 */
	cv::Mat Bucket5B_CS_P(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket5B_CS_M
	 * @param I
	 * @return           得到整个干涉图的数据调制系数gama = I'' / I'
	 */
	cv::Mat Bucket5B_CS_M(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket9A_CS_P
	 * @note Phase Shift: 90; Purpose: Enhanced phase shift error correction.
	 * @param I
	 * @return
	 */
	cv::Mat Bucket9A_CS_P(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket9A_CS_M
	 * @param I
	 * @return           得到整个干涉图的数据调制系数gama = I'' / I'
	 */
	cv::Mat Bucket9A_CS_M(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket9B_CS_P
	 * @note Phase Shift: 90; Purpose: Enhanced phase shift error correction.
	 * @param I
	 * @return
	 */
	cv::Mat Bucket9B_CS_P(const std::vector<cv::Mat>& I);

	/**
	 * @brief Bucket9B_CS_M
	 * @param I
	 * @return           得到整个干涉图的数据调制系数gama = I'' / I'
	 */
	cv::Mat Bucket9B_CS_M(const std::vector<cv::Mat>& I);
};

#endif
