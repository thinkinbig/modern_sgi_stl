#include "mstl_alloc.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <set>

using namespace std;

void print_address(const char* desc, void* ptr) {
    cout << desc << ": " << ptr << endl;
}

// 用于计时的辅助函数
template<typename TimeT = std::chrono::microseconds>
struct measure {
    template<typename F>
    static typename TimeT::rep execution(F const &func) {
        auto start = std::chrono::steady_clock::now();
        func();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<TimeT>(end - start);
        return duration.count();
    }
};

// 测试函数：分配和释放内存
// 参数：
//   num_ops: 操作次数
//   size: 每次分配的内存大小
void test_alloc_dealloc(size_t num_ops, size_t size) {
    vector<int*> ptrs;
    ptrs.reserve(num_ops);
    mstl::allocator<int> alloc;
    
    // 分配内存
    for(size_t i = 0; i < num_ops; ++i) {
        ptrs.push_back(alloc.allocate(size));
    }
    
    // 释放内存
    for(auto p : ptrs) {
        alloc.deallocate(p, size);
    }
}

// 测试内存占用
// 参数：
//   num_ops: 操作次数
//   size: 每次分配的内存大小
void test_memory_usage(size_t num_ops, size_t size) {
    vector<int*> ptrs;
    ptrs.reserve(num_ops);
    mstl::allocator<int> alloc;
    set<void*> unique_addresses;
    
    cout << "\n内存占用测试:" << endl;
    
    // 分配内存并记录地址
    for(size_t i = 0; i < num_ops; ++i) {
        ptrs.push_back(alloc.allocate(size));
        unique_addresses.insert(ptrs.back());
    }
    
    cout << "分配 " << num_ops << " 个 " << size << " 字节的内存块" << endl;
    cout << "实际分配的内存块数量: " << unique_addresses.size() << endl;
    cout << "总内存占用: " << unique_addresses.size() * size << " 字节" << endl;
    
    // 释放内存
    for(auto p : ptrs) {
        alloc.deallocate(p, size);
    }
}

int main() {
    const size_t NUM_OPERATIONS = 1000000;  // 操作数
    const size_t BLOCK_SIZE = 16;           // 分配的内存块大小
    
    cout << "=== 内存分配器性能测试 ===" << endl;
    
    // 测试单线程性能
    cout << "\n单线程测试:" << endl;
    auto time = measure<>::execution([&]() {
        test_alloc_dealloc(NUM_OPERATIONS, BLOCK_SIZE);
    });
    cout << "总耗时: " << time << " 微秒" << endl;
    cout << "平均每次操作耗时: " << static_cast<double>(time) / NUM_OPERATIONS << " 微秒" << endl;
    
    // 测试内存占用
    test_memory_usage(NUM_OPERATIONS, BLOCK_SIZE);
    
    // 测试不同大小内存块的分配性能
    cout << "\n不同大小内存块测试:" << endl;
    vector<size_t> block_sizes = {8, 16, 32, 64, 128, 256};
    
    for(size_t size : block_sizes) {
        cout << "\n内存块大小 " << size << " 字节:" << endl;
        auto time = measure<>::execution([&]() {
            test_alloc_dealloc(NUM_OPERATIONS, size);
        });
        cout << "总耗时: " << time << " 微秒" << endl;
        cout << "平均每次操作耗时: " << static_cast<double>(time) / NUM_OPERATIONS << " 微秒" << endl;
        
        // 测试内存占用
        test_memory_usage(NUM_OPERATIONS, size);
    }
    
    return 0;
}