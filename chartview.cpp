#include "chartview.h"
#include <vector>
#include <stack>
#include <QMessageBox>
#include <QCoreApplication>

MyChartView::MyChartView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

MyChartView::~MyChartView()
{
    // 清理图表资源
    if (chart) {
        delete chart;
    }
}

void MyChartView::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 顶部控制面板
    QHBoxLayout *topLayout = new QHBoxLayout();
    editDataSize = new QLineEdit("1000000");
    btnCompare = new QPushButton("单次对比");
    btnTrend = new QPushButton("趋势图");

    topLayout->addWidget(new QLabel("N:"));
    topLayout->addWidget(editDataSize);
    topLayout->addWidget(btnCompare);
    topLayout->addWidget(btnTrend);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 底部内容区域
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    textLog = new QTextEdit();

    // 图表区域
    chart = new QChart();
    chart->setTitle("性能对比分析");
    chart->setAnimationOptions(QChart::SeriesAnimations);

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
    runPerformanceTest(n);
}

void MyChartView::onTrendClicked()
{
    runTrendTest();
}

void MyChartView::runPerformanceTest(int n)
{
    // 创建测试树
    TreeNode* root = createBigTree(n);
    if (!root) {
        textLog->append("创建测试树失败！");
        return;
    }

    // 测量递归遍历时间
    QElapsedTimer timer;
    timer.start();
    perfRecursive(root);
    qint64 recursiveTime = timer.nsecsElapsed();

    // 测量迭代遍历时间和最大栈深度
    int maxStack = 0;
    timer.restart();
    perfIterative(root, maxStack);
    qint64 iterativeTime = timer.nsecsElapsed();

    // 输出结果
    QString result = QString("N=%1 | 递归: %2 ms | 迭代: %3 ms | 最大栈深: %4")
                         .arg(n)
                         .arg(recursiveTime / 1e6, 0, 'f', 2)
                         .arg(iterativeTime / 1e6, 0, 'f', 2)
                         .arg(maxStack);

    textLog->append(result);

    // 更新图表
    clearChart();

    QBarSet *recursiveSet = new QBarSet("递归遍历");
    QBarSet *iterativeSet = new QBarSet("迭代遍历");

    *recursiveSet << recursiveTime / 1e6;
    *iterativeSet << iterativeTime / 1e6;

    QBarSeries *series = new QBarSeries();
    series->append(recursiveSet);
    series->append(iterativeSet);

    chart->addSeries(series);
    chart->setTitle(QString("节点数 N=%1 的性能对比").arg(n));

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
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 清理内存
    deleteTree(root);
}

void MyChartView::runTrendTest()
{
    int n = editDataSize->text().toInt();

    clearChart();

    // 创建趋势线
    QLineSeries *recursiveSeries = new QLineSeries();
    recursiveSeries->setName("递归遍历");

    QLineSeries *iterativeSeries = new QLineSeries();
    iterativeSeries->setName("迭代遍历");

    // 测试不同的节点数量
    textLog->append("开始趋势测试...");
    QVector<int> testSizes = {n, n*2, n*3, n*4, n*5};

    for (int n : testSizes) {
        TreeNode* root = createBigTree(n);
        if (!root) continue;

        // 递归测试
        QElapsedTimer timer;
        timer.start();
        perfRecursive(root);
        qint64 recursiveTime = timer.nsecsElapsed();

        // 迭代测试
        int maxStack = 0;
        timer.restart();
        perfIterative(root, maxStack);
        qint64 iterativeTime = timer.nsecsElapsed();

        // 添加数据点
        recursiveSeries->append(n, recursiveTime / 1e6);
        iterativeSeries->append(n, iterativeTime / 1e6);

        QString result = QString("N=%1: 递归=%2ms, 迭代=%3ms, 栈深=%4")
                             .arg(n)
                             .arg(recursiveTime / 1e6, 0, 'f', 2)
                             .arg(iterativeTime / 1e6, 0, 'f', 2)
                             .arg(maxStack);
        textLog->append(result);

        deleteTree(root);

        // 处理事件，保持UI响应
        QCoreApplication::processEvents();
    }

    // 添加到图表
    chart->addSeries(recursiveSeries);
    chart->addSeries(iterativeSeries);
    chart->setTitle("遍历算法性能趋势分析");

    // 创建坐标轴
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setTitleText("节点数 (N)");
    chart->axes(Qt::Vertical).first()->setTitleText("时间 (ms)");

    textLog->append("趋势测试完成！");
}

void MyChartView::clearChart()
{
    chart->removeAllSeries();

    // 移除所有坐标轴
    for (QAbstractAxis* axis : chart->axes()) {
        chart->removeAxis(axis);
    }
}

// 创建大树
MyChartView::TreeNode* MyChartView::createBigTree(int n)
{
    if (n <= 0) return nullptr;

    std::vector<TreeNode*> nodes;
    nodes.reserve(n);

    for (int i = 0; i < n; i++) {
        nodes.push_back(new TreeNode(i));
    }

    // 构建完全二叉树
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

// 删除树
void MyChartView::deleteTree(TreeNode* root)
{
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// 递归遍历
void MyChartView::perfRecursive(TreeNode* root)
{
    if (!root) return;
    volatile int x = root->val;
    (void)x; // 防止编译器优化掉
    perfRecursive(root->left);
    perfRecursive(root->right);
}

// 迭代遍历（先序）
void MyChartView::perfIterative(TreeNode* root, int &maxStack)
{
    if (!root) return;

    std::stack<TreeNode*> s;
    s.push(root);
    maxStack = 0;

    while (!s.empty()) {
        // 更新最大栈深度
        if ((int)s.size() > maxStack) {
            maxStack = s.size();
        }

        TreeNode* current = s.top();
        s.pop();

        volatile int x = current->val;
        (void)x; // 防止编译器优化掉

        // 注意：栈是LIFO，所以先压右孩子，再压左孩子
        if (current->right) {
            s.push(current->right);
        }
        if (current->left) {
            s.push(current->left);
        }
    }
}
