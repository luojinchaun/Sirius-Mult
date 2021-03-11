#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <thread>
#include <QMessageBox>
#include <windows.h>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>
#include<QtDebug>
#include<QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initData();
    initView();
    initCamera();
    initDACard();
    initGraphicsScene();
}

MainWindow::~MainWindow()
{
    m_camera->destroyCamera();

    for ( auto &alg : m_algorithmList )
    {
        delete alg;
    }

    delete m_daCard;
    delete ui;
}

void MainWindow::initData()
{
    qDebug() << "initData !";
    createDefaultConfig();
    loadConfig();
    changeLanguage(GlobalValue::lgn_tp);

    for (int i = 0; i < GlobalValue::com_h_num; ++i)
    {
        QString config = QString("hole_%1_config.ini").arg(i+1);
        if ( !GlobalFun::isFileExist(config) ) {
            createHoleConfig(config);
            hideConfig(GlobalFun::getCurrentPath() + "/" + config);//设置为隐藏文件
        }
    }

    calcConvertMatrix();    // 转换矩阵为常量

    for (int i = 0; i < GlobalValue::com_h_num; ++i)
    {
        CONFIG_PARAMS config;
        AlgorithmMain *algorith = new AlgorithmMain(config);
        m_algorithmList.push_back(algorith);
    }

    m_matList.clear();
    m_manualMask = cv::Mat();
    m_maskFillList.clear();
    m_maskEdgeList.clear();
    m_plcData.clear();
    m_apertureList.clear();
    lastTime = QDateTime::currentDateTime();
    is_in_calculation = false;
    is_in_check = false;
    is_locked = true;
    checkTimes = 0;
    cell_id = "";
    dialog = new Camera_aim_dialog(this);
    cpg.setColorStopAt(1, Qt::red);
    cpg.setColorStopAt(0.55, Qt::yellow);
    cpg.setColorStopAt(0.44, Qt::green);
    cpg.setColorStopAt(0, Qt::blue);

    //------------------------------------------------------------------------------

    QString fileName = GlobalFun::getCurrentPath() + "/log.txt";
    if ( GlobalFun::isFileExist(fileName) ) {
        m_log.openFile(fileName, QFile::ReadOnly);
        QString date = m_log.read(true);
        m_log.closeFile();

        QDate oldTime = QDate::fromString(date, "yyyy-MM-dd");
        QDate newTime = QDate::currentDate();
        qint64 day = oldTime.daysTo(newTime);
        if (day >= 7) {
            GlobalFun::removeFile(fileName);
            m_log.openFile(fileName, QFile::WriteOnly | QFile::Truncate);
            m_log.write(GlobalFun::getCurrentTime(3) + "\n");
        } else {
            m_log.openFile(fileName, QFile::WriteOnly | QFile::Append);
        }

    } else {
        m_log.openFile(fileName, QFile::WriteOnly | QFile::Truncate);
        m_log.write(GlobalFun::getCurrentTime(3) + "\n");
    }

    m_log.write(GlobalFun::getCurrentTime(2) + " - application power on\n");

    //------------------------------------------------------------------------------

    connect(this, &MainWindow::fillData, this, [&](){ on_fillData(); }, Qt::QueuedConnection);

    connect(&m_test_timer, &QTimer::timeout, [&](){ startCalculation(); });

    connect(this, &MainWindow::screenshot, [&](QString str){
        QString screenshot = str + "/screenshot.png";
        QPixmap::grabWindow(this->winId()).save(screenshot);
    });
}

void MainWindow::initView()
{
    qDebug() << "initView !";
    setWindowTitle("Sirius Mult 3.1.3");
    setMinimumSize(1320, 580);

    ui->action_Piston->setChecked(GlobalValue::zer_pis == 1);
    ui->action_Tilt->setChecked(GlobalValue::zer_tilt == 1);
    ui->action_Power->setChecked(GlobalValue::zer_pow == 1);
    ui->action_Ast->setChecked(GlobalValue::zer_ast == 1);
    ui->action_Coma->setChecked(GlobalValue::zer_coma == 1);
    ui->action_Spherical->setChecked(GlobalValue::zer_sph == 1);
    ui->toolBar->setStyleSheet("QToolButton { font-size: 14px;"
                                             "border: 2px;"
                                             "border-color: transparent;"
                                             "border-style:solid; }"
                               "QToolButton::hover { background-color: rgb(192, 220, 243);"
                                                    "font-size: 14px;"
                                                    "border: 2px;"
                                                    "border-color: transparent;"
                                                    "border-radius: 4px;"
                                                    "border-style: solid; }"
                               "QToolButton::checked { background-color: rgb(192, 220, 243);"
                                                      "font-size: 14px;"
                                                      "border: 2px;"
                                                      "border-color: transparent;"
                                                      "border-radius: 4px;"
                                                      "border-style: solid; }");

    // menu file
    connect(ui->action_load, &QAction::triggered, [&](){
        QString file = QFileDialog::getExistingDirectory(this, GlobalString::action_load, GlobalFun::getCurrentPath());

        if ( file != "" ) {
            int psi = GlobalValue::par_psi < 3 ? 5 : 9;
            m_matList.clear();

            QString png = file + "/0.png";
            QString bmp = file + "/0.bmp";
            QString format = "png";

            if ( GlobalFun::isFileExist(png) ) { format = "png"; }
            else if ( GlobalFun::isFileExist(bmp) ) { format = "bmp"; }

            for ( int i = 0; i < psi; ++i )
            {
                QString fileName = file + QString("/%1.%2").arg(i).arg(format);
                if ( !GlobalFun::isFileExist(fileName) ) {
                    m_matList.clear();
                    GlobalFun::showMessageBox(4, fileName + " is not exist !");
                    return;
                } else {
                    cv::Mat mat = cv::imread(fileName.toLatin1().data());//读入图像三通道，彩色图
                    m_matList.push_back(mat);
                }
            }

            if ( !m_scene->getLiveStatus() ) {
                m_scene->updateLiveMode(false, m_matList);
            }

            GlobalFun::showMessageBox(2, QString("Image loaded successfully ! Format: %1").arg(format));
        }
    });
    connect(ui->action_save, &QAction::triggered, [&](){
        if ( m_matList.size() == 0 ) {
            GlobalFun::showMessageBox(3, "No picture to save !");
        } else if ( !GlobalFun::judVecAvailable(m_matList) ) {
            GlobalFun::showMessageBox(3, "The picture is empty !");
        } else {
            QString file = QFileDialog::getExistingDirectory(this, GlobalString::action_save, GlobalFun::getCurrentPath());

            std::thread th([=] {
                std::vector<cv::Mat> vec = GlobalFun::cvtBGR2GRAY(m_matList, cv::Rect(), false);
                for ( size_t i = 0; i < vec.size(); ++i )
                {
                    QString fileName = file + QString("/%1.png").arg(i);
                    cv::imwrite(fileName.toStdString(), vec.at(i));
                }
            });
            th.detach();
        }
    });
    connect(ui->action_screenshot, &QAction::triggered, [&](){
        QString file = QFileDialog::getSaveFileName(nullptr, GlobalString::action_screenshot,
                                                    GlobalFun::getCurrentTime(1), "Text files (*.png)");
        QPixmap::grabWindow(this->winId()).save(file);
    });

    // menu camera
    connect(ui->action_camera_setting, &QAction::triggered, [&](){ createMenuDialog(1); });
    connect(ui->action_camera_calibration, &QAction::triggered, [&](){ createMenuDialog(2); });
    connect(ui->action_camera_aim, &QAction::triggered, [&](){ createMenuDialog(3); });

    // menu shifter
    connect(ui->action_shifter_setting, &QAction::triggered, [&](){ createMenuDialog(4); });
    connect(ui->action_shifter_calibration, &QAction::triggered, [&](){ createMenuDialog(5); });

    // menu language
    connect(ui->action_chinese, &QAction::triggered, [&](){ changeLanguage(1); });
    connect(ui->action_english, &QAction::triggered, [&](){ changeLanguage(2); });

    // toolbar
    connect(ui->action_calculate, &QAction::triggered, [&](){
        //changeStatus(is_in_calculation);
        startCalculation();
    });
    connect(ui->action_analysis, &QAction::triggered, [&](){
        if ( m_matList.size() == 0 ) {
            GlobalFun::showMessageBox(3, "No pictures !");
        } else if ( !GlobalFun::judVecAvailable(m_matList) ) {
            GlobalFun::showMessageBox(3, "Image is empty !");
        } else {
            if ( !is_in_calculation ) {
                changeStatus(false);
                m_daCard->acquisition_complete();
            }
        }
    });
    connect(ui->action_setting, &QAction::triggered, [&](){ createMenuDialog(6); });

    connect(ui->action_Piston, &QAction::triggered, [&](bool checked){ GlobalValue::zer_pis = checked ? 1 : 0; });
    connect(ui->action_Tilt, &QAction::triggered, [&](bool checked){ GlobalValue::zer_tilt = checked ? 1 : 0; });
    connect(ui->action_Power, &QAction::triggered, [&](bool checked){ GlobalValue::zer_pow = checked ? 1 : 0; });
    connect(ui->action_Ast, &QAction::triggered, [&](bool checked){ GlobalValue::zer_ast = checked ? 1 : 0; });
    connect(ui->action_Coma, &QAction::triggered, [&](bool checked){ GlobalValue::zer_coma = checked ? 1 : 0; });
    connect(ui->action_Spherical, &QAction::triggered, [&](bool checked){ GlobalValue::zer_sph = checked ? 1 : 0; });
    connect(ui->action_lock, &QAction::triggered, [&](bool checked){
        ui->action_lock->setChecked(checked);
        ui->action_camera_setting->setEnabled(!checked);
        ui->action_camera_calibration->setEnabled(!checked);
        ui->action_camera_aim->setEnabled(!checked);
        ui->action_shifter_setting->setEnabled(!checked);
        ui->action_shifter_calibration->setEnabled(!checked);
        ui->action_setting->setEnabled(!checked);
        ui->action_Piston->setEnabled(!checked);
        ui->action_Tilt->setEnabled(!checked);
        ui->action_Power->setEnabled(!checked);
        ui->action_Ast->setEnabled(!checked);
        ui->action_Coma->setEnabled(!checked);
//        if(check_loop_timer.isActive()){//一直enable，循环检测
//            ui->action_Loopcheck->setEnabled(true);
//        }else{
//            ui->action_Loopcheck->setEnabled(!checked);
//        }
        ui->action_Spherical->setEnabled(!checked);
        ui->action_get->setEnabled(!checked);
        ui->action_download->setEnabled(!checked);
    });
    connect(&check_loop_timer,&QTimer::timeout,[&](){
        startCalculation();
    });

    connect(ui->action_Loopcheck,&QAction::triggered, [&](){
        QIcon icon;
        if(isStop){
            GlobalValue::lgn_tp==1?ui->action_Loopcheck->setText(QStringLiteral("停止检测")):
                                   ui->action_Loopcheck->setText(QStringLiteral("Stop detect"));
            icon.addFile(QString::fromUtf8(":/images/StopLoop.png"), QSize(), QIcon::Normal, QIcon::Off);
            check_loop_timer.start(GlobalValue::com_auto_itl);
            startCalculation();
        }else{
            GlobalValue::lgn_tp==1?ui->action_Loopcheck->setText(QStringLiteral("循环检测")):
                                   ui->action_Loopcheck->setText(QStringLiteral("Loop detect"));
            icon.addFile(QString::fromUtf8(":/images/LoopCalibration.png"), QSize(), QIcon::Normal, QIcon::Off);
            check_loop_timer.stop();
        }
        isStop=!isStop;
        ui->action_Loopcheck->setIcon(icon);
    });
    connect(ui->action_download,&QAction::triggered, [&](){
        QString file = QFileDialog::getSaveFileName(nullptr, "",GlobalFun::getCurrentPath(),"Mask files (*.ini)");
        if(!file.size()){
            return;
        }
        m_scene->saveItemToConfig(file);
    });
    connect(ui->action_get,&QAction::triggered, [&](){
        QString file = QFileDialog::getOpenFileName(nullptr, "",GlobalFun::getCurrentPath(),"Mask files (*.ini)");
        if(!file.size()){
            return;
        }
        m_scene->loadItemToScene(file,false);
    });
    //------------------------------------------------------------------------------

    createResultTable(GlobalValue::com_h_num);
    createDataTable(GlobalValue::com_h_num);
    createQualityTable(GlobalValue::com_h_num);
    createViewTable(GlobalValue::com_h_num);
    connect(ui->result_table, &QTableWidget::customContextMenuRequested, [&](){ createContextMenu(1); });
    connect(ui->data_table, &QTableWidget::customContextMenuRequested, [&](){ createContextMenu(2); });
    connect(&m_long_press_event_timer, &QTimer::timeout, [&](){
//        if ( ui->action_lock->isEnabled() ) {
            ui->action_lock->setChecked(true);
            ui->action_lock->triggered(true);
//        }
        //ui->action_lock->setEnabled(!ui->action_lock->isEnabled());
        m_long_press_event_timer.stop();
    });
    switch (GlobalValue::com_unit) {
    case 0: ui->action_Company->setText(QStringLiteral("λ@") + QString::number(GlobalValue::com_value)); break;
    case 1: ui->action_Company->setText(QStringLiteral("fr@") + QString::number(GlobalValue::com_value/2)); break;
    case 2: ui->action_Company->setText(QStringLiteral("nm")); break;
    case 3: ui->action_Company->setText(QStringLiteral("μm")); break;
    case 4: ui->action_Company->setText(QStringLiteral("mm")); break;
    default: ui->action_Company->setText(QStringLiteral("nm")); break;
    }
    //ui->action_Company->setEnabled(true);
    //connect(ui->action_Company, &QAction::triggered, [&](){ createMenuDialog(8); });//可能需要创建这个页面，也可能不需要
}

void MainWindow::initCamera()
{
    qDebug() << "initCamera ! the camera_id is: " << GlobalValue::cam_tp;

    switch ( GlobalValue::cam_tp )
    {
    case 1: m_camera = new BBaslerCamerControl(Camera::CameraType::Basler); break;
    case 2: m_camera = new BICCameraControl(Camera::CameraType::IC_Imaging); break;
    case 3: m_camera = new BMVCameraControl(Camera::CameraType::MV); break;
    case 4: m_camera = new BVRCameraControl(Camera::CameraType::Virtual); break;
    default: m_camera = new BBaslerCamerControl(Camera::CameraType::Basler); break;
    }

    m_camera->initCamera();
    m_camera->openCamera();
    m_camera->startWork();

    if ( GlobalValue::cam_tp == 1 ) {
        BBaslerCamerControl *bs_camera = dynamic_cast<BBaslerCamerControl *>(m_camera);
        GlobalValue::cam_exp = bs_camera->getCameraProperty(BBaslerCamerControl::BaslerCameraProperty::ExposureTime).toDouble()/1000;
        qDebug() << "Basler Exposure: " + QString::number(GlobalValue::cam_exp) + " ms";
    } else if ( GlobalValue::cam_tp == 2 ) {
        BICCameraControl *ic_camera = dynamic_cast<BICCameraControl *>(m_camera);
        ic_camera->loadConfileFromFile("device.xml");
        GlobalValue::cam_exp = ic_camera->getExposure() * 1000;
        qDebug() << "IC_Imaging Exposure: " + QString::number(GlobalValue::cam_exp) + " ms";
    } else if ( GlobalValue::cam_tp == 3 ) {
        BMVCameraControl *mv_camera = dynamic_cast<BMVCameraControl *>(m_camera);
        GlobalValue::cam_exp = mv_camera->getCameraProperty(BMVCameraControl::BMVCameraProperty::ExposureTime).toDouble()/1000;
        qDebug() << "MV Exposure: " + QString::number(GlobalValue::cam_exp) + " ms";
    } else if ( GlobalValue::cam_tp == 4 ) {
        qDebug() << "Virtual Exposure: pass";
    }
}

