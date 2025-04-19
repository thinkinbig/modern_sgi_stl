#ifndef __MSGI_STL_INTERNAL_HEAP_H
#define __MSGI_STL_INTERNAL_HEAP_H

#include "mstl_functional.h"
#include "mstl_iterator.h"

namespace mstl {

// 内部函数：将新元素加入堆
// 参数说明：
// __first: 堆的起始位置
// __holeIndex: 新元素的位置
// __topIndex: 堆顶位置
// __value: 新元素的值
// __comp: 比较函数
template <typename _RandomAccessIterator, typename _Distance, typename _Tp, typename _Compare>
void __mstl__push_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __topIndex,
                       _Tp __value, _Compare __comp) {
    _Distance __parent = (__holeIndex - 1) / 2;
    while (__holeIndex > __topIndex && __comp(*(__first + __parent), __value)) {
        *(__first + __holeIndex) = *(__first + __parent);
        __holeIndex = __parent;
        __parent = (__holeIndex - 1) / 2;
    }
    *(__first + __holeIndex) = __value;
}

// 内部函数：调整堆结构
// 参数说明：
// __first: 堆的起始迭代器
// __holeIndex: 需要开始下沉的位置（通常是堆顶）
// __len: 堆的当前大小
// __value: 需要放入合适位置的值（通常是原来的最后一个元素）
// __comp: 比较函数
template <typename _RandomAccessIterator, typename _Distance, typename _Tp, typename _Compare>
void __mstl__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __len,
                         _Tp __value, _Compare __comp) {
    _Distance __topIndex = __holeIndex;
    _Distance __secondChild = 2 * __holeIndex + 2;
    while (__secondChild < __len) {
        if (__comp(*(__first + __secondChild), *(__first + (__secondChild - 1))))
            __secondChild--;
        *(__first + __holeIndex) = *(__first + __secondChild);
        __holeIndex = __secondChild;
        __secondChild = 2 * (__secondChild + 1);
    }
    if (__secondChild == __len) {
        *(__first + __holeIndex) = *(__first + (__secondChild - 1));
        __holeIndex = __secondChild - 1;
    }
    __mstl__push_heap(__first, __holeIndex, __topIndex, __value, __comp);
}

// 内部函数：构建堆
// 参数说明：
// __first: 序列的起始位置
// __last: 序列的结束位置
template <typename _RandomAccessIterator, typename _Tp, typename _Distance, typename _Compare>
void __mstl__make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Tp*,
                       _Distance*, _Compare __comp) {
    if (__last - __first < 2)
        return;
    _Distance __len = __last - __first;
    _Distance __parent = (__len - 2) / 2;
    while (true) {
        __mstl__adjust_heap(__first, __parent, __len, *(__first + __parent), __comp);
        if (__parent == 0)
            return;
        __parent--;
    }
}

// 对外接口：构建堆（带比较函数版本）
template <typename _RandomAccessIterator, typename _Compare>
inline void make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
                      _Compare __comp) {
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    __mstl__make_heap(__first, __last, static_cast<_ValueType*>(nullptr),
                      static_cast<_DistanceType*>(nullptr), __comp);
}

// 对外接口：构建堆（默认版本）
template <typename _RandomAccessIterator>
inline void make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    __mstl__make_heap(__first, __last, static_cast<_ValueType*>(nullptr),
                      static_cast<_DistanceType*>(nullptr), std::less<_ValueType>());
}

// 对外接口：将新元素加入堆（带比较函数版本）
template <typename _RandomAccessIterator, typename _Compare>
inline void push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
                      _Compare __comp) {
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    __mstl__push_heap(__first, _DistanceType(__last - __first - 1), _DistanceType(0), *(__last - 1),
                      __comp);
}

// 对外接口：将新元素加入堆（默认版本）
template <typename _RandomAccessIterator>
inline void push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    __mstl__push_heap(__first, _DistanceType(__last - __first - 1), _DistanceType(0), *(__last - 1),
                      std::less<_ValueType>());
}

// 对外接口：弹出堆顶元素（带比较函数版本）
template <typename _RandomAccessIterator, typename _Compare>
inline void pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare __comp) {
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    if (__last - __first > 1) {
        std::ranges::iter_swap(__first, __last - 1);
        __mstl__adjust_heap(__first, _DistanceType(0), _DistanceType(__last - __first - 1),
                            *__first, __comp);
    }
}

// 对外接口：弹出堆顶元素（默认版本）
template <typename _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    mstl::pop_heap(__first, __last, std::less<_ValueType>());
}

// 对外接口：堆排序（带比较函数版本）
template <typename _RandomAccessIterator, typename _Compare>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare __comp) {
    while (__last - __first > 1) {
        mstl::pop_heap(__first, __last--, __comp);
    }
}

// 对外接口：堆排序（默认版本）
template <typename _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    mstl::sort_heap(__first, __last, std::less<_ValueType>());
}

}  // namespace mstl

#endif /* __MSGI_STL_INTERNAL_HEAP_H */