#ifndef BQGRAPHICSSCENE_H
#define BQGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include "bqgraphicsitem.h"

#include <opencv2/opencv.hpp>

#include "globalfun.h"
class BQGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    BQGraphicsScene(QObject *parent = nullptr);

    void setPixmapSize(int width, int height);
    void setMask(QImage image);
    void createBGraphicsItem(BGraphicsItem::ItemType type, QString text);
    QList<BGraphicsItem *> getGraphicsItemList();
    QGraphicsPixmapItem& getPixmapItem();
    bool getLiveStatus();
    void updateLiveMode(bool status, std::vector<cv::Mat> &list);

    void saveItemToConfig(QString file = GlobalFun::getCurrentPath() + "/item.ini");
    void loadItemToScene(QString file = GlobalFun::getCurrentPath() + "/item.ini",bool isFirst=true);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

signals:
    void updatePoint(QString text, QList<QPointF> list, bool isFinished);
    void createConfig(int num);
    void removeConfig(int num);
    void updateTableView(int hole_num);
    void resetView();
    void changeLiveMode(bool status);

public slots:
    void updateFrame(QImage image);

protected:
    QList<QPointF> m_list;
    bool is_creating_BPolygon;
    bool is_ignore_contextMenu;
    QString current_polygon;

    // contextMenu
    int auto_type;
    bool is_locked;
    bool is_alive;
    bool is_auto;

    QGraphicsPixmapItem m_pixmapItem;
    QGraphicsPixmapItem m_unlivePixmap;
    QGraphicsSimpleTextItem m_simpleText;
    QGraphicsPixmapItem *m_mask;
    std::vector<cv::Mat> m_matList;
    QList<BGraphicsItem *> m_graphicsItemList;
};

#endif // BQGRAPHICSSCENE_H
