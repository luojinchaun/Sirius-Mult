#include "bqgraphicsitem.h"
#include "globalfun.h"
#include <QVector>

#include <QMenu>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QRadioButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

BGraphicsItem::BGraphicsItem(ItemType type) : m_type(type)
{
    resetAutoData();

    m_isSelected = false;
    m_isLocked = false;
    m_width = m_height = m_radius = m_rotation = 0;

    m_pen_noSelected.setColor(QColor(0, 160, 230));
    m_pen_noSelected.setWidth(2);
    m_pen_isSelected.setColor(QColor(255, 0, 255));
    m_pen_isSelected.setWidth(2);
    this->setPen(m_pen_noSelected);

    QFont font;
    font.setPixelSize(60);
    m_textItem.setFont(font);
    m_textItem.setBrush(Qt::yellow);
    m_textItem.setParentItem(this);
}

void BGraphicsItem::changeType(BGraphicsItem::ItemType type)
{
    m_type = type;
    m_width = m_height = m_radius = m_rotation = 0;

    switch (m_type) {
    case BGraphicsItem::ItemType::Circle: {
        m_width = m_height = GlobalValue::gra_c_rad;
    } break;
    case BGraphicsItem::ItemType::Ellipse: {
        m_width = GlobalValue::gra_e_wid;
        m_height = GlobalValue::gra_e_hei;
    } break;
    case BGraphicsItem::ItemType::Concentric_Circle: {
        qreal radius1 = GlobalValue::gra_cc_rad_1;
        qreal radius2 = GlobalValue::gra_cc_rad_2;
        m_width = m_height = radius1 >= radius2 ? radius1 : radius2;
        m_radius = radius1 >= radius2 ? radius2 : radius1;;
    } break;
    case BGraphicsItem::ItemType::Rectangle: {
        m_width = GlobalValue::gra_r_wid;
        m_height = GlobalValue::gra_r_hei;
    } break;
    case BGraphicsItem::ItemType::Square: {
        m_width = m_height = GlobalValue::gra_s_len;
    } break;
    case BGraphicsItem::ItemType::Polygon: {
        m_list.clear();
        emit startCreatePolygon(m_textItem.text());
    } break;
    case BGraphicsItem::ItemType::Pill: {
        m_width = GlobalValue::gra_p_wid;
        m_height = GlobalValue::gra_p_hei;
    } break;
    case BGraphicsItem::ItemType::Chamfer: {
        m_width = GlobalValue::gra_ch_wid;
        m_height = GlobalValue::gra_ch_hei;
        m_radius = GlobalValue::gra_ch_rad;
    } break;
    default: break;
    }

    updateScreen();
}

void BGraphicsItem::resetAutoData()
{
    m_autoType = BGraphicsItem::AutoType::Auto_Circle;
    is_value = true;
    m_autoWidth = GlobalValue::cam_p_cal*2* GlobalValue::gra_def_size;
    m_autoHeight = GlobalValue::cam_p_cal*2* GlobalValue::gra_def_size;
    m_autoCircleRadius = GlobalValue::cam_p_cal*2* GlobalValue::gra_def_size;
    m_autoChamferRadius = GlobalValue::cam_p_cal*0.5* GlobalValue::gra_def_size;
    m_distance1 = GlobalValue::cam_p_cal* GlobalValue::gra_def_size;
    m_distance2 = GlobalValue::cam_p_cal*(-1)* GlobalValue::gra_def_size;
    m_autoScale = 1;
}

void BGraphicsItem::setEnable(bool enable)
{
    m_isLocked = !enable;
    this->setFlag(QGraphicsItem::ItemIsSelectable, enable);
    this->setFlag(QGraphicsItem::ItemIsMovable, enable);
    this->setFlag(QGraphicsItem::ItemIsFocusable, enable);
}

void BGraphicsItem::updatePolygon(QString text, QList<QPointF> list, bool isFinished)
{
    if (text == m_textItem.text()) {
        is_create_finished = isFinished;
        m_list.clear();
        for ( int i = 0; i < list.size(); ++i )
        {
            m_list.push_back(this->mapFromScene(list.at(i)));
        }

        m_center = getCentroid(m_list);
        getMaxLength();
        updateScreen();
    }
}

