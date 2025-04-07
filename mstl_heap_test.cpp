#include "mstl_heap.h"
#include <iostream>
#include <vector>

using namespace mstl;

void test_push_heap() {
    std::cout << "Testing push_heap..." << std::endl;

    // 测试用例1：基本场景
    {
        std::cout << "\nTest case 1: Basic case" << std::endl;
        std::vector<int> vec = {3, 1, 4, 1, 5};
        mstl::make_heap(vec.begin(), vec.end());

        std::cout << "Initial heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;

        vec.push_back(6);
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // 测试用例2：空堆
    {
        std::cout << "\nTest case 2: Empty heap" << std::endl;
        std::vector<int> vec;
        vec.push_back(1);
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // 测试用例3：单元素堆
    {
        std::cout << "\nTest case 3: Single element heap" << std::endl;
        std::vector<int> vec = {1};
        vec.push_back(2);
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // 测试用例4：大数
    {
        std::cout << "\nTest case 4: Large number" << std::endl;
        std::vector<int> vec = {5, 4, 3, 2, 1};
        mstl::make_heap(vec.begin(), vec.end());

        std::cout << "Initial heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;

        vec.push_back(10);
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // 测试用例5：小数
    {
        std::cout << "\nTest case 5: Small number" << std::endl;
        std::vector<int> vec = {5, 4, 3, 2, 1};
        mstl::make_heap(vec.begin(), vec.end());

        std::cout << "Initial heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;

        vec.push_back(0);
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // 测试用例6：重复元素
    {
        std::cout << "\nTest case 6: Duplicate elements" << std::endl;
        std::vector<int> vec = {5, 4, 3, 2, 1};
        mstl::make_heap(vec.begin(), vec.end());

        std::cout << "Initial heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;

        vec.push_back(3);  // 添加一个已存在的值
        mstl::push_heap(vec.begin(), vec.end());

        std::cout << "After push_heap: ";
        for (int x : vec)
            std::cout << x << " ";
        std::cout << std::endl;
    }
}

void test_pop_heap() {
    std::cout << "\nTesting pop_heap..." << std::endl;
    std::vector<int> vec = {5, 4, 3, 2, 1};
    mstl::make_heap<std::vector<int>::iterator>(vec.begin(), vec.end());

    std::cout << "Initial heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;

    mstl::pop_heap<std::vector<int>::iterator>(vec.begin(), vec.end());
    vec.pop_back();

    std::cout << "After pop_heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;
}

void test_make_heap() {
    std::cout << "\nTesting make_heap..." << std::endl;
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};

    std::cout << "Before make_heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;

    mstl::make_heap<std::vector<int>::iterator>(vec.begin(), vec.end());

    std::cout << "After make_heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;
}

void test_sort_heap() {
    std::cout << "\nTesting sort_heap..." << std::endl;
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    mstl::make_heap<std::vector<int>::iterator>(vec.begin(), vec.end());

    std::cout << "Before sort_heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;

    mstl::sort_heap<std::vector<int>::iterator>(vec.begin(), vec.end());

    std::cout << "After sort_heap: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << std::endl;
}

int main() {
    test_push_heap();
    test_pop_heap();
    test_make_heap();
    test_sort_heap();
    return 0;
}