#ifndef __MSGI_STL_INTERNAL_VECTOR_H
#define __MSGI_STL_INTERNAL_VECTOR_H

#include <algorithm>
#include <type_traits>
#include <utility>
#include <initializer_list>
#include "mstl_alloc.h"
#include "mstl_construct.h"
#include "mstl_uninitialized.h"

namespace mstl {

template <typename T, typename Alloc = alloc>
class Vector {
public:
    using value_type = T;
    using pointer = value_type*;  // 指针式random access iterator
    using const_pointer = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

protected:
    using data_allocator = SimpleAlloc<value_type, Alloc>;

    iterator kStart;
    iterator kFinish;
    iterator kEndOfStorage;

    // 使用模板和完美转发来同时处理左值和右值
    template <typename U>
    void insertAux(iterator position, U&& x);

    void deallocate() {
        if (kStart)
            data_allocator::deallocate(kStart, kEndOfStorage - kStart);
    }

    void fillInitialize(size_type n, const T& value) {
        kStart = allocateAndFill(n, value);
        kFinish = kStart + n;
        kEndOfStorage = kFinish;
    }

public:
    // 迭代器相关
    iterator begin() {
        return kStart;
    }
    const_iterator begin() const {
        return kStart;
    }
    iterator end() {
        return kFinish;
    }
    const_iterator end() const {
        return kFinish;
    }

    // 容量相关
    size_type size() const {
        return size_type(kFinish - kStart);
    }
    size_type capacity() const {
        return size_type(kEndOfStorage - kStart);
    }
    bool empty() const {
        return kStart == kFinish;
    }

    // 元素访问
    reference operator[](size_type n) {
        return *(begin() + n);
    }

    // 构造函数
    Vector() : kStart(nullptr), kFinish(nullptr), kEndOfStorage(nullptr) {}
    Vector(size_type n, const T& value) {
        fillInitialize(n, value);
    }
    Vector(int n, const T& value) {
        fillInitialize(n, value);
    }
    Vector(long n, const T& value) {
        fillInitialize(n, value);
    }
    explicit Vector(size_type n) {
        fillInitialize(n, T());
    }

    // 初始化列表构造函数
    Vector(std::initializer_list<T> il) {
        kStart = allocateAndCopy(il.begin(), il.end());
        kFinish = kStart + il.size();
        kEndOfStorage = kFinish;
    }

    template <typename InputIterator>
    Vector(InputIterator first, InputIterator last) {
        kStart = allocateAndCopy(first, last);
        kFinish = kStart + (last - first);
        kEndOfStorage = kFinish;
    }

    ~Vector() {
        destroy(kStart, kFinish);
        deallocate();
    }

    reference front() {
        return *begin();
    }
    const_reference front() const {
        return *begin();
    }

    reference back() {
        return *(end() - 1);
    }
    const_reference back() const {
        return *(end() - 1);
    }

    // 修改容器 - 使用统一的push_back模板
    void push_back(const T& x) {
        if (kFinish != kEndOfStorage) {
            construct(kFinish, x);
            ++kFinish;
        } else {
            insertAux(end(), x);
        }
    }

    // 添加noexcept移动语义版本
    void push_back(T&& x) noexcept(std::is_nothrow_move_constructible<T>::value) {
        if (kFinish != kEndOfStorage) {
            construct(kFinish, std::move(x));
            ++kFinish;
        } else {
            insertAux(end(), std::move(x));
        }
    }

    void pop_back() {
        --kFinish;
        destroy(kFinish);
    }

    iterator erase(iterator position) {
        if (position + 1 != end()) {
            std::copy(position + 1, kFinish, position);  // 后续元素往前移动
        }
        --kFinish;
        destroy(kFinish);
        return position;
    }

    // 添加区间删除版本
    iterator erase(iterator first, iterator last) {
        iterator i = std::copy(last, kFinish, first);
        destroy(i, kFinish);
        kFinish -= (last - first);
        return first;
    }

