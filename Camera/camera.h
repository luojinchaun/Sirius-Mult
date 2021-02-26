#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QMutex>

#include <opencv2/opencv.hpp>

class Camera : public QObject
{
    Q_OBJECT

public:
    enum CameraType {
        Basler = 1,                     // 巴斯勒相机
        IC_Imaging,                     // 映美精相机
        MV,                             // 海康威视相机
        Virtual                         // 虚拟相机
    };

    explicit Camera(CameraType type = Basler) : m_type(type) {}

    virtual void initCamera() = 0;      // 初始化相机
    virtual void destroyCamera() = 0;   // 销毁相机
    virtual void openCamera() = 0;      // 打开相机
    virtual void closeCamera() = 0;     // 关闭相机
    virtual void startWork() = 0;       // 开始工作
    virtual void stopWork() = 0;        // 停止工作
    virtual cv::Mat takeAPic() = 0;     // 获取当前图像

    void start() { m_timer.start(); }
    void stop() { m_timer.stop(); }
    void setInterval(int time) { m_timer.setInterval(time); }
    CameraType getCameraType() { return m_type; }

signals:
    void updateImage(QImage image);

protected:
    CameraType m_type;
    QMutex m_mutex;
    QTimer m_timer;
};

#endif // CAMERA_H
