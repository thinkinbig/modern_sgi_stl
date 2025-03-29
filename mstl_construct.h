#ifndef __MSGI_STL_INTERNAL_CONSTRUCT_H
#define __MSGI_STL_INTERNAL_CONSTRUCT_H

#include <iterator>
#include <new>
#include <type_traits>
#include "mstl_concepts.h"
#include "mstl_iterator_traits.h"

namespace mstl {

// 构造对象函数
template <typename T1, typename... Args>
inline void construct(T1* p, Args&&... args) {
    ::new (p) T1(std::forward<Args>(args)...);
}

// 销毁单个对象函数
template <typename T>
inline void destroy(T* pointer) {
    pointer->~T();
}

// 销毁迭代器范围内对象 - 特化版本（可平凡销毁的类型）
template <InputIterator I>
inline void __destroy_aux([[maybe_unused]] I first, [[maybe_unused]] I last, ::std::true_type) {
    // 平凡析构类型不需要显式调用析构函数
}

// 销毁迭代器范围内对象 - 一般版本
template <InputIterator I>
inline void __destroy_aux(I first, I last, ::std::false_type) {
    for (; first != last; ++first) {
        destroy(&*first);
    }
}

// 基于值类型选择合适的销毁方式
template <InputIterator I, typename T>
inline void __destroy(I first, I last, T*) {
    using trivial_destructor = typename ::std::is_trivially_destructible<T>;
    __destroy_aux(first, last, trivial_destructor());
}

// 统一的迭代器范围销毁接口
template <InputIterator I>
inline void destroy(I first, I last) {
    using value_type = typename iterator_traits<I>::value_type;
    __destroy(first, last, static_cast<value_type*>(nullptr));
}

// 特化版本：char和wchar_t
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}

// 单个对象char版本
inline void destroy(char*) {}

// 单个对象wchar_t版本
inline void destroy(wchar_t*) {}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_CONSTRUCT_H
