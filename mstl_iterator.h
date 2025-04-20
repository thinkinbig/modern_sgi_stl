#ifndef __MSGI_STL_INTERNAL_ITERATOR_H
#define __MSGI_STL_INTERNAL_ITERATOR_H

#include "mstl_concepts.h"
#include "mstl_iterator_traits.h"
#include "mstl_iterator_tags.h"

namespace mstl {

// 声明 __distance 函数
template <InputIterator I>
typename iterator_traits<I>::difference_type __distance(I first, I last, mstl::input_iterator_tag);

template <RandomAccessIterator I>
typename iterator_traits<I>::difference_type __distance(I first, I last, mstl::random_access_iterator_tag);

// 统一的distance接口
template <typename I>
typename iterator_traits<I>::difference_type distance(I first, I last) {
    return __distance(first, last, typename iterator_traits<I>::iterator_category());
}

// 计算迭代器之间的距离 - 输入迭代器版本
template <InputIterator I>
typename iterator_traits<I>::difference_type __distance(I first, I last, mstl::input_iterator_tag) {
    typename iterator_traits<I>::difference_type n = 0;
    while (first != last) {
        ++first;
        ++n;
    }
    return n;
}

// 计算迭代器之间的距离 - 随机访问迭代器版本
template <RandomAccessIterator I>
typename iterator_traits<I>::difference_type __distance(I first, I last,
                                                        mstl::random_access_iterator_tag) {
    return last - first;
}

// advance实现 - 输入迭代器版本
template <InputIterator I>
void __advance(I& i, typename iterator_traits<I>::difference_type n, mstl::input_iterator_tag) {
    while (n--)
        ++i;
}

// advance实现 - 双向迭代器版本
template <BidirectionalIterator I>
void __advance(I& i, typename iterator_traits<I>::difference_type n, mstl::bidirectional_iterator_tag) {
    if (n >= 0) {
        while (n--)
            ++i;
    } else {
        while (n++)
            --i;
    }
}

// advance实现 - 随机访问迭代器版本
template <RandomAccessIterator I>
void __advance(I& i, typename iterator_traits<I>::difference_type n, mstl::random_access_iterator_tag) {
    i += n;
}

// 统一的advance接口
template <typename I>
void advance(I& i, typename iterator_traits<I>::difference_type n) {
    __advance(i, n, typename iterator_traits<I>::iterator_category());
}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_ITERATOR_H