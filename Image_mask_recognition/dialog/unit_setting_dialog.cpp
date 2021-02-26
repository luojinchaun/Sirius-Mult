#include "unit_setting_dialog.h"
#include "ui_unit_setting_dialog.h"
#include "globalfun.h"

Unit_setting_dialog::Unit_setting_dialog(QString title, QString name, int type, double value, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Unit_setting_dialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());
    connect(ui->comboBox, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), [&](int index){
        ui->doubleSpinBox->setPrefix(QString("1%1 = ").arg(ui->comboBox->currentText()));
        if ( index == 0 || index == 1 ) {
            ui->doubleSpinBox->setEnabled(true);
            ui->doubleSpinBox->setDecimals(2);
            ui->doubleSpinBox->setValue( index == 0 ? m_value : m_value/2 );
        } else {
            ui->doubleSpinBox->setEnabled(false);
            ui->doubleSpinBox->setDecimals(0);
            ui->doubleSpinBox->setValue( index == 2 ? 1 : ( index == 3 ? 1000 : 1000000 ) );
        }
    });

    ui->sureBtn->setText(name);
    ui->comboBox->setCurrentIndex(type);
    switch (type)
    {
    case 0: ui->doubleSpinBox->setValue(value); break;
    case 1: ui->doubleSpinBox->setValue(value/2); break;
    case 2: ui->doubleSpinBox->setValue(1); break;
    case 3: ui->doubleSpinBox->setValue(1000); break;
    case 4: ui->doubleSpinBox->setValue(1000000); break;
    default: break;
    }
    m_value = value;
}

Unit_setting_dialog::~Unit_setting_dialog()
{
    delete ui;
}

void Unit_setting_dialog::on_sureBtn_clicked()
{
    emit changeValue(ui->comboBox->currentIndex(),
                     ui->comboBox->currentIndex() == 0 ? ui->doubleSpinBox->value():
                     ui->comboBox->currentIndex() == 1 ? ui->doubleSpinBox->value() * 2 : m_value);
    this->close();
}
