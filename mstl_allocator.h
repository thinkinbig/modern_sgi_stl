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


template <class T, std::size_t growSize = 1024, std::size_t hotZoneSize = 1024>
class MemoryPool
{
    struct Block
    {
        Block *next;
    };

    struct HotZone {
        T* start = nullptr;
        size_t used = 0;
        size_t capacity = hotZoneSize / sizeof(T);

        HotZone() {
            // 使用 aligned_alloc 确保内存对齐
            start = reinterpret_cast<T*>(aligned_alloc(alignof(T), hotZoneSize));
        }

        ~HotZone() {
            free(start);  // 使用 free 释放 aligned_alloc 分配的内存
        }
    };

    class Buffer
    {
        static const std::size_t blockSize = sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block);
        uint8_t data[blockSize * growSize];

        public:

            Buffer *const next;

            Buffer(Buffer *next) :
                next(next)
            {
            }

            T *getBlock(std::size_t index)
            {
                return reinterpret_cast<T *>(&data[blockSize * index]);
            }
    };

    Block *firstFreeBlock = nullptr;
    Buffer *firstBuffer = nullptr;
    HotZone hotZone;
    std::size_t bufferedBlocks = growSize;

    public:

        MemoryPool() = default;
        MemoryPool(MemoryPool &&memoryPool) = delete;
        MemoryPool(const MemoryPool &memoryPool) = delete;
        MemoryPool operator =(MemoryPool &&memoryPool) = delete;
        MemoryPool operator =(const MemoryPool &memoryPool) = delete;

        ~MemoryPool()
        {
            while (firstBuffer) {
                Buffer *buffer = firstBuffer;
                firstBuffer = buffer->next;
                delete buffer;
            }
        }

        T* allocateFromBuffer()
        {
            if (bufferedBlocks >= growSize) {
                firstBuffer = new Buffer(firstBuffer);
                bufferedBlocks = 0;
            }

            return firstBuffer->getBlock(bufferedBlocks++);
        }

        T *allocate()
        {
            if (hotZone.used < hotZone.capacity) {
                return hotZone.start + hotZone.used++;
            }

            if (firstFreeBlock) {
                Block *block = firstFreeBlock;
                firstFreeBlock = block->next;
                return reinterpret_cast<T *>(block);
            }

            if (bufferedBlocks >= growSize) {
                firstBuffer = new Buffer(firstBuffer);
                bufferedBlocks = 0;
            }

            return firstBuffer->getBlock(bufferedBlocks++);
        }

        void deallocate(T *pointer)
        {
            if (pointer >= hotZone.start && 
                pointer < hotZone.start + hotZone.capacity) {
                if (pointer == hotZone.start + hotZone.used - 1) {
                    hotZone.used--;
                }
                return;
            }

            Block *block = reinterpret_cast<Block *>(pointer);
            block->next = firstFreeBlock;
            firstFreeBlock = block;
        }
};

template <class T, std::size_t growSize = 1024>
class Allocator : private MemoryPool<T, growSize>
{
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
    Allocator *copyAllocator = nullptr;
    std::allocator<T> *rebindAllocator = nullptr;
#endif

    public:

        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef T &reference;
        typedef const T &const_reference;
        typedef T value_type;

        template <class U>
        struct rebind
        {
            typedef Allocator<U, growSize> other;
        };

#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
        Allocator() = default;

        Allocator(Allocator &allocator) :
            copyAllocator(&allocator)
        {
        }

        template <class U>
        Allocator(const Allocator<U, growSize> &other)
        {
            if (!std::is_same<T, U>::value)
                rebindAllocator = new std::allocator<T>();
        }

        ~Allocator()
        {
            delete rebindAllocator;
        }
#endif

        pointer allocate(size_type n, const void *hint = 0)
        {
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
            if (copyAllocator)
                return copyAllocator->allocate(n, hint);

            if (rebindAllocator)
                return rebindAllocator->allocate(n, hint);
#endif

            if (n != 1 || hint)
                throw std::bad_alloc();

            return MemoryPool<T, growSize>::allocate();
        }

        void deallocate(pointer p, size_type n)
        {
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
            if (copyAllocator) {
                copyAllocator->deallocate(p, n);
                return;
            }

            if (rebindAllocator) {
                rebindAllocator->deallocate(p, n);
                return;
            }
#endif

            MemoryPool<T, growSize>::deallocate(p);
        }

        void construct(pointer p, const_reference val)
        {
            new (p) T(val);
        }

        void destroy(pointer p)
        {
            p->~T();
        }
};



}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_ALLOCATOR_H