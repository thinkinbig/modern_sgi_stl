#ifndef __MSGI_STL_INTERNAL_DEQUE_H
#define __MSGI_STL_INTERNAL_DEQUE_H

#include <initializer_list>
#include <stdexcept>

#include "mstl_alloc.h"
#include "mstl_uninitialized.h"
#include "mstl_construct.h"
#include "mstl_allocator.h"


namespace mstl
{

    // deque的缓冲区大小计算
    size_t __deque_buf_size(size_t sz)
    {
        return sz < 512 ? size_t(512 / sz) : size_t(1);
    }

    template <typename Tp, typename Ref, typename Ptr>
    struct DequeIterator
    {
        using iterator_category = random_access_iterator_tag;
        using value_type = Tp;
        using difference_type = ptrdiff_t;
        using pointer = Ptr;
        using reference = Ref;

        using map_pointer = Tp **;
        using self = DequeIterator<Tp, Ref, Ptr>;
        using iterator = DequeIterator<Tp, Tp &, Tp *>;
        using const_iterator = DequeIterator<Tp, const Tp &, const Tp *>;

        // 迭代器所含的5个指针
        Tp *cur;
        Tp *first;
        Tp *last;
        map_pointer node;

        // 构造函数
        DequeIterator(Tp *x, map_pointer y) : cur(x), first(*y), last(*y + buffer_size()), node(y) {}
        DequeIterator() : cur(0), first(0), last(0), node(0) {}
        DequeIterator(const self &x) : cur(x.cur), first(x.first), last(x.last), node(x.node) {}

        static size_t buffer_size() { return __deque_buf_size(sizeof(Tp)); }

        reference operator*() const { return *cur; }
        pointer operator->() const { return cur; }

        difference_type operator-(const self &x)
        {
            return buffer_size() * (node - x.node - 1) + (x.last - x.cur) + (cur - first);
        }

        self &operator++()
        {
            ++cur;
            if (cur == last)
            {
                set_node(node + 1);
                cur = first;
            }
            return *this;
        }

        self operator++(int)
        {
            self tmp = *this;
            ++(*this);
            return tmp;
        }

        self &operator--()
        {
            if (cur == first)
            {
                set_node(node - 1);
                cur = last;
            }
            --cur;
            return *this;
        }

        self operator--(int)
        {
            self tmp = *this;
            --*this;
            return tmp;
        }

        self &operator+=(difference_type n)
        {
            difference_type offset = n + (cur - first);
            if (offset >= 0 && offset < difference_type(buffer_size()))
            {
                cur += n;
            }
            else
            {
                difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size())
                                                         : -difference_type(-offset - 1) / buffer_size();
                set_node(node + node_offset);
                cur = first + (offset - node_offset * difference_type(buffer_size()));
            }
            return *this;
        }

        self operator+(difference_type n) const
        {
            self tmp = *this;
            return tmp += n;
        }

        self &operator-=(difference_type n)
        {
            return *this += -n;
        }

        self operator-(difference_type n) const
        {
            self tmp = *this;
            return tmp -= n;
        }

        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }

        bool operator==(const self &x) const { return cur == x.cur; }
        bool operator!=(const self &x) const { return cur != x.cur; }
        bool operator<(const self &x) const { return node == x.node ? cur < x.cur : node < x.node; }
        bool operator>(const self &x) const { return x < *this; }
        bool operator<=(const self &x) const { return !(x < *this); }
        bool operator>=(const self &x) const { return !(*this < x); }

        void set_node(map_pointer new_node)
        {
            node = new_node;
            first = *node;
            last = first + difference_type(buffer_size());
        }
    };

    template <typename Tp, typename Alloc = alloc>
    class Deque
    {
    public:
        using value_type = Tp;
        using pointer = value_type *;
        using reference = value_type &;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        using iterator = DequeIterator<Tp, Tp &, Tp *>;
        using const_iterator = DequeIterator<Tp, const Tp &, const Tp *>;

        using map_pointer = pointer *;

        using data_allocator = SimpleAlloc<Tp, Alloc>;
        using map_allocator = SimpleAlloc<pointer, Alloc>;

        static size_t buffer_size() { return __deque_buf_size(sizeof(Tp)); }

    protected:
        iterator start; // 第一个节点
        iterator finish; // 最后一个节点
        map_pointer map; // 指向map, map是连续空间
        size_type map_size; // map内有多少指针

    public:
        // 构造函数
        Deque(size_type n, const value_type &value) : start(), finish(), map(nullptr), map_size(0) {
            fill_initialize(n, value);
        }
        
        iterator begin() { return start; }
        const_iterator begin() const { return const_iterator(start.cur, start.node); }
        iterator end() { return finish; }
        const_iterator end() const { return const_iterator(finish.cur, finish.node); }

        reference operator[](size_type n) {
            return *(start + difference_type(n));
        }

        reference front() { return *start; }

        // TODO: 可能会有问题
        reference back() { return finish--; }

        size_type size() { return finish - start; }

        bool empty() const { return finish == start; }

        void fill_initialize(size_type n, const value_type &value) {
            create_map_and_nodes(n);
            map_pointer cur;
            try {
                for (cur = start.node; cur < finish.node; ++cur) {
                    uninitialized_fill(*cur, *cur + buffer_size(), value);
                }
                // 最后填充最后一个节点 和前面稍有不同
                uninitialized_fill(finish.first, finish.cur, value);
            }
            catch(...) {
                destroy(start, finish);
                throw;
            }
        }

        void create_map_and_nodes(size_type num_elements) {
            size_type num_nodes = num_elements / buffer_size() + 1;
            map_size = std::max(initial_map_size(), num_nodes + 2);
            map = map_allocator::allocate(map_size);
            map_pointer cur;
            try {
                for (cur = map; cur < map + map_size; ++cur) {
                    *cur = allocate_node();
                }
                start.set_node(map);
                finish.set_node(map + num_nodes - 1);
                start.cur = start.first;
                finish.cur = finish.first + num_elements % buffer_size();
            }
            catch(...) {
                // commit or rollback
                destroy(map, cur);
                map_allocator::deallocate(map, map_size);
                throw;
            }
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
    };
} // namespace mstl

#endif // __MSGI_STL_INTERNAL_DEQUE_H