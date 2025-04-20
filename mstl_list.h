#ifndef __MSGI_STL_INTERNAL_LIST_H
#define __MSGI_STL_INTERNAL_LIST_H

#include <cstddef>
#include <initializer_list>
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
    using IteratorCategory = BidirectionalIteratorTag;
    using ValueType = T;
    using Pointer = Ptr;
    using Reference = Ref;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Node = ListNode<T>;

    using Iterator = ListIterator<T, T&, T*>;
    using ConstIterator = ListIterator<T, const T&, const T*>;
    using Self = ListIterator<T, Ref, Ptr>;

    Node* kNode;

    ListIterator(Node* x) : kNode(x) {}
    ListIterator() : kNode(nullptr) {}

    // 允许从非const转换到const
    ListIterator(const Self& x) : kNode(x.kNode) {}

    Self& operator=(const Self& x) {
        kNode = x.kNode;
        return *this;
    }

    Self& operator=(Self&& x) {
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

    Reference operator*() const {
        return kNode->data;
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

    Self& operator--() {
        decr();
        return *this;
    }

    Self operator--(int) {
        Self tmp = *this;
        decr();
        return tmp;
    }

    bool operator==(const Self& x) const {
        return kNode == x.kNode;
    }

    bool operator!=(const Self& x) const {
        return kNode != x.kNode;
    }
};

template <typename T, typename Alloc = alloc>
class List {
public:
    using ValueType = T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = T&;
    using ConstReference = const T&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Node = ListNode<T>;

    using Iterator = ListIterator<T, T&, T*>;
    using ConstIterator = ListIterator<T, const T&, const T*>;

    // 构造函数
    List() {
        createNode();
    }
    List(SizeType n, const T& value = T()) {
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

    // 初始化列表构造函数
    List(std::initializer_list<T> il) {
        createNode();
        for (const auto& x : il) {
            push_back(x);
        }
    }

    // 迭代器相关
    Iterator begin() {
        return kNode->next;
    }
    ConstIterator begin() const {
        return kNode->next;
    }
    Iterator end() {
        return kNode;
    }
    ConstIterator end() const {
        return kNode;
    }

    // 容量相关
    bool empty() const {
        return kNode->next == kNode;
    }
    SizeType size() const {
        return distance(begin(), end());
    }

    // 元素访问
    Reference front() {
        return *begin();
    }
    ConstReference front() const {
        return *begin();
    }
    Reference back() {
        return *(--end());
    }
    ConstReference back() const {
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
        Iterator tmp = --end();
        erase(tmp);
    }

    void pop_front() {
        erase(begin());
    }

    template <typename U>
    Iterator insert(Iterator position, U&& x) {
        Node* tmp = getNode();
        tmp->data = std::forward<U>(x);

        Node* node = position.kNode;

        tmp->next = node;
        node->prev->next = tmp;
        tmp->prev = node->prev;
        node->prev = tmp;

        return Iterator(tmp);
    }

    template <typename U>
    Iterator insert(Iterator position, SizeType n, U&& x) {
        for (SizeType i = 0; i < n; ++i) {
            insert(position, x);
        }
        return position;
    }

    template <typename InputIterator>
    Iterator insert(Iterator position, InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insert(position, *first);
        }
        return position;
    }

    Iterator erase(Iterator position) {
        Node* node = position.kNode;
        Node* nextNode = node->next;
        Node* prevNode = node->prev;

        nextNode->prev = prevNode;
        prevNode->next = nextNode;

        putNode(node);

        return Iterator(nextNode);
    }

    Iterator erase(Iterator first, Iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return last;
    }

    void clear() {
        erase(begin(), end());
    }

    void splice(Iterator position, List& x) {
        if (!x.empty()) {
            transfer(position, x.begin(), x.end());
        }
    }

    void splice(Iterator position, List&& x) {
        if (!x.empty()) {
            transfer(position, x.begin(), x.end());
        }
    }

    void splice(Iterator position, [[maybe_unused]] List& x, Iterator i) {
        if (position != i) {
            transfer(position, i, i.kNode->next);
        }
    }

    void splice(Iterator position, [[maybe_unused]] List&& x, Iterator i) {
        if (position != i) {
            transfer(position, i, i.kNode->next);
        }
    }

    void splice(Iterator position, [[maybe_unused]] List& x, Iterator first, Iterator last) {
        if (first != last) {
            transfer(position, first, last);
        }
    }

    void splice(Iterator position, [[maybe_unused]] List&& x, Iterator first, Iterator last) {
        if (first != last) {
            transfer(position, first, last);
        }
    }

    void remove(const T& value) {
        Iterator current = begin();
        while (current != end()) {
            if (*current == value) {
                current = erase(current);
            } else {
                ++current;
            }
        }
    }

    friend bool operator==(const List<T, Alloc>& x, const List<T, Alloc>& y) {
        if (&x == &y)
            return true;  // 自反性检查 - 如果是同一个对象则立即返回true
        if (x.size() != y.size())
            return false;
        return std::equal(x.begin(), x.end(), y.begin());
    }

    friend bool operator<(const List<T, Alloc>& x, const List<T, Alloc>& y) {
        return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

protected:
    Node* kNode;
    using NodeAllocator = typename AllocatorTraits<SimpleAlloc<T, Alloc>>::template RebindAlloc<Node>;

    void createNode() {
        kNode = getNode();
        kNode->next = kNode;
        kNode->prev = kNode;
    }

    // 辅助函数
    void transfer(Iterator position, Iterator first, Iterator last) {
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
        NodeAllocator::deallocate(p, 1);
    }

    Node* getNode() {
        return NodeAllocator::allocate(1);
    }
};
}  // namespace mstl
#endif