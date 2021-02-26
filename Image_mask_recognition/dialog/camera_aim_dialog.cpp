#include "camera_aim_dialog.h"
#include "ui_camera_aim_dialog.h"
#include "globalfun.h"
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

Camera_aim_dialog::Camera_aim_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Camera_aim_dialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    is_show = true ;

    connect(&timer, &QTimer::timeout, this, &Camera_aim_dialog::updateFrame);
}

Camera_aim_dialog::~Camera_aim_dialog()
{
    delete ui;
}

void Camera_aim_dialog::showWithClose()
{
    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);
    is_show = true;
    openCamera();
}

void Camera_aim_dialog::showWithoutClose()
{
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    is_show = true ;
    openCamera();
}

void Camera_aim_dialog::takePicWithoutShow()
{
    is_show = false;
    openCamera();
}

void Camera_aim_dialog::openCamera()
{
    QString title = GlobalString::menu_camera + (GlobalValue::lgn_tp == 1 ? "" : " ") + GlobalString::action_aim;
    this->setWindowTitle(title);
    if( is_show ) {
        this->show();
    }
    if ( !m_camera.isOpened() ) {
        m_camera.open(0, cv::CAP_DSHOW);
        if ( !m_camera.isOpened() ) {
            GlobalFun::showMessageBox(3, "No camera connected !");
            this->close();
        } else {
            timer.start(100);
        }
    }
}

void Camera_aim_dialog::closeEvent(QCloseEvent *event)
{
    if ( m_camera.isOpened() ) {
        timer.stop();
        m_camera.release();
    }

    event->accept();
}

void Camera_aim_dialog::updateFrame()
{
    Mat3b mat;
    m_camera >> mat;

    QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.cols * 3, QImage::Format_RGB888);
    QImage ret = image.rgbSwapped();
    GlobalFun::changeColorToRed(ret);

    ui->label->resize(this->width(),this->height());
    ui->label->setPixmap(QPixmap::fromImage(ret.copy().scaled(ui->label->width(),ui->label->height())));
}

cv::Mat Camera_aim_dialog::takeAPic()
{
    Mat mat;
    m_camera >> mat;
    return mat.clone();
}
