#include "chartview.h"
// #include "BinaryTree.cpp"
#include <QCoreApplication>
#include <cmath>
template class BinaryTree<int>;

// 初始化静态成员变量
int MyChartView::visitCount = 0;

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

    editDataSize = new QLineEdit("10000");
    editDataSize->setFixedWidth(100);

    // 遍历类型选择下拉框（4个选项）
    comboTraversalType = new QComboBox();
    comboTraversalType->addItem("先序遍历", PRE);
    comboTraversalType->addItem("中序遍历", IN);
    comboTraversalType->addItem("后序遍历", POST);
    comboTraversalType->addItem("层序遍历", LEVEL);
    comboTraversalType->setFixedWidth(120);

    btnCompare = new QPushButton("单次对比");
    btnTrend = new QPushButton("趋势图");

    topLayout->addWidget(new QLabel("节点数(N):"));
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

    TraversalClass traversalType = static_cast<TraversalClass>(
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

void MyChartView::runPerformanceTest(int n, TraversalClass traversalType)
{
    // 创建测试树
    BinaryTree<int>* tree = createBigTree(n);
    if (!tree) {
        textLog->append("创建测试树失败！");
        return;
    }

    QString traversalName = getTraversalTypeName(traversalType);
    textLog->append(QString("开始测试：%1，N=%2").arg(traversalName).arg(n));
    textLog->append("=======================================");

    // 根据遍历类型决定测试哪些算法
    QVector<bool> recursiveFlags;
    QVector<QString> algorithmNames;

    if (traversalType == PRE || traversalType == IN || traversalType == POST) {
        // 先序、中序、后序遍历：测试递归和非递归
        recursiveFlags = {true, false};
        if (traversalType == PRE) {
            algorithmNames = {"先序递归", "先序非递归"};
        } else if (traversalType == IN) {
            algorithmNames = {"中序递归", "中序非递归"};
        } else if (traversalType == POST) {
            algorithmNames = {"后序递归", "后序非递归"};
        }
    } else if (traversalType == LEVEL) {
        // 层序遍历：只测试非递归（忽略递归信号）
        recursiveFlags = {false};
        algorithmNames = {"层序遍历"};
    }

    // 存储测试结果
    QVector<double> times;
    QVector<size_t> maxStackDepths;
    QVector<size_t> maxQueueLengths;

    // 执行所有需要测试的算法
    for (int i = 0; i < recursiveFlags.size(); i++) {
        bool isRecursive = recursiveFlags[i];

        // 重置访问计数
        visitCount = 0;

        TraversalStats stats = performSingleAlgorithm(tree, traversalType, isRecursive);
        times.append(stats.time_ms);
        maxStackDepths.append(stats.max_stack_depth);
        maxQueueLengths.append(stats.max_queue_length);

        // 输出结果
        QString result;
        if (traversalType == LEVEL) {
            result = QString("%1: %2 ms | 访问节点: %3 | 最大队列长度: %4")
                         .arg(algorithmNames[i])
                         .arg(stats.time_ms, 0, 'f', 2)
                         .arg(visitCount)
                         .arg(stats.max_queue_length);
        } else if (!isRecursive) {
            result = QString("%1: %2 ms | 访问节点: %3 | 最大栈深: %4")
                         .arg(algorithmNames[i])
                         .arg(stats.time_ms, 0, 'f', 2)
                         .arg(visitCount)
                         .arg(stats.max_stack_depth);
        } else {
            result = QString("%1: %2 ms | 访问节点: %3")
                         .arg(algorithmNames[i])
                         .arg(stats.time_ms, 0, 'f', 2)
                         .arg(visitCount);
        }

        textLog->append(result);
    }

    textLog->append("=======================================\n");

    // 更新图表（柱状图对比）
    updateBarChart(traversalName, algorithmNames, times, n);

    // 清理内存
    deleteTree(tree);
}

void MyChartView::runTrendTest()
{
    int baseN = editDataSize->text().toInt();

    clearChart();
    textLog->append("开始遍历算法趋势测试...");
    textLog->append("=======================================");

    // 创建趋势线，对应5种算法（先序递归、先序非递归、中序递归、中序非递归、后序递归、后序非递归、层序）
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

    // 对应的遍历类型和递归标志
    QVector<TraversalClass> traversalTypes = {PRE, PRE, IN, IN, POST, POST, LEVEL};
    QVector<bool> recursiveFlags = {true, false, true, false, true, false, false};

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

    // 测试不同的节点数量（等比增长）
    QVector<int> testSizes;
    for (int i = 1; i <= 5; i++) {
        testSizes.append(baseN * i);
    }

    // 对每个测试规模，测试所有7种算法
    for (int n : testSizes) {
        textLog->append(QString("\n测试规模 N=%1").arg(n));

        // 创建测试树
        BinaryTree<int>* tree = createBigTree(n);
        if (!tree) {
            textLog->append("创建测试树失败！");
            continue;
        }

        // 测试所有7种算法
        for (int alg = 0; alg < 7; alg++) {
            TraversalClass traversalType = traversalTypes[alg];
            bool isRecursive = recursiveFlags[alg];

            // 重置访问计数
            visitCount = 0;

            TraversalStats stats = performSingleAlgorithm(tree, traversalType, isRecursive);

            // 添加数据点到对应的折线
            allSeries[alg]->append(n, stats.time_ms);

            // 记录日志
            QString logMsg = QString("  %1: %2 ms")
                                 .arg(algorithmNames[alg], -8)
                                 .arg(stats.time_ms, 8, 'f', 2);

            // 添加辅助数据信息
            if (traversalType == LEVEL) {
                logMsg += QString(" (队列长度: %1)").arg(stats.max_queue_length);
            } else if (!isRecursive) {
                logMsg += QString(" (栈深: %1)").arg(stats.max_stack_depth);
            }

            logMsg += QString(" (访问节点: %1)").arg(visitCount);

            textLog->append(logMsg);
        }

        deleteTree(tree);
        QCoreApplication::processEvents(); // 保持UI响应
    }

    // 将所有折线添加到图表
    updateTrendChart("遍历算法性能趋势", allSeries, testSizes);

    textLog->append("\n=======================================");
    textLog->append("趋势测试完成！");
}

TraversalStats MyChartView::performSingleAlgorithm(BinaryTree<int>* tree, TraversalClass traversalType, bool isRecursive)
{
    // 直接调用Traversal函数，传入遍历类型和是否递归
    return tree->Traversal(traversalType, isRecursive, visitNodeForStats);
}

QString MyChartView::getTraversalTypeName(TraversalClass type) const
{
    switch (type) {
    case PRE: return "先序遍历";
    case IN: return "中序遍历";
    case POST: return "后序遍历";
    case LEVEL: return "层序遍历";
    default: return "未知类型";
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

void MyChartView::updateBarChart(const QString& title, const QVector<QString>& algorithmNames,
                                 const QVector<double>& times, int n)
{
    clearChart();

    QBarSeries *series = new QBarSeries();

    // 为每个算法创建柱状图数据
    for (int i = 0; i < algorithmNames.size(); i++) {
        QBarSet *barSet = new QBarSet(algorithmNames[i]);
        *barSet << times[i];
        series->append(barSet);
    }

    chart->addSeries(series);
    chart->setTitle(QString("%1 性能对比 (N=%2)").arg(title).arg(n));

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
}

void MyChartView::updateTrendChart(const QString& title, const QVector<QLineSeries*>& allSeries,
                                   const QVector<int>& testSizes)
{
    clearChart();

    // 将所有折线添加到图表
    for (QLineSeries* series : allSeries) {
        chart->addSeries(series);
    }

    // 创建坐标轴
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setTitleText("节点数 (N)");
    chart->axes(Qt::Vertical).first()->setTitleText("时间 (ms)");
    chart->setTitle(QString("%1 (基准N=%2)").arg(title).arg(testSizes[0]));
}

// ==================== 二叉树操作 ====================

BinaryTree<int>* MyChartView::createBigTree(int n)
{
    if (n <= 0) return nullptr;

    BinaryTree<int>* tree = new BinaryTree<int>();

    // 使用自动创建树的方法
    tree->autoCreateTree(n);

    return tree;
}

void MyChartView::deleteTree(BinaryTree<int>* tree)
{
    if (tree) {
        delete tree;
    }
}

// 用于统计的访问函数
void MyChartView::visitNodeForStats(TreeNode<int>* node)
{
    // 访问节点，增加计数
    visitCount++;

    // 为了防止编译器优化掉这个操作，可以做一些简单的计算
    volatile int temp = node->data;
    (void)temp; // 避免未使用变量的警告
}
