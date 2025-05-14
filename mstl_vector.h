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
    using ValueType = T;
    using Pointer = ValueType*;  // 指针式random access iterator
    using ConstPointer = const ValueType*;
    using Iterator = ValueType*;
    using ConstIterator = const ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

protected:
    using DataAllocator = SimpleAlloc<ValueType, Alloc>;

    Iterator kStart;
    Iterator kFinish;
    Iterator kEndOfStorage;

    // 使用模板和完美转发来同时处理左值和右值
    template <typename U>
    void insertAux(Iterator position, U&& x);

    void deallocate() {
        if (kStart)
            DataAllocator::deallocate(kStart, kEndOfStorage - kStart);
    }

    void fillInitialize(SizeType n, const T& value) {
        kStart = allocateAndFill(n, value);
        kFinish = kStart + n;
        kEndOfStorage = kFinish;
    }

public:
    // 迭代器相关
    Iterator begin() {
        return kStart;
    }
    ConstIterator begin() const {
        return kStart;
    }
    Iterator end() {
        return kFinish;
    }
    ConstIterator end() const {
        return kFinish;
    }

    // 容量相关
    SizeType size() const {
        return SizeType(kFinish - kStart);
    }
    SizeType capacity() const {
        return SizeType(kEndOfStorage - kStart);
    }
    bool empty() const {
        return kStart == kFinish;
    }

    // 元素访问
    Reference operator[](SizeType n) {
        return *(begin() + n);
    }

    // 构造函数
    Vector() : kStart(nullptr), kFinish(nullptr), kEndOfStorage(nullptr) {}
    Vector(SizeType n, const T& value) {
        fillInitialize(n, value);
    }
    Vector(int n, const T& value) {
        fillInitialize(n, value);
    }
    Vector(long n, const T& value) {
        fillInitialize(n, value);
    }
    explicit Vector(SizeType n) {
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

    Reference front() {
        return *begin();
    }
    ConstReference front() const {
        return *begin();
    }

    Reference back() {
        return *(end() - 1);
    }
    ConstReference back() const {
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

    Iterator erase(Iterator position) {
        if (position + 1 != end()) {
            std::copy(position + 1, kFinish, position);  // 后续元素往前移动
        }
        --kFinish;
        destroy(kFinish);
        return position;
    }

    // 添加区间删除版本
    Iterator erase(Iterator first, Iterator last) {
        Iterator i = std::copy(last, kFinish, first);
        destroy(i, kFinish);
        kFinish -= (last - first);
        return first;
    }

    // 添加insert实现
    void insert(Iterator position, SizeType n, const T& x) {
        if (n == 0)
            return;

        if (SizeType(kEndOfStorage - kFinish) >= n) {
            // 备用空间足够
            T xCopy = x;
            const SizeType elemsAfter = kFinish - position;
            Iterator oldFinish = kFinish;

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
            const SizeType oldSize = size();
            const SizeType len = oldSize + std::max(oldSize, n);

            Iterator newStart = DataAllocator::allocate(len);
            Iterator newFinish = newStart;

            try {
                newFinish = uninitialized_copy(kStart, position, newStart);
                newFinish = uninitialized_fill_n(newFinish, n, x);
                newFinish = uninitialized_copy(position, kFinish, newFinish);
            } catch (...) {
                destroy(newStart, newFinish);
                DataAllocator::deallocate(newStart, len);
                throw;
            }

            destroy(kStart, kFinish);
            deallocate();

            kStart = newStart;
            kFinish = newFinish;
            kEndOfStorage = newStart + len;
        }
    }

    void resize(SizeType newSize, const T& x) {
        if (newSize < size()) {
            erase(begin() + newSize, end());
        } else {
            insert(end(), newSize - size(), x);
        }
    }

    void resize(SizeType newSize) {
        resize(newSize, T());
    }

    void clear() {
        erase(begin(), end());
    }

    void reserve(SizeType n) {
        if (capacity() < n) {
            const SizeType oldSize = size();
            Iterator newStart = DataAllocator::allocate(n);
            try {
                uninitialized_copy(kStart, kFinish, newStart);
                destroy(kStart, kFinish);
                deallocate();
                kStart = newStart;
                kFinish = newStart + oldSize;
                kEndOfStorage = newStart + n;
            } catch (...) {
                DataAllocator::deallocate(newStart, n);
                throw;
            }
        }
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (kFinish != kEndOfStorage) {
            construct(kFinish, std::forward<Args>(args)...);
            ++kFinish;
        } else {
            const SizeType oldSize = size();
            const SizeType len = oldSize != 0 ? 2 * oldSize : 1;
            Iterator newStart = DataAllocator::allocate(len);
            Iterator newFinish = newStart;
            try {
                newFinish = uninitialized_copy(kStart, kFinish, newStart);
                construct(newFinish, std::forward<Args>(args)...);
                ++newFinish;
            } catch (...) {
                destroy(newStart, newFinish);
                DataAllocator::deallocate(newStart, len);
                throw;
            }
            destroy(kStart, kFinish);
            deallocate();
            kStart = newStart;
            kFinish = newFinish;
            kEndOfStorage = newStart + len;
        }
    }

protected:
    Iterator allocateAndFill(SizeType n, const T& x) {
        Iterator result = DataAllocator::allocate(n);
        uninitialized_fill_n(result, n, x);
        return result;
    }

    Iterator allocateAndCopy(ConstIterator first, ConstIterator last) {
        Iterator result = DataAllocator::allocate(last - first);
        uninitialized_copy(first, last, result);
        return result;
    }
};

// 使用单一模板函数处理左值和右值
template <typename T, typename Alloc>
template <typename U>
void Vector<T, Alloc>::insertAux(Iterator position, U&& x) {
    if (kFinish != kEndOfStorage) {
        // 在备用空间起始处构造一个元素，并以vector最后一个值作为其初值
        construct(kFinish, std::move(*(kFinish - 1)));
        // 调整水位
        ++kFinish;
        // 使用 std::move_backward 替代 std::copy_backward
        std::move_backward(position, kFinish - 2, kFinish - 1);
        *position = std::forward<U>(x);
    } else {  // 无备用空间
        const SizeType oldSize = size();
        const SizeType len = oldSize != 0 ? 2 * oldSize : 1;
        // 如果原大小不为0，扩容为两倍
        // 前半段用来放置原始数据，后半段放置新数据

        Iterator newStart = DataAllocator::allocate(len);  // 实际配置
        Iterator newFinish = newStart;

        try {
            // 将原vector的内容移动到新vector
            newFinish = mstl::uninitialized_move(kStart, position, newStart);
            // 为新元素设定初值x
            construct(newFinish, std::forward<U>(x));
            // 调整水位
            ++newFinish;
            // 将原vector的其余部分移动到新vector
            newFinish = mstl::uninitialized_move(position, kFinish, newFinish);
        } catch (...) {
            // commit or rollback semantic
            destroy(newStart, newFinish);
            DataAllocator::deallocate(newStart, len);
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

template <typename T, size_t N, typename Alloc = std::allocator<T>>
class SmallVector {
   alignas(T) unsigned char stack_[sizeof(T) * N];
   T* data_;
   size_t size_;
   size_t capacity_;
   Alloc allocator_;
   bool on_stack() const { return data_ == reinterpret_cast<const T*>(stack_); }
   void move_to_heap(size_t new_cap) {
      T* new_data = std::allocator_traits<Alloc>::allocate(allocator_, new_cap);
      for (size_t i = 0; i < size_; ++i)
         std::allocator_traits<Alloc>::construct(allocator_, new_data + i, std::move(data_[i]));
      for (size_t i = 0; i < size_; ++i)
         std::allocator_traits<Alloc>::destroy(allocator_, data_ + i);
      if (!on_stack()) std::allocator_traits<Alloc>::deallocate(allocator_, data_, capacity_);
      data_ = new_data;
      capacity_ = new_cap;
   }

   public:
   using value_type = T;
   using size_type = size_t;
   using difference_type = std::ptrdiff_t;
   using reference = T&;
   using const_reference = const T&;
   using pointer = T*;
   using const_pointer = const T*;
   using iterator = T*;
   using const_iterator = const T*;
   using allocator_type = Alloc;

   SmallVector() : data_(reinterpret_cast<T*>(stack_)), size_(0), capacity_(N), allocator_() {}
   explicit SmallVector(const Alloc& alloc) : data_(reinterpret_cast<T*>(stack_)), size_(0), capacity_(N), allocator_(alloc) {}
   ~SmallVector() {
      for (size_t i = 0; i < size_; ++i)
         std::allocator_traits<Alloc>::destroy(allocator_, data_ + i);
      if (!on_stack()) std::allocator_traits<Alloc>::deallocate(allocator_, data_, capacity_);
   }
   SmallVector(const SmallVector&) = delete;
   SmallVector& operator=(const SmallVector&) = delete;

   SmallVector(SmallVector&& other) noexcept
      : data_(reinterpret_cast<T*>(stack_)), size_(0), capacity_(N), allocator_(std::move(other.allocator_)) {
      if (other.on_stack()) {
         reserve(other.size_);
         for (size_t i = 0; i < other.size_; ++i) {
            std::allocator_traits<Alloc>::construct(allocator_, data_ + i, std::move(other.data_[i]));
            ++size_;
         }
      } else {
         data_ = other.data_;
         size_ = other.size_;
         capacity_ = other.capacity_;
         other.data_ = reinterpret_cast<T*>(other.stack_);
         other.size_ = 0;
         other.capacity_ = N;
      }
   }

   SmallVector& operator=(SmallVector&& other) noexcept {
      if (this != &other) {
         for (size_t i = 0; i < size_; ++i)
            std::allocator_traits<Alloc>::destroy(allocator_, data_ + i);
         if (!on_stack()) std::allocator_traits<Alloc>::deallocate(allocator_, data_, capacity_);

         allocator_ = std::move(other.allocator_);
         if (other.on_stack()) {
            data_ = reinterpret_cast<T*>(stack_);
            capacity_ = N;
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i) {
               std::allocator_traits<Alloc>::construct(allocator_, data_ + i, std::move(other.data_[i]));
            }
            size_ = other.size_;
         } else {
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = reinterpret_cast<T*>(other.stack_);
            other.size_ = 0;
            other.capacity_ = N;
         }
      }
      return *this;
   }

   size_type size() const { return size_; }
   size_type capacity() const { return capacity_; }
   bool empty() const { return size_ == 0; }
   void reserve(size_type n) {
      if (n > capacity_) move_to_heap(std::max(n, 2 * capacity_));
   }

   void pop_back() {
      if (size_ > 0) {
         std::allocator_traits<Alloc>::destroy(allocator_, data_ + size_ - 1);
         --size_;
      }
   }

   void resize(size_type n) {
      reserve(n);
      if (n > size_)
         for (size_t i = size_; i < n; ++i) std::allocator_traits<Alloc>::construct(allocator_, data_ + i);
      else
         for (size_t i = n; i < size_; ++i) std::allocator_traits<Alloc>::destroy(allocator_, data_ + i);
      size_ = n;
   }
   void push_back(const T& v) {
      if (size_ == capacity_) reserve(size_ + 1);
      std::allocator_traits<Alloc>::construct(allocator_, data_ + size_, v);
      ++size_;
   }
   void push_back(T&& v) {
      if (size_ == capacity_) reserve(size_ + 1);
      std::allocator_traits<Alloc>::construct(allocator_, data_ + size_, std::move(v));
      ++size_;
   }
   reference operator[](size_type i) { return data_[i]; }
   const_reference operator[](size_type i) const { return data_[i]; }
   pointer data() { return data_; }
   const_pointer data() const { return data_; }
   void clear() {
      for (size_t i = 0; i < size_; ++i)
         std::allocator_traits<Alloc>::destroy(allocator_, data_ + i);
      size_ = 0;
   }

   // 迭代器接口
   iterator begin() { return data_; }
   iterator end() { return data_ + size_; }
   const_iterator begin() const { return data_; }
   const_iterator end() const { return data_ + size_; }
   const_iterator cbegin() const { return data_; }
   const_iterator cend() const { return data_ + size_; }

   allocator_type get_allocator() const { return allocator_; }
};
}  // namespace mstl

#endif