#ifndef __MSGI_STL_INTERNAL_DEQUE_H
#define __MSGI_STL_INTERNAL_DEQUE_H

#include <initializer_list>
#include <stdexcept>

#include "mstl_alloc.h"
#include "mstl_allocator.h"
#include "mstl_construct.h"
#include "mstl_iterator_tags.h"
#include "mstl_uninitialized.h"

namespace mstl {

// deque的缓冲区大小计算
size_t __deque_buf_size(size_t sz) {
    return sz < 512 ? size_t(512 / sz) : size_t(1);
}

template <typename Tp, typename Ref, typename Ptr>
struct DequeIterator {
    using IteratorCategory = BidirectionalIteratorTag;
    using ValueType = Tp;
    using DifferenceType = ptrdiff_t;
    using Pointer = Ptr;
    using Reference = Ref;

    using MapPointer = Tp**;
    using Self = DequeIterator<Tp, Ref, Ptr>;
    using Iterator = DequeIterator<Tp, Tp&, Tp*>;
    using ConstIterator = DequeIterator<Tp, const Tp&, const Tp*>;

    // 迭代器所含的5个指针
    Tp* cur;
    Tp* first;
    Tp* last;
    MapPointer node;

    // 构造函数
    DequeIterator(Tp* x, MapPointer y) : cur(x), first(*y), last(*y + buffer_size()), node(y) {}
    DequeIterator() : cur(0), first(0), last(0), node(0) {}
    DequeIterator(const Self& x) : cur(x.cur), first(x.first), last(x.last), node(x.node) {}

    Self& operator=(const Self& x) {
        if (this != &x) {
            cur = x.cur;
            first = x.first;
            last = x.last;
            node = x.node;
        }
        return *this;
    }

    static size_t buffer_size() {
        return __deque_buf_size(sizeof(Tp));
    }

    Reference operator*() const {
        return *cur;
    }
    Pointer operator->() const {
        return cur;
    }

    DifferenceType operator-(const Self& x) const {
        return buffer_size() * (node - x.node - 1) + (x.last - x.cur) + (cur - first);
    }

