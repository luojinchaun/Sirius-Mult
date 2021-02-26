#ifndef BBASLERCAMERCONTROL_H
#define BBASLERCAMERCONTROL_H

#include "../camera.h"
#include "PylonIncludes.h"

using namespace std;
using namespace Pylon;
using namespace GenApi;

class BBaslerCamerControl : public Camera
{
    Q_OBJECT
public:
    enum BaslerCameraProperty {
        ExposureTime,               // 相机曝光时间
        Gain                        // 相机增益
    };

    explicit BBaslerCamerControl(Camera::CameraType type = Camera::CameraType::Basler);

    virtual void initCamera() override;
    virtual void destroyCamera() override;

    virtual void openCamera() override;
    virtual void closeCamera() override;

    virtual void startWork() override;
    virtual void stopWork() override;

    virtual cv::Mat takeAPic() override;

    QString getCameraProperty(BBaslerCamerControl::BaslerCameraProperty type);                  // 获取相机参数
    void setCameraProperty(BBaslerCamerControl::BaslerCameraProperty type, double value = 0.0); // 设置相机参数

public slots:
    void updateFrame();

private:
    CInstantCamera m_baslerCamera;  // 实例化相机对象
    INodeMap *m_nodeMap;            // 相机属性节点
    bool isVersion2;                // 2.0及以上版本
};

#endif // BBASLERCAMERCONTROL_H
