#ifndef GRAPHICSVEXITEM_H
#define GRAPHICSVEXITEM_H

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

class MyGraphicsVexItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
private:
    QBrush regBrush = QBrush(QColor(108,166,205));
    QBrush visitedBrush = QBrush(QColor(162,205,90));
    QFont nameFont = QFont("Corbel", 13, QFont::Normal, true);
    QTimeLine* curAnimation = nullptr;
    void startAnimation();

public:
    QPointF center;
    qreal radius;
    QVector<MyGraphicsVexItem*> nexts;
    MyGraphicsVexItem *left = nullptr;
    MyGraphicsVexItem *right = nullptr;
    QGraphicsSimpleTextItem *nameTag = nullptr;
    QString nameText = "";
    void setName(QString s);
    MyGraphicsVexItem(QPointF _center, double _r, int nameID = 0, QGraphicsItem *parent = nullptr);
    MyGraphicsVexItem(QPointF _center, double _r=10, QGraphicsItem *parent = nullptr);
    void showAnimation();
    QTimeLine* visit();
};

#endif // GRAPHICSVEXITEM_H
