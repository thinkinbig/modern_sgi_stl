#ifndef __MSGI_STL_INTERNAL_ALLOCATOR_H
#define __MSGI_STL_INTERNAL_ALLOCATOR_H

#include <cstddef>
#include <new>
#include <type_traits>
#include "mstl_alloc.h"
#include "mstl_construct.h"

namespace mstl {

// allocator_traits 实现
template <typename Alloc>
struct AllocatorTraits {
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;
    using pointer = typename Alloc::pointer;
    using const_pointer = typename Alloc::const_pointer;
    using void_pointer =
        std::conditional_t<std::is_same_v<pointer, value_type*>, void*,
                           typename std::pointer_traits<pointer>::template rebind<void>>;
    using const_void_pointer = std::conditional_t<
        std::is_same_v<const_pointer, const value_type*>, const void*,
        typename std::pointer_traits<const_pointer>::template rebind<const void>>;
    using difference_type = typename Alloc::difference_type;
    using size_type = typename Alloc::size_type;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;

    template <typename T>
    using rebind_alloc = typename Alloc::template rebind<T>::other;

    static pointer allocate(Alloc& a, size_type n) {
        return a.allocate(n);
    }

    static pointer allocate(Alloc& a, size_type n, const_void_pointer hint) {
        return a.allocate(n, hint);
    }

    static void deallocate(Alloc& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    template <typename T, typename... Args>
    static void construct(Alloc&, T* p, Args&&... args) {
        mstl::construct(p, std::forward<Args>(args)...);
    }

    template <typename T>
    static void destroy(Alloc&, T* p) {
        mstl::destroy(p);
    }

    static size_type max_size(const Alloc& a) noexcept {
        return a.max_size();
    }

    static Alloc select_on_container_copy_construction(const Alloc& rhs) {
        return rhs;
    }
};

// 标准分配器接口
template <typename Tp, typename Alloc = alloc>  // 默认使用单线程版本
class Allocator {
    using _Alloc = Alloc;  // 使用用户指定的分配器
public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = Tp*;
    using const_pointer = const Tp*;
    using reference = Tp&;
    using const_reference = const Tp&;
    using value_type = Tp;

    template <class Tp1>
    struct rebind {
        using other = Allocator<Tp1, Alloc>;  // 保持相同的分配器类型
    };

    Allocator() noexcept {}
    Allocator(const Allocator&) noexcept {}
    template <class Tp1>
    Allocator(const Allocator<Tp1, Alloc>&) noexcept {}
    ~Allocator() noexcept {}

    pointer address(reference x) const {
        return &x;
    }
    const_pointer address(const_reference x) const {
        return &x;
    }

    // n 可以为0
    pointer allocate(size_type n, const void* = nullptr) {
        return n != 0 ? static_cast<pointer>(_Alloc::allocate(n * sizeof(Tp))) : nullptr;
    }

    // p 不能为nullptr
    void deallocate(pointer p, size_type n) {
        _Alloc::deallocate(p, n * sizeof(Tp));
    }

    size_type max_size() const noexcept {
        return size_t(-1) / sizeof(Tp);
    }

    void construct(pointer p, const Tp& val) {
        new (p) Tp(val);
    }
    void destroy(pointer p) {
        p->~Tp();
    }
};

// allocator_traits 特化版本
template <typename T>
struct AllocatorTraits<Allocator<T>> {
    using allocator_type = Allocator<T>;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;

    template <typename U>
    using rebind_alloc = Allocator<U>;

    static pointer allocate(allocator_type& a, size_type n) {
        return a.allocate(n);
    }

    static pointer allocate(allocator_type& a, size_type n, const_void_pointer) {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    template <typename U, typename... Args>
    static void construct(allocator_type&, U* p, Args&&... args) {
        mstl::construct(p, std::forward<Args>(args)...);
    }

    template <typename U>
    static void destroy(allocator_type&, U* p) {
        mstl::destroy(p);
    }

    static size_type max_size(const allocator_type&) noexcept {
        return size_t(-1) / sizeof(T);
    }

    static allocator_type select_on_container_copy_construction(const allocator_type& rhs) {
        return rhs;
    }
};

// void特化版本
template <typename Alloc>
class Allocator<void, Alloc> {
public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = void*;
    using const_pointer = const void*;
    using value_type = void;

    template <typename Tp1>
    struct rebind {
        using other = Allocator<Tp1, Alloc>;
    };
};

// 分配器比较操作符
template <typename T1, typename T2, typename Alloc>
inline bool operator==(const Allocator<T1, Alloc>&, const Allocator<T2, Alloc>&) {
    return true;
}

template <typename T1, typename T2, typename Alloc>
inline bool operator!=(const Allocator<T1, Alloc>&, const Allocator<T2, Alloc>&) {
    return false;
}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_ALLOCATOR_H