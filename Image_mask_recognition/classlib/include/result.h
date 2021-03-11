/*****************************************************************
*
* fileName:  result.h
*
* function:  测试结果
*
* author:    sjt&ztb
*
* date:      2021.1.26
*
* notes:     计算各种测量量

* Copyright: Shanghai Shineoptics Technology Co. , Ltd
*
*********************************************************************/

#pragma once
#ifndef RESULT_H
#define RESULT_H

#include <string>
#include "zernike.h"
#include "baseFunc.h"

// 计算结果的单位类型（ISO的单位是固定的，就是用户输入的ISO波长）
enum UNIT_TYPE {
	UNIT_TYPE_WAVE = 0,                          // 以波长为单位
	UNIT_TYPE_FR,                                // 以Fr为单位
	UNIT_TYPE_NM,                                // 以nm为单位
	UNIT_TYPE_UM,                                // 以um为单位
	UNIT_TYPE_MM                                 // 以mm为单位
};

// GRMS计算方法
enum GRMS_METHOD {
	GRMS_BASIC_DIFF = 0,                         // 最简单差分法
	GRMS_CENTRAL_DIFF=1                          // 中心差分法
};

// 计算相关结果时，用到的用户设置参数
struct CALC_RST_INPUT_PARAMS {
	bool isUseAngleUnit = false;                 // 以角度为单位的标志，true:角度，false:弧度,默认false（TiltClock, AstClock, ComaClock）
	int row = 0;                                 // 计算孔径aperture时，用到的宽度
	int width = 0;                               // 计算sizeX时用到的宽度
	int height = 0;                              // 计算sizeY时用到的高度
	float powerX = 0.0f;						 // 计算powerX时的参数x^2
	float powerY = 0.0f;						 // 计算powerY时的参数y^2
	float px_1mm = 60.0f;                        // 每mm代表的像素个数
	float refractiveIndex = 1.5f;                // 输入的玻璃的折射率
	float testWavelength = 632.8f;               // 输入的激光器的发出的光的波长
	float ISOWavelength = 546.0f;                // 计算ISO参数时，输入的ISO波长
	float disPlayWavelength = 632.8f;            // 以波长或者FR为显示单位时，输入的波长
	float scaleFactorForHoleType = 1.0f;         // 根据不同孔的类型，设置的比例因子(摄像孔:1；油墨孔:1.485461≈940/632.8)
	UNIT_TYPE unitType = UNIT_TYPE_WAVE;         // 显示结果的单位，默认值为波长
	GRMS_METHOD grmsMethod = GRMS_CENTRAL_DIFF;  // GRMS计算方法，默认为Zygo采用的中心差分法
};

// 计算pv,rms相关项的标志，true为计算
struct CALC_PV_RMS_RST_FLAGS {
	// pv
	bool pvFlag = true;
	bool pvxFlag = false;
	bool pvyFlag = false;
	bool pvxyFlag = false;
	bool pvrFlag = false;
	bool pvresFlag = false;
	// rms
	bool rmsFlag = true;
	bool rmsxFlag = false;
	bool rmsyFlag = false;
	bool rmsxyFlag = false;
	bool rmsresFlag = false;
};

// Pv及rms的相关计算结果
struct PV_RMS_RESULT {
	float pv;                                    // zernike系数拟合结果的pv,透过波前的光程差
	float pvr;                                   // pvr = pv(zernike)+ 3 * rms(res)
	float pv_res;                                // 残差的pv
	float pv_x;                                  // zernike系数拟合结果某一行的pv
	float pv_y;                                  // zernike系数拟合结果某一列的pv
	float pv_xy;                                 // zernike系数拟合结果某一列和某一列的pv， pv_xy = (pv_x + pv_y) / 2

	float rms;                                   // zernike系数拟合结果的均方根
	float rms_res;                               // 残差的均方根
	float rms_x;                                 // zernike系数拟合结果某一行的均方根
	float rms_y;                                 // zernike系数拟合结果某一列的均方根
	float rms_xy;                                // zernike系数拟合结果某一列和某一列的rms， rms_xy = (rms_x + rms_y) / 2
};

