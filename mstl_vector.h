#ifndef __MSGI_STL_INTERNAL_VECTOR_H
#define __MSGI_STL_INTERNAL_VECTOR_H

#include "mstl_alloc.h"
#include "mstl_construct.h"
#include "mstl_uninitialized.h"
#include <algorithm>
#include <type_traits>
#include <utility>

namespace mstl {

    template <typename T, typename Alloc = alloc>
    class vector {
    public:
        using value_type = T;
        using pointer = value_type*; // 指针式random access iterator 
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using reference = value_type&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

    protected:
        using data_allocator = simple_alloc<value_type, Alloc>;

        iterator start;
        iterator finish;
        iterator end_of_storage;
        
        // 使用模板和完美转发来同时处理左值和右值
        template <typename U>
        void insert_aux(iterator position, U&& x);

        void deallocate() {
            if (start) data_allocator::deallocate(start, end_of_storage - start);
        }

        void fill_initialize(size_type n, const T& value) {
            start = allocate_and_fill(n, value);
            finish = start + n;
            end_of_storage = finish;
        }
    public:
        // 迭代器相关
        iterator begin() { return start; }
        const_iterator begin() const { return start; }
        iterator end() { return finish; }
        const_iterator end() const { return finish; }
        
        // 容量相关
        size_type size() const { return size_type(finish - start); }
        size_type capacity() const {
            return size_type(end_of_storage - start);
        }
        bool empty() const { return start == finish; }
        
        // 元素访问
        reference operator[](size_type n) { return *(begin() + n); }
        
        // 构造函数
        vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}
        vector(size_type n, const T& value) { fill_initialize(n, value); }
        vector(int n, const T& value) { fill_initialize(n, value); }
        vector(long n, const T& value) { fill_initialize(n, value); }
        explicit vector(size_type n) { fill_initialize(n, T()); }

        ~vector() {
            destroy(start, finish);
            deallocate();
        }

        reference front() { return *begin(); }
        reference back() { return *(end() - 1); }
        
        // 修改容器 - 使用统一的push_back模板
        void push_back(const T& x) {
            if (finish != end_of_storage) {
                construct(finish, x);
                ++finish;
            } else {
                insert_aux(end(), x);
            }
        }
        
        // 添加noexcept移动语义版本
        void push_back(T&& x) noexcept(std::is_nothrow_move_constructible<T>::value) {
            if (finish != end_of_storage) {
                construct(finish, std::forward<T>(x));
                ++finish;
            } else {
                insert_aux(end(), std::forward<T>(x));
            }
        }

        void pop_back() {
            --finish;
            destroy(finish);
        }

        iterator erase(iterator position) {
            if (position + 1 != end()) {
                std::copy(position + 1, finish, position); // 后续元素往前移动
            }
            --finish;
            destroy(finish);
            return position;
        }
        
        // 添加区间删除版本
        iterator erase(iterator first, iterator last) {
            std::copy(last, finish, first);
            iterator new_finish = finish - (last - first);
            destroy(new_finish, finish);
            finish = new_finish;
            return first;
        }
        
        // 添加insert实现
        void insert(iterator position, size_type n, const T& x) {
            if (n == 0) return;
            
            if (size_type(end_of_storage - finish) >= n) {
                // 备用空间足够
                T x_copy = x;
                const size_type elems_after = finish - position;
                iterator old_finish = finish;
                
                if (elems_after > n) {
                    // 后面元素数量大于新增元素数量

                    // 使用 uninitialized_copy 拷贝还没有初始化的元素
                    finish = uninitialized_copy(finish - n, finish, finish);
                    // copy_backward 从后往前拷贝, 默认处理已经初始化好的元素
                    std::copy_backward(position, old_finish - n, old_finish);
                    std::fill(position, position + n, x_copy);
                } else {
                    // 后面元素数量小于等于新增元素数量
                    finish = uninitialized_fill_n(finish, n - elems_after, x_copy);
                    finish = uninitialized_copy(position, old_finish, finish);
                    std::fill(position, old_finish, x_copy);
                }
            } else {
                // 备用空间不足，需要重新分配
                const size_type old_size = size();
                const size_type len = old_size + std::max(old_size, n);
                
                iterator new_start = data_allocator::allocate(len);
                iterator new_finish = new_start;
                
                try {
                    new_finish = uninitialized_copy(start, position, new_start);
                    new_finish = uninitialized_fill_n(new_finish, n, x);
                    new_finish = uninitialized_copy(position, finish, new_finish);
                } catch (...) {
                    destroy(new_start, new_finish);
                    data_allocator::deallocate(new_start, len);
                    throw;
                }
                
                destroy(start, finish);
                deallocate();
                
                start = new_start;
                finish = new_finish;
                end_of_storage = new_start + len;
            }
        }

        void resize(size_type new_size, const T& x) {
            if (new_size < size()) {
                erase(begin() + new_size, end());
            } else {
                insert(end(), new_size - size(), x);
            }
        }

        void resize(size_type new_size) { resize(new_size, T()); }

        void clear() {
            erase(begin(), end());
        }

    protected:
        iterator allocate_and_fill(size_type n, const T& x) {
            iterator result = data_allocator::allocate(n);
            uninitialized_fill_n(result, n, x);
            return result;
        }
    };
    
    // 使用单一模板函数处理左值和右值
    template <typename T, typename Alloc>
    template <typename U>
    void vector<T, Alloc>::insert_aux(iterator position, U&& x) {
        if (finish != end_of_storage) {
            // 在备用空间起始处构造一个元素，并以vector最后一个值作为其初值
            construct(finish, *(finish-1));
            // 调整水位
            ++finish;
            std::copy_backward(position, finish - 2, finish -1);
            *position = std::forward<U>(x);
        } else { //无备用空间
            const size_type old_size = size();
            const size_type len = old_size != 0 ? 2 * old_size : 1;
            // 如果原大小不为0，扩容为两倍
            // 前半段用来放置原始数据，后半段放置新数据

            iterator new_start = data_allocator::allocate(len); // 实际配置
            iterator new_finish = new_start;

            try {
                // 将原vector的内容拷贝到新vector
                new_finish = uninitialized_copy(start, position, new_start);
                // 为新元素设定初值x
                construct(new_finish, std::forward<U>(x));
                // 调整水位
                ++new_finish;
                // 将原vector的其余部分拷贝过来
                new_finish = uninitialized_copy(position, finish, new_finish);
            } catch (...) {
                // commit or rollback semantic
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }

            destroy(begin(), end());
            deallocate();

            //调整迭代器
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + len;
        }
    }
}

#endif