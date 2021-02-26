#ifndef BQGRAPHICSVIEW_H
#define BQGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QKeyEvent>
#include <QEvent>
#include <QScrollBar>

class BQGraphicsView : public QGraphicsView
{
public:
    BQGraphicsView(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual bool eventFilter(QObject * target, QEvent * event) override;
};

#endif // BQGRAPHICSVIEW_H
