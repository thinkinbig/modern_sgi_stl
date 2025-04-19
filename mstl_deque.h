#ifndef __MSGI_STL_INTERNAL_DEQUE_H
#define __MSGI_STL_INTERNAL_DEQUE_H

#include <initializer_list>
#include <stdexcept>

#include "mstl_alloc.h"
#include "mstl_allocator.h"
#include "mstl_construct.h"
#include "mstl_uninitialized.h"
#include "mstl_iterator_tags.h"

namespace mstl {

// deque的缓冲区大小计算
size_t __deque_buf_size(size_t sz) {
    return sz < 512 ? size_t(512 / sz) : size_t(1);
}

template <typename Tp, typename Ref, typename Ptr>
struct DequeIterator {
    using iterator_category = bidirectional_iterator_tag;
    using value_type = Tp;
    using difference_type = ptrdiff_t;
    using pointer = Ptr;
    using reference = Ref;

    using map_pointer = Tp**;
    using self = DequeIterator<Tp, Ref, Ptr>;
    using iterator = DequeIterator<Tp, Tp&, Tp*>;
    using const_iterator = DequeIterator<Tp, const Tp&, const Tp*>;

    // 迭代器所含的5个指针
    Tp* cur;
    Tp* first;
    Tp* last;
    map_pointer node;

    // 构造函数
    DequeIterator(Tp* x, map_pointer y) : cur(x), first(*y), last(*y + buffer_size()), node(y) {}
    DequeIterator() : cur(0), first(0), last(0), node(0) {}
    DequeIterator(const self& x) : cur(x.cur), first(x.first), last(x.last), node(x.node) {}

