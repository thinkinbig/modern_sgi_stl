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

void test_deque_constructors() {
    // 测试默认构造函数
    mstl::Deque<int> dq1;
    assert(dq1.empty());
    assert(dq1.size() == 0);

    // 测试带初始大小的构造函数
    mstl::Deque<int> dq2(5);
    assert(dq2.size() == 5);
    for (int i = 0; i < 5; ++i) {
        assert(dq2[i] == 0);
    }

    // 测试带初始值和大小的构造函数
    mstl::Deque<int> dq3(5, 42);
    assert(dq3.size() == 5);
    for (int i = 0; i < 5; ++i) {
        assert(dq3[i] == 42);
    }

    // 测试拷贝构造函数
    mstl::Deque<int> dq4(dq3);
    assert(dq4.size() == 5);
    for (int i = 0; i < 5; ++i) {
        assert(dq4[i] == 42);
    }

    // 测试移动构造函数
    mstl::Deque<int> dq5(std::move(dq4));
    assert(dq5.size() == 5);
    assert(dq4.empty());
    for (int i = 0; i < 5; ++i) {
        assert(dq5[i] == 42);
    }
}

void test_deque_iterators() {
    mstl::Deque<int> dq;
    for (int i = 0; i < 10; ++i) {
        dq.push_back(i);
    }

    // 测试正向迭代器
    int i = 0;
    for (auto it = dq.begin(); it != dq.end(); ++it) {
        assert(*it == i++);
    }

    // 测试反向迭代器
    i = 9;
    for (auto it = dq.rbegin(); it != dq.rend(); ++it) {
        assert(*it == i--);
    }
}

void test_deque_operations() {
    mstl::Deque<int> dq;

    // 测试 push_back 和 pop_back
    for (int i = 0; i < 5; ++i) {
        dq.push_back(i);
    }
    assert(dq.size() == 5);
    for (int i = 0; i < 5; ++i) {
        assert(dq[i] == i);
    }
    dq.pop_back();
    assert(dq.size() == 4);
    assert(dq.back() == 3);

    // 测试 push_front 和 pop_front
    dq.push_front(42);
    assert(dq.size() == 5);
    assert(dq.front() == 42);
    dq.pop_front();
    assert(dq.size() == 4);
    assert(dq.front() == 0);

    // 测试 insert 和 erase
    dq.insert(dq.begin() + 2, 99);
    assert(dq.size() == 5);
    assert(dq[2] == 99);
    dq.erase(dq.begin() + 2);
    assert(dq.size() == 4);
    assert(dq[2] == 2);
}

void test_deque_clear() {
    mstl::Deque<int> dq;
    for (int i = 0; i < 10; ++i) {
        dq.push_back(i);
    }
    assert(dq.size() == 10);
    dq.clear();
    assert(dq.empty());
    assert(dq.size() == 0);
}

int main() {
    std::cout << "Starting mstl::deque tests..." << std::endl;

    try {
        test_deque_constructors();
        test_deque_iterators();
        test_deque_operations();
        test_deque_clear();

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
