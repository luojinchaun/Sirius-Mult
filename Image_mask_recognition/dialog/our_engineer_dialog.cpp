#include "our_engineer_dialog.h"
#include "ui_our_engineer_dialog.h"
#include "globalfun.h"

Our_engineer_dialog::Our_engineer_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Our_engineer_dialog)
{
    ui->setupUi(this);

    setWindowTitle(GlobalValue::lgn_tp == 1 ? QStringLiteral("高级设置") : "Advanced Setting");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    initView();
}

Our_engineer_dialog::~Our_engineer_dialog()
{
    delete ui;
}

void Our_engineer_dialog::initView()
{   
    QSettings hole_type(QString("parameter1.ini"), QSettings::IniFormat);
    ui->fws1_box->setCurrentText(hole_type.value("parameter/fws").toString());

    ui->srs1_box->setCurrentIndex(hole_type.value("parameter/slopeRemoveSize").toInt());
    ui->st1_box->setCurrentIndex(hole_type.value("parameter/slopeThresh").toInt());
    //ui->r_i1_box->setValue(hole_type.value("parameter/refractive_index").toDouble());
    ui->zTerm_box->setCurrentText(QString::number(GlobalValue::par_ztm));

    ui->rt_box->setCurrentText(QString::number(GlobalValue::chk_cts));
    ui->gra_def_size_value->setValue(GlobalValue::gra_def_size);
    ui->p_sh_t1_box->setValue(GlobalValue::chk_p_sh_t1);
    ui->p_s_t1_box->setValue(GlobalValue::chk_p_s_t1);
    ui->doubleSpinBox->setValue(GlobalValue::par_hp_fc);


    ui->use_fs_yes->setChecked(GlobalValue::par_use_fs == 1);
    ui->use_fs_no->setChecked(GlobalValue::par_use_fs == 0);
    connect(ui->use_fs_yes, &QCheckBox::toggled, [&](bool state){ ui->use_fs_no->setChecked(!state); });
    connect(ui->use_fs_no, &QCheckBox::toggled, [&](bool state){ ui->use_fs_yes->setChecked(!state); });

}

void Our_engineer_dialog::on_OK_BTN_clicked()
{
    QSettings hole_type(QString("parameter1.ini"), QSettings::IniFormat);
    hole_type.setValue("parameter/fws", ui->fws1_box->currentText().toInt());
    hole_type.setValue("parameter/slopeRemoveSize", ui->srs1_box->currentIndex());
    hole_type.setValue("parameter/slopeThresh", ui->st1_box->currentIndex());
    //hole_type.setValue("parameter/refractive_index", ui->r_i1_box->value());
    GlobalValue::par_ztm = ui->zTerm_box->currentText().toInt();
    GlobalValue::chk_cts = ui->rt_box->currentText().toInt();
    GlobalValue::gra_def_size=ui->gra_def_size_value->value();
    GlobalValue::chk_p_sh_t1 = ui->p_sh_t1_box->value();
    GlobalValue::chk_p_s_t1 = ui->p_s_t1_box->value();
    GlobalValue::par_hp_fc = ui->doubleSpinBox->value();
    GlobalValue::par_use_fs = ui->use_fs_yes->isChecked() ? 1 : 0;

    GlobalValue::gra_c_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_e_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_e_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_cc_rad_1 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_cc_rad_2 = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*5;
    GlobalValue::gra_r_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_r_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_s_len = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_p_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size *2;
    GlobalValue::gra_p_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /2;
    GlobalValue::gra_ch_wid = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /3*4;
    GlobalValue::gra_ch_hei = GlobalValue::cam_p_cal * GlobalValue::gra_def_size;
    GlobalValue::gra_ch_rad = GlobalValue::cam_p_cal * GlobalValue::gra_def_size /6;
    this->close();
}
