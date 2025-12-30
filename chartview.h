#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QChart>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QElapsedTimer>
#include <QAbstractAxis>
#include <QGraphicsLayout>


// 遍历算法类型枚举
enum TraversalType {
    TYPE_PREORDER = 0,      // 先序
    TYPE_INORDER,           // 中序
    TYPE_POSTORDER,         // 后序
    TYPE_LEVELORDER         // 层序
};

// 详细算法枚举（用于内部实现）
typedef enum AlgorithmDetail {
    ALG_PREORDER_RECURSIVE = 0,    // 先序遍历递归
    ALG_PREORDER_ITERATIVE,        // 先序遍历非递归
    ALG_INORDER_RECURSIVE,         // 中序遍历递归
    ALG_INORDER_ITERATIVE,         // 中序遍历非递归
    ALG_POSTORDER_RECURSIVE,       // 后序遍历递归
    ALG_POSTORDER_ITERATIVE,       // 后序遍历非递归
    ALG_LEVELORDER                 // 层序遍历
} AlgorithmDetail;

class MyChartView : public QWidget
{
    Q_OBJECT

public:
    explicit MyChartView(QWidget *parent = nullptr);
    ~MyChartView();

    // 单次对比性能测试（测试指定遍历类型的递归和非递归）
    void runPerformanceTest(int n, TraversalType traversalType);

    // 趋势图性能测试（测试所有7种算法）
    void runTrendTest();

    // 清空图表
    void clearChart();

    // 获取遍历类型名称
    QString getTraversalTypeName(TraversalType type) const;

    // 获取算法详细名称
    QString getAlgorithmDetailName(AlgorithmDetail algorithm) const;

private slots:
    void onCompareClicked();
    void onTrendClicked();

private:
    // UI组件
    QLineEdit *editDataSize;
    QComboBox *comboTraversalType;  // 遍历类型选择下拉框（4个选项）
    QPushButton *btnCompare;
    QPushButton *btnTrend;
    QTextEdit *textLog;
    QChart *chart;
    QChartView *chartView;

    // 性能测试辅助函数
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    };

    // 二叉树创建和删除
    TreeNode* createBigTree(int n);
    void deleteTree(TreeNode* root);

    // 7种遍历算法的实现
    void perfPreorderRecursive(TreeNode* root);
    void perfPreorderIterative(TreeNode* root, int &maxStack);
    void perfInorderRecursive(TreeNode* root);
    void perfInorderIterative(TreeNode* root, int &maxStack);
    void perfPostorderRecursive(TreeNode* root);
    void perfPostorderIterative(TreeNode* root, int &maxStack);
    void perfLevelorder(TreeNode* root, int &maxQueue);

    // 执行单个算法并返回时间和辅助数据
    double performSingleAlgorithm(TreeNode* root, AlgorithmDetail algorithm, int &auxData);

    void setupUI();
};

#endif // CHARTVIEW_H
