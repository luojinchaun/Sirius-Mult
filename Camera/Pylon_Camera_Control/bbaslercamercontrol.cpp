#include "bbaslercamercontrol.h"
#include "globalfun.h"

BBaslerCamerControl::BBaslerCamerControl(Camera::CameraType type) : Camera(type)
{
    m_timer.setInterval(GlobalValue::cam_itl);
    connect(&m_timer, &QTimer::timeout, this, &BBaslerCamerControl::updateFrame);
}

void BBaslerCamerControl::initCamera()
{
    try {
        Pylon::PylonInitialize();   // 调用其他 pylon 函数之前必须调用 PylonInitialize 完成初始化
    } catch (GenICam::GenericException &e) {
        qDebug() << "initCamera error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::destroyCamera()
{
    try {
        stopWork();
        closeCamera();

        Pylon::PylonTerminate();    // 释放 pylon 运行时系统分配的资源
    } catch (GenICam::GenericException &e) {
        qDebug() << "destroyCamera error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::openCamera()
{
    try {
        m_baslerCamera.Attach(CTlFactory::GetInstance().CreateFirstDevice());   // 实例化找到的第一个相机
        m_baslerCamera.Open();                                                  // 打开相机
        m_nodeMap = &m_baslerCamera.GetNodeMap();                               // 获取相机属性节点

        if ( m_baslerCamera.GetSfncVersion() >= Sfnc_2_0_0 ) {
            isVersion2 = true;
        } else {
            isVersion2 = false;
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "openCamera error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::closeCamera()
{
    try {
        if ( m_baslerCamera.IsOpen() ) {
            m_baslerCamera.DetachDevice();  // 分离连接的 pylon 设备
            m_baslerCamera.Close();         // 关闭连接的 pylon 设备
            m_nodeMap = nullptr;            // 相机属性节点置空
        }

    } catch (GenICam::GenericException &e) {
        qDebug() << "closeCamera error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::startWork()
{
    // Check if camera is open.
    if ( !m_baslerCamera.IsOpen() ) {
        return;
    }

    try {
        m_baslerCamera.StartGrabbing(GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByUser);  // 开始抓取图像
        m_timer.start();
    } catch (GenICam::GenericException &e) {
        qDebug() << "startWork error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::stopWork()
{
    try {
        if ( m_baslerCamera.IsGrabbing() ) {
            m_baslerCamera.StopGrabbing();  // 停止抓取图像
            m_timer.stop();
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "stopWork error: " + QString::fromLocal8Bit(e.what());
    }
}

cv::Mat BBaslerCamerControl::takeAPic()
{
    // Check if camera is open.
    if ( !m_baslerCamera.IsOpen() ) {
        return cv::Mat();
    }

    QMutexLocker locker(&m_mutex);

    try {
        CGrabResultPtr ptrGrabResult;
        m_baslerCamera.RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);

        if ( ptrGrabResult->GrabSucceeded() ) {
            CPylonImage pylogimage;
            CImageFormatConverter formatconverter;
            formatconverter.OutputPixelFormat = PixelType_RGB8packed;
            formatconverter.Convert(pylogimage, ptrGrabResult);

            cv::Mat mat = cv::Mat(int( ptrGrabResult->GetHeight() ),
                                  int( ptrGrabResult->GetWidth() ),
                                  CV_8UC3,
                                  static_cast< uchar* >( pylogimage.GetBuffer() ));

            return mat.clone();
        } else {
            return cv::Mat();
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "takeAPic error: " + QString::fromLocal8Bit(e.what());
        return cv::Mat();
    }
}

QString BBaslerCamerControl::getCameraProperty(BBaslerCamerControl::BaslerCameraProperty type)
{
    // Check if camera is open.
    if ( !m_baslerCamera.IsOpen() ) {
        return "";
    }

    QString ret = "";

    try {
        switch (type) {
        case ExposureTime: {
            const CFloatPtr exposureTimeAbs = m_nodeMap->GetNode( isVersion2 ? "ExposureTime" : "ExposureTimeAbs" );
            ret = QString::number(exposureTimeAbs->GetValue());
        } break;
        case Gain: {
            const CIntegerPtr gainRaw = m_nodeMap->GetNode( isVersion2 ? "Gain" : "GainRaw" );
            ret = QString::number(gainRaw->GetValue());
        } break;
        default: ret = ""; break;
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "getCameraProperty error: " + QString::fromLocal8Bit(e.what());
    }

    return ret;
}

void BBaslerCamerControl::setCameraProperty(BBaslerCamerControl::BaslerCameraProperty type, double value)
{
    // Check if camera is open.
    if ( !m_baslerCamera.IsOpen() ) {
        return;
    }

    try {
        switch (type) {
        case ExposureTime: {
            const CFloatPtr exposureTimeAbs = m_nodeMap->GetNode( isVersion2 ? "ExposureTime" : "ExposureTimeAbs" );
            exposureTimeAbs->SetValue(value);
        } break;
        case Gain: {
            const CIntegerPtr gainRaw = m_nodeMap->GetNode( isVersion2 ? "Gain" : "GainRaw" );
            gainRaw->SetValue(value);
        } break;
        default: break;
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "setCameraProperty error: " + QString::fromLocal8Bit(e.what());
    }
}

void BBaslerCamerControl::updateFrame()
{
    QMutexLocker locker(&m_mutex);

    try {
        CGrabResultPtr ptrGrabResult;
        m_baslerCamera.RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);

        if ( ptrGrabResult->GrabSucceeded() ) {
            CPylonImage pylogimage;
            CImageFormatConverter formatconverter;
            formatconverter.OutputPixelFormat = PixelType_RGB8packed;
            formatconverter.Convert(pylogimage, ptrGrabResult);

            cv::Mat mat = cv::Mat(int( ptrGrabResult->GetHeight() ),
                                  int( ptrGrabResult->GetWidth() ),
                                  CV_8UC3,
                                  static_cast< uchar* >( pylogimage.GetBuffer() ));

            QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.cols * 3, QImage::Format_RGB888);
            emit updateImage(image.rgbSwapped());
        }
    } catch (GenICam::GenericException &e) {
        qDebug() << "updateFrame error: " + QString::fromLocal8Bit(e.what());
    }
}
