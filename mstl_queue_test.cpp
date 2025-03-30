#include "mstl_queue.h"
#include "mstl_list.h"
#include <cassert>
#include <iostream>

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

int main() {
    queue_deque_test();
    queue_list_test();
    return 0;
} 