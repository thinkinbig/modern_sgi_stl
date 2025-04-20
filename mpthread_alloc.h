#ifndef __MSGI_STL_PTHREAD_ALLOC_H
#define __MSGI_STL_PTHREAD_ALLOC_H

#include <pthread.h>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>
#include <new>
#include <vector>
#include "mstl_alloc.h"
#include "mstl_concepts.h"

namespace mstl {
// Constants
constexpr size_t DEFAULT_ALIGNMENT = 8;
constexpr size_t MAX_BYTES = 128;

// Forward declarations
template <size_t _Max_size>
class PthreadAllocatorTemplate;

// Aligned memory block structure
struct alignas(DEFAULT_ALIGNMENT) MemoryBlock {
    MemoryBlock* next;
};

namespace detail {
inline size_t align_up(size_t n) {
    return (n + DEFAULT_ALIGNMENT - 1) & ~(DEFAULT_ALIGNMENT - 1);
}
inline size_t size_to_index(size_t size) {
    return (size - 1) / DEFAULT_ALIGNMENT;
}
}  // namespace detail

// Per-thread state for memory management
template <size_t _Max_size = MAX_BYTES>
class PthreadAllocPerThreadState {
public:
    // Number of free lists
    static constexpr size_t NUM_FREE_LISTS = _Max_size / DEFAULT_ALIGNMENT;

    // Array of free lists, one per size class
    std::atomic<MemoryBlock*> free_lists[NUM_FREE_LISTS]{};

    // Link for list of available per-thread structures
    PthreadAllocPerThreadState<_Max_size>* next = nullptr;

    // Constructor
    PthreadAllocPerThreadState() = default;

    // Allocate memory of size n
    void* allocate(size_t n) {
        const size_t index = detail::size_to_index(n);

        // Try to get a block from the free list
        MemoryBlock* block = free_lists[index].load(std::memory_order_relaxed);

        while (block) {
            MemoryBlock* next_block = block->next;

            // Try to update the free list atomically
            if (free_lists[index].compare_exchange_weak(
                    block, next_block, std::memory_order_acquire, std::memory_order_relaxed)) {
                return block;
            }

            // If CAS failed, block has new value from compare_exchange, try again
        }

        // If free list is empty, refill it
        return refill(n);
    }

    // Deallocate memory of size n
    void deallocate(void* p, size_t n) {
        if (!p)
            return;

        const size_t index = detail::size_to_index(n);
        MemoryBlock* block = static_cast<MemoryBlock*>(p);

        // Add block to free list with atomic operations
        MemoryBlock* old_head = free_lists[index].load(std::memory_order_relaxed);
        do {
            block->next = old_head;
        } while (!free_lists[index].compare_exchange_weak(
            old_head, block, std::memory_order_release, std::memory_order_relaxed));
    }

private:
    // Refill free list for size n
    void* refill(size_t n);

    friend class PthreadAllocatorTemplate<_Max_size>;
};

// Main allocator class
template <size_t _Max_size = MAX_BYTES>
class PthreadAllocatorTemplate {
public:
    using ValueType = void;
    using SizeType = std::size_t;
    using Pointer = void*;

private:
    using ThreadState = PthreadAllocPerThreadState<_Max_size>;

    // Global memory pool management
    static std::mutex chunk_mutex;
    static char* start_free;
    static char* end_free;
    static size_t heap_size;

    // Thread state management
    static ThreadState* free_thread_states;
    static pthread_key_t thread_key;
    static std::once_flag key_init_flag;

    // Initialize thread key
    static void initialize_key() {
        pthread_key_create(&thread_key, &thread_cleanup);
    }

    // Clean up thread state when thread exits
    static void thread_cleanup(void* p) {
        if (!p)
            return;

        ThreadState* state = static_cast<ThreadState*>(p);

        // Add thread state to free list for reuse
        std::lock_guard<std::mutex> lock(chunk_mutex);
        state->next = free_thread_states;
        free_thread_states = state;
    }

