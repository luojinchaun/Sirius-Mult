#include "bmvcameracontrol.h"
#include "globalfun.h"

BMVCameraControl::BMVCameraControl(Camera::CameraType type) : Camera(type)
{
    m_timer.setInterval(GlobalValue::cam_itl);
    connect(&m_timer, &QTimer::timeout, this, &BMVCameraControl::updateFrame);
}

void BMVCameraControl::initCamera()
{
    m_devHandle = nullptr;
    m_data = nullptr;
    m_size = 0;
}

void BMVCameraControl::destroyCamera()
{
    try {
        stopWork();
        closeCamera();

        if ( m_devHandle ) {
            MV_CC_DestroyHandle(m_devHandle);   // 销毁设备句柄
            m_devHandle = nullptr;
            delete[] m_data;
        }
    } catch (...) {
        qDebug() << "destroyCamera error !";
    }
}

void BMVCameraControl::openCamera()
{
    try {
        MV_CC_DEVICE_INFO_LIST pstDevList;
        int ret = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &pstDevList);

        if ( ret == MV_OK ) {
            if ( pstDevList.nDeviceNum == 0 ) {
                qDebug() << "No device found !";
            } else {
                MV_CC_DEVICE_INFO m_stDevInfo;
                memcpy(&m_stDevInfo, pstDevList.pDeviceInfo[0], sizeof(MV_CC_DEVICE_INFO));

                ret = MV_CC_CreateHandle(&m_devHandle, &m_stDevInfo);           // 创建设备句柄
                if ( ret != MV_OK ) { qDebug() << "CreateHandle failed !"; }
                ret = MV_CC_OpenDevice(m_devHandle);                            // 打开设备
                if ( ret != MV_OK ) { qDebug() << "OpenDevice failed !"; }

                // 探测网络最佳包大小（只对GigE相机有效）
                if ( m_stDevInfo.nTLayerType == MV_GIGE_DEVICE )
                {
                    int nPacketSize = MV_CC_GetOptimalPacketSize(m_devHandle);
                    if (nPacketSize > 0)
                    {
                        ret = MV_CC_SetIntValue(m_devHandle, "GevSCPSPacketSize", nPacketSize);
                        if ( ret != MV_OK ) { qDebug() << "SetIntValue failed !"; }
                    }
                }

                ret = MV_CC_SetPixelFormat(m_devHandle, PixelType_Gvsp_Mono8);  // 设置像素格式
                if ( ret != MV_OK ) { qDebug() << "SetPixelFormat failed !"; }
            }
        }
    } catch (...) {
        qDebug() << "openCamera error !";
    }
}

void BMVCameraControl::closeCamera()
{
    try {
        MV_CC_CloseDevice(m_devHandle); // 关闭相机
    } catch (...) {
        qDebug() << "closeCamera error !";
    }
}

void BMVCameraControl::startWork()
{
    // 判断相机是否处于连接状态
    if ( !MV_CC_IsDeviceConnected(m_devHandle) ) {
        return;
    }

    try {
        // 获取一帧数据的大小
        MVCC_INTVALUE stIntvalue;
        int ret = MV_CC_GetIntValue(m_devHandle, "PayloadSize", &stIntvalue);
        if ( ret != MV_OK ) { qDebug() << "GetIntValue failed !"; }

        m_size = stIntvalue.nCurValue; // 一帧数据大小
        m_data = (unsigned char*)malloc(m_size);
        memset(&m_frame_info, 0, sizeof(MV_FRAME_OUT_INFO_EX));

        ret = MV_CC_StartGrabbing(m_devHandle); // 开始采集图像
        if ( ret != MV_OK ) { qDebug() << "StartGrabbing failed !"; }
        m_timer.start();
    } catch (...) {
        qDebug() << "startWork error !";
    }
}

void BMVCameraControl::stopWork()
{
    try {
        if ( MV_CC_IsDeviceConnected(m_devHandle) ) {
            MV_CC_StopGrabbing(m_devHandle);    // 停止取流
            m_timer.stop();
        }
    } catch (...) {
        qDebug() << "stopWork error !";
    }
}

