#include "chartview.h"
#include <vector>
#include <stack>
#include <queue>
#include <QCoreApplication>

MyChartView::MyChartView(QWidget *parent)
    : QWidget(parent)
    , chart(nullptr)
    , chartView(nullptr)
{
    setupUI();
}

MyChartView::~MyChartView()
{
    if (chart) {
        delete chart;
    }
}

void MyChartView::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部控制面板
    QHBoxLayout *topLayout = new QHBoxLayout();

    editDataSize = new QLineEdit("100000");
    editDataSize->setFixedWidth(100);

    // 遍历类型选择下拉框（4个选项）
    comboTraversalType = new QComboBox();
    comboTraversalType->addItem("先序遍历", TYPE_PREORDER);
    comboTraversalType->addItem("中序遍历", TYPE_INORDER);
    comboTraversalType->addItem("后序遍历", TYPE_POSTORDER);
    comboTraversalType->addItem("层序遍历", TYPE_LEVELORDER);
    comboTraversalType->setFixedWidth(120);

    btnCompare = new QPushButton("单次对比");
    btnTrend = new QPushButton("趋势图");

    topLayout->addWidget(new QLabel("N:"));
    topLayout->addWidget(editDataSize);
    topLayout->addWidget(new QLabel("遍历类型:"));
    topLayout->addWidget(comboTraversalType);
    topLayout->addWidget(btnCompare);
    topLayout->addWidget(btnTrend);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 底部内容区域
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    textLog = new QTextEdit();
    textLog->setMaximumWidth(400);
    textLog->setReadOnly(true);

    // 图表区域
    chart = new QChart();
    chart->setTitle("遍历算法性能分析");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    bottomLayout->addWidget(textLog, 3);
    bottomLayout->addWidget(chartView, 7);

    mainLayout->addLayout(bottomLayout);

    // 连接信号槽
    connect(btnCompare, &QPushButton::clicked, this, &MyChartView::onCompareClicked);
    connect(btnTrend, &QPushButton::clicked, this, &MyChartView::onTrendClicked);
}

void MyChartView::onCompareClicked()
{
    int n = editDataSize->text().toInt();
    if (n <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的节点数");
        return;
    }

    TraversalType traversalType = static_cast<TraversalType>(
        comboTraversalType->currentData().toInt()
        );

    runPerformanceTest(n, traversalType);
}

void MyChartView::onTrendClicked()
{
    int baseN = editDataSize->text().toInt();
    if (baseN <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的基准节点数");
        return;
    }

    runTrendTest();
}

