#ifndef __MSGI_STL_INTERNAL_ALLOC_H
#define __MSGI_STL_INTERNAL_ALLOC_H


#include <new>
#include <cstdlib>
#include <cstring>
#include <array>
#include <mutex>
#include "mstl_concepts.h"
#include "mstl_construct.h"

namespace mstl
{

    // malloc-based allocator 通常比 default alloc 速度慢

    template <int inst>
    class MallocAllocTemplate
    {
    public:
        using MallocHandler = void (*)();
        using MallocFunction = void *(*)(size_t);
        using ReallocFunction = void *(*)(void *, size_t);

        template <typename T>
        struct rebind
        {
            using other = MallocAllocTemplate<inst>;
        };

    private:
        static MallocFunction kOomMalloc;
        static ReallocFunction kOomRealloc;
        static MallocHandler kMallocAllocOomHandler;

        static void *defaultOomMalloc(size_t n)
        {
            while (true)
            {
                if (!kMallocAllocOomHandler)
                    throw std::bad_alloc();

                kMallocAllocOomHandler();
                if (void *result = std::malloc(n))
                    return result;
            }
        }

        static void *defaultOomRealloc(void *p, size_t n)
        {
            while (true)
            {
                if (!kMallocAllocOomHandler)
                    throw std::bad_alloc();

                kMallocAllocOomHandler();
                if (void *result = std::realloc(p, n))
                    return result;
            }
        }

    public:
        static void *allocate(size_t n)
        {
            try
            {
                void *result = std::malloc(n);
                if (result == nullptr)
                    throw std::bad_alloc();
                return result;
            }
            catch (std::bad_alloc &)
            {
                void *rescue = kOomMalloc(n);
                if (rescue == nullptr)
                {
                    throw;
                }
                return rescue;
            }
        }

        static void deallocate(void *p, [[maybe_unused]] size_t n)
        {
            std::free(p);
        }

        static void *reallocate(void *p, [[maybe_unused]] size_t oldSize, size_t newSize)
        {
            try
            {
                void *result = std::realloc(p, newSize);
                if (result == nullptr)
                    throw std::bad_alloc();
                return result;
            }
            catch (std::bad_alloc &)
            {
                void *rescue = kOomRealloc(p, newSize);
                if (rescue == nullptr)
                    throw;
                return rescue;
            }
        }

        static MallocHandler setMallocHandler(MallocHandler f) noexcept
        {
            auto old = kMallocAllocOomHandler;
            kMallocAllocOomHandler = f;
            return old;
        }
    };

    // malloc_alloc out-of-memory handling
    template <int inst>
    typename MallocAllocTemplate<inst>::MallocHandler
        MallocAllocTemplate<inst>::kMallocAllocOomHandler = nullptr;

    template <int inst>
    typename MallocAllocTemplate<inst>::MallocFunction
        MallocAllocTemplate<inst>::kOomMalloc = &MallocAllocTemplate<inst>::defaultOomMalloc;

    template <int inst>
    typename MallocAllocTemplate<inst>::ReallocFunction
        MallocAllocTemplate<inst>::kOomRealloc = &MallocAllocTemplate<inst>::defaultOomRealloc;

    constexpr size_t kAlignment = 8;
    constexpr size_t kMaxBytes = 128;
    constexpr size_t kNumFreeLists = kMaxBytes / kAlignment;

    // 定义分配器类型
    using malloc_alloc = MallocAllocTemplate<0>;

    using alloc = malloc_alloc;

    template <bool threads, int inst>
    class DefaultAllocTemplate
    {
    public:

        template <typename T>
        struct rebind
        {
            using other = DefaultAllocTemplate<threads, inst>;
        };

    private:
        static size_t roundUp(size_t bytes)
        {
            return (((bytes) + kAlignment - 1) & ~(kAlignment - 1));
        }

        struct Obj
        {
            union
            {
                Obj *freeListLink;
                char clientData[1]; // The client sees this;
            };
        };

        static std::array<Obj *volatile, kNumFreeLists> freeList;

        static size_t freeListIndex(size_t bytes)
        {
            return (((bytes) + kAlignment - 1) / kAlignment - 1);
        }

        static std::mutex kMutex;

