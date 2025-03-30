#ifndef __MSGI_STL_INTERNAL_QUEUE_H
#define __MSGI_STL_INTERNAL_QUEUE_H

#include "mstl_deque.h"

namespace mstl {
template <typename Tp, typename Sequence = Deque<Tp>>
class Queue {
    using value_type = Sequence::value_type;
    using size_type = Sequence::size_type;
    using reference = Sequence::reference;
    using const_reference = Sequence::const_reference;

    friend bool operator==(const Queue& x, const Queue& y) {
        return x.c == y.c;
    }

    friend bool operator<(const Queue& x, const Queue& y) {
        return x.c < y.c;
    }

public:
    bool empty() const {
        return c.empty();
    }

    size_type size() const {
        return c.size();
    }

    reference front() {
        return c.front();
    }

    const_reference front() const {
        return c.front();
    }

    reference back() {
        return c.back();
    }

    const_reference back() const {
        return c.back();
    }

    void push(const value_type& x) {
        c.push_back(x);
    }

    void pop() {
        c.pop_front();
    }

protected:
    Sequence c;
};

template <typename Tp, typename Sequence>
bool operator==(const Queue<Tp, Sequence>& x, const Queue<Tp, Sequence>& y) {
    return x.c == y.c;
}

template <typename Tp, typename Sequence>
bool operator<(const Queue<Tp, Sequence>& x, const Queue<Tp, Sequence>& y) {
    return x.c < y.c;
}





}
#endif