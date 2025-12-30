#ifndef GRAPHICSLINEITEM_H
#define GRAPHICSLINEITEM_H

#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QTimeLine>
#include <QVector>
#include <QStack>
#include <QQueue>
#include <QDebug>
#include <graphicsVexItem.h>

class MyGraphicsLineItem : public QObject, public QGraphicsLineItem{
    Q_OBJECT
private:
    qreal lineWidth = 3;
    Qt::PenStyle lineStyle = Qt::SolidLine;
    Qt::PenCapStyle capStyle = Qt::RoundCap;
    QColor defaultColor = QColor(159,182,205);
    QPen defaultPen;
public:
    MyGraphicsVexItem *startVex;
    MyGraphicsVexItem *endVex;
    MyGraphicsLineItem(MyGraphicsVexItem *start, MyGraphicsVexItem *end, QGraphicsItem *parent = nullptr);
};

#endif // GRAPHICSLINEITEM_H
