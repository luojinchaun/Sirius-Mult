#ifndef UNIT_SETTING_DIALOG_H
#define UNIT_SETTING_DIALOG_H

#include <QDialog>

namespace Ui {
class Unit_setting_dialog;
}

class Unit_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Unit_setting_dialog(QString title, QString name, int type, double value, QWidget *parent = nullptr);
    ~Unit_setting_dialog();

signals:
    void changeValue(int type, double value);

private slots:
    void on_sureBtn_clicked();

private:
    Ui::Unit_setting_dialog *ui;
    double m_value;
};

#endif // UNIT_SETTING_DIALOG_H
