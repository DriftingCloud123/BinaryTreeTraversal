#include "graphview.h"
#include <QDebug>

// ================================================================
// MyGraphicsView 类实现
// ================================================================

MyGraphicsView::MyGraphicsView()
{
    this->setMouseTracking(true);
    this->setRenderHint(QPainter::Antialiasing);
    this->setStyleSheet("padding:0px;border:0px");

    myGraphicsScene = new QGraphicsScene();
    myGraphicsScene->setBackgroundBrush(Qt::transparent);
    myGraphicsScene->setSceneRect(0,0,1183,875);

    this->setScene(myGraphicsScene);
    this->setFixedSize(1183,875);
    this->move(-5,-45);
    this->show();

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 初始化根节点
    init();
}

void MyGraphicsView::init()
{
    // 停止并清空动画队列
    aniQueue.clear();
    onAni = false;
    if(curAni) {
        curAni->stop();
        delete curAni;
        curAni = nullptr;
    }

    // 清空场景中的所有物品
    if(myGraphicsScene) {
        myGraphicsScene->clear();
    }

    // 重置变量
    vexID = 0;
    isCreating = false;
    vexes.clear();
    preVexes.clear();
    leaves.clear();
    halfLeaves.clear();
    nullVexes.clear();
    leafLines.clear();
    strtVex = nullptr;
    sketchItem = nullptr;

    // 重建根节点 (V0)
    root = new MyGraphicsVexItem(QPointF(590, 150), 10, vexID++);
    myGraphicsScene->addItem(root);
    myGraphicsScene->addItem(root->nameTag);
    vexes.push_back(root);
}

// --- 自动生成树 (Tab 1 核心功能) ---
void MyGraphicsView::autoCreateTree(int n)
{
    // 每次生成前先重置
    init();

    if (n <= 1) return; // V0 已经有了

    // 重新设置根节点位置
    qreal startX = 590;
    qreal startY = 100;

    // 更新 V0
    vexes[0]->center = QPointF(startX, startY);
    vexes[0]->setRect(startX-20, startY-20, 40, 40);
    // 更新 V0 的名字标签位置
    delete vexes[0]->nameTag;
    vexes[0]->setName(vexes[0]->nameText);

    // 生成剩余节点 (从 V1 开始)
    for(int i = 1; i < n; i++) {
        int parentIdx = (i - 1) / 2;
        // 保护机制
        if(parentIdx >= vexes.size()) break;

        MyGraphicsVexItem* parent = vexes[parentIdx];

        // 计算层级和偏移 (简单的完全二叉树布局算法)
        int level = 0;
        int tmp = i + 1;
        while(tmp >>= 1) level++;

        qreal xOffset = 600.0 / (1 << level); // 层级越深，偏移越小
        qreal yDist = 100;

        bool isLeft = (i % 2 != 0);
        qreal newX = parent->center.x() + (isLeft ? -xOffset : xOffset);
        qreal newY = startY + level * yDist;

        // 创建新节点并连接
        MyGraphicsVexItem* newVex = addVex(QPointF(newX, newY));
        parent->nexts.push_back(newVex);
        addLine(parent, newVex);

        if(isLeft) parent->left = newVex;
        else parent->right = newVex;
    }
    // 强制刷新场景
    myGraphicsScene->update();
}