void MainWindow::initDACard()
{
    qDebug() << "initDACard !";
    m_daCard = new DACardControl();

    connect(m_daCard, &DACardControl::takeAPic, this, [&](){
        cv::Mat mat = m_camera->takeAPic();
        m_matList.push_back(mat);
    }, Qt::QueuedConnection);

    connect(m_daCard, &DACardControl::acquisition_complete, this, [&](){ createThread(); }, Qt::QueuedConnection);

    connect(m_daCard, &DACardControl::error, this, [&](int type){
        switch (type) {
        case -1:
            GlobalFun::showMessageBox(4, "Failed to open the device.\n"
                                         "Please check whether the Laser Interferometer Controller is connected !");
            break;
        default: GlobalFun::showMessageBox(4, "Device is not found !"); break;
        }

        m_daCard->closeDevice();
        m_daCard->quit();
        changeStatus(true);
    }, Qt::QueuedConnection);

    connect(m_daCard, &DACardControl::test_complete, this, [&](QRect &rect){
        std::vector<cv::Mat> vec;
        vec = GlobalFun::cvtBGR2GRAY(m_matList);

        cv::Mat mat = cv::Mat();
        int shift = 0;
        float std = 0;
        cv::Rect ret(rect.x(), rect.y(), rect.width(), rect.height());
        calc::PztHistCalibrate(vec.at(0)(ret), vec.at(1)(ret), vec.at(3)(ret), vec.at(4)(ret), mat, &shift);
        std = calc::PztHistStd(mat, vec.at(0), ret);

        emit testFinished(mat, std, shift);
    }, Qt::DirectConnection);
}

void MainWindow::initGraphicsScene()
{
    qDebug() << "initGraphicsScene !";
    m_scene = new BQGraphicsScene(this);
    ui->graphicsView->setScene(m_scene);
    m_scene->setPixmapSize(ui->graphicsView->width()-20, ui->graphicsView->height()-20);
    connect(m_camera, &Camera::updateImage, m_scene, &BQGraphicsScene::updateFrame);
    connect(m_camera, &Camera::updateImage, [&](QImage image){//按照贞率刷新
        if ( (GlobalValue::com_p_sle == 1) && (image.width() != 780 || image.height() != 480) ) {
            modifyScale();
        }
    });

    connect(m_scene, &BQGraphicsScene::resetView, [&](){
        ui->graphicsView->resetTransform();
        ui->graphicsView->scale(GlobalValue::com_p_sle, GlobalValue::com_p_sle);
    });
    connect(m_scene, &BQGraphicsScene::createConfig, this, [&](int num){
        std::thread th([=] {
            QString path = QString("hole_%1_config.ini").arg(num);
            createHoleConfig(path);
            hideConfig(GlobalFun::getCurrentPath() + "/" + path);

            CONFIG_PARAMS config;
            AlgorithmMain *algorith = new AlgorithmMain(config);
            m_algorithmList.push_back(algorith);
        });
        th.detach();
    }, Qt::DirectConnection);
    connect(m_scene, &BQGraphicsScene::removeConfig, [&](int num){
        QString path = QString("hole_%1_config.ini").arg(num);
        GlobalFun::removeFile(path);

        if ( !m_algorithmList.isEmpty() ) {
            delete m_algorithmList.at(m_algorithmList.size() - 1);
            m_algorithmList.pop_back();
        }
    });
    connect(m_scene, &BQGraphicsScene::updateTableView, [&](int hole_num){
        createResultTable(hole_num);
        createDataTable(hole_num);
        createQualityTable(hole_num);
        createViewTable(hole_num);
        modifyTableHead();
    });
    connect(m_scene, &BQGraphicsScene::changeLiveMode, [&](bool status){
        if ( status ) {//传入false
            // live
            m_scene->updateLiveMode(status, m_matList);
        } else {
            // unlive
            if ( m_matList.size() == 0 ) {
                GlobalFun::showMessageBox(3, "No picture to show !");
            } else if ( !GlobalFun::judVecAvailable(m_matList) ) {
                GlobalFun::showMessageBox(3, "Image is empty !");
            } else {
                m_scene->updateLiveMode(status, m_matList);
            }
        }
    });
}

