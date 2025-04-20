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
    using ValueType = typename Sequence::ValueType;
    using SizeType = typename Sequence::SizeType;
    using Reference = typename Sequence::Reference;
    using ConstReference = typename Sequence::ConstReference;

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

    SizeType size() const {
        return c.size();
    }

    Reference front() {
        return c.front();
    }

    ConstReference front() const {
        return c.front();
    }

    Reference back() {
        return c.back();
    }

    ConstReference back() const {
        return c.back();
    }

    void push(const ValueType& x) {
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

template <class T, class Sequence = Vector<T>, class Compare = Less<typename Sequence::ValueType>>
class PriorityQueue {
public:
    using ValueType = typename Sequence::ValueType;
    using SizeType = typename Sequence::SizeType;
    using Reference = typename Sequence::Reference;
    using ConstReference = typename Sequence::ConstReference;

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

    SizeType size() const {
        return c.size();
    }

    ConstReference top() const {
        return c.front();
    }

    void push(const ValueType& x) {
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