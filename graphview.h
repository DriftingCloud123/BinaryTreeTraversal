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
#include <QDebug>
#include <graphicsVexItem.h>

class MyGraphicsView;
class MyGraphicsLineItem;

//类1：MyGraphicsVexItem
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
    // 根据坐标分配并重新排序孩子节点
    bool assignAndReorderChildren(MyGraphicsVexItem* parent,
                                  MyGraphicsVexItem* newChild,
                                  const QPointF& newChildPos);

    // 当父节点只有一个孩子时重新分配
    bool rearrangeSingleChild(MyGraphicsVexItem* parent,
                              MyGraphicsVexItem* newChild,
                              const QPointF& newChildPos);

    // 处理分配失败的情况
    void handleFailedAssignment(MyGraphicsVexItem* failedVex);

    // 重新创建父节点与孩子的连接线
    void recreateParentChildLines(MyGraphicsVexItem* parent);

    // 验证左右孩子位置是否正确
    bool validateChildPositions(MyGraphicsVexItem* parent);



    MyGraphicsVexItem* addVex(QPointF center, qreal radius = 10);
    MyGraphicsLineItem* addLine(MyGraphicsVexItem *start, MyGraphicsVexItem *end);
    void clearSketch();
    void sketchLine(QPointF start, QPointF end);

    /* Animation loop */
    QQueue<QTimeLine*> aniQueue;
    bool onAni = false;
    QTimeLine *curAni = nullptr;
    qreal speedRate = 2.0; // 动画速度
    void nextAni();
    void addAnimation(QTimeLine *ani);

    QVector<MyGraphicsVexItem*> vexes;
    // QVector<MyGraphicsVexItem*> preVexes;
    // QVector<MyGraphicsVexItem*> leaves;
    // QVector<MyGraphicsVexItem*> halfLeaves;
    // QVector<MyGraphicsVexItem*> nullVexes;
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

signals:
    // 发送统计数据给 MainWindow 显示
    void reportStats(QString desc);
};



//类3：MyGraphicsLineItem


#endif // GRAPHVIEW_H
