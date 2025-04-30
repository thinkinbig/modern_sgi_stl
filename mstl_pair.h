#ifndef __MSGI_STL_INTERNAL_PAIR_H
#define __MSGI_STL_INTERNAL_PAIR_H

namespace mstl {

template <class T1, class T2>
struct Pair {
    using FirstType = T1;
    using SecondType = T2;

    T1 first;
    T2 second;

    // 默认构造函数
    Pair() : first(T1()), second(T2()) {}

    // 带参数的构造函数
    template <typename U1 = T1, typename U2 = T2>
    Pair(const U1& a, const U2& b) : first(a), second(b) {}

    // 复制构造函数
    Pair(const Pair& p) = default;

    // 复制构造函数模板
    template <class U1, class U2>
    Pair(const Pair<U1, U2>& p) : first(p.first), second(p.second) {}

    // 赋值操作符
    Pair& operator=(const Pair& p) {
        if (this != &p) {
            first = p.first;
            second = p.second;
        }
        return *this;
    }

    // 比较操作符
    bool operator==(const Pair& p) const {
        return first == p.first && second == p.second;
    }

    bool operator!=(const Pair& p) const {
        return !(*this == p);
    }

    bool operator<(const Pair& p) const {
        return first < p.first || (!(p.first < first) && second < p.second);
    }

    bool operator<=(const Pair& p) const {
        return !(p < *this);
    }

    bool operator>(const Pair& p) const {
        return p < *this;
    }

    bool operator>=(const Pair& p) const {
        return !(*this < p);
    }
};

// 创建pair的辅助函数
template <class T1, class T2>
inline Pair<T1, T2> make_pair(const T1& x, const T2& y) {
    return Pair<T1, T2>(x, y);
}

} // namespace mstl

#endif // __MSGI_STL_INTERNAL_PAIR_H 