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

//仅重置
void MyGraphicsView::resetAllNodeStates()
{
    // 停止当前动画
    if (curAni) {
        curAni->stop();
    }
    aniQueue.clear();
    onAni = false;

    // 重置所有节点
    for (MyGraphicsVexItem* vex : vexes) {
        if (vex) {
            vex->setBrush(regBrush);
            vex->setRect(vex->center.x() - 20, vex->center.y() - 20, 40, 40);
        }
    }

    // 重置特殊节点
    for (MyGraphicsVexItem* nullVex : nullVexes) {
        if (nullVex) {
            nullVex->setBrush(QColor(255, 255, 255));
            nullVex->setRect(nullVex->center.x() - 20, nullVex->center.y() - 20, 40, 40);
        }
    }

    // 更新场景
    myGraphicsScene->update();
}

// --- 自动生成树 (Tab 1 核心功能) ---
void MyGraphicsView::autoCreateTree(int n)
{
    // 每次生成前先重置
    init();

    if (n <= 1) {
        return; // 只有一个根节点
    }

    // 检查根节点是否存在
    if (vexes.isEmpty() || !vexes[0]) {
        //问题
        qWarning() << "Root node not found after init()";
        return;
    }

    // 重新设置根节点位置（与 init() 保持一致，或使用新位置）
    // 注意：init() 中根节点在 (590, 150)，这里改为 (590, 100)
    // 如果希望保持一致，应该修改 init() 或这里使用相同位置
    qreal startX = 590;
    qreal startY = 100;  // 注意：init() 中是 150，这里不一致！

    MyGraphicsVexItem* root = vexes[0];
    root->center = QPointF(startX, startY);
    root->setRect(startX - 20, startY - 20, 40, 40);

    // 安全地更新名字标签（不要直接 delete，让 item 自己管理或使用场景移除）
    if (root->nameTag && root->nameTag->scene()) {
        // 从场景中移除旧的标签
        myGraphicsScene->removeItem(root->nameTag);
    }
    // 创建新标签（setName 应该会创建新标签）
    root->setName(root->nameText);
    // 确保新标签添加到场景
    if (root->nameTag && !root->nameTag->scene()) {
        myGraphicsScene->addItem(root->nameTag);
    }

    // 生成剩余节点 (从 V1 开始，因为 V0 已经是根节点)
    // 注意：循环应该从 1 开始，但需要确保 ID 正确递增
    for (int i = 1; i < n; i++) {
        int parentIdx = (i - 1) / 2;

        // 保护机制
        if (parentIdx >= vexes.size()) {
            qWarning() << "Parent index out of range:" << parentIdx << "for child" << i;
            break;
        }

        MyGraphicsVexItem* parent = vexes[parentIdx];
        if (!parent) {
            qWarning() << "Parent is null at index:" << parentIdx;
            break;
        }

        // 计算层级
        int level = 0;
        int tmp = i + 1;
        while (tmp >>= 1) level++;

        // 动态调整偏移量，防止节点重叠
        // 第一层偏移较大，随着层级加深逐渐减小
        qreal baseXOffset = 300.0;
        qreal xOffset = baseXOffset / (1 << (level - 1));
        qreal yDist = 100;

        bool isLeft = (i % 2 != 0);  // 奇数节点为左子节点
        qreal newX = parent->center.x() + (isLeft ? -xOffset : xOffset);
        qreal newY = startY + level * yDist;

        // 创建新节点
        MyGraphicsVexItem* newVex = addVex(QPointF(newX, newY));
        if (!newVex) {
            qWarning() << "Failed to create vertex" << i;
            continue;
        }

        // 建立连接
        MyGraphicsLineItem* line = addLine(parent, newVex);
        if (line) {
            parent->nexts.push_back(newVex);

            // 设置左右子节点
            if (isLeft) {
                parent->left = newVex;
            } else {
                parent->right = newVex;
            }
        } else {
            qWarning() << "Failed to create line for vertex" << i;
        }
    }
    //resetManualModeState()
    // 重置手动模式相关状态
    isCreating = false;
    strtVex = nullptr;
    clearSketch();

    // 清理所有半叶子和叶子列表，因为它们可能包含过时信息
    halfLeaves.clear();
    leaves.clear();
    preVexes.clear();

    // 重新计算叶子和半叶子节点
    for (MyGraphicsVexItem* vex : vexes) {
        if (vex) {
            // 检查是否为叶子节点（没有子节点）
            if (!vex->left && !vex->right && vex->nexts.isEmpty()) {
                if (!leaves.contains(vex)) {
                    leaves.push_back(vex);
                }
            }
            // 检查是否为半叶子节点（只有一个子节点）
            else if ((vex->left && !vex->right) || (!vex->left && vex->right) ||
                     vex->nexts.size() == 1) {
                if (!halfLeaves.contains(vex)) {
                    halfLeaves.push_back(vex);
                }
            }
        }
    }

    // 更新场景
    myGraphicsScene->update();

    // 可选：自动调整视图以显示所有节点
    fitTreeInView();
}

