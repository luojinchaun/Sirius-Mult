/*****************************************************************
*
* fileName:  algorithmMain.h
*
* function:  算法主函数接口
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     与应用层只留一个接口，减少耦合

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef ALGORITHMMAIN_H
#define ALGORITHMMAIN_H

#include "psi.h"
#include "psf.h"
#include "unwrap.h"
#include "filter.h"
#include "edgedetector.h"
#include "result.h"

// 孔的类型
enum HOLE_TYPE {
	HOLE_TYPE_CG = 0,                                      // 摄像孔
	HOLE_TYPE_IR,                                          // 油墨孔
	HOLE_TYPE_REVERSE1,                                    // 预留1
	HOLE_TYPE_REVERSE2                                     // 预留2
};

// 配置参数
struct CONFIG_PARAMS {
	bool isPhaseAverage = false;                           // 判断是否进入位相平均的状态
	bool isEdgeEroding = false;                            // 边缘腐蚀——缩进1个pixel
	bool isautomask = false;                               // 自动掩膜，跨区解包裹需要。
	bool isASCFile = false;                                // asc文件的标志（asc文件中的数据，不需要进行相移计算以及解包裹运算）
	bool checkFlag = false;                                // 检查位相图质量的标志
	bool removeErrorFlag = false;                          // 移除错误的标志，暂时未用
	bool removeResidualFlag = false;                       // 移除残差的标志
	bool isSingleHole = true;                              // 单孔标志，true：单孔，false:多孔（Pro版本标志，true:pro,false:ind）
	bool isFillSpikes = false;                             // 填充毛刺点的标志，true：填充，false：不填充
	unsigned int residuesNumThresh = 500;				   // 残差点个数阈值
	unsigned int span_automask_thresh = 40;                // 间隔区筛选的阈值
	int zernikeTerm = 37;                                  // zernike多项式的项数
	int unwrap_max_radius = 5;                             // 枝切法枝切线最大连接半径
	int edge_erode_r = 1;                                  // 腐蚀尺寸
	int debugGrade = 0;                                    // 打印信息的标志0：不打印任何信息；1：只打印计算结果；2：打印函数的开始与结束、计算结果；3：打印所有的信息
	float unwrap_span_adjust = -0.1f;                      // 跨区解包裹法调整因子
	float scaleFactor = 0.5f;                              // 比例因子
	float highPassFilterCoef = 0.01f;                      // 计算高通滤波半径的系数
	HOLE_TYPE holeType = HOLE_TYPE_CG;                     // 孔的类型
	CHECK_THR checkThr;                                    // 相移设置参数
	PSI_METHOD psiMethod = PSI_METHOD_BUCKET9A_CS_P;       // 相移计算方法
	UNWRAP_METHOD unwrapMethod = UNWRAP_METHOD_BRANCH_CUT; // 解包裹方法
	ZERNIKE_METHOD zernikeMethod = ZERNIKE_METHOD_CIRCLE;  // zernike计算方法
	FILTER_PARAMS filterParams;                            // 滤波去毛刺的参数
	EDGE_DETEC_PARAMS edgeDetecParams;                     // 边缘检测的相关输入参数 
	REMOVE_ZERNIKE_FLAGS removeZernikeFlags;               // zernike各种移除项的标志
	CALC_TOTAL_RST_FLAGS calcTotalResultFlags;             // 要计算的result中哪些项的标志
	CALC_RST_INPUT_PARAMS calcResultInputParams;           // 计算相关结果时，用户设置参数
};

// 输入数据
struct INPUT_DATAS {
	std::vector<cv::Mat> oriPhase;                         // 原始图像数据
	cv::Mat mask;                                          // 用户设置的掩膜
};

// 算法类
class AlgorithmMain {
public:
	cv::Mat roiMask;                                       // 最小包围圆的外接矩形
	cv::Mat retMask;                                       // 和输入图像大小相同的mask,
	cv::Mat maskEdge;                                      // mask的边缘，大小等于输入图像的大小

	cv::Mat unwrappedPhase;                                // 相位解包裹后的结果， 大小为输入图像的大小
	cv::Mat unwrappedPhase_original;                       // 未加任何处理的解包裹结果
	cv::Mat xSlice;                                        // 某一行的图形切片（单位：波长）
	cv::Mat ySlice;                                        // 某一列的图形切片（单位：波长）
	cv::Mat PSD_X;                                         // X切片对应的PSD图（经过对数处理，单位：波长^2*mm）
	cv::Mat PSD_Y;                                         // Y切片对应的PSD图（经过对数处理，单位：波长^2*mm）

	cv::Mat zernikeCoef_4;                                 // zernike系数
	cv::Mat zernikeCoef_9;                                 // zernike系数
	cv::Mat zernikeCoef_16;                                // zernike系数
	cv::Mat zernikeCoef_25;                                // zernike系数
	cv::Mat zernikeCoef_36;                                // zernike系数
	cv::Mat zernikeCoef_37;                                // zernike系数
	cv::Mat fittingSurface_all;                            // zernike拟合结果（含所有项）
	cv::Mat fittingSurface;                                // zernike拟合的结果， 大小为最小包围圆的外接矩形的大小（单位：波长）
	cv::Mat zernikeResidual;                               // 残差，大小为最小包围圆的外接矩形的大小（单位：波长）
	cv::Mat fittingSurface_fillSpikes;                     // 将毛刺点处的值用拟合的结果填充（单位：波长）
	 
	cv::Mat psf;                                           // PSF
	cv::Mat psfEnergy;                                     // psf energy
	cv::Mat mtf;                                           // MTF, 暂时没有计算
	cv::Mat intensity;                                     // intensity, 暂时没有计算，大小为最小包围圆的外接矩形的大小
	cv::Rect mask_rect;                                    // 掩膜在原图中的位置信息

	ERROR_TYPE errorType = ERROR_TYPE_NOT_ERROR;           // 错误类型
	TOTAL_RESULT totalResult;                              // 计算的总的结果
	CONFIG_PARAMS configParams;                            // 配置参数

public:
	AlgorithmMain() { }

	AlgorithmMain(CONFIG_PARAMS userSets) : configParams(userSets) {}

	// 算法处理主函数
	void process(INPUT_DATAS inputDatas);

	// 初始化对象中的矩阵，防止外面使用时崩溃
	void initMemberVar(int rows, int cols);

	// 修改配置文件,便于重新调用算法
	void modifyConfigParams(CONFIG_PARAMS userSets);

	// 通过解包裹图组得到拟合数据和其他结果
	void ResultFromUnwrapList(vector<cv::Mat> unwraplist, const cv::Mat mask);

	// 通过面形图得到结果数据
	TOTAL_RESULT ResultFromSurface(const cv::Mat fittingsurface, const cv::Mat surface_tilt, const cv::Mat mask);

private:
	// 根据位相图进行解包裹，zernike拟合以及各种结果的计算
	void processInterferogram(INPUT_DATAS inputDatas);

	// 根据位相图进行解包裹，得到解包裹后图
	void processGetUnwrappedPhase(INPUT_DATAS inputDatas);

	// 当读取的是asc文件时，算法处理函数
	void processASCFile(INPUT_DATAS inputDatas);

	// 设置zernike相关计算时的参数
	void setZernikeConfig(Zernike &zernike);

	// 改变外部输入的错误参数，防止程序崩溃
	void changeErrorParams();

	// 残差高通滤波
	void residualHighPassFilter(Zernike &zernike);

	// 获得zernike系数及拟合结果
	void getZernikeResult(Zernike zernike);

	// 获得毛刺点处的zernieke拟合结果
	void getSpikesValue(Zernike zernike, cv::Mat tmpMask);

	// 获得总的计算结果
	void getTotalResult(Result inputResult);

	// 获得计算powerX与powerY的参数
	void getPowerXYParams();

	/**
	* @brief removeNanPoint                     移除zernikeValue中不准确的值
	* @param originalMask                       滤波和去毛刺之前的原始mask
	* @param finalMask                          滤波和去毛刺之前的最终mask
	* @param zernike                            zernike
	* @param spikesZernikeValue                 毛刺点处的zernikeValue
	* @note  
	*/
	cv::Mat removeNanPoint(cv::Mat &originalMask,
		cv::Mat &finalMask,
		Zernike &zernike,
		cv::Mat &spikesZernikeValue);

	// 打印结果，后期正式版本中，去掉该函数
	void PrintResult();
};

