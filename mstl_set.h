#ifndef __MSGI_STL_INTERNAL_SET_H
#define __MSGI_STL_INTERNAL_SET_H

#include "mstl_functional.h"
#include "mstl_alloc.h"
#include "mstl_tree.h"
#include "mstl_pair.h"

#include <concepts>
namespace mstl {
    template <typename Key, typename Compare = Less<Key>, typename Alloc = alloc>
    requires (std::equality_comparable<Key> && std::strict_weak_order<Compare, Key, Key>)
    class Set {
    public:
        using KeyType = Key;
        using ValueType = Key;
        using KeyCompare = Compare;
        using ValueCompare = Compare;
    
    private:
        using RepType = RbTree<KeyType, ValueType, Identity<Key>, Compare, Alloc>;
        
        RepType t; // Red-Black Tree representation of the set
    public:
        using Pointer = typename RepType::ConstPointer;
        using ConstPointer = typename RepType::ConstPointer;
        using Reference = typename RepType::ConstReference;
        using ConstReference = typename RepType::ConstReference;
        // Iterator is not allowed to modify the set
        using Iterator = typename RepType::ConstIterator;
        using ConstIterator = typename RepType::ConstIterator;
        using ReverseIterator = typename RepType::ConstReverseIterator;
        using ConstReverseIterator = typename RepType::ConstReverseIterator;

        using SizeType = size_t;
        using DifferenceType = typename RepType::DifferenceType;

        Set() : t(Compare()) {}
        explicit Set(const Compare& comp) : t(comp) {}
        Set(const Set& x) : t(x.t) {}

        template <typename InputIterator>
        Set(InputIterator first, InputIterator last) : t(Compare()) {
            t.insert_unique(first, last);
        }

        ~Set() {}
        
        Set& operator=(const Set& x) {
            t = x.t;
            return *this;
        }

        Set& operator=(Set&& x) {
            t = x.t;
            return *this;
        }

        KeyCompare key_comp() const {
            return t.key_comp();
        }

        ValueCompare value_comp() const {
            return t.key_comp();
        }

        Iterator begin() const {
            return t.begin();
        }

        Iterator end() const {
            return t.end();
        }

        ReverseIterator rbegin() const {
            return t.rbegin();
        }

        ReverseIterator rend() const {
            return t.rend();
        }

        bool empty() const {
            return t.empty();
        }

        SizeType size() const {
            return t.size();
        }

        SizeType max_size() const {
            return t.max_size();
        }
        
        void swap(Set& x) {
            t.swap(x.t);
        }

        using PairIteratorBool = Pair<Iterator, bool>;

        PairIteratorBool insert(const ValueType& x) {
            return t.insert_unique(x);
        }

        Iterator insert(Iterator position, const ValueType& x) {
            return t.insert_unique(position, x);
        }

        template <typename InputIterator>
        void insert(InputIterator first, InputIterator last) {
            t.insert_unique(first, last);
        }
        

        void erase(ConstIterator position) {
            t.erase(position);
        }

        void erase(Iterator first, Iterator last) {
            t.erase(first, last);
        }

    SizeType erase(const Key& x) {
        auto p = t.equal_range(x);
        SizeType n = mstl::distance(p.first, p.second);
        t.erase(p.first, p.second);
        return n;
    }

        void clear() {
            t.clear();
        }

        Iterator find(const Key& x) {
            return t.find(x);
        }

        SizeType count(const Key& x) const {
            return t.count(x);
        }

        Iterator lower_bound(const Key& x) {
            return t.lower_bound(x);
        }

        Iterator upper_bound(const Key& x) {
            return t.upper_bound(x);
        }

        Pair<ConstIterator, ConstIterator> equal_range(const Key& x) const {
            return t.equal_range(x);
        }

        friend bool operator==(const Set& x, const Set& y) {
            return std::equal(x.begin(), x.end(), y.begin(), y.end());
        }
        friend bool operator!=(const Set& x, const Set& y) {
            return !(x == y);
        }
        friend bool operator<(const Set& x, const Set& y) {
            return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
        }
        friend bool operator>(const Set& x, const Set& y) {
            return y < x;
        }
        friend bool operator<=(const Set& x, const Set& y) {
            return !(y < x);
        }
        friend bool operator>=(const Set& x, const Set& y) {
            return !(x < y);
        }
    };
}

#endif // __MSGI_STL_INTERNAL_SET_H