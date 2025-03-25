#ifndef __MSGI_STL_INTERNAL_CONCEPTS_H
#define __MSGI_STL_INTERNAL_CONCEPTS_H

#include <concepts>
#include <type_traits>
#include <iterator>
#include "mstl_iterator_tags.h"
#include "mstl_iterator_traits.h"

namespace mstl {

// 基本类型合约
template <typename T>
concept Destructible = std::destructible<T>;

template <typename T, typename... Args>
concept ConstructibleFrom = std::constructible_from<T, Args...>;

template <typename T, typename U>
concept CopyConstructibleFrom = std::constructible_from<T, const U&>;

template <typename T>
concept DefaultConstructible = std::default_initializable<T>;

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

// 基本迭代器概念
template<typename I>
concept Iterator = requires(I i) {
    typename iterator_traits<I>::iterator_category;
    typename iterator_traits<I>::value_type;
    typename iterator_traits<I>::difference_type;
    typename iterator_traits<I>::pointer;
    typename iterator_traits<I>::reference;
};

// 输入迭代器概念
template<typename I>
concept InputIterator = Iterator<I> && requires(I i) {
    { *i } -> std::same_as<typename iterator_traits<I>::reference>;
    { ++i } -> std::same_as<I&>;
    { i++ } -> std::same_as<I>;
    { i != i } -> std::convertible_to<bool>;
};

// 输出迭代器概念
template<typename I, typename T>
concept OutputIterator = Iterator<I> && requires(I i, T t) {
    { *i = t } -> std::same_as<typename iterator_traits<I>::reference>;
    { ++i } -> std::same_as<I&>;
    { i++ } -> std::same_as<I>;
};

// 前向迭代器概念
template<typename I>
concept ForwardIterator = InputIterator<I> && std::is_base_of_v<forward_iterator_tag, typename iterator_traits<I>::iterator_category>;

// 双向迭代器概念
template<typename I>
concept BidirectionalIterator = ForwardIterator<I> && std::is_base_of_v<bidirectional_iterator_tag, typename iterator_traits<I>::iterator_category>;

// 随机访问迭代器概念
template<typename I>
concept RandomAccessIterator = BidirectionalIterator<I> && std::is_base_of_v<random_access_iterator_tag, typename iterator_traits<I>::iterator_category>;

// 连续迭代器概念
template<typename I>
concept ContiguousIterator = RandomAccessIterator<I> && std::is_base_of_v<contiguous_iterator_tag, typename iterator_traits<I>::iterator_category>;

// 容器相关合约
template <typename C>
concept Container = requires(C c) {
    typename C::value_type;
    typename C::reference;
    typename C::const_reference;
    typename C::iterator;
    typename C::const_iterator;
    typename C::difference_type;
    typename C::size_type;
    { c.begin() } -> std::same_as<typename C::iterator>;
    { c.end() } -> std::same_as<typename C::iterator>;
    { c.size() } -> std::same_as<typename C::size_type>;
    { c.max_size() } -> std::same_as<typename C::size_type>;
    { c.empty() } -> std::convertible_to<bool>;
};

// 分配器相关合约
template <typename T>
concept AllocatableType = std::is_object_v<T> && !std::is_array_v<T>;

template <typename A>
concept AllocatorBase = requires {
    typename A::value_type;
    typename A::size_type;
    typename A::difference_type;
    typename A::pointer;
    typename A::const_pointer;
    typename A::reference;
    typename A::const_reference;
};

template <typename A, typename T = typename A::value_type>
concept StandardAllocator = AllocatorBase<A> && 
    requires(A a, typename A::size_type n, typename A::pointer p) {
    { a.allocate(n) } -> std::same_as<typename A::pointer>;
    { a.deallocate(p, n) } -> std::same_as<void>;
    { a.address(std::declval<typename A::reference>()) } -> std::same_as<typename A::pointer>;
    { a.address(std::declval<typename A::const_reference>()) } -> std::same_as<typename A::const_pointer>;
    { a.max_size() } -> std::convertible_to<typename A::size_type>;
    
    // 检查rebind成员
    typename A::template rebind<int>::other;
};

// 简化的分配器，仅需要基本操作
template <typename A, typename T>
concept SimpleAllocator = requires(A a, std::size_t n, T* p) {
    { A::allocate(n) } -> std::convertible_to<void*>;
    { A::deallocate(p, n) } -> std::same_as<void>;
};

// 函数对象合约
template <typename F, typename... Args>
concept Invocable = std::invocable<F, Args...>;

template <typename F, typename... Args>
concept Predicate = Invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, bool>;

template <typename F, typename T>
concept UnaryPredicate = Predicate<F, T>;

template <typename F, typename T, typename U>
concept BinaryPredicate = Predicate<F, T, U>;

template <typename F, typename T, typename U>
concept Compare = BinaryPredicate<F, T, U> && BinaryPredicate<F, U, T>;

} // namespace mstl

#endif // __MSGI_STL_INTERNAL_CONCEPTS_H 