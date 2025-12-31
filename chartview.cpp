#include "chartview.h"
#include <QCoreApplication>
#include <cmath>
#include <QProgressDialog>
#include <QThread>
#include <algorithm>

// template class BinaryTree<int>;

// 初始化静态成员变量
int MyChartView::visitCount = 0;

// 全局线型配置 - 7种算法的线型（true=实线，false=虚线）
const std::vector<bool> MyChartView::lineStyleConfig = {
    false,   // 0: 先序递归 - 实线
    true,  // 1: 先序非递归 - 虚线
    false,   // 2: 中序递归 - 实线
    true,  // 3: 中序非递归 - 虚线
    false,   // 4: 后序递归 - 实线
    true,  // 5: 后序非递归 - 虚线
    true   // 6: 层序遍历 - 虚线
};

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

    // 第一行：单次测试控制
    QHBoxLayout *singleTestLayout = new QHBoxLayout();

    editDataSize = new QLineEdit("10000");
    editDataSize->setFixedWidth(100);

    comboTraversalType = new QComboBox();
    comboTraversalType->addItem("先序遍历", PRE);
    comboTraversalType->addItem("中序遍历", IN);
    comboTraversalType->addItem("后序遍历", POST);
    comboTraversalType->addItem("层序遍历", LEVEL);
    comboTraversalType->setFixedWidth(120);

    btnCompare = new QPushButton("单次对比");

    singleTestLayout->addWidget(new QLabel("单次测试 - 节点数(N):"));
    singleTestLayout->addWidget(editDataSize);
    singleTestLayout->addWidget(new QLabel("遍历类型:"));
    singleTestLayout->addWidget(comboTraversalType);
    singleTestLayout->addWidget(btnCompare);
    singleTestLayout->addStretch();

    // 第二行：趋势测试参数
    QHBoxLayout *trendParamsLayout = new QHBoxLayout();

    editMinNodes = new QLineEdit("1000");
    editMinNodes->setFixedWidth(80);
    editMinNodes->setToolTip("最小节点数");

    editMaxNodes = new QLineEdit("20000");
    editMaxNodes->setFixedWidth(80);
    editMaxNodes->setToolTip("最大节点数");

    editStepSize = new QLineEdit("2000");
    editStepSize->setFixedWidth(80);
    editStepSize->setToolTip("节点数增长步长");

    editRepeatTimes = new QLineEdit("3");
    editRepeatTimes->setFixedWidth(60);
    editRepeatTimes->setToolTip("每个节点数重复测试次数");

    btnTrend = new QPushButton("趋势图(详细统计)");
    btnQuickTrend = new QPushButton("快速趋势图");

    trendParamsLayout->addWidget(new QLabel("趋势测试 - 最小节点数:"));
    trendParamsLayout->addWidget(editMinNodes);
    trendParamsLayout->addWidget(new QLabel("最大节点数:"));
    trendParamsLayout->addWidget(editMaxNodes);
    trendParamsLayout->addWidget(new QLabel("步长:"));
    trendParamsLayout->addWidget(editStepSize);
    trendParamsLayout->addWidget(new QLabel("重复次数:"));
    trendParamsLayout->addWidget(editRepeatTimes);
    trendParamsLayout->addWidget(btnTrend);
    trendParamsLayout->addWidget(btnQuickTrend);
    trendParamsLayout->addStretch();

    // 第三行：统计信息显示
    QHBoxLayout *statsLayout = new QHBoxLayout();
    lblStatsInfo = new QLabel("就绪");
    lblStatsInfo->setStyleSheet("color: green; font-weight: bold;");
    statsLayout->addWidget(lblStatsInfo);
    statsLayout->addStretch();

    // 添加到主布局
    mainLayout->addLayout(singleTestLayout);
    mainLayout->addLayout(trendParamsLayout);
    mainLayout->addLayout(statsLayout);

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
    connect(btnQuickTrend, &QPushButton::clicked, this, &MyChartView::onQuickTrendClicked);
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
    // 验证输入参数
    int minNodes = editMinNodes->text().toInt();
    int maxNodes = editMaxNodes->text().toInt();
    int stepSize = editStepSize->text().toInt();
    int repeatTimes = editRepeatTimes->text().toInt();

    if (minNodes <= 0 || maxNodes <= 0 || stepSize <= 0 || repeatTimes <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的参数值");
        return;
    }

    if (minNodes > maxNodes) {
        QMessageBox::warning(this, "输入错误", "最小节点数不能大于最大节点数");
        return;
    }

    if (stepSize > (maxNodes - minNodes)) {
        QMessageBox::warning(this, "输入错误", "步长不能超过节点数范围");
        return;
    }

    runDetailedTrendTest(minNodes, maxNodes, stepSize, repeatTimes);
}