// --- 鼠标交互 (手动建树) ---
void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if(isCreating){
        clearSketch();
        MyGraphicsVexItem* endVex = addVex(event->pos());
        if(strtVex) {
            strtVex->nexts.push_back(endVex);
            // 简单判断左右：如果在左边就是左孩子
            if(endVex->center.x() < strtVex->center.x()) strtVex->left = endVex;
            else strtVex->right = endVex;

            addLine(strtVex, endVex);
        }
        isCreating = !isCreating;
    } else {
        bool flag = false;
        for(int i=0; i<vexes.size(); i++) {
            // 简单的点击碰撞检测
            QPointF p = event->pos();
            QPointF c = vexes[i]->center;
            if(p.x() >= c.x()-20 && p.x() <= c.x()+20 &&
                p.y() >= c.y()-20 && p.y() <= c.y()+20)
            {
                flag = true;
                strtVex = vexes[i];
            }
        }
        if(flag){
            isCreating = !isCreating;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event){
    if(isCreating && strtVex){
        clearSketch();
        sketchLine(strtVex->pos() + strtVex->rect().center(), event->pos());
    }
    QGraphicsView::mouseMoveEvent(event);
}

MyGraphicsVexItem* MyGraphicsView::addVex(QPointF center, qreal radius)
{
    MyGraphicsVexItem *newVex = new MyGraphicsVexItem(center, radius, vexID++);
    myGraphicsScene->addItem(newVex);
    myGraphicsScene->addItem(newVex->nameTag);
    newVex->showAnimation();
    vexes.push_back(newVex);
    return newVex;
}

MyGraphicsLineItem* MyGraphicsView::addLine(MyGraphicsVexItem *start, MyGraphicsVexItem *end)
{
    MyGraphicsLineItem * line = new MyGraphicsLineItem(start, end);
    myGraphicsScene->addItem(line);
    return line;
}

void MyGraphicsView::sketchLine(QPointF start, QPointF end){
    QGraphicsLineItem *newLine = new QGraphicsLineItem(start.x(), start.y(), end.x(), end.y());
    QPen pen;
    pen.setWidth(3);
    pen.setStyle(Qt::DashLine);
    pen.setBrush(QColor(58, 143, 192, 100));
    pen.setCapStyle(Qt::RoundCap);
    newLine->setPen(pen);
    scene()->addItem(newLine);
    sketchItem = newLine;
}

void MyGraphicsView::clearSketch(){
    if(sketchItem != nullptr){
        scene()->removeItem(sketchItem);
        delete sketchItem;
        sketchItem = nullptr;
    }
}

// --- 动画系统 ---
void MyGraphicsView::addAnimation(QTimeLine *ani){
    aniQueue.push_back(ani);
    if(!onAni){
        onAni = true;
        nextAni();
    }
}

void MyGraphicsView::nextAni(){
    if(aniQueue.size() > 0){
        QTimeLine *next = aniQueue.front();
        curAni = next;
        aniQueue.pop_front();
        connect(next, &QTimeLine::finished, this, [=](){
            nextAni();
            next->deleteLater();
        });
        next->setDuration(next->duration() / speedRate);
        next->start();
    } else {
        onAni = false;
        curAni = nullptr;
    }
}

// --- 遍历算法实现 ---

// 1. 先序非递归
void MyGraphicsView::pre(MyGraphicsVexItem * head)
{
    if(head == nullptr) return;
    QStack<MyGraphicsVexItem*> s;
    s.push(head);
    int maxStack = 0;

    while(!s.empty()) {
        if(s.size() > maxStack) maxStack = s.size();
        head = s.top();
        s.pop();
        addAnimation(head->visit());
        if(head->right) s.push(head->right);
        if(head->left) s.push(head->left);
    }
    emit reportStats(QString("先序非递归 | 最大栈深: %1").arg(maxStack));
}

// 1.1 先序递归
void MyGraphicsView::preRecursive(MyGraphicsVexItem* head) {
    preRecHelper(head);
    emit reportStats("先序递归模式 (动画演示中)");
}
void MyGraphicsView::preRecHelper(MyGraphicsVexItem* node) {
    if(!node) return;
    addAnimation(node->visit());
    preRecHelper(node->left);
    preRecHelper(node->right);
}

// 2. 中序非递归
void MyGraphicsView::in(MyGraphicsVexItem * head)
{
    if(head == nullptr) return;
    QStack<MyGraphicsVexItem*> s;
    int maxStack = 0;
    while(!s.empty() || head!=nullptr) {
        if(s.size() > maxStack) maxStack = s.size();
        if(head!=nullptr) {
            s.push(head);
            head = head->left;
        } else {
            head = s.top();
            s.pop();
            addAnimation(head->visit());
            head = head->right;
        }
    }
    emit reportStats(QString("中序非递归 | 最大栈深: %1").arg(maxStack));
}

// 2.1 中序递归
void MyGraphicsView::inRecursive(MyGraphicsVexItem* head) {
    inRecHelper(head);
    emit reportStats("中序递归模式 (动画演示中)");
}
void MyGraphicsView::inRecHelper(MyGraphicsVexItem* node) {
    if(!node) return;
    inRecHelper(node->left);
    addAnimation(node->visit());
    inRecHelper(node->right);
}

// 3. 后序非递归
void MyGraphicsView::pos(MyGraphicsVexItem * head)
{
    if(head == nullptr) return;
    QStack<MyGraphicsVexItem*> s;
    QStack<MyGraphicsVexItem*> col;
    s.push(head);
    int maxStack = 0;
    while(!s.empty()) {
        if(s.size() > maxStack) maxStack = s.size();
        head = s.top();
        s.pop();
        col.push(head);
        if(head->left) s.push(head->left);
        if(head->right) s.push(head->right);
    }
    while(!col.empty()) {
        addAnimation(col.top()->visit());
        col.pop();
    }
    emit reportStats(QString("后序非递归 | 最大栈深: %1 (双栈法)").arg(maxStack));
}

// 3.1 后序递归
void MyGraphicsView::posRecursive(MyGraphicsVexItem* head) {
    posRecHelper(head);
    emit reportStats("后序递归模式 (动画演示中)");
}
void MyGraphicsView::posRecHelper(MyGraphicsVexItem* node) {
    if(!node) return;
    posRecHelper(node->left);
    posRecHelper(node->right);
    addAnimation(node->visit());
}

// 4. 层序遍历
void MyGraphicsView::levelOrder(MyGraphicsVexItem* head)
{
    if(head == nullptr) return;
    QQueue<MyGraphicsVexItem*> q;
    q.enqueue(head);
    int maxQ = 0;
    while(!q.empty()) {
        if(q.size() > maxQ) maxQ = q.size();
        MyGraphicsVexItem* cur = q.dequeue();
        addAnimation(cur->visit());
        if(cur->left) q.enqueue(cur->left);
        if(cur->right) q.enqueue(cur->right);
    }
    emit reportStats(QString("层序遍历 | 最大队列长度: %1").arg(maxQ));
}

// Morris 占位 (防止链接错误)
void MyGraphicsView::morris(MyGraphicsVexItem * head) {
    // 暂未实现完整逻辑，防止报错
    Q_UNUSED(head);
}
QTimeLine* MyGraphicsView::changeName(QString s, MyGraphicsVexItem * head) {
    Q_UNUSED(s); Q_UNUSED(head);
    return nullptr;
}


// ================================================================
// MyGraphicsVexItem 类实现 (节点)
// ================================================================

MyGraphicsVexItem::MyGraphicsVexItem(QPointF _center, qreal _r, int nameID, QGraphicsItem *parent) :
    QGraphicsEllipseItem(_center.x()-20, _center.y()-20, 40, 40, parent),
    center(_center),
    radius(_r)
{
    nameText = QString::asprintf("V%d", nameID);
    setName(nameText);
    this->setPen(Qt::NoPen);
    this->setBrush(regBrush);
}

MyGraphicsVexItem::MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent) :
    QGraphicsEllipseItem(_center.x()-20, _center.y()-20, 40, 40, parent),
    center(_center),
    radius(_r)
{
    nameText = "nullptr";
    setName(nameText);
    QPen pen(QColor(108,166,205));
    pen.setStyle(Qt::DashLine);
    pen.setWidth(3);
    this->setPen(pen);
    this->setBrush(QColor(255,255,255));
}

