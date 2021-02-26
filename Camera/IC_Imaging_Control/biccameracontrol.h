#ifndef BICCAMERACONTROL_H
#define BICCAMERACONTROL_H

#include "../camera.h"
#include <tisudshl.h>
using namespace _DSHOWLIB_NAMESPACE;

#define NUM_BUFFERS 10

class BICCameraControl : public Camera
{
    Q_OBJECT
public:
    explicit BICCameraControl(Camera::CameraType type = Camera::CameraType::IC_Imaging);

    virtual void initCamera() override;
    virtual void destroyCamera() override;

    virtual void openCamera() override;
    virtual void closeCamera() override;

    virtual void startWork() override;
    virtual void stopWork() override;

    virtual cv::Mat takeAPic() override;

    void saveConfigToFile(QString path);
    void loadConfileFromFile(QString path);
    void showDialog();
    double getExposure();

public slots:
    void updateFrame();

protected:
    DShowLib::Grabber m_Grabber;
    DShowLib::FrameHandlerSink::tFHSPtr m_Sink;
};

#endif // BICCAMERACONTROL_H
