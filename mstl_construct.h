#ifndef __MSGI_STL_INTERNAL_CONSTRUCT_H
#define __MSGI_STL_INTERNAL_CONSTRUCT_H

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

template <typename ForwardIterator, typename T>
void __destroy(ForwardIterator first, ForwardIterator last, T*) {
    __destroy_aux(first, last, std::is_trivially_destructible<T>());
}

template <typename ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last) {
    using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
    __destroy(first, last, static_cast<value_type*>(nullptr));
}

void destroy([[maybe_unused]] char*) {}

void destroy([[maybe_unused]] wchar_t*) {}

} // namespace mstl


#endif // __MSGI_STL_INTERNAL_CONSTRUCT_H