void MyChartView::runPerformanceTest(int n, TraversalType traversalType)
{
    // 创建测试树
    TreeNode* root = createBigTree(n);
    if (!root) {
        textLog->append("创建测试树失败！");
        return;
    }

    QString traversalName = getTraversalTypeName(traversalType);
    textLog->append(QString("开始测试：%1，N=%2").arg(traversalName).arg(n));
    textLog->append("=======================================");

    // 根据遍历类型决定测试哪些算法
    QVector<AlgorithmDetail> algorithmsToTest;
    QVector<QString> algorithmNames;

    if (traversalType == TYPE_PREORDER) {
        // 先序遍历：测试递归和非递归
        algorithmsToTest = {ALG_PREORDER_RECURSIVE, ALG_PREORDER_ITERATIVE};
        algorithmNames = {"先序递归", "先序非递归"};
    } else if (traversalType == TYPE_INORDER) {
        // 中序遍历：测试递归和非递归
        algorithmsToTest = {ALG_INORDER_RECURSIVE, ALG_INORDER_ITERATIVE};
        algorithmNames = {"中序递归", "中序非递归"};
    } else if (traversalType == TYPE_POSTORDER) {
        // 后序遍历：测试递归和非递归
        algorithmsToTest = {ALG_POSTORDER_RECURSIVE, ALG_POSTORDER_ITERATIVE};
        algorithmNames = {"后序递归", "后序非递归"};
    } else if (traversalType == TYPE_LEVELORDER) {
        // 层序遍历：只测试非递归
        algorithmsToTest = {ALG_LEVELORDER};
        algorithmNames = {"层序遍历"};
    }

    // 存储测试结果
    QVector<double> times;
    QVector<int> auxDataResults;

    // 执行所有需要测试的算法
    for (int i = 0; i < algorithmsToTest.size(); i++) {
        AlgorithmDetail algorithm = algorithmsToTest[i];
        int auxData = 0;

        double timeMs = performSingleAlgorithm(root, algorithm, auxData);
        times.append(timeMs);
        auxDataResults.append(auxData);

        // 输出结果
        QString result;
        if (algorithm == ALG_LEVELORDER) {
            result = QString("%1: %2 ms | 最大队列长度: %3")
                         .arg(algorithmNames[i])
                         .arg(timeMs, 0, 'f', 2)
                         .arg(auxData);
        } else if (algorithm == ALG_PREORDER_ITERATIVE ||
                   algorithm == ALG_INORDER_ITERATIVE ||
                   algorithm == ALG_POSTORDER_ITERATIVE) {
            result = QString("%1: %2 ms | 最大栈深: %3")
                         .arg(algorithmNames[i])
                         .arg(timeMs, 0, 'f', 2)
                         .arg(auxData);
        } else {
            result = QString("%1: %2 ms")
            .arg(algorithmNames[i])
                .arg(timeMs, 0, 'f', 2);
        }

        textLog->append(result);
    }

    textLog->append("=======================================\n");

    // 更新图表（柱状图对比）
    clearChart();

    QBarSeries *series = new QBarSeries();

    // 为每个算法创建柱状图数据
    for (int i = 0; i < algorithmNames.size(); i++) {
        QBarSet *barSet = new QBarSet(algorithmNames[i]);
        *barSet << times[i];
        series->append(barSet);
    }

    chart->addSeries(series);
    chart->setTitle(QString("%1 性能对比 (N=%2)").arg(traversalName).arg(n));

    // 设置X轴
    QStringList categories;
    categories << QString("N=%1").arg(n);
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 设置Y轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("时间 (ms)");
    axisY->setMin(0);

    // 计算合适的Y轴最大值
    double maxTime = 0;
    for (double time : times) {
        if (time > maxTime) maxTime = time;
    }
    double maxY = maxTime * 1.2;
    if (maxY < 1.0) maxY = 1.0;
    axisY->setMax(maxY);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 清理内存
    deleteTree(root);
}

