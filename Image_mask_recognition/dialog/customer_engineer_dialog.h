#ifndef CUSTOMER_ENGINEER_DIALOG_H
#define CUSTOMER_ENGINEER_DIALOG_H

#include <QDialog>

namespace Ui {
class Customer_engineer_dialog;
}

class Customer_engineer_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Customer_engineer_dialog(QWidget *parent = nullptr);
    ~Customer_engineer_dialog();

    void initView();

private slots:
    void on_pp_btn_clicked();

    void on_lp_btn_clicked();

    void on_ok_btn_clicked();
private:signals:
    void unit_value_changed(int index);
private:
    Ui::Customer_engineer_dialog *ui;
};

#endif // CUSTOMER_ENGINEER_DIALOG_H
