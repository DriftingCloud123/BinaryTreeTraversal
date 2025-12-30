#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QtCharts>
#include "chartview.h"

// 【重要】删除 using namespace QtCharts;
// 【重要】你的环境似乎不需要这个，写了反而报错

#include "graphview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// //Node结构体
// struct TreeNode {
//     int val;
//     TreeNode *left = nullptr;
//     TreeNode *right = nullptr;
//     TreeNode(int x) : val(x) {}
// };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    MyGraphicsView *gv;

    // Tab 1 控件
    QLineEdit *editNodeNum;
    QComboBox *comboTraversal;
    QCheckBox *checkRecursive;
    QLabel *labelStats;

    // Tab 2 控件
    QLineEdit *editDataSize;
    QTextEdit *textLog;

    //Qt图表
    MyChartView *chartView;
    QChart *chart;

    void setupUiCustom();
    // TreeNode* createBigTree(int n);
    // void deleteTree(TreeNode* root);
    // void perfRecursive(TreeNode* root);
    // void perfIterative(TreeNode* root, int &maxStack);

private slots:
    void onAutoGenerate();
    void onRunVisual();
    void onRunPerformance();
    void onRunTrend();
    void updateStats(QString s);
};
#endif // MAINWINDOW_H
