#include <iostream>
#include <queue>
#include <stack>
#include <chrono>
#include <algorithm>
#include <vector>

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

    void print() const {
        std::cout << "遍历时间: " << time_ms << " ms" << std::endl;
        std::cout << "内存使用: " << memory_usage << " bytes" << std::endl;
        std::cout << "最长队列长度: " << max_queue_length << std::endl;
        std::cout << "最大栈深度: " << max_stack_depth << std::endl;
    }
};

// 二叉树节点
template<typename T>
struct TreeNode {
    T data;
    TreeNode* left;
    TreeNode* right;

    TreeNode(T val) : data(val), left(nullptr), right(nullptr) {}
};

// 二叉树类
template<typename T>
class BinaryTree {
private:
    TreeNode<T>* root;

    // 按完全二叉树索引方式构建二叉树
    void autoCreateTreeByIndex(int n, T defaultValue = T()) {
        // 清空当前树
        clearTree(root);
        root = nullptr;

        if (n <= 0) return;

        // 创建所有节点，存储到数组中
        std::vector<TreeNode<T>*> nodes(n, nullptr);

        // 创建所有节点
        for (int i = 0; i < n; i++) {
            T value;
            if constexpr (std::is_same<T, int>::value || std::is_same<T, char>::value) {
                value = static_cast<T>(i);
            } else if constexpr (std::is_same<T, std::string>::value) {
                value = "V" + std::to_string(i);
            } else {
                value = defaultValue;
            }
            nodes[i] = new TreeNode<T>(value);
        }

        // 建立节点间的连接关系（完全二叉树的特性）
        for (int i = 0; i < n; i++) {
            int leftChildIdx = 2 * i + 1;
            int rightChildIdx = 2 * i + 2;

            if (leftChildIdx < n) {
                nodes[i]->left = nodes[leftChildIdx];
            }
            if (rightChildIdx < n) {
                nodes[i]->right = nodes[rightChildIdx];
            }
        }

        // 设置根节点
        root = nodes[0];
    }

    // 计算树的高度
    int getHeight(TreeNode<T>* node) {
        if (!node) return 0;
        return 1 + std::max(getHeight(node->left), getHeight(node->right));
    }

    // 清空树
    void clearTree(TreeNode<T>* node) {
        if (!node) return;
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }

public:
    // 默认构造函数
    BinaryTree() : root(nullptr) {}

    // // 构造函数：从CSV文件加载树
    // BinaryTree(const std::string& filename) : root(nullptr) {
    //     loadFromCSV(filename);
    // }

    // 析构函数
    ~BinaryTree() {
        clearTree(root);
    }

    TraversalStats Traversal(TraversalClass traversal_class, bool is_recursive, void (*visit)(TreeNode<T>*)) {
        TraversalStats stats;   //状态记录
        auto start = std::chrono::high_resolution_clock::now(); //开始计时

        //是递归
        if (is_recursive) {
            switch (traversal_class) {
            case PRE:
                preorderRecursiveHelper(root, visit);   //进行前序遍历
                break;
            case IN:
                inorderRecursiveHelper(root, visit);    //进行中序遍历
                break;
            case POST:
                postorderRecursiveHelper(root, visit);  //进行后序遍历
                break;
            case LEVEL:
                //无视递归信号，直接非递归
                levelorderNonRecursive(visit); //进行层序遍历
                break;
            }
        }
        //非递归
        else {
            switch (traversal_class) {
            case PRE:
                preorderNonRecursive(visit);   //进行前序遍历
                break;
            case IN:
                inorderNonRecursive(visit);    //进行中序遍历
                break;
            case POST:
                postorderNonRecursive(visit);  //进行后序遍历
                break;
            case LEVEL:
                levelorderNonRecursive(visit); //进行层序遍历
                break;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        stats.time_ms = duration.count() / 1000.0;

        stats.memory_usage = getHeight(root) * sizeof(TreeNode<T>*) * 2;
        stats.max_stack_depth = getHeight(root);

        return stats;
    }

    /*——————————————————————————————————*/
    // 递归辅助函数
    void preorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*)) {
        if (!node) return;
        visit(node);
        preorderRecursiveHelper(node->left, visit);
        preorderRecursiveHelper(node->right, visit);
    }