        // 返回一个大小为n的对象， 并可能加入大小为n的其他区块到free list
        static void *refill(size_t n);

        // 配置一大块空间，可容纳nobjs个大小为"size"的区块
        // 如果配置nobjs个区块有所不便, nobjs可能会降低
        static char *chunkAlloc(size_t size, int &nobjs);

        // Chunk allocation state
        static char *startFree; // 内存池开始位置。只在chunk_alloc()中变化
        static char *endFree;   // 内存池结束位置。 只在chunk_alloc()中变化
        static size_t heapSize;

    public:
        static void *allocate(size_t n)
        {
            Obj *volatile *myFreeList;
            Obj *result;
            if (n > kMaxBytes)
                return malloc_alloc::allocate(n);

            myFreeList = std::begin(freeList) + freeListIndex(n);

            if constexpr (threads)
            {
                std::lock_guard<std::mutex> lock(kMutex);
            }

            result = *myFreeList;
            if (result == nullptr)
            {
                void *r = refill(roundUp(n));
                return r;
            }
            *myFreeList = result->freeListLink;

            return result;
        }

        static void deallocate(void *p, size_t n)
        {
            Obj *q = static_cast<Obj *>(p);
            Obj *volatile *myFreeList;

            if (n > kMaxBytes)
            {
                malloc_alloc::deallocate(p, n);
                return;
            }

            if constexpr (threads)
            {
                std::lock_guard<std::mutex> lock(kMutex);
            }

            myFreeList = std::begin(freeList) + freeListIndex(n);
            q->freeListLink = *myFreeList;
            *myFreeList = q;
        }

        static void *reallocate(void *p, size_t oldSize, size_t newSize)
        {
            void *result;
            size_t copySize;

            if (oldSize > kMaxBytes && newSize > kMaxBytes)
            {
                return malloc_alloc::reallocate(p, oldSize, newSize);
            }

            if (roundUp(oldSize) == roundUp(newSize))
                return p;

            result = allocate(newSize);
            copySize = newSize > oldSize ? oldSize : newSize;
            std::memcpy(result, p, copySize);
            deallocate(p, oldSize);
            return result;
        }
    };

    // 静态成员定义
    template <bool threads, int inst>
    std::mutex DefaultAllocTemplate<threads, inst>::kMutex;

    // 定义分配器类型
    using default_alloc = DefaultAllocTemplate<false, 0>;    // 单线程版本
    using thread_safe_alloc = DefaultAllocTemplate<true, 0>; // 多线程版本

    // 简单的分配器封装
    template <class Tp, class Alloc>
    class SimpleAlloc
    {
    public:
        using value_type = Tp;
        using pointer = Tp*;
        using const_pointer = const Tp*;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        static Tp *allocate(size_t n)
        {
            return 0 == n ? 0 : static_cast<Tp *>(Alloc::allocate(n * sizeof(Tp)));
        }
        static Tp *allocate(void)
        {
            return static_cast<Tp *>(Alloc::allocate(sizeof(Tp)));
        }
        static void deallocate(Tp *p, size_t n)
        {
            if (0 != n)
                Alloc::deallocate(p, n * sizeof(Tp));
        }
        static void deallocate(Tp *p)
        {
            Alloc::deallocate(p, sizeof(Tp));
        }

        template <class Tp1>
        struct rebind
        {
            using other = SimpleAlloc<Tp1, Alloc>;
        };
    };

    // 静态断言，确保simple_alloc满足SimpleAllocator合约
    template <class Tp, class Alloc>
    inline constexpr bool checkSimpleAlloc = SimpleAllocator<SimpleAlloc<Tp, Alloc>, Tp>;
    static_assert(checkSimpleAlloc<int, default_alloc>, "simple_alloc must satisfy SimpleAllocator concept");

    // allocator_traits 实现
    template <typename Alloc>
    struct allocator_traits
    {
        using allocator_type = Alloc;
        using value_type = typename Alloc::value_type;
        using pointer = typename Alloc::pointer;
        using const_pointer = typename Alloc::const_pointer;
        using void_pointer = std::conditional_t<std::is_same_v<pointer, value_type*>,
                                              void*,
                                              typename std::pointer_traits<pointer>::rebind<void>>;
        using const_void_pointer = std::conditional_t<std::is_same_v<const_pointer, const value_type*>,
                                                    const void*,
                                                    typename std::pointer_traits<const_pointer>::rebind<const void>>;
        using difference_type = typename Alloc::difference_type;
        using size_type = typename Alloc::size_type;
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_swap = std::false_type;

