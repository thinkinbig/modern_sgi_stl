#ifndef __MSGI_STL_PTHREAD_ALLOC_H
#define __MSGI_STL_PTHREAD_ALLOC_H

#include <cstddef>
#include <mutex>
#include <pthread.h>
#include <cstring>
#include <new>
#include <atomic>
#include <vector>
#include <memory>

namespace mstl
{
    // Constants
    constexpr size_t DEFAULT_ALIGNMENT = 8;
    constexpr size_t MAX_BYTES = 128;

    // Forward declarations
    template <size_t _Max_size>
    class PthreadAllocator;

    // Aligned memory block structure
    struct alignas(DEFAULT_ALIGNMENT) MemoryBlock
    {
        MemoryBlock *next;
    };

    // Per-thread state for memory management
    template <size_t _Max_size = MAX_BYTES>
    class PthreadAllocPerThreadState
    {
    public:
        // Number of free lists
        static constexpr size_t NUM_FREE_LISTS = _Max_size / DEFAULT_ALIGNMENT;

        // Array of free lists, one per size class
        std::atomic<MemoryBlock *> free_lists[NUM_FREE_LISTS]{};

        // Link for list of available per-thread structures
        PthreadAllocPerThreadState<_Max_size> *next = nullptr;

        // Constructor
        PthreadAllocPerThreadState() = default;

        // Allocate memory of size n
        void *allocate(size_t n)
        {
            const size_t index = (n - 1) / DEFAULT_ALIGNMENT;
            // Try to get a block from the free list
            MemoryBlock *block = free_lists[index].load(std::memory_order_relaxed);

            while (block)
            {
                MemoryBlock *next_block = block->next;

                // Try to update the free list atomically
                if (free_lists[index].compare_exchange_weak(
                        block, next_block,
                        std::memory_order_acquire,
                        std::memory_order_relaxed))
                {
                    return block;
                }

                // If CAS failed, block has new value from compare_exchange, try again
            }

            // If free list is empty, refill it
            return refill(n);
        }

        // Deallocate memory of size n
        void deallocate(void *p, size_t n)
        {
            if (!p)
                return;

            const size_t index = (n - 1) / DEFAULT_ALIGNMENT;
            MemoryBlock *block = static_cast<MemoryBlock *>(p);

            // Add block to free list with atomic operations
            MemoryBlock *old_head = free_lists[index].load(std::memory_order_relaxed);
            do
            {
                block->next = old_head;
            } while (!free_lists[index].compare_exchange_weak(
                old_head, block,
                std::memory_order_release,
                std::memory_order_relaxed));
        }

    private:
        // Refill free list for size n
        void *refill(size_t n);

        friend class PthreadAllocator<_Max_size>;
    };

    // Main allocator class
    template <size_t _Max_size = MAX_BYTES>
    class PthreadAllocator
    {
    private:
        using ThreadState = PthreadAllocPerThreadState<_Max_size>;

        // Global memory pool management
        static std::mutex chunk_mutex;
        static char *start_free;
        static char *end_free;
        static size_t heap_size;

        // Thread state management
        static ThreadState *free_thread_states;
        static pthread_key_t thread_key;
        static std::once_flag key_init_flag;

        // Initialize thread key
        static void initialize_key()
        {
            pthread_key_create(&thread_key, &thread_cleanup);
        }

        // Clean up thread state when thread exits
        static void thread_cleanup(void *p)
        {
            if (!p)
                return;

            ThreadState *state = static_cast<ThreadState *>(p);

            // Add thread state to free list for reuse
            std::lock_guard<std::mutex> lock(chunk_mutex);
            state->next = free_thread_states;
            free_thread_states = state;
        }

        // Get current thread's state, creating if necessary
        static ThreadState *get_thread_state()
        {

            // Ensure key is initialized
            std::call_once(key_init_flag, initialize_key);

            // Get thread state or create new one
            ThreadState *state = static_cast<ThreadState *>(pthread_getspecific(thread_key));

            if (!state)
            {
                // Try to reuse from free list first
                {
                    std::lock_guard<std::mutex> lock(chunk_mutex);
                    if (free_thread_states)
                    {
                        state = free_thread_states;
                        free_thread_states = state->next;
                        state->next = nullptr;
                    }
                }

                // If no free states, create a new one
                if (!state)
                {
                    state = new ThreadState();
                }

                // Associate with thread
                pthread_setspecific(thread_key, state);
            }

            return state;
        }