// 辅助函数：调整视图以显示整棵树
void MyGraphicsView::fitTreeInView()
{
    if (vexes.isEmpty()) return;

    // 计算所有节点的边界
    QRectF boundingRect;
    for (MyGraphicsVexItem* vex : vexes) {
        if (vex) {
            boundingRect = boundingRect.united(vex->sceneBoundingRect());
        }
    }

    // 包括边
    QList<QGraphicsItem*> allItems = myGraphicsScene->items();
    for (QGraphicsItem* item : allItems) {
        if (item && (dynamic_cast<MyGraphicsLineItem*>(item) ||
                     dynamic_cast<QGraphicsTextItem*>(item))) {
            boundingRect = boundingRect.united(item->sceneBoundingRect());
        }
    }

    // 添加一些边距
    boundingRect.adjust(-50, -50, 50, 50);

    // 调整视图
    this->setSceneRect(boundingRect);
    this->fitInView(boundingRect, Qt::KeepAspectRatio);
}

// --- 鼠标交互 (手动建树) ---
void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // 将视图坐标转换为场景坐标
    QPointF scenePos = this->mapToScene(event->pos());

    if(isCreating){
        clearSketch();

        // 检查当前节点是否已经有左右子节点
        if(strtVex) {
            // 检查是否已经有左右子节点（二叉树最多两个子节点）
            if(strtVex->left && strtVex->right) {
                // 如果已经有左右子节点，不允许再添加
                isCreating = !isCreating;
                strtVex = nullptr;
                qDebug() << "该节点已经有左右子节点，无法再添加新子节点";
                QGraphicsView::mousePressEvent(event);
                return;
            }

            // 检查是否试图连接已存在的节点
            for(MyGraphicsVexItem* existingVex : vexes) {
                if(existingVex != strtVex) {
                    QPointF c = existingVex->center;
                    // 检查是否点击了已存在的节点（使用场景坐标）
                    if(scenePos.x() >= c.x()-20 && scenePos.x() <= c.x()+20 &&
                        scenePos.y() >= c.y()-20 && scenePos.y() <= c.y()+20) {
                        // 不允许连接到已存在的节点
                        isCreating = !isCreating;
                        strtVex = nullptr;
                        qDebug() << "不允许连接到已存在的节点";
                        QGraphicsView::mousePressEvent(event);
                        return;
                    }
                }
            }

            // 新增：检查子节点是否高过父节点
            if(scenePos.y() < strtVex->center.y()) {
                // 子节点的 Y 坐标不能小于父节点的 Y 坐标
                qDebug() << "子节点不能高过父节点";

                // 可以给用户一个视觉反馈，比如闪烁父节点或显示提示
                // 暂时只输出日志，不创建节点
                QGraphicsView::mousePressEvent(event);
                return;
            }
        }

        // 使用场景坐标添加新节点
        MyGraphicsVexItem* endVex = addVex(scenePos);
        if(strtVex) {
            // 确保不重复添加
            if(!strtVex->nexts.contains(endVex)) {
                strtVex->nexts.push_back(endVex);

                // 分配左右子节点：先左后右
                if(!strtVex->left) {
                    strtVex->left = endVex;
                } else if(!strtVex->right) {
                    strtVex->right = endVex;
                }

                addLine(strtVex, endVex);
            }
        }
        isCreating = !isCreating;
        strtVex = nullptr;  // 重置起始节点
    } else {
        bool flag = false;
        for(int i=0; i<vexes.size(); i++) {
            // 使用场景坐标进行点击碰撞检测
            QPointF c = vexes[i]->center;
            if(scenePos.x() >= c.x()-20 && scenePos.x() <= c.x()+20 &&
                scenePos.y() >= c.y()-20 && scenePos.y() <= c.y()+20)
            {
                flag = true;
                strtVex = vexes[i];
                break;  // 找到第一个匹配的节点就退出
            }
        }
        if(flag){
            isCreating = !isCreating;
        } else {
            // 点击空白处，清除当前选择
            isCreating = false;
            strtVex = nullptr;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event){
    // 将视图坐标转换为场景坐标
    QPointF scenePos = this->mapToScene(event->pos());

    if(isCreating && strtVex){
        clearSketch();
        // 使用场景坐标绘制临时线
        sketchLine(strtVex->pos() + strtVex->rect().center(), scenePos);
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
