#include "mstl_deque.h"
#include <cassert>
#include <iostream>
#include <string>

// 用于测试的辅助函数
template <typename T>
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
    mstl::Deque<int> dq1;
    assert(dq1.empty());
    assert(dq1.size() == 0);

    // 测试带初始值的构造函数
    mstl::Deque<int> dq2(10, 1);
    assert(dq2.empty() == false);
    assert(dq2.size() == 10);
    for (auto it = dq2.begin(); it != dq2.end(); ++it) {
        assert(*it == 1);
    }

    std::cout << "Constructor tests passed!" << std::endl;
}

void testDequeIterator() {
    std::cout << "\n=== 测试迭代器 ===" << std::endl;

    mstl::Deque<int> dq(10, 1);

    // 测试迭代器的基本操作
    auto begin = dq.begin();
    auto end = dq.end();
    assert(dq.size() == 10);
    assert(begin + dq.size() == end);

    // 测试迭代器的比较操作
    auto it1 = dq.begin();
    auto it2 = dq.begin();
    assert(it1 == it2);

    // 测试迭代器的算术操作
    auto it3 = dq.begin() + 5;
    assert(*it3 == 1);
    assert(it3 - dq.begin() == 5);

    // 测试迭代器的自增自减
    auto it4 = dq.begin();
    assert(*(++it4) == 1);
    assert(*(--it4) == 1);

    std::cout << "Iterator tests passed!" << std::endl;
}

void testDequeOperations() {
    std::cout << "\n=== 测试基本操作 ===" << std::endl;

    mstl::Deque<int> dq(10, 1);

    // 测试empty()和size()
    assert(dq.empty() == false);
    assert(dq.size() == 10);

    // 测试front()和back()
    assert(dq.front() == 1);
    assert(dq.back() == 1);

    // 测试operator[]
    assert(dq[0] == 1);
    assert(dq[5] == 1);
    assert(dq[9] == 1);

    std::cout << "Basic operations tests passed!" << std::endl;
}

void testDequePushPop() {
    std::cout << "\n=== 测试push和pop操作 ===" << std::endl;

    mstl::Deque<int> dq;

    // 测试push_back
    for (int i = 0; i < 5; ++i) {
        dq.push_back(i);
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

void testDequeErase() {
    std::cout << "\n=== 测试erase操作 ===" << std::endl;

    mstl::Deque<int> dq;
    for (int i = 0; i < 10; ++i) {
        dq.push_back(i);
    }
    printDeque(dq, "Initial deque");

    // 测试单个元素erase
    auto it = dq.begin() + 5;
    it = dq.erase(it);
    assert(dq.size() == 9);
    assert(*it == 6);
    printDeque(dq, "After single erase");

    // 测试范围erase
    auto first = dq.begin() + 2;
    auto last = dq.begin() + 5;
    it = dq.erase(first, last);
    assert(dq.size() == 6);
    assert(*it == 6);
    printDeque(dq, "After range erase");

    std::cout << "Erase operations tests passed!" << std::endl;
}

void testDequeInsert() {
    std::cout << "\n=== 测试insert操作 ===" << std::endl;

    mstl::Deque<int> dq;
    for (int i = 0; i < 5; ++i) {
        dq.push_back(i);
    }
    printDeque(dq, "Initial deque");

    // 测试在中间插入
    auto it = dq.begin() + 2;
    it = dq.insert(it, 10);
    assert(dq.size() == 6);
    assert(*it == 10);
    printDeque(dq, "After insert in middle");

    // 测试在开头插入
    it = dq.insert(dq.begin(), 20);
    assert(dq.size() == 7);
    assert(*it == 20);
    printDeque(dq, "After insert at front");

    // 测试在结尾插入
    it = dq.insert(dq.end(), 30);
    assert(dq.size() == 8);
    assert(*it == 30);
    printDeque(dq, "After insert at back");

    std::cout << "Insert operations tests passed!" << std::endl;
}

void testDequeEmplace() {
    std::cout << "\n=== 测试emplace操作 ===" << std::endl;

    mstl::Deque<int> dq;

    // 测试emplace_back
    dq.emplace_back(1);
    assert(dq.size() == 1);
    assert(dq.back() == 1);

    // 测试emplace_front
    dq.emplace_front(2);
    assert(dq.size() == 2);
    assert(dq.front() == 2);

    // 测试emplace
    auto it = dq.begin() + 1;
    it = dq.emplace(it, 3);
    assert(dq.size() == 3);
    assert(*it == 3);

    printDeque(dq, "After emplace operations");

    std::cout << "Emplace operations tests passed!" << std::endl;
}

int main() {
    std::cout << "Starting mstl::deque tests..." << std::endl;

    try {
        testDequeConstructor();
        testDequeIterator();
        testDequeOperations();
        testDequePushPop();
        testDequeErase();
        testDequeInsert();
        testDequeEmplace();

        std::cout << "\nAll tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception!" << std::endl;
        return 1;
    }

    return 0;
}