// 计算zernike,seidel相关项的标志，true为计算
struct CALC_ZER_SEI_RST_FLAGS {
	// zernike
	bool zernikeTiltFlag = false;
	bool zernikePowerFlag = false;
	bool zernikePowerXFlag = false;
	bool zernikePowerYFlag = false;
	bool zernikeAstFlag = false;
	bool zernikeComaFlag = false;
	bool zernikeSphericalFlag = false;
	// seidel
	bool seidelTiltFlag = false;
	bool seidelFocusFlag = false;
	bool seidelAstFlag = false;
	bool seidelComaFlag = false;
	bool seidelSphericalFlag = false;
	// angle
	bool seidelTiltClockFlag = false;
	bool seidelAstClockFlag = false;
	bool seidelComaClockFlag = false;
	// rms
	bool rmsPowerFlag = false;
	bool rmsAstFlag = false;
	bool rmsComaFlag = false;
	bool rmsSaFlag = false;
};

// zernike和seidel的相关计算结果
struct ZERNIKE_SEIDEL_RESULT {
	// zernike 像差
	float zernikeTilt;                           // zernike tilt
	float zernikePower;                          // zernike power
	float zernikePowerX;                         // zernike power x
	float zernikePowerY;                         // zernike power y
	float zernikeAst;                            // zernike ast
	float zernikeComa;                           // zernike coma
	float zernikeSpherical;                      // zernike Spherical
	// seidel 像差
	float seidelTilt;                            // seidel tilt
	float seidelFocus;                           // seidel focus
	float seidelAst;                             // seidel Astigmatism
	float seidelComa;                            // seidel coma
	float seidelSpherical;                       // seidel Spherical
	// seidel 像差角度
	float seidelTiltClock;                       // seidel tilt clock
	float seidelAstClock;                        // seidel ast clock
	float seidelComaClock;                       // seidel coma clock
	// rms
	float rms_power;                             // zernikePower / 2 / 3^0.5
	float rms_ast;                               // seidelAst / 2 / 6^0.5
	float rms_coma;                              // seidelComa / 6 / 2^0.5
	float rms_sa;                                // seidelSpherical / 6 / 5^0.5
};

// 计算ISO相关项的标志，true为计算
struct CALC_ISO_RST_FLAGS {
	bool sagFlag = false;
	bool irrFlag = false;
	bool rsiFlag = false;
	bool rmstFlag = false;
	bool rmsaFlag = false;
	bool rmsiFlag = false;
};

// ISO指标的相关计算结果
struct ISO_RESULT {
	float sag;                                   // 弧矢误差
	float irr;                                   // 不规则偏差
	float rsi;                                   // 旋转对称不规则偏差
	float rmst;                                  // 总表面偏差的均方根
	float rmsa;                                  // 不规则偏差的均方根
	float rmsi;                                  // 非旋转对称偏差的均方根
};

// 计算总结果相关项的标志，true为计算
struct CALC_TOTAL_RST_FLAGS {
	bool ttvFlag = false;                        // 几何轮廓的厚度差
	bool fringesFlag = false;                    // 条纹数
	bool strehlFlag = false;                     // 计算斯特列尔比的标志
	bool apertureFlag = false;                   // 口径
	bool parallelDegreeFlag = false;             // 平行度测试标志，该参数为true时，必须计算口径
	bool sizeXFlag = false;                      // x方向的尺寸
	bool sizeYFlag = false;                      // y方向的尺寸
	bool parallelThetaFlag = false;              // 平行角度
	bool thicknessFlag = false;                  // 测量厚度
	bool crinkleFlag = false;                    // ？
	bool grmsFlag = false;                       // 是否计算grms   
	bool categoryFlag = false;                   // 是否合格
	bool concavityFlag = false;                  // 凹度，-1表示凸，1表示凹           
	CALC_ISO_RST_FLAGS isoResultFlags;           // ISO指标的相关计算结果
	CALC_PV_RMS_RST_FLAGS pvRmsResultFlags;      // pv和rms的相关计算结果	
	CALC_ZER_SEI_RST_FLAGS zerSeiResultFlags;    // zernike和seidel的相关计算结果
};

