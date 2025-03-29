#ifndef __MSGI_STL_INTERNAL_LIST_H
#define __MSGI_STL_INTERNAL_LIST_H

#include <cstddef>
#include "mstl_alloc.h"
#include "mstl_allocator.h"
#include "mstl_iterator.h"
#include "mstl_iterator_tags.h"
#include "mstl_iterator_traits.h"
namespace mstl {

template <typename T>
struct ListNode {
    ListNode* next;
    ListNode* prev;
    T data;

    ListNode() : next(nullptr), prev(nullptr) {}
    ListNode(const T& x) : next(nullptr), prev(nullptr), data(x) {}
};

template <typename T, typename Ref, typename Ptr>
struct ListIterator {
    using iterator_category = bidirectional_iterator_tag;
    using value_type = T;
    using pointer = Ptr;
    using reference = Ref;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using Node = ListNode<T>;

    using iterator = ListIterator<T, T&, T*>;
    using const_iterator = ListIterator<T, const T&, const T*>;
    using self = ListIterator<T, Ref, Ptr>;

    Node* kNode;

    ListIterator(Node* x) : kNode(x) {}
    ListIterator() : kNode(nullptr) {}

    // 允许从非const转换到const
    ListIterator(const self& x) : kNode(x.kNode) {}

    self& operator=(const self& x) {
        kNode = x.kNode;
        return *this;
    }

    self& operator=(self&& x) {
        kNode = x.kNode;
        x.kNode = nullptr;
        return *this;
    }

    ~ListIterator() {
        kNode = nullptr;
    }

    void incr() {
        kNode = kNode->next;
    }
    void decr() {
        kNode = kNode->prev;
    }

    reference operator*() const {
        return kNode->data;
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

    self& operator--() {
        decr();
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        decr();
        return tmp;
    }

    bool operator==(const self& x) const {
        return kNode == x.kNode;
    }

    bool operator!=(const self& x) const {
        return kNode != x.kNode;
    }
};

template <typename T, typename Alloc = alloc>
class List {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using Node = ListNode<T>;

    using iterator = ListIterator<T, T&, T*>;
    using const_iterator = ListIterator<T, const T&, const T*>;

    // 构造函数
    List() {
        createNode();
    }
    List(size_type n, const T& value = T()) {
        createNode();
        insert(begin(), n, value);
    }
    List(const List& x) {
        createNode();
        insert(begin(), x.begin(), x.end());
    }

    List(List&& x) noexcept {
        kNode = x.kNode;
        x.kNode = nullptr;
    }

    List& operator=(const List& x) {
        if (this != &x) {
            clear();
            insert(begin(), x.begin(), x.end());
        }
        return *this;
    }

    List& operator=(List&& x) noexcept {
        if (this != &x) {
            clear();
            kNode = x.kNode;
            x.kNode = nullptr;
        }
        return *this;
    }

    ~List() {
        clear();
        putNode(kNode);
    }

    // 迭代器相关
    iterator begin() {
        return kNode->next;
    }
    const_iterator begin() const {
        return kNode->next;
    }
    iterator end() {
        return kNode;
    }
    const_iterator end() const {
        return kNode;
    }

    // 容量相关
    bool empty() const {
        return kNode->next == kNode;
    }
    size_type size() const {
        return distance(begin(), end());
    }

    // 元素访问
    reference front() {
        return *begin();
    }
    const_reference front() const {
        return *begin();
    }
    reference back() {
        return *(--end());
    }
    const_reference back() const {
        return *(--end());
    }

    // 修改器
    void push_back(const T& x) {
        insert(end(), x);
    }
    void push_back(T&& x) {
        insert(end(), std::move(x));
    }
    void push_front(const T& x) {
        insert(begin(), x);
    }
    void push_front(T&& x) {
        insert(begin(), std::move(x));
    }

    void pop_back() {
        iterator tmp = --end();
        erase(tmp);
    }

    void pop_front() {
        erase(begin());
    }

    template <typename U>
    iterator insert(iterator position, U&& x) {
        Node* tmp = getNode();
        tmp->data = std::forward<U>(x);

        Node* node = position.kNode;

        tmp->next = node;
        node->prev->next = tmp;
        tmp->prev = node->prev;
        node->prev = tmp;

        return iterator(tmp);
    }

    template <typename U>
    iterator insert(iterator position, size_type n, U&& x) {
        for (size_type i = 0; i < n; ++i) {
            insert(position, x);
        }
        return position;
    }

    template <typename InputIterator>
    iterator insert(iterator position, InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insert(position, *first);
        }
        return position;
    }

    iterator erase(iterator position) {
        Node* node = position.kNode;
        Node* nextNode = node->next;
        Node* prevNode = node->prev;

        nextNode->prev = prevNode;
        prevNode->next = nextNode;

        putNode(node);

        return iterator(nextNode);
    }

    iterator erase(iterator first, iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return last;
    }

    void clear() {
        erase(begin(), end());
    }

    void splice(iterator position, List& x) {
        if (!x.empty()) {
            transfer(position, x.begin(), x.end());
        }
    }

    void splice(iterator position, List&& x) {
        if (!x.empty()) {
            transfer(position, x.begin(), x.end());
        }
    }

    void splice(iterator position, [[maybe_unused]] List& x, iterator i) {
        if (position != i) {
            transfer(position, i, i.kNode->next);
        }
    }

    void splice(iterator position, [[maybe_unused]] List&& x, iterator i) {
        if (position != i) {
            transfer(position, i, i.kNode->next);
        }
    }

    void splice(iterator position, [[maybe_unused]] List& x, iterator first, iterator last) {
        if (first != last) {
            transfer(position, first, last);
        }
    }

    void splice(iterator position, [[maybe_unused]] List&& x, iterator first, iterator last) {
        if (first != last) {
            transfer(position, first, last);
        }
    }

    void remove(const T& value) {
        iterator current = begin();
        while (current != end()) {
            if (*current == value) {
                current = erase(current);
            } else {
                ++current;
            }
        }
    }

protected:
    Node* kNode;
    using node_allocator =
        typename allocator_traits<SimpleAlloc<T, Alloc>>::template rebind_alloc<Node>;

    void createNode() {
        kNode = getNode();
        kNode->next = kNode;
        kNode->prev = kNode;
    }

    // 辅助函数
    void transfer(iterator position, iterator first, iterator last) {
        if (position != first && position != last) {
            Node* firstNode = first.kNode;
            Node* lastNode = last.kNode->prev;

            firstNode->prev->next = last.kNode;
            last.kNode->prev = firstNode->prev;

            firstNode->prev = position.kNode->prev;
            lastNode->next = position.kNode;

            position.kNode->prev->next = firstNode;
            position.kNode->prev = lastNode;
        }
    }

    void putNode(Node* p) {
        node_allocator::deallocate(p, 1);
    }

    Node* getNode() {
        return node_allocator::allocate(1);
    }
};
}  // namespace mstl
#endif