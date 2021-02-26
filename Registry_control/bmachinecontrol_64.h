#ifndef BMACHINECONTROL_64_H
#define BMACHINECONTROL_64_H

#include <QAxObject>
#include <QUuid>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSysInfo>
#include <QSettings>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileInfoList>
#include <QDir>
#include <QLibrary>
#include <QTimer>
#include <QHostInfo>
#include <Windows.h>

class BMachineControl_64
{
public:
    BMachineControl_64();

    QString getWMIHWInfo(int type);     // 获取各类序列号
    QString getMachineName();           // 获取计算机名称
    QString getIP();                    // 获取IP地址
    QString getMac();                   // 获取计算机MAC地址
    QString getCPU();                   // 获取计算机CPU信息
    QString getInfo();                  // 获取设备信息


    QString getKey(QString machineinfo, QString ddMMyyyy, int months);  // 根据设备信息、日期、可使用月份生成key
    bool activeKey(QString key);        // 注册key
    bool initializeReg();               // 无注册表信息则初始化并返回true, 否则直接返回false
    bool judgeDate();                   // 判断日期是否被故意修改
    int judgeKey();                     // 判断key，返回剩余天数，过期则为负
    void refreshDT1();                  // 刷新 DT1

private:
    const QString kReg = "HKEY_CURRENT_USER\\Software\\Lenovo\\Sirius_Pro";
    const QString kKey = "K";
    const QString kDateTime0 = "DT0";
    const QString kDateTime1 = "DT1";
    const QString kDateTime2 = "DT2";
    const QString kMonths = "M";

    const int kForever = 1000000;
    const QList<int> kValidity = {1, 3, 6, 12, kForever};
};

#endif // BMACHINECONTROL_64_H
