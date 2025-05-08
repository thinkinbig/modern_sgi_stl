#ifndef __MSGI_STL_INTERNAL_TREE_H
#define __MSGI_STL_INTERNAL_TREE_H

#include <cstddef>
#include "mstl_iterator_tags.h"
#include "mstl_alloc.h"
#include "mstl_construct.h"
#include "mstl_pair.h"
#include "mstl_iterator.h"

namespace mstl {

using RbTreeColorType = bool;

const RbTreeColorType __rb_tree_red = false;
const RbTreeColorType __rb_tree_black = true;

struct RbTreeNodeBase {
    using ColorType = RbTreeColorType;
    using BasePtr = RbTreeNodeBase*;

    ColorType color;
    BasePtr parent;
    BasePtr left;
    BasePtr right;

    static BasePtr minimum(BasePtr x) noexcept {
        while (x->left != 0) x = x->left;
        return x;
    }

    static BasePtr maximum(BasePtr x) noexcept {
        while (x->right != 0) x = x->right;
        return x;
    }

    BasePtr base_ptr() const noexcept {
        return const_cast<BasePtr>(this);
    }
};

template <typename Value>
struct RbTreeNode : public RbTreeNodeBase {
    using LinkType = RbTreeNode<Value>*;
    using ValueType = Value;
    ValueType value_field;
};

struct RbTreeBaseIterator {
    using BasePtr = RbTreeNodeBase::BasePtr;
    using IteratorCategory = BidirectionalIteratorTag;
    using DifferenceType = ptrdiff_t;

    BasePtr node;

    void increment() {
        if (node->right != 0) {
            node = node->right;
            while (node->left != 0)
                node = node->left;
        }
        else {
            BasePtr y = node->parent;
            while (node == y->right) {
                node = y;
                y = y->parent;
            }
            if (node->right != y)
                node = y;
        }
    }

    void decrement() {
        if (node->color == __rb_tree_red && node->parent->parent == node)
            node = node->right;
        else if (node->left != 0) {
            BasePtr y = node->left;
            while (y->right != 0)
                y = y->right;
            node = y;
        }
        else {
            BasePtr y = node->parent;
            while (node == y->left) {
                node = y;
                y = y->parent;
            }
            node = y;
        }
    }
};

template <typename Value, typename Ref, typename Ptr>
struct RbTreeIterator : public RbTreeBaseIterator {
    using ValueType = Value;
    using Reference = Ref;
    using Pointer = Ptr;
    using Iterator = RbTreeIterator<Value, Value&, Value*>;
    using ConstIterator = RbTreeIterator<Value, const Value&, const Value*>;
    using Self = RbTreeIterator<Value, Ref, Ptr>;
    using LinkType = RbTreeNode<Value>*;

    RbTreeIterator() {}
    RbTreeIterator(LinkType x) { node = x; }
    RbTreeIterator(const Iterator& it) { node = it.node; }

    Reference operator*() const { return LinkType(node)->value_field; }
    Pointer operator->() const { return &(operator*()); }

    Self& operator++() { 
        increment(); 
        return *this; 
    }
    
    Self operator++(int) {
        Self tmp = *this;
        increment();
        return tmp;
    }

    Self& operator--() {
        decrement();
        return *this;
    }

    Self operator--(int) {
        Self tmp = *this;
        decrement();
        return tmp;
    }

    bool operator!=(const Self& x) const { return node != x.node; }
    bool operator==(const Self& x) const { return node == x.node; }
};

template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc = SimpleAlloc<Value, MallocAllocTemplate<0>>>
class RbTree {
protected:
    using BasePtr = RbTreeNodeBase::BasePtr;
    using LinkType = RbTreeNode<Value>*;
    using RbTreeNodeAllocator = SimpleAlloc<RbTreeNode<Value>, MallocAllocTemplate<0>>;
    using SizeType = size_t;
    using Reference = Value&;
    using ColorType = RbTreeNodeBase::ColorType;

    LinkType header;
    SizeType node_count;
    Compare key_compare;