QPointF BGraphicsItem::getCentroid(QList<QPointF> list)
{
    qreal x = 0;
    qreal y = 0;
    for (auto &temp : list)
    {
        x += temp.x();
        y += temp.y();
    }
    x = x/list.size();
    y = y/list.size();
    return QPointF(x,y);
}

void BGraphicsItem::getMaxLength()
{
    QVector<qreal> vec;
    for (auto &temp : m_list)
    {
        qreal dis = sqrt(pow(m_center.x() - temp.x(), 2) + pow(m_center.y() - temp.y(), 2));
        vec.append(dis);
    }

    qreal ret = 0;
    for (auto &temp : vec)
    {
        if (temp > ret) {
            ret = temp;
        }
    }
    m_width = ret * 2;
    m_height = ret * 2;
}

void BGraphicsItem::updateScreen()
{
    if (m_width >= m_radius) {
        if ( m_type == BGraphicsItem::ItemType::Polygon ) {
            setTextPos( m_center.x() - m_width/2, m_center.y() - m_height/2 - getTextHeight() );
        } else {
            setTextPos( (-1)*m_width/2, (-1)*m_height/2 - getTextHeight() );
        }
    } else {
        setTextPos( (-1)*m_radius/2, (-1)*m_radius/2 - getTextHeight() );
    }
    setRotation(m_rotation);

    this->hide();
    this->update();
    this->show();
}

void BGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ( event->buttons() == Qt::LeftButton && !m_isLocked ) {
        this->setPen(m_pen_isSelected);
        this->m_isSelected = true;
    }

    QGraphicsItem::mousePressEvent(event);
}

void BGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->setPen(m_pen_noSelected);
    this->m_isSelected = false;
}

void BGraphicsItem::keyPressEvent(QKeyEvent *event)
{
    switch ( event->key() )
    {
    case Qt::Key_Up:
    case Qt::Key_W: {
        this->moveBy(0, -1);
    } break;
    case Qt::Key_Down:
    case Qt::Key_S: {
        this->moveBy(0, 1);
    } break;
    case Qt::Key_Left:
    case Qt::Key_A: {
        this->moveBy(-1, 0);
    } break;
    case Qt::Key_Right:
    case Qt::Key_D: {
        this->moveBy(1, 0);
    } break;
    case Qt::Key_F4:
    case Qt::Key_Enter:
    case Qt::Key_L:
    case Qt::Key_F1:
    case Qt::Key_P:
    case Qt::Key_J:
    case Qt::Key_B:
    case Qt::Key_F12: {
        QAbstractGraphicsShapeItem::keyPressEvent(event);
    }
    default: break;
    }
}

void BGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!m_isLocked) {
        double value = GlobalValue::cam_p_cal/10;

        switch (m_type)
        {
        case BGraphicsItem::ItemType::Concentric_Circle: {
            if ( event->delta() > 0 ) { m_radius += value; }
            if ( event->delta() < 0 && m_radius > value*3 ) { m_radius -= value; }
        }
        case BGraphicsItem::ItemType::Circle:
        case BGraphicsItem::ItemType::Ellipse:
        case BGraphicsItem::ItemType::Rectangle:
        case BGraphicsItem::ItemType::Square:
        case BGraphicsItem::ItemType::Pill: {
            if ( event->delta() > 0 ) { m_width += value; m_height += value; }
            if ( event->delta() < 0 && m_width > value*5 && m_height > value*5 ) { m_width -= value; m_height -= value; }
        } break;
        case BGraphicsItem::ItemType::Chamfer: {
            if ( event->delta() > 0 ) { m_width += value; m_height += value; }
            if ( event->delta() < 0 && m_width > value*5 && m_height > value*5 ) {
                m_width -= value; m_height -= value;
                m_width = m_width < m_radius*2 ? m_radius*2 : m_width;
                m_height = m_height < m_radius*2 ? m_radius*2 : m_height;
            }
        } break;
        case BGraphicsItem::ItemType::Polygon: {} break;
        default: break;
        }

        updateScreen();
    }
}

//------------------------------------------------------------------------------

BSyncytia::BSyncytia(ItemType type, qreal width, qreal height, QString text) : BGraphicsItem(type)
{
    m_width = width;
    m_height = height;
    m_textItem.setText(text);//按照要求将字体大小缩小到一半,原pixelSize为60
    QFont font;
    font.setPixelSize(30);
    m_textItem.setFont(font);
    setTextPos( (-1)*m_width/2, (-1)*m_height/2 - getTextHeight() );
}