void MyGraphicsVexItem::setName(QString s)
{
    nameText = s;
    if(nameTag) {
        delete nameTag;
        nameTag = nullptr;
    }
    nameTag = new QGraphicsSimpleTextItem();
    nameTag->setPos(this->center + QPointF(10, - 10 - QFontMetrics(nameFont).height()));
    nameTag->setFont(nameFont);
    nameTag->setText(nameText);
    nameTag->setZValue(this->zValue());
    nameTag->setBrush(Qt::black);
    nameTag->setFlags(QGraphicsItem::ItemIsSelectable);
}

QTimeLine* MyGraphicsVexItem::visit()
{
    QTimeLine *timeLine = new QTimeLine(500, this);
    timeLine->setFrameRange(0, 200);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = 26;
    qreal difRadius = -6;
    connect(timeLine, &QTimeLine::frameChanged, timeLine, [=](int frame){
        this->setBrush(visitedBrush);
        qreal curProgress = curve.valueForProgress(frame / 200.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    });
    return timeLine;
}

void MyGraphicsVexItem::showAnimation(){
    QTimeLine *timeLine = new QTimeLine(500, this);
    timeLine->setFrameRange(0, 200);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = 26;
    qreal difRadius = -6;
    connect(timeLine, &QTimeLine::frameChanged, timeLine, [=](int frame){
        qreal curProgress = curve.valueForProgress(frame / 200.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    });
    curAnimation = timeLine;
    startAnimation();
}

void MyGraphicsVexItem::startAnimation(){
    if(curAnimation != nullptr){
        curAnimation->start();
    }
}


// ================================================================
// MyGraphicsLineItem 类实现 (连线)
// ================================================================

MyGraphicsLineItem::MyGraphicsLineItem(MyGraphicsVexItem *start, MyGraphicsVexItem *end, QGraphicsItem *parent) :
    QGraphicsLineItem(parent),
    startVex(start),
    endVex(end)
{
    defaultPen.setWidth(lineWidth);
    defaultPen.setStyle(lineStyle);
    defaultPen.setCapStyle(capStyle);
    defaultPen.setColor(defaultColor);
    this->setPen(defaultPen);
    this->setLine(startVex->center.rx(),startVex->center.ry(),endVex->center.rx(),endVex->center.ry());
    this->setZValue(-2);
}
