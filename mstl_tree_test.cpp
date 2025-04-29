#include <iostream>
#include <string>
#include "mstl_tree.h"
#include "mstl_pair.h"

// 用于测试的键值提取器
struct KeyOfValue {
    int operator()(const int& x) const { return x; }
    std::string operator()(const std::string& x) const { return x; }
    int operator()(const mstl::Pair<int, std::string>& x) const { return x.first; }
};

// 用于测试的比较函数
struct Compare {
    bool operator()(const int& a, const int& b) const { return a < b; }
    bool operator()(const std::string& a, const std::string& b) const { return a < b; }
    bool operator()(const mstl::Pair<int, std::string>& a, const mstl::Pair<int, std::string>& b) const {
        return a.first < b.first;
    }
};

// 使用迭代器打印红黑树
template <typename Tree>
void print_tree_with_iterator(const Tree& tree) {
    std::cout << "使用迭代器打印树的内容: ";
    for (const auto& value : tree) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

void test_basic_operations() {
    std::cout << "测试基本操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> tree;
    try {
        // 测试插入
        tree.insert_unique(1);
        tree.insert_unique(2);
        tree.insert_unique(3);
        tree.insert_unique(4);
        tree.insert_unique(5);

        // 使用迭代器打印树的内容
        print_tree_with_iterator(tree);
        
        // 测试查找
        auto it = tree.find(3);
        if (it != tree.end()) {
            std::cout << "找到元素: " << *it << std::endl;
        }
        
        // 测试大小
        std::cout << "树的大小: " << tree.size() << std::endl;
        
        // 测试遍历
        std::cout << "树的内容: ";
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 打印完整树结构
        std::cout << "完整树结构：" << std::endl;
        print_tree_with_iterator(tree);
    } catch(...) {
        std::cerr << "test_basic_operations 发生异常" << std::endl;
    }
    // 确保在函数结束时清理资源
    tree.clear();
}

void test_pair_operations() {
    std::cout << "\n测试pair操作..." << std::endl;
    
    mstl::RbTree<int, mstl::Pair<int, std::string>, KeyOfValue, Compare> tree;
    try {
        // 插入pair
        tree.insert_unique(mstl::make_pair(1, "one"));
        tree.insert_unique(mstl::make_pair(2, "two"));
        tree.insert_unique(mstl::make_pair(3, "three"));
        
        // 查找pair
        auto it = tree.find(2);
        if (it != tree.end()) {
            std::cout << "找到pair: (" << it->first << ", " << it->second << ")" << std::endl;
        }
        
        // 遍历pair
        std::cout << "所有pair: ";
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            std::cout << "(" << it->first << ", " << it->second << ") ";
        }
        std::cout << std::endl;
    } catch(...) {
        std::cerr << "test_pair_operations 发生异常" << std::endl;
    }
    // 确保在函数结束时清理资源
    tree.clear();
}

void test_erase_operations() {
    std::cout << "\n测试删除操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> tree;
    try {
        // 插入元素
        for (int i = 1; i <= 5; ++i) {
            tree.insert_unique(i);
        }
        
        std::cout << "删除前: ";
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        
        // 删除元素
        tree.erase(3);
        
        std::cout << "删除后: ";
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    } catch(...) {
        std::cerr << "test_erase_operations 发生异常" << std::endl;
    }
    // 确保在函数结束时清理资源
    tree.clear();
}

void test_bound_operations() {
    std::cout << "\n测试边界操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> tree;
    try {
        // 插入元素
        for (int i = 1; i <= 10; i += 2) {
            tree.insert_unique(i);
        }
        
        // 测试lower_bound和upper_bound
        auto lb = tree.lower_bound(4);
        auto ub = tree.upper_bound(7);
        
        std::cout << "lower_bound(4): " << *lb << std::endl;
        std::cout << "upper_bound(7): " << *ub << std::endl;
        
        // 测试equal_range
        auto range = tree.equal_range(5);
        std::cout << "equal_range(5): ";
        for (auto it = range.first; it != range.second; ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    } catch(...) {
        std::cerr << "test_bound_operations 发生异常" << std::endl;
    }
    // 确保在函数结束时清理资源
    tree.clear();
}

void test_copy_operations() {
    std::cout << "\n测试复制操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> tree1;
    mstl::RbTree<int, int, KeyOfValue, Compare> tree2;
    try {
        // 插入元素
        for (int i = 1; i <= 5; ++i) {
            tree1.insert_unique(i);
        }
        
        // 复制树
        tree2 = tree1;
        
        std::cout << "原始树: ";
        for (auto it = tree1.begin(); it != tree1.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        
        std::cout << "复制树: ";
        for (auto it = tree2.begin(); it != tree2.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    } catch(...) {
        std::cerr << "test_copy_operations 发生异常" << std::endl;
    }
    // 确保在函数结束时清理所有资源
    tree1.clear();
    tree2.clear();
}

int main() {
    std::cout << "开始测试红黑树..." << std::endl;
    
    try {
        test_basic_operations();
        // test_pair_operations();
        // test_erase_operations();
        // test_bound_operations();
        // test_copy_operations();
    } catch(...) {
        std::cerr << "测试过程中发生异常" << std::endl;
        return 1;
    }
    
    std::cout << "\n测试完成!" << std::endl;
    return 0;
} 