QRectF BSyncytia::boundingRect() const
{
    int penWidth = m_pen_isSelected.width();

    if (m_type == BGraphicsItem::ItemType::Polygon) {
        return QRectF ( m_center.x() - m_width/2 - penWidth, m_center.y() - m_height/2 - penWidth,
                        m_width + penWidth*2, m_height + penWidth*2 );
    }

    if (m_width >= m_radius) {
        return QRectF ( (-1)*m_width/2 - penWidth, (-1)*m_height/2 - penWidth,
                        m_width + penWidth*2, m_height + penWidth*2 );
    } else {
        return QRectF ( (-1)*m_radius/2 - penWidth, (-1)*m_radius/2 - penWidth,
                        m_radius + penWidth*2, m_radius + penWidth*2 );
    }
}

void BSyncytia::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(this->pen());
    painter->setBrush(this->brush());

    QRectF rect( (-1)*m_width/2, (-1)*m_height/2, m_width, m_height );

    switch (m_type) {
    case BGraphicsItem::ItemType::Circle: {
        painter->drawEllipse(rect);
    } break;
    case BGraphicsItem::ItemType::Ellipse: {
        painter->drawEllipse(rect);
    } break;
    case BGraphicsItem::ItemType::Concentric_Circle: {
        painter->drawEllipse(rect);
        painter->drawEllipse(QRectF( (-1)*m_radius/2, (-1)*m_radius/2, m_radius, m_radius ));
    } break;
    case BGraphicsItem::ItemType::Rectangle: {
        painter->drawRect(rect);
    } break;
    case BGraphicsItem::ItemType::Square: {
        painter->drawRect(rect);
    } break;
    case BGraphicsItem::ItemType::Polygon: {
        if (m_list.size() > 1) {
            if (is_create_finished) {
                for (int i = 1; i < m_list.size(); i++)
                {
                    painter->drawLine(m_list.at(i-1), m_list.at(i));
                }

                painter->drawLine(m_list.at(m_list.size()-1), m_list.at(0));
            } else {
                for (int i = 1; i < m_list.size(); i++)
                {
                    painter->drawLine(m_list.at(i-1), m_list.at(i));
                }
            }
        }
    } break;
    case BGraphicsItem::ItemType::Pill: {
        painter->drawRoundedRect(rect, m_height/2, m_height/2);
    } break;
    case BGraphicsItem::ItemType::Chamfer: {
        painter->drawRoundedRect(rect, m_radius, m_radius);
    } break;
    default: break;
    }
}

