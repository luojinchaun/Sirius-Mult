#ifndef BMVCAMERCONTROL_H
#define BMVCAMERCONTROL_H

#include "../camera.h"
#include "MvCameraControl.h"

class BMVCameraControl : public Camera
{
    Q_OBJECT
public:
    enum BMVCameraProperty {
        Width,                      // 图片宽度
        Height,                     // 图片高度
        ExposureTime,               // 曝光时间
        Brightness,                 // 亮度
        FrameRate,                  // 帧率
        Gain,                       // 增益
        PixelFormat,                // 像素格式
        TriggerMode                 // 触发模式
    };

    explicit BMVCameraControl(Camera::CameraType type = Camera::CameraType::MV);

    virtual void initCamera() override;
    virtual void destroyCamera() override;

    virtual void openCamera() override;
    virtual void closeCamera() override;

    virtual void startWork() override;
    virtual void stopWork() override;

    virtual cv::Mat takeAPic() override;

    QString getCameraProperty(BMVCameraControl::BMVCameraProperty type);                  // 获取相机参数
    void setCameraProperty(BMVCameraControl::BMVCameraProperty type, double value = 0.0); // 设置相机参数

public slots:
    void updateFrame();

private:
    void *m_devHandle;
    unsigned char *m_data;
    int m_size;
    MV_FRAME_OUT_INFO_EX m_frame_info;
};

#endif // BMVCAMERCONTROL_H
