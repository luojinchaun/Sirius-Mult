#include "bqgraphicsscene.h"
#include <QMenu>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

BQGraphicsScene::BQGraphicsScene(QObject *parent) : QGraphicsScene(parent)
{
    is_creating_BPolygon = false;
    is_ignore_contextMenu = false;

    auto_type = 0;
    is_locked = true;
    is_alive = true;
    is_auto = true;

    m_mask = new QGraphicsPixmapItem(&m_pixmapItem);

    QFont font;
    font.setPixelSize(60);
    m_simpleText.setFont(font);
    m_simpleText.setBrush(Qt::yellow);
    m_simpleText.setParentItem(&m_unlivePixmap);

    this->addItem(&m_pixmapItem);
    this->addItem(&m_unlivePixmap);
    this->setBackgroundBrush(Qt::gray);

    int hole_num = GlobalValue::com_h_num;
    int type = GlobalValue::com_tp;

    for ( int i = 0; i < hole_num; ++i )
    {
        if ( type == 1 ) {
            createBGraphicsItem(BGraphicsItem::ItemType::Circle, QString("#%1").arg(i+1));
        } else {
            createBGraphicsItem(BGraphicsItem::ItemType::Square, QString("#%1").arg(i+1));
        }
    }
    QString file = GlobalFun::getCurrentPath() + "/item.ini";
    if ( GlobalFun::isFileExist(file) ) {
        loadItemToScene();
    }
}

void BQGraphicsScene::setPixmapSize(int width, int height)
{
    QPixmap pixmap(QSize(width, height));
    pixmap.fill(QColor(50, 50, 50));
    m_pixmapItem.setPixmap(pixmap);
    m_pixmapItem.setPos(-width/2, -height/2);
}

void BQGraphicsScene::setMask(QImage image)
{
    m_mask->setPixmap(QPixmap::fromImage(image));
}

void BQGraphicsScene::createBGraphicsItem(BGraphicsItem::ItemType type, QString text)
{
    BSyncytia *item;

    switch (type) {
    case BGraphicsItem::ItemType::Circle: {
        qreal radius = GlobalValue::gra_c_rad;
        item = new BSyncytia(BGraphicsItem::ItemType::Circle, radius, radius, text);
    } break;
    case BGraphicsItem::ItemType::Ellipse: {
        qreal width = GlobalValue::gra_e_wid;
        qreal height = GlobalValue::gra_e_hei;
        item = new BSyncytia(BGraphicsItem::ItemType::Ellipse, width, height, text);
    } break;
    case BGraphicsItem::ItemType::Concentric_Circle: {
        qreal radius1 = GlobalValue::gra_cc_rad_1;
        qreal radius2 = GlobalValue::gra_cc_rad_2;
        item = new BSyncytia(BGraphicsItem::ItemType::Concentric_Circle, radius2, radius2, text);
        item->setRadius(radius1);
    } break;
    case BGraphicsItem::ItemType::Rectangle: {
        qreal width = GlobalValue::gra_r_wid;
        qreal height = GlobalValue::gra_r_hei;
        item = new BSyncytia(BGraphicsItem::ItemType::Rectangle, width, height, text);
    } break;
    case BGraphicsItem::ItemType::Square: {
        qreal length = GlobalValue::gra_s_len;
        item = new BSyncytia(BGraphicsItem::ItemType::Square, length, length, text);
    } break;
    case BGraphicsItem::ItemType::Polygon: {
        is_creating_BPolygon = true;
        item = new BSyncytia(BGraphicsItem::ItemType::Polygon, 0, 0, text);
    } break;
    case BGraphicsItem::ItemType::Pill: {
        qreal width = GlobalValue::gra_p_wid;
        qreal height = GlobalValue::gra_p_hei;
        item = new BSyncytia(BGraphicsItem::ItemType::Pill, width, height, text);
    } break;
    case BGraphicsItem::ItemType::Chamfer: {
        qreal width = GlobalValue::gra_ch_wid;
        qreal height = GlobalValue::gra_ch_hei;
        qreal radius = GlobalValue::gra_ch_rad;
        item = new BSyncytia(BGraphicsItem::ItemType::Chamfer, width, height, text);
        item->setRadius(radius);
    } break;
    default: break;
    }

    connect(item, &BSyncytia::startCreatePolygon, [this](QString text){
        m_list.clear();
        is_creating_BPolygon = true;
        current_polygon = text;
    });
    connect(this, SIGNAL(updatePoint(QString, QList<QPointF>, bool)), item, SLOT(updatePolygon(QString, QList<QPointF>, bool)));

    item->setEnable(!is_locked);
    this->addItem(item);
    this->m_graphicsItemList.push_back(item);
}