void MyChartView::onQuickTrendClicked()
{
    // 快速趋势图，使用默认参数，不重复测试
    int minNodes = 1000;
    int maxNodes = 10000;
    int stepSize = 1000;

    runDetailedTrendTest(minNodes, maxNodes, stepSize, 1);
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

void MyChartView::runDetailedTrendTest(int minNodes, int maxNodes, int stepSize, int repeatTimes)
{
    // 生成测试节点数序列
    QVector<int> testSizes;
    for (int n = minNodes; n <= maxNodes; n += stepSize) {
        testSizes.append(n);
    }

    if (testSizes.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法生成有效的测试序列");
        return;
    }

    // 创建进度对话框
    QProgressDialog progress("正在进行详细统计测试...", "取消", 0, testSizes.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);

    clearChart();
    textLog->append("开始详细统计趋势测试...");
    textLog->append(QString("测试范围: %1 ~ %2 (步长: %3, 重复次数: %4)")
                        .arg(minNodes).arg(maxNodes).arg(stepSize).arg(repeatTimes));
    textLog->append("=======================================");

    // 7种算法的配置
    QStringList algorithmNames = {
        "先序递归", "先序非递归",
        "中序递归", "中序非递归",
        "后序递归", "后序非递归",
        "层序遍历"
    };
    QVector<TraversalClass> traversalTypes = {PRE, PRE, IN, IN, POST, POST, LEVEL};
    QVector<bool> recursiveFlags = {true, false, true, false, true, false, false};
    QColor colors[7] = {
        QColor(255, 0, 0), QColor(255, 100, 100),
        QColor(0, 255, 0), QColor(100, 255, 100),
        QColor(0, 0, 255), QColor(100, 100, 255),
        QColor(255, 165, 0)
    };

    // 初始化 Series
    QVector<QLineSeries*> allSeries;
    QVector<QLineSeries*> errorSeries;
    QVector<QVector<double>> allTimes(7);
    QVector<QVector<double>> allStackDepths(7);
    QVector<QVector<double>> allQueueLengths(7);

    for (int i = 0; i < 7; i++) {
        QLineSeries *series = new QLineSeries();
        series->setName(algorithmNames[i]);
        series->setColor(colors[i]);
        allSeries.append(series);

        QLineSeries *errorSeriesItem = new QLineSeries();
        errorSeriesItem->setName(algorithmNames[i] + " 误差范围");
        errorSeriesItem->setColor(colors[i].lighter(150));
        errorSeriesItem->setOpacity(0.3);
        errorSeries.append(errorSeriesItem);
    }

    // --- 主测试循环 ---
    for (int n : testSizes) {
        if (progress.wasCanceled()) break;

        textLog->append(QString("\n测试规模 N=%1").arg(n));

        // 【优化关键点 1】: 在重复测试循环之外创建树，复用数据结构
        // 极大地减少了 new/delete 的开销，解决了“速度慢”的问题
        BinaryTree<int>* tree = createBigTree(n);

        // 【优化关键点 2】: 检查树是否创建成功，防止空树导致曲线掉落
        if (!tree) {
            textLog->append("错误：内存不足，无法创建此规模的树，跳过测试。");
            continue;
        }

        // 存储当前规模下，7种算法每次重复的数据
        QVector<QVector<double>> currentSizeTimes(7);
        QVector<size_t> currentAvgStack(7, 0);
        QVector<size_t> currentAvgQueue(7, 0);

        // 重复测试
        for (int repeat = 0; repeat < repeatTimes; repeat++) {
            for (int alg = 0; alg < 7; alg++) {
                visitCount = 0; // 重置计数器

                // 执行算法
                TraversalStats stats = performSingleAlgorithm(tree, traversalTypes[alg], recursiveFlags[alg]);

                // 记录数据
                currentSizeTimes[alg].append(stats.time_ms);

                // 累加空间占用数据（只需统计总和，最后取平均）
                if (traversalTypes[alg] == LEVEL) {
                    currentAvgQueue[alg] += stats.max_queue_length;
                } else if (!recursiveFlags[alg]) {
                    currentAvgStack[alg] += stats.max_stack_depth;
                }
            }
        }

        // 【优化关键点 3】: 测试完当前规模的所有重复次数后，再销毁树
        deleteTree(tree);

        // --- 数据处理与绘图 ---
        for (int alg = 0; alg < 7; alg++) {
            if (currentSizeTimes[alg].isEmpty()) continue;

            // 1. 计算平均时间
            double sumTime = 0;
            for (double t : currentSizeTimes[alg]) sumTime += t;
            double avgTime = sumTime / currentSizeTimes[alg].size();

            // 2. 计算标准差
            double stdDev = 0;
            for (double t : currentSizeTimes[alg]) stdDev += std::pow(t - avgTime, 2);
            stdDev = std::sqrt(stdDev / currentSizeTimes[alg].size());

            // 3. 记录到全局数据
            allTimes[alg].append(avgTime);
            allSeries[alg]->append(n, avgTime);
            errorSeries[alg]->append(n, avgTime - stdDev);
            errorSeries[alg]->append(n, avgTime + stdDev);

            // 4. 处理空间复杂度数据
            QString extraInfo = "";
            if (traversalTypes[alg] == LEVEL) {
                size_t avgQ = currentAvgQueue[alg] / repeatTimes;
                allQueueLengths[alg].append(avgQ);
                extraInfo = QString(" | Q: %1").arg(avgQ);
            } else if (!recursiveFlags[alg]) {
                size_t avgS = currentAvgStack[alg] / repeatTimes;
                allStackDepths[alg].append(avgS);
                extraInfo = QString(" | S: %1").arg(avgS);
            }

            // 输出简略日志 (避免刷屏)
            if (alg == 0 || alg == 6) { // 只打印第一个和最后一个作为进度提示
                textLog->append(QString("  -> %1: Avg %2 ms%3")
                                    .arg(algorithmNames[alg])
                                    .arg(avgTime, 0, 'f', 2)
                                    .arg(extraInfo));
            }
        }

        // 更新进度条
        progress.setValue(progress.value() + 1);
        QCoreApplication::processEvents(); // 保持界面不卡死
    }

    progress.close();

    if (allSeries[0]->count() > 0) {
        updateDetailedTrendChart("遍历算法性能详细统计", allSeries, errorSeries,
                                 testSizes, algorithmNames, allTimes);
        displayStatisticsSummary(testSizes, algorithmNames, allTimes,
                                 allStackDepths, allQueueLengths);
        textLog->append("\n详细统计测试完成！");
    } else {
        textLog->append("\n测试未产生有效数据。");
    }

    lblStatsInfo->setText("测试结束");
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
    // 清除所有系列
    QList<QAbstractSeries*> allSeries = chart->series();
    for (QAbstractSeries* series : allSeries) {
        chart->removeSeries(series);
        delete series;
    }

    // 清除所有坐标轴
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

void MyChartView::updateDetailedTrendChart(const QString& title,
                                           const QVector<QLineSeries*>& allSeries,
                                           const QVector<QLineSeries*>& errorSeries,
                                           const QVector<int>& testSizes,
                                           const QStringList& algorithmNames,
                                           const QVector<QVector<double>>& allTimes)
{
    clearChart();

    // 首先添加误差区域（作为背景）
    for (int i = 0; i < errorSeries.size(); i++) {
        QLineSeries* errorSerie = errorSeries[i];

        // 创建区域系列来显示误差范围
        QAreaSeries *areaSeries = new QAreaSeries(errorSerie);
        areaSeries->setName(errorSerie->name());
        areaSeries->setColor(errorSerie->color());
        areaSeries->setOpacity(0.1);
        areaSeries->setVisible(false); // 隐藏误差区域的图例
        chart->addSeries(areaSeries);
    }

    // 添加主折线，根据全局配置设置线型
    for (int i = 0; i < allSeries.size(); i++) {
        QLineSeries* series = allSeries[i];

        // 使用全局配置的线型
        bool isSolidLine = lineStyleConfig[i]; // 获取全局配置

        QPen pen(series->color());
        pen.setWidth(2);

        if (isSolidLine) {
            pen.setStyle(Qt::SolidLine); // 实线
            // 在算法名称后添加线型说明
            series->setName(algorithmNames[i] + " (实线)");
        } else {
            pen.setStyle(Qt::DashLine); // 虚线
            // 设置虚线样式
            pen.setDashPattern({4, 3}); // 4像素实线，3像素空白
            // 在算法名称后添加线型说明
            series->setName(algorithmNames[i] + " (虚线)");
        }

        series->setPen(pen);
        chart->addSeries(series);
    }

    // 创建坐标轴
    chart->createDefaultAxes();

    // 设置X轴
    QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setTitleText("节点数 (N)");
        axisX->setLabelFormat("%d");
        if (!testSizes.isEmpty()) {
            axisX->setRange(testSizes.first(), testSizes.last());
        }
    }

    // 设置Y轴
    QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if (axisY) {
        axisY->setTitleText("平均时间 (ms)");
        axisY->setLabelFormat("%.2f");

        // 计算Y轴范围
        double minTime = std::numeric_limits<double>::max();
        double maxTime = 0;

        for (const QVector<double>& times : allTimes) {
            for (double time : times) {
                if (time < minTime) minTime = time;
                if (time > maxTime) maxTime = time;
            }
        }

        if (minTime < maxTime) {
            // 设置Y轴范围，留出10%的余量
            axisY->setRange(0, maxTime * 1.1);
        }
    }

    chart->setTitle(title);
    chart->legend()->setVisible(true);

    // 在日志中添加线型配置说明
    textLog->append("\n线型配置：");
    for (int i = 0; i < algorithmNames.size(); i++) {
        QString lineType = lineStyleConfig[i] ? "实线" : "虚线";
        textLog->append(QString("  %1: %2").arg(algorithmNames[i], -10).arg(lineType));
    }
}

void MyChartView::displayStatisticsSummary(const QVector<int>& testSizes,
                                           const QStringList& algorithmNames,
                                           const QVector<QVector<double>>& allTimes,
                                           const QVector<QVector<double>>& allStackDepths,
                                           const QVector<QVector<double>>& allQueueLengths)
{
    textLog->append("\n=========== 统计摘要 ===========");

    // 查找最快的算法
    for (int i = 0; i < testSizes.size(); i++) {
        int n = testSizes[i];
        double minTime = std::numeric_limits<double>::max();
        int fastestAlg = -1;

        for (int alg = 0; alg < 7; alg++) {
            if (i < allTimes[alg].size() && allTimes[alg][i] < minTime) {
                minTime = allTimes[alg][i];
                fastestAlg = alg;
            }
        }

        if (fastestAlg >= 0) {
            textLog->append(QString("N=%1 时最快: %2 (%3 ms)")
                                .arg(n).arg(algorithmNames[fastestAlg]).arg(minTime, 0, 'f', 2));
        }
    }

    // 计算平均性能提升
    if (testSizes.size() >= 2) {
        textLog->append("\n=========== 性能分析 ===========");

        for (int alg = 0; alg < 7; alg++) {
            if (allTimes[alg].size() >= 2) {
                double firstTime = allTimes[alg].first();
                double lastTime = allTimes[alg].last();
                double growthRate = (lastTime - firstTime) / firstTime * 100;

                textLog->append(QString("%1: %2 → %3 ms (增长: %4%)")
                                    .arg(algorithmNames[alg], -10)
                                    .arg(firstTime, 6, 'f', 2)
                                    .arg(lastTime, 6, 'f', 2)
                                    .arg(growthRate, 6, 'f', 1));
            }
        }
    }
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
