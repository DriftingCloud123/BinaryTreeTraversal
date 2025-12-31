#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QElapsedTimer>
#include <algorithm>
#include "BinaryTree.cpp"

    class MyChartView : public QWidget
{
    Q_OBJECT

public:
    explicit MyChartView(QWidget *parent = nullptr);
    ~MyChartView();

private slots:
    void onCompareClicked();
    void onTrendClicked();

private:
    void setupUI();
    void runPerformanceTest(int n, TraversalClass traversalType);
    void runTrendTest();
    void clearChart();

    // 创建二叉树
    BinaryTree<int>* createBigTree(int n);
    // 删除二叉树
    void deleteTree(BinaryTree<int>* tree);

    // 获取遍历类型名称
    QString getTraversalTypeName(TraversalClass type) const;

    // 执行单个算法测试
    TraversalStats performSingleAlgorithm(BinaryTree<int>* tree, TraversalClass traversalType, bool isRecursive);

    // 用于统计的访问函数
    static void visitNodeForStats(TreeNode<int>* node);

    // 更新图表（柱状图对比）
    void updateBarChart(const QString& title, const QVector<QString>& algorithmNames,
                        const QVector<double>& times, int n);

    // 更新图表（趋势图）
    void updateTrendChart(const QString& title, const QVector<QLineSeries*>& allSeries,
                          const QVector<int>& testSizes);

private:
    // UI组件
    QChart *chart;
    QChartView *chartView;
    QLineEdit *editDataSize;
    QComboBox *comboTraversalType;
    QPushButton *btnCompare;
    QPushButton *btnTrend;
    QTextEdit *textLog;

    // 统计变量
    static int visitCount;
};

#endif // CHARTVIEW_H
