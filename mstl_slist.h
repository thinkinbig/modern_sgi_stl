#ifndef __MSGI_STL_INTERNAL_SLIST_H
#define __MSGI_STL_INTERNAL_SLIST_H

#include "mstl_alloc.h"
#include "mstl_allocator.h"
#include "mstl_iterator_tags.h"

namespace mstl {

template <typename T>
struct SlistNode {
    SlistNode* next;
    T data;
};

// 全局函数 已知某个节点插入新节点于其后
template <typename T>
SlistNode<T>* __mstl_slist_make_link(SlistNode<T>* prev_node, SlistNode<T>* new_node) {
    new_node->next = prev_node->next;
    prev_node->next = new_node;
    return new_node;
}

// 全局函数 单向链表的大小（元素个数）
template <typename T>
size_t __mstl_slist_size(SlistNode<T>* node) {
    size_t result = 0;
    for (; node != 0; node = node->next) {
        ++result;
    }
    return result;
}

template <typename T, typename Ref, typename Ptr>
struct SlistIterator {
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using IteratorCategory = ForwardIteratorTag;

    using Iterator = SlistIterator<T, T&, T*>;
    using ConstIterator = SlistIterator<T, const T&, const T*>;
    using Self = SlistIterator<T, Ref, Ptr>;
    using ValueType = T;
    using Pointer = Ptr;
    using Reference = Ref;
    using ListNode = SlistNode<T>;

    ListNode* node;

    SlistIterator(ListNode* x) : node(x) {}
    SlistIterator() : node(0) {}
    SlistIterator(const Iterator& x) : node(x.node) {}

    void incr() {
        node = node->next;
    }

    bool operator==(const Self& x) const {
        return node == x.node;
    }

    bool operator!=(const Self& x) const {
        return node != x.node;
    }

    Reference operator*() const {
        return node->data;
    }

    Pointer operator->() const {
        return &(operator*());
    }

    Self& operator++() {
        incr();
        return *this;
    }

    Self operator++(int) {
        Self tmp = *this;
        incr();
        return tmp;
    }
};

template <typename T, typename Alloc = alloc>
class Slist {
public:
    using ValueType = T;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

    using Iterator = SlistIterator<T, T&, T*>;
    using ConstIterator = SlistIterator<T, const T&, const T*>;

private:
    using ListNode = SlistNode<T>;
    using ListNodeAllocator = SimpleAlloc<ListNode, Alloc>;

    static ListNode* create_node(const ValueType& x) {
        ListNode* node = ListNodeAllocator::allocate();
        try {
            construct(&node->data, x);
            node->next = 0;
        } catch (...) {
            ListNodeAllocator::deallocate(node);
            throw;
        }
        return node;
    }

    static void destroy_node(ListNode* node) {
        destroy(&node->data);
        ListNodeAllocator::deallocate(node);
    }

private:
    ListNode head;

public:
    Slist() {
        head.next = nullptr;
    }
    ~Slist() {
        clear();
    }

    Iterator begin() {
        return Iterator(head.next);
    }

    Iterator end() {
        return Iterator(nullptr);
    }

    SizeType size() const {
        return __mstl_slist_size(head.next);
    }

    bool empty() const {
        return head.next == nullptr;
    }

    void swap(Slist& L) {
        ListNode* tmp = head.next;
        head.next = L.head.next;
        L.head.next = tmp;
    }

    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

public:
    Reference front() {
        return (head.next)->data;
    }

    void push_front(const ValueType& x) {
        __mstl_slist_make_link(&head, create_node(x));
    }

    void pop_front() {
        ListNode* node = head.next;
        head.next = node->next;
        destroy_node(node);
    }
};

}  // namespace mstl

#endif