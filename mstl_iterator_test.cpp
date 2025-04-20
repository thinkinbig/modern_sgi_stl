#include "mstl_iterator.h"
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "mstl_iterator_traits.h"

int main() {
    std::cout << "===== 迭代器工具函数测试 =====" << std::endl;

    // 测试 distance
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {10, 20, 30, 40};

    std::cout << "1. 向量distance测试:" << std::endl;
    auto vec_dist = mstl::distance(vec.begin(), vec.end());
    std::cout << "  向量大小: " << vec.size() << std::endl;
    std::cout << "  mstl::distance结果: " << vec_dist << std::endl;
    std::cout << "  结果正确: "
              << (vec_dist == static_cast<decltype(vec_dist)>(vec.size()) ? "是" : "否")
              << std::endl;
    std::cout << std::endl;

    std::cout << "2. 链表distance测试:" << std::endl;
    auto lst_dist = mstl::distance(lst.begin(), lst.end());
    std::cout << "  链表大小: " << lst.size() << std::endl;
    std::cout << "  mstl::distance结果: " << lst_dist << std::endl;
    std::cout << "  结果正确: "
              << (lst_dist == static_cast<decltype(lst_dist)>(lst.size()) ? "是" : "否")
              << std::endl;
    std::cout << std::endl;

    // 测试 advance
    std::cout << "3. 向量advance测试:" << std::endl;
    auto vec_it = vec.begin();
    std::cout << "  初始值: " << *vec_it << std::endl;

    mstl::advance(vec_it, 2);
    std::cout << "  前进2步后: " << *vec_it << std::endl;
    std::cout << "  正确位置值: " << vec[2] << std::endl;
    std::cout << "  结果正确: " << (*vec_it == vec[2] ? "是" : "否") << std::endl;
    std::cout << std::endl;

    std::cout << "4. 链表advance测试:" << std::endl;
    auto lst_it = lst.begin();
    std::cout << "  初始值: " << *lst_it << std::endl;

    mstl::advance(lst_it, 2);
    auto expected_it = lst.begin();
    std::advance(expected_it, 2);
    std::cout << "  前进2步后: " << *lst_it << std::endl;
    std::cout << "  期望值: " << *expected_it << std::endl;
    std::cout << "  结果正确: " << (*lst_it == *expected_it ? "是" : "否") << std::endl;
    std::cout << std::endl;

    // 测试负数advance (双向迭代器)
    std::cout << "5. 双向迭代器负数advance测试:" << std::endl;
    auto lst_back_it = --lst.end();
    std::cout << "  初始值: " << *lst_back_it << std::endl;

    mstl::advance(lst_back_it, -2);
    auto expected_back_it = --lst.end();
    std::advance(expected_back_it, -2);
    std::cout << "  后退2步后: " << *lst_back_it << std::endl;
    std::cout << "  期望值: " << *expected_back_it << std::endl;
    std::cout << "  结果正确: " << (*lst_back_it == *expected_back_it ? "是" : "否") << std::endl;
    std::cout << std::endl;

    // 测试 iterator_category, distance_type, value_type
    std::cout << "6. 类型辅助函数测试:" << std::endl;

    std::cout << "  迭代器类别测试: ";
    if (std::is_same_v<decltype(mstl::detail::iterator_category(vec.begin())),
                       mstl::RandomAccessIteratorTag>) {
        std::cout << "正确" << std::endl;
    } else {
        std::cout << "错误" << std::endl;
    }

    std::cout << "  距离类型测试: ";
    if (std::is_same_v<std::remove_pointer_t<decltype(mstl::detail::distance_type(vec.begin()))>,
                       typename std::vector<int>::iterator::difference_type>) {
        std::cout << "正确" << std::endl;
    } else {
        std::cout << "错误" << std::endl;
    }

    std::cout << "  值类型测试: ";
    if (std::is_same_v<std::remove_pointer_t<decltype(mstl::detail::value_type(vec.begin()))>,
                       typename std::vector<int>::iterator::value_type>) {
        std::cout << "正确" << std::endl;
    } else {
        std::cout << "错误" << std::endl;
    }

    std::cout << "\n测试完成!" << std::endl;
    return 0;
}