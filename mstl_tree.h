#ifndef __MSGI_STL_INTERNAL_TREE_H
#define __MSGI_STL_INTERNAL_TREE_H


#include <cstddef>
#include "mstl_iterator_tags.h"
#include "mstl_alloc.h"
#include "mstl_construct.h"

namespace mstl {

using RbTreeColorType = bool;

const RbTreeColorType __rb_tree_red = false;   // 红色为0
const RbTreeColorType __rb_tree_black = true;  // 黑色为1

struct RbTreeNodeBase {
    using ColorType = RbTreeColorType;
    using BasePtr = RbTreeNodeBase*;

    ColorType color;  // 节点颜色
    BasePtr parent;   // RB树的许多操作， 必须知道父节点
    BasePtr left;     // 指向左节点
    BasePtr right;    // 指向右节点

    static BasePtr minimum(BasePtr x) {
        while (x->left != 0)
            x = x->left;
        return x;
    }

    static BasePtr maximum(BasePtr x) {
        while (x->right != 0)
            x = x->right;
        return x;
    }
};

template <typename Value>
struct RbTreeNode : public RbTreeNodeBase {
    using LinkType= RbTreeNode<Value>*;
    using ValueType = Value;
    ValueType value_field;  // 节点值
};

struct RbTreeBaseIterator {
    using BasePtr = RbTreeNodeBase::BasePtr;
    using IteratorCategory = BidirectionalIteratorTag;
    using DifferenceType = ptrdiff_t;

    BasePtr node;

    void increment() {
        if (node->right != 0) {
            node = node->right;
            while (node->left != 0) {
                node = node->left;
            }
        }
        else {
            BasePtr y = node->parent;
            // 如果现行节点本身是个右子节点
            // 一直上溯，直到不为右节点为止
            while (node == y->right) {
                node = y;
                y = y->parent;
            }

            // 若此时的右子节点不等于此时的父节点
            // 此时的父节点即为节点
            // 否则此时的node为解答
            if (node->right != y) {
                node = y;
            }
        }
        // 注意，以上判断 “若此时的右子节点不等于此时的父节点”，是为了应付一种
        // 特殊情况：我们欲寻找根节点的下一节点，而恰跟节点无右子节点
        // 当然，以上特殊做法必须配合 RB-tree 根节点与特殊 header 之间的
        // 特殊关系
    }

    void decrement() {
        if (node->color == __rb_tree_red 
            && node->parent->parent == node) {
            node = node->right;
            // 以上情况发生于node为header时(即node为end()时)
            // 注意， header 之右子节点即mostright，指向整棵树的max节点
        }
        else if (node->left != 0) {
            BasePtr y = node->left;
            while (y->right != 0) {
                y = y->right;
            }
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
};

template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc = alloc>
class RbTree {
protected:
    using VoidPointer = void*;
    using BasePtr = RbTreeNodeBase*;
    using RbTreeNode = RbTreeNode<Value>;
    using RbTreeNodeAllocator = SimpleAlloc<RbTreeNode, Alloc>;
    using ColorType = RbTreeColorType;

public:
    using KeyType = Key;
    using ValueType = Value;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using LinkType = RbTreeNode*;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;

    LinkType get_node() { return RbTreeNodeAllocator::allocate(); }
    
    void put_node(LinkType p) {
        RbTreeBaseIterator::deallocate(p);
    }

    LinkType create_node(const ValueType& x) {
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
        //复制一个节点的值和颜色
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
protected:
    // RB-tree 只以三笔数据表现
    SizeType node_count;  // 追踪记录树的大小
    LinkType header;      // 这是实现上的一个技巧
    Compare key_compare;  // 节点间的键值大小比较准则，应该会是个function_object

    LinkType& root() const { return reinterpret_cast<LinkType&>(header->parent); }
    LinkType& leftmost() const { return reinterpret_cast<LinkType&>(header->left); }
    LinkType& rightmost() const { return reinterpret_cast<LinkType&>(header->right); }

    static LinkType& left(LinkType x) {
        return reinterpret_cast<LinkType&>(x->left);
    }

    static LinkType& right(LinkType x) {
        return reinterpret_cast<LinkType&>(x->right);
    }

    static LinkType& parent(LinkType x) {
        return reinterpret_cast<LinkType&>(x->parent);
    }

    static Reference value(LinkType x) {
        return x->value_field;
    }

    static const Key& key(LinkType x) {
        return KeyOfValue()(value(x));
    }

    static ColorType& color(LinkType x) {
        return reinterpret_cast<ColorType&>(x->color);
    }

    static LinkType minimum(LinkType x) {
        return reinterpret_cast<LinkType>(RbTreeNodeBase::minimum(x));
    }

    static LinkType maximum(LinkType x) {
        return reinterpret_cast<LinkType>(RbTreeNodeBase::maximum(x));
    }

public:
    using Iterator = RbTreeIterator<ValueType, Reference, Pointer>;

private:
    Iterator __insert(BasePtr x, BasePtr y, const ValueType& v);
    LinkType __copy(LinkType x, LinkType p);
    void __erase(LinkType x);

    void init() {
        header = get_node();  // 产生一个节点空间， 令header指向它
        color(header) = __rb_tree_red; // 令header为红色， 用来区分header
                                    // 和root，在iterator.operator++之中

        root() = 0;
        leftmost() = header;  // 令header的左子节点为自己
        rightmost() = header; // 令header的右子节点为自己
    }

public:
    RbTree(const Compare& comp = Compare())
    : node_count(0), key_compare(comp) {
        init();
    }

    ~RbTree() {
        clear();
        put_node(header);
    }

    RbTree<Key, Value, KeyOfValue, Compare, Alloc>& 
    operator=(const RbTree<Key, Value, KeyOfValue, Compare, Alloc>& x);

public:
    // accessors
    Compare key_comp() const { return key_compare; }
    
    Iterator begin() { return leftmost(); }
    Iterator end() { return header; }
    bool empty() const { return node_count == 0; }
    SizeType size() const { return node_count; }
    SizeType max_size() const { return SizeType(-1); }

public:
    std::pair<Iterator, bool> insert_unique(const ValueType& x);
    Iterator insert_equal(const ValueType& x);
};

template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc>
typename RbTree<Key, Value, KeyOfValue, Compare, Alloc>::Iterator
RbTree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const ValueType& v) {
    LinkType y = header;
    LinkType x = root();
    while (x != 0) {
        y = x;
        x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
        // 以上，遇大则往左， 遇小或等于则往右
    }
    return __insert(x, y, v);
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc>
std::pair<typename RbTree<Key, Value, KeyOfValue, Compare, Alloc>::Iterator,
 bool> RbTree<Key, Value, KeyOfValue, Compare, Alloc>::insert_unique(const Value& v) {
    LinkType y = header;
    LinkType x = root();
    bool comp = true;
    while (x != 0) {
        y = x;
        comp = key_compare(KeyOfValue()(v), key(x));
        x = comp ? left(x) : right(x);
    }

    Iterator j = Iterator(y);
    if (comp)
        if (j == begin()) {
            return std::pair<Iterator, bool>(__insert(x, y, v), true);
        } else {
            // 插入点的父节点不为最左节点
            // 调整j，回头准备测试
            --j;
        }
        if (key_compare(key(j.node), KeyOfValue()(v))) {
            // 小于新值 插入到右边
            return std::pair<Iterator, bool>(__insert(x, y, v), true);
        }

    // 新值一定和树中的键重复， 不该插入该值
    return std::pair<Iterator, bool>(j, false);
 }

}  // namespace mstl

#endif //__MSGI_STL_INTERNAL_TREE_H 
