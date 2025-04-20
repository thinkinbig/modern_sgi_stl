#ifndef __MSTL_ITERATOR_TRAITS_H
#define __MSTL_ITERATOR_TRAITS_H

#include <cstddef>
#include <iterator>
#include "mstl_iterator_tags.h"

namespace mstl {
// 基本迭代器特征
template <typename Iterator>
struct IteratorTraits {
    using IteratorCategory = typename Iterator::IteratorCategory;
    using ValueType = typename Iterator::ValueType;
    using DifferenceType = typename Iterator::DifferenceType;
    using Pointer = typename Iterator::Pointer;
    using Reference = typename Iterator::Reference;
};

// 指针特化
template <typename T>
struct IteratorTraits<T*> {
    using IteratorCategory = RandomAccessIteratorTag;
    using ValueType = std::remove_cv_t<T>;
    using DifferenceType = ptrdiff_t;
    using Pointer = T*;
    using Reference = T&;
};

// const指针特化
template <typename T>
struct IteratorTraits<const T*> {
    using IteratorCategory = RandomAccessIteratorTag;
    using ValueType = std::remove_cv_t<T>;
    using DifferenceType = ptrdiff_t;
    using Pointer = const T*;
    using Reference = const T&;
};

// void类型特化
template <>
struct IteratorTraits<void> {
    using IteratorCategory = void;
    using ValueType = void;
    using DifferenceType = std::ptrdiff_t;
    using Pointer = void;
    using Reference = void;
};

// 类型别名辅助工具
template <typename I>
using IteratorValueType = typename IteratorTraits<I>::ValueType;

template <typename I>
using IteratorReferenceType = typename IteratorTraits<I>::Reference;

template <typename I>
using IteratorDifferenceType = typename IteratorTraits<I>::DifferenceType;

template <typename I>
using IteratorCategoryType = typename IteratorTraits<I>::IteratorCategory;

template <typename I>
using IteratorPointerType = typename IteratorTraits<I>::Pointer;

// 迭代器类型检查辅助函数，用于代替concepts
namespace detail {
template <typename T, typename = void>
struct is_valid_iterator : std::false_type {};

template <typename T>
struct is_valid_iterator<
    T, std::void_t<typename IteratorTraits<T>::IteratorCategory,
                   typename IteratorTraits<T>::ValueType,
                   typename IteratorTraits<T>::DifferenceType,
                   typename IteratorTraits<T>::Pointer,
                   typename IteratorTraits<T>::Reference>>
    : std::true_type {};

template <typename T>
struct is_input_iterator
    : std::bool_constant<
          is_valid_iterator<T>::value &&
          std::is_base_of_v<InputIteratorTag, typename IteratorTraits<T>::IteratorCategory>> {};

template <typename T>
struct is_forward_iterator
    : std::bool_constant<
          is_input_iterator<T>::value &&
          std::is_base_of_v<ForwardIteratorTag, typename IteratorTraits<T>::IteratorCategory>> {};

template <typename T>
struct is_bidirectional_iterator
    : std::bool_constant<is_forward_iterator<T>::value &&
                         std::is_base_of_v<BidirectionalIteratorTag,
                                           typename IteratorTraits<T>::IteratorCategory>> {};

template <typename T>
struct is_random_access_iterator
    : std::bool_constant<is_bidirectional_iterator<T>::value &&
                         std::is_base_of_v<RandomAccessIteratorTag,
                                           typename IteratorTraits<T>::IteratorCategory>> {};

template <typename T>
struct is_contiguous_iterator
    : std::bool_constant<is_random_access_iterator<T>::value &&
                         std::is_base_of_v<ContiguousIteratorTag,
                                           typename IteratorTraits<T>::IteratorCategory>> {};

template <typename I>
typename IteratorTraits<I>::IteratorCategory iterator_category(const I&) {
    return typename IteratorTraits<I>::IteratorCategory();
}

template <typename I>
typename IteratorTraits<I>::DifferenceType* distance_type(const I&) {
    return static_cast<typename IteratorTraits<I>::DifferenceType*>(nullptr);
}

template <typename I>
typename IteratorTraits<I>::ValueType* value_type(const I&) {
    return static_cast<typename IteratorTraits<I>::ValueType*>(nullptr);
}
}  // namespace detail

// 迭代器基类 - 用于简化自定义迭代器的实现
template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T*,
          typename Reference = T&>
struct IteratorBase {
    using iterator_category = Category;
    using value_type = T;
    using difference_type = Distance;
    using pointer = Pointer;
    using reference = Reference;
};

// 反向迭代器
template <typename Iterator>
class ReverseIterator {
protected:
    Iterator current;

public:
    using IteratorType = Iterator;
    using IteratorCategory = typename IteratorTraits<Iterator>::IteratorCategory;
    using ValueType = typename IteratorTraits<Iterator>::ValueType;
    using DifferenceType = typename IteratorTraits<Iterator>::DifferenceType;
    using Pointer = typename IteratorTraits<Iterator>::Pointer;
    using Reference = typename IteratorTraits<Iterator>::Reference;

    // 构造函数
    ReverseIterator() = default;
    explicit ReverseIterator(Iterator x) : current(x) {}

    template <typename U>
    ReverseIterator(const ReverseIterator<U>& other) : current(other.base()) {}

    // 访问底层迭代器
    Iterator base() const {
        return current;
    }

    // 运算符
    Reference operator*() const {
        Iterator tmp = current;
        return *--tmp;
    }

    Pointer operator->() const {
        return &(operator*());
    }

    ReverseIterator& operator++() {
        --current;
        return *this;
    }

    ReverseIterator operator++(int) {
        ReverseIterator tmp = *this;
        --current;
        return tmp;
    }

    ReverseIterator& operator--() {
        ++current;
        return *this;
    }

    ReverseIterator operator--(int) {
        ReverseIterator tmp = *this;
        ++current;
        return tmp;
    }

    ReverseIterator operator+(DifferenceType n) const {
        return ReverseIterator(current - n);
    }

    ReverseIterator& operator+=(DifferenceType n) {
        current -= n;
        return *this;
    }

    ReverseIterator operator-(DifferenceType n) const {
        return ReverseIterator(current + n);
    }

    ReverseIterator& operator-=(DifferenceType n) {
        current += n;
        return *this;
    }

    Reference operator[](DifferenceType n) const {
        return *(*this + n);
    }
};

// 反向迭代器的非成员函数
template <typename Iterator1, typename Iterator2>
bool operator==(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() == y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator!=(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() != y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator<(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() > y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator<=(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() >= y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator>(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() < y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator>=(const ReverseIterator<Iterator1>& x, const ReverseIterator<Iterator2>& y) {
    return x.base() <= y.base();
}

template <typename Iterator1, typename Iterator2>
auto operator-(const ReverseIterator<Iterator1>& x,
               const ReverseIterator<Iterator2>& y) -> decltype(y.base() - x.base()) {
    return y.base() - x.base();
}

template <typename Iterator>
ReverseIterator<Iterator> operator+(typename ReverseIterator<Iterator>::difference_type n,
                                    const ReverseIterator<Iterator>& x) {
    return ReverseIterator<Iterator>(x.base() - n);
}

// 便捷函数：创建反向迭代器
template <typename Iterator>
ReverseIterator<Iterator> make_reverse_iterator(Iterator i) {
    return ReverseIterator<Iterator>(i);
}

}  // namespace mstl

#endif  // __MSTL_ITERATOR_TRAITS_H