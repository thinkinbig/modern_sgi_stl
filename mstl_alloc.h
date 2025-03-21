#ifndef __MSGI_STL_INTERNAL_ALLOC_H
#define __MSGI_STL_INTERNAL_ALLOC_H


#include <new>
#include <cstdlib>
#include <cstring>
#include <array>
#include <mutex>

namespace mstl
{

    // malloc-based allocator 通常比 default alloc 速度慢

    template <int inst>
    class __malloc_alloc_template
    {
    public:
        using MallocHandler = void (*)();
        using MallocFunction = void *(*)(size_t);
        using ReallocFunction = void *(*)(void *, size_t);

    private:
        static MallocFunction oom_malloc;
        static ReallocFunction oom_realloc;
        static MallocHandler malloc_alloc_oom_handler;

        static void *default_oom_malloc(size_t n)
        {
            while (true)
            {
                if (!malloc_alloc_oom_handler)
                    throw std::bad_alloc();

                malloc_alloc_oom_handler();
                if (void *result = std::malloc(n))
                    return result;
            }
        }

        static void *default_oom_realloc(void *p, size_t n)
        {
            while (true)
            {
                if (!malloc_alloc_oom_handler)
                    throw std::bad_alloc();

                malloc_alloc_oom_handler();
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
                void *rescue = oom_malloc(n);
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

        static void *reallocate(void *p, [[maybe_unused]] size_t old_sz, size_t new_sz)
        {
            try
            {
                void *result = std::realloc(p, new_sz);
                if (result == nullptr)
                    throw std::bad_alloc();
                return result;
            }
            catch (std::bad_alloc &)
            {
                void *rescue = oom_realloc(p, new_sz);
                if (rescue == nullptr)
                    throw;
                return rescue;
            }
        }

        static MallocHandler set_malloc_handler(MallocHandler f) noexcept
        {
            auto old = malloc_alloc_oom_handler;
            malloc_alloc_oom_handler = f;
            return old;
        }
    };

    // malloc_alloc out-of-memory handling
    template <int inst>
    typename __malloc_alloc_template<inst>::MallocHandler
        __malloc_alloc_template<inst>::malloc_alloc_oom_handler = nullptr;

    template <int inst>
    typename __malloc_alloc_template<inst>::MallocFunction
        __malloc_alloc_template<inst>::oom_malloc = &__malloc_alloc_template<inst>::default_oom_malloc;

    template <int inst>
    typename __malloc_alloc_template<inst>::ReallocFunction
        __malloc_alloc_template<inst>::oom_realloc = &__malloc_alloc_template<inst>::default_oom_realloc;

    constexpr size_t __ALIGN = 8;
    constexpr size_t __MAX_BYTES = 128;
    constexpr size_t __NFREELISTS = __MAX_BYTES / __ALIGN;

    // 定义分配器类型
    using malloc_alloc = __malloc_alloc_template<0>;

    template <bool threads, int inst>
    class __default_alloc_template
    {

    private:
        static size_t ROUND_UP(size_t bytes)
        {
            return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
        }

        struct obj
        {
            union
            {
                obj *free_list_link;
                char client_data[1]; // The client sees this;
            };
        };

        static std::array<obj *volatile, __NFREELISTS> free_list;

        static size_t FREELIST_INDEX(size_t bytes)
        {
            return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
        }

        static std::mutex mutex;

        // 返回一个大小为n的对象， 并可能加入大小为n的其他区块到free list
        static void *refill(size_t n);

        // 配置一大块空间，可容纳nobjs个大小为"size"的区块
        // 如果配置nobjs个区块有所不便, nobjs可能会降低
        static char *chunk_alloc(size_t size, int &nobjs);

        // Chunk allocation state
        static char *start_free; // 内存池开始位置。只在chunk_alloc()中变化
        static char *end_free;   // 内存池结束位置。 只在chunk_alloc()中变化
        static size_t heap_size;

    public:
        static void *allocate(size_t n)
        {
            obj *volatile *my_free_list;
            obj *result;
            if (n > __MAX_BYTES)
                return malloc_alloc::allocate(n);

            my_free_list = std::begin(free_list) + FREELIST_INDEX(n);

            if constexpr (threads)
            {
                std::lock_guard<std::mutex> lock(mutex);
            }

            result = *my_free_list;
            if (result == nullptr)
            {
                void *r = refill(ROUND_UP(n));
                return r;
            }
            *my_free_list = result->free_list_link;

            return result;
        }

        static void deallocate(void *p, size_t n)
        {
            obj *q = static_cast<obj *>(p);
            obj *volatile *my_free_list;

            if (n > __MAX_BYTES)
            {
                malloc_alloc::deallocate(p, n);
                return;
            }

            if constexpr (threads)
            {
                std::lock_guard<std::mutex> lock(mutex);
            }

            my_free_list = std::begin(free_list) + FREELIST_INDEX(n);
            q->free_list_link = *my_free_list;
            *my_free_list = q;
        }

        static void *reallocate(void *p, size_t old_sz, size_t new_sz)
        {
            void *result;
            size_t copy_sz;

            if (old_sz > __MAX_BYTES && new_sz > __MAX_BYTES)
            {
                return malloc_alloc::reallocate(p, old_sz, new_sz);
            }

            if (ROUND_UP(old_sz) == ROUND_UP(new_sz))
                return p;

            result = allocate(new_sz);
            copy_sz = new_sz > old_sz ? old_sz : new_sz;
            std::memcpy(result, p, copy_sz);
            deallocate(p, old_sz);
            return result;
        }
    };

    // 静态成员定义
    template <bool threads, int inst>
    std::mutex __default_alloc_template<threads, inst>::mutex;

    // 定义分配器类型
    using default_alloc = __default_alloc_template<false, 0>;    // 单线程版本
    using thread_safe_alloc = __default_alloc_template<true, 0>; // 多线程版本

    // 简单的分配器封装
    template <class Tp, class Alloc>
    class simple_alloc
    {
    public:
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
    char *__default_alloc_template<threads, inst>::start_free = nullptr;

    template <bool threads, int inst>
    char *__default_alloc_template<threads, inst>::end_free = nullptr;

    template <bool threads, int inst>
    size_t __default_alloc_template<threads, inst>::heap_size = 0;

    template <bool threads, int inst>
    std::array<typename __default_alloc_template<threads, inst>::obj *volatile, __NFREELISTS>
        __default_alloc_template<threads, inst>::free_list = {0}; // 定义

    template <bool threads, int inst>
    void *__default_alloc_template<threads, inst>::refill(size_t n)
    {
        int nobjs = 20;
        char *chunk = chunk_alloc(n, nobjs);
        if (chunk == nullptr)
        {
            return nullptr;
        }

        obj *volatile *my_free_list;
        obj *result;
        obj *current_obj;
        obj *next_obj;
        int i;

        if (nobjs == 1)
            return chunk;

        my_free_list = std::begin(free_list) + FREELIST_INDEX(n);
        result = static_cast<obj *>(static_cast<void *>(chunk));
        *my_free_list = next_obj = static_cast<obj *>(static_cast<void *>(chunk + n));

        for (i = 1;; ++i)
        {
            current_obj = next_obj;
            next_obj = static_cast<obj *>(static_cast<void *>(reinterpret_cast<char *>(current_obj) + n));
            if (nobjs - 1 == i)
            {
                current_obj->free_list_link = nullptr;
                break;
            }
            else
            {
                current_obj->free_list_link = next_obj;
            }
        }
        return result;
    }

    template <bool threads, int inst>
    char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size, int &nobjs)
    {
        char *result;
        size_t total_bytes = size * nobjs;
        size_t bytes_left = end_free - start_free;

        if (bytes_left >= total_bytes)
        {
            result = start_free;
            start_free += total_bytes;
            return result;
        }
        else if (bytes_left >= size)
        {
            nobjs = bytes_left / size;
            total_bytes = size * nobjs;
            result = start_free;
            start_free += total_bytes;
            return result;
        }
        else
        {
            size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);

            if (bytes_left > 0)
            {
                obj *volatile *my_free_list = std::begin(free_list) + FREELIST_INDEX(bytes_left);
                obj *head = reinterpret_cast<obj *>(start_free);
                head->free_list_link = *my_free_list;
                *my_free_list = head;
            }

            start_free = static_cast<char *>(malloc(bytes_to_get));
            if (start_free == nullptr)
            {
                size_t i;
                obj *volatile *my_free_list, *p;

                for (i = size; i <= __MAX_BYTES; i += __ALIGN)
                {
                    my_free_list = std::begin(free_list) + FREELIST_INDEX(i);
                    p = *my_free_list;
                    if (nullptr != p)
                    {
                        *my_free_list = p->free_list_link;
                        start_free = reinterpret_cast<char *>(p);
                        end_free = start_free + i;
                        return chunk_alloc(size, nobjs);
                    }
                }
                end_free = nullptr;
                start_free = static_cast<char *>(malloc_alloc::allocate(bytes_to_get));
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            return chunk_alloc(size, nobjs);
        }
        return nullptr;
    }

} // namespace mstl

#endif // __MSGI_STL_INTERNAL_ALLOC_H
