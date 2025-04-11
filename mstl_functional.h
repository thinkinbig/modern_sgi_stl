#ifndef __MSGI_STL_INTERNAL_FUNCTIONAL_H
#define __MSGI_STL_INTERNAL_FUNCTIONAL_H

namespace mstl {

// 基础函数对象
template <typename T>
struct less {
    using first_argument_type = T;
    using second_argument_type = T;
    using result_type = bool;

    bool operator()(const T& x, const T& y) const {
        return x < y;
    }
};

template <typename T>
struct greater {
    using first_argument_type = T;
    using second_argument_type = T;
    using result_type = bool;

    bool operator()(const T& x, const T& y) const {
        return x > y;
    }
};

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_FUNCTIONAL_H