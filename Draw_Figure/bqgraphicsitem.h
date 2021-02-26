#ifndef BQGRAPHICSITEM_H
#define BQGRAPHICSITEM_H

#include <QObject>
#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

//#define PI 3.1415926

class BGraphicsItem : public QObject, public QAbstractGraphicsShapeItem
{
    Q_OBJECT

public:
    enum ItemType {
        Circle = 0,         // 圆
        Ellipse,            // 椭圆
        Concentric_Circle,  // 同心圆
        Rectangle,          // 矩形
        Square,             // 正方形
        Polygon,            // 多边形
        Pill,               // 圆端矩形
        Chamfer             // 圆角矩形
    };

    enum AutoType {
        Auto_Circle = 0,    // 自动圆
        Auto_Ellipse,       // 自动椭圆
        Auto_Pill,          // 自动圆端矩形
        Auto_Chamfer,       // 自动圆角矩形
        Auto_RotateRec,     // 自动旋转矩形
        Auto_RoundEdgeRec,  // 自动圆边矩形
    };

    bool getIfSelected() { return m_isSelected; }
    QPointF getCurrentCenter() { return this->mapToScene(QPointF(0,0)); }
    qreal getTextHeight() { return m_textItem.boundingRect().height(); }
    QString getTextCon() { return m_textItem.text(); }
    void setTextCon(QString text) { m_textItem.setText(text); }
    void setTextPos(qreal x, qreal y) { m_textItem.setPos(x,y); }

    AutoType getAutoType() { return m_autoType; }
    bool getIsUseScale() { return !is_value; }
    qreal getAutoWidth() { return m_autoWidth; }
    qreal getAutoHeight() { return m_autoHeight; }
    qreal getAutoCirRadius() { return m_autoCircleRadius; }
    qreal getAutoChaRadius() { return m_autoChamferRadius; }
    qreal getAutoDis1() { return m_distance1; }
    qreal getAutoDis2() { return m_distance2; }
    qreal getAutoScale() { return m_autoScale; }

    void setAutoType(AutoType type) { m_autoType = type; }
    void setIsUseScale(bool useScale) { is_value = !useScale; }
    void setAutoWidth(qreal width) { m_autoWidth = width; }
    void setAutoHeight(qreal height) { m_autoHeight = height; }
    void setAutoCirRadius(qreal circleRadius) { m_autoCircleRadius = circleRadius; }
    void setAutoChaRadius(qreal chamferRadius) { m_autoChamferRadius = chamferRadius; }
    void setAutoDis1(qreal distance1) { m_distance1 = distance1; }
    void setAutoDis2(qreal distance2) { m_distance2 = distance2; }
    void setAutoScale(qreal scale) { m_autoScale = scale; }

    ItemType getType() { return m_type; }
    qreal getWidth() { return m_width; }
    qreal getHeight() { return m_height; }
    qreal getRadius() { return m_radius; }
    qreal getRotation() { return m_rotation; }
    QList<QPointF> getPointList() { return m_list; }

    void setType(ItemType type) { m_type = type; }
    void setWidth(qreal width) { m_width = width; }
    void setHeight(qreal height) { m_height = height; }
    void setRadius(qreal radius) { m_radius = radius; }
    void setMyRotation(qreal rotation) { m_rotation = rotation; }

    void changeType(BGraphicsItem::ItemType type);
    void resetAutoData();
    void setEnable(bool enable);
    void updateScreen();

    QPointF getCentroid(QList<QPointF> list);
    void getMaxLength();

signals:
    void startCreatePolygon(QString text);

public slots:
    void updatePolygon(QString text, QList<QPointF> list, bool isFinished);

protected:
    BGraphicsItem(ItemType type);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

protected:
    ItemType m_type;
    bool m_isSelected;
    bool m_isLocked;

    AutoType m_autoType;
    bool is_value;
    qreal m_autoWidth;
    qreal m_autoHeight;
    qreal m_autoCircleRadius;
    qreal m_autoChamferRadius;
    qreal m_distance1;
    qreal m_distance2;
    qreal m_autoScale;

    qreal m_width;
    qreal m_height;
    qreal m_radius;
    qreal m_rotation;
    QGraphicsSimpleTextItem m_textItem;

    QList<QPointF> m_list;
    bool is_create_finished;
    QPointF m_center;

    QPen m_pen_isSelected;
    QPen m_pen_noSelected;
};

//------------------------------------------------------------------------------

class BSyncytia : public BGraphicsItem
{
public:
    BSyncytia(ItemType type, qreal width, qreal height, QString text);

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};

#endif // BQGRAPHICSITEM_H