    self& operator=(const self& x) {
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

    reference operator*() const {
        return *cur;
    }
    pointer operator->() const {
        return cur;
    }

    difference_type operator-(const self& x) const {
        return buffer_size() * (node - x.node - 1) + (x.last - x.cur) + (cur - first);
    }

    self& operator++() {
        ++cur;
        if (cur == last) {
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        ++(*this);
        return tmp;
    }

    self& operator--() {
        if (cur == first) {
            set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        --*this;
        return tmp;
    }

    self& operator+=(difference_type n) {
        difference_type offset = n + (cur - first);
        if (offset >= 0 && offset < difference_type(buffer_size())) {
            cur += n;
        } else {
            difference_type node_offset = offset > 0
                                              ? offset / difference_type(buffer_size())
                                              : -difference_type(-offset - 1) / buffer_size();
            set_node(node + node_offset);
            cur = first + (offset - node_offset * difference_type(buffer_size()));
        }
        return *this;
    }

    self operator+(difference_type n) const {
        self tmp = *this;
        return tmp += n;
    }

    self& operator-=(difference_type n) {
        return *this += -n;
    }

    self operator-(difference_type n) const {
        self tmp = *this;
        return tmp -= n;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }

    bool operator==(const self& x) const {
        return cur == x.cur;
    }
    bool operator!=(const self& x) const {
        return cur != x.cur;
    }
    bool operator<(const self& x) const {
        return node == x.node ? cur < x.cur : node < x.node;
    }
    bool operator>(const self& x) const {
        return x < *this;
    }
    bool operator<=(const self& x) const {
        return !(x < *this);
    }
    bool operator>=(const self& x) const {
        return !(*this < x);
    }

    void set_node(map_pointer new_node) {
        node = new_node;
        first = *node;
        last = first + difference_type(buffer_size());
    }
};

template <typename Tp, typename Alloc = Allocator<Tp>>
class Deque {
public:
    using value_type = Tp;
    using pointer = value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using iterator = DequeIterator<Tp, Tp&, Tp*>;
    using const_iterator = DequeIterator<Tp, const Tp&, const Tp*>;
    using reverse_iterator = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;

    using map_pointer = pointer*;

    using data_allocator = SimpleAlloc<Tp, Alloc>;
    using map_allocator = SimpleAlloc<pointer, Alloc>;

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
    iterator start;      // 第一个节点
    iterator finish;     // 最后一个节点
    map_pointer map;     // 指向map, map是连续空间
    size_type map_size;  // map内有多少指针

public:
    // 构造函数
    Deque() : start(), finish(), map(nullptr), map_size(0) {
        create_map_and_nodes(1);  // 分配一个空节点
        start.cur = start.first;
        finish.cur = finish.first;
    }

    explicit Deque(size_type n) : start(), finish(), map(nullptr), map_size(0) {
        fill_initialize(n, value_type());
    }

    Deque(size_type n, const value_type& value) : start(), finish(), map(nullptr), map_size(0) {
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
        other.start = iterator();
        other.finish = iterator();
        other.map = nullptr;
        other.map_size = 0;
    }

    // 移动赋值运算符
    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            // 释放当前资源
            if (map) {
                destroy(start, finish);
                for (map_pointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                map_allocator::deallocate(map, map_size);
            }

            // 交换资源
            start = other.start;
            finish = other.finish;
            map = other.map;
            map_size = other.map_size;

            // 清空源对象
            other.start = iterator();
            other.finish = iterator();
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
            for (map_pointer node = start.node; node <= finish.node; ++node) {
                deallocate_node(*node);
            }
            map_allocator::deallocate(map, map_size);
            throw;
        }
    }

    // 拷贝赋值运算符
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            // 释放当前资源
            if (map) {
                destroy(start, finish);
                for (map_pointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                map_allocator::deallocate(map, map_size);
            }

            // 分配新资源并复制元素
            create_map_and_nodes(other.size());
            try {
                uninitialized_copy(other.start, other.finish, start);
            } catch (...) {
                destroy(start, finish);
                for (map_pointer node = start.node; node <= finish.node; ++node) {
                    deallocate_node(*node);
                }
                map_allocator::deallocate(map, map_size);
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
            for (map_pointer node = start.node; node <= finish.node; ++node) {
                deallocate_node(*node);
            }

            // 释放 map 数组
            map_allocator::deallocate(map, map_size);
            map = nullptr;
            map_size = 0;
        }
    }

    iterator begin() {
        return start;
    }
    const_iterator begin() const {
        return const_iterator(start.cur, start.node);
    }
    iterator end() {
        return finish;
    }
    const_iterator end() const {
        return const_iterator(finish.cur, finish.node);
    }

    reference operator[](size_type n) {
        return *(start + difference_type(n));
    }

    reference front() {
        return *start;
    }
    const_reference front() const {
        return *start;
    }

    reference back() {
        return *(finish - 1);
    }
    const_reference back() const {
        return *(finish - 1);
    }

    size_type size() const {
        return size_type(finish - start);
    }

    bool empty() const {
        return finish == start;
    }

    void fill_initialize(size_type n, const value_type& value) {
        create_map_and_nodes(n);
        map_pointer cur;
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

    void create_map_and_nodes(size_type num_elements) {
        size_type num_nodes = num_elements / buffer_size() + 1;
        map_size = std::max(initial_map_size(), num_nodes + 2);
        map = map_allocator::allocate(map_size);
        map_pointer nstart = map + (map_size - num_nodes) / 2;
        map_pointer nfinish = nstart + num_nodes;

        map_pointer cur;
        try {
            for (cur = nstart; cur < nfinish; ++cur) {
                *cur = allocate_node();
            }
        } catch (...) {
            // 释放已分配的节点
            for (map_pointer node = nstart; node < cur; ++node) {
                deallocate_node(*node);
            }
            // 释放 map 数组
            map_allocator::deallocate(map, map_size);
            map = nullptr;
            map_size = 0;
            throw;
        }

        start.set_node(nstart);
        finish.set_node(nfinish - 1);
        start.cur = start.first;
        finish.cur = finish.first + num_elements % buffer_size();
    }

    size_type initial_map_size() const {
        return std::max(map_size, size_type(8));
    }

    pointer allocate_node() {
        return data_allocator::allocate(buffer_size());
    }

    void deallocate_node(pointer p) {
        data_allocator::deallocate(p, buffer_size());
    }

    template <typename U>
    void push_back(U&& x)
        requires std::constructible_from<value_type, U>
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
        requires std::constructible_from<value_type, U>
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

    iterator erase(iterator pos) {
        iterator next = pos;
        ++next;
        difference_type index = pos - start;
        if (index < difference_type(size() >> 1)) {
            std::copy_backward(start, pos, next);
            pop_front();
        } else {
            std::copy(next, finish, pos);
            pop_back();
        }
        return start + index;
    }

    iterator erase(iterator first, iterator last) {
        if (first == start && last == finish) {
            clear();
            return finish;
        } else {
            difference_type n = last - first;
            difference_type elems_before = first - start;
            if (elems_before < difference_type((size() - n) / 2)) {
                std::copy_backward(start, first, last);
                iterator new_start = start + n;
                destroy(start, new_start);

                for (map_pointer cur = start.node; cur < new_start.node; ++cur) {
                    data_allocator::deallocate(*cur, buffer_size());
                }
                start = new_start;
            } else {
                std::copy(last, finish, first);
                iterator new_finish = finish - n;
                destroy(new_finish, finish);

                for (map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur) {
                    data_allocator::deallocate(*cur, buffer_size());
                }
                finish = new_finish;
            }
            return start + elems_before;
        }
    }

    void reserve_map_at_front(size_type nodes_to_add = 1) {
        if (static_cast<size_type>(start.node - map) < nodes_to_add) {
            reallocate_map(nodes_to_add, true);
        }
    }

    void reserve_map_at_back(size_type nodes_to_add = 1) {
        if (static_cast<size_type>(map_size - (finish.node - map)) < nodes_to_add + 1) {
            reallocate_map(nodes_to_add, false);
        }
    }

    void reallocate_map(size_type nodes_to_add, bool add_at_front) {
        size_type old_num_nodes = finish.node - start.node + 1;
        size_type new_num_nodes = old_num_nodes + nodes_to_add;

        map_pointer new_start;
        if (map_size > 2 * new_num_nodes) {
            new_start = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            if (new_start < start.node) {
                std::copy(start.node, finish.node + 1, new_start);
            } else {
                std::copy_backward(start.node, finish.node + 1, new_start + old_num_nodes);
            }
        } else {
            size_type new_map_size = map_size + std::max(map_size, nodes_to_add) + 2;
            map_pointer new_map = map_allocator::allocate(new_map_size);
            new_start =
                new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            std::copy(start.node, finish.node + 1, new_start);
            map_allocator::deallocate(map, map_size);
            map = new_map;
            map_size = new_map_size;
        }

        start.set_node(new_start);
        finish.set_node(new_start + old_num_nodes - 1);
    }

    iterator insert(iterator position, const value_type& x) {
        if (position.cur == start.cur) {
            push_front(x);
            return start;
        }
        if (position.cur == finish.cur) {
            push_back(x);
            iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, x);
        }
    }

    iterator insert(iterator position, value_type&& x) {
        if (position.cur == start.cur) {
            push_front(std::move(x));
            return start;
        }
        if (position.cur == finish.cur) {
            push_back(std::move(x));
            iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, std::move(x));
        }
    }

    template <typename... Args>
    iterator emplace(iterator position, Args&&... args)
        requires std::constructible_from<value_type, Args...>
    {
        if (position.cur == start.cur) {
            emplace_front(std::forward<Args>(args)...);
            return start;
        }
        if (position.cur == finish.cur) {
            emplace_back(std::forward<Args>(args)...);
            iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return __insert_aux(position, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    iterator __insert_aux(iterator position, Args&&... args) {
        difference_type index = position - start;
        if (index < difference_type(size() / 2)) {
            push_front(front());
            iterator front1 = start;
            ++front1;
            iterator front2 = front1;
            ++front2;
            position = start + index;
            iterator position1 = position;
            ++position1;
            std::copy(front2, position1, front1);
        } else {
            push_back(back());
            iterator back1 = finish;
            --back1;
            iterator back2 = back1;
            --back2;
            position = start + index;
            std::copy_backward(position, back2, back1);
        }
        construct(position.cur, std::forward<Args>(args)...);
        return position;
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
        requires std::constructible_from<value_type, Args...>
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
        requires std::constructible_from<value_type, Args...>
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
        requires std::constructible_from<value_type, Args...>
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
        requires std::constructible_from<value_type, Args...>
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
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }
};
}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_DEQUE_H