#include "mstl_alloc.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <set>
#include <thread>
#include <iomanip>

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

void test_multithread_performance(size_t num_ops, size_t size, size_t num_threads) {
    cout << "\n多线程测试 (" << num_threads << " 线程):" << endl;
    
    // 创建线程数组
    vector<thread> threads;
    threads.reserve(num_ops);
    
    // 每个线程的操作数
    size_t ops_per_thread = num_ops / num_threads;
    
    // 创建分配器
    mstl::allocator<int, mstl::thread_safe_alloc> alloc;
    
    // 启动所有线程
    auto start = std::chrono::steady_clock::now();
    
    for(size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&alloc, ops_per_thread, size]() {
            vector<int*> ptrs;
            ptrs.reserve(ops_per_thread);
            
            // 分配内存
            for(size_t j = 0; j < ops_per_thread; ++j) {
                ptrs.push_back(alloc.allocate(size));
            }
            
            // 释放内存
            for(auto p : ptrs) {
                alloc.deallocate(p, size);
            }
        });
    }
    
    // 等待所有线程完成
    for(auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    cout << "总耗时: " << duration.count() << " 微秒" << endl;
    cout << "平均每次操作耗时: " << static_cast<double>(duration.count()) / num_ops << " 微秒" << endl;
    
    // 测试内存占用
    test_memory_usage(num_ops, size);
}

// 测试多线程内存分配争用情况
void test_thread_contention() {
    const int num_threads = 4;  // 测试4个线程
    const int num_allocations = 1000000;  // 每个线程分配100万次
    const int allocation_size = 32;  // 每次分配32字节
    
    std::cout << "\n测试多线程内存分配争用情况:\n";
    std::cout << "线程数: " << num_threads << "\n";
    std::cout << "每个线程分配次数: " << num_allocations << "\n";
    std::cout << "每次分配大小: " << allocation_size << " 字节\n\n";
    
    // 测试单线程版本
    {
        std::cout << "单线程版本:\n";
        auto start = std::chrono::high_resolution_clock::now();
        
        for(int i = 0; i < num_threads; ++i) {
            for(int j = 0; j < num_allocations; ++j) {
                void* p = mstl::default_alloc::allocate(allocation_size);
                mstl::default_alloc::deallocate(p, allocation_size);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "总时间: " << duration.count() << "ms\n";
        std::cout << "平均每次分配时间: " << (duration.count() / (num_threads * num_allocations)) << "微秒\n\n";
    }
    
    // 测试多线程版本
    {
        std::cout << "多线程版本:\n";
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for(int i = 0; i < num_threads; ++i) {
            threads.emplace_back([num_allocations, allocation_size]() {
                for(int j = 0; j < num_allocations; ++j) {
                    void* p = mstl::thread_safe_alloc::allocate(allocation_size);
                    mstl::thread_safe_alloc::deallocate(p, allocation_size);
                }
            });
        }
        
        for(auto& thread : threads) {
            thread.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "总时间: " << duration.count() << "ms\n";
        std::cout << "平均每次分配时间: " << (duration.count() / (num_threads * num_allocations)) << "微秒\n";
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
    
    // 测试多线程性能
    vector<size_t> thread_counts = {1, 2, 4, 8, 16};  // 测试不同的线程数
    for(size_t threads : thread_counts) {
        test_multithread_performance(NUM_OPERATIONS, BLOCK_SIZE, threads);
    }
    
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
    
    test_thread_contention();
    
    return 0;
}