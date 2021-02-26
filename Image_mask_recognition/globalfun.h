#ifndef GLOBALFUN_H
#define GLOBALFUN_H

#include <QTextStream>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QTableWidget>
#include <QThread>
#include <QMutex>
#include <QScreen>
#include <QTime>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>
using namespace std;

class GlobalFun
{
public:
    GlobalFun();

    /* 用于全局缩放元素 */

    static pair<double,double> getScaleFactor(int width=1440,int height=900);

    // 读写配置文件ini
    static QString getProperty(const QString &source, QString section, QString key);
    static void setProperty(const QString &source, QString section, QString key, QString value);

    // 读取JSON
    static QJsonObject getJsonObj(const QString &source);

    // 获取当前时间
    static QString getCurrentTime(int type);

    // 获取当前工程执行路径
    static QString getCurrentPath();

    // 获取当前线程ID
    static QString getCurrentThreadID();

    // 睡眠
    static void bsleep(int second);

    // 判断浮点数是否相等
    static bool equals(qreal a, qreal b);

    // 判断文件是否存在
    static bool isDirExist(QString &path);

    // 判断文件夹是否存在
    static bool isFileExist(QString &fileName);

    // 删除文件夹
    static bool removeFolder(const QString &path);

    // 删除文件
    static bool removeFile(const QString &fileName);

    // 导出csv文件
    static void exportCSV(QString path, QTableWidget *widget);

    // 判断图像是否可用
    static bool judVecAvailable(std::vector<cv::Mat> vec);

    // 提示弹窗
    static void showMessageBox(int type, QString info);

    //------------------------------------------------------------------------------

    // 改变图像中部分点的颜色
    static void changeColorToRed(QImage& image);

    // 过曝
    static void overExposure(QImage& image);

    // BGR 转 GRAY
    static std::vector<cv::Mat> cvtBGR2GRAY(const std::vector<cv::Mat> &vec, cv::Rect rect = cv::Rect(), bool state = false);

    // QImage 转 cv::Mat
    static cv::Mat convertQImageToMat(QImage &image);

    // cv::Mat 转 QImage
    static QImage convertMatToQImage(const cv::Mat mat);
};

//******************************************************************************************************************

class GlobalString
{
public:
    static QString menu_file;
    static QString menu_camera;
    static QString menu_shifter;
    static QString menu_language;

    static QString action_load;
    static QString action_save;
    static QString action_screenshot;
    static QString action_calibration;
    static QString action_aim;
    static QString action_chinese;
    static QString action_english;
    static QString action_calculate;
    static QString action_analysis;
    static QString action_setting;
    static QString action_Loopcheck;
    static QString action_download;
    static QString action_get;
    static QString action_lock;

    static QString graphics_circle;
    static QString graphics_ellipse;
    static QString graphics_concentric_circle;
    static QString graphics_rectangle;
    static QString graphics_square;
    static QString graphics_polygon;
    static QString graphics_pill;
    static QString graphics_chamfer;

    static QString graphics_auto_circle;
    static QString graphics_auto_ellipse;
    static QString graphics_auto_pill;
    static QString graphics_auto_chamfer;
    static QString graphics_auto_roundEdgeRec;
    static QString graphics_auto_rotateRec;

    static QString graphics_lock;
    static QString graphics_unlock;
    static QString graphics_live;
    static QString graphics_showMask;
    static QString graphics_manual;
    static QString graphics_auto;
    static QString graphics_reset;
    static QString graphics_hole_num;
    static QString graphics_sure;
    static QString graphics_change;

    static QString contextMenu_clear;
    static QString contextMenu_delete;
    static QString contextMenu_export;
    static QString contextMenu_show;
    static QString contextMenu_unit;
    static QString contextMenu_sure;
    static QString contextMenu_fitting;
    static QString contextMenu_residual;
    static QString contextMenu_fillSpikes;
};

//******************************************************************************************************************

class GlobalValue
{
public:
    static int cam_tp;
    static int cam_itl;
    static double cam_exp;
    static int cam_exp_dly;
    static QString cam_fmt;
    static double cam_fps;
    static double cam_p_cal;

    static int com_auto_itl;
    static int com_h_num;
    static int com_tp;
    static double com_p_sle;
    static int com_s_p;
    static double com_value;
    static QString com_log_path;
    static QString com_pic_path;
    static int com_unit;

    static int atm_t_i;
    static int atm_bte;
    static int atm_dbt;
    static int atm_cbt;
    static int atm_sbt;
    static int atm_fcl;
    static QString atm_ip;
    static QString atm_port;
    static QString atm_sts;
    static QString atm_c_id;
    static QString atm_all_ret;
    static int atm_spe;
    static int atm_use_spe;

    static int lgn_tp;

    static int sft_ch_num;
    static double sft_s_vol;
    static double sft_i_vol;
    static double sft_v_cal;
    static int sft_s_rest;
    static int sft_i_rest;
    static int sft_a_s_t;

    static double gra_c_rad;
    static double gra_e_wid;
    static double gra_e_hei;
    static double gra_cc_rad_1;
    static double gra_cc_rad_2;
    static double gra_r_wid;
    static double gra_r_hei;
    static double gra_s_len;
    static double gra_p_wid;
    static double gra_p_hei;
    static double gra_ch_wid;
    static double gra_ch_hei;
    static double gra_ch_rad;
    static int gra_def_size;

    static int zer_pis;
    static int zer_tilt;
    static int zer_pow;
    static int zer_ast;
    static int zer_coma;
    static int zer_sph;

    static int ret_pv;
    static int ret_pv_x;
    static int ret_pv_y;
    static int ret_pv_res;
    static int ret_rms;
    static int ret_rms_res;
    static int ret_tilt;
    static int ret_power;
    static int ret_pwr_x;
    static int ret_pwr_y;
    static int ret_ast;
    static int ret_coma;
    static int ret_sph;
    static int ret_ttv;
    static int ret_fringe;

    static int chk_check;
    static int chk_cts;
    static double chk_p_sh_t1;
    static double chk_p_s_t1;

    static int par_psi;
    static int par_unw;
    static int par_ztm;
    static double par_i_s_f;
    static double par_t_w;
    static double par_i_w;
    static double par_hp_fc;
    static int par_use_fs;
};

#endif // GLOBALFUN_H
