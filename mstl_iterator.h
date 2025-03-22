#ifndef __MSGI_STL_INTERNAL_ITERATOR_H
#define __MSGI_STL_INTERNAL_ITERATOR_H

#include "mstl_iterator_traits.h"
#include "mstl_concepts.h"

namespace mstl {

    // 辅助函数: 返回迭代器的类别
    template <typename I>
    typename iterator_traits<I>::iterator_category 
    iterator_category(const I&) {
        return typename iterator_traits<I>::iterator_category();
    }

    // 辅助函数: 返回迭代器的距离类型
    template <typename I>
    typename iterator_traits<I>::difference_type*
    distance_type(const I&) {
        return static_cast<typename iterator_traits<I>::difference_type*>(nullptr);
    }

    // 辅助函数: 返回迭代器的值类型
    template <typename I>
    typename iterator_traits<I>::value_type*
    value_type(const I&) {
        return static_cast<typename iterator_traits<I>::value_type*>(nullptr);
    }

    // 计算迭代器之间的距离 - 输入迭代器版本
    template <typename I>
    typename iterator_traits<I>::difference_type
    __distance(I first, I last, input_iterator_tag) {
        typename iterator_traits<I>::difference_type n = 0;
        while (first != last) {
            ++first; ++n;
        }
        return n;
    }

    // 计算迭代器之间的距离 - 随机访问迭代器版本
    template <typename I>
    typename iterator_traits<I>::difference_type
    __distance(I first, I last, random_access_iterator_tag) {
        return last - first;
    }

    // 统一的distance接口
    template <typename I>
    typename iterator_traits<I>::difference_type
    distance(I first, I last) {
        return __distance(first, last, typename iterator_traits<I>::iterator_category());
    }

    // advance实现 - 输入迭代器版本
    template <typename I>
    void __advance(I& i, typename iterator_traits<I>::difference_type n, input_iterator_tag) {
        while (n--) ++i;
    }

    // advance实现 - 双向迭代器版本
    template <typename I>
    void __advance(I& i, typename iterator_traits<I>::difference_type n, bidirectional_iterator_tag) {
        if (n >= 0) {
            while (n--) ++i;
        } else {
            while (n++) --i;
        }
    }

    // advance实现 - 随机访问迭代器版本
    template <typename I>
    void __advance(I& i, typename iterator_traits<I>::difference_type n, random_access_iterator_tag) {
        i += n;
    }

    // 统一的advance接口
    template <typename I>
    void advance(I& i, typename iterator_traits<I>::difference_type n) {
        __advance(i, n, typename iterator_traits<I>::iterator_category());
    }
    
} // namespace mstl

#endif // __MSGI_STL_INTERNAL_ITERATOR_H 