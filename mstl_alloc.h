#ifndef __MSGI_STL_INTERNAL_ALLOC_H
#define __MSGI_STL_INTERNAL_ALLOC_H

#include <array>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <new>
#include "mstl_concepts.h"
#include "mstl_construct.h"

namespace mstl {

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// 这个文件包含 STL 的最底层内存分配实现
// 为了性能考虑，我们直接使用 malloc/free
// 内存安全性由上层容器保证

template <int inst>
class MallocAllocTemplate {
public:
    using MallocHandler = void (*)();
    using MallocFunction = void* (*)(size_t);
    using ReallocFunction = void* (*)(void*, size_t);
    using Pointer = void*;
    using ConstPointer = const void*;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

    template <typename T>
    struct rebind {
        using other = MallocAllocTemplate<inst>;
    };

private:
    static MallocFunction kOomMalloc;
    static ReallocFunction kOomRealloc;
    static MallocHandler kMallocAllocOomHandler;

    static void* defaultOomMalloc(size_t n) {
        while (true) {
            if (!kMallocAllocOomHandler)
                throw std::bad_alloc();

            kMallocAllocOomHandler();
            if (void* result = std::malloc(n))
                return result;
        }
    }

    static void* defaultOomRealloc(void* p, size_t n) {
        while (true) {
            if (!kMallocAllocOomHandler)
                throw std::bad_alloc();

            kMallocAllocOomHandler();
            if (void* result = std::realloc(p, n))
                return result;
        }
    }

public:
    static void* allocate(size_t n) {
        try {
            void* result = std::malloc(n);
            if (result == nullptr)
                throw std::bad_alloc();
            return result;
        } catch (std::bad_alloc&) {
            void* rescue = kOomMalloc(n);
            if (rescue == nullptr) {
                throw;
            }
            return rescue;
        }
    }

    static void deallocate(void* p, [[maybe_unused]] size_t n) {
        std::free(p);
    }

    static void* reallocate(void* p, [[maybe_unused]] size_t oldSize, size_t newSize) {
        try {
            void* result = std::realloc(p, newSize);
            if (result == nullptr)
                throw std::bad_alloc();
            return result;
        } catch (std::bad_alloc&) {
            void* rescue = kOomRealloc(p, newSize);
            if (rescue == nullptr)
                throw;
            return rescue;
        }
    }

    static MallocHandler setMallocHandler(MallocHandler f) noexcept {
        auto old = kMallocAllocOomHandler;
        kMallocAllocOomHandler = f;
        return old;
    }
};

// NOLINTEND(cppcoreguidelines-owning-memory)

// malloc-based allocator 通常比 default alloc 速度慢

template <int inst>
typename MallocAllocTemplate<inst>::MallocHandler
    MallocAllocTemplate<inst>::kMallocAllocOomHandler = nullptr;

template <int inst>
typename MallocAllocTemplate<inst>::MallocFunction MallocAllocTemplate<inst>::kOomMalloc =
    &MallocAllocTemplate<inst>::defaultOomMalloc;

template <int inst>
typename MallocAllocTemplate<inst>::ReallocFunction MallocAllocTemplate<inst>::kOomRealloc =
    &MallocAllocTemplate<inst>::defaultOomRealloc;

constexpr size_t kAlignment = 8;
constexpr size_t kMaxBytes = 128;
constexpr size_t kNumFreeLists = kMaxBytes / kAlignment;

// 定义分配器类型
using malloc_alloc = MallocAllocTemplate<0>;

using alloc = malloc_alloc;

template <bool threads, int inst>
class DefaultAllocTemplate {
public:
    template <typename T>
    struct rebind {
        using other = DefaultAllocTemplate<threads, inst>;
    };

private:
    static size_t roundUp(size_t bytes) {
        return (((bytes) + kAlignment - 1) & ~(kAlignment - 1));
    }

    struct Obj {
        union {
            Obj* freeListLink;
            char clientData[1];  // The client sees this;
        };
    };

    static typename DefaultAllocTemplate<threads, inst>::Obj* volatile freeList[kNumFreeLists];

    static size_t freeListIndex(size_t bytes) {
        return (((bytes) + kAlignment - 1) / kAlignment - 1);
    }

    static std::mutex kMutex;

    // 返回一个大小为n的对象， 并可能加入大小为n的其他区块到free list
    static void* refill(size_t n);

    // 配置一大块空间，可容纳nobjs个大小为"size"的区块
    // 如果配置nobjs个区块有所不便, nobjs可能会降低
    static char* chunkAlloc(size_t size, int& nobjs);

    // Chunk allocation state
    static char* startFree;  // 内存池开始位置。只在chunk_alloc()中变化
    static char* endFree;    // 内存池结束位置。 只在chunk_alloc()中变化
    static size_t heapSize;

public:
    static void* allocate(size_t n) {
        Obj* volatile* myFreeList;
        Obj* result;
        if (n > kMaxBytes)
            return malloc_alloc::allocate(n);

        myFreeList = freeList + freeListIndex(n);

        if constexpr (threads) {
            std::lock_guard<std::mutex> lock(kMutex);
        }

        result = *myFreeList;
        if (result == nullptr) {
            void* r = refill(roundUp(n));
            return r;
        }
        *myFreeList = result->freeListLink;

        return result;
    }