// 计算结果
struct TOTAL_RESULT {
	ISO_RESULT isoResult;                        // ISO指标的相关计算结果
	PV_RMS_RESULT pvRmsResult;                   // pv和rms的相关计算结果
	ZERNIKE_SEIDEL_RESULT zerSeiResult;          // zernike和seidel的相关计算结果
	// others 
	float mean;                                  // 面形的平均值
	float ttv;                                   // 几何轮廓的厚度差
	float fringes;                               // 条纹数
	float strehl;                                // 斯特列尔比
	float aperture;                              // 口径
	float sizeX;                                 // x方向的尺寸
	float sizeY;                                 // y方向的尺寸
	float parallelTheta;                         // 平行角度
	float thickness;                             // 测量厚度
	float crinkle;                               // ？
	float grms;                                  // grms（波前梯度均方根）
	bool category;                               // 是否合格
	int concavity;                               // 凹度，-1表示凸，1表示凹
	int pointnumber;                             // 区域像素点个数
};

// Result类
class Result {
public:
	CHECK_THR checkThr;                          // check功能使用的阈值
	TOTAL_RESULT totalResult;                    // 测试结果
	CALC_TOTAL_RST_FLAGS calcTotalResultFlags;   // 是否计算该项结果的标志，true，计算，false:不计算
	CALC_RST_INPUT_PARAMS calcResultInputParams; // 计算相关结果时，用到的用户设置参数
	ERROR_TYPE errorType = ERROR_TYPE_NOT_ERROR; // 错误类型
	bool isSingleHole = true;                    // 是否是单孔模式，true:单孔，false:多孔
	float ISOResultCoef;                         // 计算ISO结果时的转换系数
	float otherResultCoef;                       // 计算除了ISO结果外的其他结果时的转换系数
	float angleCoef;                             // 计算TiltClock, AstClock, ComaClock时的角度转换系数
	cv::Mat psdx;                                // X切片对应的psd（取对数）
	cv::Mat psdy;                                // Y切片对应的psd（取对数）

public:
	Result(CALC_TOTAL_RST_FLAGS inputFlags, CALC_RST_INPUT_PARAMS inputParams, CHECK_THR thr):
		calcTotalResultFlags(inputFlags), calcResultInputParams(inputParams), checkThr(thr) {}

	// 初始化参数
	void initConfig(CALC_TOTAL_RST_FLAGS inputFlags, CALC_RST_INPUT_PARAMS inputParams);

	// 根据mask设置计算sizeX与sizeY的参数
	void setSize(cv::Mat& mask);

	// 计算各种结果
	void ResultProcess(Zernike zernike);

	// 计算面形图结果
	void GetSurfaceResult(const cv::Mat surface, const cv::Mat surface_tilt, const cv::Mat mask);

	/**
	* @brief Pv                            计算拟合曲面的Pv
	* @param src                           拟合曲面
	* @param pv                            Pv = max - min
	* @param min                           最小值
	* @param max                           最大值
	*/
	inline void PvMinMax(const cv::Mat& src,
		float& pv,
		float& min,
		float& max) {
		double minv;
		double maxv;
		//计算mask区域的最大值与最小值;
		cv::minMaxIdx(src, &minv, &maxv, nullptr, nullptr, src == src);
		pv = (float)(maxv - minv);
		min = (float)minv;
		max = (float)maxv;
	}

