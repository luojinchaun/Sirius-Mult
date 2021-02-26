#ifndef SHIFTER_CALIBRATION_DIALOG_H
#define SHIFTER_CALIBRATION_DIALOG_H

#include <QDialog>
#include <QtCharts/QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include "opencv2/opencv.hpp"

namespace Ui {
class Shifter_calibration_dialog;
}

class Shifter_calibration_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Shifter_calibration_dialog(QString title, QPixmap pixmap, double voltage_calibration, QWidget *parent = nullptr);
    ~Shifter_calibration_dialog();

    void fillData(cv::Mat mat, float std, int shift, std::vector<cv::Mat> list);

protected:
    virtual void closeEvent(QCloseEvent *) override;

signals:
    void test(double value, QRect rect);
    void changeValue(double voltage_calibration);
    void changeStatus();

public slots:
    void updateFrame();
    void changePic(bool state);

private slots:
    void on_okBtn_clicked();

    void on_cancleBtn_clicked();

private:
    Ui::Shifter_calibration_dialog *ui;

    QChart *m_chart;
    QSplineSeries *m_series;
    cv::Mat m_mat;

    QPixmap m_pixmap;
    std::vector<cv::Mat> m_matList;
    bool is_in_calculation;
};

#endif // SHIFTER_CALIBRATION_DIALOG_H
