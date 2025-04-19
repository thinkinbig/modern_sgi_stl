#ifndef __MSGI_STL_INTERNAL_STACK_H
#define __MSGI_STL_INTERNAL_STACK_H

#include "mstl_deque.h"

namespace mstl {

template <class T, class Sequence = Deque<T>>
class Stack {
public:
    using value_type = typename Sequence::value_type;
    using size_type = typename Sequence::size_type;
    using reference = typename Sequence::reference;
    using const_reference = typename Sequence::const_reference;

    friend bool operator==(const Stack& x, const Stack& y) {
        return x.c == y.c;
    }

    friend bool operator<(const Stack& x, const Stack& y) {
        return x.c < y.c;
    }

protected:
    Sequence c;  // 底层容器
public:
    bool empty() const {
        return c.empty();
    }
    size_type size() const {
        return c.size();
    }
    reference top() {
        return c.back();
    }
    const_reference top() const {
        return c.back();
    }
    void push(const value_type& x) {
        c.push_back(x);
    }
    void pop() {
        c.pop_back();
    }
};

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_STACK_H