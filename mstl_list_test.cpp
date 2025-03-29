#include "mstl_list.h"
#include <cassert>
#include <iostream>
#include <string>

// 用于测试的辅助函数
template <typename T>
void printList(const mstl::List<T>& lst, const std::string& msg) {
    std::cout << msg << ": ";
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void testListConstructor() {
    std::cout << "\n=== 测试构造函数 ===" << std::endl;

    // 测试默认构造函数
    mstl::List<int> lst1;
    assert(lst1.empty());
    assert(lst1.size() == 0);

    // 测试带初始大小的构造函数
    mstl::List<int> lst2(5);
    assert(lst2.size() == 5);

    // 测试带初始大小和值的构造函数
    mstl::List<int> lst3(3, 100);
    assert(lst3.size() == 3);
    for (auto it = lst3.begin(); it != lst3.end(); ++it) {
        assert(*it == 100);
    }

    // 测试拷贝构造函数
    mstl::List<int> lst4(lst3);
    assert(lst4.size() == lst3.size());

    std::cout << "Constructor tests passed!" << std::endl;
}

void testListPushBack() {
    std::cout << "\n=== 测试push_back ===" << std::endl;

    mstl::List<int> lst;

    // 测试空列表的push_back
    lst.push_back(1);
    assert(lst.size() == 1);
    assert(lst.back() == 1);

    // 测试多次push_back
    lst.push_back(2);
    lst.push_back(3);
    assert(lst.size() == 3);
    assert(lst.back() == 3);

    // 测试迭代器是否正确
    auto it = lst.begin();
    assert(*it == 1);
    ++it;
    assert(*it == 2);
    ++it;
    assert(*it == 3);

    std::cout << "Push back tests passed!" << std::endl;
}

void testListPushFront() {
    std::cout << "\n=== 测试push_front ===" << std::endl;

    mstl::List<int> lst;

    // 测试空列表的push_front
    lst.push_front(1);
    assert(lst.size() == 1);
    assert(lst.front() == 1);

    // 测试多次push_front
    lst.push_front(2);
    lst.push_front(3);
    assert(lst.size() == 3);
    assert(lst.front() == 3);

    // 验证顺序是否正确
    auto it = lst.begin();
    assert(*it == 3);
    ++it;
    assert(*it == 2);
    ++it;
    assert(*it == 1);

    std::cout << "Push front tests passed!" << std::endl;
}

void testListIterator() {
    std::cout << "\n=== 测试迭代器 ===" << std::endl;

    mstl::List<int> lst;
    for (int i = 0; i < 5; ++i) {
        lst.push_back(i);
    }

    // 测试正向遍历
    int expected = 0;
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        assert(*it == expected++);
    }

    // 测试反向遍历
    expected = 4;
    auto it = lst.end();
    while (it != lst.begin()) {
        --it;
        assert(*it == expected--);
    }

    // 测试迭代器比较操作
    auto it1 = lst.begin();
    auto it2 = lst.begin();
    assert(it1 == it2);
    ++it2;
    assert(it1 != it2);

    // 测试迭代器的自增自减
    it1 = lst.begin();
    auto it3 = it1++;
    assert(*it3 == 0);
    assert(*it1 == 1);

    it1 = lst.begin();
    auto it4 = ++it1;
    assert(*it4 == 1);
    assert(*it1 == 1);

    std::cout << "Iterator tests passed!" << std::endl;
}

void testListOperations() {
    std::cout << "\n=== 测试其他操作 ===" << std::endl;

    mstl::List<int> lst;

    // 测试insert
    auto it = lst.begin();
    it = lst.insert(it, 1);
    assert(*it == 1);

    it = lst.insert(it, 2);
    assert(*it == 2);

    // 测试erase
    it = lst.begin();
    ++it;
    it = lst.erase(it);
    assert(lst.size() == 1);

    // 测试clear
    lst.clear();
    assert(lst.empty());
    assert(lst.size() == 0);

    // 测试赋值操作
    mstl::List<int> lst2;
    lst2.push_back(10);
    lst2.push_back(20);

    lst = lst2;
    assert(lst.size() == lst2.size());

    // 测试splice
    mstl::List<int> lst3;
    lst3.push_back(30);
    lst3.push_back(40);

    lst.splice(lst.end(), lst3);
    printList(lst, "lst");
    assert(lst.size() == 4);
    assert(lst3.empty());

    std::cout << "Operations tests passed!" << std::endl;
}

int main() {
    std::cout << "Starting mstl::list tests..." << std::endl;

    try {
        testListConstructor();
        testListPushBack();
        testListPushFront();
        testListIterator();
        testListOperations();

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