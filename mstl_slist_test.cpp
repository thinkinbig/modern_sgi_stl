#include "mstl_slist.h"
#include <iostream>

int main() {
    mstl::Slist<int> slist;

    // 测试 push_front 和 size
    slist.push_front(10);
    slist.push_front(20);
    slist.push_front(30);

    std::cout << "Size after 3 insertions: " << slist.size() << std::endl;

    // 测试 front
    std::cout << "Front element: " << slist.front() << std::endl;

    // 测试 pop_front
    slist.pop_front();
    std::cout << "Size after 1 pop: " << slist.size() << std::endl;
    std::cout << "Front element after 1 pop: " << slist.front() << std::endl;

    slist.pop_front();
    slist.pop_front();

    std::cout << "Size after popping all elements: " << slist.size() << std::endl;
    std::cout << "Is empty: " << std::boolalpha << slist.empty() << std::endl;

    return 0;
}