QList<BGraphicsItem *> BQGraphicsScene::getGraphicsItemList()
{
    return m_graphicsItemList;
}

QGraphicsPixmapItem& BQGraphicsScene::getPixmapItem()
{
    return m_pixmapItem;
}

bool BQGraphicsScene::getLiveStatus()
{
    return is_alive;
}

void BQGraphicsScene::updateFrame(QImage image)
{
    GlobalFun::overExposure(image);

    m_pixmapItem.setPixmap(QPixmap::fromImage(image));
    m_pixmapItem.setPos((-1)*image.width()/2, (-1)*image.height()/2);
}

void BQGraphicsScene::updateLiveMode(bool status, std::vector<cv::Mat> &list)
{
//    is_alive = status;

//    if ( status ) {
//        // live
//        m_unlivePixmap.setZValue(-1);
//        m_pixmapItem.setZValue(0);
//        m_mask->setParentItem(&m_pixmapItem);
//        m_simpleText.setText("");
//    } else {
//        // unlive
//        m_matList.clear();
//        for ( auto &temp : list ) { m_matList.push_back(temp); }

//        QImage image = GlobalFun::convertMatToQImage(list.at(0));
//        m_unlivePixmap.setPixmap(QPixmap::fromImage(image));
//        m_unlivePixmap.setPos((-1)*image.width()/2, (-1)*image.height()/2);
//        m_unlivePixmap.setZValue(0);
//        m_pixmapItem.setZValue(-1);
//        m_mask->setParentItem(&m_unlivePixmap);
//        m_simpleText.setText(QString("1/%1").arg(list.size()));
//    }


//换上了Pro版本就没有多余的黑色边缘了
    is_alive = status;
    if ( status ) {
        // live
        m_pixmapItem.setVisible(true);
        m_mask->setParentItem(&m_pixmapItem);

        m_unlivePixmap.setPixmap(QPixmap::fromImage(QImage()));
        m_simpleText.setText("");
    } else {
        // unlive
        m_matList.clear();
        for ( auto &temp : list ) { m_matList.push_back(temp); }
        qDebug()<<"mat type"<<list.at(0).type();
        QImage image = GlobalFun::convertMatToQImage(list.at(0));
        m_unlivePixmap.setPixmap(QPixmap::fromImage(image));
        m_unlivePixmap.setPos((-1)*image.width()/2, (-1)*image.height()/2);
        m_simpleText.setText(QString("1/%1").arg(list.size()));
        m_pixmapItem.setVisible(false);
        m_mask->setParentItem(&m_unlivePixmap);
    }
}

void BQGraphicsScene::saveItemToConfig(QString file)
{
    GlobalFun::removeFile(file);
    QSettings item(file, QSettings::IniFormat);

    item.beginWriteArray("itemList");
    for ( int i = 0; i < m_graphicsItemList.size(); ++i ) {
        item.setArrayIndex(i);

        if ( GlobalValue::com_tp == 1 )
        {
            BGraphicsItem::ItemType type = m_graphicsItemList.at(i)->getType();
            item.setValue("type", (int)type);
            item.setValue("rotation", m_graphicsItemList.at(i)->getRotation());

            switch (type)
            {
            case BGraphicsItem::ItemType::Chamfer:
            case BGraphicsItem::ItemType::Concentric_Circle: {
                item.setValue("radius", m_graphicsItemList.at(i)->getRadius());
            }
            case BGraphicsItem::ItemType::Circle:
            case BGraphicsItem::ItemType::Ellipse:
            case BGraphicsItem::ItemType::Rectangle:
            case BGraphicsItem::ItemType::Square:
            case BGraphicsItem::ItemType::Pill: {
                item.setValue("width", m_graphicsItemList.at(i)->getWidth());
                item.setValue("height", m_graphicsItemList.at(i)->getHeight());
                item.setValue("center", m_graphicsItemList.at(i)->getCurrentCenter());
            } break;
            case BGraphicsItem::ItemType::Polygon: {
                QList<QPointF> list = m_graphicsItemList.at(i)->getPointList();
                item.beginWriteArray("polygon");
                for (int j = 0; j < list.size(); j++)
                {
                    item.setArrayIndex(j);
                    item.setValue("edge", m_graphicsItemList.at(i)->mapToScene( list.at(j) ));
                }
                item.endArray();
            } break;
            default: break;
            }
        }
        else
        {
            item.setValue("width", m_graphicsItemList.at(i)->getWidth());
            item.setValue("height", m_graphicsItemList.at(i)->getHeight());
            item.setValue("center", m_graphicsItemList.at(i)->getCurrentCenter());

            item.setValue("type", (int)m_graphicsItemList.at(i)->getAutoType());
            item.setValue("useScale", m_graphicsItemList.at(i)->getIsUseScale());
            item.setValue("autoWidth", m_graphicsItemList.at(i)->getAutoWidth());
            item.setValue("autoHeight", m_graphicsItemList.at(i)->getAutoHeight());
            item.setValue("cirRadius", m_graphicsItemList.at(i)->getAutoCirRadius());
            item.setValue("chaRadius", m_graphicsItemList.at(i)->getAutoChaRadius());
            item.setValue("dis1", m_graphicsItemList.at(i)->getAutoDis1());
            item.setValue("dis2", m_graphicsItemList.at(i)->getAutoDis2());
            item.setValue("scale", m_graphicsItemList.at(i)->getAutoScale());
        }
    }
    item.endArray();
}

