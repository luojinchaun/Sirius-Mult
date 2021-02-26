#include "shifter_calibration_dialog.h"
#include "ui_shifter_calibration_dialog.h"
#include <QPainter>
#include <QPen>
#include <QCloseEvent>

Shifter_calibration_dialog::Shifter_calibration_dialog(QString title, QPixmap pixmap, double voltage_calibration, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Shifter_calibration_dialog)
{
    ui->setupUi(this);

    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    ui->slider->setMinimum(0);
    ui->slider->setMaximum(pixmap.height());
    ui->slider->setValue(pixmap.height()/2);

    m_pixmap = pixmap;
    updateFrame();

    m_chart = new QChart();
    m_chart->setMinimumHeight(150);
    m_chart->legend()->hide();
    ui->chartView->setChart(m_chart);
    m_series = new QSplineSeries();
    m_chart->addSeries(m_series);
    m_chart->createDefaultAxes();

    ui->doubleSpinBox->setValue(voltage_calibration);
    ui->index->setText("0");
    ui->label_std->setText("std: #");
    ui->label_shift->setText("shift: #");

    m_matList.clear();
    is_in_calculation = false;

    connect(ui->slider, &QSlider::valueChanged, this, &Shifter_calibration_dialog::updateFrame);
    connect(ui->leftBtn, &QToolButton::clicked, [&]{ changePic(true); });
    connect(ui->rightBtn, &QToolButton::clicked, [&]{ changePic(false); });
    connect(ui->testBtn, &QToolButton::clicked, [&]{
        QRect ret(( m_pixmap.width() - ui->slider->value() )/2, ( m_pixmap.height() - ui->slider->value() )/2,
                  ui->slider->value(), ui->slider->value());
        emit test(ui->doubleSpinBox->value(), ret);
        is_in_calculation = true;
        ui->testBtn->setEnabled(false);
    });
    connect(this, &Shifter_calibration_dialog::changeStatus, this, [&]{
        m_chart->removeSeries(m_series);
        m_series->clear();
        for ( int i = 0; i != m_mat.rows; ++i)
        {
            m_series->append(i, m_mat.at<float>(i, 0));
        }
        m_chart->addSeries(m_series);
        m_chart->createDefaultAxes();

        is_in_calculation = false;
        ui->testBtn->setEnabled(true);
    }, Qt::QueuedConnection);
}

Shifter_calibration_dialog::~Shifter_calibration_dialog()
{
    delete ui;
}

void Shifter_calibration_dialog::updateFrame()
{
    ui->label_value->setText(QString::number(ui->slider->value()) + " px");

    QImage image = m_pixmap.toImage();
    QPainter painter(&image);
    QPen pen(QColor(255, 0, 0, 255));
    pen.setWidth(3);
    painter.setPen(pen);
    int x = ( m_pixmap.width() - ui->slider->value() )/2;
    int y = ( m_pixmap.height() - ui->slider->value() )/2;
    painter.drawRect(x, y, ui->slider->value(), ui->slider->value());
    ui->frame->setPixmap(QPixmap::fromImage(image).scaled(ui->frame->size(), Qt::KeepAspectRatio));
}

void Shifter_calibration_dialog::changePic(bool state)
{
    if ( m_matList.size() == 0 ) {
        return;
    }

    if ( state ) {
        // left
        int index = ui->index->text().toInt();
        if ( index > 0 ) {
            index--;
            ui->index->setText(QString::number(index));
            cv::Mat mat = m_matList.at(index);
            cvtColor(mat, mat, cv::COLOR_BGR2RGB);
            QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, QImage::Format_RGB888);
            m_pixmap = QPixmap::fromImage(image.rgbSwapped());
            updateFrame();
        }
    } else {
        // right
        int index = ui->index->text().toInt();
        if ( index < (int)m_matList.size() - 1 ) {
            index++;
            ui->index->setText(QString::number(index));
            cv::Mat mat = m_matList.at(index);
            cvtColor(mat, mat, cv::COLOR_BGR2RGB);
            QImage image((const unsigned char *)(mat.data), mat.cols, mat.rows, QImage::Format_RGB888);
            m_pixmap = QPixmap::fromImage(image.rgbSwapped());
            updateFrame();
        }
    }
}

void Shifter_calibration_dialog::fillData(cv::Mat mat, float std, int shift, std::vector<cv::Mat> list)
{
    float ret = floor((std + 0.00005)*10000)/10000;
    ui->label_std->setText(QString("std: %1").arg(ret));
    ui->label_shift->setText(QString("shift: %1").arg(shift));

    m_matList.clear();
    for ( auto &temp : list )
    {
        m_matList.push_back(temp);
    }
    cv::Mat new_mat = m_matList.at(0);
    cvtColor(new_mat, new_mat, cv::COLOR_BGR2RGB);
    QImage image((const unsigned char*)(new_mat.data), new_mat.cols, new_mat.rows, QImage::Format_RGB888);
    m_pixmap = QPixmap::fromImage(image.rgbSwapped());
    ui->index->setText("0");
    updateFrame();

    m_mat = mat.clone();
    emit changeStatus();
}

void Shifter_calibration_dialog::closeEvent(QCloseEvent *event)
{
    if ( is_in_calculation ) {
        event->ignore();
    } else {
        event->accept();
    }
}

void Shifter_calibration_dialog::on_okBtn_clicked()
{
    if ( !is_in_calculation ) {
        changeValue(ui->doubleSpinBox->value());
        this->close();
    }
}

void Shifter_calibration_dialog::on_cancleBtn_clicked()
{
    if ( !is_in_calculation ) {
        this->close();
    }
}
