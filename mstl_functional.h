#ifndef __MSGI_STL_INTERNAL_FUNCTIONAL_H
#define __MSGI_STL_INTERNAL_FUNCTIONAL_H

namespace mstl {

// 基础函数对象
template <typename T>
struct Less {
    using FirstArgumentType = T;
    using SecondArgumentType = T;
    using ResultType = bool;

    bool operator()(const T& x, const T& y) const {
        return x < y;
    }
};

template <typename T>
struct Greater {
    using FirstArgumentType = T;
    using SecondArgumentType = T;
    using ResultType = bool;

    bool operator()(const T& x, const T& y) const {
        return x > y;
    }
};


template <typename T>
struct Identity {
    using ArgumentType = T;
    using ResultType = T;

    const T& operator()(const T& x) const {
        return x;
    }
};

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_FUNCTIONAL_H