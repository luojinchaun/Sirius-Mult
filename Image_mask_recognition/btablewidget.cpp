#include "btablewidget.h"

BTableWidget::BTableWidget(QWidget *parent) : QTableWidget(parent)
{

}

void BTableWidget::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_F4 || event->key() == Qt::Key_Enter ) {
        event->ignore();
    }

    QTableWidget::keyPressEvent(event);
}
