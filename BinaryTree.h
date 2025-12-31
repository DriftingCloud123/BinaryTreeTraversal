#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <string>
#include <vector>

// using namespace std;

/*
* 遍历类型：
* 先序 Pre     =0
* 中序 IN      =1
* 后序 POST    =2
* 层序 LEVEL   =3
*/

enum TraversalClass {
    PRE,
    IN,
    POST,
    LEVEL,
};

// 统计信息结构体
struct TraversalStats {
    double time_ms = 0.0;        // 遍历时间(毫秒)
    size_t memory_usage = 0;     // 内存使用情况(字节)
    size_t max_queue_length = 0; // 层序遍历最长队列长度
    size_t max_stack_depth = 0;  // 非递归遍历最大栈深度

    void print() const;
};

// 二叉树节点
template<typename T>
struct TreeNode {
    T data;
    TreeNode* left;
    TreeNode* right;

    TreeNode(T val) : data(val), left(nullptr), right(nullptr) {}
};

// 二叉树类声明
template<typename T>
class BinaryTree {
private:
    TreeNode<T>* root;

    // 递归辅助函数
    void preorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*));
    void inorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*));
    void postorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*));
    void levelorderRecursiveHelper(TreeNode<T>* node, int level, void (*visit)(TreeNode<T>*));

    // 计算树的高度
    int getHeight(TreeNode<T>* node);

    // 清空树
    void clearTree(TreeNode<T>* node);

    // 从先序序列构建树的辅助函数
    TreeNode<T>* buildFromPreorder(std::vector<std::string>& nodes, size_t& index);

    // 先序序列化树的辅助函数
    void preorderSerializeHelper(TreeNode<T>* node, std::vector<std::string>& result);

public:
    // 构造函数和析构函数
    BinaryTree();
    ~BinaryTree();

    // 树操作

    // 遍历方法
    TraversalStats Traversal(TraversalClass traversal_class, bool is_recursive, void (*visit)(TreeNode<T>*));

    void autoCreateTree(int n);

    //先序递归遍历
    TraversalStats preorderRecursive(void (*visit)(TreeNode<T>*));
    //中序递归遍历
    TraversalStats inorderRecursive(void (*visit)(TreeNode<T>*));
    //后序递归遍历
    TraversalStats postorderRecursive(void (*visit)(TreeNode<T>*));
    //层序遍历
    TraversalStats levelOrderRecursive(void (*visit)(TreeNode<T>*));
    //先序非递归
    TraversalStats preorderNonRecursive(void (*visit)(TreeNode<T>*));
    //中序非递归
    TraversalStats inorderNonRecursive(void (*visit)(TreeNode<T>*));
    //后序非递归
    TraversalStats postorderNonRecursive(void (*visit)(TreeNode<T>*));
    //层级遍历
    TraversalStats levelorderNonRecursive(void (*visit)(TreeNode<T>*));
};

// 声明模板类的实例化（如果需要）
extern template class BinaryTree<int>;
extern template class BinaryTree<float>;
extern template class BinaryTree<double>;

#endif // BINARYTREE_H