/***********************************************************
* 以下是关于多线程的操作
************************************************************/

/**
* @brief calcPsiFilterUnwrapForThread          计算psi,unwrap,filter的线程 
* @param psi                                   Psi对象
* @param unwrap                                Unwrap对象
* @param images_roi                            输入数据
* @param filterParams                          输入滤波参数
* @param roiMask                               mask
* @note
*/
void calcPsiFilterUnwrapForThread(Psi &psi,
	Unwrap &unwrap,
	const std::vector<cv::Mat> &images_roi,
	const FILTER_PARAMS &filterParams,
	const cv::Mat &roiMask);

/**
* @brief calcZernikeForThread                  计算zernike多项式的线程
* @param zernike                               Zernike对象
* @param ang_roi                               相位
* @param mag_roi                               幅值
* @param roiMask                               mask
* @note
*/
void calcZernikeForThread(Zernike &zernike,
	const cv::Mat &ang_roi,
	const cv::Mat& mag_roi,
	const cv::Mat &roiMask);

// 在算法中开辟多线程，
void mulHolesForThread(AlgorithmMain &algorithm,
	const INPUT_DATAS &inputDatas);

// 在算法中开辟多线程，再封装一层算法
class AlgorithmMainForThread {
public:
	std::vector<AlgorithmMain> algorithmList;                  // 算法分析的对象
	std::vector<cv::Mat> imageList;                            // 原始数据,
	std::vector<cv::Rect> roiList;                             // 截取图像参数
	cv::Mat mask;
public:
	AlgorithmMainForThread() {}
	void operator()();
};

#endif