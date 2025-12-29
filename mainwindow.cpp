#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QDebug>

// 【关键修改】删除 using namespace QtCharts;
// 你的环境里这些类似乎在全局命名空间

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Binary Tree Visualizer & Analyzer");
    this->resize(1600, 950);
    setupUiCustom();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiCustom()
{
    QWidget *central = new QWidget(this);
    this->setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QTabWidget *tabWidget = new QTabWidget();
    mainLayout->addWidget(tabWidget);

    // --- TAB 1: Visualization ---
    QWidget *tabVis = new QWidget();
    QHBoxLayout *visLayout = new QHBoxLayout(tabVis);

    gv = new MyGraphicsView();
    connect(gv, &MyGraphicsView::reportStats, this, &MainWindow::updateStats);
    visLayout->addWidget(gv, 7);

    QGroupBox *ctrlGroup = new QGroupBox("控制面板");
    QVBoxLayout *ctrlLayout = new QVBoxLayout(ctrlGroup);

    QGroupBox *boxGen = new QGroupBox("1. 生成树");
    QFormLayout *formGen = new QFormLayout(boxGen);
    editNodeNum = new QLineEdit("15");
    QPushButton *btnAuto = new QPushButton("自动生成 (完全二叉树)");
    QPushButton *btnClear = new QPushButton("清空画布");
    QPushButton *btnManual = new QPushButton("手动模式");

    formGen->addRow("节点数:", editNodeNum);
    formGen->addRow(btnAuto);
    formGen->addRow(btnClear);
    formGen->addRow(btnManual);

    ctrlLayout->addWidget(boxGen);

    QGroupBox *boxRun = new QGroupBox("2. 遍历演示");
    QVBoxLayout *layoutRun = new QVBoxLayout(boxRun);
    comboTraversal = new QComboBox();
    comboTraversal->addItems({"先序遍历", "中序遍历", "后序遍历", "层序遍历"});
    checkRecursive = new QCheckBox("递归模式");
    checkRecursive->setChecked(true);
    QPushButton *btnRunVis = new QPushButton("运行动画");
    layoutRun->addWidget(comboTraversal);
    layoutRun->addWidget(checkRecursive);
    layoutRun->addWidget(btnRunVis);
    ctrlLayout->addWidget(boxRun);

    QGroupBox *boxStats = new QGroupBox("3. 统计");
    QVBoxLayout *layoutStats = new QVBoxLayout(boxStats);
    labelStats = new QLabel("等待运行...");
    layoutStats->addWidget(labelStats);
    ctrlLayout->addWidget(boxStats);
    ctrlLayout->addStretch();

    visLayout->addWidget(ctrlGroup, 3);

    // --- TAB 2: Performance ---
    QWidget *tabPerf = new QWidget();
    QVBoxLayout *perfLayout = new QVBoxLayout(tabPerf);

    QHBoxLayout *topPerf = new QHBoxLayout();
    editDataSize = new QLineEdit("1000000");
    QPushButton *btnCompare = new QPushButton("单次对比");
    QPushButton *btnTrend = new QPushButton("趋势图");
    topPerf->addWidget(new QLabel("N:"));
    topPerf->addWidget(editDataSize);
    topPerf->addWidget(btnCompare);
    topPerf->addWidget(btnTrend);
    topPerf->addStretch();
    perfLayout->addLayout(topPerf);

    QHBoxLayout *botPerf = new QHBoxLayout();
    textLog = new QTextEdit();
    botPerf->addWidget(textLog, 3);

    // 直接使用 QChart，不需要前缀
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    botPerf->addWidget(chartView, 7);

    perfLayout->addLayout(botPerf);

    tabWidget->addTab(tabVis, "演示");
    tabWidget->addTab(tabPerf, "性能分析");

    connect(btnAuto, &QPushButton::clicked, this, &MainWindow::onAutoGenerate);
    connect(btnClear, &QPushButton::clicked, [=](){ gv->init(); });
    connect(btnRunVis, &QPushButton::clicked, this, &MainWindow::onRunVisual);
    connect(btnCompare, &QPushButton::clicked, this, &MainWindow::onRunPerformance);
    connect(btnTrend, &QPushButton::clicked, this, &MainWindow::onRunTrend);
}

