#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QDebug>
#include <vector>
#include <stack>
#include <chartview.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("二叉树遍历的可视化演示与算法性能统计");
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
    QVBoxLayout *mainLayout = new QVBoxLayout(central);  // 创建时不指定父部件

    //选项卡
    QTabWidget *tabWidget = new QTabWidget();
    mainLayout->addWidget(tabWidget);

    // 第一部分：二叉树遍历的可视化演示
    QWidget *tabVis = new QWidget();
    QHBoxLayout *visLayout = new QHBoxLayout(tabVis);

    gv = new MyGraphicsView();
    connect(gv, &MyGraphicsView::reportStats, this, &MainWindow::updateStats);  //连接信号槽：当图形视图报告统计信息时，调用主窗口的更新统计函数
    visLayout->addWidget(gv, 7);    //添加到水平布局，拉伸因子为7（占据7/10的空间）

    QGroupBox *ctrlGroup = new QGroupBox("控制面板");
    QVBoxLayout *ctrlLayout = new QVBoxLayout(ctrlGroup);

    //第一块：生成树
    QGroupBox *boxGen = new QGroupBox("1. 生成树");
    QFormLayout *formGen = new QFormLayout(boxGen); //表单布局
    editNodeNum = new QLineEdit("15");
    QPushButton *btnAuto = new QPushButton("自动生成 (完全二叉树)");
    QPushButton *btnClear = new QPushButton("清空画布");
    QPushButton *btnManual = new QPushButton("手动模式");

    formGen->addRow("节点数:", editNodeNum);
    formGen->addRow(btnAuto);
    formGen->addRow(btnClear);
    formGen->addRow(btnManual);

    ctrlLayout->addWidget(boxGen);

    //第二块：遍历演示
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

    //第三块：统计
    QGroupBox *boxStats = new QGroupBox("3. 统计");
    QVBoxLayout *layoutStats = new QVBoxLayout(boxStats);
    labelStats = new QLabel("等待运行...");
    layoutStats->addWidget(labelStats);
    ctrlLayout->addWidget(boxStats);
    ctrlLayout->addStretch();

    //将控制面板添加到水平布局，拉伸因子为3（占据3/10的空间）
    visLayout->addWidget(ctrlGroup, 3);

    // //第二部分：演示
    // QWidget *tabPerf = new QWidget();
    // QVBoxLayout *perfLayout = new QVBoxLayout(tabPerf);

    // QHBoxLayout *topPerf = new QHBoxLayout();
    // editDataSize = new QLineEdit("1000000");
    // QPushButton *btnCompare = new QPushButton("单次对比");
    // QPushButton *btnTrend = new QPushButton("趋势图");
    // topPerf->addWidget(new QLabel("N:"));
    // topPerf->addWidget(editDataSize);
    // topPerf->addWidget(btnCompare);
    // topPerf->addWidget(btnTrend);
    // topPerf->addStretch();
    // perfLayout->addLayout(topPerf);

    // QHBoxLayout *botPerf = new QHBoxLayout();
    // textLog = new QTextEdit();
    // botPerf->addWidget(textLog, 3);

    // // 图表类型
    // chart = new QChart();
    // chartView = new QChartView(chart);
    // chartView->setRenderHint(QPainter::Antialiasing);
    // botPerf->addWidget(chartView, 7);

    // perfLayout->addLayout(botPerf);

    // tabWidget->addTab(tabVis, "演示");
    // tabWidget->addTab(tabPerf, "性能分析");

    // ========== 第二部分：性能分析 ==========
    QWidget *tabPerf = new QWidget();
    QVBoxLayout *perfLayout = new QVBoxLayout(tabPerf);

    // 创建 MyChartView 实例
    chartView = new MyChartView();
    perfLayout->addWidget(chartView);

    tabWidget->addTab(tabVis, "演示");
    tabWidget->addTab(tabPerf, "性能分析");

    // 连接信号槽
    connect(btnAuto, &QPushButton::clicked, this, &MainWindow::onAutoGenerate);
    connect(btnClear, &QPushButton::clicked, this, [=](){ gv->init(); });
    connect(btnRunVis, &QPushButton::clicked, this, &MainWindow::onRunVisual);

    connect(btnAuto, &QPushButton::clicked, this, &MainWindow::onAutoGenerate);
    connect(btnClear, &QPushButton::clicked, this, [=](){ gv->init(); });
    connect(btnRunVis, &QPushButton::clicked, this, &MainWindow::onRunVisual);
    // connect(btnCompare, &QPushButton::clicked, this, &MainWindow::onRunPerformance);
    // connect(btnTrend, &QPushButton::clicked, this, &MainWindow::onRunTrend);
}

//自动生成完全二叉树
void MainWindow::onAutoGenerate() {
    int n = editNodeNum->text().toInt();
    //不允许用户生成多于64个节点的树，保持界面稳定
    if(n > 64){
        QMessageBox::warning(this,
                             "节点数过多",
                             "节点数不能超过100，请减少节点数量以保持界面稳定。",
                             QMessageBox::Ok);
        return;
    }
    gv->autoCreateTree(n);
    labelStats->setText("生成完毕");
}

