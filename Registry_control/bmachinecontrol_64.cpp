#include "bmachinecontrol_64.h"
#include <QMessageBox>

BMachineControl_64::BMachineControl_64()
{

}

QString BMachineControl_64::getWMIHWInfo(int type)
{
    /*
     * 注意：qt调用wmi时，对查询语句要求很严格，所以like之类的句子务必精确才能有结果出来
     *
     * 1. 当前原生网卡地址
     *    SELECT MACAddress ...
     *
     * 2. 硬盘序列号
     *    SELECT PNPDeviceID ...
     *
     * 3. 获取主板序列号
     *    SELECT SerialNumber ...
     *
     * 4. 处理器ID
     *    SELECT ProcessorId ...
     *
     * 5. BIOS序列号
     *    SELECT SerialNumber ...
     *
     * 6. 主板型号
     *    SELECT Product ...
     */

    QString hwInfo;
    QStringList sqlCmd;
    sqlCmd.clear();
    sqlCmd << "SELECT MACAddress FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))";
    sqlCmd << "SELECT PNPDeviceID FROM Win32_DiskDrive WHERE( PNPDeviceID IS NOT NULL) AND (MediaType LIKE 'Fixed%')";
    sqlCmd << "SELECT SerialNumber FROM Win32_BaseBoard WHERE (SerialNumber IS NOT NULL)";
    sqlCmd << "SELECT ProcessorId FROM Win32_Processor WHERE (ProcessorId IS NOT NULL)";
    sqlCmd << "SELECT SerialNumber FROM Win32_BIOS WHERE (SerialNumber IS NOT NULL)";
    sqlCmd << "SELECT Product FROM Win32_BaseBoard WHERE (Product IS NOT NULL)";

    QStringList columnName;
    columnName.clear();
    columnName << "MACAddress";
    columnName << "PNPDeviceID";
    columnName << "SerialNumber";
    columnName << "ProcessorId";
    columnName << "SerialNumber";
    columnName << "Product";

    QAxObject *objIWbemLocator = new QAxObject("WbemScripting.SWbemLocator");
    QAxObject *objWMIService = objIWbemLocator->querySubObject("ConnectServer(QString&,QString&)",QString("."), QString("root\\cimv2"));
    QString query;
    if ( type < sqlCmd.size() ) {
        query = sqlCmd.at(type);
    }

    QAxObject *objInterList = objWMIService->querySubObject("ExecQuery(QString&))", query);
    QAxObject *enum1 = objInterList->querySubObject("_NewEnum");

    IEnumVARIANT *enumInterface = 0;
    enum1->queryInterface(IID_IEnumVARIANT, (void**)&enumInterface);
    enumInterface->Reset();

    for ( int i = 0; i < objInterList->dynamicCall("Count").toInt(); ++i )
    {
        VARIANT *theItem = (VARIANT*)malloc(sizeof(VARIANT));
        if ( enumInterface->Next(1, theItem, NULL) != S_FALSE )
        {
            QAxObject *item = new QAxObject((IUnknown *)theItem->punkVal);
            if (item) {
                if ( type<columnName.size() ) {
                    QByteArray datagram(columnName.at(type).toLatin1());
                    const char* tempConstChar = datagram.data();
                    hwInfo=item->dynamicCall(tempConstChar).toString();
                }
            }
        }
    }

    return hwInfo;
}

QString BMachineControl_64::getMachineName()
{
    QString machineName = QHostInfo::localHostName();
    return machineName;
}

QString BMachineControl_64::getIP()
{
    QString ip = "";

    QList<QNetworkInterface> interFaceList = QNetworkInterface::allInterfaces();

    for( int i = 0; i < interFaceList.size(); ++i )
    {
        QNetworkInterface m_interface = interFaceList.at(i);

        if ( m_interface.flags().testFlag(QNetworkInterface::IsRunning) )
        {
            QList<QNetworkAddressEntry> entryList = m_interface.addressEntries();

            foreach( QNetworkAddressEntry entry, entryList )
            {
                if( QAbstractSocket::IPv4Protocol == entry.ip().protocol() &&
                        entry.ip() != QHostAddress::LocalHost &&
                        entry.ip().toString().startsWith("192.168.") )
                {
                    ip = entry.ip().toString();
                    break;
                }
            }
        }
    }

    return ip;
}

