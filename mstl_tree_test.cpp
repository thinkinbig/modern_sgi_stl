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

void test_basic_operations() {
    std::cout << "测试基本操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> tree;
    try {
        std::cout << "开始插入元素..." << std::endl;
        // 测试插入
        tree.insert_unique(1);
        std::cout << "插入 1 后树大小: " << tree.size() << std::endl;
        
        tree.insert_unique(2);
        std::cout << "插入 2 后树大小: " << tree.size() << std::endl;
        
        tree.insert_unique(3);
        std::cout << "插入 3 后树大小: " << tree.size() << std::endl;
        
        tree.insert_unique(4);
        std::cout << "插入 4 后树大小: " << tree.size() << std::endl;
        
        tree.insert_unique(5);
        std::cout << "插入 5 后树大小: " << tree.size() << std::endl;
        
        // 测试查找
        std::cout << "\n测试查找..." << std::endl;
        auto it = tree.find(4);
        if (it != tree.end()) {
            std::cout << "找到元素: " << *it << std::endl;
        } else {
            std::cout << "未找到元素 4" << std::endl;
        }
        
        // 测试大小
        std::cout << "\n树的大小: " << tree.size() << std::endl;
        
        // 测试遍历
        std::cout << "树的内容: ";
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 测试结构化输出
        std::cout << "\n树的结构化输出:" << std::endl;
        std::cout << tree;

    } catch(...) {
        std::cerr << "test_basic_operations 发生异常" << std::endl;
    }
    std::cout << "清理树..." << std::endl;
    tree.clear();
    std::cout << "清理后树大小: " << tree.size() << std::endl;
}

void test_copy_operations() {
    std::cout << "\n测试复制操作..." << std::endl;
    
    mstl::RbTree<int, int, KeyOfValue, Compare> original_tree;
    try {
        std::cout << "向原始树中插入元素..." << std::endl;
        // 向原始树中插入元素
        original_tree.insert_unique(10);
        std::cout << "插入 10 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(5);
        std::cout << "插入 5 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(15);
        std::cout << "插入 15 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(3);
        std::cout << "插入 3 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(7);
        std::cout << "插入 7 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(12);
        std::cout << "插入 12 后树大小: " << original_tree.size() << std::endl;
        
        original_tree.insert_unique(17);
        std::cout << "插入 17 后树大小: " << original_tree.size() << std::endl;

        std::cout << "\n原始树的内容: ";
        for (auto it = original_tree.begin(); it != original_tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        std::cout << "\n测试复制构造函数..." << std::endl;
        // 测试复制构造函数
        mstl::RbTree<int, int, KeyOfValue, Compare> copied_tree(original_tree);
        
        std::cout << "复制树的内容: ";
        for (auto it = copied_tree.begin(); it != copied_tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 验证两个树的大小是否相同
        std::cout << "原始树大小: " << original_tree.size() << std::endl;
        std::cout << "复制树大小: " << copied_tree.size() << std::endl;

        std::cout << "\n测试赋值运算符..." << std::endl;
        // 测试赋值运算符
        mstl::RbTree<int, int, KeyOfValue, Compare> assigned_tree;
        assigned_tree = original_tree;
        
        std::cout << "赋值树的内容: ";
        for (auto it = assigned_tree.begin(); it != assigned_tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 验证赋值后的大小
        std::cout << "赋值树大小: " << assigned_tree.size() << std::endl;

        std::cout << "\n测试修改复制树..." << std::endl;
        // 测试修改复制树是否影响原始树
        copied_tree.insert_unique(20);
        std::cout << "修改后复制树的内容: ";
        for (auto it = copied_tree.begin(); it != copied_tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        std::cout << "修改后原始树的内容: ";
        for (auto it = original_tree.begin(); it != original_tree.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

    } catch(...) {
        std::cerr << "test_copy_operations 发生异常" << std::endl;
    }
    std::cout << "清理原始树..." << std::endl;
    original_tree.clear();
    std::cout << "清理后原始树大小: " << original_tree.size() << std::endl;
}

int main() {
    std::cout << "开始测试红黑树..." << std::endl;
    
    try {
        test_basic_operations();
        test_copy_operations();
    } catch(...) {
        std::cerr << "测试过程中发生异常" << std::endl;
        return 1;
    }
    
    std::cout << "\n测试完成!" << std::endl;
    return 0;
} 