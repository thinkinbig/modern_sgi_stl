#include "mstl_construct.h"
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <cstdlib>
using namespace std;

// 测试用的类
class TestClass {
public:
    TestClass(int x) : value(x) {
        cout << "构造: " << value << endl;
    }

    ~TestClass() {
        cout << "析构: " << value << endl;
    }

    TestClass() : value(0) {
        cout << "默认构造: " << value << endl;
    }

    int getValue() const { return value; }
private:
    int value;
};

// 测试单个对象的构造和析构
void test_single_object() {
    cout << "\n=== 测试单个对象 ===" << endl;
    
    // 分配内存
    void* p = ::operator new(sizeof(TestClass));
    TestClass* ptr = static_cast<TestClass*>(p);
    
    // 构造对象
    cout << "构造对象..." << endl;
    mstl::construct(ptr, 42);
    
    // 验证对象
    cout << "对象值: " << ptr->getValue() << endl;
    
    // 析构对象
    cout << "析构对象..." << endl;
    mstl::destroy(ptr);
    
    // 释放内存
    ::operator delete(p);
}

// 测试数组的构造和析构
void test_array() {
    cout << "\n=== 测试数组 ===" << endl;
    
    const int size = 5;
    void* p = ::operator new(sizeof(TestClass) * size);
    TestClass* ptr = static_cast<TestClass*>(p);
    
    // 构造数组
    cout << "构造数组..." << endl;
    for(int i = 0; i < size; ++i) {
        mstl::construct(ptr + i, i);
    }
    
    // 验证数组
    cout << "数组值: ";
    for(int i = 0; i < size; ++i) {
        cout << ptr[i].getValue() << " ";
    }
    cout << endl;
    
    // 析构数组
    cout << "析构数组..." << endl;
    mstl::destroy(ptr, ptr + size);
    
    // 释放内存
    ::operator delete(p);
}

// 测试平凡类型
void test_trivial_types() {
    cout << "\n=== 测试平凡类型 ===" << endl;
    
    // 测试 char 数组
    char* chars = new char[5];
    mstl::destroy(chars, chars + 5);
    delete[] chars;
    
    // 测试 int 数组
    int* ints = new int[5];
    mstl::destroy(ints, ints + 5);
    delete[] ints;
    
    cout << "平凡类型测试完成" << endl;
}

// 测试 STL 容器
void test_stl_containers() {
    cout << "\n=== 测试 STL 容器 ===" << endl;
    
    vector<TestClass> vec(3);

    for(int i = 0; i < 3; ++i) {
        mstl::construct(&vec[i], i);
    }

    for(const auto& x : vec) {
        cout << "对象值: " << x.getValue() << endl;
    }

    vec.clear();
    cout << "容器测试完成" << endl;
}

int main() {
    cout << "开始测试 mstl::construct 和 mstl::destroy" << endl;
    
    test_single_object();
    test_array();
    test_trivial_types();
    test_stl_containers();
    
    cout << "\n所有测试完成" << endl;
    return 0;
} 