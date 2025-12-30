#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QChart>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QElapsedTimer>


class MyChartView : public QWidget
{
    Q_OBJECT

public:
    explicit MyChartView(QWidget *parent = nullptr);
    ~MyChartView();

    // 单次对比性能测试
    void runPerformanceTest(int n);

    // 趋势图性能测试
    void runTrendTest();

    // 清空图表
    void clearChart();

private slots:
    void onCompareClicked();
    void onTrendClicked();

private:
    // UI组件
    QLineEdit *editDataSize;
    QPushButton *btnCompare;
    QPushButton *btnTrend;
    QTextEdit *textLog;
    QChart *chart;
    QChartView *chartView;

    // 性能测试辅助函数
    struct TreeNode {
        int val;
        TreeNode *left = nullptr;
        TreeNode *right = nullptr;
        TreeNode(int x) : val(x) {}
    };

    TreeNode* createBigTree(int n);
    void deleteTree(TreeNode* root);
    void perfRecursive(TreeNode* root);
    void perfIterative(TreeNode* root, int &maxStack);

    void setupUI();
};

#endif // MYCHARTVIEW_H

#endif // CHARTVIEW_H