    // Get current thread's state, creating if necessary
    static ThreadState* get_thread_state() {
        // Ensure key is initialized
        std::call_once(key_init_flag, initialize_key);

        // Get thread state or create new one
        ThreadState* state = static_cast<ThreadState*>(pthread_getspecific(thread_key));

        if (!state) {
            // Try to reuse from free list first
            {
                std::lock_guard<std::mutex> lock(chunk_mutex);
                if (free_thread_states) {
                    state = free_thread_states;
                    free_thread_states = state->next;
                    state->next = nullptr;
                }
            }

            // If no free states, create a new one
            if (!state) {
                state = new ThreadState();
            }

            // Associate with thread
            pthread_setspecific(thread_key, state);
        }

        return state;
    }

public:
    // Allocate memory of size n
    static void* allocate(size_t n) {
        // If n is too large, use standard allocator
        if (n > _Max_size) {
            return ::operator new(n, std::nothrow);
        }

        // Round up to alignment multiple
        n = detail::align_up(n);

        // Delegate to thread-local state
        ThreadState* state = get_thread_state();
        return state->allocate(n);
    }

    // Deallocate memory of size n
    static void deallocate(void* p, size_t n) {
        if (!p)
            return;

        // If n is too large, use standard deallocator
        if (n > _Max_size) {
            ::operator delete(p);
            return;
        }

        // Round up to alignment multiple
        n = detail::align_up(n);

        // Delegate to thread-local state
        get_thread_state()->deallocate(p, n);
    }