    public:
        // Allocate memory of size n
        static void *allocate(size_t n)
        {

            // If n is too large, use standard allocator
            if (n > _Max_size)
            {
                return ::operator new(n, std::nothrow);
            }

            // Round up to alignment multiple
            n = (n + DEFAULT_ALIGNMENT - 1) & ~(DEFAULT_ALIGNMENT - 1);

            // Delegate to thread-local state
            ThreadState *state = get_thread_state();

            return state->allocate(n);
        }

        // Deallocate memory of size n
        static void deallocate(void *p, size_t n)
        {
            if (!p)
                return;

            // If n is too large, use standard deallocator
            if (n > _Max_size)
            {
                ::operator delete(p);
                return;
            }

            // Round up to alignment multiple
            n = (n + DEFAULT_ALIGNMENT - 1) & ~(DEFAULT_ALIGNMENT - 1);

            // Delegate to thread-local state
            get_thread_state()->deallocate(p, n);
        }

        // Allocate chunk of memory from global pool
        static char *chunk_allocate(size_t size, size_t &adjustment)
        {

            std::lock_guard<std::mutex> lock(chunk_mutex);

            char *result = nullptr;

            // Try to fulfill from current memory pool
            size_t total_bytes = size + adjustment;

            if (static_cast<size_t>(end_free - start_free) >= total_bytes)
            {
                result = start_free;
                start_free += total_bytes;
                return result;
            }

            // If current pool is not enough but can provide at least one object
            if (static_cast<size_t>(end_free - start_free) >= size)
            {
                // Use what's left for this request
                result = start_free;
                start_free += size;
                adjustment = 0;
                return result;
            }

            // Need to allocate a new chunk
            // Try to add any remaining fragment to appropriate free list
            if (end_free - start_free > 0)
            {
                // Determine which free list to add this memory to
                size_t leftover_size = end_free - start_free;
                size_t index = (leftover_size - 1) / DEFAULT_ALIGNMENT;

                if (index < ThreadState::NUM_FREE_LISTS)
                {
                    // Get thread state
                    ThreadState *state = get_thread_state();

                    // Add fragment to free list
                    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(start_free);
                    block->next = state->free_lists[index].load(std::memory_order_relaxed);
                    state->free_lists[index].store(block, std::memory_order_release);
                }
            }

            // Calculate how much to allocate
            size_t bytes_to_get = 2 * total_bytes + (heap_size >> 4);

            // Try to allocate a new pool
            start_free = static_cast<char *>(::operator new(bytes_to_get, std::nothrow));

            if (!start_free)
            {
                // Out of memory, try to find a free list that has blocks
                // and use one of those
                for (size_t i = size / DEFAULT_ALIGNMENT; i < ThreadState::NUM_FREE_LISTS; ++i)
                {
                    ThreadState *state = get_thread_state();
                    MemoryBlock *block = state->free_lists[i].exchange(nullptr, std::memory_order_acquire);

                    if (block)
                    {
                        // Found a free block, split it if necessary
                        start_free = reinterpret_cast<char *>(block);
                        end_free = start_free + (i + 1) * DEFAULT_ALIGNMENT;
                        return chunk_allocate(size, adjustment); // Retry with the found memory
                    }
                }

                // Last resort - try allocating exactly what's needed
                start_free = static_cast<char *>(::operator new(size, std::nothrow));

                if (!start_free)
                {
                    throw std::bad_alloc(); // Genuinely out of memory
                }

                // Set end_free for the minimal allocation
                end_free = start_free + size;

                // Return directly instead of recursive call
                result = start_free;
                start_free += size;
                return result;
            }

            // Successfully allocated a new chunk
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;

            // Return from this allocation instead of recursive call
            result = start_free;
            start_free += total_bytes;
            return result;
        }
    };

