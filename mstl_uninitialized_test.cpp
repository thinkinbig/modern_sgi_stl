#include "mstl_uninitialized.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class TestClass {
public:
    TestClass(int x) : value(x) {
        cout << "构造: " << value << endl;
    }
    
    TestClass(const TestClass& other) : value(other.value) {
        cout << "拷贝构造: " << value << endl;
    }
    
    TestClass(TestClass&& other) noexcept : value(other.value) {
        cout << "移动构造: " << value << endl;
        other.value = 0;
    }
    
    ~TestClass() {
        cout << "析构: " << value << endl;
    }
    
    int getValue() const { return value; }
    
private:
    int value;
};

void test_pod_types() {
    cout << "\n=== 测试POD类型 ===" << endl;
    
    // 测试int数组
    int src[5] = {1, 2, 3, 4, 5};
    int* dest = static_cast<int*>(::operator new(sizeof(int) * 5));
    
    cout << "测试 uninitialized_copy:" << endl;
    mstl::uninitialized_copy(src, src + 5, dest);
    cout << "复制后的值: ";
    for(int i = 0; i < 5; ++i) {
        cout << dest[i] << " ";
    }
    cout << endl;
    ::operator delete(dest);
    
    // 测试fill_n
    dest = static_cast<int*>(::operator new(sizeof(int) * 5));
    cout << "\n测试 uninitialized_fill_n:" << endl;
    mstl::uninitialized_fill_n(dest, 5, 42);
    cout << "填充后的值: ";
    for(int i = 0; i < 5; ++i) {
        cout << dest[i] << " ";
    }
    cout << endl;
    ::operator delete(dest);
}

void test_non_pod_types() {
    cout << "\n=== 测试非POD类型 ===" << endl;
    
    // 准备源数组
    TestClass src[3] = {TestClass(1), TestClass(2), TestClass(3)};
    
    // 测试 uninitialized_copy
    cout << "\n测试 uninitialized_copy:" << endl;
    void* memory = ::operator new(sizeof(TestClass) * 3);
    TestClass* dest = static_cast<TestClass*>(memory);
    
    mstl::uninitialized_copy(src, src + 3, dest);
    
    cout << "复制后的值: ";
    for(int i = 0; i < 3; ++i) {
        cout << dest[i].getValue() << " ";
    }
    cout << endl;
    
    // 清理对象和内存
    mstl::destroy(dest, dest + 3);
    ::operator delete(memory);
    
    // 测试 uninitialized_fill_n
    cout << "\n测试 uninitialized_fill_n:" << endl;
    memory = ::operator new(sizeof(TestClass) * 3);
    dest = static_cast<TestClass*>(memory);
    
    cout << "使用左值填充:" << endl;
    TestClass val(42);
    mstl::uninitialized_fill_n(dest, 3, val);
    
    cout << "\n填充后的值: ";
    for(int i = 0; i < 3; ++i) {
        cout << dest[i].getValue() << " ";
    }
    cout << endl;
    
    // 清理对象和内存
    mstl::destroy(dest, dest + 3);
    ::operator delete(memory);
    
    // 测试右值填充
    cout << "\n使用右值填充:" << endl;
    memory = ::operator new(sizeof(TestClass) * 3);
    dest = static_cast<TestClass*>(memory);
    
    mstl::uninitialized_fill_n(dest, 3, TestClass(100));
    
    cout << "\n填充后的值: ";
    for(int i = 0; i < 3; ++i) {
        cout << dest[i].getValue() << " ";
    }
    cout << endl;
    
    // 清理对象和内存
    mstl::destroy(dest, dest + 3);
    ::operator delete(memory);
}

int main() {
    cout << "开始测试 mstl::uninitialized_copy 和 mstl::uninitialized_fill_n" << endl;
    
    test_pod_types();
    test_non_pod_types();
    
    cout << "\n所有测试完成" << endl;
    return 0;
}