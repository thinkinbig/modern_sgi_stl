#ifndef __MSGI_STL_INTERNAL_STACK_H
#define __MSGI_STL_INTERNAL_STACK_H

#include <stack>
#include "mstl_deque.h"

namespace mstl {
template <typename Tp, typename Sequence = Deque<Tp>>
class Stack {
    using value_type = Sequence::value_type;
    using size_type = Sequence::size_type;
    using reference = Sequence::reference;
    using const_reference = Sequence::const_reference;

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