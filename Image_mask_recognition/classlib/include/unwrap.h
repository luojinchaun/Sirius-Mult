/*****************************************************************
*
* fileName:  unwrap.h
*
* function:  相位解包裹
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     解包裹算法

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef UNWRAP_H
#define UNWRAP_H

#include <Windows.h>
#include "baseFunc.h"
using namespace std;

enum UNWRAP_METHOD {
	UNWRAP_METHOD_BRANCH_CUT = 0,          //枝切法
	UNWRAP_METHOD_HISTOGRAM=1,             //质量图（未完成）
	UNWRAP_METHOD_SPAN_BRANCH_CUT=2,       //跨区枝切法
	UNWRAP_METHOD_DCT=3,                   //DCT全局法（待完善）
	UNWRAP_METHOD_RESERVE3
};

class Unwrap {
public:
	ERROR_TYPE errorType;                  // 错误类型
	UNWRAP_METHOD unwrapMethod;            // 解包裹的方法
	cv::Mat unwrappedPhase;                // 解包裹后的相位
	cv::Mat unwrappedPhase_original;       // 存储原始解包裹后的相位
	int maxBoxRadius;                      // 枝切法解包裹的最大搜索框的半径
	int residuesNumThresh;                 // 残差点个数阈值
	int rowref;                            // 参考起始点的行坐标
	int colref;                            // 参考起始点的列坐标
	float adjust;                          // 跨区枝切法的周期调整因子

public:
	Unwrap(int inputBoxRadius = 20, int inputThresh = 150, float inputadjust=0.05,UNWRAP_METHOD Method = UNWRAP_METHOD_BRANCH_CUT)
		: maxBoxRadius(inputBoxRadius), residuesNumThresh(inputThresh),adjust(inputadjust), unwrapMethod(Method){}

	/**
	* @brief UnwrapProcess            解包裹主运行程序
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @Param mask                     掩膜
	*/
	void UnwrapProcess(cv::Mat &phase,
		const cv::Mat &modulation,
		const cv::Mat &mask);

	// 点与点的求余运算 a / b
	inline float modf(float a, float b) {
		if (b == 0.0f) {
			return a;
		}
		else {
			return a - b * floorf(a / b);
		}
	}

	/**
	* @brief getUnwrappedPhase        获得解包裹数据
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @param branchcuts               枝切线
	* @param mask                     掩膜
	* @return                         解包裹后的结果
	*/
	inline cv::Mat getUnwrappedPhase(const cv::Mat& phase,
		const cv::Mat& modulation,
		const cv::Mat& branchcuts,
		const cv::Mat& mask) {

		// 找到起始参考点坐标，取的是调制度大于某阈值的所有点的中心点
		GetRefPoint(modulation, branchcuts);

		// 图像全白或者全黑时，会出现崩溃的Bug
		if (errorType == ERROR_TYPE_UNWRAP_ERROR) {
			return phase.clone();
		}
		// 四邻域洪水填充法解包裹
		// cv::Mat unwrappedphase = ptrGoldsteinFloodFill(phase, modulation, branchcuts, mask);
		// cv::Mat unwrappedphase = ptrGoldsteinFloodFill2(phase, modulation, branchcuts, mask);
		cv::Mat unwrappedphase = ptrGoldsteinFloodFill3(phase, modulation, branchcuts, mask);
		return unwrappedphase;
	}