    void inorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*)) {
        if (!node) return;
        inorderRecursiveHelper(node->left, visit);
        visit(node);
        inorderRecursiveHelper(node->right, visit);
    }

    void postorderRecursiveHelper(TreeNode<T>* node, void (*visit)(TreeNode<T>*)) {
        if (!node) return;
        postorderRecursiveHelper(node->left, visit);
        postorderRecursiveHelper(node->right, visit);
        visit(node);
    }
    /*——————————————————————————————————*/

    // 前序遍历（递归）
    void preorderRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        preorderRecursiveHelper(root, visit);   //进行递归调用
        /*——————*/
    }

    // 中序遍历（递归）
    void inorderRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        inorderRecursiveHelper(root, visit);
        /*——————*/
    }

    // 后序遍历（递归）
    void postorderRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        postorderRecursiveHelper(root, visit);
        /*——————*/
    }

    // 前序非递归
    void preorderNonRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        std::stack<TreeNode<T>*> stack;
        TreeNode<T>* current = root;
        size_t maxDepth = 0;

        while (current || !stack.empty()) {
            maxDepth = std::max(maxDepth, stack.size());

            while (current) {
                visit(current);
                stack.push(current);
                current = current->left;
                maxDepth = std::max(maxDepth, stack.size());
            }

            current = stack.top();
            stack.pop();
            current = current->right;
        }
        /*——————*/
    }

    // 中序非递归
    void inorderNonRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        std::stack<TreeNode<T>*> stack;
        TreeNode<T>* current = root;
        size_t maxDepth = 0;

        while (current || !stack.empty()) {
            maxDepth = std::max(maxDepth, stack.size());

            while (current) {
                stack.push(current);
                current = current->left;
                maxDepth = std::max(maxDepth, stack.size());
            }

            current = stack.top();
            stack.pop();
            visit(current);
            current = current->right;
        }
        /*——————*/
    }

    // 后序非递归
    void postorderNonRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        std::stack<TreeNode<T>*> stack;
        TreeNode<T>* current = root;
        TreeNode<T>* lastVisited = nullptr;
        size_t maxDepth = 0;

        while (current || !stack.empty()) {
            maxDepth = std::max(maxDepth, stack.size());

            while (current) {
                stack.push(current);
                current = current->left;
                maxDepth = std::max(maxDepth, stack.size());
            }

            TreeNode<T>* peekNode = stack.top();

            if (peekNode->right && lastVisited != peekNode->right) {
                current = peekNode->right;
            }
            else {
                visit(peekNode);
                lastVisited = peekNode;
                stack.pop();
            }
        }
        /*——————*/
    }

    // 层序遍历
    void levelorderNonRecursive(void (*visit)(TreeNode<T>*)) {
        /*——————*/
        if (!root) return;

        std::queue<TreeNode<T>*> q;
        q.push(root);
        size_t maxQueueLength = 0;

        while (!q.empty()) {
            maxQueueLength = std::max(maxQueueLength, q.size());

            TreeNode<T>* current = q.front();
            q.pop();
            visit(current);

            if (current->left) q.push(current->left);
            if (current->right) q.push(current->right);
        }
        /*——————*/
    }

    //TODO:问题检查：
    // 在BinaryTree类的public部分添加：

    // 设置根节点
    void setRoot(TreeNode<T>* newRoot) {
        // 先清空旧树
        clearTree(root);
        root = newRoot;
    }

    // 自动创建完全二叉树
    void autoCreateTree(int n) {
        // 先清空当前树
        clearTree(root);
        root = nullptr;

        if (n <= 0) return;

        // 创建根节点
        root = new TreeNode<T>(T(0));

        if (n == 1) return;

        // 使用队列来帮助按层创建节点
        std::queue<TreeNode<T>*> nodeQueue;
        nodeQueue.push(root);

        int createdCount = 1;

        while (!nodeQueue.empty() && createdCount < n) {
            TreeNode<T>* parent = nodeQueue.front();
            nodeQueue.pop();

            // 创建左子节点
            if (createdCount < n) {
                TreeNode<T>* leftChild = new TreeNode<T>(T(createdCount));
                parent->left = leftChild;
                nodeQueue.push(leftChild);
                createdCount++;
            }

            // 创建右子节点
            if (createdCount < n) {
                TreeNode<T>* rightChild = new TreeNode<T>(T(createdCount));
                parent->right = rightChild;
                nodeQueue.push(rightChild);
                createdCount++;
            }
        }
    }

    // 递归的层序遍历助手函数
    void levelorderRecursiveHelper(TreeNode<T>* node, int level, void (*visit)(TreeNode<T>*)) {
        if (!node || level < 0) return;

        if (level == 0) {
            visit(node);
        } else {
            levelorderRecursiveHelper(node->left, level - 1, visit);
            levelorderRecursiveHelper(node->right, level - 1, visit);
        }
    }

    // 递归层序遍历（通过多次调用不同层级的递归实现）
    TraversalStats levelOrderRecursive(void (*visit)(TreeNode<T>*)) {
        TraversalStats stats;
        auto start = std::chrono::high_resolution_clock::now();

        int height = getHeight(root);
        for (int level = 0; level < height; level++) {
            levelorderRecursiveHelper(root, level, visit);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        stats.time_ms = duration.count() / 1000.0;

        stats.memory_usage = height * sizeof(TreeNode<T>*) * 2;
        stats.max_stack_depth = height;

        return stats;
    }
};