void MainWindow::onAutoGenerate() {
    int n = editNodeNum->text().toInt();
    if(n>63) return;
    gv->autoCreateTree(n);
    labelStats->setText("生成完毕");
}

void MainWindow::onRunVisual() {
    int type = comboTraversal->currentIndex();
    bool isRec = checkRecursive->isChecked();
    if(type==0) isRec ? gv->preRecursive(gv->root) : gv->pre(gv->root);
    else if(type==1) isRec ? gv->inRecursive(gv->root) : gv->in(gv->root);
    else if(type==2) isRec ? gv->posRecursive(gv->root) : gv->pos(gv->root);
    else if(type==3) gv->levelOrder(gv->root);
}

void MainWindow::updateStats(QString s) { labelStats->setText(s); }

TreeNode* MainWindow::createBigTree(int n) {
    if(n<=0) return nullptr;
    std::vector<TreeNode*> nodes;
    nodes.reserve(n);
    for(int i=0; i<n; i++) nodes.push_back(new TreeNode(i));
    for(int i=0; i<n/2; i++) {
        if(2*i+1<n) nodes[i]->left=nodes[2*i+1];
        if(2*i+2<n) nodes[i]->right=nodes[2*i+2];
    }
    return nodes[0];
}

void MainWindow::deleteTree(TreeNode* root) {
    if(!root) return;
    deleteTree(root->left); deleteTree(root->right); delete root;
}

void MainWindow::perfRecursive(TreeNode* root) {
    if(!root) return; volatile int x=root->val; (void)x;
    perfRecursive(root->left); perfRecursive(root->right);
}

void MainWindow::perfIterative(TreeNode* root, int &maxStack) {
    if(!root) return; std::stack<TreeNode*> s; s.push(root); maxStack=0;
    while(!s.empty()){
        if((int)s.size()>maxStack) maxStack=s.size();
        TreeNode* c=s.top(); s.pop(); volatile int x=c->val; (void)x;
        if(c->right) s.push(c->right); if(c->left) s.push(c->left);
    }
}

void MainWindow::onRunPerformance() {
    int n=editDataSize->text().toInt();
    TreeNode* root=createBigTree(n);
    QElapsedTimer t; t.start(); perfRecursive(root); qint64 t1=t.nsecsElapsed();
    int ms=0; t.restart(); perfIterative(root,ms); qint64 t2=t.nsecsElapsed();
    textLog->append(QString("N=%1 Rec=%2ms Iter=%3ms Stack=%4").arg(n).arg(t1/1e6).arg(t2/1e6).arg(ms));

    chart->removeAllSeries();
    // 移除旧轴
    for(auto axis : chart->axes()) chart->removeAxis(axis);

    // 直接使用 QBarSet, QBarSeries (无前缀)
    QBarSet *s0=new QBarSet("Rec"); *s0<<t1/1e6;
    QBarSet *s1=new QBarSet("Iter"); *s1<<t2/1e6;
    QBarSeries *bs=new QBarSeries(); bs->append(s0); bs->append(s1);
    chart->addSeries(bs); chart->createDefaultAxes();
    deleteTree(root);
}

void MainWindow::onRunTrend() {
    chart->removeAllSeries();
    for(auto axis : chart->axes()) chart->removeAxis(axis);

    // 直接使用 QLineSeries (无前缀)
    QLineSeries *ls1=new QLineSeries(); ls1->setName("Rec");
    QLineSeries *ls2=new QLineSeries(); ls2->setName("Iter");

    // 简化循环次数以便演示
    for(int n=100000; n<=300000; n+=100000){
        TreeNode* r=createBigTree(n);
        QElapsedTimer t; t.start(); perfRecursive(r); double d1=t.nsecsElapsed()/1e6;
        int m=0; t.restart(); perfIterative(r,m); double d2=t.nsecsElapsed()/1e6;
        ls1->append(n,d1); ls2->append(n,d2);
        deleteTree(r);
        QCoreApplication::processEvents();
    }
    chart->addSeries(ls1); chart->addSeries(ls2); chart->createDefaultAxes();
}
