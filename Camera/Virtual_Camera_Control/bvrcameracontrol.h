#ifndef BVRCAMERACONTROL_H
#define BVRCAMERACONTROL_H

#include "../camera.h"

class BVRCameraControl : public Camera
{
    Q_OBJECT
public:
    explicit BVRCameraControl(Camera::CameraType type = Camera::CameraType::Virtual);

    virtual void initCamera() override;
    virtual void destroyCamera() override;

    virtual void openCamera() override;
    virtual void closeCamera() override;

    virtual void startWork() override;
    virtual void stopWork() override;

    virtual cv::Mat takeAPic() override;

public slots:
    void updateFrame();

private:
    cv::VideoCapture m_camera;
};

#endif // BVRCAMERACONTROL_H