bool MainWindow::event(QEvent *event)
{
    if ( event->type() == QEvent::WindowStateChange ) {
        if( this->windowState() == Qt::WindowMaximized ) {
            updateView(true);
        } else if ( this->windowState() == Qt::WindowNoState ) {
            updateView(false);
        }
    }
    if ( event->type() == QEvent::Resize) {
        updateView(true);
    }

    return QMainWindow::event(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_F4 || event->key() == Qt::Key_Enter ) { startCalculation(); }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         event->key() == Qt::Key_F12 ) {
        createMenuDialog(7);
    }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         (event->key() == Qt::Key_L || event->key() == Qt::Key_F1) ) {
        if ( ui->action_lock->isChecked() ) {
            ui->action_lock->setChecked(false);
            ui->action_lock->triggered(false);
        } else {
            ui->action_lock->setChecked(true);
            ui->action_lock->triggered(true);
        }
        //ui->action_lock->setEnabled(!ui->action_lock->isEnabled());
        m_long_press_event_timer.stop();
    }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         event->key() == Qt::Key_P ) {
        QString file = QFileDialog::getSaveFileName(nullptr, GlobalString::action_screenshot,
                                                    GlobalFun::getCurrentTime(1), "Text files (*.png)");
        QPixmap::grabWindow(this->winId()).save(file);
    }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         event->key() == Qt::Key_B ) {
        QString file = QFileDialog::getSaveFileName(nullptr, GlobalString::action_screenshot,
                                                    GlobalFun::getCurrentTime(1), "Text files (*.bmp)");
        QPixmap::grabWindow(this->winId()).save(file);
    }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         event->key() == Qt::Key_J ) {
        QString file = QFileDialog::getSaveFileName(nullptr, GlobalString::action_screenshot,
                                                    GlobalFun::getCurrentTime(1), "Text files (*.jpg)");
        QPixmap::grabWindow(this->winId()).save(file);
    }

    if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier) &&
         event->key() == Qt::Key_O ) {
        createMenuDialog(8);
    }

    QWidget::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveConfig();
    hideConfig(GlobalFun::getCurrentPath() + "/config.ini");
    hideConfig(GlobalFun::getCurrentPath() + "/parameter.ini");

    m_scene->saveItemToConfig();
    hideConfig(GlobalFun::getCurrentPath() + "/item.ini");

    m_log.write(GlobalFun::getCurrentTime(2) + " - application power off");
    m_log.write("\n****************************************************************************\n");
    m_log.closeFile();
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if ( event->buttons() == Qt::LeftButton ) {
        if ( event->x() <= 20 && event->y() >= this->height() - 20 ) {
            m_long_press_event_timer.start(3000);
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if ( m_long_press_event_timer.isActive() ) {
        m_long_press_event_timer.stop();
    }
}

//******************************************************************************************************************

void MainWindow::loadConfig()
{
    QSettings config("config.ini", QSettings::IniFormat);

    GlobalValue::cam_tp = config.value("camera/type").toInt();                              // 相机类型 1-映美精 2-巴斯勒 3-海康威视
    GlobalValue::cam_itl = config.value("camera/interval").toInt();                         // 相机抓图间隔
    GlobalValue::cam_exp = config.value("camera/exposure").toDouble();                      // 相机曝光
    GlobalValue::cam_exp_dly = config.value("camera/exposure_delay").toInt();               // 相机曝光延时
    GlobalValue::cam_fmt = config.value("camera/format").toString();                        // 相机 Format
    GlobalValue::cam_fps = config.value("camera/fps").toDouble();                           // 相机 FPS
    GlobalValue::cam_p_cal = config.value("camera/pixel_calibration").toDouble();           // 像素标定值

    GlobalValue::com_auto_itl=config.value("common/auto_test_interval", 5000).toInt();      //自动检测的间隔
    GlobalValue::com_h_num = config.value("common/hole_number").toInt();                    // 孔数
    GlobalValue::com_tp = config.value("common/type").toInt();                              // 掩膜方式 1-手动 2-自动
    GlobalValue::com_p_sle = config.value("common/pic_scale").toDouble();                   // 图片缩放比例
    GlobalValue::com_s_p = config.value("common/save_picture").toInt();                     // 保存原图
    GlobalValue::com_log_path = config.value("common/log_path").toString();                 // log 文件保存路径
    GlobalValue::com_pic_path = config.value("common/pic_path").toString();                 // pic 文件保存路径
    GlobalValue::com_unit = config.value("common/unit").toInt();
    GlobalValue::com_value = config.value("common/value").toDouble();

    GlobalValue::atm_t_i = config.value("automation/test_interval").toInt();                // 自动化测试 - 定时器的间隔

    GlobalValue::lgn_tp = config.value("language/type").toInt();                            // 语言类型

    GlobalValue::sft_ch_num = config.value("shifter/channel_number").toInt();               // DA卡使用的通道号
    GlobalValue::sft_s_vol = config.value("shifter/step_voltage").toDouble();               // 步进电压
    GlobalValue::sft_i_vol = config.value("shifter/interval_voltage").toDouble();           // 间隔电压
    GlobalValue::sft_v_cal = config.value("shifter/voltage_calibration").toDouble();        // 电压标定值
    GlobalValue::sft_s_rest = config.value("shifter/step_rest").toInt();                    // 步进睡眠
    GlobalValue::sft_i_rest = config.value("shifter/interval_rest").toInt();                // 间隔睡眠
    GlobalValue::sft_a_s_t = config.value("shifter/add_several_times").toInt();             // 增加电压的次数

    GlobalValue::gra_c_rad = config.value("graphicsItem/circle_radius").toDouble();         // 圆-半径
    GlobalValue::gra_e_wid = config.value("graphicsItem/ellipse_width").toDouble();         // 椭圆-宽度
    GlobalValue::gra_e_hei = config.value("graphicsItem/ellipse_height").toDouble();        // 椭圆-高度
    GlobalValue::gra_cc_rad_1 = config.value("graphicsItem/conCircle_radius_1").toDouble(); // 同心圆-内圆半径
    GlobalValue::gra_cc_rad_2 = config.value("graphicsItem/conCircle_radius_2").toDouble(); // 同心圆-外圆半径
    GlobalValue::gra_r_wid = config.value("graphicsItem/rectangle_width").toDouble();       // 矩形-宽度
    GlobalValue::gra_r_hei = config.value("graphicsItem/rectangle_height").toDouble();      // 矩形-高度
    GlobalValue::gra_s_len = config.value("graphicsItem/square_length").toDouble();         // 正方形-边长
    GlobalValue::gra_p_wid = config.value("graphicsItem/pill_width").toDouble();            // 圆端矩形-宽度
    GlobalValue::gra_p_hei = config.value("graphicsItem/pill_height").toDouble();           // 圆端矩形-高度
    GlobalValue::gra_ch_wid = config.value("graphicsItem/chamfer_width").toDouble();        // 圆角矩形-宽度
    GlobalValue::gra_ch_hei = config.value("graphicsItem/chamfer_height").toDouble();       // 圆角矩形-高度
    GlobalValue::gra_ch_rad = config.value("graphicsItem/chamfer_radius").toDouble();       // 圆角矩形-倒角半径
    GlobalValue::gra_def_size=config.value("graphicsItem/default_size").toInt();            // 缩放尺寸

    GlobalValue::zer_pis = config.value("zernike/piston").toInt();                          // 偏移量
    GlobalValue::zer_tilt = config.value("zernike/tilt").toInt();                           // 倾斜量
    GlobalValue::zer_pow = config.value("zernike/power").toInt();                           // 离焦
    GlobalValue::zer_ast = config.value("zernike/ast").toInt();                             // 像散
    GlobalValue::zer_coma = config.value("zernike/coma").toInt();                           // 慧差
    GlobalValue::zer_sph = config.value("zernike/spherical").toInt();                       // 球差

    GlobalValue::ret_pv = config.value("result/pv").toInt();                                // zernike系数拟合结果的pv, 整个曲面最大值-最小值
    GlobalValue::ret_pv_x = config.value("result/pv_x").toInt();                            // zernike系数拟合结果的pv, x切片上的最大值-最小值
    GlobalValue::ret_pv_y = config.value("result/pv_y").toInt();                            // zernike系数拟合结果的pv, y切片上的最大值-最小值
    GlobalValue::ret_pv_res = config.value("result/pv_res").toInt();                        // 残差的pv
    GlobalValue::ret_rms = config.value("result/rms").toInt();                              // zernike系数拟合结果的均方根
    GlobalValue::ret_rms_res = config.value("result/rms_res").toInt();                      // 残差的均方根
    GlobalValue::ret_tilt = config.value("result/tilt").toInt();                            // zernike tilt
    GlobalValue::ret_power = config.value("result/power").toInt();                          // zernike power
    GlobalValue::ret_pwr_x = config.value("result/pwr_x").toInt();                          // zernike power_x
    GlobalValue::ret_pwr_y = config.value("result/pwr_y").toInt();                          // zernike power_y
    GlobalValue::ret_ast = config.value("result/ast").toInt();                              // zernike astigmatism
    GlobalValue::ret_coma = config.value("result/coma").toInt();                            // zernike coma
    GlobalValue::ret_sph = config.value("result/sph").toInt();                              // zernike spherical
    GlobalValue::ret_ttv = config.value("result/ttv").toInt();                              // 几何轮廓的厚度差
    GlobalValue::ret_fringe = config.value("result/fringe").toInt();                        // 条纹数


    QSettings parameter("parameter.ini", QSettings::IniFormat);

    //GlobalValue::chk_check = parameter.value("check/check").toInt();                        // 是否启用check
    GlobalValue::chk_cts = parameter.value("check/checkTimes").toInt();                     // 复测次数
    GlobalValue::chk_p_sh_t1 = parameter.value("check/psi_shift_thresh1").toDouble();       // 判据1
    GlobalValue::chk_p_s_t1 = parameter.value("check/psi_std_thresh1").toDouble();          // 判据1

    GlobalValue::par_psi = parameter.value("parameter/psi").toInt();                        // 几步移相
    GlobalValue::par_unw = parameter.value("parameter/unwrap").toInt();                     // 解包裹
    GlobalValue::par_ztm = parameter.value("parameter/zernikeTerm").toInt();                // zernike组
    GlobalValue::par_i_s_f = parameter.value("parameter/intf_scale_factor").toDouble();     // 比例因子
    GlobalValue::par_t_w = parameter.value("parameter/test_wavelength").toDouble();         // 测试波长
    GlobalValue::par_i_w = parameter.value("parameter/iso_wavelength").toDouble();          // ISO波长
    GlobalValue::par_hp_fc = parameter.value("parameter/highPass_filterCoef").toDouble();
    GlobalValue::par_use_fs = parameter.value("parameter/use_fillSpikes").toInt();

    GlobalValue::gra_c_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_e_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_e_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_cc_rad_1 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_cc_rad_2 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*5;
    GlobalValue::gra_r_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_r_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_s_len = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_p_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size *2;
    GlobalValue::gra_p_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /2;
    GlobalValue::gra_ch_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_ch_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_ch_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /6;
}

void MainWindow::saveConfig()
{
    QSettings config("config.ini", QSettings::IniFormat);

    config.setValue("camera/interval", GlobalValue::cam_itl);
    config.setValue("camera/exposure", GlobalValue::cam_exp);
    config.setValue("camera/format", GlobalValue::cam_fmt);
    config.setValue("camera/fps", GlobalValue::cam_fps);
    config.setValue("camera/pixel_calibration", GlobalValue::cam_p_cal);

    config.setValue("common/auto_test_interval",GlobalValue::com_auto_itl);
    config.setValue("common/hole_number", GlobalValue::com_h_num);
    config.setValue("common/type", GlobalValue::com_tp);
    config.setValue("common/pic_scale", GlobalValue::com_p_sle);
    config.setValue("common/save_picture", GlobalValue::com_s_p);
    config.setValue("common/log_path", GlobalValue::com_log_path);
    config.setValue("common/pic_path", GlobalValue::com_pic_path);
    config.setValue("common/unit", GlobalValue::com_unit);
    config.setValue("common/value", GlobalValue::com_value);

    config.setValue("automation/test_interval", GlobalValue::atm_t_i);

    config.setValue("language/type", GlobalValue::lgn_tp);

    config.setValue("shifter/step_voltage", GlobalValue::sft_s_vol);
    config.setValue("shifter/interval_voltage", GlobalValue::sft_i_vol);
    config.setValue("shifter/voltage_calibration", GlobalValue::sft_v_cal);
    config.setValue("shifter/step_rest", GlobalValue::sft_s_rest);
    config.setValue("shifter/interval_rest", GlobalValue::sft_i_rest);
    config.setValue("shifter/add_several_times", GlobalValue::sft_a_s_t);

    config.setValue("graphicsItem/circle_radius", GlobalValue::gra_c_rad);
    config.setValue("graphicsItem/ellipse_width", GlobalValue::gra_e_wid);
    config.setValue("graphicsItem/ellipse_height", GlobalValue::gra_e_hei);
    config.setValue("graphicsItem/conCircle_radius_1", GlobalValue::gra_cc_rad_1);
    config.setValue("graphicsItem/conCircle_radius_2", GlobalValue::gra_cc_rad_2);
    config.setValue("graphicsItem/rectangle_width", GlobalValue::gra_r_wid);
    config.setValue("graphicsItem/rectangle_height", GlobalValue::gra_r_hei);
    config.setValue("graphicsItem/square_length", GlobalValue::gra_s_len);
    config.setValue("graphicsItem/pill_width", GlobalValue::gra_p_wid);
    config.setValue("graphicsItem/pill_height", GlobalValue::gra_p_hei);
    config.setValue("graphicsItem/chamfer_width", GlobalValue::gra_ch_wid);
    config.setValue("graphicsItem/chamfer_height", GlobalValue::gra_ch_hei);
    config.setValue("graphicsItem/chamfer_radius", GlobalValue::gra_ch_rad);
    config.setValue("graphicsItem/default_size", GlobalValue::gra_def_size);
    config.setValue("zernike/piston", GlobalValue::zer_pis);
    config.setValue("zernike/tilt", GlobalValue::zer_tilt);
    config.setValue("zernike/power", GlobalValue::zer_pow);
    config.setValue("zernike/ast", GlobalValue::zer_ast);
    config.setValue("zernike/coma", GlobalValue::zer_coma);
    config.setValue("zernike/spherical", GlobalValue::zer_sph);

    config.setValue("result/pv", GlobalValue::ret_pv);
    config.setValue("result/pv_x", GlobalValue::ret_pv_x);
    config.setValue("result/pv_y", GlobalValue::ret_pv_y);
    config.setValue("result/pv_res", GlobalValue::ret_pv_res);
    config.setValue("result/rms", GlobalValue::ret_rms);
    config.setValue("result/rms_res", GlobalValue::ret_rms_res);
    config.setValue("result/tilt", GlobalValue::ret_tilt);
    config.setValue("result/power", GlobalValue::ret_power);
    config.setValue("result/pwr_x", GlobalValue::ret_pwr_x);
    config.setValue("result/pwr_y", GlobalValue::ret_pwr_y);
    config.setValue("result/ast", GlobalValue::ret_ast);
    config.setValue("result/coma", GlobalValue::ret_coma);
    config.setValue("result/sph", GlobalValue::ret_sph);
    config.setValue("result/ttv", GlobalValue::ret_ttv);
    config.setValue("result/fringe", GlobalValue::ret_fringe);


    QSettings parameter("parameter.ini", QSettings::IniFormat);

    //parameter.setValue("check/check", GlobalValue::chk_check);
    parameter.setValue("check/checkTimes", GlobalValue::chk_cts);
    parameter.setValue("check/psi_shift_thresh1", GlobalValue::chk_p_sh_t1);
    parameter.setValue("check/psi_std_thresh1", GlobalValue::chk_p_s_t1);

    parameter.setValue("parameter/psi", GlobalValue::par_psi);
    parameter.setValue("parameter/unwrap", GlobalValue::par_unw);
    parameter.setValue("parameter/zernikeTerm", GlobalValue::par_ztm);
    parameter.setValue("parameter/intf_scale_factor", GlobalValue::par_i_s_f);
    parameter.setValue("parameter/test_wavelength", GlobalValue::par_t_w);
    parameter.setValue("parameter/iso_wavelength", GlobalValue::par_i_w);
    parameter.setValue("parameter/highPass_filterCoef", GlobalValue::par_hp_fc);
    parameter.setValue("parameter/use_fillSpikes", GlobalValue::par_use_fs);
}

void MainWindow::updateView(bool state)
{
    if (state) {//Hcompensate对于result的补偿值
        bool Hcompensate=false;
        if(this->width()>1600){
            Hcompensate=true;
        }
        ui->graphicsView->setGeometry(10, 10,Hcompensate==false?800:800*(this->width()-40)/1560,
                                      this->height()>900?500*(this->height()-90)/810:500);
        ui->data->setGeometry(10, ui->graphicsView->height() + 20,ui->graphicsView->width(),
                              this->height()>900?310*(this->height()-90)/810:
                                                 310*(this->height()-90)/810+500*(this->height()-90)/810-500);

        ui->result->setGeometry(ui->graphicsView->width() + 20, 10,
                                Hcompensate==false?280*(this->width()-40)/1560+480*(this->width()-40)/1560-480+800*(this->width()-40)/1560-800:
                                               280*(this->width()-40)/1560+480*(this->width()-40)/1560-480,
                                ui->graphicsView->height());

        ui->view->setGeometry(ui->graphicsView->width() + ui->result->width() + 30, 10,
                              480, 820*(this->height()-90)/810);

        ui->quality->setGeometry(ui->result->x(), ui->data->y(), ui->result->width(), ui->data->height());

    } else {
        this->resize(1600, 900);
        ui->graphicsView->setGeometry(10, 10, 800, 500);
        ui->result->setGeometry(820, 10, 280, 500);
        ui->view->setGeometry(1110, 10, 480, 820);
        ui->data->setGeometry(10, 520, 800, 310);
        ui->quality->setGeometry(820, 520, 280, 310);
    }
    modifyScale();
    ui->result_table->resize(ui->result->width(), ui->result->height() - 20);
    ui->view_table->resize(ui->view->width(), ui->view->height() - 20);
    ui->data_table->resize(ui->data->width(), ui->data->height() - 20);
    ui->quality_table->resize(ui->quality->width(), ui->quality->height() - 20);
    ui->quality_table->setColumnWidth(0, ui->quality->width());
    double height = (ui->quality_table->height()-1)/10;
    for ( int i = 0; i < GlobalValue::com_h_num; ++i )
    {
        ui->quality_table->setRowHeight(i, height);
    }
}

void MainWindow::modifyScale()
{
    int picWidth = m_scene->getPixmapItem().pixmap().width();
    int picHeight = m_scene->getPixmapItem().pixmap().height();
    int sceneWidth = ui->graphicsView->width() - 20;
    int sceneHeight = ui->graphicsView->height() - 20;

    double w_sceneScale = floor(picWidth/(sceneWidth*1.0f) * 100.000f) / 100.000f;
    double h_sceneScale = floor(picHeight/(sceneHeight*1.0f) * 100.000f) / 100.000f;
    double sceneScale = w_sceneScale >= h_sceneScale ? w_sceneScale : h_sceneScale;

    double w_viewScale = floor(sceneWidth/(picWidth*1.0f) * 100.000f) / 100.000f;
    double h_viewScale = floor(sceneHeight/(picHeight*1.0f) * 100.000f) / 100.000f;
    double viewScale = w_viewScale <= h_viewScale ? w_viewScale : h_viewScale;

    if ( picWidth == 780 && picHeight == 480 ) {
        sceneScale = viewScale = 1;
    }
    GlobalValue::com_p_sle = viewScale;

    ui->graphicsView->setSceneRect(-sceneWidth/2 * sceneScale, -sceneHeight/2 * sceneScale,
                                   sceneWidth * sceneScale, sceneHeight * sceneScale);

    ui->graphicsView->resetTransform();
    ui->graphicsView->scale(viewScale, viewScale);
}

void MainWindow::changeLanguage(int type)
{
    QString source = "";
    switch (type)
    {
    case 1: {
        source = ":/source/strings-C.json";
        ui->result->setTabText(0, QStringLiteral("检测结果"));
        ui->view->setTabText(0, QStringLiteral("二维图形"));
        ui->data->setTabText(0, QStringLiteral("数据保存"));
        ui->quality->setTabText(0, QStringLiteral("质检分类"));
    } break;
    case 2: {
        source = ":/source/strings-E.json";
        ui->result->setTabText(0, QStringLiteral("Result"));
        ui->view->setTabText(0, QStringLiteral("2D"));
        ui->data->setTabText(0, QStringLiteral("Data"));
        ui->quality->setTabText(0, QStringLiteral("Quality"));
    } break;
    default: break;
    }

    QJsonObject obj = GlobalFun::getJsonObj(source);
    GlobalString::menu_file = obj.value("menu_file").toString();
    GlobalString::menu_camera = obj.value("menu_camera").toString();
    GlobalString::menu_shifter = obj.value("menu_shifter").toString();
    GlobalString::menu_language = obj.value("menu_language").toString();

    GlobalString::action_load = obj.value("action_load").toString();
    GlobalString::action_save = obj.value("action_save").toString();
    GlobalString::action_screenshot = obj.value("action_screenshot").toString();
    GlobalString::action_calibration = obj.value("action_calibration").toString();
    GlobalString::action_chinese = obj.value("action_chinese").toString();
    GlobalString::action_english = obj.value("action_english").toString();
    GlobalString::action_aim = obj.value("action_aim").toString();
    GlobalString::action_calculate=obj.value("action_calculate").toString();
    GlobalString::action_analysis= obj.value("action_analysis").toString();
    GlobalString::action_setting = obj.value("action_setting").toString();
    GlobalString::action_Loopcheck= obj.value("action_Loopcheck").toString();
    GlobalString::action_download= obj.value("action_download").toString();
    GlobalString::action_get= obj.value("action_get").toString();
    GlobalString::action_lock= obj.value("action_lock").toString();

    GlobalString::graphics_circle = obj.value("graphics_circle").toString();
    GlobalString::graphics_ellipse = obj.value("graphics_ellipse").toString();
    GlobalString::graphics_concentric_circle = obj.value("graphics_concentric_circle").toString();
    GlobalString::graphics_rectangle = obj.value("graphics_rectangle").toString();
    GlobalString::graphics_square = obj.value("graphics_square").toString();
    GlobalString::graphics_polygon = obj.value("graphics_polygon").toString();
    GlobalString::graphics_pill = obj.value("graphics_pill").toString();
    GlobalString::graphics_chamfer = obj.value("graphics_chamfer").toString();

    GlobalString::graphics_auto_circle = obj.value("graphics_auto_circle").toString();
    GlobalString::graphics_auto_ellipse = obj.value("graphics_auto_ellipse").toString();
    GlobalString::graphics_auto_pill = obj.value("graphics_auto_pill").toString();
    GlobalString::graphics_auto_chamfer = obj.value("graphics_auto_chamfer").toString();
    GlobalString::graphics_auto_roundEdgeRec = obj.value("graphics_auto_roundEdgeRec").toString();
    GlobalString::graphics_auto_rotateRec = obj.value("graphics_auto_rotateRec").toString();

    GlobalString::graphics_lock = obj.value("graphics_lock").toString();
    GlobalString::graphics_unlock = obj.value("graphics_unlock").toString();
    GlobalString::graphics_live = obj.value("graphics_live").toString();
    GlobalString::graphics_showMask = obj.value("graphics_showMask").toString();
    GlobalString::graphics_manual = obj.value("graphics_manual").toString();
    GlobalString::graphics_auto = obj.value("graphics_auto").toString();
    GlobalString::graphics_reset = obj.value("graphics_reset").toString();
    GlobalString::graphics_hole_num = obj.value("graphics_hole_num").toString();
    GlobalString::graphics_sure = obj.value("graphics_sure").toString();
    GlobalString::graphics_change = obj.value("graphics_change").toString();

    GlobalString::contextMenu_clear = obj.value("contextMenu_clear").toString();
    GlobalString::contextMenu_delete = obj.value("contextMenu_delete").toString();
    GlobalString::contextMenu_export = obj.value("contextMenu_export").toString();
    GlobalString::contextMenu_show = obj.value("contextMenu_show").toString();
    GlobalString::contextMenu_sure = obj.value("contextMenu_sure").toString();
    GlobalString::contextMenu_unit = obj.value("contextMenu_unit").toString();

    ui->menu_file->setTitle(GlobalString::menu_file);
    ui->menu_camera->setTitle(GlobalString::menu_camera);
    ui->menu_shifter->setTitle(GlobalString::menu_shifter);
    ui->menu_language->setTitle(GlobalString::menu_language);

    ui->action_load->setText(GlobalString::action_load);
    ui->action_save->setText(GlobalString::action_save);
    ui->action_screenshot->setText(GlobalString::action_screenshot);
    ui->action_camera_setting->setText(GlobalString::action_setting);
    ui->action_camera_calibration->setText(GlobalString::action_calibration);
    ui->action_shifter_setting->setText(GlobalString::action_setting);
    ui->action_shifter_calibration->setText(GlobalString::action_calibration);
    ui->action_chinese->setText(GlobalString::action_chinese);
    ui->action_english->setText(GlobalString::action_english);
    /* 工具栏的所有 */
    ui->action_calculate->setToolTip(GlobalString::action_calculate);
    ui->action_analysis->setText(GlobalString::action_analysis);
    ui->action_setting->setText(GlobalString::action_setting);
    ui->action_Loopcheck->setText(GlobalString::action_Loopcheck);
    ui->action_camera_aim->setText(GlobalString::action_aim);
    ui->action_download->setText(GlobalString::action_download);
    ui->action_get->setToolTip(GlobalString::action_get);
    ui->action_lock->setToolTip(GlobalString::action_lock);

    GlobalValue::lgn_tp = type;

}

void MainWindow::changeStatus(bool status)
{
//    if(check_loop_timer.isActive()){
//        ui->graphicsView->setEnabled(false);
//        ui->result_table->setEnabled(false);
//        ui->data_table->setEnabled(false);
//        ui->quality_table->setEnabled(false);
//    }else{
//        ui->menu_file->setEnabled(status);
//        ui->menu_camera->setEnabled(status);
//        ui->menu_shifter->setEnabled(status);
//        ui->menu_language->setEnabled(status);
//        //ui->action_calculate->setEnabled(status);
//        ui->action_analysis->setEnabled(status);
//        ui->graphicsView->setEnabled(status);
//        ui->result_table->setEnabled(status);
//        ui->data_table->setEnabled(status);
//        ui->quality_table->setEnabled(status);
//    }

//    is_in_calculation = !status;
//    if(status){
//        ui->action_lock->triggered(!status);//计算后是上锁还是解锁，这里是解锁,这里根据需求改动triggered true or false
//        ui->action_lock->setEnabled(status);
//    }else{
//        ui->action_lock->triggered(!status);
//        ui->action_lock->setEnabled(status);
//    }

//    return;
    /* 原函数 */
    ui->menu_file->setEnabled(status);
    ui->menu_camera->setEnabled(status);
    ui->menu_shifter->setEnabled(status);
    ui->menu_language->setEnabled(status);
    //ui->action_calculate->setEnabled(status);
    ui->action_analysis->setEnabled(status);
    ui->graphicsView->setEnabled(status);
    ui->result_table->setEnabled(status);
    ui->data_table->setEnabled(status);
    ui->quality_table->setEnabled(status);
    is_in_calculation = !status;
    if(status){
        ui->action_lock->triggered(!status);
        ui->action_lock->setEnabled(status);
    }else{
        ui->action_lock->triggered(!status);
        ui->action_lock->setEnabled(status);
    }
}

void MainWindow::hideConfig(QString path)
{
    SetFileAttributes((LPCWSTR)path.unicode(), FILE_ATTRIBUTE_HIDDEN);
}

void MainWindow::createDefaultConfig()
{
    QString config = "config.ini";
                                // 单位 0-λ 1-fr 2-nm 3-μm 4-mm
    if ( !GlobalFun::isFileExist(config) ) {
        QSettings settings(config, QSettings::IniFormat);
        settings.setValue("camera/type", 1);
        settings.setValue("camera/interval", 100);
        settings.setValue("camera/exposure", 33000);
        settings.setValue("camera/exposure_delay", 10);
        settings.setValue("camera/format", "Y800 (1920x1200)");
        settings.setValue("camera/fps", 30.0);
        settings.setValue("camera/pixel_calibration", 47.6);

        settings.setValue("common/hole_number", 2);
        settings.setValue("common/type", 1);//==================
        settings.setValue("common/pic_scale", 1);
        settings.setValue("common/save_picture", 0);
        settings.setValue("common/log_path", "D:/DataLog");
        settings.setValue("common/pic_path", "D:/PicSave");
        settings.setValue("common/auto_test_interval", 5000);
        settings.setValue("common/unit", 0);
        settings.setValue("common/value", 632.8);                       // 1个λ对应多少nm
        settings.setValue("language/type", 1);

        settings.setValue("shifter/channel_number", 0);
        settings.setValue("shifter/step_voltage", 0.1);
        settings.setValue("shifter/interval_voltage", 0.775);
        settings.setValue("shifter/voltage_calibration", 3.1);
        settings.setValue("shifter/step_rest", 5);
        settings.setValue("shifter/interval_rest", 50);
        settings.setValue("shifter/add_several_times", 8);

        settings.setValue("graphicsItem/circle_radius", 142.8);
        settings.setValue("graphicsItem/ellipse_width", 190.4);
        settings.setValue("graphicsItem/ellipse_height", 142.8);
        settings.setValue("graphicsItem/conCircle_radius_1", 142.8);
        settings.setValue("graphicsItem/conCircle_radius_2", 238);
        settings.setValue("graphicsItem/rectangle_width", 190.4);
        settings.setValue("graphicsItem/rectangle_height", 142.8);
        settings.setValue("graphicsItem/square_length", 142.8);
        settings.setValue("graphicsItem/pill_width", 285.6);
        settings.setValue("graphicsItem/pill_height", 71.4);
        settings.setValue("graphicsItem/chamfer_width", 190.4);
        settings.setValue("graphicsItem/chamfer_height", 142.8);
        settings.setValue("graphicsItem/chamfer_radius", 23.8);
        settings.setValue("graphicsItem/default_size", 1.0);

        settings.setValue("zernike/piston", 1);
        settings.setValue("zernike/tilt", 1);
        settings.setValue("zernike/power", 0);
        settings.setValue("zernike/ast", 0);
        settings.setValue("zernike/coma", 0);
        settings.setValue("zernike/spherical", 0);

        settings.setValue("result/pv", 1);
        settings.setValue("result/pv_x", 0);
        settings.setValue("result/pv_y", 0);
        settings.setValue("result/pv_res", 1);
        settings.setValue("result/rms", 1);
        settings.setValue("result/rms_res", 1);
        settings.setValue("result/tilt", 0);
        settings.setValue("result/power", 0);
        settings.setValue("result/pwr_x", 0);
        settings.setValue("result/pwr_y", 0);
        settings.setValue("result/ast", 0);
        settings.setValue("result/coma", 0);
        settings.setValue("result/sph", 0);
        settings.setValue("result/ttv", 0);
        settings.setValue("result/fringe", 0);
    }

    QString parameter1 = "parameter1.ini";
    if ( !GlobalFun::isFileExist(parameter1) ) {
        QSettings settings(parameter1, QSettings::IniFormat);
        settings.setValue("parameter/filter", 0);               // 滤波 0-none 1-LOW_PASS 2-HIGH_PASS 3-BAND_PASS 4-BAND_REJECT
        settings.setValue("parameter/fws", 5);                  // filter windows size
        settings.setValue("parameter/remove_spikes", 0);        // 去毛刺
        settings.setValue("parameter/slopeRemoveSize", 1);      // 去毛刺阈值像素数
        settings.setValue("parameter/slopeThresh", 6);          // 去毛刺阈值
        settings.setValue("parameter/remove_residual", 1);      // 去残差
        settings.setValue("parameter/refractive_index", 1.5);   // 折射率，计算ttv时要用    
    }

    QString parameter = "parameter.ini";
    if ( !GlobalFun::isFileExist(parameter) ) {
        QSettings settings(parameter, QSettings::IniFormat);

        //settings.setValue("check/check", 0);                    // 检验
        settings.setValue("check/checkTimes", 3);               // 复测次数
        settings.setValue("check/psi_shift_thresh1", 3);        // 相移角度阈值
        settings.setValue("check/psi_std_thresh1", 0.05);       // 角度直方图的方差 [70,110]
        settings.setValue("check/res_pv_thresh1", 0.095);       // 残差的pv阈值

        settings.setValue("parameter/psi", 1);                  // 0-4A 1-5AH 2-5BCS 3-9ACS 4-9BCS
        settings.setValue("parameter/unwrap", 0);               // 0-branchcut 1-histogram
        settings.setValue("parameter/zernikeTerm", 37);         // zernike显示数量
        settings.setValue("parameter/intf_scale_factor", 0.5);  // 内标度系数
        settings.setValue("parameter/test_wavelength", 632.8);  // 检测波长
        settings.setValue("parameter/iso_wavelength", 546);     // iso波长
        settings.setValue("parameter/highPass_filterCoef", 0.005); // 计算高通滤波半径的系数
        settings.setValue("parameter/use_fillSpikes", 1);       // 是否启用（将毛刺点处的值用拟合的结果填充）0-不启用 1-启用

        settings.setValue("algorithm/pv", 1);                   // zernike系数拟合结果的pv,透过波前的光程差
        settings.setValue("algorithm/pv_x", 1);                 // zernike系数拟合结果某一行的pv
        settings.setValue("algorithm/pv_y", 1);                 // zernike系数拟合结果某一列的pv
        settings.setValue("algorithm/pv_xy", 0);                // zernike系数拟合结果某一列和某一列的pv，pv_xy = (pv_x + pv_y) / 2
        settings.setValue("algorithm/pv_res", 1);               // 残差的pv
        settings.setValue("algorithm/pvr", 0);                  // pvr = pv(zernike)+ 3 * rms(res)
        settings.setValue("algorithm/rms", 1);                  // zernike系数拟合结果的均方根
        settings.setValue("algorithm/rms_x", 0);                // zernike系数拟合结果某一行的均方根
        settings.setValue("algorithm/rms_y", 0);                // zernike系数拟合结果某一列的均方根
        settings.setValue("algorithm/rms_xy", 0);               // zernike系数拟合结果某一列和某一列的pv，rms_xy = (rms_x + rms_y) / 2
        settings.setValue("algorithm/rms_res", 1);              // 残差的均方根

        settings.setValue("algorithm/zernike_tilt", 1);         // zernike tilt
        settings.setValue("algorithm/zernike_power", 1);        // zernike power
        settings.setValue("algorithm/zernike_ast", 1);          // zernike astigmatism
        settings.setValue("algorithm/zernike_coma", 1);         // zernike coma
        settings.setValue("algorithm/zernike_spherical", 1);    // zernike Spherical
        settings.setValue("algorithm/seidel_tilt", 0);          // seidel tilt
        settings.setValue("algorithm/seidel_focus", 0);         // seidel focus
        settings.setValue("algorithm/seidel_ast", 0);           // seidel astigmatism
        settings.setValue("algorithm/seidel_coma", 0);          // seidel coma
        settings.setValue("algorithm/seidel_spherical", 0);     // seidel spherical
        settings.setValue("algorithm/seidel_tilt_clock", 0);    // seidel tilt clock
        settings.setValue("algorithm/seidel_ast_clock", 0);     // seidel ast clock
        settings.setValue("algorithm/seidel_coma_clock", 0);    // seidel coma clock
        settings.setValue("algorithm/rms_power", 0);            // zernike_power / 2 / 3^0.5
        settings.setValue("algorithm/rms_ast", 0);              // seidel_ast / 2 / 6^0.5
        settings.setValue("algorithm/rms_coma", 0);             // seidel_coma / 6 / 2^0.5
        settings.setValue("algorithm/rms_sa", 0);               // seidel_spherical / 6 / 5^0.5

        settings.setValue("algorithm/sag", 0);                  // 弧矢误差
        settings.setValue("algorithm/irr", 0);                  // 不规则偏差
        settings.setValue("algorithm/rsi", 0);                  // 旋转对称不规则偏差
        settings.setValue("algorithm/rmst", 0);                 // 总表面偏差的均方根
        settings.setValue("algorithm/rmsa", 0);                 // 不规则偏差的均方根
        settings.setValue("algorithm/rmsi", 0);                 // 非旋转对称偏差的均方根

        settings.setValue("algorithm/ttv", 1);                  // 几何轮廓的厚度差
        settings.setValue("algorithm/fringe", 1);               // 条纹数
        settings.setValue("algorithm/strehl", 0);               // strehl ？
        settings.setValue("algorithm/aperture", 1);             // 口径
        settings.setValue("algorithm/sizeX", 0);                // x方向的尺寸
        settings.setValue("algorithm/sizeY", 0);                // y方向的尺寸
        settings.setValue("algorithm/parallelTheta", 0);        // 平行角度
        settings.setValue("algorithm/thickness", 0);            // 测量厚度
        settings.setValue("algorithm/category", 0);             // 是否合格
        settings.setValue("algorithm/concavity", 0);            // 凹度，-1表示凸，1表示凹
    }

    hideConfig(GlobalFun::getCurrentPath() + "/" + config);
    hideConfig(GlobalFun::getCurrentPath() + "/" + parameter1);
    hideConfig(GlobalFun::getCurrentPath() + "/" + parameter);
}

void MainWindow::createHoleConfig(QString &path)
{
    QSettings settings(path, QSettings::IniFormat);

    settings.setValue("property/type", 1);                      // 孔类别

    settings.setValue("property/pv_min", 0);                    // pv 最小值
    settings.setValue("property/pv_max", 1);                    // pv 最大值
    settings.setValue("property/pv_x_min", 0);                  // pv_x 最小值
    settings.setValue("property/pv_x_max", 1);                  // pv_x 最大值
    settings.setValue("property/pv_y_min", 0);                  // pv_y 最小值
    settings.setValue("property/pv_y_max", 1);                  // pv_y 最大值
    settings.setValue("property/pv_res_min", 0);                // pv_res 最小值
    settings.setValue("property/pv_res_max", 1);                // pv_res 最大值
    settings.setValue("property/rms_min", 0);                   // rms 最小值
    settings.setValue("property/rms_max", 1);                   // rms 最大值
    settings.setValue("property/rms_res_min", 0);               // rms_res 最小值
    settings.setValue("property/rms_res_max", 1);               // rms_res 最大值
    settings.setValue("property/tilt_min", 0);                  // tilt 最小值
    settings.setValue("property/tilt_max", 1);                  // tilt 最大值
    settings.setValue("property/power_min", 0);                 // power 最小值
    settings.setValue("property/power_max", 1);                 // power 最大值
    settings.setValue("property/pwr_x_min", 0);                 // pwr_x 最小值
    settings.setValue("property/pwr_x_max", 1);                 // pwr_x 最大值
    settings.setValue("property/pwr_y_min", 0);                 // pwr_y 最小值
    settings.setValue("property/pwr_y_max", 1);                 // pwr_y 最大值
    settings.setValue("property/ast_min", 0);                   // ast 最小值
    settings.setValue("property/ast_max", 1);                   // ast 最大值
    settings.setValue("property/coma_min", 0);                  // coma 最小值
    settings.setValue("property/coma_max", 1);                  // coma 最大值
    settings.setValue("property/sph_min", 0);                   // sph 最小值
    settings.setValue("property/sph_max", 1);                   // sph 最大值
    settings.setValue("property/ttv_min", 0);                   // ttv 最小值
    settings.setValue("property/ttv_max", 1);                   // ttv 最大值
    settings.setValue("property/fringe_min", 0);                // fringe 最小值
    settings.setValue("property/fringe_max", 1);                // fringe 最大值
}

void MainWindow::createResultTable(int num)
{
    ui->result_table->clear();
    ui->result_table->setRowCount(num);
    QStringList rowHead;
    int columnCount = 0;
    QStringList columnHead;
    if (GlobalValue::ret_pv == 1) { columnCount++; columnHead << "pv" << "pv_min" << "pv_max"; }
    if (GlobalValue::ret_pv_x == 1) { columnCount++; columnHead << "pv_x" << "pv_x_min" << "pv_x_max"; }
    if (GlobalValue::ret_pv_y == 1) { columnCount++; columnHead << "pv_y" << "pv_y_min" << "pv_y_max"; }
    if (GlobalValue::ret_pv_res == 1) { columnCount++; columnHead << "pv_res" << "pv_res_min" << "pv_res_max"; }
    if (GlobalValue::ret_rms == 1) { columnCount++; columnHead << "rms" << "rms_min" << "rms_max"; }
    if (GlobalValue::ret_rms_res == 1) { columnCount++; columnHead << "rms_res" << "rms_res_min" << "rms_res_max"; }
    if (GlobalValue::ret_tilt == 1) { columnCount++; columnHead << "tilt" << "tilt_min" << "tilt_max"; }
    if (GlobalValue::ret_power == 1) { columnCount++; columnHead << "power" << "power_min" << "power_max"; }
    if (GlobalValue::ret_pwr_x == 1) { columnCount++; columnHead << "pwr_x" << "pwr_x_min" << "pwr_x_max"; }
    if (GlobalValue::ret_pwr_y == 1) { columnCount++; columnHead << "pwr_y" << "pwr_y_min" << "pwr_y_max"; }
    if (GlobalValue::ret_ast == 1) { columnCount++; columnHead << "ast" << "ast_min" << "ast_max"; }
    if (GlobalValue::ret_coma == 1) { columnCount++; columnHead << "coma" << "coma_min" << "coma_max"; }
    if (GlobalValue::ret_sph == 1) { columnCount++; columnHead << "sph" << "sph_min" << "sph_max"; }
    if (GlobalValue::ret_ttv == 1) { columnCount++; columnHead << "ttv" << "ttv_min" << "ttv_max"; }
    if (GlobalValue::ret_fringe == 1) { columnCount++; columnHead << "fringe" << "fringe_min" << "fringe_max"; }
    ui->result_table->setColumnCount(columnCount*3);
    //增加一个筛选
    QStringList temp,head;
    foreach (auto& variable, columnHead){
        if(variable.endsWith("_max") || variable.endsWith("_min")){
            temp.push_back(variable);
        }else{
            head.push_back(variable);
        }
    }
    head+=temp;
    QFont font0;
    font0.setPixelSize(10);
    ui->result_table->setHorizontalHeaderLabels(head);
    ui->result_table->horizontalHeader()->setFont(font0);//直接设置头部字体
    for ( int i = 0; i < columnCount;++i )
    {
        QFont font;
        font.setPixelSize(12);
        font.setBold(true);
        ui->result_table->horizontalHeaderItem(i)->setFont(font);
    }

     //setting
    ui->result_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->result_table->horizontalHeader()->setFrameShape(QFrame::NoFrame);
    ui->result_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->result_table->verticalHeader()->setFrameShape(QFrame::NoFrame);

    ui->result_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->result_table->setAlternatingRowColors(true);
    ui->result_table->setContextMenuPolicy(Qt::CustomContextMenu);

    // data
    for ( int i = 0; i < num; ++i )
    {
        QString source = QString("hole_%1_config.ini").arg(i+1);
        for ( int j = 0; j < columnCount*3; ++j )
        {
            if ( j<columnCount ) {
                QTableWidgetItem *item = new QTableWidgetItem("nan");
                QFont font;
                font.setPixelSize(12);
                item->setFont(font);
                item->setTextAlignment(Qt::AlignCenter);
                item->setBackgroundColor(QColor(187, 255, 255));
                item->setTextColor(QColor(54, 54, 54));
                ui->result_table->setColumnWidth(j,72);
                ui->result_table->setItem(i, j, item);
                ui->result_table->setRowHeight(i,40);
            } else {
                double temp = GlobalFun::getProperty(source, "property", ui->result_table->horizontalHeaderItem(j)->text()).toDouble();//存在隐患
                QDoubleSpinBox *box = new QDoubleSpinBox(ui->result_table);
                box->setAlignment(Qt::AlignCenter);
                box->setRange(-100, 100);
                box->setSingleStep(0.1);
                box->setDecimals(3);
                box->setValue(temp);
                box->setEnabled(false);
                QFont font;
                font.setPixelSize(12);
                box->setFont(font);
                ui->result_table->setColumnWidth(j,72);
                ui->result_table->setRowHeight(i,40);
                ui->result_table->setCellWidget(i, j, box);
                connect(box, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [&](double v){
                    int rowIndex = ui->result_table->currentRow();
                    int columnIndex = ui->result_table->currentColumn();
                    QString config = QString("hole_%1_config.ini").arg(rowIndex+1);
                    QString str = ui->result_table->horizontalHeaderItem(columnIndex)->text();
                    GlobalFun::setProperty(config, "property", str, QString::number(v));
                    hideConfig(GlobalFun::getCurrentPath() + "/" + config);
                });
            }
        }
    }
    is_locked = true;
}

void MainWindow::createViewTable(int num)//二维图像
{
    ui->view_table->clear();
    int rowCnt=ceil(static_cast<double>(num)/2);//注意这里，如果不强转奇数的结果会少一
    ui->view_table->setColumnCount(2);
    ui->view_table->setColumnWidth(0, (ui->view_table->width())/2);
    ui->view_table->setColumnWidth(1, (ui->view_table->width())/2);
    ui->view_table->setRowCount(rowCnt);
    ui->view_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->view_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->view_table->setStyleSheet("selection-background-color:transparent;");
    ui->view_table->setFrameShape(QFrame::NoFrame);
    ui->view_table->setLineWidth(0);
    int width=(ui->view_table->width()-Hmarin)/2;
    int height=800/5;//这里设置了100在on_fillData里需要把这个减去
    int pageheight=20;
    for ( int i = 0; i < num; i++ )
    {
        ui->view_table->setRowHeight(i,height);
        QWidget* tmpview=new QWidget();
        tmpview->setFixedSize(width-2,height);

        QCustomPlot *customPlot = new QCustomPlot(tmpview);
        customPlot->setFixedSize(tmpview->width(), tmpview->height()-pageheight/2);
        QCPColorScale *colorScale = new QCPColorScale(customPlot);// 颜色滚动条
        colorScale->axis()->setTicks(true);// 设置标签可见
        colorScale->axis()->setRange(0, 1);// 设置范围
        colorScale->setType(QCPAxis::atRight);// 设置显示位置
        colorScale->setRangeDrag(false);// 设置不能拖动
        colorScale->setRangeZoom(false);// 设置不能滚动
        colorScale->setBarWidth(8);//类似于图像温度计在view_table区域  // 设置宽度为8
        QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
        colorMap->data()->setSize(200, 200);// 调整数据数组的大小，key和value都是200
        colorMap->data()->setRange(QCPRange(0, 200), QCPRange(0, 200));// 设置key和value的范围都是0-200
        colorMap->setColorScale(colorScale);// 设置色谱图的颜色滚动条
        colorMap->setGradient(cpg);// 设置用于表示数据的颜色渐变
        colorMap->rescaleDataRange(true);// 设置数据范围以跨越当前数据集中出现的最小值和最大值
        customPlot->plotLayout()->addElement(0, 1, colorScale);// 将元素添加到具有行和列的单元格中
        customPlot->rescaleAxes();// 重新缩放轴，使绘图中的所有绘图表完全可见

        QLabel* page=new QLabel("( "+QString::number(1+i)+" )",tmpview);
        QFont font;
        font.setPixelSize(9);
        page->setFont(font);
        page->setGeometry(0,customPlot->height()-pageheight/2,width,pageheight);
        page->setAlignment(Qt::AlignCenter);

        ui->view_table->setCellWidget(i/2,i%2==0?0:1,tmpview);
    }
    // setting
    ui->view_table->horizontalHeader()->setVisible(false);
    ui->view_table->verticalHeader()->setVisible(false);
    ui->view_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::createDataTable(int num)
{
    // clear
    ui->data_table->clear();

    // column count
    int columnCount = 3;
    QStringList columnHead;
    columnHead << "productID" << "date"<< "total";

    for ( int i = 0; i < num; ++i )
    {
        QString str = "#" + QString::number(i+1) + "_";
        //columnCount ++; //columnHead << str + "type";
        if (GlobalValue::ret_pv == 1) { columnCount ++; columnHead << str + "pv"; }
        if (GlobalValue::ret_pv_x == 1) { columnCount ++; columnHead << str + "pv_x"; }
        if (GlobalValue::ret_pv_y == 1) { columnCount ++; columnHead << str + "pv_y"; }
        if (GlobalValue::ret_pv_res == 1) { columnCount ++; columnHead << str + "pv_res"; }
        if (GlobalValue::ret_rms == 1) { columnCount ++; columnHead << str + "rms"; }
        if (GlobalValue::ret_rms_res == 1) { columnCount ++; columnHead << str + "rms_res"; }
        if (GlobalValue::ret_tilt == 1) { columnCount ++; columnHead << str + "tilt"; }
        if (GlobalValue::ret_power == 1) { columnCount ++; columnHead << str + "power"; }
        if (GlobalValue::ret_pwr_x == 1) { columnCount ++; columnHead << str + "pwr_x"; }
        if (GlobalValue::ret_pwr_y == 1) { columnCount ++; columnHead << str + "pwr_y"; }
        if (GlobalValue::ret_ast == 1) { columnCount ++; columnHead << str + "ast"; }
        if (GlobalValue::ret_coma == 1) { columnCount ++; columnHead << str + "coma"; }
        if (GlobalValue::ret_sph == 1) { columnCount ++; columnHead << str + "sph"; }
        if (GlobalValue::ret_ttv == 1) { columnCount ++; columnHead << str + "ttv"; }
        if (GlobalValue::ret_fringe == 1) { columnCount ++; columnHead << str + "fringe"; }
        columnCount ++; columnHead << str + "aperture";
        columnCount++; columnHead << str + "qc";
    }
    ui->data_table->setColumnCount(columnCount);
    ui->data_table->setHorizontalHeaderLabels(columnHead);

    QString style = "QHeaderView { font-size: 12px }"
                    "QHeaderView::section { background-color: #2c3e50;"
                                           "color: white;"
                                           "height: 24px;"
                                           "border-left: 2px solid #FFFFFF;"
                                           "border-right: 2px solid #FFFFFF; }"
                    "QHeaderView::section:checked { background-color: #8B0000; }";

    // setting
    ui->data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->data_table->horizontalHeader()->setStyleSheet(style);
    ui->data_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->data_table->verticalHeader()->setFrameShape(QFrame::NoFrame);
    ui->data_table->setAlternatingRowColors(true);
    ui->data_table->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->data_table->setFocusPolicy(Qt::StrongFocus);

    ui->data_table->setRowCount(0);
    ui->data_table->setColumnWidth(0, 100);
    ui->data_table->setColumnWidth(1, 100);
}

void MainWindow::createQualityTable(int num)
{
    ui->quality_table->clear();

    ui->quality_table->setRowCount(num);
    ui->quality_table->setColumnCount(1);
    ui->quality_table->setColumnWidth(0 , ui->quality_table->width());
    ui->quality_table->verticalHeader()->setVisible(false);
    ui->quality_table->horizontalHeader()->setVisible(false);
    ui->quality_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->quality_table->setSelectionMode(QAbstractItemView::NoSelection);
    ui->quality_table->setWordWrap(true);

    int height = (ui->quality_table->height()-1)/10;
    for ( int i = 0; i < num; ++i )
    {
        QTableWidgetItem *item = new QTableWidgetItem("hole-" + QString::number(i+1) + "-qc-" +  "nan");
        item->setBackgroundColor(QColor(187, 255, 255));
        item->setTextColor(QColor(54, 54, 54));
        QFont font("微软雅黑", 12);
        font.setBold(true);
        item->setFont(font);
        item->setTextAlignment(Qt::AlignCenter);
        ui->quality_table->setRowHeight(i, height);
        ui->quality_table->setItem(0, i, item);
    }
}

void MainWindow::createContextMenu(int type)//右键单击会产生的表格
{
    QMenu *menu = new QMenu(this);

    switch (type)
    {
    case 1: {//这里还没有改完
        QAction* options = new QAction(QIcon(":/images/Gear.png"), GlobalString::action_setting, menu);
        QAction* lock;
        if ( is_locked ) {
            lock = new QAction(QIcon(":/images/Unlock.png"), GlobalString::graphics_unlock, menu);
        } else {
            lock = new QAction(QIcon(":/images/Locked.png"), GlobalString::graphics_lock, menu);
        }
        connect(lock, &QAction::triggered, [&](){
            int columnCnt=0;
            for(columnCnt;ui->result_table->item(0,columnCnt)!=nullptr;++columnCnt){
                ;
            }
            for ( int i = 0; i < ui->result_table->rowCount(); ++i )
            {
                for ( int j = columnCnt; j < ui->result_table->columnCount(); ++j )
                {
                    ui->result_table->cellWidget(i, j)->setEnabled(is_locked);
                }
            }
            is_locked = !is_locked;
        });

        connect(options, &QAction::triggered, [&](){
            QMenu *chooseResultType = new QMenu(this);
            QVBoxLayout *layout = new QVBoxLayout();
            QHBoxLayout *top = new QHBoxLayout();
            QVBoxLayout *left = new QVBoxLayout();
            QVBoxLayout *right = new QVBoxLayout();

            QCheckBox *pv =      new QCheckBox("pv",        chooseResultType);
            QCheckBox *pv_x =    new QCheckBox("pv_x",      chooseResultType);
            QCheckBox *pv_y =    new QCheckBox("pv_y",      chooseResultType);
            QCheckBox *pv_res =  new QCheckBox("pv_res",    chooseResultType);
            QCheckBox *rms =     new QCheckBox("rms",       chooseResultType);
            QCheckBox *rms_res = new QCheckBox("rms_res",   chooseResultType);
            QCheckBox *tilt =    new QCheckBox("tilt",      chooseResultType);
            QCheckBox *power =   new QCheckBox("power",     chooseResultType);
            QCheckBox *pwr_x =   new QCheckBox("pwr_x",     chooseResultType);
            QCheckBox *pwr_y =   new QCheckBox("pwr_y",     chooseResultType);
            QCheckBox *ast =     new QCheckBox("ast",       chooseResultType);
            QCheckBox *coma =    new QCheckBox("coma",      chooseResultType);
            QCheckBox *sph =     new QCheckBox("sph",       chooseResultType);
            QCheckBox *ttv =     new QCheckBox("ttv",       chooseResultType);
            QCheckBox *fringe =  new QCheckBox("fringe",    chooseResultType);
            pv->setChecked(GlobalValue::ret_pv == 1);
            pv_x->setChecked(GlobalValue::ret_pv_x == 1);
            pv_y->setChecked(GlobalValue::ret_pv_y == 1);
            pv_res->setChecked(GlobalValue::ret_pv_res == 1);
            rms->setChecked(GlobalValue::ret_rms == 1);
            rms_res->setChecked(GlobalValue::ret_rms_res == 1);
            tilt->setChecked(GlobalValue::ret_tilt == 1);
            power->setChecked(GlobalValue::ret_power == 1);
            pwr_x->setChecked(GlobalValue::ret_pwr_x == 1);
            pwr_y->setChecked(GlobalValue::ret_pwr_y == 1);
            ast->setChecked(GlobalValue::ret_ast == 1);
            coma->setChecked(GlobalValue::ret_coma == 1);
            sph->setChecked(GlobalValue::ret_sph == 1);
            ttv->setChecked(GlobalValue::ret_ttv == 1);
            fringe->setChecked(GlobalValue::ret_fringe == 1);

            QPushButton *sure = new QPushButton(GlobalString::graphics_sure, chooseResultType);
            connect(sure, &QPushButton::clicked, [&](){
                GlobalValue::ret_pv = pv->isChecked() ? 1 : 0;
                GlobalValue::ret_pv_x = pv_x->isChecked() ? 1 : 0;
                GlobalValue::ret_pv_y = pv_y->isChecked() ? 1 : 0;
                GlobalValue::ret_pv_res = pv_res->isChecked() ? 1 : 0;
                GlobalValue::ret_rms = rms->isChecked() ? 1 : 0;
                GlobalValue::ret_rms_res = rms_res->isChecked() ? 1 : 0;
                GlobalValue::ret_tilt = tilt->isChecked() ? 1 : 0;
                GlobalValue::ret_power = power->isChecked() ? 1 : 0;
                GlobalValue::ret_pwr_x = pwr_x->isChecked() ? 1 : 0;
                GlobalValue::ret_pwr_y = pwr_y->isChecked() ? 1 : 0;
                GlobalValue::ret_ast = ast->isChecked() ? 1 : 0;
                GlobalValue::ret_coma = coma->isChecked() ? 1 : 0;
                GlobalValue::ret_sph = sph->isChecked() ? 1 : 0;
                GlobalValue::ret_ttv = ttv->isChecked() ? 1 : 0;
                GlobalValue::ret_fringe = fringe->isChecked() ? 1 : 0;

                createResultTable(ui->result_table->rowCount());
                createDataTable(ui->result_table->rowCount());
                modifyTableHead();
                chooseResultType->close();
            });

            left->addWidget(pv);
            left->addWidget(pv_x);
            left->addWidget(pv_y);
            left->addWidget(pv_res);
            left->addWidget(rms);
            left->addWidget(rms_res);
            left->addWidget(tilt);

            right->addWidget(power);
            right->addWidget(pwr_x);
            right->addWidget(pwr_y);
            right->addWidget(ast);
            right->addWidget(coma);
            right->addWidget(sph);
            right->addWidget(ttv);
            right->addWidget(fringe);

            top->addLayout(left);
            top->addLayout(right);
            layout->addLayout(top);
            layout->addWidget(sure);
            chooseResultType->setLayout(layout);

            chooseResultType->exec(QCursor::pos());
            delete chooseResultType;
        });

        menu->addAction(lock);
        menu->addAction(options);
    } break;
    case 2: {
        QAction* clear = new QAction(QIcon(":/images/Clear.png"), GlobalString::contextMenu_clear, menu);
        QAction* cutoff = new QAction(QIcon(":/images/Delete.png"), GlobalString::contextMenu_delete, menu);
        QAction* exportData = new QAction(QIcon(":/images/Folder.png"), GlobalString::contextMenu_export, menu);

        connect(clear, &QAction::triggered, [&](){
            ui->data_table->clearContents();
            ui->data_table->setRowCount(0);
            ui->data_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
            ui->data_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
            ui->data_table->setColumnWidth(0, 100);
            ui->data_table->setColumnWidth(1, 100);
        });

        connect(cutoff, &QAction::triggered, [&](){
            int index = ui->data_table->currentRow();
            if (index != -1) {
                ui->data_table->removeRow(index);
            }

            if ( ui->data_table->rowCount() == 0 ) {
                ui->data_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
                ui->data_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
                ui->data_table->setColumnWidth(0, 100);
                ui->data_table->setColumnWidth(1, 100);
            }
        });

        connect(exportData, &QAction::triggered, [&](){
            if ( ui->data_table->rowCount() == 0 ) {
                GlobalFun::showMessageBox(3, "No log to save !");
            } else {
                QString path = GlobalFun::getCurrentPath() + "/" + GlobalFun::getCurrentTime(1) + ".csv";
                QString file = QFileDialog::getSaveFileName(nullptr, GlobalString::action_save, path, "Text files (*.csv)");
                GlobalFun::exportCSV(file, ui->data_table);
            }
        });

        menu->addAction(clear);
        menu->addAction(cutoff);
        menu->addAction(exportData);
    } break;
    case 3: {
        QAction* options = new QAction(QIcon(":/images/Enlarge.png"), GlobalString::contextMenu_show, menu);
        menu->addAction(options);
        connect(options, &QAction::triggered, [&](){ createMenuDialog(8); });
    } break;
    default: break;
    }

    menu->exec(QCursor::pos());
    delete menu;
}

void MainWindow::createMenuDialog(int type)
{
    QString temp = GlobalValue::lgn_tp == 1 ? "" : " ";

    switch (type)
    {
    case 1: {
        if ( GlobalValue::cam_tp == 1 || GlobalValue::cam_tp == 3 ) {
            QString title = GlobalString::menu_camera + temp + GlobalString::action_setting;

            Camera_setting_dialog *dialog = new Camera_setting_dialog(title, GlobalString::graphics_sure,
                                                                      GlobalValue::cam_itl, GlobalValue::cam_exp, this);

            connect(dialog, &Camera_setting_dialog::changeValue, [&](int interval, double exposure){
                GlobalValue::cam_itl = interval;
                GlobalValue::cam_exp = exposure / 1000;

                if ( GlobalValue::cam_tp == 1 ) {
                    BBaslerCamerControl *bs_camera = dynamic_cast<BBaslerCamerControl *>(m_camera);
                    bs_camera->stop();
                    bs_camera->setInterval(interval);
                    bs_camera->start();
                    bs_camera->setCameraProperty(BBaslerCamerControl::BaslerCameraProperty::ExposureTime, exposure);
                } else {
                    BMVCameraControl *mv_camera = dynamic_cast<BMVCameraControl *>(m_camera);
                    mv_camera->stop();
                    mv_camera->setInterval(interval);
                    mv_camera->start();
                    mv_camera->setCameraProperty(BMVCameraControl::BMVCameraProperty::ExposureTime, exposure);
                }
            });

            dialog->exec();
            delete dialog;
        } else if ( GlobalValue::cam_tp == 2 ) {
            BICCameraControl *ic_camera = dynamic_cast<BICCameraControl *>(m_camera);
            ic_camera->showDialog();
        }
    } break;
    case 2: {
        QString title = GlobalString::menu_camera + temp + GlobalString::action_calibration;

        Camera_calibration_dialog *dialog = new Camera_calibration_dialog(title, m_scene->getPixmapItem().pixmap(),
                                                                          GlobalValue::cam_p_cal, this);

        connect(dialog, &Camera_calibration_dialog::changeValue, [](double pixel_calibration){
            GlobalValue::cam_p_cal = pixel_calibration;
            GlobalValue::gra_c_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;//多孔版本现在用的缩放值
            GlobalValue::gra_e_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
            GlobalValue::gra_e_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
            GlobalValue::gra_cc_rad_1 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
            GlobalValue::gra_cc_rad_2 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*5;
            GlobalValue::gra_r_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
            GlobalValue::gra_r_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
            GlobalValue::gra_s_len = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
            GlobalValue::gra_p_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size *2;
            GlobalValue::gra_p_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /2;
            GlobalValue::gra_ch_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
            GlobalValue::gra_ch_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
            GlobalValue::gra_ch_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /6;
        });

        dialog->exec();
        delete dialog;
    } break;
    case 3: {
        dialog->showWithClose();
    } break;
    case 4: {
        QString title = GlobalString::menu_shifter + temp + GlobalString::action_setting;

        Shifter_setting_dialog *dialog = new Shifter_setting_dialog(title, GlobalString::graphics_sure,
                                                                    GlobalValue::sft_s_vol, GlobalValue::sft_i_vol,
                                                                    GlobalValue::sft_s_rest, GlobalValue::sft_i_rest, this);

        connect(dialog, &Shifter_setting_dialog::changeValue, [](double s_vol, double i_vol, int s_rest, int i_rest){
            GlobalValue::sft_s_vol = s_vol;
            GlobalValue::sft_i_vol = i_vol;
            GlobalValue::sft_s_rest = s_rest;
            GlobalValue::sft_i_rest = i_rest;
            GlobalValue::sft_a_s_t = ceil(i_vol / s_vol);
        });

        dialog->exec();
        delete dialog;
    } break;
    case 5: {
        QString title = GlobalString::menu_shifter + temp + GlobalString::action_calibration;

        Shifter_calibration_dialog *dialog = new Shifter_calibration_dialog(title, m_scene->getPixmapItem().pixmap(),
                                                                            GlobalValue::sft_v_cal, this);

        connect(dialog, &Shifter_calibration_dialog::test, [&](double value, QRect rect){
            m_matList.clear();
            m_daCard->test(value, rect);
        });
        connect(this, &MainWindow::testFinished, dialog, [&](cv::Mat mat, float std, int shift){
            dialog->fillData(mat, std, shift, m_matList);
        }, Qt::DirectConnection);
        connect(dialog, &Shifter_calibration_dialog::changeValue, [](double voltage_calibration){
            GlobalValue::sft_v_cal = voltage_calibration;
            GlobalValue::sft_i_vol = voltage_calibration / 4;
            GlobalValue::sft_a_s_t = ceil(GlobalValue::sft_i_vol / GlobalValue::sft_s_vol);
        });

        dialog->exec();
        delete dialog;
    } break;
    case 6: {
        Customer_engineer_dialog *dialog = new Customer_engineer_dialog(this);
        connect(dialog,&Customer_engineer_dialog::unit_value_changed,[&](int index){
            switch (index) {
            case 0: ui->action_Company->setText(QStringLiteral("λ@") + QString::number(GlobalValue::com_value)); break;
            case 1: ui->action_Company->setText(QStringLiteral("fr@") + QString::number(GlobalValue::com_value)); break;
            case 2: ui->action_Company->setText(QStringLiteral("nm")) ;break;
            case 3: ui->action_Company->setText(QStringLiteral("μm")) ;break;
            case 4: ui->action_Company->setText(QStringLiteral("mm")) ;break;
            default: ui->action_Company->setText(QStringLiteral("nm"));break;
            }
        });
        dialog->exec();
        delete dialog;
    } break;
    case 7: {
        Our_engineer_dialog *dialog = new Our_engineer_dialog(this);
        dialog->exec();
        delete dialog;
    } break;
    case 8:{
        Unit_setting_dialog *dialog = new Unit_setting_dialog(GlobalString::contextMenu_unit, GlobalString::contextMenu_sure,
                                                              GlobalValue::com_unit, GlobalValue::com_value, this);
        connect(dialog, &Unit_setting_dialog::changeValue, [&](int type, double value){
            GlobalValue::com_unit = type;
            GlobalValue::com_value = value;

            QString unit = "";
            switch (GlobalValue::com_unit) {
            case 0: unit = QStringLiteral("λ"); ui->action_Company->setText(unit + "@" + QString::number(GlobalValue::com_value)); break;
            case 1: unit = QStringLiteral("fr"); ui->action_Company->setText(unit + "@" + QString::number(GlobalValue::com_value/2)); break;
            case 2: unit = QStringLiteral("nm"); ui->action_Company->setText(unit); break;
            case 3: unit = QStringLiteral("μm"); ui->action_Company->setText(unit); break;
            case 4: unit = QStringLiteral("mm"); ui->action_Company->setText(unit); break;
            default: unit = QStringLiteral("nm"); ui->action_Company->setText(unit); break;
            }
        });

        dialog->exec();
        delete dialog;
    } break;
    default: break;
    }
}

//******************************************************************************************************************

void MainWindow::startCalculation()
{
    if ( !m_daCard->isRunning() && !is_in_calculation ) {
        m_matList.clear();
        changeStatus(false);
        if ( m_daCard->isRunning() ) { m_daCard->wait(); }
        m_daCard->start();

        m_log.write(GlobalFun::getCurrentTime(2) + " - start to increase the voltage");
    }
}

//******************************************************************************************************************

void MainWindow::createThread()
{
    std::thread th([=](){
        qDebug()<<"thread start";
        if ( is_in_check ) {
            // in check
            if ( checkTimes == 1 ) {
                for ( auto &temp : m_algorithmList )
                {
                    temp->configParams.checkThr.resPvThr = 0;//0.095    0--------
                    temp->configParams.checkThr.resRmsThr = 0;//0.018   0
                }
            }
            checkTimes--;
            m_log.write(GlobalFun::getCurrentTime(2) + " - in the retest");
        } else {
            m_log.write(GlobalFun::getCurrentTime(2) + " - start to modify config");
            modifyConfig();
            m_log.write(GlobalFun::getCurrentTime(2) + " - end of modifying config");
        }

        m_maskFillList.clear();
        m_maskEdgeList.clear();
        qDebug()<<"algorithm start";
        for ( int i = 0; i < m_algorithmList.size(); ++i )
        {
            execProcess(i);
        }
        qDebug()<<"algorithm end";
         // check
         if ( GlobalValue::par_psi==5)//GlobalValue::chk_check == 1,默认值0，可能会有问题
         {
             int state = false;
             for ( auto &temp : m_algorithmList )
             {
                 if ( temp->errorType == ERROR_TYPE_PSI_ERROR ) {
                     state = true;
                 }
             }

             if ( state ) {
                 if ( is_in_check ) {
                     if ( checkTimes > 0 ) {
                         m_matList.clear();
                         if ( m_daCard->isRunning() ) { m_daCard->wait(); }
                         m_daCard->start();
                         m_log.write(GlobalFun::getCurrentTime(2) + " - retest times + 1");
                         return;
                     }
                 } else {
                     is_in_check = true;
                     checkTimes = GlobalValue::chk_cts - 1;
                     m_matList.clear();
                     if ( m_daCard->isRunning() ) { m_daCard->wait(); }
                     m_daCard->start();
                     m_log.write(GlobalFun::getCurrentTime(2) + " - start to retest");
                     return;
                 }
             }
         }

        is_in_check = false;
        checkTimes = 0;
        qDebug()<<"thread end";
        emit fillData();
    });
    th.detach();
}

void MainWindow::modifyConfig()
{
    QTime startTime = QTime::currentTime();

    for ( int i = 0; i < m_algorithmList.size(); ++i )
    {
        int type = GlobalFun::getProperty(QString("hole_%1_config.ini").arg(i+1), "property", "type").toInt();
        QSettings hole_type(QString("parameter%1.ini").arg(type), QSettings::IniFormat);

        m_algorithmList.at(i)->configParams.debugGrade = 0;
        m_algorithmList.at(i)->configParams.isASCFile = false;
        if ( type == 2 ){
            m_algorithmList.at(i)->configParams.checkFlag = false;
       } else {
           m_algorithmList.at(i)->configParams.checkFlag = GlobalValue::par_psi==5;
       }
        m_algorithmList.at(i)->configParams.removeErrorFlag = false;
        m_algorithmList.at(i)->configParams.removeResidualFlag = hole_type.value("parameter/remove_residual").toInt() == 1;
        m_algorithmList.at(i)->configParams.isSingleHole = false;
        m_algorithmList.at(i)->configParams.zernikeTerm = GlobalValue::par_ztm;
        m_algorithmList.at(i)->configParams.scaleFactor = GlobalValue::par_i_s_f;
        m_algorithmList.at(i)->configParams.isFillSpikes = GlobalValue::par_use_fs == 1;

        m_algorithmList.at(i)->configParams.holeType = HOLE_TYPE(type);

        m_algorithmList.at(i)->configParams.checkThr.phaseShiftThr = GlobalValue::chk_p_sh_t1;
        m_algorithmList.at(i)->configParams.checkThr.stdPhaseHistThr = GlobalValue::chk_p_s_t1;
        m_algorithmList.at(i)->configParams.checkThr.resPvThr =0;// 0.095  0-------------
        m_algorithmList.at(i)->configParams.checkThr.resRmsThr = 0;//0.018;  0---------------
        m_algorithmList.at(i)->configParams.highPassFilterCoef = GlobalValue::par_hp_fc;//新增加的一项，请注意

        switch ( GlobalValue::par_psi )
        {
        case 0: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKET4A_P; break;
        case 1: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKETB5A_H_P; break;
        case 2: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKET5B_CS_P; break;
        case 3: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKET9A_CS_P; break;
        case 4: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKET9B_CS_P; break;
        case 5: {
            m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_OPT_SEQUENCE; break;//增加的抗震仪相算法
        }
        default: m_algorithmList.at(i)->configParams.psiMethod = PSI_METHOD_BUCKETB5A_H_P; break;
        }

        m_algorithmList.at(i)->configParams.unwrapMethod = UNWRAP_METHOD_BRANCH_CUT;
        m_algorithmList.at(i)->configParams.filterParams.filterType = hole_type.value("parameter/filter").toInt() == 0
                                                                      ? FILTER_TYPE_NONE : FILTER_TYPE_LOW_PASS;
        m_algorithmList.at(i)->configParams.filterParams.filterWindowSize = hole_type.value("parameter/fws").toInt();
        m_algorithmList.at(i)->configParams.filterParams.removeSpikesParams.rsFlag =
                                                                      hole_type.value("parameter/remove_spikes").toInt() == 1;
        m_algorithmList.at(i)->configParams.filterParams.removeSpikesParams.removeSize =
                                                                      hole_type.value("parameter/slopeRemoveSize").toInt();
        m_algorithmList.at(i)->configParams.filterParams.removeSpikesParams.slopeSize = 3;
        m_algorithmList.at(i)->configParams.filterParams.removeSpikesParams.rsThreshCoef =
                                                                      hole_type.value("parameter/slopeThresh").toInt();

        if ( GlobalValue::com_tp == 1 ) {
            if ( m_scene->getGraphicsItemList().at(i)->getType() == BGraphicsItem::ItemType::Circle ) {
                m_algorithmList.at(i)->configParams.zernikeMethod = ZERNIKE_METHOD_CIRCLE;
            } else {
                m_algorithmList.at(i)->configParams.zernikeMethod = ZERNIKE_METHOD_ORTHO;
            }

            m_algorithmList.at(i)->configParams.edgeDetecParams.isUseScale = false;
            m_algorithmList.at(i)->configParams.edgeDetecParams.detecScale = 1;
            m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_MANUAL;
        } else {
            if ( m_scene->getGraphicsItemList().at(i)->getAutoType() == BGraphicsItem::AutoType::Auto_Circle ) {
                m_algorithmList.at(i)->configParams.zernikeMethod = ZERNIKE_METHOD_CIRCLE;
            } else {
                m_algorithmList.at(i)->configParams.zernikeMethod = ZERNIKE_METHOD_ORTHO;
            }

            m_algorithmList.at(i)->configParams.edgeDetecParams.isUseScale = m_scene->getGraphicsItemList().at(i)->getIsUseScale();
            m_algorithmList.at(i)->configParams.edgeDetecParams.detecScale = m_scene->getGraphicsItemList().at(i)->getAutoScale();

            switch ( m_scene->getGraphicsItemList().at(i)->getAutoType() )
            {
            case BGraphicsItem::AutoType::Auto_Circle: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_CIRCLE;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize = cv::Size2f(0, 0);
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = m_scene->getGraphicsItemList().at(i)->getAutoCirRadius()/2;
            } break;
            case BGraphicsItem::AutoType::Auto_Ellipse: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_ELLIPSE;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.width = m_scene->getGraphicsItemList().at(i)->getAutoWidth();
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.height = m_scene->getGraphicsItemList().at(i)->getAutoHeight();
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = 0;
            } break;
            case BGraphicsItem::AutoType::Auto_Pill: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_PILL;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.width = m_scene->getGraphicsItemList().at(i)->getAutoWidth();
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.height = m_scene->getGraphicsItemList().at(i)->getAutoHeight();
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = 0;
            } break;
            case BGraphicsItem::AutoType::Auto_Chamfer: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_RECT_CHAMFER;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.width = m_scene->getGraphicsItemList().at(i)->getAutoWidth();
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.height = m_scene->getGraphicsItemList().at(i)->getAutoHeight();
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = m_scene->getGraphicsItemList().at(i)->getAutoChaRadius();
            } break;
            case BGraphicsItem::AutoType::Auto_RotateRec: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_ROTATED_RECT;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.width = m_scene->getGraphicsItemList().at(i)->getAutoWidth();
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.height = m_scene->getGraphicsItemList().at(i)->getAutoHeight();
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = 0;
            } break;
            case BGraphicsItem::AutoType::Auto_RoundEdgeRec: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_CIRCLE_RECT;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.width = m_scene->getGraphicsItemList().at(i)->getAutoDis1();
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize.height = m_scene->getGraphicsItemList().at(i)->getAutoDis2();
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = m_scene->getGraphicsItemList().at(i)->getAutoCirRadius()/2;
            } break;
            default: {
                m_algorithmList.at(i)->configParams.edgeDetecParams.maskShape = MASK_SHAPE_CIRCLE;
                m_algorithmList.at(i)->configParams.edgeDetecParams.rectSize = cv::Size2f(0, 0);
                m_algorithmList.at(i)->configParams.edgeDetecParams.inputRadius = m_scene->getGraphicsItemList().at(i)->getAutoCirRadius()/2;
            } break;
            }
        }

        m_algorithmList.at(i)->configParams.removeZernikeFlags.positionFlag = GlobalValue::zer_pis == 1;
        m_algorithmList.at(i)->configParams.removeZernikeFlags.tiltFlag = GlobalValue::zer_tilt == 1;
        m_algorithmList.at(i)->configParams.removeZernikeFlags.powerFlag = GlobalValue::zer_pow == 1;
        m_algorithmList.at(i)->configParams.removeZernikeFlags.astFlag = GlobalValue::zer_ast == 1;
        m_algorithmList.at(i)->configParams.removeZernikeFlags.comaFlag = GlobalValue::zer_coma == 1;
        m_algorithmList.at(i)->configParams.removeZernikeFlags.sphericalFlag = GlobalValue::zer_sph == 1;

        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.pvFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.pvxFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.pvyFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.pvresFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.rmsFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.pvRmsResultFlags.rmsresFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikeTiltFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikePowerFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikePowerXFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikePowerYFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikeAstFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikeComaFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.zerSeiResultFlags.zernikeSphericalFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.ttvFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.fringesFlag = true;
        m_algorithmList.at(i)->configParams.calcTotalResultFlags.apertureFlag = true;

        m_algorithmList.at(i)->configParams.calcResultInputParams.px_1mm = GlobalValue::cam_p_cal;
        qDebug()<<"======================="<<hole_type.value("parameter/refractive_index").toDouble();
        m_algorithmList.at(i)->configParams.calcResultInputParams.refractiveIndex = hole_type.value("parameter/refractive_index").toDouble();
        m_algorithmList.at(i)->configParams.calcResultInputParams.testWavelength = GlobalValue::par_t_w;
        m_algorithmList.at(i)->configParams.calcResultInputParams.ISOWavelength = GlobalValue::par_i_w;
        m_algorithmList.at(i)->configParams.calcResultInputParams.disPlayWavelength = GlobalValue::par_t_w;
        m_algorithmList.at(i)->configParams.calcResultInputParams.scaleFactorForHoleType =1;//默认写为1
        m_algorithmList.at(i)->configParams.calcResultInputParams.unitType = (UNIT_TYPE)(GlobalValue::com_unit);//最后两项为新加入的



    }

    QTime stopTime = QTime::currentTime();
    int interval = startTime.msecsTo(stopTime);
    qDebug() << "Modify config finished: " << interval << " ms";
}

