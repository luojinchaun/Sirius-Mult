#include "camera_calibration_dialog.h"
#include "ui_camera_calibration_dialog.h"
#include <QPainter>
#include <QPen>

Camera_calibration_dialog::Camera_calibration_dialog(QString title, QPixmap pixmap, double pixel_calibration, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Camera_calibration_dialog)
{
    ui->setupUi(this);

    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMaximumSize(this->width(), this->height());
    setMinimumSize(this->width(), this->height());

    m_pixmap = pixmap;
    ui->frame->setPixmap(pixmap.scaled(ui->frame->size(), Qt::KeepAspectRatio));

    ui->left_slider->setMinimum(0);
    ui->left_slider->setMaximum(pixmap.width());
    ui->left_slider->setValue(0);

    ui->right_slider->setMinimum(0);
    ui->right_slider->setMaximum(pixmap.width());
    ui->right_slider->setValue(pixmap.width());

    updateFrame();

    ui->frame_width->setText(QString::number(pixmap.width()));
    ui->doubleSpinBox->setValue(pixmap.width() / pixel_calibration);
    ui->pixel->setText(QString("%1 px").arg(pixel_calibration));

    connect(ui->doubleSpinBox, static_cast<void (QDoubleSpinBox::*) (double)>(&QDoubleSpinBox::valueChanged), [&](double value){
        double pixel = ui->frame_width->text().toDouble() / value;
        ui->pixel->setText(QString("%1 px").arg(pixel));
    });

    connect(ui->left_slider, &QSlider::valueChanged, this, &Camera_calibration_dialog::updateFrame);
    connect(ui->right_slider, &QSlider::valueChanged, this, &Camera_calibration_dialog::updateFrame);
}

Camera_calibration_dialog::~Camera_calibration_dialog()
{
    delete ui;
}

void Camera_calibration_dialog::updateFrame()
{
    int height = m_pixmap.height();
    int left = ui->left_slider->value();
    int right = ui->right_slider->value();
    QImage image = m_pixmap.toImage();
    QPainter painter(&image);
    QPen pen(QColor(255, 0, 0, 255));
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawLine(QLine(left, 0, left, height));
    pen.setColor(QColor(0, 255, 0, 255));
    painter.setPen(pen);
    painter.drawLine(QLine(right, 0, right, height));
    ui->frame->setPixmap(QPixmap::fromImage(image).scaled(ui->frame->size(), Qt::KeepAspectRatio));

    int width = abs(ui->right_slider->value() - ui->left_slider->value());
    ui->frame_width->setText(QString::number(width));
    ui->pixel->setText(QString("%1 px").arg( width / ui->doubleSpinBox->value() ));
}

void Camera_calibration_dialog::on_okBtn_clicked()
{
    changeValue(ui->pixel->text().left(ui->pixel->text().size()-3).toDouble());
    this->close();
}

void Camera_calibration_dialog::on_cancelBtn_clicked()
{
    this->close();
}
