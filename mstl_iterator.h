#ifndef __MSGI_STL_INTERNAL_ITERATOR_H
#define __MSGI_STL_INTERNAL_ITERATOR_H

#include "mstl_concepts.h"
#include "mstl_iterator_tags.h"
#include "mstl_iterator_traits.h"

namespace mstl {

// 声明 __distance 函数
template <InputIterator I>
typename IteratorTraits<I>::DifferenceType __distance(I first, I last, InputIteratorTag);

template <RandomAccessIterator I>
typename IteratorTraits<I>::DifferenceType __distance(I first, I last, RandomAccessIteratorTag);

// 统一的distance接口
template <typename I>
typename IteratorTraits<I>::DifferenceType distance(I first, I last) {
    return __distance(first, last, typename IteratorTraits<I>::IteratorCategory());
}


// 计算迭代器之间的距离 - 输入迭代器版本
template <InputIterator I>
typename IteratorTraits<I>::DifferenceType __distance(I first, I last, InputIteratorTag) {
    typename IteratorTraits<I>::DifferenceType n = 0;
    while (first != last) {
        ++first;
        ++n;
    }
    return n;
}

// 计算迭代器之间的距离 - 随机访问迭代器版本
template <RandomAccessIterator I>
typename IteratorTraits<I>::DifferenceType __distance(I first, I last, RandomAccessIteratorTag) {
    return last - first;
}

// advance实现 - 输入迭代器版本
template <InputIterator I>
void __advance(I& i, typename IteratorTraits<I>::DifferenceType n, InputIteratorTag) {
    while (n--)
        ++i;
}

// advance实现 - 双向迭代器版本
template <BidirectionalIterator I>
void __advance(I& i, typename IteratorTraits<I>::DifferenceType n, BidirectionalIteratorTag) {
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
void __advance(I& i, typename IteratorTraits<I>::DifferenceType n, RandomAccessIteratorTag) {
    i += n;
}

// 统一的advance接口
template <typename I>
void advance(I& i, typename IteratorTraits<I>::DifferenceType n) {
    __advance(i, n, typename IteratorTraits<I>::IteratorCategory());
}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_ITERATOR_H