void BQGraphicsScene::loadItemToScene(QString file,bool isFirst)
{ 
    QSettings item(file, QSettings::IniFormat);
    int size = item.beginReadArray("itemList");
    if( !isFirst&& size!=GlobalValue::com_h_num){
        if(size>GlobalValue::com_h_num){//add
            for (int i=GlobalValue::com_h_num;i<size;++i) {
                    if ( GlobalValue::com_tp == 1 ) {
                        createBGraphicsItem(BGraphicsItem::ItemType::Circle, QString("#%1").arg(i+1));
                    } else {
                        createBGraphicsItem(BGraphicsItem::ItemType::Square, QString("#%1").arg(i+1));
                    }
                    emit createConfig(i);
            }
        }else{//reduce
            for (int i=GlobalValue::com_h_num;i>size;--i) {
                this->removeItem(m_graphicsItemList.at(i-1));
                delete m_graphicsItemList.at(i-1);
                m_graphicsItemList.pop_back();
                emit removeConfig(i);
            }
        }
        if ( GlobalValue::com_h_num != size) {
            GlobalValue::com_h_num = size;
            emit updateTableView(size);
        }
    }
    for (int i = 0; i < size; ++i) {
        item.setArrayIndex(i);

        if ( GlobalValue::com_tp == 1 )
        {
            BGraphicsItem::ItemType type = (BGraphicsItem::ItemType)item.value("type").toInt();
            m_graphicsItemList.at(i)->changeType(type);

            qreal rotation = item.value("rotation").toDouble();
            m_graphicsItemList.at(i)->setMyRotation(rotation);

            switch (type)
            {
            case BGraphicsItem::ItemType::Chamfer:
            case BGraphicsItem::ItemType::Concentric_Circle: {
                qreal radius = item.value("radius").toDouble();
                m_graphicsItemList.at(i)->setRadius(radius);
            }
            case BGraphicsItem::ItemType::Circle:
            case BGraphicsItem::ItemType::Ellipse:
            case BGraphicsItem::ItemType::Rectangle:
            case BGraphicsItem::ItemType::Square:
            case BGraphicsItem::ItemType::Pill: {
                qreal width = item.value("width").toDouble();
                m_graphicsItemList.at(i)->setWidth(width);

                qreal height = item.value("height").toDouble();
                m_graphicsItemList.at(i)->setHeight(height);

                QPointF center = item.value("center").toPointF();
                m_graphicsItemList.at(i)->setPos(center);
            } break;
            case BGraphicsItem::ItemType::Polygon: {
                int count = item.beginReadArray("polygon");
                QList<QPointF> list;
                for (int j = 0; j < count; ++j)
                {
                    item.setArrayIndex(j);
                    QPointF edge = m_graphicsItemList.at(i)->mapFromScene( item.value("edge").toPointF() );
                    list.append(edge);
                    emit updatePoint(current_polygon, list, false);
                }
                item.endArray();

                emit updatePoint(current_polygon, list, true);
                is_creating_BPolygon = false;
                is_ignore_contextMenu = false;
            } break;
            default: break;
            }
        }
        else
        {
            qreal width = item.value("width").toDouble();
            m_graphicsItemList.at(i)->setWidth(width);

            qreal height = item.value("height").toDouble();
            m_graphicsItemList.at(i)->setHeight(height);

            QPointF center = item.value("center").toPointF();
            m_graphicsItemList.at(i)->setPos(center);

            BGraphicsItem::AutoType type = (BGraphicsItem::AutoType)item.value("type").toInt();
            m_graphicsItemList.at(i)->setAutoType(type);

            bool useScale = item.value("useScale").toBool();
            m_graphicsItemList.at(i)->setIsUseScale(useScale);

            qreal autoWidth = item.value("autoWidth").toDouble();
            m_graphicsItemList.at(i)->setAutoWidth(autoWidth);

            qreal autoHeight = item.value("autoHeight").toDouble();
            m_graphicsItemList.at(i)->setAutoHeight(autoHeight);

            qreal cirRadius = item.value("cirRadius").toDouble();
            m_graphicsItemList.at(i)->setAutoCirRadius(cirRadius);

            qreal chaRadius = item.value("chaRadius").toDouble();
            m_graphicsItemList.at(i)->setAutoChaRadius(chaRadius);

            qreal dis1 = item.value("dis1").toDouble();
            m_graphicsItemList.at(i)->setAutoDis1(dis1);

            qreal dis2 = item.value("dis2").toDouble();
            m_graphicsItemList.at(i)->setAutoDis2(dis2);

            qreal scale = item.value("scale").toDouble();
            m_graphicsItemList.at(i)->setAutoScale(scale);
        }

        m_graphicsItemList.at(i)->updateScreen();
    }
    item.endArray();
}

void BQGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (is_creating_BPolygon) {
        QPointF p(event->scenePos().x(), event->scenePos().y());

        switch ( event->buttons() )
        {
        case Qt::LeftButton: {
            m_list.push_back(p);
            emit updatePoint(current_polygon, m_list, false);
        } break;
        case Qt::RightButton: {
            if (m_list.size() >= 3) {
                emit updatePoint(current_polygon, m_list, true);
                is_creating_BPolygon = false;
                is_ignore_contextMenu = true;
                m_list.clear();
            }
        } break;
        default: break;
        }
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}

void BQGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int index = m_simpleText.text().left(1).toInt();
    int size = m_matList.size();

    if ( event->delta() > 0 && index < size ) {
        index++;
        QImage image = GlobalFun::convertMatToQImage(m_matList.at(index-1));
        m_unlivePixmap.setPixmap(QPixmap::fromImage(image));
        m_unlivePixmap.setPos((-1)*image.width()/2, (-1)*image.height()/2);
        m_simpleText.setText(QString("%1/%2").arg(index).arg(m_matList.size()));
    } else if ( event->delta() < 0 && index > 1 ) {
        index--;
        QImage image = GlobalFun::convertMatToQImage(m_matList.at(index-1));
        m_unlivePixmap.setPixmap(QPixmap::fromImage(image));
        m_unlivePixmap.setPos((-1)*image.width()/2, (-1)*image.height()/2);
        m_simpleText.setText(QString("%1/%2").arg(index).arg(m_matList.size()));
    }

    QGraphicsScene::wheelEvent(event);
}

void BQGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    bool jud = false;
    for (auto &temp : m_graphicsItemList)
    {
        if ( temp->getIfSelected() ) {
            jud = true;
        }
    }

    if ( !jud && !is_ignore_contextMenu && !is_creating_BPolygon ) {
        QMenu* menu = new QMenu();
        QVBoxLayout *layout = new QVBoxLayout(menu);

        QCheckBox *lock = new QCheckBox(GlobalString::graphics_lock, menu);
        lock->setChecked(is_locked);
        connect(lock, &QCheckBox::stateChanged, [=](int state) {
            is_locked = state == 2 ? true : false;

            for ( int i = 0; i < m_graphicsItemList.size(); ++i )
            {
                m_graphicsItemList.at(i)->setEnable(!is_locked);
            }

            menu->close();
        });

        QCheckBox* live = new QCheckBox(GlobalString::graphics_live, menu);
        live->setEnabled(!is_locked);
        live->setChecked(is_alive);
        connect(live, &QCheckBox::stateChanged, [=](int state) {
            changeLiveMode(state == 2);//点击实时模式state为1
            menu->close();
        });

        QCheckBox* mask = new QCheckBox(GlobalString::graphics_showMask, menu);
        mask->setEnabled(!is_locked);
        mask->setChecked(m_mask->isVisible());
        connect(mask, &QCheckBox::stateChanged, [=](int state) {
            m_mask->setVisible(state == 2);
            menu->close();
        });

        QRadioButton *manual_mask = new QRadioButton(GlobalString::graphics_manual);
        QRadioButton *auto_mask = new QRadioButton(GlobalString::graphics_auto);
        if ( GlobalValue::com_tp == 1 ) {
            manual_mask->setChecked(true);
        } else {
            auto_mask->setChecked(true);
        }

        QHBoxLayout *hbox1 = new QHBoxLayout;
        hbox1->addWidget(manual_mask);
        hbox1->addWidget(auto_mask);
        hbox1->addStretch(1);
        QGroupBox *box = new QGroupBox(menu);
        box->setEnabled(!is_locked);
        box->setLayout(hbox1);

        connect(manual_mask, &QRadioButton::clicked, [&](){
            for ( int i = 0; i < m_graphicsItemList.size(); ++i )
            {
                m_graphicsItemList.at(i)->changeType(BGraphicsItem::ItemType::Circle);
            }

            GlobalValue::com_tp = 1;
            menu->close();
        });

        connect(auto_mask, &QRadioButton::clicked, [&](){
            for ( int i = 0; i < m_graphicsItemList.size(); ++i )
            {
                m_graphicsItemList.at(i)->changeType(BGraphicsItem::ItemType::Square);
            }

            GlobalValue::com_tp = 2;
            menu->close();
        });

        QPushButton *resetBtn = new QPushButton(GlobalString::graphics_reset, menu);
        resetBtn->setEnabled(!is_locked);
        connect(resetBtn, &QPushButton::clicked, [&](){
            for ( int i = 0; i < m_graphicsItemList.size(); ++i )
            {
                m_graphicsItemList.at(i)->changeType(BGraphicsItem::ItemType::Circle);
                m_graphicsItemList.at(i)->resetAutoData();
                m_graphicsItemList.at(i)->setPos(0, 0);
            }

            emit resetView();

            GlobalValue::com_tp = 1;
            menu->close();
        });

        QLabel *label = new QLabel(GlobalString::graphics_hole_num + ": ",  menu);
        QSpinBox *hole = new QSpinBox(menu);
        hole->setRange(1, 100);
        hole->setSingleStep(1);
        hole->setValue(GlobalValue::com_h_num);
        hole->setEnabled(!is_locked);
        QPushButton *sureBtn = new QPushButton(GlobalString::graphics_sure, menu);
        sureBtn->setEnabled(!is_locked);
        QHBoxLayout *hbox2 = new QHBoxLayout;
        hbox2->addWidget(label);
        hbox2->addWidget(hole);
        hbox2->addWidget(sureBtn);

        connect(sureBtn, &QPushButton::clicked, [&](){
            if ( hole->text().toInt() > m_graphicsItemList.size() ) {
                // add
                for ( int i = m_graphicsItemList.size() + 1; i <= hole->text().toInt(); ++i )
                {
                    if ( GlobalValue::com_tp == 1 ) {
                        createBGraphicsItem(BGraphicsItem::ItemType::Circle, QString("#%1").arg(i));
                    } else {
                        createBGraphicsItem(BGraphicsItem::ItemType::Square, QString("#%1").arg(i));
                    }

                    emit createConfig(i);
                }
            } else {
                // reduce
                for ( int i = m_graphicsItemList.size(); i > hole->text().toInt(); --i )
                {
                    delete m_graphicsItemList.at(i-1);
                    m_graphicsItemList.pop_back();

                    emit removeConfig(i);
                }
            }

            if ( GlobalValue::com_h_num != hole->text().toInt() ) {
                GlobalValue::com_h_num = hole->text().toInt();
                emit updateTableView(hole->text().toInt());
            }

            menu->close();
        });

        layout->addWidget(lock);
        layout->addWidget(live);
        layout->addWidget(mask);
        layout->addWidget(box);
        layout->addWidget(resetBtn);
        layout->addLayout(hbox2);
        menu->setLayout(layout);

        menu->exec(QCursor::pos());
        delete menu;
    }

    if (is_ignore_contextMenu) {
        is_ignore_contextMenu = false;
    }

    QGraphicsScene::contextMenuEvent(event);
}

