#include "customer_engineer_dialog.h"
#include "ui_customer_engineer_dialog.h"
#include "globalfun.h"
#include <QMessageBox>

Customer_engineer_dialog::Customer_engineer_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Customer_engineer_dialog)
{
    ui->setupUi(this);

    setWindowTitle(GlobalValue::lgn_tp == 1 ? QStringLiteral("设置") : "Setting");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    initView();
}

Customer_engineer_dialog::~Customer_engineer_dialog()
{
    delete ui;
}

void Customer_engineer_dialog::initView()
{

    connect(ui->r_s_yes_2, &QCheckBox::toggled, [&](bool state){ ui->r_s_no_2->setChecked(!state); });
    connect(ui->r_s_no_2, &QCheckBox::toggled, [&](bool state){ ui->r_s_yes_2->setChecked(!state); });
    connect(ui->r_r_yes_4, &QCheckBox::toggled, [&](bool state){ ui->r_r_no_4->setChecked(!state); });
    connect(ui->r_r_no_4, &QCheckBox::toggled, [&](bool state){ ui->r_r_yes_4->setChecked(!state); });

    QSettings hole_type1(QString("parameter1.ini"), QSettings::IniFormat);
    ui->a_t_itl_box_2->setValue(GlobalValue::com_auto_itl);
    switch ( GlobalValue::par_psi ) {
    case 1: ui->psi_box_2->setCurrentIndex(0); break;
    case 3: ui->psi_box_2->setCurrentIndex(1); break;
    case 5: ui->psi_box_2->setCurrentIndex(2); break;
    default: ui->psi_box_2->setCurrentIndex(0); break;
    }
    ui->unwrap_box_2->setCurrentIndex(GlobalValue::par_unw);
    ui->filter_box_2->setCurrentIndex(hole_type1.value("parameter/filter").toInt());
    ui->r_s_yes_2->setChecked(hole_type1.value("parameter/remove_spikes").toInt() == 1);
    ui->r_s_no_2->setChecked(hole_type1.value("parameter/remove_spikes").toInt() == 0);//r_r_yes_4
    ui->r_r_yes_4->setChecked(hole_type1.value("parameter/remove_residual").toInt() == 1);
    ui->r_r_no_4->setChecked(hole_type1.value("parameter/remove_residual").toInt() == 0);


    ui->i_s_f_box->setValue(GlobalValue::par_i_s_f);
    ui->t_w_box->setValue(GlobalValue::par_t_w);
    ui->i_w_box->setValue(GlobalValue::par_i_w);
    ui->r_i_box->setValue(hole_type1.value("parameter/refractive_index").toDouble());
    ui->comboBox->setCurrentIndex(GlobalValue::com_s_p);
    ui->pp_edit->setText(GlobalValue::com_pic_path);
    ui->lp_edit->setText(GlobalValue::com_log_path);
    ui->unit_box->setCurrentIndex(GlobalValue::com_unit);
}

void Customer_engineer_dialog::on_pp_btn_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Picture path", ui->pp_edit->text());
    if ( path != "" ) {
        ui->pp_edit->setText(path);
    }
}

void Customer_engineer_dialog::on_lp_btn_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Log path", ui->lp_edit->text());
    if ( path != "" ) {
        ui->lp_edit->setText(path);
    }
}

void Customer_engineer_dialog::on_ok_btn_clicked()
{
    GlobalValue::com_auto_itl=ui->a_t_itl_box_2->value();
    switch ( ui->psi_box_2->currentIndex() ) {
    case 0: GlobalValue::par_psi = 1; break;
    case 1: GlobalValue::par_psi = 3; break;
    case 2: GlobalValue::par_psi = 5; break;
    default: GlobalValue::par_psi = 1; break;
    }
    GlobalValue::par_unw = ui->unwrap_box_2->currentIndex();
    QSettings hole_type1(QString("parameter1.ini"), QSettings::IniFormat);
    hole_type1.setValue("parameter/filter", ui->filter_box_2->currentIndex());
    hole_type1.setValue("parameter/remove_spikes", ui->r_s_yes_2->isChecked() ? 1 : 0);
    hole_type1.setValue("parameter/remove_residual", ui->r_r_yes_4->isChecked() ? 1 : 0);
    GlobalValue::par_i_s_f = ui->i_s_f_box->value();
    GlobalValue::par_t_w = ui->t_w_box->value();
    GlobalValue::par_i_w = ui->i_w_box->value();
    hole_type1.setValue("parameter/refractive_index", ui->r_i_box->value());
    GlobalValue::com_s_p = ui->comboBox->currentIndex();

    GlobalValue::com_unit=ui->unit_box->currentIndex();
    emit unit_value_changed(ui->unit_box->currentIndex());

    GlobalValue::com_pic_path = ui->pp_edit->text();
    GlobalValue::com_log_path = ui->lp_edit->text();
    this->close();
}