void MainWindow::execProcess(int num)
{
    INPUT_DATAS input;
    if ( !GlobalFun::judVecAvailable(m_matList) ) {
        qDebug() << "Image and mask are all empty !";
        int picSize = GlobalValue::par_psi < 3 ? 5 : 9;
        input.oriPhase.clear();
        input.oriPhase.resize(picSize);
        for ( int i = 0; i < picSize; ++i )
        {
            input.oriPhase.push_back(cv::Mat());
        }
        input.mask = cv::Mat();
        m_log.write(GlobalFun::getCurrentTime(2) + " - image and mask are all empty");
    } else {
        input.oriPhase = getMatList(num);
        m_log.write(GlobalFun::getCurrentTime(2) + " - " + QString::number(num) + " get mat list finished");
        input.mask = getMask(num, input.oriPhase.at(0));
        m_log.write(GlobalFun::getCurrentTime(2) + " - " + QString::number(num) + " get mask finished");
    }

    m_log.write(GlobalFun::getCurrentTime(2) + " - " + QString::number(num) + " start process");
    m_algorithmList.at(num)->process(input);
    m_log.write(GlobalFun::getCurrentTime(2) + " - " + QString::number(num) + " end process");

    if ( GlobalValue::com_tp == 1 ) {
        m_maskFillList.insert(num, m_manualMask);
    } else {
        m_maskFillList.insert(num, m_algorithmList.at(num)->retMask);
        m_maskEdgeList.insert(num, m_algorithmList.at(num)->maskEdge);
    }
}

