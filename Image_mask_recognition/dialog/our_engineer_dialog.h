#ifndef OUR_ENGINEER_DIALOG_H
#define OUR_ENGINEER_DIALOG_H

#include <QDialog>

namespace Ui {
class Our_engineer_dialog;
}

class Our_engineer_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Our_engineer_dialog(QWidget *parent = nullptr);
    ~Our_engineer_dialog();

    void initView();

private slots:
    void on_OK_BTN_clicked();
signals:
    void updatePlot(bool state);

private:
    Ui::Our_engineer_dialog *ui;
};

#endif // OUR_ENGINEER_DIALOG_H