void BSyncytia::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if ( !this->isSelected() )
        return;

    if ( GlobalValue::com_tp == 1 )
    {
        QMenu* menu = new QMenu();

        switch (m_type)
        {
        case BGraphicsItem::ItemType::Concentric_Circle: {
            QDoubleSpinBox* radius_spinBox = new QDoubleSpinBox(menu);
            radius_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            radius_spinBox->setRange(0.1, 100);
            radius_spinBox->setPrefix("d1: ");
            radius_spinBox->setSuffix(" mm");
            radius_spinBox->setSingleStep(0.1);
            radius_spinBox->setValue(m_radius/GlobalValue::cam_p_cal);
            connect(radius_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                m_radius = v*GlobalValue::cam_p_cal;
                updateScreen();
            });

            QWidgetAction* radius_widgetAction = new QWidgetAction(menu);
            radius_widgetAction->setDefaultWidget(radius_spinBox);
            menu->addAction(radius_widgetAction);
        }
        case BGraphicsItem::ItemType::Circle: {
            QDoubleSpinBox* radius_spinBox = new QDoubleSpinBox(menu);
            radius_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            radius_spinBox->setRange(0.1, 100);
            if (m_type == BGraphicsItem::ItemType::Concentric_Circle) {
                radius_spinBox->setPrefix("d2: ");
            } else {
                radius_spinBox->setPrefix("d: ");
            }
            radius_spinBox->setSuffix(" mm");
            radius_spinBox->setSingleStep(0.1);
            radius_spinBox->setValue(m_width/GlobalValue::cam_p_cal);
            connect(radius_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                m_width = v*GlobalValue::cam_p_cal;
                m_height = v*GlobalValue::cam_p_cal;
                updateScreen();
            });

            QWidgetAction* radius_widgetAction = new QWidgetAction(menu);
            radius_widgetAction->setDefaultWidget(radius_spinBox);
            menu->addAction(radius_widgetAction);
        } break;
        case BGraphicsItem::ItemType::Square: {
            QDoubleSpinBox* width_spinBox = new QDoubleSpinBox(menu);
            width_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            width_spinBox->setRange(0.1, 100);
            width_spinBox->setPrefix("L: ");
            width_spinBox->setSuffix(" mm");
            width_spinBox->setSingleStep(0.1);
            width_spinBox->setValue(m_width/GlobalValue::cam_p_cal);
            connect(width_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                m_width = v*GlobalValue::cam_p_cal;
                m_height = v*GlobalValue::cam_p_cal;
                updateScreen();
            });

            QWidgetAction* width_widgetAction = new QWidgetAction(menu);
            width_widgetAction->setDefaultWidget(width_spinBox);
            menu->addAction(width_widgetAction);
        } break;
        case BGraphicsItem::ItemType::Chamfer: {
            QDoubleSpinBox* radius_spinBox = new QDoubleSpinBox(menu);
            radius_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            radius_spinBox->setRange(0, 100);
            radius_spinBox->setPrefix("r: ");
            radius_spinBox->setSuffix(" mm");
            radius_spinBox->setSingleStep(0.1);
            radius_spinBox->setValue(m_radius/GlobalValue::cam_p_cal);
            connect(radius_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                double temp = m_width >= m_height ? m_height/2/GlobalValue::cam_p_cal : m_width/2/GlobalValue::cam_p_cal;
                if ( v > temp ) {
                    m_radius = temp*GlobalValue::cam_p_cal;
                } else {
                    m_radius = v*GlobalValue::cam_p_cal;
                }

                updateScreen();
            });

            QWidgetAction* radius_widgetAction = new QWidgetAction(menu);
            radius_widgetAction->setDefaultWidget(radius_spinBox);
            menu->addAction(radius_widgetAction);
        }
        case BGraphicsItem::ItemType::Ellipse:
        case BGraphicsItem::ItemType::Rectangle:
        case BGraphicsItem::ItemType::Pill: {
            QDoubleSpinBox* width_spinBox = new QDoubleSpinBox(menu);
            width_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            width_spinBox->setRange(0.1, 100);
            width_spinBox->setPrefix("w: ");
            width_spinBox->setSuffix(" mm");
            width_spinBox->setSingleStep(0.1);
            width_spinBox->setValue(m_width/GlobalValue::cam_p_cal);
            connect(width_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                if ( m_type == BGraphicsItem::ItemType::Chamfer && v < m_radius*2/GlobalValue::cam_p_cal ) {
                    m_width = m_radius*2;
                } else if ( m_type == BGraphicsItem::ItemType::Pill && v < m_height/GlobalValue::cam_p_cal ) {
                    m_width = m_height;
                } else {
                    m_width = v*GlobalValue::cam_p_cal;
                }
                updateScreen();
            });

            QDoubleSpinBox* height_spinBox = new QDoubleSpinBox(menu);
            height_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
            height_spinBox->setRange(0.1, 100);
            height_spinBox->setPrefix("h: ");
            height_spinBox->setSuffix(" mm");
            height_spinBox->setSingleStep(0.1);
            height_spinBox->setValue(m_height/GlobalValue::cam_p_cal);
            connect(height_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
                if ( m_type == BGraphicsItem::ItemType::Chamfer && v < m_radius*2/GlobalValue::cam_p_cal ) {
                    m_height = m_radius*2;
                } else if ( m_type == BGraphicsItem::ItemType::Pill && v > m_width/GlobalValue::cam_p_cal ) {
                    m_height = m_width;
                } else {
                    m_height = v*GlobalValue::cam_p_cal;
                }
                updateScreen();
            });

            QWidgetAction* width_widgetAction = new QWidgetAction(menu);
            width_widgetAction->setDefaultWidget(width_spinBox);
            menu->addAction(width_widgetAction);

            QWidgetAction* height_widgetAction = new QWidgetAction(menu);
            height_widgetAction->setDefaultWidget(height_spinBox);
            menu->addAction(height_widgetAction);
        } break;
        case BGraphicsItem::ItemType::Polygon: break;
        default: break;
        }

        QSpinBox* rotation_spinBox = new QSpinBox(menu);
        rotation_spinBox->setStyleSheet("QSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        rotation_spinBox->setRange(-360, 360);
        rotation_spinBox->setPrefix("rotate: ");
        rotation_spinBox->setSuffix("");
        rotation_spinBox->setSingleStep(1);
        rotation_spinBox->setValue(m_rotation);
        connect(rotation_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int v){
            m_rotation = v;
            updateScreen();
        });

        QWidgetAction* rotation_widgetAction = new QWidgetAction(menu);
        rotation_widgetAction->setDefaultWidget(rotation_spinBox);
        menu->addAction(rotation_widgetAction);

        //------------------------------------------------------------------------------

        QMenu* change_menu = new QMenu(GlobalString::graphics_change, menu);
        change_menu->setIcon(QIcon(":/images/Change.png"));
        QAction* circle     = new QAction(QIcon(":/images/Circle.png"), GlobalString::graphics_circle, change_menu);
        QAction* ellipse    = new QAction(QIcon(":/images/Ellipse.png"), GlobalString::graphics_ellipse, change_menu);
        QAction* conCircle  = new QAction(QIcon(":/images/Concentric_Circle.png"), GlobalString::graphics_concentric_circle, change_menu);
        QAction* rectangle  = new QAction(QIcon(":/images/Rectangle.png"), GlobalString::graphics_rectangle, change_menu);
        QAction* square     = new QAction(QIcon(":/images/Square.png"), GlobalString::graphics_square, change_menu);
        QAction* polygon    = new QAction(QIcon(":/images/Polygon.png"), GlobalString::graphics_polygon, change_menu);
        QAction* pill       = new QAction(QIcon(":/images/Pill.png"), GlobalString::graphics_pill, change_menu);
        QAction* chamfer    = new QAction(QIcon(":/images/Chamfer.png"), GlobalString::graphics_chamfer, change_menu);

        connect(circle, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Circle); });
        connect(ellipse, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Ellipse); });
        connect(conCircle, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Concentric_Circle); });
        connect(rectangle, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Rectangle); });
        connect(square, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Square); });
        connect(polygon, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Polygon); });
        connect(pill, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Pill); });
        connect(chamfer, &QAction::triggered, [&](){ changeType(BGraphicsItem::ItemType::Chamfer); });

        change_menu->addAction(circle);
        change_menu->addAction(ellipse);
        change_menu->addAction(conCircle);
        change_menu->addAction(rectangle);
        change_menu->addAction(square);
        change_menu->addAction(polygon);
        change_menu->addAction(pill);
        change_menu->addAction(chamfer);

        menu->addMenu(change_menu);
        qDebug()<<"contextMenuEvent is 3";

        menu->exec(QCursor::pos());
        delete menu;

    }
    else
    {
        qDebug()<<"contextMenuEvent is 2";
        QMenu* menu = new QMenu();

        QDoubleSpinBox* width_spinBox = new QDoubleSpinBox(menu);
        width_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        width_spinBox->setRange(0.1, 100);
        width_spinBox->setPrefix("L: ");
        width_spinBox->setSuffix(" mm");
        width_spinBox->setSingleStep(0.1);
        width_spinBox->setValue(m_width/GlobalValue::cam_p_cal);
        connect(width_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_width = v*GlobalValue::cam_p_cal;
            m_height = v*GlobalValue::cam_p_cal;
            updateScreen();
        });

        QWidgetAction* width_widgetAction = new QWidgetAction(menu);
        width_widgetAction->setDefaultWidget(width_spinBox);
        menu->addAction(width_widgetAction);

        //------------------------------------------------------------------------------

        QDoubleSpinBox* auto_width_spinBox = new QDoubleSpinBox(menu);
        auto_width_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_width_spinBox->setRange(0.1, 100);
        auto_width_spinBox->setPrefix("w: ");
        auto_width_spinBox->setSuffix(" mm");
        auto_width_spinBox->setSingleStep(0.1);
        auto_width_spinBox->setValue(m_autoWidth/GlobalValue::cam_p_cal);
        connect(auto_width_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_autoWidth = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_height_spinBox = new QDoubleSpinBox(menu);
        auto_height_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_height_spinBox->setRange(0.1, 100);
        auto_height_spinBox->setPrefix("h: ");
        auto_height_spinBox->setSuffix(" mm");
        auto_height_spinBox->setSingleStep(0.1);
        auto_height_spinBox->setValue(m_autoHeight/GlobalValue::cam_p_cal);
        connect(auto_height_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_autoHeight = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_circleRadius_spinBox = new QDoubleSpinBox(menu);
        auto_circleRadius_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_circleRadius_spinBox->setRange(0.1, 100);
        auto_circleRadius_spinBox->setPrefix("d: ");
        auto_circleRadius_spinBox->setSuffix(" mm");
        auto_circleRadius_spinBox->setSingleStep(0.1);
        auto_circleRadius_spinBox->setValue(m_autoCircleRadius/GlobalValue::cam_p_cal);
        connect(auto_circleRadius_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_autoCircleRadius = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_chamferRadius_spinBox = new QDoubleSpinBox(menu);
        auto_chamferRadius_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_chamferRadius_spinBox->setRange(0.1, 100);
        auto_chamferRadius_spinBox->setPrefix("r: ");
        auto_chamferRadius_spinBox->setSuffix(" mm");
        auto_chamferRadius_spinBox->setSingleStep(0.1);
        auto_chamferRadius_spinBox->setValue(m_autoChamferRadius/GlobalValue::cam_p_cal);
        connect(auto_chamferRadius_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_autoChamferRadius = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_dis1_spinBox = new QDoubleSpinBox(menu);
        auto_dis1_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_dis1_spinBox->setRange(-100, 100);
        auto_dis1_spinBox->setPrefix("d1: ");
        auto_dis1_spinBox->setSuffix(" mm");
        auto_dis1_spinBox->setSingleStep(0.1);
        auto_dis1_spinBox->setValue(m_distance1/GlobalValue::cam_p_cal);
        connect(auto_dis1_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_distance1 = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_dis2_spinBox = new QDoubleSpinBox(menu);
        auto_dis2_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_dis2_spinBox->setRange(-100, 100);
        auto_dis2_spinBox->setPrefix("d2: ");
        auto_dis2_spinBox->setSuffix(" mm");
        auto_dis2_spinBox->setSingleStep(0.1);
        auto_dis2_spinBox->setValue(m_distance2/GlobalValue::cam_p_cal);
        connect(auto_dis2_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_distance2 = v*GlobalValue::cam_p_cal;
        });

        QDoubleSpinBox* auto_scale_spinBox = new QDoubleSpinBox(menu);
        auto_scale_spinBox->setStyleSheet("QDoubleSpinBox{ width:120px; height:30px; font-size:16px; font-weight:bold; }");
        auto_scale_spinBox->setRange(0.5, 1);
        auto_scale_spinBox->setPrefix("s: ");
        auto_scale_spinBox->setSuffix("");
        auto_scale_spinBox->setSingleStep(0.1);
        auto_scale_spinBox->setValue(m_autoScale);
        connect(auto_scale_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v){
            m_autoScale = v;
        });

        QWidgetAction* auto_width_widgetAction = new QWidgetAction(menu);
        auto_width_widgetAction->setDefaultWidget(auto_width_spinBox);

        QWidgetAction* auto_height_widgetAction = new QWidgetAction(menu);
        auto_height_widgetAction->setDefaultWidget(auto_height_spinBox);

        QWidgetAction* auto_circleRadius_widgetAction = new QWidgetAction(menu);
        auto_circleRadius_widgetAction->setDefaultWidget(auto_circleRadius_spinBox);

        QWidgetAction* auto_chamferRadius_widgetAction = new QWidgetAction(menu);
        auto_chamferRadius_widgetAction->setDefaultWidget(auto_chamferRadius_spinBox);

        QWidgetAction* auto_dis1_widgetAction = new QWidgetAction(menu);
        auto_dis1_widgetAction->setDefaultWidget(auto_dis1_spinBox);

        QWidgetAction* auto_dis2_widgetAction = new QWidgetAction(menu);
        auto_dis2_widgetAction->setDefaultWidget(auto_dis2_spinBox);

        QWidgetAction* auto_scale_widgetAction = new QWidgetAction(menu);
        auto_scale_widgetAction->setDefaultWidget(auto_scale_spinBox);

        //------------------------------------------------------------------------------

        QRadioButton *value = new QRadioButton("Value");
        QRadioButton *scale = new QRadioButton("Scale");
        if (is_value) {
            value->setChecked(true);

            if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                menu->addAction(auto_circleRadius_widgetAction);
            } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                menu->addAction(auto_width_widgetAction);
                menu->addAction(auto_height_widgetAction);
                menu->addAction(auto_chamferRadius_widgetAction);
            } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                menu->addAction(auto_dis1_widgetAction);
                menu->addAction(auto_dis2_widgetAction);
                menu->addAction(auto_circleRadius_widgetAction);
            } else {
                menu->addAction(auto_width_widgetAction);
                menu->addAction(auto_height_widgetAction);
            }

        } else {
            scale->setChecked(true);
            menu->addAction(auto_scale_widgetAction);
        }

        QHBoxLayout *patternLayout = new QHBoxLayout;
        patternLayout->addWidget(value);
        patternLayout->addWidget(scale);
        patternLayout->addStretch(1);
        QGroupBox *patternBox = new QGroupBox(menu);
        patternBox->setLayout(patternLayout);

        QWidgetAction* patternBox_widgetAction = new QWidgetAction(menu);
        patternBox_widgetAction->setDefaultWidget(patternBox);
        menu->addAction(patternBox_widgetAction);

        connect(value, &QRadioButton::clicked, [&](){
            if (!is_value) {
                is_value = true;
                menu->removeAction(auto_scale_widgetAction);
                if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                    menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                    menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                    menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    menu->insertAction(patternBox_widgetAction, auto_chamferRadius_widgetAction);
                } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                    menu->insertAction(patternBox_widgetAction, auto_dis1_widgetAction);
                    menu->insertAction(patternBox_widgetAction, auto_dis2_widgetAction);
                    menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                } else {
                    menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                    menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                }
            }
        });

        connect(scale, &QRadioButton::clicked, [&](){
            if (is_value) {
                is_value = false;
                if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                    menu->removeAction(auto_circleRadius_widgetAction);
                } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                    menu->removeAction(auto_width_widgetAction);
                    menu->removeAction(auto_height_widgetAction);
                    menu->removeAction(auto_chamferRadius_widgetAction);
                } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                    menu->removeAction(auto_dis1_widgetAction);
                    menu->removeAction(auto_dis2_widgetAction);
                    menu->removeAction(auto_circleRadius_widgetAction);
                } else {
                    menu->removeAction(auto_width_widgetAction);
                    menu->removeAction(auto_height_widgetAction);
                }
                menu->insertAction(patternBox_widgetAction, auto_scale_widgetAction);
            }
        });


