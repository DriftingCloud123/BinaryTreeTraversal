#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>
#include <QtCharts>
#include <vector>
#include "BinaryTree.cpp"


// // 遍历类型枚举
// typedef enum TraversalClass {
//     PRE,
//     IN,
//     POST,
//     LEVEL
// }TraversalClass;

// // 遍历统计结构体
// struct TraversalStats {
//     double time_ms;
//     size_t max_stack_depth;
//     size_t max_queue_length;
// };

// 前向声明
template<typename T>
class BinaryTree;

template<typename T>
class TreeNode;

class MyChartView : public QWidget
{
    Q_OBJECT

public:
    explicit MyChartView(QWidget *parent = nullptr);
    ~MyChartView();

    // 访问节点计数（用于统计）
    static int visitCount;

    // 算法线型配置
    static const std::vector<bool> lineStyleConfig; // true=实线, false=虚线

private slots:
    void onCompareClicked();
    void onTrendClicked();
    void onQuickTrendClicked();

private:
    void setupUI();
    void runPerformanceTest(int n, TraversalClass traversalType);
    void runDetailedTrendTest(int minNodes, int maxNodes, int stepSize, int repeatTimes);
    TraversalStats performSingleAlgorithm(BinaryTree<int>* tree, TraversalClass traversalType, bool isRecursive);

    QString getTraversalTypeName(TraversalClass type) const;
    void clearChart();
    void updateBarChart(const QString& title, const QVector<QString>& algorithmNames,
                        const QVector<double>& times, int n);
    void updateDetailedTrendChart(const QString& title,
                                  const QVector<QLineSeries*>& allSeries,
                                  const QVector<QLineSeries*>& errorSeries,
                                  const QVector<int>& testSizes,
                                  const QStringList& algorithmNames,
                                  const QVector<QVector<double>>& allTimes);
    void displayStatisticsSummary(const QVector<int>& testSizes,
                                  const QStringList& algorithmNames,
                                  const QVector<QVector<double>>& allTimes,
                                  const QVector<QVector<double>>& allStackDepths,
                                  const QVector<QVector<double>>& allQueueLengths);

    // 二叉树操作
    BinaryTree<int>* createBigTree(int n);
    void deleteTree(BinaryTree<int>* tree);
    static void visitNodeForStats(TreeNode<int>* node);

private:
    // UI 控件
    QLineEdit *editDataSize;
    QLineEdit *editMinNodes;
    QLineEdit *editMaxNodes;
    QLineEdit *editStepSize;
    QLineEdit *editRepeatTimes;
    QComboBox *comboTraversalType;
    QPushButton *btnCompare;
    QPushButton *btnTrend;
    QPushButton *btnQuickTrend;
    QLabel *lblStatsInfo;
    QTextEdit *textLog;

    // 图表相关
    QChart *chart;
    QChartView *chartView;
};

#endif // CHARTVIEW_H
