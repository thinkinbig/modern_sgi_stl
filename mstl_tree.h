#ifndef __MSGI_STL_INTERNAL_TREE_H
#define __MSGI_STL_INTERNAL_TREE_H


#include <cstddef>
#include "mstl_iterator_tags.h"
#include "mstl_alloc.h"

namespace mstl {

using __rb_tree_color_type = bool;

const __rb_tree_color_type __rb_tree_red = false;   // 红色为0
const __rb_tree_color_type __rb_tree_black = true;  // 黑色为1

struct RbTreeNodeBase {
    using ColorType = __rb_tree_color_type;
    using BasePtr = RbTreeNodeBase*;

    ColorType color;  // 节点颜色
    BasePtr parent;   // RB树的许多操作， 必须知道父节点
    BasePtr left;     // 指向左节点
    BasePtr right;    // 指向右节点

    static BasePtr minimun(BasePtr x) {
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
    using link_type = RbTreeNode<Value>*;
    Value value_field;  // 节点值
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
    
};

}  // namespace mstl

#endif