void MyChartView::runTrendTest()
{
    int baseN = editDataSize->text().toInt();

    clearChart();
    textLog->append("开始7种算法的趋势测试...");
    textLog->append("=======================================");

    // 创建7条趋势线，对应7种详细算法
    QVector<QLineSeries*> allSeries;

    // 7种算法的名称和颜色
    QStringList algorithmNames = {
        "先序递归",
        "先序非递归",
        "中序递归",
        "中序非递归",
        "后序递归",
        "后序非递归",
        "层序遍历"
    };

    QColor colors[7] = {
        QColor(255, 0, 0),      // 红 - 先序递归
        QColor(255, 100, 100),  // 浅红 - 先序非递归
        QColor(0, 255, 0),      // 绿 - 中序递归
        QColor(100, 255, 100),  // 浅绿 - 中序非递归
        QColor(0, 0, 255),      // 蓝 - 后序递归
        QColor(100, 100, 255),  // 浅蓝 - 后序非递归
        QColor(255, 165, 0)     // 橙 - 层序遍历
    };

    // 为每种算法创建折线
    for (int i = 0; i < 7; i++) {
        QLineSeries *series = new QLineSeries();
        series->setName(algorithmNames[i]);
        series->setColor(colors[i]);
        allSeries.append(series);
    }

    // 测试不同的节点数量
    QVector<int> testSizes;
    for (int i = 1; i <= 5; i++) {
        testSizes.append(baseN * i);
    }

    // 对每个测试规模，测试所有7种算法
    for (int n : testSizes) {
        textLog->append(QString("\n测试规模 N=%1").arg(n));

        // 创建测试树
        TreeNode* root = createBigTree(n);
        if (!root) {
            textLog->append("创建测试树失败！");
            continue;
        }

        // 测试所有7种算法
        for (int alg = 0; alg < 7; alg++) {
            AlgorithmDetail algorithm = static_cast<AlgorithmDetail>(alg);
            int auxData = 0;

            double timeMs = performSingleAlgorithm(root, algorithm, auxData);

            // 添加数据点到对应的折线
            allSeries[alg]->append(n, timeMs);

            // 记录日志
            QString logMsg = QString("  %1: %2 ms")
                                 .arg(algorithmNames[alg], -8)
                                 .arg(timeMs, 8, 'f', 2);

            // 添加辅助数据信息
            if (algorithm == ALG_LEVELORDER) {
                logMsg += QString(" (队列长度: %1)").arg(auxData);
            } else if (algorithm == ALG_PREORDER_ITERATIVE ||
                       algorithm == ALG_INORDER_ITERATIVE ||
                       algorithm == ALG_POSTORDER_ITERATIVE) {
                logMsg += QString(" (栈深: %1)").arg(auxData);
            }

            textLog->append(logMsg);
        }

        deleteTree(root);
        QCoreApplication::processEvents(); // 保持UI响应
    }

    // 将所有折线添加到图表
    for (QLineSeries* series : allSeries) {
        chart->addSeries(series);
    }

    // 创建坐标轴
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setTitleText("节点数 (N)");
    chart->axes(Qt::Vertical).first()->setTitleText("时间 (ms)");
    chart->setTitle(QString("7种遍历算法性能趋势 (基准N=%1)").arg(baseN));

    textLog->append("\n=======================================");
    textLog->append("趋势测试完成！");
}

double MyChartView::performSingleAlgorithm(TreeNode* root, AlgorithmDetail algorithm, int &auxData)
{
    QElapsedTimer timer;
    timer.start();

    switch (algorithm) {
    case ALG_PREORDER_RECURSIVE:
        perfPreorderRecursive(root);
        auxData = 0;
        break;

    case ALG_PREORDER_ITERATIVE:
        perfPreorderIterative(root, auxData);
        break;

    case ALG_INORDER_RECURSIVE:
        perfInorderRecursive(root);
        auxData = 0;
        break;

    case ALG_INORDER_ITERATIVE:
        perfInorderIterative(root, auxData);
        break;

    case ALG_POSTORDER_RECURSIVE:
        perfPostorderRecursive(root);
        auxData = 0;
        break;

    case ALG_POSTORDER_ITERATIVE:
        perfPostorderIterative(root, auxData);
        break;

    case ALG_LEVELORDER:
        perfLevelorder(root, auxData);
        break;
    }

    return timer.nsecsElapsed() / 1e6; // 返回毫秒
}

QString MyChartView::getTraversalTypeName(TraversalType type) const
{
    switch (type) {
    case TYPE_PREORDER: return "先序遍历";
    case TYPE_INORDER: return "中序遍历";
    case TYPE_POSTORDER: return "后序遍历";
    case TYPE_LEVELORDER: return "层序遍历";
    default: return "未知类型";
    }
}

QString MyChartView::getAlgorithmDetailName(AlgorithmDetail algorithm) const
{
    switch (algorithm) {
    case ALG_PREORDER_RECURSIVE: return "先序递归";
    case ALG_PREORDER_ITERATIVE: return "先序非递归";
    case ALG_INORDER_RECURSIVE: return "中序递归";
    case ALG_INORDER_ITERATIVE: return "中序非递归";
    case ALG_POSTORDER_RECURSIVE: return "后序递归";
    case ALG_POSTORDER_ITERATIVE: return "后序非递归";
    case ALG_LEVELORDER: return "层序遍历";
    default: return "未知算法";
    }
}

void MyChartView::clearChart()
{
    chart->removeAllSeries();

    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis* axis : axes) {
        chart->removeAxis(axis);
        delete axis;
    }
}