	/**
	* @brief Pvxy                          分别计算中间一列和一行的Pv，然后取两者的均值
	* @param src                           拟合曲面
	* @param pv                            计算结果
	*/
	inline void Pvxy(const cv::Mat& src,
		float &pv) {
		double minv;
		double maxv;
		cv::Mat t;
		t = src.row(src.rows / 2);          //选取中间的一行数据
		//计算最大值与最小值
		cv::minMaxIdx(t, &minv, &maxv, nullptr, nullptr, t == t);
		float pvx = (float)(maxv - minv);
		t = src.col(src.cols / 2);          //选取中间的一列数据
		//计算最大值与最小值
		cv::minMaxIdx(t, &minv, &maxv, nullptr, nullptr, t == t);
		float pvy = (float)(maxv - minv);

		pv = (float)(pvx + pvy) / 2.0f;          //取两者的均值
	}

	/**
	* @brief Rms                           计算rms均方根
	* @param src                           拟合曲面
	* @param rms                           计算结果
	*/
	inline void Rms(const cv::Mat& src,
		float &rms) {
		cv::Mat tmp;
		cv::pow(src, 2.0, tmp);
		float mean_tmp = (float)(cv::mean(tmp, tmp == tmp)[0]);     // 计算均值(nan != nan)
		rms = std::sqrt(mean_tmp);          //得到rms
	}

	/**
	* @brief Rmsxy                         分别计算中间一列和一行的rms均方根，然后取两者的均值
	* @param src                           拟合曲面
	* @param rms                           计算结果
	*/
	inline void Rmsxy(const cv::Mat& src,
		float &rms) {
		cv::Mat tmp;
		cv::pow(src.row(src.rows / 2), 2.0, tmp);             //选取中间的一行数据
		double mean_tmp_x = cv::mean(tmp, tmp == tmp)[0];     // 计算x均值(nan != nan)
		float rms_x = (float)std::sqrt(mean_tmp_x);

		cv::pow(src.col(src.cols / 2), 2.0, tmp);             //选取中间的一列数据
		double mean_tmp_y = cv::mean(tmp, tmp == tmp)[0];     // 计算y均值(nan != nan)
		float rms_y = (float)std::sqrt(mean_tmp_y);
		rms = (rms_x + rms_y) / 2.0f;
	}

	/**
	* @brief Mean                          计算面形图的均值
	* @param mean                          计算结果
	*/
	inline void Mean(const cv::Mat& src,
		float &mean) {
		mean= (float)(cv::mean(src, src == src)[0]);        //得到非nan区的均值
	}

	/**
	* @brief ZernikeTilt                    计算Zernike Tilt
	* @param coefficient                    zernike系数
	* @param zerniketilt                    zernike tilt
	*/
	inline void ZernikeTilt(const cv::Mat& coefficient,
		float& zerniketilt) {
		zerniketilt = 2.0f * std::sqrtf(
			std::powf(coefficient.at<float>(1), 2.0f) +
			std::powf(coefficient.at<float>(2), 2.0f));
	}

	/**
	* @brief ZernikePower                   计算Zernike power
	* @param coefficient_4                  四阶多项式拟合，计算出的系数
	* @param zernikepower                   zernike power
	*/
	inline void ZernikePower(const cv::Mat& coefficient_4,
		float& zernikepower,
		int& concavity) {
		zernikepower = 2.0f * coefficient_4.at<float>(3);
		if (zernikepower > 0) {
			concavity = -1;
		}
		else {
			concavity = 1;
		}
	}

	/**
	* @brief ZernikeAst                     计算Zernike Ast
	* @param coefficient                    zernike系数
	* @param zernikeast                     Zernike Ast
	*/
	inline void ZernikeAst(const cv::Mat& coefficient,
		float& zernikeast) {
		zernikeast = std::sqrtf(std::powf(coefficient.at<float>(4), 2.0) +
			std::powf(coefficient.at<float>(5), 2.0));
	}

	/**
	* @brief ZernikeComa                    计算Zernike Coma
	* @param coefficient                    zernike系数
	* @param zernikecoma                    Zernike coma
	*/
	inline void ZernikeComa(const cv::Mat& coefficient,
		float& zernikecoma) {
		zernikecoma;
		zernikecoma = std::sqrtf(std::powf(coefficient.at<float>(6), 2.0) +
			std::powf(coefficient.at<float>(7), 2.0));
	}

