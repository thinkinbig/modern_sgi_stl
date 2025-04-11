#include "mstl_queue.h"
#include <cassert>
#include <iostream>
#include "mstl_functional.h"
#include "mstl_list.h"
#include "mstl_vector.h"

using namespace mstl;

void queue_deque_test() {
    std::cout << "开始测试 Queue (使用 Deque 作为底层容器)..." << std::endl;

    // 测试构造函数和基本操作
    Queue<int> q1;
    assert(q1.empty());
    assert(q1.size() == 0);

    // 测试push和front/back
    q1.push(1);
    assert(!q1.empty());
    assert(q1.size() == 1);
    assert(q1.front() == 1);
    assert(q1.back() == 1);

    // 测试pop
    q1.pop();
    assert(q1.empty());
    assert(q1.size() == 0);

    // 测试多个元素
    q1.push(1);
    q1.push(2);
    q1.push(3);
    assert(q1.size() == 3);
    assert(q1.front() == 1);
    assert(q1.back() == 3);

    // 测试比较运算符
    Queue<int> q2;
    q2.push(1);
    q2.push(2);
    q2.push(3);
    assert(q1 == q2);

    // 测试小于运算符
    Queue<int> q3;
    q3.push(1);
    q3.push(2);
    q3.push(4);
    assert(q1 < q3);

    std::cout << "Queue (Deque) 测试通过！" << std::endl;
}

void queue_list_test() {
    std::cout << "开始测试 Queue (使用 List 作为底层容器)..." << std::endl;

    // 测试构造函数和基本操作
    Queue<int, List<int>> q1;
    assert(q1.empty());
    assert(q1.size() == 0);

    // 测试push和front/back
    q1.push(1);
    assert(!q1.empty());
    assert(q1.size() == 1);
    assert(q1.front() == 1);
    assert(q1.back() == 1);

    // 测试pop
    q1.pop();
    assert(q1.empty());
    assert(q1.size() == 0);

    // 测试多个元素
    q1.push(1);
    q1.push(2);
    q1.push(3);
    assert(q1.size() == 3);
    assert(q1.front() == 1);
    assert(q1.back() == 3);

    // 测试比较运算符
    Queue<int, List<int>> q2;
    q2.push(1);
    q2.push(2);
    q2.push(3);
    assert(q1 == q2);

    // 测试小于运算符
    Queue<int, List<int>> q3;
    q3.push(1);
    q3.push(2);
    q3.push(4);
    assert(q1 < q3);

    std::cout << "Queue (List) 测试通过！" << std::endl;
}

void priority_queue_test() {
    std::cout << "开始测试 PriorityQueue..." << std::endl;

    // 测试默认构造函数和基本操作
    PriorityQueue<int> pq1;
    assert(pq1.empty());
    assert(pq1.size() == 0);

    // 测试push和top
    pq1.push(3);
    pq1.push(1);
    pq1.push(4);
    assert(!pq1.empty());
    assert(pq1.size() == 3);
    assert(pq1.top() == 4);  // 默认是最大堆

    // 测试pop
    pq1.pop();
    assert(pq1.size() == 2);
    assert(pq1.top() == 3);

    // 测试自定义比较函数（最小堆）
    PriorityQueue<int, Vector<int>, mstl::greater<int>> pq2;
    pq2.push(3);
    pq2.push(1);
    pq2.push(4);
    assert(pq2.top() == 1);  // 最小堆，顶部是最小值

    // 测试使用迭代器范围构造
    int arr[] = {5, 2, 8, 1, 9};
    PriorityQueue<int> pq3(arr, arr + 5);
    assert(pq3.size() == 5);
    assert(pq3.top() == 9);

    // 测试使用迭代器范围和比较函数构造
    PriorityQueue<int, Vector<int>, mstl::greater<int>> pq4(arr, arr + 5);
    assert(pq4.size() == 5);
    assert(pq4.top() == 1);

    // 测试异常安全性
    try {
        PriorityQueue<int> pq5;
        pq5.push(1);
        throw std::runtime_error("test exception");
    } catch (...) {
        // 确保异常时容器被清空
    }

    std::cout << "PriorityQueue 测试通过！" << std::endl;
}

int main() {
    queue_deque_test();
    queue_list_test();
    priority_queue_test();
    return 0;
}