    static void deallocate(void* p, size_t n) {
        Obj* q = static_cast<Obj*>(p);
        Obj* volatile* myFreeList;

        if (n > kMaxBytes) {
            malloc_alloc::deallocate(p, n);
            return;
        }

        if constexpr (threads) {
            std::lock_guard<std::mutex> lock(kMutex);
        }

        myFreeList = reinterpret_cast<Obj* volatile*>(&freeList[freeListIndex(n)]);
        q->freeListLink = *myFreeList;
        *myFreeList = q;
    }

    static void* reallocate(void* p, size_t oldSize, size_t newSize) {
        void* result;
        size_t copySize;

        if (oldSize > kMaxBytes && newSize > kMaxBytes) {
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
::std::mutex DefaultAllocTemplate<threads, inst>::kMutex;

// 定义分配器类型
using default_alloc = DefaultAllocTemplate<false, 0>;     // 单线程版本
using thread_safe_alloc = DefaultAllocTemplate<true, 0>;  // 多线程版本

// 简单的分配器封装
template <typename Tp, typename Alloc>
class SimpleAlloc {
public:
    using ValueType = Tp;
    using Pointer = Tp*;
    using ConstPointer = const Tp*;
    using Reference = Tp&;
    using ConstReference = const Tp&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

    template <typename U>
    struct RebindAlloc {
        using Other = SimpleAlloc<U, Alloc>;
    };

    static Tp* allocate(size_t n = 1) {
        Alloc a;
        return 0 == n ? 0 : reinterpret_cast<Tp*>(a.allocate(n * sizeof(Tp)));
    }

    static void deallocate(Tp* p, size_t n = 1) {
        if (p != 0) {
            Alloc a;
            a.deallocate(reinterpret_cast<typename Alloc::Pointer>(p), n * sizeof(Tp));
        }
    }
};

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::startFree = nullptr;

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::endFree = nullptr;

template <bool threads, int inst>
size_t DefaultAllocTemplate<threads, inst>::heapSize = 0;

template <bool threads, int inst>
typename DefaultAllocTemplate<threads, inst>::Obj* volatile DefaultAllocTemplate<
    threads, inst>::freeList[kNumFreeLists] = {0};  // 定义

template <bool threads, int inst>
void* DefaultAllocTemplate<threads, inst>::refill(size_t n) {
    int nobjs = 20;
    char* chunk = chunkAlloc(n, nobjs);
    if (chunk == nullptr) {
        return nullptr;
    }

    Obj* volatile* myFreeList;
    Obj* result;
    Obj* currentObj;
    Obj* nextObj;
    int i;

    if (nobjs == 1)
        return chunk;

    myFreeList = std::begin(freeList) + freeListIndex(n);
    result = static_cast<Obj*>(static_cast<void*>(chunk));
    *myFreeList = nextObj = static_cast<Obj*>(static_cast<void*>(chunk + n));

    for (i = 1;; ++i) {
        currentObj = nextObj;
        nextObj = static_cast<Obj*>(static_cast<void*>(reinterpret_cast<char*>(currentObj) + n));
        if (nobjs - 1 == i) {
            currentObj->freeListLink = nullptr;
            break;
        } else {
            currentObj->freeListLink = nextObj;
        }
    }
    return result;
}

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::chunkAlloc(size_t size, int& nobjs) {
    char* result;
    size_t totalBytes = size * nobjs;
    size_t bytesLeft = endFree - startFree;

    if (bytesLeft >= totalBytes) {
        result = startFree;
        startFree += totalBytes;
        return result;
    } else if (bytesLeft >= size) {
        nobjs = bytesLeft / size;
        totalBytes = size * nobjs;
        result = startFree;
        startFree += totalBytes;
        return result;
    } else {
        size_t bytesToGet = 2 * totalBytes + roundUp(heapSize >> 4);

        if (bytesLeft > 0) {
            Obj* volatile* myFreeList = std::begin(freeList) + freeListIndex(bytesLeft);
            Obj* head = reinterpret_cast<Obj*>(startFree);
            head->freeListLink = *myFreeList;
            *myFreeList = head;
        }

        startFree = static_cast<char*>(std::malloc(bytesToGet));
        if (startFree == nullptr) {
            size_t i;
            Obj *volatile *myFreeList, *p;

            for (i = size; i <= kMaxBytes; i += kAlignment) {
                myFreeList = std::begin(freeList) + freeListIndex(i);
                p = *myFreeList;
                if (nullptr != p) {
                    *myFreeList = p->freeListLink;
                    startFree = reinterpret_cast<char*>(p);
                    endFree = startFree + i;
                    return chunkAlloc(size, nobjs);
                }
            }
            endFree = nullptr;
            startFree = static_cast<char*>(malloc_alloc::allocate(bytesToGet));
        }
        heapSize += bytesToGet;
        endFree = startFree + bytesToGet;
        return chunkAlloc(size, nobjs);
    }
    return nullptr;
}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_ALLOC_H