	/**
	* @brief ZernikeSpher                   计算Zernike Spherical
	* @param coefficient                    zernike系数
	* @param zernikespher                   Zernike Spherical
	*/
	inline void ZernikeSpher(const cv::Mat& coefficient,
		float& zernikespher) {
		zernikespher = coefficient.at<float>(8);
	}

	/**
	* @brief SeidelTilt                     计算Seidel Tilt
	* @param coefficient                    zernike系数
	* @param seideltilt                     Seidel Tilt
	*/
	inline void SeidelTilt(const cv::Mat& coefficient,
		float& seideltilt) {

		seideltilt = std::sqrtf(
			std::powf((coefficient.at<float>(1) - 2 * coefficient.at<float>(6)), 2.0) +
			std::powf((coefficient.at<float>(2) - 2 * coefficient.at<float>(7)), 2.0));
	}

	/**
	* @brief SeidelTiltClock                计算Seidel Tilt Clock
	* @param coefficient                    zernike系数
	* @param seideltiltclock                Seidel Tilt Clock
	*/
	inline void SeidelTiltClock(const cv::Mat& coefficient,
		float& seideltiltclock) {
		// 返回值是弧度
		seideltiltclock = std::atan2f(
			(coefficient.at<float>(2) - 2 * coefficient.at<float>(7)),
			(coefficient.at<float>(1) - 2 * coefficient.at<float>(6)));
	}

	/**
	* @brief SeidelFocus                    计算Seidel Focus
	* @param coefficient                    zernike系数
	* @param seidelfocus                    Seidel Focus
	*/
	inline void SeidelFocus(const cv::Mat& coefficient,
		float& seidelfocus) {
		seidelfocus = 2.0f * coefficient.at<float>(3) - 6.0f * coefficient.at<float>(8);
	}

	/**
	* @brief SeidelAst                      计算Seidel Ast
	* @param coefficient                    zernike系数
	* @param seidelast                      Seidel Ast
	*/
	inline void SeidelAst(const cv::Mat& coefficient,
		float& seidelast) {
		seidelast = 2.0f * std::sqrtf(std::powf(coefficient.at<float>(4), 2.0f) +
			std::powf(coefficient.at<float>(5), 2.0f));
	}

	/**
	* @brief SeidelAstClock                 计算Seidel Ast Clock
	* @param coefficient                    zernike系数
	* @param seidelastclock                 Seidel Ast Clock
	*/
	inline void SeidelAstClock(const cv::Mat& coefficient,
		float& seidelastclock) {
		// 结果为弧度
		seidelastclock = 0.5f * std::atan2f(coefficient.at<float>(5), coefficient.at<float>(4));
	}

	/**
	* @brief SeidelComa                     计算Seidel Coma
	* @param coefficient                    zernike系数
	* @param seidelcoma                     Seidel Coma
	*/
	inline void SeidelComa(const cv::Mat& coefficient,
		float& seidelcoma) {
		seidelcoma = 3.0f * std::sqrtf(std::powf(coefficient.at<float>(6), 2.0) +
			std::powf(coefficient.at<float>(7), 2.0));
	}

	/**
	* @brief SeidelComaClock                计算Seidel Coma Clock
	* @param coefficient                    zernike系数
	* @param seidelcomaclock                Seidel Coma Clock
	*/
	inline void SeidelComaClock(const cv::Mat& coefficient,
		float& seidelcomaclock) {
		seidelcomaclock = std::atan2f(coefficient.at<float>(7),
			coefficient.at<float>(6));
	}

	/**
	* @brief SeidelSpher                    计算Seidel Spher
	* @param coefficient                    zernike系数
	* @param seidelspher                    Seidel Spher
	*/
	inline void SeidelSpher(const cv::Mat& coefficient,
		float& seidelspher) {
		seidelspher = 6.0f * coefficient.at<float>(8);
	}

