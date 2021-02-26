#include "dacardcontrol.h"
#include "USB3000.h"
#include "math.h"
#include "globalfun.h"

DACardControl::DACardControl()
{
    devIndex = 0;
    chan = 0;
    current_voltage = 0;
    num = 0;
    state = false;

    step_voltage = 0;
    interval_voltage = 0;
    step_rest = 0;
    interval_rest = 0;
    add_several_times = 0;
    count = 0;
}

DACardControl::~DACardControl()
{
    closeDevice();
}

void DACardControl::closeDevice()
{
    // Close Device
    USB3CloseDevice(devIndex);
}

void DACardControl::run()
{
    // Open Device
    int ret = USB3OpenDevice(devIndex);
    if ( ret != 0 ) {
        emit error(ret);
        return;
    }

    current_voltage = 0;
    num = 0;
    state = true;
    count = GlobalValue::par_psi < 3 ? 4 : 8;
    int psi = count + 1;

    step_voltage = GlobalValue::sft_s_vol;
    interval_voltage = GlobalValue::sft_i_vol;
    voltage_calibration = GlobalValue::par_psi < 3 ? (GlobalValue::sft_v_cal + 1) : (GlobalValue::sft_v_cal*2 + 1);
    step_rest = GlobalValue::sft_s_rest;
    interval_rest = GlobalValue::sft_i_rest;
    add_several_times = GlobalValue::sft_a_s_t;

    while ( current_voltage < voltage_calibration )
    {
        current_voltage += step_voltage;
        num++;

        if ( state && current_voltage >= 1.0 ) {
            current_voltage = 1.0;
            num = 0;
            state = false;

            ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
            if ( ret != 0 ) {
                emit error(ret);
                return;
            }

            GlobalFun::bsleep(interval_rest);
            emit takeAPic();
            GlobalFun::bsleep(GlobalValue::cam_exp + GlobalValue::cam_exp_dly);
            qDebug() << QStringLiteral("当前电压为: ") + QString::number(current_voltage) + QStringLiteral("v，采集图像！");
            continue;
        }

        if ( !state && num == add_several_times ) {
            current_voltage = 1 + (psi-count)*interval_voltage;
            num = 0;
            count--;

            ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
            if ( ret != 0 ) {
                emit error(ret);
                return;
            }

            GlobalFun::bsleep(interval_rest);
            emit takeAPic();
            GlobalFun::bsleep(GlobalValue::cam_exp + GlobalValue::cam_exp_dly);
            qDebug() << QStringLiteral("当前电压为: ") + QString::number(current_voltage) + QStringLiteral("v，采集图像！");
            continue;
        }

        ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
        if ( ret != 0 ) {
            emit error(ret);
            return;
        }
        GlobalFun::bsleep(step_rest);
    }

    emit acquisition_complete();

    while ( current_voltage > 0 ) {
        current_voltage -= step_voltage * 1.5;
        if ( current_voltage < 0 ) { current_voltage = 0; }

        ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
        if ( ret != 0 ) {
            emit error(ret);
            return;
        }
        GlobalFun::bsleep(step_rest);
    }

    closeDevice();
}

void DACardControl::test(double value, QRect rect)
{
    std::thread th([=]()mutable {
        // Open Device
        int ret = USB3OpenDevice(devIndex);
        if ( ret != 0 ) {
            emit error(ret);
            return;
        }

        current_voltage = 0;
        num = 0;
        state = true;
        count = GlobalValue::par_psi < 3 ? 4 : 8;
        int psi = count + 1;

        step_voltage = GlobalValue::sft_s_vol;
        interval_voltage = value / 4;
        voltage_calibration = GlobalValue::par_psi < 3 ? (value + 1) : (value*2 + 1);
        step_rest = GlobalValue::sft_s_rest;
        interval_rest = GlobalValue::sft_i_rest;
        add_several_times = ceil(interval_voltage / step_voltage);;

        qDebug() << endl << "//------------------------------------------------------------------------------" << endl;
        while ( current_voltage < voltage_calibration )
        {
            current_voltage += step_voltage;
            qDebug() << "current_voltage: " << current_voltage;
            num++;

            if ( state && current_voltage >= 1.0 ) {
                current_voltage = 1.0;
                num = 0;
                state = false;

                ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
                if ( ret != 0 ) {
                    emit error(ret);
                    return;
                }

                GlobalFun::bsleep(interval_rest);
                emit takeAPic();
                GlobalFun::bsleep(GlobalValue::cam_exp + GlobalValue::cam_exp_dly);
                qDebug() << QStringLiteral("当前电压为: ") + QString::number(current_voltage) + QStringLiteral("v，采集图像！");
                continue;
            }

            if ( !state && num == add_several_times ) {
                current_voltage = 1 + (psi-count)*interval_voltage;
                num = 0;
                count--;

                ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
                if ( ret != 0 ) {
                    emit error(ret);
                    return;
                }

                GlobalFun::bsleep(interval_rest);
                emit takeAPic();
                GlobalFun::bsleep(GlobalValue::cam_exp + GlobalValue::cam_exp_dly);
                qDebug() << QStringLiteral("当前电压为: ") + QString::number(current_voltage) + QStringLiteral("v，采集图像！");
                continue;
            }

            ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
            if ( ret != 0 ) {
                emit error(ret);
                return;
            }
            GlobalFun::bsleep(step_rest);
        }

        qDebug() << endl << "//------------------------------------------------------------------------------" << endl;
        while ( current_voltage > 0 ) {
            current_voltage -= step_voltage * 1.5;
            if ( current_voltage < 0 ) { current_voltage = 0; }
            qDebug() << "current_voltage: " << current_voltage;

            ret = SetUSB3AoImmediately(devIndex, chan, current_voltage);
            if ( ret != 0 ) {
                emit error(ret);
                return;
            }
            GlobalFun::bsleep(step_rest);
        }

        emit test_complete(rect);

        closeDevice();
    });
    th.detach();
}