cv::Mat BMVCameraControl::takeAPic()
{
    // 判断相机是否处于连接状态
    if ( !MV_CC_IsDeviceConnected(m_devHandle) ) {
        return cv::Mat();
    }

    QMutexLocker locker(&m_mutex);

    try {
        int nRet = MV_CC_GetOneFrameTimeout(m_devHandle, m_data, m_size, &m_frame_info, 1000);
        if ( nRet == MV_OK ) {
            QImage image(static_cast<uchar*>(m_data),
                         m_frame_info.nWidth,
                         m_frame_info.nHeight,
                         QImage::Format_Grayscale8);

            QImage retImage = image.convertToFormat(QImage::Format_RGB888);
            cv::Mat mat = GlobalFun::convertQImageToMat(retImage);
            return mat.clone();
        } else {
            return cv::Mat();
        }
    } catch (...) {
        qDebug() << "takeAPic error !";
        return cv::Mat();
    }
}

QString BMVCameraControl::getCameraProperty(BMVCameraControl::BMVCameraProperty type)
{
    // 判断相机是否处于连接状态
    if ( !MV_CC_IsDeviceConnected(m_devHandle) ) {
        return "";
    }

    QString ret = "";
    MVCC_INTVALUE int_value;
    MVCC_FLOATVALUE float_value;
    MVCC_ENUMVALUE enum_value;

    try {
        switch (type) {
        case Width: {
            MV_CC_GetWidth(m_devHandle, &int_value);
            ret = QString::number(int_value.nCurValue);
        } break;
        case Height: {
            MV_CC_GetHeight(m_devHandle, &int_value);
            ret = QString::number(int_value.nCurValue);
        } break;
        case ExposureTime: {
            MV_CC_GetExposureTime(m_devHandle, &float_value);
            ret = QString::number(float_value.fCurValue);
        } break;
        case Brightness: {
            MV_CC_GetBrightness(m_devHandle, &int_value);
            ret = QString::number(int_value.nCurValue);
        } break;
        case FrameRate: {
            MV_CC_GetFrameRate(m_devHandle, &float_value);
            ret = QString::number(float_value.fCurValue);
        } break;
        case Gain: {
            MV_CC_GetGain(m_devHandle, &float_value);
            ret = QString::number(float_value.fCurValue);
        } break;
        case PixelFormat: {
            MV_CC_GetPixelFormat(m_devHandle, &enum_value);
            ret = QString::number(enum_value.nCurValue);
        } break;
        case TriggerMode: {
            MV_CC_GetTriggerMode(m_devHandle, &enum_value);
            ret = QString::number(enum_value.nCurValue);
        } break;
        default: ret = ""; break;
        }
    } catch (...) {
        qDebug() << "getCameraProperty error !";
    }

    return ret;
}

void BMVCameraControl::setCameraProperty(BMVCameraControl::BMVCameraProperty type, double value)
{
    // 判断相机是否处于连接状态
    if ( !MV_CC_IsDeviceConnected(m_devHandle) ) {
        return;
    }

    try {
        switch (type) {
        case Width: {
            MV_CC_SetWidth(m_devHandle, value);
        } break;
        case Height: {
            MV_CC_SetHeight(m_devHandle, value);
        } break;
        case ExposureTime: {
            MV_CC_SetExposureTime(m_devHandle, value);
        } break;
        case Brightness: {
            MV_CC_SetBrightness(m_devHandle, value);
        } break;
        case FrameRate: {
            MV_CC_SetFrameRate(m_devHandle, value);
        } break;
        case Gain: {
            MV_CC_SetGain(m_devHandle, value);
        } break;
        case PixelFormat: {
            MV_CC_SetPixelFormat(m_devHandle, value);
        } break;
        case TriggerMode: {
            MV_CC_SetTriggerMode(m_devHandle, value);
        } break;
        default: break;
        }
    } catch (...) {
        qDebug() << "setCameraProperty error !";
    }
}

void BMVCameraControl::updateFrame()
{
    QMutexLocker locker(&m_mutex);

    try {
        int nRet = MV_CC_GetOneFrameTimeout(m_devHandle, m_data, m_size, &m_frame_info, 1000);
        if ( nRet == MV_OK ) {
            QImage image(static_cast<uchar*>(m_data),
                         m_frame_info.nWidth,
                         m_frame_info.nHeight,
                         QImage::Format_Grayscale8);

            emit updateImage(image.convertToFormat(QImage::Format_RGB888));
        }
    } catch (...) {
        qDebug() << "updateFrame error !";
    }
}

