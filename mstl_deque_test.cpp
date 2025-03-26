#include <iostream>
#include <cassert>
#include <string>
#include "mstl_deque.h"

// 用于测试的辅助函数
template<typename T>
void printDeque(const mstl::Deque<T>& dq, const std::string& msg) {
    std::cout << msg << ": ";
    for (auto it = dq.begin(); it != dq.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void testDequeConstructor() {
    std::cout << "\n=== 测试构造函数 ===" << std::endl;
    
    // 测试默认构造函数
    mstl::Deque<int> dq1(10, 1);
    assert(dq1.empty() == false);
    assert(dq1.size() == 10);
    for (auto it = dq1.begin(); it != dq1.end(); ++it) {
        assert(*it == 1);
    }
    int back = dq1.back();
    std::cout << back <<std::endl;

    std::cout << "Constructor tests passed!" << std::endl;
}

void testDequeIterator() {
    std::cout << "\n=== 测试迭代器 ===" << std::endl;
    
    mstl::Deque<int> dq(10, 1);
    
    // 测试空deque的迭代器
    auto begin = dq.begin();
    auto end = dq.end();
    assert(dq.size() == 10);
    assert(begin + dq.size() == end);
    
    // 测试迭代器的比较操作
    auto it1 = dq.begin();
    auto it2 = dq.begin();
    assert(it1 == it2);
    
    std::cout << "Iterator tests passed!" << std::endl;
}

void testDequeOperations() {
    std::cout << "\n=== 测试基本操作 ===" << std::endl;
    
    mstl::Deque<int> dq(10, 1);
    
    // 测试empty()和size()
    assert(dq.empty() == false);
    assert(dq.size() == 10);
    
    std::cout << "Basic operations tests passed!" << std::endl;
}

void testDequePushPop() {
    std::cout << "\n=== 测试push和pop操作 ===" << std::endl;
    
    mstl::Deque<int> dq;
    
    // 测试push_back
    for (int i = 0; i < 5; ++i) {
        dq.push_back(i);
        std::cout <<dq.size() <<std::endl;
        assert(dq.size() == static_cast<mstl::Deque<int>::size_type>(i + 1));
        assert(dq.back() == i);
    }
    printDeque(dq, "After push_back");
    
    // 测试push_front
    for (int i = 5; i < 10; ++i) {
        dq.push_front(i);
        assert(dq.size() == static_cast<mstl::Deque<int>::size_type>(i + 1));
        assert(dq.front() == i);
    }
    printDeque(dq, "After push_front");
    
    // 测试pop_back
    for (int i = 0; i < 3; ++i) {
        int last = dq.back();
        dq.pop_back();
        assert(dq.size() == static_cast<mstl::Deque<int>::size_type>(10 - i - 1));
        assert(dq.back() != last);
    }
    printDeque(dq, "After pop_back");
    
    // 测试pop_front
    for (int i = 0; i < 3; ++i) {
        int first = dq.front();
        dq.pop_front();
        assert(dq.size() == static_cast<mstl::Deque<int>::size_type>(7 - i - 1));
        assert(dq.front() != first);
    }
    printDeque(dq, "After pop_front");
    
    // 测试clear
    dq.clear();
    assert(dq.empty());
    assert(dq.size() == 0);
    printDeque(dq, "After clear");
    
    std::cout << "Push and pop operations tests passed!" << std::endl;
}

int main() {
    std::cout << "Starting mstl::deque tests..." << std::endl;
    
    try {
        testDequeConstructor();
        testDequeIterator();
        testDequeOperations();
        testDequePushPop();
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Test failed with unknown exception!" << std::endl;
        return 1;
    }
    
    return 0;
}
