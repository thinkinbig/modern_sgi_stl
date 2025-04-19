#ifndef __MSGI_STL_INTERNAL_QUEUE_H
#define __MSGI_STL_INTERNAL_QUEUE_H

#include "mstl_deque.h"
#include "mstl_functional.h"
#include "mstl_heap.h"
#include "mstl_vector.h"

namespace mstl {

template <class T, class Sequence = Deque<T>>
class Queue {
public:
    using value_type = typename Sequence::value_type;
    using size_type = typename Sequence::size_type;
    using reference = typename Sequence::reference;
    using const_reference = typename Sequence::const_reference;

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

template <class T, class Sequence = Deque<T>>
bool operator==(const Queue<T, Sequence>& x, const Queue<T, Sequence>& y) {
    return x.c == y.c;
}

template <class T, class Sequence = Deque<T>>
bool operator<(const Queue<T, Sequence>& x, const Queue<T, Sequence>& y) {
    return x.c < y.c;
}

template <class T, class Sequence = Vector<T>, class Compare = Less<typename Sequence::value_type>>
class PriorityQueue {
public:
    using value_type = typename Sequence::value_type;
    using size_type = typename Sequence::size_type;
    using reference = typename Sequence::reference;
    using const_reference = typename Sequence::const_reference;

protected:
    Sequence c;
    Compare comp;

public:
    PriorityQueue() : c() {}
    explicit PriorityQueue(const Compare& x) : c(), comp(x) {}

    template <class InputIterator>
    PriorityQueue(InputIterator first, InputIterator last, const Compare& x)
        : c(first, last), comp(x) {
        mstl::make_heap(c.begin(), c.end(), comp);
    }

    template <class InputIterator>
    PriorityQueue(InputIterator first, InputIterator last) : c(first, last) {
        mstl::make_heap(c.begin(), c.end(), comp);
    }

    bool empty() const {
        return c.empty();
    }

    size_type size() const {
        return c.size();
    }

    const_reference top() const {
        return c.front();
    }

    void push(const value_type& x) {
        try {
            c.push_back(x);
            mstl::push_heap(c.begin(), c.end(), comp);
        } catch (...) {
            c.clear();
            throw;
        }
    }

    void pop() {
        try {
            mstl::pop_heap(c.begin(), c.end(), comp);
            c.pop_back();
        } catch (...) {
            c.clear();
            throw;
        }
    }
};

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_QUEUE_H