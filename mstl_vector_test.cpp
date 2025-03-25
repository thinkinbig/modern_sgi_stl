#include <iostream>
#include "mstl_vector.h"

// 测试vector的基本功能
int main() {
    // 测试构造函数和基本操作
    mstl::Vector<int> vec;
    std::cout << "初始vector是否为空: " << (vec.empty() ? "是" : "否") << std::endl;
    std::cout << "初始vector大小: " << vec.size() << std::endl;
    std::cout << "初始vector容量: " << vec.capacity() << std::endl;
    
    // 测试push_back
    std::cout << "\n测试push_back:" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        vec.push_back(i); // 使用常规push_back
        std::cout << "添加元素 " << i << " 后，vector大小: " << vec.size() 
                  << "，容量: " << vec.capacity() << std::endl;
    }
    
    // 测试访问元素
    std::cout << "\n测试元素访问:" << std::endl;
    std::cout << "首元素: " << vec.front() << std::endl;
    std::cout << "尾元素: " << vec.back() << std::endl;
    std::cout << "通过下标访问[2]: " << vec[2] << std::endl;
    
    // 测试迭代器
    std::cout << "\n测试迭代器遍历:" << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试pop_back
    std::cout << "\n测试pop_back:" << std::endl;
    vec.pop_back();
    std::cout << "调用pop_back后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试单个元素erase
    std::cout << "\n测试单个元素erase:" << std::endl;
    vec.erase(vec.begin() + 1);  // 删除第二个元素
    std::cout << "删除第二个元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试区间erase
    std::cout << "\n测试区间erase:" << std::endl;
    // 先插入更多元素用于测试
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    std::cout << "插入元素后，Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 删除区间 [begin+1, begin+3)
    vec.erase(vec.begin() + 1, vec.begin() + 3);
    std::cout << "删除区间[begin+1, begin+3)后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试区间insert
    std::cout << "\n测试区间insert:" << std::endl;
    
    // 场景1: 在中间位置插入多个相同的元素
    vec.insert(vec.begin() + 1, 3, 42);  // 在第二个位置插入3个值为42的元素
    std::cout << "在位置1插入3个值为42的元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 场景2: 在开始处插入
    vec.insert(vec.begin(), 2, 100);  // 在开头插入2个值为100的元素
    std::cout << "在开头插入2个值为100的元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 场景3: 在末尾插入
    vec.insert(vec.end(), 1, 999);  // 在末尾插入1个值为999的元素
    std::cout << "在末尾插入1个值为999的元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试resize
    std::cout << "\n测试resize:" << std::endl;
    vec.resize(6, 10);  // 扩大到6个元素，新元素初始化为10
    std::cout << "扩大到6个元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    vec.resize(2);  // 缩小到2个元素
    std::cout << "缩小到2个元素后，vector大小: " << vec.size() << std::endl;
    std::cout << "Vector内容: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // 测试clear
    std::cout << "\n测试clear:" << std::endl;
    vec.clear();
    std::cout << "调用clear后，vector是否为空: " << (vec.empty() ? "是" : "否") << std::endl;
    std::cout << "vector大小: " << vec.size() << std::endl;
    
    return 0;
} 