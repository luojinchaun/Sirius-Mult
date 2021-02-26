#include "bvrcameracontrol.h"
#include "globalfun.h"

BVRCameraControl::BVRCameraControl(Camera::CameraType type) : Camera(type)
{
    m_timer.setInterval(GlobalValue::cam_itl);
    connect(&m_timer, &QTimer::timeout, this, &BVRCameraControl::updateFrame);
}

void BVRCameraControl::initCamera()
{
    // pass
}

void BVRCameraControl::destroyCamera()
{
    stopWork();
    closeCamera();
}

void BVRCameraControl::openCamera()
{
    m_camera.open(0, cv::CAP_DSHOW);
}

void BVRCameraControl::closeCamera()
{
    m_camera.release();
}

void BVRCameraControl::startWork()
{
    m_timer.start();
}

void BVRCameraControl::stopWork()
{
    m_timer.stop();
}

cv::Mat BVRCameraControl::takeAPic()
{
    cv::Mat3b mat;
    m_camera >> mat;

    QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.cols * 3, QImage::Format_RGB888);
    QImage ret = image.rgbSwapped();

    cv::Mat retMat = GlobalFun::convertQImageToMat(ret);
    return retMat.clone();
}

void BVRCameraControl::updateFrame()
{
    cv::Mat3b mat;
    m_camera >> mat;

    QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.cols * 3, QImage::Format_RGB888);
    QImage ret = image.rgbSwapped();

    emit updateImage(ret.convertToFormat(QImage::Format_RGB888));
}