    LinkType& root() const { return reinterpret_cast<LinkType&>(header->parent); }
    LinkType& leftmost() const { return reinterpret_cast<LinkType&>(header->left); }
    LinkType& rightmost() const { return reinterpret_cast<LinkType&>(header->right); }

    static LinkType& left(LinkType x) { return reinterpret_cast<LinkType&>(x->left); }
    static LinkType& right(LinkType x) { return reinterpret_cast<LinkType&>(x->right); }
    static LinkType& parent(LinkType x) { return reinterpret_cast<LinkType&>(x->parent); }
    static Reference value(LinkType x) { return x->value_field; }
    static const Key& key(LinkType x) { 
        static Key k = KeyOfValue()(value(x));
        k = KeyOfValue()(value(x));
        return k;
    }
    static ColorType& color(LinkType x) { return x->color; }
    static LinkType minimum(LinkType x) { return reinterpret_cast<LinkType>(RbTreeNodeBase::minimum(x)); }
    static LinkType maximum(LinkType x) { return reinterpret_cast<LinkType>(RbTreeNodeBase::maximum(x)); }

    LinkType get_node() { return RbTreeNodeAllocator::allocate(); }
    void put_node(LinkType p) { RbTreeNodeAllocator::deallocate(p); }

    LinkType create_node(const Value& x) {
        LinkType tmp = get_node();
        try {
            construct(&tmp->value_field, x);
        } catch(...) {
            put_node(tmp);
            throw;
        }
        return tmp;
    }

    LinkType clone_node(LinkType x) {
        LinkType tmp = create_node(x->value_field);
        tmp->color = x->color;
        tmp->left = 0;
        tmp->right = 0;
        return tmp;
    }

    void destroy_node(LinkType p) {
        destroy(&p->value_field);
        put_node(p);
    }

    void init() {
        header = get_node();
        color(header) = __rb_tree_red;
        root() = 0;
        leftmost() = header;
        rightmost() = header;
    }

    void __rb_tree_rebalance(LinkType x, BasePtr header) {
        x->color = __rb_tree_red;
        while (x != root() && color(parent(x)) == __rb_tree_red) {
            if (parent(x) == left(parent(parent(x)))) {
                LinkType y = right(parent(parent(x)));
                if (y && color(y) == __rb_tree_red) {
                    color(parent(x)) = __rb_tree_black;
                    color(y) = __rb_tree_black;
                    color(parent(parent(x))) = __rb_tree_red;
                    x = parent(parent(x));
                }
                else {
                    if (x == right(parent(x))) {
                        x = parent(x);
                        __rb_tree_rotate_left(x, header->parent);
                    }
                    color(parent(x)) = __rb_tree_black;
                    color(parent(parent(x))) = __rb_tree_red;
                    __rb_tree_rotate_right(parent(parent(x)), header->parent);
                }
            }
            else {
                LinkType y = left(parent(parent(x)));
                if (y && color(y) == __rb_tree_red) {
                    color(parent(x)) = __rb_tree_black;
                    color(y) = __rb_tree_black;
                    color(parent(parent(x))) = __rb_tree_red;
                    x = parent(parent(x));
                }
                else {
                    if (x == left(parent(x))) {
                        x = parent(x);
                        __rb_tree_rotate_right(x, header->parent);
                    }
                    color(parent(x)) = __rb_tree_black;
                    color(parent(parent(x))) = __rb_tree_red;
                    __rb_tree_rotate_left(parent(parent(x)), header->parent);
                }
            }
        }
        color(root()) = __rb_tree_black;
    }

