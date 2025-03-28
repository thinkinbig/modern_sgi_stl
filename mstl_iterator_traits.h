#ifndef __MSTL_ITERATOR_TRAITS_H
#define __MSTL_ITERATOR_TRAITS_H

#include <cstddef>
#include "mstl_iterator_tags.h"

namespace mstl {
// 基本迭代器特征
template <typename I>
struct iterator_traits {
    using iterator_category = typename I::iterator_category;
    using value_type = typename I::value_type;
    using difference_type = typename I::difference_type;
    using pointer = typename I::pointer;
    using reference = typename I::reference;
};

// 指针特化
template <typename T>
struct iterator_traits<T*> {
    using iterator_category = random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;
};

// const指针特化
template <typename T>
struct iterator_traits<const T*> {
    using iterator_category = random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
};

// void类型特化
template <>
struct iterator_traits<void> {
    using iterator_category = void;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;
};

// 类型别名辅助工具
template <typename I>
using iterator_value_t = typename iterator_traits<I>::value_type;

template <typename I>
using iterator_reference_t = typename iterator_traits<I>::reference;

template <typename I>
using iterator_difference_t = typename iterator_traits<I>::difference_type;

template <typename I>
using iterator_category_t = typename iterator_traits<I>::iterator_category;

template <typename I>
using iterator_pointer_t = typename iterator_traits<I>::pointer;

// 迭代器类型检查辅助函数，用于代替concepts
namespace detail {
template <typename T, typename = void>
struct is_valid_iterator : std::false_type {};

template <typename T>
struct is_valid_iterator<
    T, std::void_t<typename iterator_traits<T>::iterator_category,
                   typename iterator_traits<T>::value_type,
                   typename iterator_traits<T>::difference_type,
                   typename iterator_traits<T>::pointer, typename iterator_traits<T>::reference>>
    : std::true_type {};

template <typename T>
struct is_input_iterator
    : std::bool_constant<
          is_valid_iterator<T>::value &&
          std::is_base_of_v<input_iterator_tag, typename iterator_traits<T>::iterator_category>> {};

template <typename T>
struct is_forward_iterator
    : std::bool_constant<
          is_input_iterator<T>::value &&
          std::is_base_of_v<forward_iterator_tag, typename iterator_traits<T>::iterator_category>> {
};

template <typename T>
struct is_bidirectional_iterator
    : std::bool_constant<is_forward_iterator<T>::value &&
                         std::is_base_of_v<bidirectional_iterator_tag,
                                           typename iterator_traits<T>::iterator_category>> {};

template <typename T>
struct is_random_access_iterator
    : std::bool_constant<is_bidirectional_iterator<T>::value &&
                         std::is_base_of_v<random_access_iterator_tag,
                                           typename iterator_traits<T>::iterator_category>> {};

template <typename T>
struct is_contiguous_iterator
    : std::bool_constant<is_random_access_iterator<T>::value &&
                         std::is_base_of_v<contiguous_iterator_tag,
                                           typename iterator_traits<T>::iterator_category>> {};

template <typename I>
typename iterator_traits<I>::iterator_category iterator_category(const I&) {
    return typename iterator_traits<I>::iterator_category();
}

template <typename I>
typename iterator_traits<I>::difference_type* distance_type(const I&) {
    return static_cast<typename iterator_traits<I>::difference_type*>(nullptr);
}

template <typename I>
typename iterator_traits<I>::value_type* value_type(const I&) {
    return static_cast<typename iterator_traits<I>::value_type*>(nullptr);
}
}  // namespace detail

// 迭代器基类 - 用于简化自定义迭代器的实现
template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T*,
          typename Reference = T&>
struct iterator {
    using iterator_category = Category;
    using value_type = T;
    using difference_type = Distance;
    using pointer = Pointer;
    using reference = Reference;
};

// 反向迭代器
template <typename Iterator>
class reverse_iterator {
protected:
    Iterator current;

public:
    using iterator_type = Iterator;
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type = typename iterator_traits<Iterator>::value_type;
    using difference_type = typename iterator_traits<Iterator>::difference_type;
    using pointer = typename iterator_traits<Iterator>::pointer;
    using reference = typename iterator_traits<Iterator>::reference;

    // 构造函数
    reverse_iterator() = default;
    explicit reverse_iterator(Iterator x) : current(x) {}

    template <typename U>
    reverse_iterator(const reverse_iterator<U>& other) : current(other.base()) {}

    // 访问底层迭代器
    Iterator base() const {
        return current;
    }

    // 运算符
    reference operator*() const {
        Iterator tmp = current;
        return *--tmp;
    }

    pointer operator->() const {
        return &(operator*());
    }

    reverse_iterator& operator++() {
        --current;
        return *this;
    }

    reverse_iterator operator++(int) {
        reverse_iterator tmp = *this;
        --current;
        return tmp;
    }

    reverse_iterator& operator--() {
        ++current;
        return *this;
    }

    reverse_iterator operator--(int) {
        reverse_iterator tmp = *this;
        ++current;
        return tmp;
    }

    reverse_iterator operator+(difference_type n) const {
        return reverse_iterator(current - n);
    }

    reverse_iterator& operator+=(difference_type n) {
        current -= n;
        return *this;
    }

    reverse_iterator operator-(difference_type n) const {
        return reverse_iterator(current + n);
    }

    reverse_iterator& operator-=(difference_type n) {
        current += n;
        return *this;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }
};

// 反向迭代器的非成员函数
template <typename Iterator1, typename Iterator2>
bool operator==(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() == y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator!=(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() != y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator<(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() > y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator<=(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() >= y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator>(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() < y.base();
}

template <typename Iterator1, typename Iterator2>
bool operator>=(const reverse_iterator<Iterator1>& x, const reverse_iterator<Iterator2>& y) {
    return x.base() <= y.base();
}

template <typename Iterator1, typename Iterator2>
auto operator-(const reverse_iterator<Iterator1>& x,
               const reverse_iterator<Iterator2>& y) -> decltype(y.base() - x.base()) {
    return y.base() - x.base();
}

template <typename Iterator>
reverse_iterator<Iterator> operator+(typename reverse_iterator<Iterator>::difference_type n,
                                     const reverse_iterator<Iterator>& x) {
    return reverse_iterator<Iterator>(x.base() - n);
}

// 便捷函数：创建反向迭代器
template <typename Iterator>
reverse_iterator<Iterator> make_reverse_iterator(Iterator i) {
    return reverse_iterator<Iterator>(i);
}

}  // namespace mstl

#endif  // __MSTL_ITERATOR_TRAITS_H