	/**
	* @brief Ttv                            计算Ttv
	* @param fitting_all                    zernike系数拟合结果，（阶数 = terms）
	* @param refractiveindex                折射率
	* @param ttv                            ttv
	*/
	inline void Ttv(const cv::Mat& fitting_all,
		float refractiveindex,
		float& ttv) {
		double minv;
		double maxv;
		cv::minMaxIdx(fitting_all, &minv, &maxv, nullptr, nullptr, fitting_all == fitting_all);
		ttv = (float)(maxv - minv) / (refractiveindex - 1.0f);
	}

	/**
	* @brief Fringes fr in testwavelengthnm
	* @param fitting_all
	* @param fringes
	* @param in
	* @param testwavelengthnm
	*/
	inline void Fringes(const cv::Mat& fitting_all,
		float& fringes) {
		double minv;
		double maxv;
		cv::minMaxIdx(fitting_all, &minv, &maxv,
			nullptr, nullptr, fitting_all == fitting_all);
		fringes = (float)(maxv - minv);
	}

	/**
	* @brief IsoSag fr in ISO wavelength_nm
	* @param coefficient_4
	* @param sag
	*/
	inline void IsoSag(const cv::Mat& coefficient_4,
		float& sag) {
		sag = abs(2.0f * coefficient_4.at<float>(3));
	}

	/**
	* @brief IsoIrr fr in ISO wavelength_nm
	* @param fitting_r0_r1_r2_r3
	* @param irr
	*/
	inline void IsoIrr(const cv::Mat& fitting_r0_r1_r2_r3,
		float& irr) {
		double minv;
		double maxv;
		cv::minMaxIdx(fitting_r0_r1_r2_r3, &minv, &maxv,
			nullptr, nullptr, fitting_r0_r1_r2_r3 == fitting_r0_r1_r2_r3);
		irr = (float)(maxv - minv);
	}

	/**
	* @brief IsoRsi fr in ISO wavelength_nm
	* @param fitting_8_15_24_35_36
	* @param rsi
	*/
	inline void IsoRsi(const cv::Mat& fitting_z8_z15_z24_z35_z36,
		float& rsi) {
		double minv;
		double maxv;
		cv::minMaxIdx(fitting_z8_z15_z24_z35_z36, &minv, &maxv, nullptr,
			nullptr, fitting_z8_z15_z24_z35_z36 == fitting_z8_z15_z24_z35_z36);
		rsi = (float)(maxv - minv);
	}

	/**
	* @brief ParallelTheta
	* @param paralleltheta unit: s
	* @param fringes unit: fr
	* @param refractiveindex
	* @param diameter unit: mm
	*/
	inline void ParallelTheta(float& paralleltheta,
		float tilt,
		float refractiveindex,
		float diameter) {
		paralleltheta = 206.246f *0.6328* tilt / diameter / (refractiveindex - 1.0f);
		//paralleltheta = 0.206246f * 632.8f *fringes / diameter / (2*refractiveindex );//菲索型是2*n，泰曼型是2*（n-1）。详见论文：《平板玻璃的平行度快速检测研究》
	}

private:
	// 计算单位转换系数
	void calcUintConvertCoef();

	// 计算pv及rms
	void calcPvRmsResult(Zernike zernike);

	// 计算zernike与seidel参数
	void calcZerSeiResult(Zernike zernike);

	// 计算ISO相关参数
	void calcISOResult(Zernike zernike);

	// 计算结果中其他的相关参数
	void calcOthersResult(Zernike zernike);

	// 计算psd（psd未统一单位，当前单位为波长^2*mm）
	void calcPsd(const cv::Mat fitting_rem_zer);

	// 计算切片psd
	cv::Mat calcPsdSlice(const cv::Mat Slice, float pixel_mm);

	// 计算波前梯度均方根—Grms（当测量单位为波长时，该值单位为波长^2*mm）
	void calcGrms(const cv::Mat fitting_rem_zer);

	// Grms最简单差分算法
	float Grms_Basic_Diff(cv::Mat fitting_rem_zer,cv::Mat mask);

	// Grms中心差分算法
	float Grms_CENTRAL_Diff(cv::Mat fitting_rem_zer, cv::Mat mask);

};

#endif
