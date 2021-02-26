#ifndef CAMERA_CALIBRATION_DIALOG_H
#define CAMERA_CALIBRATION_DIALOG_H

#include <QDialog>

namespace Ui {
class Camera_calibration_dialog;
}

class Camera_calibration_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Camera_calibration_dialog(QString title, QPixmap pixmap, double pixel_calibration, QWidget *parent = nullptr);
    ~Camera_calibration_dialog();

signals:
    void changeValue(double pixel_calibration);

private slots:
    void updateFrame();

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::Camera_calibration_dialog *ui;
    QPixmap m_pixmap;
};

#endif // CAMERA_CALIBRATION_DIALOG_H
