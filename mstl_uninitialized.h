#ifndef __MSGI_STL_INTERNAL_UNINITIALIZED_H
#define __MSGI_STL_INTERNAL_UNINITIALIZED_H

#include <algorithm>
#include <cstring>
#include <iterator>
#include <type_traits>
#include "mstl_concepts.h"
#include "mstl_construct.h"

namespace mstl {

// 全局函数
template <InputIterator InputIterator, ForwardIterator ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last,
                                         ForwardIterator result, std::true_type) {
    return std::copy(first, last, result);
}

template <InputIterator InputIterator, ForwardIterator ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last,
                                         ForwardIterator result, std::false_type) {
    ForwardIterator cur = result;
    try {
        for (; first != last; ++cur, ++first) {
            mstl::construct(&*cur, *first);
        }
        return cur;
    } catch (...) {
        mstl::destroy(result, cur);
        throw;
    }
}

template <InputIterator InputIterator, ForwardIterator ForwardIterator, typename T>
ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last,
                                     ForwardIterator result, T*) {
    return __uninitialized_copy_aux(first, last, result, std::is_trivially_copyable<T>());
}

template <InputIterator InputIterator, ForwardIterator ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first, InputIterator last,
                                   ForwardIterator result) {
    using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
    return __uninitialized_copy(first, last, result, static_cast<value_type*>(nullptr));
}

char* uninitialized_copy(const char* first, const char* last, char* result) {
    std::memcpy(result, first, last - first);
    return result + (last - first);
}

wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result) {
    std::memcpy(result, first, (last - first) * sizeof(wchar_t));
    return result + (last - first);
}

// fill_n 相关函数声明
template <ForwardIterator ForwardIterator, typename Size, typename T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x,
                                           std::true_type) {
    return std::fill_n(first, n, x);
}

template <ForwardIterator ForwardIterator, typename Size, typename T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x,
                                           std::false_type) {
    ForwardIterator cur = first;
    try {
        for (; n > 0; --n, ++cur) {
            mstl::construct(&*cur, x);
        }
        return cur;
    } catch (...) {
        mstl::destroy(first, cur);
        throw;
    }
}

template <ForwardIterator ForwardIterator, typename Size, typename T, typename U>
ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, U*) {
    return __uninitialized_fill_n_aux(first, n, x, std::is_trivially_copyable<U>());
}

template <ForwardIterator ForwardIterator, std::integral Size, typename T>
ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
    using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
    return __uninitialized_fill_n(first, n, x, static_cast<value_type*>(nullptr));
}

template <ForwardIterator ForwardIterator, typename T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x,
                              std::false_type) {
    ForwardIterator curr = first;
    try {
        for (; curr != last; ++curr) {
            mstl::construct(&*curr, x);
        }
    } catch (...) {
        mstl::destroy(first, curr);
        throw;
    }
}

template <ForwardIterator ForwardIterator, typename T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x,
                              std::true_type) {
    std::fill(first, last, x);
}

template <ForwardIterator ForwardIterator, typename T, typename U>
void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, U*) {
    return __uninitialized_fill_aux(first, last, x, std::is_trivially_copyable<U>());
}

template <ForwardIterator ForwardIterator, typename T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x) {
    using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
    __uninitialized_fill(first, last, x, static_cast<value_type*>(nullptr));
}

}  // namespace mstl
#endif