    // 添加insert实现
    void insert(iterator position, size_type n, const T& x) {
        if (n == 0)
            return;

        if (size_type(kEndOfStorage - kFinish) >= n) {
            // 备用空间足够
            T xCopy = x;
            const size_type elemsAfter = kFinish - position;
            iterator oldFinish = kFinish;

            if (elemsAfter > n) {
                // 后面元素数量大于新增元素数量

                // 使用 uninitialized_copy 拷贝还没有初始化的元素
                kFinish = uninitialized_copy(kFinish - n, kFinish, kFinish);
                // copy_backward 从后往前拷贝, 默认处理已经初始化好的元素
                std::copy_backward(position, oldFinish - n, oldFinish);
                std::fill(position, position + n, xCopy);
            } else {
                // 后面元素数量小于等于新增元素数量
                kFinish = uninitialized_fill_n(kFinish, n - elemsAfter, xCopy);
                kFinish = uninitialized_copy(position, oldFinish, kFinish);
                std::fill(position, oldFinish, xCopy);
            }
        } else {
            // 备用空间不足，需要重新分配
            const size_type oldSize = size();
            const size_type len = oldSize + std::max(oldSize, n);

            iterator newStart = data_allocator::allocate(len);
            iterator newFinish = newStart;

            try {
                newFinish = uninitialized_copy(kStart, position, newStart);
                newFinish = uninitialized_fill_n(newFinish, n, x);
                newFinish = uninitialized_copy(position, kFinish, newFinish);
            } catch (...) {
                destroy(newStart, newFinish);
                data_allocator::deallocate(newStart, len);
                throw;
            }

            destroy(kStart, kFinish);
            deallocate();

            kStart = newStart;
            kFinish = newFinish;
            kEndOfStorage = newStart + len;
        }
    }

    void resize(size_type newSize, const T& x) {
        if (newSize < size()) {
            erase(begin() + newSize, end());
        } else {
            insert(end(), newSize - size(), x);
        }
    }

    void resize(size_type newSize) {
        resize(newSize, T());
    }

    void clear() {
        erase(begin(), end());
    }

protected:
    iterator allocateAndFill(size_type n, const T& x) {
        iterator result = data_allocator::allocate(n);
        uninitialized_fill_n(result, n, x);
        return result;
    }

    iterator allocateAndCopy(const_iterator first, const_iterator last) {
        iterator result = data_allocator::allocate(last - first);
        uninitialized_copy(first, last, result);
        return result;
    }
};

// 使用单一模板函数处理左值和右值
template <typename T, typename Alloc>
template <typename U>
void Vector<T, Alloc>::insertAux(iterator position, U&& x) {
    if (kFinish != kEndOfStorage) {
        // 在备用空间起始处构造一个元素，并以vector最后一个值作为其初值
        construct(kFinish, *(kFinish - 1));
        // 调整水位
        ++kFinish;
        std::copy_backward(position, kFinish - 2, kFinish - 1);
        *position = std::forward<U>(x);
    } else {  // 无备用空间
        const size_type oldSize = size();
        const size_type len = oldSize != 0 ? 2 * oldSize : 1;
        // 如果原大小不为0，扩容为两倍
        // 前半段用来放置原始数据，后半段放置新数据

        iterator newStart = data_allocator::allocate(len);  // 实际配置
        iterator newFinish = newStart;

        try {
            // 将原vector的内容拷贝到新vector
            newFinish = uninitialized_copy(kStart, position, newStart);
            // 为新元素设定初值x
            construct(newFinish, std::forward<U>(x));
            // 调整水位
            ++newFinish;
            // 将原vector的其余部分拷贝过来
            newFinish = uninitialized_copy(position, kFinish, newFinish);
        } catch (...) {
            // commit or rollback semantic
            destroy(newStart, newFinish);
            data_allocator::deallocate(newStart, len);
            throw;
        }

        destroy(begin(), end());
        deallocate();

        // 调整迭代器
        kStart = newStart;
        kFinish = newFinish;
        kEndOfStorage = newStart + len;
    }
}
}  // namespace mstl

#endif