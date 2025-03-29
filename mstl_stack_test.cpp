#include "mstl_stack.h"
#include <cassert>
#include <iostream>

using namespace mstl;

void stack_test() {
    std::cout << "开始测试 Stack..." << std::endl;

    // 测试构造函数和基本操作
    Stack<int> s1;
    assert(s1.empty());
    assert(s1.size() == 0);

    // 测试push和top
    s1.push(1);
    assert(!s1.empty());
    assert(s1.size() == 1);
    assert(s1.top() == 1);

    // 测试pop
    s1.pop();
    assert(s1.empty());
    assert(s1.size() == 0);

    // 测试多个元素
    s1.push(1);
    s1.push(2);
    s1.push(3);
    assert(s1.size() == 3);
    assert(s1.top() == 3);

    // 测试比较运算符
    Stack<int> s2;
    s2.push(1);
    s2.push(2);
    s2.push(3);
    assert(s1 == s2);

    // 测试小于运算符
    Stack<int> s3;
    s3.push(1);
    s3.push(2);
    s3.push(4);
    assert(s1 < s3);

    // 测试不同序列类型
    Stack<int, Deque<int>> s4;
    s4.push(1);
    assert(s4.top() == 1);

    std::cout << "Stack 测试通过！" << std::endl;
}


int main() {
    stack_test();
    return 0;
}
