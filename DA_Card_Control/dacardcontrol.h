#ifndef DACARDCONTROL_H
#define DACARDCONTROL_H

#include <QThread>

class DACardControl : public QThread
{
    Q_OBJECT
public:
    explicit DACardControl();
    ~DACardControl();

    virtual void run() override;
    void closeDevice();

    void test(double value, QRect rect);

signals:
    void error(int type);           // 错误警报
    void takeAPic();                // 拍照
    void acquisition_complete();    // 采集数据完成
    void test_complete(QRect &rect);// 测试完成

private:
    int devIndex;                   // 设备号
    unsigned char chan;             // 通道号, 0~3 对应 Ao_0~Ao_3

    float current_voltage;          // 当前电压值
    int num;                        // 计数
    bool state;                     // 是否检测电压大于1V

    float step_voltage;             // 步进电压
    float interval_voltage;         // 间隔电压
    float voltage_calibration;      // 电压标定值
    int step_rest;                  // 步进睡眠
    int interval_rest;              // 间隔睡眠
    int add_several_times;          // 增加电压的次数
    int count;                      // 移相次数，5A对应4步移相，9A对应8步移相
};

#endif // DACARDCONTROL_H
