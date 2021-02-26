#ifndef SHIFTER_SETTING_DIALOG_H
#define SHIFTER_SETTING_DIALOG_H

#include <QDialog>

namespace Ui {
class Shifter_setting_dialog;
}

class Shifter_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Shifter_setting_dialog(QString title, QString name, double s_vol, double i_vol,
                                    int s_rest, int i_rest, QWidget *parent = nullptr);
    ~Shifter_setting_dialog();

signals:
    void changeValue(double s_vol, double i_vol, int s_rest, int i_rest);

private slots:
    void on_sureBtn_clicked();

private:
    Ui::Shifter_setting_dialog *ui;
};

#endif // SHIFTER_SETTING_DIALOG_H