        template <typename T>
        using rebind_alloc = typename Alloc::template rebind<T>::other;

        static pointer allocate(Alloc& a, size_type n)
        {
            return a.allocate(n);
        }

        static pointer allocate(Alloc& a, size_type n, const_void_pointer hint)
        {
            return a.allocate(n, hint);
        }

        static void deallocate(Alloc& a, pointer p, size_type n)
        {
            a.deallocate(p, n);
        }

        template <typename T, typename... Args>
        static void construct(Alloc&, T* p, Args&&... args)
        {
            mstl::construct(p, std::forward<Args>(args)...);
        }

        template <typename T>
        static void destroy(Alloc&, T* p)
        {
            mstl::destroy(p);
        }

        static size_type max_size(const Alloc& a) noexcept
        {
            return a.max_size();
        }

        static Alloc select_on_container_copy_construction(const Alloc& rhs)
        {
            return rhs;
        }
    };

    // 标准分配器接口
    template <typename Tp, typename Alloc = default_alloc> // 默认使用单线程版本
    class allocator
    {
        using _Alloc = Alloc; // 使用用户指定的分配器
    public:
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = Tp *;
        using const_pointer = const Tp *;
        using reference = Tp &;
        using const_reference = const Tp &;
        using value_type = Tp;

        template <class Tp1>
        struct rebind
        {
            using other = allocator<Tp1, Alloc>; // 保持相同的分配器类型
        };

        allocator() noexcept {}
        allocator(const allocator &) noexcept {}
        template <class Tp1>
        allocator(const allocator<Tp1, Alloc> &) noexcept {}
        ~allocator() noexcept {}

        pointer address(reference x) const { return &x; }
        const_pointer address(const_reference x) const { return &x; }

        // n 可以为0
        pointer allocate(size_type n, const void * = nullptr)
        {
            return n != 0 ? static_cast<pointer>(_Alloc::allocate(n * sizeof(Tp))) : nullptr;
        }

        // p 不能为nullptr
        void deallocate(pointer p, size_type n)
        {
            _Alloc::deallocate(p, n * sizeof(Tp));
        }

        size_type max_size() const noexcept
        {
            return size_t(-1) / sizeof(Tp);
        }

        void construct(pointer p, const Tp &val)
        {
            new (p) Tp(val);
        }
        void destroy(pointer p)
        {
            p->~Tp();
        }
    };

    // allocator_traits 特化版本
    template <typename T>
    struct allocator_traits<allocator<T>>
    {
        using allocator_type = allocator<T>;
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
        using rebind_alloc = allocator<U>;

        static pointer allocate(allocator_type& a, size_type n)
        {
            return a.allocate(n);
        }

        static pointer allocate(allocator_type& a, size_type n, const_void_pointer)
        {
            return a.allocate(n);
        }

        static void deallocate(allocator_type& a, pointer p, size_type n)
        {
            a.deallocate(p, n);
        }

        template <typename U, typename... Args>
        static void construct(allocator_type&, U* p, Args&&... args)
        {
            mstl::construct(p, std::forward<Args>(args)...);
        }

        template <typename U>
        static void destroy(allocator_type&, U* p)
        {
            mstl::destroy(p);
        }

        static size_type max_size(const allocator_type&) noexcept
        {
            return size_t(-1) / sizeof(T);
        }

        static allocator_type select_on_container_copy_construction(const allocator_type& rhs)
        {
            return rhs;
        }
    };

    // 静态断言，确保allocator满足StandardAllocator合约
    template <typename Tp, typename Alloc>
    inline constexpr bool checkStandardAllocator = StandardAllocator<allocator<Tp, Alloc>>;
    static_assert(checkStandardAllocator<int, default_alloc>, "allocator must satisfy StandardAllocator concept");

