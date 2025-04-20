#ifndef __MSGI_STL_INTERNAL_CONCEPTS_H
#define __MSGI_STL_INTERNAL_CONCEPTS_H

#include <concepts>
#include <iterator>
#include <type_traits>
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
template <typename I>
concept Iterator = requires(I i) {
    typename IteratorTraits<I>::IteratorCategory;
    typename IteratorTraits<I>::ValueType;
    typename IteratorTraits<I>::DifferenceType;
    typename IteratorTraits<I>::Pointer;
    typename IteratorTraits<I>::Reference;
};

// 输入迭代器概念
template <typename I>
concept InputIterator = Iterator<I> && requires(I i) {
    { *i } -> std::same_as<typename IteratorTraits<I>::Reference>;
    { ++i } -> std::same_as<I&>;
    { i++ } -> std::same_as<I>;
    { i != i } -> std::convertible_to<bool>;
};

// 输出迭代器概念
template <typename I, typename T>
concept OutputIterator = Iterator<I> && requires(I i, T t) {
    { *i = t } -> std::same_as<typename IteratorTraits<I>::Reference>;
    { ++i } -> std::same_as<I&>;
    { i++ } -> std::same_as<I>;
};

// 前向迭代器概念
template <typename I>
concept ForwardIterator =
    InputIterator<I> &&
    std::is_base_of_v<ForwardIteratorTag, typename IteratorTraits<I>::IteratorCategory>;

// 双向迭代器概念
template <typename I>
concept BidirectionalIterator =
    ForwardIterator<I> &&
    std::is_base_of_v<BidirectionalIteratorTag, typename IteratorTraits<I>::IteratorCategory>;

// 随机访问迭代器概念
template <typename I>
concept RandomAccessIterator =
    BidirectionalIterator<I> &&
    std::is_base_of_v<RandomAccessIteratorTag, typename IteratorTraits<I>::IteratorCategory>;

// 连续迭代器概念
template <typename I>
concept ContiguousIterator =
    RandomAccessIterator<I> &&
    std::is_base_of_v<ContiguousIteratorTag, typename IteratorTraits<I>::IteratorCategory>;

// 容器相关合约
template <typename C>
concept Container = requires(C c) {
    typename C::ValueType;
    typename C::Reference;
    typename C::ConstReference;
    typename C::Iterator;
    typename C::ConstIterator;
    typename C::DifferenceType;
    typename C::SizeType;
    { c.begin() } -> std::same_as<typename C::Iterator>;
    { c.end() } -> std::same_as<typename C::Iterator>;
    { c.size() } -> std::same_as<typename C::SizeType>;
    { c.max_size() } -> std::same_as<typename C::SizeType>;
    { c.empty() } -> std::convertible_to<bool>;
};

// 分配器相关合约
template <typename T>
concept AllocatableType = std::is_object_v<T> && !std::is_array_v<T>;

template <typename A>
concept AllocatorBase = requires {
    typename A::ValueType;
    typename A::SizeType;
    typename A::DifferenceType;
    typename A::Pointer;
    typename A::ConstPointer;
    typename A::Reference;
    typename A::ConstReference;
};

template <typename A, typename T = typename A::ValueType>
concept StandardAllocator =
    AllocatorBase<A> && requires(A a, typename A::SizeType n, typename A::Pointer p) {
        { a.allocate(n) } -> std::same_as<typename A::Pointer>;
        { a.deallocate(p, n) } -> std::same_as<void>;
        { a.address(std::declval<typename A::Reference>()) } -> std::same_as<typename A::Pointer>;
        {
            a.address(std::declval<typename A::ConstReference>())
        } -> std::same_as<typename A::ConstPointer>;
        { a.max_size() } -> std::convertible_to<typename A::SizeType>;

        // 检查rebind成员
        typename A::template rebind<int>::other;
    };

// 简化的分配器，仅需要基本操作
template <typename A, typename T>
concept SimpleAllocator = requires(A a, typename A::SizeType n, typename A::Pointer p) {
    { A::allocate(n) } -> std::convertible_to<typename A::Pointer>;
    { A::deallocate(p, n) } -> std::same_as<void>;
};

// 函数对象合约
template <typename F, typename... Args>
concept Invocable = std::invocable<F, Args...>;

template <typename F, typename... Args>
concept Predicate =
    Invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, bool>;

template <typename F, typename T>
concept UnaryPredicate = Predicate<F, T>;

template <typename F, typename T, typename U>
concept BinaryPredicate = Predicate<F, T, U>;

template <typename F, typename T, typename U>
concept Compare = BinaryPredicate<F, T, U> && BinaryPredicate<F, U, T>;

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_CONCEPTS_H