    // Implement the refill method for thread state
    template <size_t _Max_size>
    void *PthreadAllocPerThreadState<_Max_size>::refill(size_t n)
    {

        // How many objects to allocate at once
        constexpr size_t NOBJS = 20;

        // Try to allocate memory for NOBJS objects of size n
        size_t total_bytes = n * NOBJS;
        size_t adjustment = 0; // Will be used if alignment adjustment is needed

        char *chunk = PthreadAllocator<_Max_size>::chunk_allocate(total_bytes, adjustment);

        if (!chunk)
        {
            return nullptr; // Out of memory
        }

        // Adjust for alignment if necessary
        char *aligned_chunk = chunk;
        if (adjustment > 0)
        {
            aligned_chunk = chunk + adjustment;
        }

        // Always return the first object to satisfy current request
        void *result = aligned_chunk;

        // If there's only one object, just return it
        if (NOBJS == 1)
        {
            return result;
        }

        // Add remaining objects to the free list
        size_t index = (n - 1) / DEFAULT_ALIGNMENT;
        MemoryBlock *current_block = nullptr;
        MemoryBlock *next_block = nullptr;

        // Start at second object (first is returned to caller)
        for (size_t i = 1; i < NOBJS; ++i)
        {
            current_block = reinterpret_cast<MemoryBlock *>(aligned_chunk + n * i);

            if (i < NOBJS - 1)
            {
                next_block = reinterpret_cast<MemoryBlock *>(aligned_chunk + n * (i + 1));
                current_block->next = next_block;
            }
            else
            {
                current_block->next = nullptr; // Last block in chain
            }
        }

        // Update free list, handling possible race conditions
        MemoryBlock *first_block = reinterpret_cast<MemoryBlock *>(aligned_chunk + n);
        MemoryBlock *old_head = free_lists[index].load(std::memory_order_relaxed);

        do
        {
            // Point last block to current free list head
            current_block->next = old_head;
        } while (!free_lists[index].compare_exchange_weak(
            old_head, first_block,
            std::memory_order_release,
            std::memory_order_relaxed));

        return result;
    }

    // Initialize static members
    template <size_t _Max_size>
    std::mutex PthreadAllocator<_Max_size>::chunk_mutex;

    template <size_t _Max_size>
    char *PthreadAllocator<_Max_size>::start_free = nullptr;

    template <size_t _Max_size>
    char *PthreadAllocator<_Max_size>::end_free = nullptr;

    template <size_t _Max_size>
    size_t PthreadAllocator<_Max_size>::heap_size = 0;

    template <size_t _Max_size>
    typename PthreadAllocator<_Max_size>::ThreadState *PthreadAllocator<_Max_size>::free_thread_states = nullptr;

    template <size_t _Max_size>
    pthread_key_t PthreadAllocator<_Max_size>::thread_key;

    template <size_t _Max_size>
    std::once_flag PthreadAllocator<_Max_size>::key_init_flag;

    // STL-compatible allocator adapter
    template <typename T, size_t _Max_size = MAX_BYTES>
    class StlPthreadAllocator
    {
    public:
        using value_type = T;
        using pointer = T *;
        using const_pointer = const T *;
        using reference = T &;
        using const_reference = const T &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <typename U>
        struct rebind
        {
            using other = StlPthreadAllocator<U, _Max_size>;
        };

        StlPthreadAllocator() = default;

        template <typename U>
        StlPthreadAllocator(const StlPthreadAllocator<U, _Max_size> &) noexcept {}

        pointer allocate(size_type n)
        {
            return static_cast<pointer>(PthreadAllocator<_Max_size>::allocate(n * sizeof(T)));
        }

        void deallocate(pointer p, size_type n)
        {
            PthreadAllocator<_Max_size>::deallocate(p, n * sizeof(T));
        }

        // Construct and destroy methods are default in modern C++

        // Equality operations
        bool operator==(const StlPthreadAllocator &) const noexcept { return true; }
        bool operator!=(const StlPthreadAllocator &) const noexcept { return false; }
    };
}

#endif