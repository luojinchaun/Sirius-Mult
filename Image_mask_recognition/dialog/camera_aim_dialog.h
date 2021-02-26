#ifndef CAMERA_AIM_DIALOG_H
#define CAMERA_AIM_DIALOG_H

#include <QDialog>
#include <QTimer>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
using namespace cv;

namespace Ui {
class Camera_aim_dialog;
}

class Camera_aim_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Camera_aim_dialog(QWidget *parent = nullptr);
    ~Camera_aim_dialog();

    void showWithClose();
    void showWithoutClose();
    void openCamera();
    cv::Mat takeAPic();
    virtual void closeEvent(QCloseEvent *event) override;
    void takePicWithoutShow();

public slots:
    void updateFrame();

private:
    Ui::Camera_aim_dialog *ui;

    VideoCapture m_camera;
    QTimer timer;
    bool is_show;
};

#endif // CAMERA_AIM_DIALOG_H
