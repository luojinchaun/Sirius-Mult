#include "camera_setting_dialog.h"
#include "ui_camera_setting_dialog.h"

Camera_setting_dialog::Camera_setting_dialog(QString title, QString name, int interval, double exposure, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Camera_setting_dialog)
{
    ui->setupUi(this);

    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    ui->spinBox_1->setValue(interval);
    ui->spinBox_2->setValue(exposure * 1000);
    ui->sureBtn->setText(name);
}

Camera_setting_dialog::~Camera_setting_dialog()
{
    delete ui;
}

void Camera_setting_dialog::on_sureBtn_clicked()
{
    emit changeValue(ui->spinBox_1->value(), ui->spinBox_2->value());
    this->close();
}