//        QHBoxLayout *typeLayout = new QHBoxLayout;//去掉这里就可以去掉cg，ir孔留下的白色区域
//        typeLayout->addStretch(1);
//        QGroupBox *typeBox = new QGroupBox(menu);
//        typeBox->setLayout(typeLayout);

//        QWidgetAction* typeBox_widgetAction = new QWidgetAction(menu);
//        typeBox_widgetAction->setDefaultWidget(typeBox);
//        menu->addAction(typeBox_widgetAction);
        qDebug()<<"contextMenuEvent is 4";

        //------------------------------------------------------------------------------

        QRadioButton *circle = new QRadioButton(GlobalString::graphics_auto_circle);
        QRadioButton *ellipse = new QRadioButton(GlobalString::graphics_auto_ellipse);
        QRadioButton *pill = new QRadioButton(GlobalString::graphics_auto_pill);
        QRadioButton *chamfer = new QRadioButton(GlobalString::graphics_auto_chamfer);
        QRadioButton *graphics_auto_roundEdgeRec = new QRadioButton(GlobalString::graphics_auto_roundEdgeRec);
        QRadioButton *rotationRec = new QRadioButton(GlobalString::graphics_auto_rotateRec);

        connect(circle, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_Circle ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                        menu->removeAction(auto_width_widgetAction);
                        menu->removeAction(auto_height_widgetAction);
                        menu->removeAction(auto_chamferRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                        menu->removeAction(auto_dis1_widgetAction);
                        menu->removeAction(auto_dis2_widgetAction);
                    } else {
                        menu->removeAction(auto_width_widgetAction);
                        menu->removeAction(auto_height_widgetAction);
                    }

                    menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                }

                m_autoType = BGraphicsItem::AutoType::Auto_Circle;
            }
        });
        connect(ellipse, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_Ellipse ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                        menu->removeAction(auto_chamferRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                        menu->removeAction(auto_dis1_widgetAction);
                        menu->removeAction(auto_dis2_widgetAction);
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    }
                }

                m_autoType = BGraphicsItem::AutoType::Auto_Ellipse;
            }
        });
        connect(pill, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_Pill ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                        menu->removeAction(auto_chamferRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                        menu->removeAction(auto_dis1_widgetAction);
                        menu->removeAction(auto_dis2_widgetAction);
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    }
                }

                m_autoType = BGraphicsItem::AutoType::Auto_Pill;
            }
        });
        connect(chamfer, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_Chamfer ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_chamferRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                        menu->removeAction(auto_dis1_widgetAction);
                        menu->removeAction(auto_dis2_widgetAction);
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_chamferRadius_widgetAction);
                    } else {
                        menu->insertAction(patternBox_widgetAction, auto_chamferRadius_widgetAction);
                    }
                }

                m_autoType = BGraphicsItem::AutoType::Auto_Chamfer;
            }
        });
        connect(graphics_auto_roundEdgeRec, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis1_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis2_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                        menu->removeAction(auto_width_widgetAction);
                        menu->removeAction(auto_height_widgetAction);
                        menu->removeAction(auto_chamferRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis1_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis2_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                    } else {
                        menu->removeAction(auto_width_widgetAction);
                        menu->removeAction(auto_height_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis1_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_dis2_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_circleRadius_widgetAction);
                    }
                }

                m_autoType = BGraphicsItem::AutoType::Auto_RoundEdgeRec;
            }
        });
        connect(rotationRec, &QRadioButton::clicked, [&](){
            if ( m_autoType != BGraphicsItem::AutoType::Auto_RotateRec ) {
                if (is_value) {
                    if ( m_autoType == BGraphicsItem::AutoType::Auto_Circle ) {
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_Chamfer ) {
                        menu->removeAction(auto_chamferRadius_widgetAction);
                    } else if ( m_autoType == BGraphicsItem::AutoType::Auto_RoundEdgeRec ) {
                        menu->removeAction(auto_dis1_widgetAction);
                        menu->removeAction(auto_dis2_widgetAction);
                        menu->removeAction(auto_circleRadius_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_width_widgetAction);
                        menu->insertAction(patternBox_widgetAction, auto_height_widgetAction);
                    }
                }

                m_autoType = BGraphicsItem::AutoType::Auto_RotateRec;
            }
        });

        switch (m_autoType)
        {
        case BGraphicsItem::AutoType::Auto_Circle: circle->setChecked(true); break;
        case BGraphicsItem::AutoType::Auto_Ellipse: ellipse->setChecked(true); break;
        case BGraphicsItem::AutoType::Auto_Pill: pill->setChecked(true); break;
        case BGraphicsItem::AutoType::Auto_Chamfer: chamfer->setChecked(true); break;
        case BGraphicsItem::AutoType::Auto_RoundEdgeRec: graphics_auto_roundEdgeRec->setChecked(true); break;
        case BGraphicsItem::AutoType::Auto_RotateRec: rotationRec->setChecked(true); break;
        default: break;
        }

        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(circle);
        vbox->addWidget(ellipse);
        vbox->addWidget(pill);
        vbox->addWidget(chamfer);
        vbox->addWidget(graphics_auto_roundEdgeRec);
        vbox->addWidget(rotationRec);
        vbox->addStretch(1);
        QGroupBox *box2 = new QGroupBox(menu);
        box2->setLayout(vbox);

        QWidgetAction* type_widgetAction = new QWidgetAction(menu);
        type_widgetAction->setDefaultWidget(box2);
        menu->addAction(type_widgetAction);

        menu->exec(QCursor::pos());
        delete menu;
    }

    QGraphicsItem::contextMenuEvent(event);
}
