#ifndef __MSGI_STL_INTERNAL_STACK_H
#define __MSGI_STL_INTERNAL_STACK_H

#include "mstl_deque.h"

namespace mstl {

template <class T, class Sequence = Deque<T>>
class Stack {
public:
    using ValueType = typename Sequence::ValueType;
    using SizeType = typename Sequence::SizeType;
    using Reference = typename Sequence::Reference;
    using ConstReference = typename Sequence::ConstReference;

    friend bool operator==(const Stack& x, const Stack& y) {
        return x.c == y.c;
    }

    friend bool operator<(const Stack& x, const Stack& y) {
        return std::lexicographical_compare(
            x.c.begin(), x.c.end(),
            y.c.begin(), y.c.end(),
            [](const ValueType& a, const ValueType& b) {
                return a < b;
            }
        );
    }

protected:
    Sequence c;  // 底层容器
public:
    bool empty() const {
        return c.empty();
    }
    SizeType size() const {
        return c.size();
    }
    Reference top() {
        return c.back();
    }
    ConstReference top() const {
        return c.back();
    }
    void push(const ValueType& x) {
        c.push_back(x);
    }
    void pop() {
        c.pop_back();
    }
};

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_STACK_H