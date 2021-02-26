#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "globalfun.h"
#include "biccameracontrol.h"
#include "bbaslercamercontrol.h"
#include "bmvcameracontrol.h"
#include "bvrcameracontrol.h"
#include "dacardcontrol.h"
#include "bqgraphicsscene.h"
#include "qcustomplot.h"
#include "algorithmMain.h"
#include "blog.h"
#include "dialog/camera_setting_dialog.h"
#include "dialog/camera_calibration_dialog.h"
#include "dialog/camera_aim_dialog.h"
#include "dialog/shifter_setting_dialog.h"
#include "dialog/shifter_calibration_dialog.h"
#include "dialog/customer_engineer_dialog.h"
#include "dialog/our_engineer_dialog.h"
#include "dialog/unit_setting_dialog.h"
#include "pztcalibrate.h"
#include<QVector>
#include<QSpacerItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initData();                            // 初始化数据
    void initView();                            // 初始化界面
    void initCamera();                          // 初始化相机
    void initDACard();                          // 初始化DA卡
    void initGraphicsScene();                   // 初始化场景

    void loadConfig();                          // 加载配置文件
    void saveConfig();                          // 保存配置文件
    void updateView(bool state);                // 更新界面
    void modifyScale();                         // 修改图像的显示比例
    void changeLanguage(int type);              // 改变语言
    void changeStatus(bool status);             // 改变状态
    void hideConfig(QString path);              // 隐藏配置文件

    void createDefaultConfig();                 // 创建默认配置文件
    void createHoleConfig(QString &path);       // 创建孔参数配置文件
    void createResultTable(int num);            // 创建 result table
    void createViewTable(int num);              // 创建 view table      //默认十个孔，超过十个不处理
    void createDataTable(int num);              // 创建 data table
    void createQualityTable(int num);           // 创建 quality table
    void createContextMenu(int type);           // 创建右键菜单栏
    void createMenuDialog(int type);            // 创建菜单栏弹窗

    void startCalculation();                    // 开始计算

    void createThread();                        // 创建线程
    void modifyConfig();                        // 修改配置参数
    void execProcess(int num);                  // 执行计算
    std::vector<cv::Mat> getMatList(int num);   // 获取传给算法的图像
    cv::Mat getMask(int num, cv::Mat image);    // 获取传给算法的掩膜
    void drawMask();                            // 画掩膜
    void savePicAndLog();                       // 保存图像和日志
    void modifyTableHead();                     // 修改日志表头
    void changeViewTable(int type);

protected:
    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void fillData();                                        // 填充数据
    void screenshot(QString str);                           // 截屏
    void testFinished(cv::Mat mat, float std, int shift);   // 移相器标定

private slots:
    void on_fillData();

private:
    Ui::MainWindow *ui;

    Camera *m_camera;                           // 相机实例
    DACardControl *m_daCard;                    // DA卡实例
    BQGraphicsScene *m_scene;                   // 场景实例
    BLog m_log;                                 // 日志实例

    QList<AlgorithmMain *> m_algorithmList;     // 算法实例列表
    std::vector<cv::Mat> m_matList;             // 图像列表
    cv::Mat m_manualMask;                       // 手动掩膜
    cv::Rect m_rect;                            // 截取掩膜的矩形框
    QMap<int, cv::Mat> m_maskFillList;          // 掩膜填充列表
    QMap<int, cv::Mat> m_maskEdgeList;          // 掩膜边缘列表
    QMap<int, QString> m_plcData;               // plc地址列表
    QList<float> m_apertureList;                // 口径列表
    QDateTime lastTime;                         // 最后一次时间，用于计算时间间隔
    bool is_in_calculation;                     // 是否处于计算中
    bool is_in_check;                           // 是否处于复测中
    bool is_locked;                             // result table 锁定状态
    int checkTimes;                             // 剩余复测次数
    QString cell_id;                            // PLC读取到的id
    QCPColorGradient cpg;                       // 渐变色色条
    QMutex mtx;                                 // 互斥锁

    QTimer m_long_press_event_timer;            // 鼠标长按定时器
    QTimer m_test_timer;                        // 测试定时器
    QTimer check_loop_timer;                         // 开始检测的定时器
    bool isStop=true;                          // 循环检测是否停止
    QVector<QLabel *>lablevec;                 //view的lable的容器
    int Hmarin=60;                             //控制行间距，正好合适
    Camera_aim_dialog *dialog;                 //对准相机
};
#endif // MAINWINDOW_H
