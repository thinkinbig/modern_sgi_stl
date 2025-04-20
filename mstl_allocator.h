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
    using AllocatorType = Alloc;
    using ValueType = typename Alloc::ValueType;
    using Pointer = typename Alloc::Pointer;
    using ConstPointer = typename Alloc::ConstPointer;
    using VoidPointer =
        std::conditional_t<std::is_same_v<Pointer, ValueType*>, void*,
                           typename std::pointer_traits<Pointer>::template rebind<void>>;
    using ConstVoidPointer = std::conditional_t<
        std::is_same_v<ConstPointer, const ValueType*>, const void*,
        typename std::pointer_traits<ConstPointer>::template rebind<const void>>;
    using DifferenceType = typename Alloc::DifferenceType;
    using SizeType = typename Alloc::SizeType;
    using PropagateOnContainerCopyAssignment = std::false_type;
    using PropagateOnContainerMoveAssignment = std::false_type;
    using PropagateOnContainerSwap = std::false_type;

    template <typename T>
    using RebindAlloc = typename Alloc::template RebindAlloc<T>::Other;

    static Pointer allocate(Alloc& a, SizeType n) {
        return a.allocate(n);
    }

    static Pointer allocate(Alloc& a, SizeType n, ConstVoidPointer hint) {
        return a.allocate(n, hint);
    }

    static void deallocate(Alloc& a, Pointer p, SizeType n) {
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

    static SizeType max_size(const Alloc& a) noexcept {
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
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Pointer = Tp*;
    using ConstPointer = const Tp*;
    using Reference = Tp&;
    using ConstReference = const Tp&;
    using ValueType = Tp;

    template <class Tp1>
    struct rebind {
        using other = Allocator<Tp1, Alloc>;  // 保持相同的分配器类型
    };

    Allocator() noexcept {}
    Allocator(const Allocator&) noexcept {}
    template <class Tp1>
    Allocator(const Allocator<Tp1, Alloc>&) noexcept {}
    ~Allocator() noexcept {}

    Pointer address(Reference x) const {
        return &x;
    }
    ConstPointer address(ConstReference x) const {
        return &x;
    }

    // n 可以为0
    Pointer allocate(SizeType n, const void* = nullptr) {
        return n != 0 ? static_cast<Pointer>(_Alloc::allocate(n * sizeof(Tp))) : nullptr;
    }

    // p 不能为nullptr
    void deallocate(Pointer p, SizeType n) {
        _Alloc::deallocate(p, n * sizeof(Tp));
    }

    SizeType max_size() const noexcept {
        return size_t(-1) / sizeof(Tp);
    }

    void construct(Pointer p, const Tp& val) {
        new (p) Tp(val);
    }
    void destroy(Pointer p) {
        p->~Tp();
    }
};

// allocator_traits 特化版本
template <typename T>
struct AllocatorTraits<Allocator<T>> {
    using AllocatorType = Allocator<T>;
    using ValueType = T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using VoidPointer = void*;
    using ConstVoidPointer = const void*;
    using DifferenceType = std::ptrdiff_t;
    using SizeType = std::size_t;
    using PropagateOnContainerCopyAssignment = std::false_type;
    using PropagateOnContainerMoveAssignment = std::false_type;
    using PropagateOnContainerSwap = std::false_type;

    template <typename U>
    using RebindAlloc = Allocator<U>;

    static Pointer allocate(AllocatorType& a, SizeType n) {
        return a.allocate(n);
    }

    static Pointer allocate(AllocatorType& a, SizeType n, ConstVoidPointer) {
        return a.allocate(n);
    }

    static void deallocate(AllocatorType& a, Pointer p, SizeType n) {
        a.deallocate(p, n);
    }

    template <typename U, typename... Args>
    static void construct(AllocatorType&, U* p, Args&&... args) {
        mstl::construct(p, std::forward<Args>(args)...);
    }

    template <typename U>
    static void destroy(AllocatorType&, U* p) {
        mstl::destroy(p);
    }

    static SizeType max_size(const AllocatorType&) noexcept {
        return size_t(-1) / sizeof(T);
    }

    static AllocatorType select_on_container_copy_construction(const AllocatorType& rhs) {
        return rhs;
    }
};

// void特化版本
template <typename Alloc>
class Allocator<void, Alloc> {
public:
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Pointer = void*;
    using ConstPointer = const void*;
    using ValueType = void;

    template <typename Tp1>
    struct RebindAlloc {
        using Other = Allocator<Tp1, Alloc>;
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