    // Allocate chunk of memory from global pool
    static char* chunk_allocate(size_t size, size_t& adjustment) {
        std::lock_guard<std::mutex> lock(chunk_mutex);

        char* result = nullptr;

        // Try to fulfill from current memory pool
        size_t total_bytes = size + adjustment;

        if (static_cast<size_t>(end_free - start_free) >= total_bytes) {
            result = start_free;
            start_free += total_bytes;
            return result;
        }

        // If current pool is not enough but can provide at least one object
        if (static_cast<size_t>(end_free - start_free) >= size) {
            // Use what's left for this request
            result = start_free;
            start_free += size;
            adjustment = 0;
            return result;
        }

        // Need to allocate a new chunk
        // Try to add any remaining fragment to appropriate free list
        if (end_free - start_free > 0) {
            // Determine which free list to add this memory to
            size_t leftover_size = end_free - start_free;
            size_t index = detail::size_to_index(leftover_size);

            if (index < ThreadState::NUM_FREE_LISTS) {
                // Get thread state
                ThreadState* state = get_thread_state();

                // Add fragment to free list
                MemoryBlock* block = reinterpret_cast<MemoryBlock*>(start_free);
                block->next = state->free_lists[index].load(std::memory_order_relaxed);
                state->free_lists[index].store(block, std::memory_order_release);
            }
        }

        // Calculate how much to allocate
        size_t bytes_to_get = 2 * total_bytes + detail::align_up(heap_size >> 4);

        // Try to allocate a new pool
        start_free = static_cast<char*>(::operator new(bytes_to_get, std::nothrow));

        if (!start_free) {
            // Out of memory, try to find a free list that has blocks
            // and use one of those
            for (size_t i = size / DEFAULT_ALIGNMENT; i < ThreadState::NUM_FREE_LISTS; ++i) {
                ThreadState* state = get_thread_state();
                MemoryBlock* block =
                    state->free_lists[i].exchange(nullptr, std::memory_order_acquire);

                if (block) {
                    // Found a free block, split it if necessary
                    start_free = reinterpret_cast<char*>(block);
                    end_free = start_free + (i + 1) * DEFAULT_ALIGNMENT;
                    return chunk_allocate(size, adjustment);  // Retry with the found memory
                }
            }

            // Last resort - try allocating exactly what's needed, use malloc
            start_free = static_cast<char*>(malloc_alloc::allocate(total_bytes));

            if (!start_free) {
                throw std::bad_alloc();  // Genuinely out of memory
            }

            // Set end_free for the minimal allocation
            end_free = start_free + total_bytes;

            // Return directly instead of recursive call
            result = start_free;
            start_free += total_bytes;
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
void* PthreadAllocPerThreadState<_Max_size>::refill(size_t n) {
    // How many objects to allocate at once
    constexpr size_t NOBJS = 20;

    // Try to allocate memory for NOBJS objects of size n
    size_t total_bytes = n * NOBJS;
    size_t adjustment = 0;  // Will be used if alignment adjustment is needed

    char* chunk = PthreadAllocatorTemplate<_Max_size>::chunk_allocate(total_bytes, adjustment);

    if (!chunk) {
        return nullptr;  // Out of memory
    }

    // Adjust for alignment if necessary
    char* aligned_chunk = chunk;
    if (adjustment > 0) {
        aligned_chunk = chunk + adjustment;
    }

    // Always return the first object to satisfy current request
    void* result = aligned_chunk;

    // If there's only one object, just return it
    if (NOBJS == 1) {
        return result;
    }

    // Add remaining objects to the free list
    size_t index = detail::size_to_index(n);
    MemoryBlock* current_block = nullptr;
    MemoryBlock* next_block = nullptr;

    // Start at second object (first is returned to caller)
    for (size_t i = 1; i < NOBJS; ++i) {
        current_block = reinterpret_cast<MemoryBlock*>(aligned_chunk + n * i);

        if (i < NOBJS - 1) {
            next_block = reinterpret_cast<MemoryBlock*>(aligned_chunk + n * (i + 1));
            current_block->next = next_block;
        } else {
            current_block->next = nullptr;  // Last block in chain
        }
    }

    // Update free list, handling possible race conditions
    MemoryBlock* first_block = reinterpret_cast<MemoryBlock*>(aligned_chunk + n);
    MemoryBlock* old_head = free_lists[index].load(std::memory_order_relaxed);

    do {
        // Point last block to current free list head
        current_block->next = old_head;
    } while (!free_lists[index].compare_exchange_weak(
        old_head, first_block, std::memory_order_release, std::memory_order_relaxed));

    return result;
}

// Initialize static members
template <size_t _Max_size>
std::mutex PthreadAllocatorTemplate<_Max_size>::chunk_mutex;

template <size_t _Max_size>
char* PthreadAllocatorTemplate<_Max_size>::start_free = nullptr;

template <size_t _Max_size>
char* PthreadAllocatorTemplate<_Max_size>::end_free = nullptr;

template <size_t _Max_size>
size_t PthreadAllocatorTemplate<_Max_size>::heap_size = 0;

template <size_t _Max_size>
typename PthreadAllocatorTemplate<_Max_size>::ThreadState*
    PthreadAllocatorTemplate<_Max_size>::free_thread_states = nullptr;

template <size_t _Max_size>
pthread_key_t PthreadAllocatorTemplate<_Max_size>::thread_key;

template <size_t _Max_size>
std::once_flag PthreadAllocatorTemplate<_Max_size>::key_init_flag;

template <size_t _Max_size>
inline constexpr bool check_pthread_alloc =
    SimpleAllocator<PthreadAllocatorTemplate<_Max_size>, int>;
static_assert(check_pthread_alloc<MAX_BYTES>,
              "PthreadAllocatorTemplate must satisfy SimpleAllocator concept");

// STL-compatible allocator adapter
template <typename T, size_t _Max_size = MAX_BYTES>
class StlPthreadAllocator {
public:
    using ValueType = T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = T&;
    using ConstReference = const T&;
    using SizeType = std::size_t;
    using DifferenceType = std::ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = StlPthreadAllocator<U, _Max_size>;
    };

    StlPthreadAllocator() = default;

    template <typename U>
    StlPthreadAllocator(const StlPthreadAllocator<U, _Max_size>&) noexcept {}

    Pointer allocate(SizeType n) {
        return static_cast<Pointer>(PthreadAllocatorTemplate<_Max_size>::allocate(n * sizeof(T)));
    }

    void deallocate(Pointer p, SizeType n) {
        PthreadAllocatorTemplate<_Max_size>::deallocate(p, n * sizeof(T));
    }

    Pointer address(Reference x) const noexcept {
        return std::addressof(x);
    }

    ConstPointer address(ConstReference x) const noexcept {
        return std::addressof(x);
    }

    SizeType max_size() const noexcept {
        return SizeType(-1) / sizeof(T);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    // Equality operations
    bool operator==(const StlPthreadAllocator&) const noexcept {
        return true;
    }
    bool operator!=(const StlPthreadAllocator&) const noexcept {
        return false;
    }
};

template <typename T, size_t _Max_size>
inline constexpr bool check_stl_pthread_allocator =
    StandardAllocator<StlPthreadAllocator<T, _Max_size>>;
static_assert(check_stl_pthread_allocator<int, MAX_BYTES>,
              "StlPthreadAllocator must satisfy StandardAllocator concept");
}  // namespace mstl

#endif