std::vector<cv::Mat> MainWindow::getMatList(int num)
{
    /*
     * 手动和自动传的都是矩形框的大小
     * 自动：场景中的矩形框
     * 手动：boundingRect + 扩充5个像素；如果是多边形，找出4个点再扩充5个像素
     *
     * 注意：图像与矩形框的与操作，如果矩形框超出图像了，与出来的结果是矩形框没有超出图像的那部分
     */
    std::vector<cv::Mat> ret;
    int offset = 0;
    if ( GlobalValue::com_tp == 1 ) { offset = 5; }

    if ( GlobalValue::com_tp == 1 && m_scene->getGraphicsItemList().at(num)->getType() == BGraphicsItem::ItemType::Polygon )
    {
        qreal minX = 10000, maxX = -10000, minY = 10000, maxY = -10000;
        QList<QPointF> list = m_scene->getGraphicsItemList().at(num)->getPointList();
        for (auto &temp : list)
        {
            if ( temp.x() < minX ) { minX = temp.x(); }
            if ( temp.x() > maxX ) { maxX = temp.x(); }
            if ( temp.y() < minY ) { minY = temp.y(); }
            if ( temp.y() > maxY ) { maxY = temp.y(); }
        }

        QPointF minP = m_scene->getGraphicsItemList().at(num)->mapToScene(QPointF(minX, minY));
        QPointF maxP = m_scene->getGraphicsItemList().at(num)->mapToScene(QPointF(maxX, maxY));
        qreal x = minP.x() - offset;
        qreal y = minP.y() - offset;
        qreal w = maxP.x() - minP.x() + offset * 2;
        qreal h = maxP.y() - minP.y() + offset * 2;
        m_rect = cv::Rect(x + m_matList.at(0).cols/2, y + m_matList.at(0).rows/2, w, h) &
                 cv::Rect(0, 0, m_matList.at(0).cols, m_matList.at(0).rows);

        ret = GlobalFun::cvtBGR2GRAY(m_matList, m_rect, true);
        return ret;
    }

    QPolygonF polygon = m_scene->getGraphicsItemList().at(num)->mapToScene(m_scene->getGraphicsItemList().at(num)->boundingRect());
    qreal x = polygon.boundingRect().x() - offset;
    qreal y = polygon.boundingRect().y() - offset;
    qreal w = polygon.boundingRect().width() + offset * 2;
    qreal h = polygon.boundingRect().height() + offset * 2;
    m_rect = cv::Rect(x + m_matList.at(0).cols/2, y + m_matList.at(0).rows/2, w, h) &
             cv::Rect(0, 0, m_matList.at(0).cols, m_matList.at(0).rows);

    ret = GlobalFun::cvtBGR2GRAY(m_matList, m_rect, true);
    return ret;
}

