#include "mstl_set.h"
#include <iostream>
#include <string>
#include <cassert>

using namespace mstl;

void test_basic_operations() {
    std::cout << "\n=== Set 基本操作测试 ===" << std::endl;
    Set<int> s;
    assert(s.empty());
    assert(s.size() == 0);

    // 插入元素
    auto res1 = s.insert(3);
    assert(res1.second);
    assert(*res1.first == 3);
    assert(s.size() == 1);

    auto res2 = s.insert(1);
    auto res3 = s.insert(4);
    auto res4 = s.insert(1); // 重复插入
    assert(res2.second);
    assert(res3.second);
    assert(!res4.second);
    assert(s.size() == 3);

    // 查找
    auto it = s.find(3);
    assert(it != s.end());
    assert(*it == 3);
    assert(s.count(3) == 1);
    assert(s.count(100) == 0);

    // lower_bound/upper_bound
    assert(*s.lower_bound(2) == 3);
    assert(*s.upper_bound(3) == 4);

    // 遍历
    std::cout << "Set内容: ";
    for (auto x : s) std::cout << x << " ";
    std::cout << std::endl;

    // 反向遍历
    std::cout << "Set反向: ";
    for (auto it = s.rbegin(); it != s.rend(); ++it) std::cout << *it << " ";
    std::cout << std::endl;

    // erase by value
    assert(s.erase(1) == 1);
    assert(s.size() == 2);
    assert(s.erase(100) == 0);

    // erase by iterator
    s.erase(s.begin());
    assert(s.size() == 1);

    // clear
    s.clear();
    assert(s.empty());
    std::cout << "基本操作测试通过!" << std::endl;
}

void test_string_set() {
    std::cout << "\n=== Set<string> 测试 ===" << std::endl;
    Set<std::string> s;
    s.insert("apple");
    s.insert("banana");
    s.insert("cherry");
    s.insert("banana");
    assert(s.size() == 3);
    assert(s.count("banana") == 1);
    assert(s.find("apple") != s.end());
    s.erase("banana");
    assert(s.count("banana") == 0);
    std::cout << "Set<string>内容: ";
    for (const auto& str : s) std::cout << str << " ";
    std::cout << std::endl;
    std::cout << "Set<string> 测试通过!" << std::endl;
}

void test_copy_and_move() {
    std::cout << "\n=== Set 拷贝和移动测试 ===" << std::endl;
    Set<int> s1;
    for (int i = 0; i < 5; ++i) s1.insert(i);
    Set<int> s2(s1); // 拷贝构造
    assert(s2.size() == s1.size());
    Set<int> s3;
    s3 = s1; // 拷贝赋值
    assert(s3.size() == s1.size());
    Set<int> s4(std::move(s1)); // 移动构造
    assert(s4.size() == 5);
    Set<int> s5;
    s5 = std::move(s2); // 移动赋值
    assert(s5.size() == 5);
    std::cout << "拷贝和移动测试通过!" << std::endl;
}

void test_comparisons() {
    std::cout << "\n=== Set 比较操作测试 ===" << std::endl;
    Set<int> s1, s2;
    for (int i = 0; i < 3; ++i) s1.insert(i);
    for (int i = 0; i < 3; ++i) s2.insert(i);
    assert(s1 == s2);
    s2.insert(100);
    assert(s1 != s2);
    assert(s1 < s2);
    assert(s2 > s1);
    assert(s1 <= s2);
    assert(s2 >= s1);
    std::cout << "比较操作测试通过!" << std::endl;
}

void test_iterator_validity() {
    std::cout << "\n=== Set 迭代器有效性测试 ===" << std::endl;
    Set<int> s;
    for (int i = 0; i < 10; ++i) s.insert(i);
    auto it = s.begin();
    int count = 0;
    while (it != s.end()) {
        ++count;
        ++it;
    }
    assert(count == 10);
    std::cout << "迭代器有效性测试通过!" << std::endl;
}

int main() {
    std::cout << "开始测试 mstl::Set..." << std::endl;
    try {
        test_basic_operations();
        test_string_set();
        test_copy_and_move();
        test_comparisons();
        test_iterator_validity();
        std::cout << "\n所有测试通过!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "测试发生未知异常!" << std::endl;
        return 1;
    }
} 