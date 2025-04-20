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
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using iterator_category = ForwardIteratorTag;

    using iterator = SlistIterator<T, T&, T*>;
    using const_iterator = SlistIterator<T, const T&, const T*>;
    using self = SlistIterator<T, Ref, Ptr>;
    using value_type = T;
    using pointer = Ptr;
    using reference = Ref;
    using list_node = SlistNode<T>;

    list_node* node;

    SlistIterator(list_node* x) : node(x) {}
    SlistIterator() : node(0) {}
    SlistIterator(const iterator& x) : node(x.node) {}

    void incr() {
        node = node->next;
    }

    bool operator==(const self& x) const {
        return node == x.node;
    }

    bool operator!=(const self& x) const {
        return node != x.node;
    }

    reference operator*() const {
        return node->data;
    }

    pointer operator->() const {
        return &(operator*());
    }

    self& operator++() {
        incr();
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        incr();
        return tmp;
    }
};

template <typename T, typename Alloc = alloc>
class Slist {
public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using iterator = SlistIterator<T, T&, T*>;
    using const_iterator = SlistIterator<T, const T&, const T*>;

private:
    using list_node = SlistNode<T>;
    using list_node_allocator = SimpleAlloc<list_node, Alloc>;

    static list_node* create_node(const value_type& x) {
        list_node* node = list_node_allocator::allocate();
        try {
            construct(&node->data, x);
            node->next = 0;
        } catch (...) {
            list_node_allocator::deallocate(node);
            throw;
        }
        return node;
    }

    static void destroy_node(list_node* node) {
        destroy(&node->data);
        list_node_allocator::deallocate(node);
    }

private:
    list_node head;

public:
    Slist() {
        head.next = nullptr;
    }
    ~Slist() {
        clear();
    }

    iterator begin() {
        return iterator(head.next);
    }

    iterator end() {
        return iterator(nullptr);
    }

    size_type size() const {
        return __mstl_slist_size(head.next);
    }

    bool empty() const {
        return head.next == nullptr;
    }

    void swap(Slist& L) {
        list_node* tmp = head.next;
        head.next = L.head.next;
        L.head.next = tmp;
    }

    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

public:
    reference front() {
        return (head.next)->data;
    }

    void push_front(const value_type& x) {
        __mstl_slist_make_link(&head, create_node(x));
    }

    void pop_front() {
        list_node* node = head.next;
        head.next = node->next;
        destroy_node(node);
    }
};

}  // namespace mstl

#endif