    // void特化版本
    template <typename Alloc>
    class allocator<void, Alloc>
    {
    public:
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = void *;
        using const_pointer = const void *;
        using value_type = void;

        template <typename Tp1>
        struct rebind
        {
            using other = allocator<Tp1, Alloc>;
        };
    };

    // 分配器比较操作符
    template <typename T1, typename T2, typename Alloc>
    inline bool operator==(const allocator<T1, Alloc> &, const allocator<T2, Alloc> &)
    {
        return true;
    }

    template <typename T1, typename T2, typename Alloc>
    inline bool operator!=(const allocator<T1, Alloc> &, const allocator<T2, Alloc> &)
    {
        return false;
    }

    template <bool threads, int inst>
    char *DefaultAllocTemplate<threads, inst>::startFree = nullptr;

    template <bool threads, int inst>
    char *DefaultAllocTemplate<threads, inst>::endFree = nullptr;

    template <bool threads, int inst>
    size_t DefaultAllocTemplate<threads, inst>::heapSize = 0;

    template <bool threads, int inst>
    std::array<typename DefaultAllocTemplate<threads, inst>::Obj *volatile, kNumFreeLists>
        DefaultAllocTemplate<threads, inst>::freeList = {0}; // 定义

    template <bool threads, int inst>
    void *DefaultAllocTemplate<threads, inst>::refill(size_t n)
    {
        int nobjs = 20;
        char *chunk = chunkAlloc(n, nobjs);
        if (chunk == nullptr)
        {
            return nullptr;
        }

        Obj *volatile *myFreeList;
        Obj *result;
        Obj *currentObj;
        Obj *nextObj;
        int i;

        if (nobjs == 1)
            return chunk;

        myFreeList = std::begin(freeList) + freeListIndex(n);
        result = static_cast<Obj *>(static_cast<void *>(chunk));
        *myFreeList = nextObj = static_cast<Obj *>(static_cast<void *>(chunk + n));

        for (i = 1;; ++i)
        {
            currentObj = nextObj;
            nextObj = static_cast<Obj *>(static_cast<void *>(reinterpret_cast<char *>(currentObj) + n));
            if (nobjs - 1 == i)
            {
                currentObj->freeListLink = nullptr;
                break;
            }
            else
            {
                currentObj->freeListLink = nextObj;
            }
        }
        return result;
    }

    template <bool threads, int inst>
    char *DefaultAllocTemplate<threads, inst>::chunkAlloc(size_t size, int &nobjs)
    {
        char *result;
        size_t totalBytes = size * nobjs;
        size_t bytesLeft = endFree - startFree;

        if (bytesLeft >= totalBytes)
        {
            result = startFree;
            startFree += totalBytes;
            return result;
        }
        else if (bytesLeft >= size)
        {
            nobjs = bytesLeft / size;
            totalBytes = size * nobjs;
            result = startFree;
            startFree += totalBytes;
            return result;
        }
        else
        {
            size_t bytesToGet = 2 * totalBytes + roundUp(heapSize >> 4);

            if (bytesLeft > 0)
            {
                Obj *volatile *myFreeList = std::begin(freeList) + freeListIndex(bytesLeft);
                Obj *head = reinterpret_cast<Obj *>(startFree);
                head->freeListLink = *myFreeList;
                *myFreeList = head;
            }

            startFree = static_cast<char *>(std::malloc(bytesToGet));
            if (startFree == nullptr)
            {
                size_t i;
                Obj *volatile *myFreeList, *p;

                for (i = size; i <= kMaxBytes; i += kAlignment)
                {
                    myFreeList = std::begin(freeList) + freeListIndex(i);
                    p = *myFreeList;
                    if (nullptr != p)
                    {
                        *myFreeList = p->freeListLink;
                        startFree = reinterpret_cast<char *>(p);
                        endFree = startFree + i;
                        return chunkAlloc(size, nobjs);
                    }
                }
                endFree = nullptr;
                startFree = static_cast<char *>(malloc_alloc::allocate(bytesToGet));
            }
            heapSize += bytesToGet;
            endFree = startFree + bytesToGet;
            return chunkAlloc(size, nobjs);
        }
        return nullptr;
    }

} // namespace mstl

#endif // __MSGI_STL_INTERNAL_ALLOC_H