//开始运行
void MainWindow::onRunVisual() {

    // 先重置所有节点状态
    gv->resetAllNodeStates();

    int type = comboTraversal->currentIndex();
    bool isRec = checkRecursive->isChecked();

    //根据具体模式选择遍历方式
    if(type==0)
        isRec ? gv->preRecursive(gv->root) : gv->pre(gv->root);
    else if(type==1)
        isRec ? gv->inRecursive(gv->root) : gv->in(gv->root);
    else if(type==2)
        isRec ? gv->posRecursive(gv->root) : gv->pos(gv->root);
    else if(type==3)    //TODO:递归？
        gv->levelOrder(gv->root);
}

//改变统计标签显示
//TODO:参数如何传递
void MainWindow::updateStats(QString s) {
    labelStats->setText(s);
}

// //创建大树
// TreeNode* MainWindow::createBigTree(int n) {
//     if(n<=0) return nullptr;
//     std::vector<TreeNode*> nodes;
//     nodes.reserve(n);
//     for(int i=0; i<n; i++) nodes.push_back(new TreeNode(i));
//     for(int i=0; i<n/2; i++) {
//         if(2*i+1<n) nodes[i]->left=nodes[2*i+1];
//         if(2*i+2<n) nodes[i]->right=nodes[2*i+2];
//     }
//     return nodes[0];
// }

// //删除树
// void MainWindow::deleteTree(TreeNode* root) {
//     if(!root) return;
//     deleteTree(root->left); deleteTree(root->right); delete root;
// }

// //先序遍历递归
// void MainWindow::perfRecursive(TreeNode* root) {
//     if(!root) return; volatile int x=root->val; (void)x;
//     perfRecursive(root->left); perfRecursive(root->right);
// }

// //先序遍历非递归
// void MainWindow::perfIterative(TreeNode* root, int &maxStack) {
//     if(!root) return; std::stack<TreeNode*> s; s.push(root); maxStack=0;
//     while(!s.empty()){
//         if((int)s.size()>maxStack) maxStack=s.size();
//         TreeNode* c=s.top(); s.pop(); volatile int x=c->val; (void)x;
//         if(c->right) s.push(c->right); if(c->left) s.push(c->left);
//     }
// }

//TODO:包容多方式的遍历

//TODO:容器分离问题
//c++11 range-loop might detach Qt container (QList)
void MainWindow::onRunPerformance() {
    // //得到用户输入的数据大小
    // int n=editDataSize->text().toInt();
    // //创建包含n个节点的大树
    // TreeNode* root=createBigTree(n);
    // //测量递归遍历勇士
    // QElapsedTimer t; t.start(); perfRecursive(root); qint64 t1=t.nsecsElapsed();
    // int ms=0; t.restart(); perfIterative(root,ms); qint64 t2=t.nsecsElapsed();
    // textLog->append(QString("N=%1 Rec=%2ms Iter=%3ms Stack=%4").arg(n).arg(t1/1e6).arg(t2/1e6).arg(ms));

    // chart->removeAllSeries();
    // // 移除旧轴
    // //c++11 range-loop might detach Qt container (QList)
    // for(auto axis : chart->axes()) chart->removeAxis(axis);

    // // 直接使用 QBarSet, QBarSeries (无前缀)
    // QBarSet *s0=new QBarSet("Rec"); *s0<<t1/1e6;
    // QBarSet *s1=new QBarSet("Iter"); *s1<<t2/1e6;
    // QBarSeries *bs=new QBarSeries(); bs->append(s0); bs->append(s1);
    // chart->addSeries(bs); chart->createDefaultAxes();
    // deleteTree(root);
}

//TODO:容器分离问题
//c++11 range-loop might detach Qt container (QList)
void MainWindow::onRunTrend() {
    // chart->removeAllSeries();
    // //c++11 range-loop might detach Qt container (QList)
    // for(auto axis : chart->axes()) chart->removeAxis(axis);

    // // 直接使用 QLineSeries (无前缀)
    // QLineSeries *ls1=new QLineSeries(); ls1->setName("Rec");
    // QLineSeries *ls2=new QLineSeries(); ls2->setName("Iter");

    // // 简化循环次数以便演示
    // for(int n=100000; n<=300000; n+=100000){
    //     TreeNode* r=createBigTree(n);
    //     QElapsedTimer t; t.start(); perfRecursive(r); double d1=t.nsecsElapsed()/1e6;
    //     int m=0; t.restart(); perfIterative(r,m); double d2=t.nsecsElapsed()/1e6;
    //     ls1->append(n,d1); ls2->append(n,d2);
    //     deleteTree(r);
    //     QCoreApplication::processEvents();
    // }
    // chart->addSeries(ls1); chart->addSeries(ls2); chart->createDefaultAxes();
}