    void __rb_tree_rotate_left(BasePtr x, BasePtr& root) {
        BasePtr y = x->right;
        x->right = y->left;
        if (y->left != 0)
            y->left->parent = x;
        y->parent = x->parent;
        if (x == root)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void __rb_tree_rotate_right(BasePtr x, BasePtr& root) {
        BasePtr y = x->left;
        x->left = y->right;
        if (y->right != 0)
            y->right->parent = x;
        y->parent = x->parent;
        if (x == root)
            root = y;
        else if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    LinkType __copy(LinkType x, LinkType p) {
        LinkType top = clone_node(x);
        top->parent = p;
        
        try {
            if (x->right != 0)
                top->right = __copy(static_cast<LinkType>(x->right), top);
            p = top;
            x = static_cast<LinkType>(x->left);
            
            while (x != 0) {
                LinkType y = clone_node(x);
                p->left = y;
                y->parent = p;
                if (x->right != 0)
                    y->right = __copy(static_cast<LinkType>(x->right), y);
                p = y;
                x = static_cast<LinkType>(x->left);
            }
        } catch(...) {
            __erase(top);
            throw;
        }
        return top;
    }

    void __erase(LinkType x) {
        while (x != 0) {
            __erase(static_cast<LinkType>(x->right));
            LinkType y = static_cast<LinkType>(x->left);
            destroy_node(x);
            x = y;
        }
    }

    BasePtr __rb_tree_rebalance_for_erase(BasePtr z, BasePtr& root) {
        BasePtr y = z;
        BasePtr x = 0;
        BasePtr x_parent = 0;

        if (y->left == 0)
            x = y->right;
        else if (y->right == 0)
            x = y->left;
        else {
            y = y->right;
            while (y->left != 0)
                y = y->left;
            x = y->right;
        }

        if (y != z) {
            z->left->parent = y;
            y->left = z->left;
            if (y != z->right) {
                x_parent = y->parent;
                if (x)
                    x->parent = y->parent;
                y->parent->left = x;
                y->right = z->right;
                z->right->parent = y;
            }
            else
                x_parent = y;

            if (root == z)
                root = y;
            else if (z->parent->left == z)
                z->parent->left = y;
            else
                z->parent->right = y;
            y->parent = z->parent;
            std::swap(y->color, z->color);
            y = z;
        }
        else {
            x_parent = y->parent;
            if (x)
                x->parent = y->parent;
            if (root == z)
                root = x;
            else if (z->parent->left == z)
                z->parent->left = x;
            else
                z->parent->right = x;
            if (leftmost() == z) {
                if (z->right == 0)
                    leftmost() = static_cast<LinkType>(z->parent);
                else
                    leftmost() = minimum(static_cast<LinkType>(x));
            }
            if (rightmost() == z) {
                if (z->left == 0)
                    rightmost() = static_cast<LinkType>(z->parent);
                else
                    rightmost() = maximum(static_cast<LinkType>(x));
            }
        }

        if (y->color != __rb_tree_red) {
            while (x != root && (x == 0 || x->color == __rb_tree_black)) {
                if (x == x_parent->left) {
                    BasePtr w = x_parent->right;
                    if (w->color == __rb_tree_red) {
                        w->color = __rb_tree_black;
                        x_parent->color = __rb_tree_red;
                        __rb_tree_rotate_left(x_parent, root);
                        w = x_parent->right;
                    }
                    if ((w->left == 0 || w->left->color == __rb_tree_black) &&
                        (w->right == 0 || w->right->color == __rb_tree_black)) {
                        w->color = __rb_tree_red;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    }
                    else {
                        if (w->right == 0 || w->right->color == __rb_tree_black) {
                            w->left->color = __rb_tree_black;
                            w->color = __rb_tree_red;
                            __rb_tree_rotate_right(w, root);
                            w = x_parent->right;
                        }
                        w->color = x_parent->color;
                        x_parent->color = __rb_tree_black;
                        if (w->right)
                            w->right->color = __rb_tree_black;
                        __rb_tree_rotate_left(x_parent, root);
                        break;
                    }
                }
                else {
                    BasePtr w = x_parent->left;
                    if (w->color == __rb_tree_red) {
                        w->color = __rb_tree_black;
                        x_parent->color = __rb_tree_red;
                        __rb_tree_rotate_right(x_parent, root);
                        w = x_parent->left;
                    }
                    if ((w->right == 0 || w->right->color == __rb_tree_black) &&
                        (w->left == 0 || w->left->color == __rb_tree_black)) {
                        w->color = __rb_tree_red;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    }
                    else {
                        if (w->left == 0 || w->left->color == __rb_tree_black) {
                            w->right->color = __rb_tree_black;
                            w->color = __rb_tree_red;
                            __rb_tree_rotate_left(w, root);
                            w = x_parent->left;
                        }
                        w->color = x_parent->color;
                        x_parent->color = __rb_tree_black;
                        if (w->left)
                            w->left->color = __rb_tree_black;
                        __rb_tree_rotate_right(x_parent, root);
                        break;
                    }
                }
            }
            if (x)
                x->color = __rb_tree_black;
        }
        return y;
    }

public:
    using KeyType = Key;
    using ValueType = Value;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using ConstReference = const ValueType&;
    using DifferenceType = ptrdiff_t;
    using Iterator = RbTreeIterator<Value, Reference, Pointer>;
    using ConstIterator = RbTreeIterator<Value, ConstReference, ConstPointer>;
    using ReverseIterator = ReverseIterator<Iterator>;
    using ConstReverseIterator = ReverseIterator<ConstIterator>;

    RbTree(const Compare& comp = Compare())
    : node_count(0), key_compare(comp) {
        init();
    }

    RbTree(const RbTree& x) : node_count(0), key_compare(x.key_compare) {
        init();
        if (x.root() != 0) {
            try {
                root() = __copy(x.root(), header);
                leftmost() = minimum(root());
                rightmost() = maximum(root());
                node_count = x.node_count;
            } catch(...) {
                clear();
                put_node(header);
                throw;
            }
        }
    }

    RbTree& operator=(const RbTree& x) {
        if (this != &x) {
            clear();
            node_count = 0;
            key_compare = x.key_compare;
            
            try {
                if (x.root() != 0) {
                    root() = __copy(x.root(), header);
                    leftmost() = minimum(root());
                    rightmost() = maximum(root());
                    node_count = x.node_count;
                }
            } catch(...) {
                clear();
                throw;
            }
        }
        return *this;
    }

    ~RbTree() {
        clear();
        if (header) {
            put_node(header);
            header = 0;
        }
    }

    Compare key_comp() const { return key_compare; }
    Iterator begin() { return leftmost(); }
    Iterator end() { return header; }
    ConstIterator begin() const { return leftmost(); }
    ConstIterator end() const { return header; }

    ReverseIterator rbegin() { return ReverseIterator(end()); }
    ReverseIterator rend() { return ReverseIterator(begin()); }
    ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
    ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }
    ConstReverseIterator crbegin() const { return ConstReverseIterator(end()); }
    ConstReverseIterator crend() const { return ConstReverseIterator(begin()); }

    bool empty() const { return node_count == 0; }
    SizeType size() const { return node_count; }
    SizeType max_size() const { return SizeType(-1); }

    Pair<Iterator, bool> insert_unique(const Value& v) {
        LinkType y = header;
        LinkType x = root();
        bool comp = true;
        while (x != 0) {
            y = x;
            comp = key_compare(KeyOfValue()(v), key(x));
            x = comp ? left(x) : right(x);
        }

        Iterator j = Iterator(y);
        if (comp) {
            if (j == begin())
                return Pair<Iterator, bool>(__insert(x, y, v), true);
            else
                --j;
        }
        if (key_compare(key(static_cast<LinkType>(j.node)), KeyOfValue()(v)))
            return Pair<Iterator, bool>(__insert(x, y, v), true);
        return Pair<Iterator, bool>(j, false);
    }

    template <typename InputIterator>
    void insert_unique(InputIterator first, InputIterator last) {
        for (; first != last; ++first)
            insert_unique(*first);
    }

    Iterator insert_equal(const ValueType& v) {
        LinkType y = header;
        LinkType x = root();
        while (x != 0) {
            y = x;
            x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
        }
        return __insert(x, y, v);
    }

    void clear() {
        if (node_count != 0) {
            __erase(root());
            root() = 0;
            leftmost() = header;
            rightmost() = header;
            node_count = 0;
        }
    }

    Iterator find(const Key& k) {
        LinkType x = root();

        while (x != 0) {
            if (key_compare(key(x), k)) {
                x = right(x);
            } else if (key_compare(k, key(x))) {
                x = left(x);
            } else {
                return Iterator(x);
            }
        }
        return end();
    }

    SizeType count(const Key& k) const {
        Pair<ConstIterator, ConstIterator> p = equal_range(k);
        SizeType n = 0;
        ConstIterator first = p.first;
        while (first != p.second) {
            ++n;
            ++first;
        }
        return n;
    }

    Pair<Iterator, Iterator> equal_range(const Key& k) {
        return Pair<Iterator, Iterator>(lower_bound(k), upper_bound(k));
    }

    Iterator lower_bound(const Key& k) {
        LinkType y = header;
        LinkType x = root();

        while (x != 0) {
            if (!key_compare(key(x), k)) {
                y = x;
                x = left(x);
            }
            else
                x = right(x);
        }
        return Iterator(y);
    }

    Iterator upper_bound(const Key& k) {
        LinkType y = header;
        LinkType x = root();

        while (x != 0) {
            if (key_compare(k, key(x))) {
                y = x;
                x = left(x);
            }
            else
                x = right(x);
        }
        return Iterator(y);
    }

    void erase(Iterator position) {
        BasePtr y = __rb_tree_rebalance_for_erase(position.node, header->parent);
        destroy_node(static_cast<LinkType>(y));
        --node_count;
    }

    SizeType erase(const Key& x) {
        Pair<Iterator, Iterator> p = equal_range(x);
        SizeType n = mstl::distance(p.first, p.second);
        erase(p.first, p.second);
        return n;
    }

    void erase(Iterator first, Iterator last) {
        if (first == begin() && last == end())
            clear();
        else {
            while (first != last)
                erase(first++);
        }
    }


    // 正确的模板友元声明，允许 operator<< 访问 protected 成员
    template <typename K, typename V, typename KoV, typename C, typename A>
    friend std::ostream& operator<<(std::ostream&, const RbTree<K, V, KoV, C, A>&);

private:
    Iterator __insert(BasePtr x_, BasePtr y_, const Value& v) {
        LinkType x = static_cast<LinkType>(x_);
        LinkType y = static_cast<LinkType>(y_);
        LinkType z;

        z = create_node(v);
        if (y == header || x != 0 || key_compare(KeyOfValue()(v), key(y))) {
            left(y) = z;
            if (y == header) {
                root() = z;
                rightmost() = z;
            }
            else if (y == leftmost())
                leftmost() = z;
        }
        else {
            right(y) = z;
            if (y == rightmost())
                rightmost() = z;
        }

        parent(z) = y;
        left(z) = 0;
        right(z) = 0;
        __rb_tree_rebalance(z, header);
        ++node_count;
        return Iterator(z);
    }
};

namespace detail {
template <typename Value>
void print_tree_impl(std::ostream& os, RbTreeNode<Value>* node, RbTreeNode<Value>* header, int depth = 0, char branch = ' ') {
    if (!node || node == header) return;
    print_tree_impl<Value>(os, static_cast<RbTreeNode<Value>*>(node->right), header, depth + 1, '/');
    for (int i = 0; i < depth; ++i) os << "    ";
    os << branch << "--" << node->value_field << (node->color == __rb_tree_red ? " (R)" : " (B)") << std::endl;
    print_tree_impl<Value>(os, static_cast<RbTreeNode<Value>*>(node->left), header, depth + 1, '\\');
}
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc>
std::ostream& operator<<(std::ostream& os, const RbTree<Key, Value, KeyOfValue, Compare, Alloc>& tree) {
    using NodePtr = RbTreeNode<Value>*;
    if (tree.root() == nullptr || tree.header == tree.root()) {
        os << "<empty tree>" << std::endl;
        return os;
    }
    detail::print_tree_impl<Value>(os, static_cast<NodePtr>(tree.root()), static_cast<NodePtr>(tree.header));
    return os;
}

}  // namespace mstl

#endif  // __MSGI_STL_INTERNAL_TREE_H