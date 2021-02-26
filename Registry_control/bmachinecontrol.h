#ifndef BMACHINECONTROL_H
#define BMACHINECONTROL_H

#include <QString>
#include <QAxObject>
#include <QDebug>
#include <QUuid>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDataStream>
#include <QList>
#include <Windows.h>

class BMachineControl
{
public:
    BMachineControl();

    QString getWMIHWInfo(int type);

    QString getCPUID1();

    QString getCPUID2();

    QString getHDLogicalID();

    QString getMac();

    QString getCPUManID();

    QString getInfo();

    // 根据设备信息、日期、可使用月份生成key
    QString getKey(QString machineinfo, QString ddMMyyyy, int months);

    // 注册key
    bool activeKey(QString key);

    // 拷贝至文件
    void copyToFile(QSettings* reg, bool magic = false);

    // 无注册表信息则初始化并返回true, 否则直接返回false
    bool initializeReg();

    // 比较是否与文件一致
    bool judgeFile();

    // 判断日期是否被故意修改
    bool judgeDate();

    // 判断key，返回剩余天数，过期则为负
    int judgeKey();

    // 刷新 DT1
    void refreshDT1();

private:
    const QString kReg = "HKEY_CURRENT_USER\\Software\\Lenovo\\App";
    const QString kKey = "K";
    const QString kDateTime0 = "DT0";
    const QString kDateTime1 = "DT1";
    const QString kDateTime2 = "DT2";
    const QString kMonths = "M";

    const int kForever = 1000000;
    const QList<int> kValidity = {1, 3, 6, 12, kForever};

    const QString kFile = "C:/ProgramData/Lenovo/app.dat";
};

#endif // BMACHINECONTROL_H
