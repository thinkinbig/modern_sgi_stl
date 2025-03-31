#ifndef __MSGI_STL_INTERNAL_HEAP_H
#define __MSGI_STL_INTERNAL_HEAP_H

#include "mstl_iterator.h"

namespace mstl {

// 堆操作函数：push_heap, pop_heap, make_heap, sort_heap

// 内部函数：将新元素加入堆
// 参数说明：
// __first: 堆的起始位置
// __holeIndex: 新元素的位置
// __topIndex: 堆顶位置
// __value: 新元素的值
template <class _RandomAccessIterator, class _Distance, class _Tp>
void __push_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __topIndex,
                 _Tp __value) {
    _Distance __parent = (__holeIndex - 1) / 2;

    while (__holeIndex > __topIndex && *(__first + __parent) < __value) {
        *(__first + __holeIndex) = *(__first + __parent);
        __holeIndex = __parent;
        __parent = (__holeIndex - 1) / 2;
    }

    *(__first + __holeIndex) = __value;
}

// 内部函数：在移除堆顶元素后重建堆结构
// 参数说明：
// __first: 堆的起始迭代器
// __holeIndex: 需要开始下沉的位置（通常是堆顶）
// __len: 堆的当前大小
// __value: 需要放入合适位置的值（通常是原来的最后一个元素）
template <class _RandomAccessIterator, class _Distance, class _Tp>
void __adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __len,
                   _Tp __value) {
    _Distance __topIndex = __holeIndex;
    _Distance __rightIndex = __holeIndex * 2 + 2;

    // 如果存在子节点
    while (__rightIndex < __len) {
        if (*(__first + __rightIndex) < *(__first + __rightIndex - 1)) {
            __rightIndex--;
        }
        // 将较大的子节点上移
        *(__first + __holeIndex) = *(__first + __rightIndex);
        __holeIndex = __rightIndex;
        __rightIndex = __rightIndex * 2 + 2;
    }

    // 处理特殊情况 只有左节点
    if (__rightIndex == __len) {
        *(__first + __holeIndex) = *(__first + __rightIndex - 1);
        __holeIndex = __rightIndex - 1;
    }
    __push_heap(__first, __holeIndex, __topIndex, __value);
}

/*
// 我们之前的实现（一步到位的方式）
template <class _RandomAccessIterator, class _Distance, class _Tp>
void __adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __len,
                   _Tp __value) {
    _Distance __rightIndex = __holeIndex * 2 + 2;

    // 如果存在子节点
    while (__rightIndex < __len) {
        // 选择较大的子节点
        if (*(__first + __rightIndex) < *(__first + __rightIndex - 1)) {
            __rightIndex--;
        }
        
        // 如果当前节点已经大于等于子节点，说明找到了正确位置
        if (*(__first + __holeIndex) >= *(__first + __rightIndex)) {
            break;
        }
        
        // 将较大的子节点上移
        *(__first + __holeIndex) = *(__first + __rightIndex);
        __holeIndex = __rightIndex;
        __rightIndex = __rightIndex * 2 + 2;
    }

    // 处理特殊情况 只有左节点
    if (__rightIndex == __len) {
        if (*(__first + __holeIndex) < *(__first + __rightIndex - 1)) {
            *(__first + __holeIndex) = *(__first + __rightIndex - 1);
            __holeIndex = __rightIndex - 1;
        }
    }

    // 将值放入最终位置
    *(__first + __holeIndex) = __value;
}

为什么源码的实现更好：

1. 性能优势：
   - 在大多数情况下，需要调整的节点都更接近树的底部
   - 源码先下沉到底部再上浮，减少了比较次数
   - 下沉时只需要移动较大的子节点，不需要比较 __value

2. 代码复用：
   - 源码复用了 __push_heap 函数
   - 减少了代码重复，提高了可维护性
   - 核心的上浮逻辑只在一个地方

3. 实现简单性：
   - 下沉和上浮都是单向操作，逻辑更清晰
   - 不需要在中间位置反复比较
   - 减少了边界条件的处理

4. 实际应用场景：
   - 在随机数据中，新插入的元素通常需要下沉到接近底部
   - 源码的实现更符合实际使用场景
*/

// 内部函数：弹出堆顶元素
// 参数说明：
// __first: 堆的起始位置
// __last: 堆的结束位置
// __result: 存储弹出元素的位置
// __value: 最后一个元素的值
template <class _RandomAccessIterator, class _Tp, class _Distance>
inline void __pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
                       _RandomAccessIterator __result, _Tp __value, _Distance*) {
    // 1. 将堆顶元素保存到结果位置
    *(__result) = *(__first);
    
    // 2. 将最后一个元素移到堆顶
    *(__first) = __value;
    
    // 3. 调整堆结构
    __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
}

// 内部函数：构建堆
// 参数说明：
// __first: 序列的起始位置
// __last: 序列的结束位置
template <class _RandomAccessIterator, class _Tp, class _Distance>
void __make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Tp*, _Distance*) {

    if (__last - __first < 2) {
        return;
    }

    _Distance __len = __last - __first;

    //  从非子节点开始 对每个节点进行下沉操作
    for (_Distance __hole_index = (__len - 2) / 2; __hole_index >=0; --__hole_index) {
        __adjust_heap(__first, __hole_index, __len, *(__first + __hole_index));
    }
}

// 对外接口：将新元素加入堆
template <class _RandomAccessIterator>
inline void push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {

    using _Distance = typename iterator_traits<_RandomAccessIterator>::difference_type;
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    
    _Distance __holeIndex = __last - __first - 1;

    _ValueType __value = *(__last - 1);
    __push_heap(__first, __holeIndex, _Distance(0), __value);
}

// 对外接口：弹出堆顶元素
template <class _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    using _Distance = typename iterator_traits<_RandomAccessIterator>::difference_type;
    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;

    __pop_heap(__first, __last - 1, __last - 1, _ValueType(*(__last - 1)), static_cast<_Distance*>(nullptr));
}

// 对外接口：构建堆
template <class _RandomAccessIterator>
inline void make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {

    using _ValueType = typename iterator_traits<_RandomAccessIterator>::value_type;
    using _DistanceType = typename iterator_traits<_RandomAccessIterator>::difference_type;
    __make_heap(__first, __last, static_cast<_ValueType*>(nullptr), static_cast<_DistanceType*>(nullptr));
}

// 对外接口：堆排序
template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {    // TODO: 实现堆排序
    // 1. 重复执行pop_heap操作
    // 2. 直到堆中只剩一个元素
    while (__last - __first > 1) {
        pop_heap(__first, __last--);
    }
}

}  // namespace mstl

#endif /* __MSTL_HEAP_H */