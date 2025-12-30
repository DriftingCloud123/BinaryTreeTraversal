#include "graphview.h"
#include <QDebug>
#include <graphicsLineItem.h>
#include <graphicsVexItem.h>

double startX = 590;
double startY = 100;
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
    myGraphicsScene->setSceneRect(0,0,1183,875);    //设置场景矩形

    this->setScene(myGraphicsScene);
    this->setFixedSize(1183,875);
    this->move(-5,-45);
    this->show();

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 初始化根节点
    init();
}

//初始化函数
//清除所有动画->恢复所有外观->清除场景和变量->创建初始节点
void MyGraphicsView::init()
{
    // 停止并清空动画队列animation queue
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
    //？
    strtVex = nullptr;
    sketchItem = nullptr;

    // 重建根节点 (V0)
    root = new MyGraphicsVexItem(QPointF(startX, startY), 10, vexID++);
    myGraphicsScene->addItem(root);
    myGraphicsScene->addItem(root->nameTag);
    vexes.push_back(root);
}

//仅重置
void MyGraphicsView::resetAllNodeStates()
{
    // 停止当前动画
    aniQueue.clear();
    onAni = false;
    if (curAni) {
        curAni->stop();
    }

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
    //TODO:是否有判断重复冗余？
    if (vexes.isEmpty() || !vexes[0]) {
        //问题
        qWarning() << "Root node not found after init()";
        return;
    }

    // 重新设置根节点位置（与 init() 保持一致，或使用新位置）

    MyGraphicsVexItem* root = vexes[0];
    //?坐标转换问题
    root->center = QPointF(startX, startY);
    root->setRect(startX - 20, startY - 20, 40, 40);

    // 更新名字标签(让item自己管理)
    if (root->nameTag && root->nameTag->scene()) {
        // 从场景中移除旧的标签
        myGraphicsScene->removeItem(root->nameTag);
    }
    // 创建新标签(setName)
    root->setName(root->nameText);
    // 确保新标签添加到场景
    if (root->nameTag && !root->nameTag->scene()) {
        myGraphicsScene->addItem(root->nameTag);
    }

    // 生成剩余节点 (V0根节点，循环从V1开始，)
    for (int i = 1; i < n; i++) {
        //找到父节点的索引
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

        // // 计算层级
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
        //创建失败警告
        if (!newVex) {
            qWarning() << "Failed to create vertex" << i;
            continue;
        }

        // 创建连线
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
    //循环结束

    //resetManualModeState()
    // 重置手动模式相关状态
    isCreating = false;
    //？
    strtVex = nullptr;
    clearSketch();

    // 清理所有半叶子和叶子列表，因为它们可能包含过时信息
    //？
    //TODO:可用性分析
    halfLeaves.clear();
    leaves.clear();
    preVexes.clear();

    // 重新计算叶子和半叶子节点??????
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

    // 自动调整视图以显示所有节点
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

            // 检查子节点是否高过父节点
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

                // 根据坐标智能分配左右孩子，并可能需要重新排序
                bool assigned = assignAndReorderChildren(strtVex, endVex, scenePos);

                if (!assigned) {
                    // 分配失败：从场景中移除节点并清理
                    handleFailedAssignment(endVex);

                    // 重置状态
                    isCreating = false;
                    strtVex = nullptr;
                    qDebug() << "无法添加子节点：该节点已满";
                    QGraphicsView::mousePressEvent(event);
                    return;
                }

                // 重新创建所有连接线（确保正确连接）
                recreateParentChildLines(strtVex);

                // 验证分配结果
                if (validateChildPositions(strtVex)) {
                    qDebug() << "节点分配成功，左孩子："
                             << (strtVex->left ? strtVex->left->nameText : "无")
                             << "，右孩子："
                             << (strtVex->right ? strtVex->right->nameText : "无");
                }
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
            qDebug() << "选中节点：" << strtVex->nameText << "，准备添加子节点";
        } else {
            // 点击空白处，清除当前选择
            isCreating = false;
            strtVex = nullptr;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

/**
 * 根据坐标分配左右孩子，并根据需要重新排序
 * 规则：左孩子的x坐标应小于右孩子的x坐标
 */
bool MyGraphicsView::assignAndReorderChildren(MyGraphicsVexItem* parent,
                                              MyGraphicsVexItem* newChild,
                                              const QPointF& newChildPos)
{
    // 情况1：父节点没有子节点
    if (!parent->left && !parent->right) {
        // 首次分配：根据新节点相对于父节点的位置
        if (newChildPos.x() < parent->center.x()) {
            parent->left = newChild;
            qDebug() << "首次分配：新节点在父节点左侧，设为左孩子";
        } else {
            parent->right = newChild;
            qDebug() << "首次分配：新节点在父节点右侧，设为右孩子";
        }
        return true;
    }

    // 情况2：父节点已满（两个孩子都存在）
    if (parent->left && parent->right) {
        qDebug() << "父节点已满，无法添加更多子节点";
        return false;
    }

    // 情况3：父节点只有一个孩子，需要重新分配
    return rearrangeSingleChild(parent, newChild, newChildPos);
}

/**
 * 当父节点只有一个孩子时，重新分配左右孩子
 * 规则：比较新节点和现有孩子的x坐标，x坐标小的为左孩子，大的为右孩子
 */
bool MyGraphicsView::rearrangeSingleChild(MyGraphicsVexItem* parent,
                                          MyGraphicsVexItem* newChild,
                                          const QPointF& newChildPos)
{
    // 获取现有孩子的引用和坐标
    MyGraphicsVexItem* existingChild = parent->left ? parent->left : parent->right;
    // bool existingIsLeft = (parent->left == existingChild);

    // 获取坐标
    qreal existingX = existingChild->center.x();
    qreal newX = newChildPos.x();

    // 收集两个孩子节点
    MyGraphicsVexItem* child1 = existingChild;
    MyGraphicsVexItem* child2 = newChild;
    qreal x1 = existingX;
    qreal x2 = newX;

    // 根据x坐标排序：小的为左孩子，大的为右孩子
    if (x1 <= x2) {
        // 现有孩子在左边或相等
        parent->left = child1;
        parent->right = child2;
        qDebug() << QString("重新分配：现有孩子(%1, x=%2)在左边，新节点(%3, x=%4)在右边")
                        .arg(child1->nameText).arg(x1)
                        .arg(child2->nameText).arg(x2);
    } else {
        // 新节点在更左边
        parent->left = child2;
        parent->right = child1;
        qDebug() << QString("重新分配：新节点(%1, x=%2)在左边，现有孩子(%3, x=%4)改为右孩子")
                        .arg(child2->nameText).arg(x2)
                        .arg(child1->nameText).arg(x1);
    }

    return true;
}

/**
 * 处理分配失败的情况
 */
void MyGraphicsView::handleFailedAssignment(MyGraphicsVexItem* failedVex)
{
    // 从场景中移除
    myGraphicsScene->removeItem(failedVex);
    myGraphicsScene->removeItem(failedVex->nameTag);

    // 从节点列表中移除
    if (vexes.contains(failedVex)) {
        vexes.removeOne(failedVex);
    }

    // 修正ID（可选，保持ID连续）
    if (vexID > 0) {
        vexID--;
    }

    // 删除对象
    delete failedVex;

    qDebug() << "分配失败，已移除创建的节点";
}

/**
 * 重新创建父节点与所有子节点的连接线
 * 先删除旧线，再创建新线，确保图形正确
 */
void MyGraphicsView::recreateParentChildLines(MyGraphicsVexItem* parent)
{
    // 步骤1：删除父节点到所有子节点的旧连接线
    QList<QGraphicsItem*> itemsToRemove;

    // 查找并标记所有需要删除的连接线
    for (QGraphicsItem* item : myGraphicsScene->items()) {
        MyGraphicsLineItem* line = dynamic_cast<MyGraphicsLineItem*>(item);
        if (line && (line->startVex == parent || line->endVex == parent)) {
            // 检查是否是连接到子节点（不是父节点）
            if ((line->startVex == parent && parent->nexts.contains(line->endVex)) ||
                (line->endVex == parent && parent->nexts.contains(line->startVex))) {
                itemsToRemove.append(item);
            }
        }
    }

    // 实际删除
    for (QGraphicsItem* item : itemsToRemove) {
        myGraphicsScene->removeItem(item);
        delete item;
    }

    // 步骤2：创建新的连接线
    // 确保nexts列表包含所有子节点（按左右顺序）
    parent->nexts.clear();
    if (parent->left) parent->nexts.push_back(parent->left);
    if (parent->right) parent->nexts.push_back(parent->right);

    // 为每个子节点创建连接线
    for (MyGraphicsVexItem* child : parent->nexts) {
        addLine(parent, child);
    }

    // 更新场景
    myGraphicsScene->update();
    qDebug() << "重新创建了" << parent->nexts.size() << "条连接线";
}

/**
 * 验证左右孩子分配是否正确
 * 规则：左孩子的x坐标应小于右孩子的x坐标
 */
bool MyGraphicsView::validateChildPositions(MyGraphicsVexItem* parent)
{
    if (!parent) return false;

    if (parent->left && parent->right) {
        qreal leftX = parent->left->center.x();
        qreal rightX = parent->right->center.x();

        if (leftX >= rightX) {
            qDebug() << "警告：左孩子的x坐标(" << leftX
                     << ") >= 右孩子的x坐标(" << rightX << ")";
            return false;
        }
        return true;
    }

    // 只有一个孩子或没有孩子也是有效的
    return true;
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

/*————三种递归的辅助函数————*/
/*————————————————*/
//先序递归辅助函数
void MyGraphicsView::preRecHelper(MyGraphicsVexItem* node) {
    if(!node)
        return;
    addAnimation(node->visit());
    preRecHelper(node->left);
    preRecHelper(node->right);
}

//中序递归辅助函数
void MyGraphicsView::inRecHelper(MyGraphicsVexItem* node) {
    if(!node)
        return;
    inRecHelper(node->left);
    addAnimation(node->visit());
    inRecHelper(node->right);
}

//后序递归辅助函数
void MyGraphicsView::posRecHelper(MyGraphicsVexItem* node) {
    if(!node) return;
    posRecHelper(node->left);
    posRecHelper(node->right);
    addAnimation(node->visit());
}

/*————————————————*/

/*——————遍历算法实现——————*/

//先序递归
void MyGraphicsView::preRecursive(MyGraphicsVexItem* head) {
    preRecHelper(head);
    emit reportStats("先序递归模式 (动画演示中)");
}

//先序非递归
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

//中序递归
void MyGraphicsView::inRecursive(MyGraphicsVexItem* head) {
    inRecHelper(head);
    emit reportStats("中序递归模式 (动画演示中)");
}


//中序非递归
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

//后序递归
void MyGraphicsView::posRecursive(MyGraphicsVexItem* head) {
    posRecHelper(head);
    emit reportStats("后序递归模式 (动画演示中)");
}

//后序非递归
void MyGraphicsView::pos(MyGraphicsVexItem * head)
{
    if(head == nullptr)
        return;
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


//层序遍历
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

/*———————递归算法完毕———————*/
