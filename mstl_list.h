#ifndef __MSGI_STL_INTERNAL_LIST_H
#define __MSGI_STL_INTERNAL_LIST_H

#include <cstddef>
#include "mstl_iterator_traits.h"
#include "mstl_iterator.h"
#include "mstl_alloc.h"
#include "mstl_iterator_tags.h"

namespace mstl
{
    template <typename T>
    struct list_node
    {
        list_node* next;
        list_node* prev;
        T data;
        
        list_node() : next(nullptr), prev(nullptr) {}
        list_node(const T& x) : next(nullptr), prev(nullptr), data(x) {}
    };
    
    template<typename T, typename Ref, typename Ptr>
    struct list_iterator {
        using iterator_category = bidirectional_iterator_tag;
        using value_type = T;
        using pointer = Ptr;
        using reference = Ref;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using Node = list_node<T>;

        using iterator = list_iterator<T, T&, T*>;
        using const_iterator = list_iterator<T, const T&, const T*>;
        using self = list_iterator<T, Ref, Ptr>;

        Node* node;

        list_iterator(Node* x) : node(x) {}
        list_iterator() : node(nullptr) {}
        
        // 允许从非const转换到const
        list_iterator(const self& x) : node(x.node) {}

        self& operator=(const self& x) {
            node = x.node;
            return *this;
        }

        self& operator=(self&& x) {
            node = x.node;
            x.node = nullptr;
            return *this;
        }

        ~list_iterator() {
            node = nullptr;
        }

        void incr() { node = node->next; }
        void decr() { node = node->prev; }

        reference operator*() const { return node->data; }
        pointer operator->() const { return &(operator*()); }

        self& operator++() {
            incr();
            return *this;
        }

        self operator++(int) {
            self tmp = *this;
            incr();
            return tmp;
        }

        self& operator--() {
            decr();
            return *this;
        }

        self operator--(int) {
            self tmp = *this;
            decr();
            return tmp;
        }

        bool operator==(const self& x) const {
            return node == x.node;
        }

        bool operator!=(const self& x) const {
            return node != x.node;
        }
    };

    template<typename T, class Alloc = alloc> 
    class list {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*; 
        using reference = T&;
        using const_reference = const T&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using Node = list_node<T>;

        using iterator = list_iterator<T, T&, T*>;
        using const_iterator = list_iterator<T, const T&, const T*>;

        // 构造函数
        list() { create_node(); }
        list(size_type n, const T& value = T()) {
            create_node();
            insert(begin(), n, value);
        }
        list(const list& x) {
            create_node();
            insert(begin(), x.begin(), x.end());
        }

        list(list&& x) {
            node = x.node;
            x.node = nullptr;
        }

        list& operator=(const list& x) {
            if (this != &x) {
                clear();
                insert(begin(), x.begin(), x.end());
            }
            return *this;
        }

        list& operator=(list&& x) {
            if (this != &x) {
                clear();
                node = x.node;
                x.node = nullptr;
            }
            return *this;
        }

        ~list() {
            clear();
            delete node;
        }

        // 迭代器相关
        iterator begin() { return node->next; }
        const_iterator begin() const { return node->next; }
        iterator end() { return node; }
        const_iterator end() const { return node; }
        
        // 容量相关
        bool empty() const { return node->next == node; }
        size_type size() const {
            return distance(begin(), end());
        }
        
        // 元素访问
        reference front() { return *begin(); }
        const_reference front() const { return *begin(); }
        reference back() { return *(--end()); }
        const_reference back() const { return *(--end()); }

        // 修改器
        void push_back(const T& x) { insert(end(), x); }
        void push_back(T&& x) { insert(end(), std::move(x)); }
        void push_front(const T& x) { insert(begin(), x); }
        void push_front(T&& x) { insert(begin(), std::move(x)); }
        
        void pop_back() {
            iterator tmp = --end();
            erase(tmp);
        }
        
        void pop_front() {
            erase(begin());
        }

        template<typename U>
        iterator insert(iterator position, U&& x) {
            Node* tmp = get_node();
            tmp->data = std::forward<U>(x);
            
            tmp->next = position.node;
            tmp->prev = position.node->prev;
            
            position.node->prev->next = tmp;
            position.node->prev = tmp;
            
            return iterator(tmp);
        }

        template<typename U>
        iterator insert(iterator position, size_type n, U&& x) {
            for (size_type i = 0; i < n; ++i) {
                insert(position, x);
            }
            return position;
        }

        template<typename InputIterator>
        iterator insert(iterator position, InputIterator first, InputIterator last) {
            for (; first != last; ++first) {
                insert(position, *first);
            }
            return position;
        }
        
        iterator erase(iterator position) {
            Node* next_node = position.node->next;
            Node* prev_node = position.node->prev;
            
            prev_node->next = next_node;
            next_node->prev = prev_node;
            
            put_node(position.node);
            return iterator(next_node);
        }

        iterator erase(iterator first, iterator last) {
            while (first != last) {
                first = erase(first);
            }
            return last;
        }
        
        void clear() {
            erase(begin(), end());
        }
        
        void splice(iterator position, list& x) {
            if (!x.empty()) {
                transfer(position, x.begin(), x.end());
            }
        }

        void splice(iterator position, list&& x) {
            if (!x.empty()) {
                transfer(position, x.begin(), x.end());
            }
        }

        void splice(iterator position, list& x, iterator i) {
            if (position != i) {
                transfer(position, i, i.node->next);
            }
        }

        void splice(iterator position, list&& x, iterator i) {
            if (position != i) {
                transfer(position, i, i.node->next);
            }
        }

        void splice(iterator position, list& x, iterator first, iterator last) {
            if (first != last) {
                transfer(position, first, last);
            }
        }

        void splice(iterator position, list&& x, iterator first, iterator last) {
            if (first != last) {
                transfer(position, first, last);
            }
        }

        void remove(const T& value) {
            iterator current = begin();
            while (current != end()) {
                if (*current == value) {
                    current = erase(current);
                }
                else {
                    ++current;
                }
            }
        }

    protected:
        Node* node;
    
        void create_node() {
            node = new Node;
            node->next = node;
            node->prev = node;
        }

        // 辅助函数
        void transfer(iterator position, iterator first, iterator last) {
            if (position != first && position != last) {
                Node* first_node = first.node;
                Node* last_node = last.node->prev;
                
                first_node->prev->next = last.node;
                last.node->prev = first_node->prev;
                
                first_node->prev = position.node->prev;
                last_node->next = position.node;
                
                position.node->prev->next = first_node;
                position.node->prev = last_node;
            }
        }

        void put_node(Node* p) { delete p; }
        Node* get_node() { return new Node; }
    };
}
#endif