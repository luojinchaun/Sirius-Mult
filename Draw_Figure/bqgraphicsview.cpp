#include "bqgraphicsview.h"
#include "globalfun.h"

BQGraphicsView::BQGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    // 隐藏水平/竖直滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 设置场景范围
    setSceneRect(-390, -240, 780, 480);

    // 反锯齿
    setRenderHints(QPainter::Antialiasing);
}

void BQGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if ( event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Equal ) {
        this->scale(1.05, 1.05);
    } else if ( event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Minus) {
        this->scale(0.95, 0.95);
    } else if ( event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_0 ) {
        this->resetTransform();
        this->scale(GlobalValue::com_p_sle, GlobalValue::com_p_sle);
    }
    QGraphicsView::keyPressEvent(event);
}

bool BQGraphicsView::eventFilter(QObject * target, QEvent * event)
{
     if ( (target == this->verticalScrollBar() || target == this->horizontalScrollBar()) && event->type() == QEvent::Wheel ) {
         return true;
     }
    return false;
}