cv::Mat MainWindow::getMask(int num, cv::Mat image)
{
    /*
     * 手动：用上面的矩形框截取实际的图像大小
     * 自动：全0矩阵
     */
    if ( GlobalValue::com_tp == 1 ) {
        QPolygonF polygon = m_scene->getGraphicsItemList().at(num)->mapToScene(m_scene->getGraphicsItemList().at(num)->boundingRect());
        qreal centerX = polygon.boundingRect().center().x();
        qreal centerY = polygon.boundingRect().center().y();

        qreal width = m_scene->getGraphicsItemList().at(num)->getWidth();
        qreal height = m_scene->getGraphicsItemList().at(num)->getHeight();
        qreal radius = m_scene->getGraphicsItemList().at(num)->getRadius();
        qreal rotation = m_scene->getGraphicsItemList().at(num)->getRotation();
        BGraphicsItem::ItemType type = m_scene->getGraphicsItemList().at(num)->getType();

        m_manualMask = cv::Mat(m_matList.at(0).rows, m_matList.at(0).cols, CV_8UC1, cv::Scalar(0));

        Point2i point1(centerX + m_manualMask.cols/2, centerY + m_manualMask.rows/2);
        Point2f point2(centerX + m_manualMask.cols/2, centerY + m_manualMask.rows/2);
        Size2f size(width, height);
        RotatedRect rect(point2, size, rotation);

        switch (type)
        {
        case BGraphicsItem::ItemType::Circle: { calc::DrawCircle(m_manualMask, point1, width/2, 255, -1); } break;
        case BGraphicsItem::ItemType::Ellipse: { calc::DrawEllipse(m_manualMask, rect, 255, -1); } break;
        case BGraphicsItem::ItemType::Concentric_Circle: {
            qreal max = width > radius ? width : radius;
            qreal min = width < radius ? width : radius;
            calc::DrawCircle(m_manualMask, point1, max/2, 255, -1);
            calc::DrawCircle(m_manualMask, point1, min/2, 0, -1);
        } break;
        case BGraphicsItem::ItemType::Rectangle: { calc::DrawRotatedRect(m_manualMask, rect, 255, -1); } break;
        case BGraphicsItem::ItemType::Square: { calc::DrawRotatedRect(m_manualMask, rect, 255, -1); } break;
        case BGraphicsItem::ItemType::Polygon: {
            std::vector<cv::Point> vec;
            QList<QPointF> list = m_scene->getGraphicsItemList().at(num)->getPointList();
            for (auto &temp : list)
            {
                QPointF point = m_scene->getGraphicsItemList().at(num)->mapToScene(temp);
                vec.push_back(cv::Point(point.x() + m_manualMask.cols/2, point.y() + m_manualMask.rows/2));
            }
            calc::DrawPolygon(m_manualMask, vec, 255, -1);
        } break;
        case BGraphicsItem::ItemType::Pill: { calc::DrawPill(m_manualMask, rect, 255, true, -1); } break;
        case BGraphicsItem::ItemType::Chamfer: { calc::DrawRotatedRectChamfer(m_manualMask, rect, radius, 255, -1); } break;
        default: break;
        }

        return m_manualMask(m_rect);
    } else {
        return cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));
    }
}

