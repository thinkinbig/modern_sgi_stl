#ifndef __MSGI_STL_INTERNAL_SET_H
#define __MSGI_STL_INTERNAL_SET_H

#include "mstl_functional.h"
#include "mstl_alloc.h"
#include "mstl_tree.h"

namespace mstl {
    template <typename Key, typename Compare = Less<Key>, typename Alloc = alloc>
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

        using SizeType = typename RepType::SizeType;
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
        
    };
}

#endif // __MSGI_STL_INTERNAL_SET_H