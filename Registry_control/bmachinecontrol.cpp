#include "bmachinecontrol.h"
#include <QMessageBox>

BMachineControl::BMachineControl()
{

}

QString BMachineControl::getWMIHWInfo(int type)
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

QString BMachineControl::getCPUID1()
{
#if defined(_AMD64_)
    return "";
#else
    char    OEMString[13];
    QString result;
    int     iEAXValue, iEBXValue, iECXValue, iEDXValue;

    _asm
    {
        mov     eax,0
        cpuid
        mov     DWORD     PTR     OEMString,ebx
        mov     DWORD     PTR     OEMString+4,edx
        mov     DWORD     PTR     OEMString+8,ecx
        mov     BYTE      PTR     OEMString+12,0
    }

    _asm
    {
        mov     eax,1
        cpuid
        mov     iEAXValue,eax
        mov     iEBXValue,ebx
        mov     iECXValue,ecx
        mov     iEDXValue,edx
    }

    int iCPUFamily = (0xf00 & iEAXValue) >> 8;
    char Family[10] = {0};
    _itoa_s(iCPUFamily, Family, 10);

    _asm
    {
        mov     eax,2
        CPUID
    }

    char szCPUID[129] = {NULL};
    char szTmp[33] = {NULL};
    unsigned long s1 = 0, s2 = 0;

    _asm
    {
        mov     eax,01h
        xor     edx,edx
        cpuid
        mov     s1,edx
        mov     s2,eax
    }
    sprintf_s(szTmp, "%08X%08X", s1, s2);
    strcpy_s(szCPUID, szTmp);

    _asm
    {
        mov     eax,03h
        xor     ecx,ecx
        xor     edx,edx
        cpuid
        mov     s1,edx
        mov     s2,ecx
    }

    sprintf_s(szTmp, "%08X%08X", s1, s2);
    strcat_s(szCPUID, szTmp);

    result = QString(szCPUID).toUpper();
    return result;
#endif
}

QString BMachineControl::getCPUID2()
{
#if defined(_AMD64_)
    return "";
#else
    DWORD   dwId1, dwId2, dwId3, dwId4;
    char    szCompany[13];
    PCHAR   pCompany = szCompany;
    szCompany[12] = 0;

    _asm
    {
        pushfd
        pushad
        mov   eax,1
        _emit   0x0f
        _emit   0xa2
        mov   dwId1,eax
        mov   dwId2,ebx
        mov   dwId3,ecx
        mov   dwId4,edx
        mov   edi,pCompany
        mov   eax,0
        _emit   0x0f
        _emit   0xa2
        mov   eax,ebx
        stosd
        mov   eax,edx
        stosd
        mov   eax,ecx
        stosd
        popad
        popfd
    }

    DWORD dwResult = 0;
    DWORD dwTemp1 = dwId1 << 12;
    DWORD dwTemp2 = dwId2 << 8 ;
    DWORD dwTemp3 = dwId3 << 4;

    QString res = QString("splitted string is %1_%2_%3_%4").arg(QString::number(dwTemp1,16)).
            arg(QString::number(dwTemp2,16)).arg(QString::number(dwTemp3,16)).arg(QString::number(dwId4,16));
    dwResult = dwTemp1 + dwTemp2 + dwTemp3 + dwId4;
    QString result = QString::number(dwResult,16).toUpper();
    QString cpy = QString::fromLocal8Bit(szCompany);
    return result;
#endif
}

QString BMachineControl::getHDLogicalID()
{
    DWORD VolumeSerialNumber;
    GetVolumeInformation(L"C:\\", NULL, 0, &VolumeSerialNumber, NULL, NULL, NULL, 0);
    return QString::number(VolumeSerialNumber, 16).toUpper();
}

