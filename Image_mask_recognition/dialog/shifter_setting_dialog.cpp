#include "shifter_setting_dialog.h"
#include "ui_shifter_setting_dialog.h"

Shifter_setting_dialog::Shifter_setting_dialog(QString title, QString name, double s_vol, double i_vol,
                                               int s_rest, int i_rest, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Shifter_setting_dialog)
{
    ui->setupUi(this);

    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    ui->sureBtn->setText(name);
    ui->doubleSpinBox_1->setValue(s_vol);
    ui->doubleSpinBox_2->setValue(i_vol);
    ui->spinBox_1->setValue(s_rest);
    ui->spinBox_2->setValue(i_rest);
}

Shifter_setting_dialog::~Shifter_setting_dialog()
{
    delete ui;
}

void Shifter_setting_dialog::on_sureBtn_clicked()
{
    emit changeValue(ui->doubleSpinBox_1->value(), ui->doubleSpinBox_2->value(), ui->spinBox_1->value(), ui->spinBox_2->value());
    this->close();
}
