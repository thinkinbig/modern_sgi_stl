#ifndef __MSGI_STL_INTERNAL_TREE_H
#define __MSGI_STL_INTERNAL_TREE_H

namespace mstl {

using __rb_tree_color_type = bool;

const __rb_tree_color_type __rb_tree_red = false;   // 红色为0
const __rb_tree_color_type __rb_tree_black = true;  // 黑色为1

struct RbTreeNodeBase {
    using color_type = __rb_tree_color_type;
    using base_ptr = RbTreeNodeBase*;

    color_type color;  // 节点颜色
    base_ptr parent;   // RB树的许多操作， 必须知道父节点
    base_ptr left;     // 指向左节点
    base_ptr right;    // 指向右节点

    static base_ptr minimun(base_ptr x) {
        while (x->left != 0)
            x = x->left;
        return x;
    }

    static base_ptr maximum(base_ptr x) {
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

}  // namespace mstl

#endif