QString BMachineControl::getMac()
{
    QString macAddress;
    QList<QNetworkAddressEntry> lclInfAE;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list)
    {
        if ( !(iface.humanReadableName().contains("VMware",Qt::CaseInsensitive)) &&
             !(iface.humanReadableName().contains("Tunnel",Qt::CaseInsensitive)) &&
             !(iface.humanReadableName().contains("Tunneling",Qt::CaseInsensitive)) &&
             !(iface.humanReadableName().contains("Loopback",Qt::CaseInsensitive)) &&
             !(iface.humanReadableName().contains("Pseudo",Qt::CaseInsensitive)) )
        {
            if ( iface.hardwareAddress() != "" ) {
                macAddress = iface.hardwareAddress().toUpper();
            }

        }
    }
    return macAddress;
}

QString BMachineControl::getCPUManID()
{
#if defined(_AMD64_)
    return "";
#else
    DWORD deax;
    DWORD debx;
    DWORD decx;
    DWORD dedx;

    char ID[25];
    memset(ID, 0, sizeof(ID));
    __asm
    {
        mov eax,0
        cpuid
        mov deax,eax
        mov debx,ebx
        mov decx,ecx
        mov dedx,edx
    }

    memcpy(ID+0, &debx, 4);
    memcpy(ID+4, &dedx, 4);
    memcpy(ID+8, &decx, 4);

    return QString::fromLocal8Bit(ID);
#endif
}

QString BMachineControl::getInfo()
{
    QString info;
    for ( int i = 1; i != 6; ++i )
    {
        info.append(getWMIHWInfo(i));
    }
    info.append(getCPUID1());
    info.append(getCPUID2());
    info.append(getHDLogicalID());
//    info.append(getMac());
    info.append(getCPUManID());

    return info;
}

QString BMachineControl::getKey(QString machineinfo, QString ddMMyyyy, int months)
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

bool BMachineControl::activeKey(QString key)
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

                copyToFile(reg, true);
                return true;
            }
        }
        dtct.addDays(-1);
    }
    return false;
}

void BMachineControl::copyToFile(QSettings* reg, bool magic)
{
    QFileInfo info(kFile);
    QString path = info.path();

    QDir dir(path);
    if ( !dir.exists() ) {
        dir.mkpath(path);
    }

    QFile f(kFile);
    f.open(QIODevice::ReadWrite);
    QDataStream out(&f);
    out << reg->value(kKey).toString();
    out << reg->value(kDateTime0).toDateTime();
    out << reg->value(kDateTime1).toDateTime();
    out << reg->value(kDateTime2).toDateTime();
    out << reg->value(kMonths).toInt();

    if ( magic ) {
        out << "Stray birds of summer come to my window to sing and fly away. "
               "And yellow leaves of autumn, which have no songs, "
               "flutter and fall there with a sign.";
    }

    f.close();
}

bool BMachineControl::initializeReg()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dtc = QDateTime::currentDateTime();

    QFileInfo info(kFile);

    if ( !reg->allKeys().contains(kDateTime0) ) {
        reg->setValue(kKey, "");
        reg->setValue(kDateTime0, dtc);
        reg->setValue(kDateTime1, dtc);
        reg->setValue(kDateTime2, dtc);
        reg->setValue(kMonths, 0);

        copyToFile(reg, true);
        return true;
    } else {
        return false;
    }
}

bool BMachineControl::judgeFile()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QFile f(kFile);
    f.open(QIODevice::ReadOnly);

    QString k;
    QDateTime dt0;
    QDateTime dt1;
    QDateTime dt2;
    int m;

    QDataStream in(&f);
    in >> k >> dt0 >> dt1 >> dt2 >> m;

    f.close();

    if (k != reg->value(kKey).toString() ||
            dt0 != reg->value(kDateTime0).toDateTime() ||
            dt1 != reg->value(kDateTime1).toDateTime() ||
            dt2 != reg->value(kDateTime2).toDateTime() ||
            m != reg->value(kMonths).toInt()) {
        return false;
    } else {
        return true;
    }
}

bool BMachineControl::judgeDate()
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

int BMachineControl::judgeKey()
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

void BMachineControl::refreshDT1()
{
    QSettings *reg = new QSettings(kReg, QSettings::NativeFormat);

    QDateTime dtc = QDateTime::currentDateTime();

    reg->setValue(kDateTime1, dtc);

    copyToFile(reg);
}
