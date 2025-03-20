#ifndef MSTL_CONSTRUCT_H
#define MSTL_CONSTRUCT_H

#include <new>
#include <type_traits>
#include <iterator>

namespace mstl {

// 全局函数

template <typename T1, typename... Args>
void construct(T1* p, Args&&... args) {
    new (p) T1(std::forward<Args>(args)...);
}

template <typename T>
void destroy(T* pointer) {
    pointer->~T();
}

template <typename ForwardIterator>
void __destroy_aux(ForwardIterator first, ForwardIterator last, std::true_type) {}

template <typename ForwardIterator>
void __destroy_aux(ForwardIterator first, ForwardIterator last, std::false_type) {
    for (; first != last; ++first) {
        destroy(&*first);
    }
}

template <typename ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last) {
    __destroy_aux(first, last, std::is_trivially_destructible<typename std::iterator_traits<ForwardIterator>::value_type>());
}

template <typename ForwardIterator, typename T>
void __destroy(ForwardIterator first, ForwardIterator last, T*) {
    __destroy_aux(first, last, std::is_trivially_destructible<T>());
}

void destroy([[maybe_unused]] char*) {}

void destroy([[maybe_unused]] wchar_t*) {}

} // namespace mstl


#endif // MSTL_CONSTRUCT_H