// ==================== 二叉树操作 ====================

MyChartView::TreeNode* MyChartView::createBigTree(int n)
{
    if (n <= 0) return nullptr;

    std::vector<TreeNode*> nodes;
    nodes.reserve(n);

    for (int i = 0; i < n; i++) {
        nodes.push_back(new TreeNode(i));
    }

    for (int i = 0; i < n / 2; i++) {
        if (2 * i + 1 < n) {
            nodes[i]->left = nodes[2 * i + 1];
        }
        if (2 * i + 2 < n) {
            nodes[i]->right = nodes[2 * i + 2];
        }
    }

    return nodes[0];
}

void MyChartView::deleteTree(TreeNode* root)
{
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// ==================== 7种遍历算法的实现 ====================

// 1. 先序遍历递归
void MyChartView::perfPreorderRecursive(TreeNode* root)
{
    if (!root) return;
    volatile int x = root->val; (void)x;
    perfPreorderRecursive(root->left);
    perfPreorderRecursive(root->right);
}

// 2. 先序遍历非递归
void MyChartView::perfPreorderIterative(TreeNode* root, int &maxStack)
{
    if (!root) return;

    std::stack<TreeNode*> s;
    s.push(root);
    maxStack = 1;

    while (!s.empty()) {
        if ((int)s.size() > maxStack) {
            maxStack = s.size();
        }

        TreeNode* current = s.top();
        s.pop();

        volatile int x = current->val; (void)x;

        if (current->right) {
            s.push(current->right);
        }
        if (current->left) {
            s.push(current->left);
        }
    }
}

// 3. 中序遍历递归
void MyChartView::perfInorderRecursive(TreeNode* root)
{
    if (!root) return;
    perfInorderRecursive(root->left);
    volatile int x = root->val; (void)x;
    perfInorderRecursive(root->right);
}

// 4. 中序遍历非递归
void MyChartView::perfInorderIterative(TreeNode* root, int &maxStack)
{
    if (!root) return;

    std::stack<TreeNode*> s;
    TreeNode* current = root;
    maxStack = 0;

    while (current || !s.empty()) {
        if ((int)s.size() > maxStack) {
            maxStack = s.size();
        }

        if (current) {
            s.push(current);
            current = current->left;
        } else {
            current = s.top();
            s.pop();

            volatile int x = current->val; (void)x;

            current = current->right;
        }
    }
}

// 5. 后序遍历递归
void MyChartView::perfPostorderRecursive(TreeNode* root)
{
    if (!root) return;
    perfPostorderRecursive(root->left);
    perfPostorderRecursive(root->right);
    volatile int x = root->val; (void)x;
}

// 6. 后序遍历非递归（双栈法）
void MyChartView::perfPostorderIterative(TreeNode* root, int &maxStack)
{
    if (!root) return;

    std::stack<TreeNode*> s1, s2;
    s1.push(root);
    maxStack = 1;

    while (!s1.empty()) {
        if ((int)s1.size() > maxStack) {
            maxStack = s1.size();
        }

        TreeNode* current = s1.top();
        s1.pop();
        s2.push(current);

        if (current->left) {
            s1.push(current->left);
        }
        if (current->right) {
            s1.push(current->right);
        }
    }

    while (!s2.empty()) {
        TreeNode* current = s2.top();
        s2.pop();
        volatile int x = current->val; (void)x;
    }
}

// 7. 层序遍历
void MyChartView::perfLevelorder(TreeNode* root, int &maxQueue)
{
    if (!root) return;

    std::queue<TreeNode*> q;
    q.push(root);
    maxQueue = 1;

    while (!q.empty()) {
        if ((int)q.size() > maxQueue) {
            maxQueue = q.size();
        }

        TreeNode* current = q.front();
        q.pop();

        volatile int x = current->val; (void)x;

        if (current->left) {
            q.push(current->left);
        }
        if (current->right) {
            q.push(current->right);
        }
    }
}
