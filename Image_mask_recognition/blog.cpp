#include "blog.h"

BLog::BLog()
{

}

void BLog::openFile(const QString &source, QIODevice::OpenMode flags)
{
    m_file.setFileName(source);
    m_file.open(flags);
}

void BLog::closeFile()
{
    m_file.close();
}

QString BLog::read(bool oneLine)
{
    QString fileContent = "";
    if ( m_file.isOpen() && m_file.isReadable() ) {
        QString line;
        QTextStream t( &m_file );
        do {
            line = t.readLine();
            fileContent += line;
        } while (!line.isNull() && !oneLine);
    }

    return fileContent;
}

bool BLog::write(const QString &data)
{
    if ( m_file.isOpen() && m_file.isWritable() ) {
        QTextStream out( &m_file );
        out << data << "\n";
        return true;
    }

    return false;
}