QString BMachineControl_64::getMac()
{
    QString strMac;

    QList<QNetworkInterface> netList = QNetworkInterface::allInterfaces();

    foreach( QNetworkInterface item, netList )
    {
        if( (QNetworkInterface::IsUp & item.flags()) && (QNetworkInterface::IsRunning & item.flags()) )
        {
            if( strMac.isEmpty() || strMac < item.hardwareAddress() )
            {
                strMac = item.hardwareAddress();
            }
        }
    }

    return strMac;
}

QString BMachineControl_64::getCPU()
{
    QSettings *CPU = new QSettings("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                                   QSettings::NativeFormat);

    QString m_cpuDescribe = CPU->value("ProcessorNameString").toString();

    delete CPU;
    return m_cpuDescribe;
}

QString BMachineControl_64::getInfo()
{
    QString info;
    for ( int i = 1; i != 6; ++i )
    {
        info.append(getWMIHWInfo(i));
    }
    info.append(getMachineName());
//    info.append(getIP());
//    info.append(getMac());
    info.append(getCPU());

    return info;
}

//******************************************************************************************************************

QString BMachineControl_64::getKey(QString machineinfo, QString ddMMyyyy, int months)
{
    QString originalStr120;
    if ( machineinfo.isEmpty() ) {
        originalStr120 = QString("machineinfo") + ddMMyyyy + QString::number(months);
    } else {
        originalStr120 = machineinfo + ddMMyyyy + QString::number(months);
    }
    QCryptographicHash sha1(QCryptographicHash::Sha1);

    QByteArray datagram(originalStr120.toLatin1());

    for ( int i = 0; i != datagram.size(); ++i )
    {
        datagram[i] = datagram[i] ^ 'q' ^ 'y';
    }

    const char* tempConstChar = datagram.data();
    sha1.addData(tempConstChar);

    QString activeCode = sha1.result().toHex();

    return activeCode;
}

bool BMachineControl_64::activeKey(QString key)
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dtc = QDateTime::currentDateTime();
    QDateTime dtct = dtc;

    int daydelay = 2;
    for ( int d = 0; d != daydelay; ++d )
    {
        for ( int i = 0; i <= kValidity.size(); ++i )
        {
            int validity = kValidity.at(i);
            QString k0 = getKey(getInfo(), dtct.date().toString("ddMMyyyy"), validity);
            if (k0 == key) {
                reg->setValue(kKey, key);
                reg->setValue(kDateTime0, dtct);
                reg->setValue(kDateTime1, dtc);
                reg->setValue(kDateTime2, dtc.addMonths(validity));
                reg->setValue(kMonths, validity);

                return true;
            }
        }
        dtct.addDays(-1);
    }
    return false;
}

bool BMachineControl_64::initializeReg()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dtc = QDateTime::currentDateTime();

    if ( !reg->allKeys().contains(kDateTime0) ) {
        reg->setValue(kKey, "");
        reg->setValue(kDateTime0, dtc);
        reg->setValue(kDateTime1, dtc);
        reg->setValue(kDateTime2, dtc);
        reg->setValue(kMonths, 0);

        return true;
    } else {
        return false;
    }
}

bool BMachineControl_64::judgeDate()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dt1 = reg->value(kDateTime1).toDateTime();

    QDateTime dtc = QDateTime::currentDateTime();

    if ( dt1.secsTo(dtc) > 0 ) {
        return true;
    } else {
        return false;
    }
}

int BMachineControl_64::judgeKey()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dt0 = reg->value(kDateTime0).toDateTime();
    QDate d0 = dt0.date();
    QDateTime dt2 = reg->value(kDateTime2).toDateTime();
    int months = reg->value(kMonths).toInt();
    QDateTime dtc = QDateTime::currentDateTime();

    {
        QDateTime dt1 = reg->value(kDateTime1).toDateTime();
        int secs = dt0.secsTo(dt1);
        int days1 = dt0.daysTo(dtc);
        if (secs == 0) {
            dt2 = dt2.addDays(days1);
            reg->setValue(kDateTime2, dt2);
            QMessageBox::information(nullptr, "Infomation", QString("%1 days !  ").arg(dt0.daysTo(dt2)), QMessageBox::Ok);
        }
    }

    QString k0 = getKey(getInfo(), d0.toString("ddMMyyyy"), months);

    if ( k0 == reg->value(kKey).toString() ) {
        if ( dtc.secsTo(dt2) > 0 ) {
            return dtc.daysTo(dt2);
        } else {
            return -1;
        }
    } else {
        return -2;
    }
}

void BMachineControl_64::refreshDT1()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dtc = QDateTime::currentDateTime();

    reg->setValue(kDateTime1, dtc);
}