void MainWindow::drawMask()
{
    if ( GlobalValue::com_tp == 1 )
    {
        if ( m_maskFillList.size() == 0 )
        {
            return;
        }

        cv::Mat fillMask = m_maskFillList.value(0);
        for ( auto &temp : m_maskFillList )
        {
            if ( !temp.empty() ) {
                fillMask += temp;
            }
        }

        QImage image(fillMask.cols, fillMask.rows, QImage::Format_RGBA8888);
        image.fill(QColor(0, 255, 0, 35));
        for ( int i = 0; i != fillMask.rows; ++i )
        {
            for ( int j = 0; j != fillMask.cols; ++j )
            {
                if ( fillMask.at<uchar>(i, j) != 0 )
                    image.setPixelColor(j, i, QColor(255, 255, 255, 0));
            }
        }

        m_scene->setMask(image);
    }
    else
    {
        if ( !GlobalFun::judVecAvailable(m_matList) || m_maskFillList.size() == 0 || m_maskEdgeList.size() == 0 )
        {
            return;
        }

        cv::Mat fillMask = cv::Mat(m_matList.at(0).rows, m_matList.at(0).cols, CV_8UC1, cv::Scalar(0));
        cv::Mat edgeMask = cv::Mat(m_matList.at(0).rows, m_matList.at(0).cols, CV_8UC1, cv::Scalar(0));
        for ( int i = 0; i < m_maskFillList.size(); ++i )
        {
            QPolygonF polygon = m_scene->getGraphicsItemList().at(i)->mapToScene(m_scene->getGraphicsItemList().at(i)->boundingRect());
            int startY = polygon.boundingRect().topLeft().y() + fillMask.rows/2;
            int endY = startY + m_maskFillList.value(i).rows;
            endY = Max(endY, endY - startY);
            startY = Max(startY, 0);

            int startX = polygon.boundingRect().topLeft().x() + fillMask.cols/2;
            int endX = startX + m_maskFillList.value(i).cols;
            endX = Max(endX, endX - startX);
            startX = Max(startX, 0);

            m_maskFillList.value(i).copyTo(fillMask(cv::Range(startY, endY), cv::Range(startX, endX)));
            m_maskEdgeList.value(i).copyTo(edgeMask(cv::Range(startY, endY), cv::Range(startX, endX)));
        }

        QImage image(fillMask.cols, fillMask.rows, QImage::Format_RGBA8888);
        image.fill(QColor(0, 255, 0, 35));
        for ( int i = 0; i != fillMask.rows; ++i )
        {
            for ( int j = 0; j != fillMask.cols; ++j )
            {
                if ( fillMask.at<uchar>(i, j) != 0 )
                    image.setPixelColor(j, i, QColor(255, 255, 255, 0));

                if ( edgeMask.at<uchar>(i, j) != 0 )
                    image.setPixelColor(j, i, QColor(255, 0, 0));
            }
        }

        m_scene->setMask(image);
    }
}

void MainWindow::savePicAndLog()
{
    // save log
    GlobalFun::isDirExist(GlobalValue::com_log_path);

    QString mouth = GlobalValue::com_log_path + "/" + GlobalFun::getCurrentTime(4);
    GlobalFun::isDirExist(mouth);

    QString path = mouth + "/" + GlobalFun::getCurrentTime(3) + ".csv";
    bool isFileExist = GlobalFun::isFileExist(path);

    QFile file(path);
    if ( file.open(QFile::WriteOnly | QFile::Append) ) {
        QTextStream data(&file);
        QStringList linelist;

        if ( !isFileExist ) {
            for (int i = 0; i != ui->data_table->columnCount(); ++i) {
                linelist.append(ui->data_table->horizontalHeaderItem(i)->text());
            }
            data << linelist.join(",") + "\n";
        }

        linelist.clear();
        for (int i = 0; i != ui->data_table->columnCount(); ++i) {
            linelist.append(ui->data_table->item(ui->data_table->rowCount() - 1, i)->text());
        }
        data << linelist.join(",") + "\n";
        file.close();
    }

    //------------------------------------------------------------------------------

    // save pic
    if ( GlobalValue::com_s_p == 1 ||
         ( GlobalValue::com_s_p == 2 && ui->data_table->item(ui->data_table->rowCount() - 1, 3)->text() != "OK" ) )
    {
        GlobalFun::isDirExist(GlobalValue::com_pic_path);

        QString mouth = GlobalValue::com_pic_path + "/" + GlobalFun::getCurrentTime(4);
        GlobalFun::isDirExist(mouth);

        QString today = mouth + "/" + GlobalFun::getCurrentTime(3);
        GlobalFun::isDirExist(today);

        QString product = ui->data_table->item(ui->data_table->rowCount() - 1, 0)->text();
        QString current = today + "/" + (product == "" ? "" : product + "_") + GlobalFun::getCurrentTime(1);
        GlobalFun::isDirExist(current);

        if ( m_matList.size() > 0 && !m_matList.at(0).empty() ) {
            std::thread th([=] {
                for ( size_t i = 0; i < m_matList.size(); ++i )
                {
                    QString fileName = current + QString("/%1.png").arg(i);
                    cv::imwrite(fileName.toStdString(), m_matList.at(i));
                }
            });
            th.detach();
        }

        std::thread th([=](){
            QThread::msleep(500);
            emit screenshot(current);
        });
        th.detach();
    }

    // only screenshot
    if ( GlobalValue::com_s_p == 3 ||
         ( GlobalValue::com_s_p == 4 && ui->data_table->item(ui->data_table->rowCount() - 1, 3)->text() != "OK" ) )
    {
        GlobalFun::isDirExist(GlobalValue::com_pic_path);

        QString mouth = GlobalValue::com_pic_path + "/" + GlobalFun::getCurrentTime(4);
        GlobalFun::isDirExist(mouth);

        QString today = mouth + "/" + GlobalFun::getCurrentTime(3);
        GlobalFun::isDirExist(today);

        QString product = ui->data_table->item(ui->data_table->rowCount() - 1, 0)->text();
        QString current = today + "/" + (product == "" ? "" : product + "_") + GlobalFun::getCurrentTime(1);
        GlobalFun::isDirExist(current);

        std::thread th([=](){
            QThread::msleep(500);
            emit screenshot(current);
        });
        th.detach();
    }
}

void MainWindow::modifyTableHead()
{
    // modify table horizontalHeader
    GlobalFun::isDirExist(GlobalValue::com_log_path);

    QString mouth = GlobalValue::com_log_path + "/" + GlobalFun::getCurrentTime(4);
    GlobalFun::isDirExist(mouth);

    QString path = mouth + "/" + GlobalFun::getCurrentTime(3) + ".csv";
    bool isFileExist = GlobalFun::isFileExist(path);

    QFile file(path);
    if ( file.open(QFile::WriteOnly | QFile::Append) ) {
        QTextStream data(&file);
        QStringList linelist;
        for (int i = 0; i != ui->data_table->columnCount(); ++i) {
            linelist.append(ui->data_table->horizontalHeaderItem(i)->text());
        }
        if ( isFileExist ) {
            data << "\n" + linelist.join(",") + "\n";
        } else {
            data << linelist.join(",") + "\n";
        }
        file.close();
    }
}

