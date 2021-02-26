#ifndef BTABLEWIDGET_H
#define BTABLEWIDGET_H

#include <QTableWidget>
#include <QKeyEvent>
#include <QEvent>

class BTableWidget : public QTableWidget
{
public:
    BTableWidget(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif // BTABLEWIDGET_H
