#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

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
#include <QDebug> // 用于输出调试信息

class MyGraphicsView;
class MyGraphicsVexItem;
class MyGraphicsLineItem;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT

private:
    QGraphicsScene* myGraphicsScene;
    QBrush regBrush = QBrush(QColor(108,166,205));

    int vexID = 0;
    bool isCreating = false; // 手动创建模式标记
    MyGraphicsVexItem *strtVex = nullptr;
    QGraphicsItem *sketchItem = nullptr;

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    MyGraphicsVexItem* addVex(QPointF center, qreal radius = 10);
    MyGraphicsLineItem* addLine(MyGraphicsVexItem *start, MyGraphicsVexItem *end);
    void clearSketch();
    void sketchLine(QPointF start, QPointF end);

    /* Animation loop */
    QQueue<QTimeLine*> aniQueue;
    bool onAni = false;
    QTimeLine *curAni = nullptr;
    qreal speedRate = 1.0; // 动画速度
    void nextAni();
    void addAnimation(QTimeLine *ani);

    // For morris
    QTimeLine* changeName(QString s,MyGraphicsVexItem * head);

    QVector<MyGraphicsVexItem*> vexes;
    QVector<MyGraphicsVexItem*> preVexes;
    QVector<MyGraphicsVexItem*> leaves;
    QVector<MyGraphicsVexItem*> halfLeaves;
    QVector<MyGraphicsVexItem*> nullVexes;
    QVector<MyGraphicsLineItem*> leafLines;

    // 递归辅助函数
    void preRecHelper(MyGraphicsVexItem* node);
    void inRecHelper(MyGraphicsVexItem* node);
    void posRecHelper(MyGraphicsVexItem* node);

public:
    MyGraphicsView();
    MyGraphicsVexItem * root;

    void init(); // 清空并重置
    void resetAllNodeStates();  //仅重置

    // --- 新增/修改的功能 ---
    void autoCreateTree(int n); // 自动生成完全二叉树
    void fitTreeInView();

    // 遍历入口
    void pre(MyGraphicsVexItem * head);        // 非递归
    void preRecursive(MyGraphicsVexItem* head);// 递归

    void in(MyGraphicsVexItem * head);         // 非递归
    void inRecursive(MyGraphicsVexItem* head); // 递归

    void pos(MyGraphicsVexItem * head);        // 非递归
    void posRecursive(MyGraphicsVexItem* head);// 递归

    void levelOrder(MyGraphicsVexItem* head);  // 层序遍历 (非递归)

    void morris(MyGraphicsVexItem * head);     // Morris

signals:
    // 发送统计数据给 MainWindow 显示
    void reportStats(QString desc);
};

// ... MyGraphicsVexItem 和 MyGraphicsLineItem 类保持不变 (除非你想加 moveTo) ...
// 建议在 MyGraphicsVexItem 中添加一个 updatePos 方法方便移动
// 但为了简化，下面的cpp实现中我会直接删掉重建，这样就不需要改 Item 类了。

class MyGraphicsVexItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    // ... 保持原有代码不变 ...
    // ... 可以在 public 里加: void moveTo(QPointF p); ...
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
    MyGraphicsVexItem(QPointF _center, qreal _r, int nameID = 0, QGraphicsItem *parent = nullptr);
    MyGraphicsVexItem(QPointF _center, qreal _r=10, QGraphicsItem *parent = nullptr);
    void showAnimation();
    QTimeLine* visit();
};

class MyGraphicsLineItem : public QObject, public QGraphicsLineItem{
    Q_OBJECT
private:
    qreal lineWidth = 3;
    Qt::PenStyle lineStyle = Qt::SolidLine;
    Qt::PenCapStyle capStyle = Qt::RoundCap;
    QColor defaultColor = QColor(159,182,205);
    QPen defaultPen;
    MyGraphicsVexItem *startVex;
    MyGraphicsVexItem *endVex;
public:
    MyGraphicsLineItem(MyGraphicsVexItem *start, MyGraphicsVexItem *end, QGraphicsItem *parent = nullptr);
};

#endif // GRAPHVIEW_H
