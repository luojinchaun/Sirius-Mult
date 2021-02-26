#ifndef BLOG_H
#define BLOG_H

#include <QTextStream>
#include <QFile>
#include <QIODevice>

class BLog
{
public:
    BLog();

    void openFile(const QString &source, QIODevice::OpenMode flags);

    void closeFile();

    QString read(bool oneLine);

    bool write(const QString &data);

private:
    QFile m_file;
};

#endif // BLOG_H