void MainWindow::on_fillData()
{
    m_log.write(GlobalFun::getCurrentTime(2) + " - start to fill data");

    // data_table 当前行数
    int dRow = ui->data_table->rowCount();
    ui->data_table->setRowCount(dRow+1);

    int ret_count = 0;
    if (GlobalValue::ret_pv == 1) { ret_count ++; }
    if (GlobalValue::ret_pv_x == 1) { ret_count ++; }
    if (GlobalValue::ret_pv_y == 1) { ret_count ++; }
    if (GlobalValue::ret_pv_res == 1) { ret_count ++; }
    if (GlobalValue::ret_rms == 1) { ret_count ++; }
    if (GlobalValue::ret_rms_res == 1) { ret_count ++; }
    if (GlobalValue::ret_tilt == 1) { ret_count ++; }
    if (GlobalValue::ret_power == 1) { ret_count ++; }
    if (GlobalValue::ret_pwr_x == 1) { ret_count ++; }
    if (GlobalValue::ret_pwr_y == 1) { ret_count ++; }
    if (GlobalValue::ret_ast == 1) { ret_count ++; }
    if (GlobalValue::ret_coma == 1) { ret_count ++; }
    if (GlobalValue::ret_sph == 1) { ret_count ++; }
    if (GlobalValue::ret_ttv == 1) { ret_count ++; }
    if (GlobalValue::ret_fringe == 1) { ret_count ++; }

    //------------------------------------------------------------------------------

    QApplication::processEvents();

    // data_table 中的起始点
    int start = 3;
    ui->data_table->setItem(dRow, 2, new QTableWidgetItem("OK"));
    m_apertureList.clear();
    for ( int i = 0; i < m_algorithmList.size(); ++i )
    {
        // 孔的类别
//        int type = GlobalFun::getProperty(QString("hole_%1_config.ini").arg(i+1), "property", "type").toInt();
//        ui->data_table->setItem(dRow, start++, new QTableWidgetItem(type == 1 ? "CG" : "IR"));

        // 孔径
        qreal aperture = 0;
        if ( GlobalValue::com_tp == 1 &&
             m_scene->getGraphicsItemList().at(i)->getType() == BGraphicsItem::ItemType::Circle )
        {
            aperture = m_scene->getGraphicsItemList().at(i)->getWidth() / GlobalValue::cam_p_cal;
        }
        else if ( GlobalValue::com_tp == 2 &&
                  m_scene->getGraphicsItemList().at(i)->getAutoType() == BGraphicsItem::AutoType::Auto_Circle &&
                  !m_scene->getGraphicsItemList().at(i)->getIsUseScale() )
        {
            aperture = m_scene->getGraphicsItemList().at(i)->getAutoCirRadius() / GlobalValue::cam_p_cal;
        }
        else
        {
            if ( m_algorithmList.at(i)->errorType == ERROR_TYPE_NOT_ERROR ) {
                aperture = floor((m_algorithmList.at(i)->totalResult.aperture + 0.0005) * 1000) / 1000;
            } else {
                aperture = 0;
            }
        }
        m_apertureList.append(aperture);

        // 填充表格
        if ( m_algorithmList.at(i)->errorType == ERROR_TYPE_NOT_ERROR )
        {
            //int rRow = 0;
            int rCol=ret_count,colCnt=0;
            bool status = false;
            qDebug()<<"=========on_filldata start :";
            if (GlobalValue::ret_pv == 1) {
                float pv = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.pv + 0.0005)*1000)/1000;
//                float pv_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(rRow+1, i))->value();
//                float pv_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(rRow+2, i))->value();

                float pv_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pv_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                //ui->result_table->item(rRow, i)->setText(QString::number(pv));
                ui->result_table->item(i, colCnt)->setText(QString::number(pv));

                if ( pv >= pv_min && pv <= pv_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pv)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.pv;
            }
            if (GlobalValue::ret_pv_x == 1) {
                float pv_x = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.pv_x + 0.0005)*1000)/1000;
                float pv_x_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pv_x_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(pv_x));
                if ( pv_x >= pv_x_min && pv_x <= pv_x_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pv_x)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.pv_x;
            }
            if (GlobalValue::ret_pv_y == 1) {
                float pv_y = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.pv_y + 0.0005)*1000)/1000;
                float pv_y_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pv_y_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(pv_y));
                if ( pv_y >= pv_y_min && pv_y <= pv_y_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pv_y)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.pv_y;
            }
            if (GlobalValue::ret_pv_res == 1) {
                float pv_res = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.pv_res + 0.0005)*1000)/1000;
                float pv_res_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pv_res_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(pv_res));
                if ( pv_res >= pv_res_min && pv_res <= pv_res_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pv_res)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.pv_res;
            }
            if (GlobalValue::ret_rms == 1) {
                float rms = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.rms + 0.0005)*1000)/1000;
                float rms_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float rms_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(rms));
                if ( rms >= rms_min && rms <= rms_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(rms)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.rms;
            }
            if (GlobalValue::ret_rms_res == 1) {
                float rms_res = floor((m_algorithmList.at(i)->totalResult.pvRmsResult.rms_res + 0.0005)*1000)/1000;
                float rms_res_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float rms_res_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(rms_res));
                if ( rms_res >= rms_res_min && rms_res <= rms_res_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;

                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(rms_res)));
                qDebug()<<m_algorithmList.at(i)->totalResult.pvRmsResult.rms_res;
            }
            if (GlobalValue::ret_tilt == 1) {
                float tilt = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeTilt + 0.0005)*1000)/1000;
                float tilt_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float tilt_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(tilt));
                if ( tilt >= tilt_min && tilt <= tilt_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;

                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(tilt)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeTilt;
            }
            if (GlobalValue::ret_power == 1) {
                float power = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePower + 0.0005)*1000)/1000;
                float power_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float power_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(power));
                if ( power >= power_min && power <= power_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(power)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePower;
            }
            if (GlobalValue::ret_pwr_x == 1) {
                float pwr_x = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePowerX + 0.0005)*1000)/1000;
                float pwr_x_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pwr_x_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(pwr_x));
                if ( pwr_x >= pwr_x_min && pwr_x <= pwr_x_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pwr_x)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePowerX;
            }
            if (GlobalValue::ret_pwr_y == 1) {
                float pwr_y = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePowerY + 0.0005)*1000)/1000;
                float pwr_y_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float pwr_y_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(pwr_y));
                if ( pwr_y >= pwr_y_min && pwr_y <= pwr_y_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(pwr_y)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikePowerY;
            }
            if (GlobalValue::ret_ast == 1) {
                float ast = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeAst + 0.0005)*1000)/1000;
                float ast_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float ast_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(ast));
                if ( ast >= ast_min && ast <= ast_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;

                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(ast)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeAst;
            }
            if (GlobalValue::ret_coma == 1) {
                float coma = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeComa + 0.0005)*1000)/1000;
                float coma_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float coma_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(coma));
                if ( coma >= coma_min && coma <= coma_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(coma)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeComa;
            }
            if (GlobalValue::ret_sph == 1) {
                float sph = floor((m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeSpherical + 0.0005)*1000)/1000;
                float sph_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float sph_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(sph));
                if ( sph >= sph_min && sph <= sph_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(sph)));
                qDebug()<<m_algorithmList.at(i)->totalResult.zerSeiResult.zernikeSpherical;
            }
            if (GlobalValue::ret_ttv == 1) {
                float ttv = floor((m_algorithmList.at(i)->totalResult.ttv + 0.0005)*1000)/1000;
                float ttv_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float ttv_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(ttv));
                if ( ttv >= ttv_min && ttv <= ttv_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(ttv)));
                qDebug()<<m_algorithmList.at(i)->totalResult.ttv;
            }
            if (GlobalValue::ret_fringe == 1) {
                float fringe = floor((m_algorithmList.at(i)->totalResult.fringes + 0.0005)*1000)/1000;
                float fringe_min = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol))->value();
                float fringe_max = static_cast<QDoubleSpinBox *>(ui->result_table->cellWidget(i, rCol+1))->value();

                ui->result_table->item(i, colCnt)->setText(QString::number(fringe));
                if ( fringe >= fringe_min && fringe <= fringe_max ) {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(0, 255, 0));
                } else {
                    ui->result_table->item(i, colCnt)->setBackgroundColor(QColor(255, 0, 0));
                    status = true;
                }
                rCol += 2;
                ++colCnt;
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(fringe)));
                qDebug()<<m_algorithmList.at(i)->totalResult.fringes;
            }

            ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(aperture)));
            if ( status ) {
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem("NG"));
                ui->quality_table->item(i, 0)->setText(QString("#%1 - NG").arg(i+1));
                ui->quality_table->item(i, 0)->setBackgroundColor(QColor(255, 0, 0));
            } else {
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem("OK"));
                ui->quality_table->item(i, 0)->setText(QString("#%1 - OK").arg(i+1));
                ui->quality_table->item(i, 0)->setBackgroundColor(QColor(0, 255, 0));
            }
            qDebug()<<"fill m_algorithm data end;";
        }
        else
        {
            int rRow = ret_count;
            for ( int j = 0; j < rRow; ++j )
            {
                ui->result_table->item(i, j)->setText("nan");
                if ( m_algorithmList.at(i)->errorType == ERROR_TYPE_CHECK ) {
                    ui->result_table->item(i, j)->setBackgroundColor(Qt::yellow);
                } else {
                    ui->result_table->item(i, j)->setBackgroundColor(Qt::gray);
                }
            }

            for ( int j = 0; j < ret_count ; ++j )
            {
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem("nan"));
            }

            ui->data_table->setItem(dRow, start++, new QTableWidgetItem(QString::number(aperture)));

            if ( m_algorithmList.at(i)->errorType == ERROR_TYPE_PSI_ERROR ) {
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem("E0"));
                ui->quality_table->item(i, 0)->setText(QString("#%1 - E0").arg(i+1));
                ui->quality_table->item(i, 0)->setBackgroundColor(QColor(255, 255, 0));
            } else {
                ui->data_table->setItem(dRow, start++, new QTableWidgetItem("ER"));

                QString errStr = "";
                switch ( m_algorithmList.at(i)->errorType )
                {
                case ERROR_TYPE_NO_ENOUGH_IMAGES: errStr = " no enough images !"; break;
                case ERROR_TYPE_NO_MASK: errStr = " no mask !"; break;
                case ERROR_TYPE_ROI_TOO_SMALL: errStr = " roi too small !" ; break;
                case ERROR_TYPE_PSI_ERROR: errStr = " psi error ! " ; break;
                case ERROR_TYPE_UNWRAP_ERROR: errStr = " unwrap error !" ; break;
                case ERROR_TYPE_ZERNIKE_ERROR: errStr = " zernike error ! " ; break;
                case ERROR_TYPE_CHECK: errStr = " check error !" ; break;
                case ERROR_TYPE_SPIKES_TOO_MUCH: errStr="spokes too much !"; break;
                case ERROR_TYPE_NOT_AUTOMASK: errStr = " no automask ! " ; break;
                case ERROR_TYPE_PIC_SIZE: errStr = " pic size ! " ; break;
                case ERROR_TYPE_NO_RESULT: errStr = " no result ! " ; break;
                default: break;
                }
                ui->quality_table->item(i, 0)->setText(QString("#%1 ER %2").arg(i+1).arg(errStr));
                ui->quality_table->item(i, 0)->setBackgroundColor(Qt::gray);                
            }
        }

        QString total = ui->data_table->item(dRow, 2)->text();
        QString currt = ui->data_table->item(dRow,1-i + (3 + ret_count)*(i+1))->text();
        if ( (total == "OK" && currt != "OK") ||
             (total == "NG" && (currt == "E0" || currt == "ER")) ||
             (total == "E0" && currt == "ER") )
        {
            ui->data_table->item(dRow, 2)->setText(currt);
        }
    }

    //------------------------------------------------------------------------------

    QApplication::processEvents();
    ui->data_table->setItem(dRow, 1, new QTableWidgetItem( GlobalFun::getCurrentTime(6) ));
    ui->data_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    lastTime = QDateTime::currentDateTime();

    QTableWidgetItem *item = new QTableWidgetItem(cell_id);
    if ( cell_id != "" ) { ui->data_table->setColumnWidth(0, 130); }
    ui->data_table->setItem(dRow, 0, item);

    for ( int i = cell_id != "" ? 0 : 1; i < ui->data_table->columnCount(); ++i )
    {
        ui->data_table->item(dRow, i)->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->data_table->item(dRow, i)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    cell_id = "";

    //------------------------------------------------------------------------------

    QApplication::processEvents();
   ui->view_table->clearContents();
   int width=(ui->view_table->width()-Hmarin)/2;
   int height=800/5;
   int pageheight=20;
   for ( int i = 0; i < m_algorithmList.size(); ++i )
   {
       ui->view_table->setRowHeight(i,height);
       QWidget* tmpview=new QWidget();
       tmpview->setFixedSize(width,height);

       QCustomPlot *customPlot = new QCustomPlot(tmpview);
       customPlot->setFixedSize(tmpview->width(), tmpview->height()-pageheight/2);
       QCPColorScale *colorScale = new QCPColorScale(customPlot);
       colorScale->axis()->setTicks(true);
       colorScale->axis()->setRange(0, 1);
       colorScale->setType(QCPAxis::atRight);
       colorScale->setRangeDrag(false);
       colorScale->setRangeZoom(false);
       colorScale->setBarWidth(8);
       QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
       QLabel* page=new QLabel("( "+QString::number(i+1)+" )",tmpview);
       QFont font;
       font.setPixelSize(9);
       page->setFont(font);
       page->setGeometry(0,customPlot->height()-pageheight/2,width,pageheight);
       page->setAlignment(Qt::AlignCenter);

       if ( m_algorithmList.at(i)->errorType == ERROR_TYPE_NOT_ERROR ) {
           cv::Mat mat;
           int type=GlobalValue::par_use_fs == 1 ? 3 : 1;
           switch (type) {
           case 1: mat = m_algorithmList.at(i)->fittingSurface.clone(); break;
           case 2: mat = m_algorithmList.at(i)->zernikeResidual.clone(); break;
           case 3: mat = m_algorithmList.at(i)->fittingSurface_fillSpikes.clone(); break;
           case 5: mat = m_algorithmList.at(i)->fittingSurface_fillSpikes.clone(); break;
           }
           //cv::Mat mat = m_algorithmList.at(i)->fitttingSurface.clone();
           bool landscape = mat.cols >= mat.rows;
           int max = landscape ? mat.cols : mat.rows;
           colorMap->data()->setSize(max, max);
           colorMap->data()->setRange(QCPRange(0, max), QCPRange(0, max));

           double minV = 0, maxV = 0;
           cv::minMaxIdx(mat, &minV, &maxV, nullptr, nullptr, mat == mat);
           colorScale->axis()->setRange(minV, maxV);

           for ( int x = 0; x < mat.cols; ++x )
           {
               for ( int y = 0; y < mat.rows; ++y )
               {
                   float value = mat.at<float>(y, x);
                   if ( isnan(value) ) {
                       colorMap->data()->setAlpha(landscape ? x : x + (mat.rows-mat.cols)/2,
                                                  landscape ? mat.rows - 1 - y + (mat.cols-mat.rows)/2 : mat.rows - 1 - y,
                                                  0);
                   } else {
                       colorMap->data()->setCell(landscape ? x : x + (mat.rows-mat.cols)/2,
                                                 landscape ? mat.rows - 1 - y + (mat.cols-mat.rows)/2 : mat.rows - 1 - y,
                                                 value);
                   }
               }
           }

           if ( landscape ) {
               qreal minY = (mat.cols - mat.rows)/2;
               qreal maxY = (mat.cols + mat.rows)/2;
               for ( int x = 0; x < mat.cols; ++x )
               {
                   for ( int y = 0; y < minY; ++y )
                   {
                       colorMap->data()->setAlpha(x, y, 0);
                   }

                   for ( int y = maxY; y < mat.cols; ++y )
                   {
                       colorMap->data()->setAlpha(x, y, 0);
                   }
               }
           } else {
               qreal minX = (mat.rows - mat.cols)/2;
               qreal maxX = (mat.rows + mat.cols)/2;
               for ( int x = 0; x < mat.rows; ++x )
               {
                   for ( int y = 0; y < minX; ++y )
                   {
                       colorMap->data()->setAlpha(y, x, 0);
                   }

                   for ( int y = maxX; y < mat.rows; ++y )
                   {
                       colorMap->data()->setAlpha(y, x, 0);
                   }
               }
           }
       } else {
           colorMap->data()->setSize(200, 200);
           colorMap->data()->setRange(QCPRange(0, 200), QCPRange(0, 200));
       }
       colorMap->setColorScale(colorScale);
       colorMap->setGradient(cpg);
       colorMap->rescaleDataRange(true);
       customPlot->plotLayout()->addElement(0, 1, colorScale);
       customPlot->rescaleAxes();
       customPlot->replot();
       ui->view_table->setCellWidget(static_cast<double>(i)/2, i%2==0?0:1, tmpview);
   }

    //------------------------------------------------------------------------------

    QApplication::processEvents();

    if ( !m_scene->getLiveStatus() ) {
        m_scene->updateLiveMode(false, m_matList);
    }

    if ( m_daCard->isRunning() ) {
        m_daCard->quit();
    }

    m_log.write(GlobalFun::getCurrentTime(2) + " - start to draw mask");
    drawMask();
    m_log.write(GlobalFun::getCurrentTime(2) + " - end of drawing mask");

    m_log.write(GlobalFun::getCurrentTime(2) + " - start to save pic and log");
    savePicAndLog();
    m_log.write(GlobalFun::getCurrentTime(2) + " - end of saving pic and log");

    //------------------------------------------------------------------------------

    changeStatus(true);

    // 光标在第一列
    ui->data_table->setCurrentItem(item, QItemSelectionModel::Current);
    if ( ui->data_table->item(dRow, 0)->text() == "" ) {
        ui->data_table->editItem(item);
    }
    m_log.write(GlobalFun::getCurrentTime(2) + " - end of filling data\n");
}