    Self& operator++() {
        ++cur;
        if (cur == last) {
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }

    Self operator++(int) {
        Self tmp = *this;
        ++(*this);
        return tmp;
    }

    Self& operator--() {
        if (cur == first) {
            set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    Self operator--(int) {
        Self tmp = *this;
        --*this;
        return tmp;
    }

    Self& operator+=(DifferenceType n) {
        DifferenceType offset = n + (cur - first);
        if (offset >= 0 && offset < DifferenceType(buffer_size())) {
            cur += n;
        } else {
            DifferenceType node_offset = offset > 0
                                              ? offset / DifferenceType(buffer_size())
                                              : -DifferenceType(-offset - 1) / buffer_size();
            set_node(node + node_offset);
            cur = first + (offset - node_offset * DifferenceType(buffer_size()));
        }
        return *this;
    }

    Self operator+(DifferenceType n) const {
        Self tmp = *this;
        return tmp += n;
    }

    Self& operator-=(DifferenceType n) {
        return *this += -n;
    }

    Self operator-(DifferenceType n) const {
        Self tmp = *this;
        return tmp -= n;
    }

    Reference operator[](DifferenceType n) const {
        return *(*this + n);
    }

    bool operator==(const Self& x) const {
        return cur == x.cur;
    }
    bool operator!=(const Self& x) const {
        return cur != x.cur;
    }
    bool operator<(const Self& x) const {
        return node == x.node ? cur < x.cur : node < x.node;
    }
    bool operator>(const Self& x) const {
        return x < *this;
    }
    bool operator<=(const Self& x) const {
        return !(x < *this);
    }
    bool operator>=(const Self& x) const {
        return !(*this < x);
    }

    void set_node(MapPointer new_node) {
        node = new_node;
        first = *node;
        last = first + DifferenceType(buffer_size());
    }
};

template <typename Tp, typename Alloc = Allocator<Tp>>
class Deque {
public:
    using ValueType = Tp;
    using Pointer = ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

    using Iterator = DequeIterator<Tp, Tp&, Tp*>;
    using ConstIterator = DequeIterator<Tp, const Tp&, const Tp*>;
    using ReverseIterator = mstl::ReverseIterator<Iterator>;
    using ConstReverseIterator = mstl::ReverseIterator<ConstIterator>;

    using MapPointer = Pointer*;

    using DataAllocator = SimpleAlloc<ValueType, Alloc>;
    using MapAllocator = SimpleAlloc<Pointer, Alloc>;

    static size_t buffer_size() {
        return __deque_buf_size(sizeof(Tp));
    }

    // 添加比较运算符
    friend bool operator==(const Deque& x, const Deque& y) {
        if (x.size() != y.size())
            return false;
        return std::equal(x.begin(), x.end(), y.begin());
    }

    friend bool operator<(const Deque& x, const Deque& y) {
        return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

protected:
    Iterator start;      // 第一个节点
    Iterator finish;     // 最后一个节点
    MapPointer map;     // 指向map, map是连续空间
    SizeType map_size;  // map内有多少指针

public:
    // 构造函数
    Deque() : start(), finish(), map(nullptr), map_size(0) {
        create_map_and_nodes(1);  // 分配一个空节点
        start.cur = start.first;
        finish.cur = finish.first;
    }

    explicit Deque(SizeType n) : start(), finish(), map(nullptr), map_size(0) {
        fill_initialize(n, ValueType());
    }

    Deque(SizeType n, const ValueType& value) : start(), finish(), map(nullptr), map_size(0) {
        fill_initialize(n, value);
    }

    // 移动构造函数
    Deque(Deque&& other) noexcept : start(), finish(), map(nullptr), map_size(0) {
        // 交换资源
        start = other.start;
        finish = other.finish;
        map = other.map;
        map_size = other.map_size;

        // 清空源对象
        other.start = Iterator();
        other.finish = Iterator();
        other.map = nullptr;
        other.map_size = 0;
    }

    // 移动赋值运算符
    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            // 释放当前资源
            if (map) {
                destroy(start, finish);
                for (MapPointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                MapAllocator::deallocate(map, map_size);
            }

            // 交换资源
            start = other.start;
            finish = other.finish;
            map = other.map;
            map_size = other.map_size;

            // 清空源对象
            other.start = Iterator();
            other.finish = Iterator();
            other.map = nullptr;
            other.map_size = 0;
        }
        return *this;
    }

    // 拷贝构造函数
    Deque(const Deque& other) : start(), finish(), map(nullptr), map_size(0) {
        create_map_and_nodes(other.size());
        try {
            uninitialized_copy(other.start, other.finish, start);
        } catch (...) {
            destroy(start, finish);
            for (MapPointer node = start.node; node <= finish.node; ++node) {
                deallocate_node(*node);
            }
            MapAllocator::deallocate(map, map_size);
            throw;
        }
    }

    // 拷贝赋值运算符
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            // 释放当前资源
            if (map) {
                destroy(start, finish);
                for (MapPointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                MapAllocator::deallocate(map, map_size);
            }

            // 分配新资源并复制元素
            create_map_and_nodes(other.size());
            try {
                uninitialized_copy(other.start, other.finish, start);
            } catch (...) {
                destroy(start, finish);
                for (MapPointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                MapAllocator::deallocate(map, map_size);
                throw;
            }
        }
        return *this;
    }

    // 析构函数
    ~Deque() {
        if (map) {
            // 销毁所有元素
            destroy(start, finish);

            // 释放所有节点
            for (MapPointer node = start.node; node <= finish.node; ++node) {
                deallocate_node(*node);
            }

            // 释放 map 数组
            MapAllocator::deallocate(map, map_size);
            map = nullptr;
            map_size = 0;
        }
    }

    Iterator begin() {
        return start;
    }
    ConstIterator begin() const {
        return ConstIterator(start.cur, start.node);
    }
    Iterator end() {
        return finish;
    }
    ConstIterator end() const {
        return ConstIterator(finish.cur, finish.node);
    }

    Reference operator[](SizeType n) {
        return *(start + DifferenceType(n));
    }

    Reference front() {
        return *start;
    }
    ConstReference front() const {
        return *start;
    }

    Reference back() {
        return *(finish - 1);
    }
    ConstReference back() const {
        return *(finish - 1);
    }

    SizeType size() const {
        return SizeType(finish - start);
    }

    bool empty() const {
        return finish == start;
    }

    void fill_initialize(SizeType n, const ValueType& value) {
        create_map_and_nodes(n);
        MapPointer cur;
        try {
            for (cur = start.node; cur < finish.node; ++cur) {
                uninitialized_fill(*cur, *cur + buffer_size(), value);
            }
            // 最后填充最后一个节点 和前面稍有不同
            uninitialized_fill(finish.first, finish.cur, value);
        } catch (...) {
            destroy(start, finish);
            throw;
        }
    }

    void create_map_and_nodes(SizeType num_elements) {
        SizeType num_nodes = num_elements / buffer_size() + 1;
        map_size = std::max(initial_map_size(), num_nodes + 2);
        map = MapAllocator::allocate(map_size);
        MapPointer nstart = map + (map_size - num_nodes) / 2;
        MapPointer nfinish = nstart + num_nodes;

        MapPointer cur;
        try {
            for (cur = nstart; cur < nfinish; ++cur) {
                *cur = allocate_node();
            }
        } catch (...) {
            // 释放已分配的节点
            for (MapPointer node = nstart; node < cur; ++node) {
                deallocate_node(*node);
            }
            // 释放 map 数组
            MapAllocator::deallocate(map, map_size);
            map = nullptr;
            map_size = 0;
            throw;
        }

        start.set_node(nstart);
        finish.set_node(nfinish - 1);
        start.cur = start.first;
        finish.cur = finish.first + num_elements % buffer_size();
    }

    SizeType initial_map_size() const {
        return std::max(map_size, SizeType(8));
    }

    Pointer allocate_node() {
        return DataAllocator::allocate(buffer_size());
    }

    void deallocate_node(Pointer p) {
        DataAllocator::deallocate(p, buffer_size());
    }

    template <typename U>
    void push_back(U&& x)
        requires std::constructible_from<ValueType, U>
    {
        if (finish.cur != finish.last - 1) {
            construct(finish.cur, std::forward<U>(x));
            ++finish.cur;
        } else {
            __push_back_aux(std::forward<U>(x));
        }
    }

    template <typename U>
    void push_front(U&& x)
        requires std::constructible_from<ValueType, U>
    {
        if (start.cur != start.first) {
            construct(start.cur - 1, std::forward<U>(x));
            --start.cur;
        } else {
            __push_front_aux(std::forward<U>(x));
        }
    }

    void pop_back() {
        if (finish.cur != finish.first) {
            --finish.cur;
            destroy(finish.cur);
        } else {
            __pop_back_aux();
        }
    }

    void __pop_back_aux() {
        deallocate_node(finish.first);
        finish.set_node(finish.node - 1);
        finish.cur = finish.last - 1;
        destroy(finish.cur);
    }

    void pop_front() {
        if (start.cur != start.last - 1) {
            destroy(start.cur);
            ++start.cur;
        } else {
            __pop_front_aux();
        }
    }

    void __pop_front_aux() {
        destroy(start.cur);
        deallocate_node(start.first);
        start.set_node(start.node + 1);
        start.cur = start.first;
    }

    void clear() {
        // 销毁并释放中间节点
        destroy(start, finish);
        // 处理首尾节点
        if (start.node != finish.node) {
            destroy(start.cur, start.last);
            destroy(finish.first, finish.cur);
            deallocate_node(finish.first);
        } else {
            destroy(start.cur, finish.cur);
        }

        // 重置迭代器
        finish = start;
    }

    Iterator erase(Iterator pos) {
        Iterator next = pos;
        ++next;
        DifferenceType index = pos - start;
        if (index < DifferenceType(size() >> 1)) {
            std::copy_backward(start, pos, next);
            pop_front();
        } else {
            std::copy(next, finish, pos);
            pop_back();
        }
        return start + index;
    }

    Iterator erase(Iterator first, Iterator last) {
        if (first == start && last == finish) {
            clear();
            return finish;
        } else {
            DifferenceType n = last - first;
            DifferenceType elems_before = first - start;
            if (elems_before < DifferenceType((size() - n) / 2)) {
                std::copy_backward(start, first, last);
                Iterator new_start = start + n;
                destroy(start, new_start);

                for (MapPointer cur = start.node; cur < new_start.node; ++cur) {
                    DataAllocator::deallocate(*cur, buffer_size());
                }
                start = new_start;
            } else {
                std::copy(last, finish, first);
                Iterator new_finish = finish - n;
                destroy(new_finish, finish);

                for (MapPointer cur = new_finish.node + 1; cur <= finish.node; ++cur) {
                    DataAllocator::deallocate(*cur, buffer_size());
                }
                finish = new_finish;
            }
            return start + elems_before;
        }
    }

    void reserve_map_at_front(SizeType nodes_to_add = 1) {
        if (static_cast<SizeType>(start.node - map) < nodes_to_add) {
            reallocate_map(nodes_to_add, true);
        }
    }

    void reserve_map_at_back(SizeType nodes_to_add = 1) {
        if (static_cast<SizeType>(map_size - (finish.node - map)) < nodes_to_add + 1) {
            reallocate_map(nodes_to_add, false);
        }
    }

    void reallocate_map(SizeType nodes_to_add, bool add_at_front) {
        SizeType old_num_nodes = finish.node - start.node + 1;
        SizeType new_num_nodes = old_num_nodes + nodes_to_add;

        MapPointer new_start;
        if (map_size > 2 * new_num_nodes) {
            new_start = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            if (new_start < start.node) {
                std::copy(start.node, finish.node + 1, new_start);
            } else {
                std::copy_backward(start.node, finish.node + 1, new_start + old_num_nodes);
            }
        } else {
            SizeType new_map_size = map_size + std::max(map_size, nodes_to_add) + 2;
            MapPointer new_map = MapAllocator::allocate(new_map_size);
            new_start =
                new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            std::copy(start.node, finish.node + 1, new_start);
            MapAllocator::deallocate(map, map_size);
            map = new_map;
            map_size = new_map_size;
        }

        start.set_node(new_start);
        finish.set_node(new_start + old_num_nodes - 1);
    }

    Iterator insert(Iterator position, const ValueType& x) {
        if (position.cur == start.cur) {
            push_front(x);
            return start;
        }
        if (position.cur == finish.cur) {
            push_back(x);
            Iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, x);
        }
    }

    Iterator insert(Iterator position, ValueType&& x) {
        if (position.cur == start.cur) {
            push_front(std::move(x));
            return start;
        }
        if (position.cur == finish.cur) {
            push_back(std::move(x));
            Iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, std::move(x));
        }
    }

    template <typename... Args>
    Iterator emplace(Iterator position, Args&&... args)
        requires std::constructible_from<ValueType, Args...>
    {
        if (position.cur == start.cur) {
            emplace_front(std::forward<Args>(args)...);
            return start;
        }
        if (position.cur == finish.cur) {
            emplace_back(std::forward<Args>(args)...);
            Iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    Iterator __insert_aux(Iterator position, Args&&... args) {
        DifferenceType index = position - start;
        if (index < DifferenceType(size() / 2)) {
            push_front(front());
            Iterator front1 = start;
            ++front1;
            Iterator front2 = front1;
            ++front2;
            position = start + index;
            Iterator position1 = position;
            ++position1;
            std::copy(front2, position1, front1);
        } else {
            push_back(back());
            Iterator back1 = finish;
            --back1;
            Iterator back2 = back1;
            --back2;
            position = start + index;
            std::copy_backward(position, back2, back1);
        }
        construct(position.cur, std::forward<Args>(args)...);
        return position;
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
        requires std::constructible_from<ValueType, Args...>
    {
        if (start.cur != start.first) {
            construct(start.cur - 1, std::forward<Args>(args)...);
            --start.cur;
        } else {
            __push_front_aux(std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
        requires std::constructible_from<ValueType, Args...>
    {
        if (finish.cur != finish.last - 1) {
            construct(finish.cur, std::forward<Args>(args)...);
            ++finish.cur;
        } else {
            __push_back_aux(std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void __push_front_aux(Args&&... args)
        requires std::constructible_from<ValueType, Args...>
    {
        reserve_map_at_front();
        *(start.node - 1) = allocate_node();
        try {
            start.set_node(start.node - 1);
            start.cur = start.last - 1;
            construct(start.cur, std::forward<Args>(args)...);
        } catch (...) {
            start.set_node(start.node + 1);
            start.cur = start.first;
            deallocate_node(*(start.node - 1));
            throw;
        }
    }

    template <typename... Args>
    void __push_back_aux(Args&&... args)
        requires std::constructible_from<ValueType, Args...>
    {
        reserve_map_at_back();
        *(finish.node + 1) = allocate_node();
        try {
            construct(finish.cur, std::forward<Args>(args)...);
            finish.set_node(finish.node + 1);
            finish.cur = finish.first;
        } catch (...) {
            deallocate_node(*(finish.node + 1));
            throw;
        }
    }

    // 反向迭代器相关函数
    ReverseIterator rbegin() {
        return ReverseIterator(end());
    }

    ConstReverseIterator rbegin() const {
        return ConstReverseIterator(end());
    }

    ReverseIterator rend() {
        return ReverseIterator(begin());
    }

    ConstReverseIterator rend() const {
        return ConstReverseIterator(begin());
    }
};
}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_DEQUE_H