private:
	// 矩阵与矩阵的求余运算 a ./ b
	cv::Mat modf(const cv::Mat& a, const cv::Mat& b);
	
	// 矩阵与点的求余运算 a / b 
	cv::Mat modf(const cv::Mat& a, float b);

	/*--------------------- BranchCuts -------------------------*/
	/**
	* @brief UnwrapBranchCuts         枝切法相位解包裹
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @Param mask                     掩膜
	* @return                         
	*/
	int UnwrapBranchCuts(cv::Mat &phase,
		const cv::Mat &modulation,
		const cv::Mat &mask);

	/**
	* @brief PhaseResidues            计算包裹相位残差
	* @param phase CV_32FC1           包裹相位
	* @return CV_8SC1 -127,0,127      残差
	*/
	cv::Mat PhaseResidues(const cv::Mat& phase);

	/**
	* @brief ResiduesCount            统计残差点的个数
	* @param residues                 PhaseResidues计算得到的残差
	* @Param mask                     掩膜
	* @return                         残差点个数
	*/
	inline int ResiduesCount(const cv::Mat& residues,
		const cv::Mat& mask);

	/**
	* @brief ResiduesCount            根据残差得到枝切线
	* @param residues                 PhaseResidues计算得到的残差
	* @Param mask                     掩膜
	* @Param maxboxradius             最大搜索框的半径
	* @return                         枝切线
	*/
	cv::Mat BranchCuts(const cv::Mat& residues,
		const cv::Mat& mask,
		int maxboxradius = 20);

	/**
	* @brief PlaceBranchCutInternal   在两点之间放置枝切线
	* @param branchcuts               枝切线
	* @param r0                       0点行坐标
	* @param c0                       0点列坐标
	* @param r1                       1点行坐标
	* @param c1                       1点列坐标
	* @return BranchCuts              枝切线
	*/
	void PlaceBranchCutInternal(cv::Mat& branchcuts,
		int r0,
		int c0,
		int r1,
		int c1);

	/**
	* @brief getCoordinates           查找不为0的点的坐标
	* @param adjoin                   近邻点
	* @param rows                     mask的rows
	* @param cols                     mask的cols
	* @param Coordinate               近邻点的坐标
	* @param count                    偏离解包裹起始点的个数
	* @retur                          不为0的点的个数
	*/
	int getCoordinates(uchar **adjoin,
		int rows,
		int cols,
		int **Coordinate,
		int count);

	/**
	* @brief getCoordinates           查找不为0的点的坐标
	* @param adjoin                   近邻点
	* @param rows                     mask的rows
	* @param cols                     mask的cols
	* @param Coordinate               近邻点的坐标
	* @param count                    偏离解包裹起始点的个数
	* @retur                          不为0的点的个数
	*/
	int getCoordinates2(uchar *adjoin,
		int rows,
		int cols,
		int **Coordinate,
		int count);

	/**
	* @brief GoldsteinFloodFill       四邻域洪水填充法解包裹
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @param branchcuts               枝切线
	* @param mask                     掩膜
	* @return                         解包裹后的结果
	*/
	cv::Mat GoldsteinFloodFill(const cv::Mat& phase,
		const cv::Mat& modulation,
		const cv::Mat& branchcuts,
		const cv::Mat& mask);

	/**
	* @brief ptrGoldsteinFloodFill    四邻域洪水填充法解包裹,用ptr替换at
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @param branchcuts               枝切线
	* @param mask                     掩膜
	* @return                         解包裹后的结果
	*/
	cv::Mat ptrGoldsteinFloodFill(const cv::Mat& phase,
		const cv::Mat& modulation,
		const cv::Mat& branchcuts,
		const cv::Mat& mask);

	/**
	* @brief ptrGoldsteinFloodFill2   四邻域洪水填充法解包裹，用ptr替换at,用getCoordinates替换findNonZero；
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @param branchcuts               枝切线
	* @param mask                     掩膜
	* @return                         解包裹后的结果
	*/
	cv::Mat ptrGoldsteinFloodFill2(const cv::Mat &phase,
		const cv::Mat &modulation, 
		const cv::Mat &branchcuts,
		const cv::Mat &mask);

	/**
	* @brief ptrGoldsteinFloodFill2   四邻域洪水填充法解包裹，用ptr替换at,用getCoordinates替换findNonZero,用一维数组替换二维数组
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @param branchcuts               枝切线
	* @param mask                     掩膜
	* @return                         解包裹后的结果
	*/
	cv::Mat ptrGoldsteinFloodFill3(const cv::Mat &phase,
		const cv::Mat &modulation,
		const cv::Mat &branchcuts,
		const cv::Mat &mask);

	/**
	* @brief GetRefPoint              获得起始参考点的坐标
	* @param modulation               调制度
	* @param branchcuts               枝切线
	*/
	void GetRefPoint(const cv::Mat& modulation,
		const cv::Mat& branchcuts);

	/*--------------------- SpanBranchCuts -------------------------*/
	/**
	* @brief UnwrapSpanBranchCuts     跨区枝切法解包裹
	* @param phase                    包裹相位
	* @param modulation               调制度
	* @Param mask                     掩膜
	*/
	int UnwrapSpanBranchCuts(cv::Mat &phase,
		const cv::Mat &modulation,
		const cv::Mat &mask,
		const float adjust);

	/**
    * @brief UnwrapSpanCalcTile       跨区解包裹法计算参考连通区的拟合面(平面)
    * @param unwrap_phase             参考区的解包裹后相位
    * @Param mask                     参考区的掩膜
    */
	cv::Mat UnwrapSpanCalcTile(const cv::Mat &unwrap_phase,
		const cv::Mat &mask);

	/*--------------------- DCT（待完善） -------------------------*/
	//void DctProcess(cv::Mat &phase,
	//	const cv::Mat &modulation,
	//	const cv::Mat &mask);
};

#endif // UNWRAP_H

