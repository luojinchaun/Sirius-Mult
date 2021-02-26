#ifndef CAMERA_SETTING_DIALOG_H
#define CAMERA_SETTING_DIALOG_H

#include <QDialog>

namespace Ui {
class Camera_setting_dialog;
}

class Camera_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Camera_setting_dialog(QString title, QString name, int interval, double exposure, QWidget *parent = nullptr);
    ~Camera_setting_dialog();

signals:
    void changeValue(int interval, double exposure);

private slots:
    void on_sureBtn_clicked();

private:
    Ui::Camera_setting_dialog *ui;
};

#endif // CAMERA_SETTING_DIALOG_H
