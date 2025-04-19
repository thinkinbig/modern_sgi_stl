#include "mpthread_alloc.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// 定义一个用于跟踪内存分配的简单类
class LeakTracker {
private:
    std::vector<std::pair<void*, size_t>> allocated_memory;

public:
    void record_allocation(void* ptr, size_t size) {
        if (!ptr)
            return;
        allocated_memory.push_back({ptr, size});
    }

    void record_deallocation(void* ptr) {
        if (!ptr)
            return;
        for (auto it = allocated_memory.begin(); it != allocated_memory.end(); ++it) {
            if (it->first == ptr) {
                allocated_memory.erase(it);
                return;
            }
        }
        std::cerr << "警告：尝试释放未跟踪的内存 " << ptr << std::endl;
    }

    void report_leaks() const {
        if (allocated_memory.empty()) {
            std::cout << "没有检测到内存泄漏" << std::endl;
            return;
        }

        std::cout << "检测到 " << allocated_memory.size() << " 处内存泄漏:" << std::endl;
        for (const auto& leak : allocated_memory) {
            std::cout << "地址: " << leak.first << ", 大小: " << leak.second << " 字节"
                      << std::endl;
        }
    }

    static LeakTracker& instance() {
        static LeakTracker tracker;
        return tracker;
    }
};

// 封装分配和释放函数，以跟踪内存使用
void* tracked_allocate(size_t size) {
    void* ptr = mstl::PthreadAllocatorTemplate<>::allocate(size);
    LeakTracker::instance().record_allocation(ptr, size);
    return ptr;
}

void tracked_deallocate(void* ptr, size_t size) {
    LeakTracker::instance().record_deallocation(ptr);
    mstl::PthreadAllocatorTemplate<>::deallocate(ptr, size);
}

void test_basic_allocation() {
    std::cout << "Testing basic allocation..." << std::endl;

    void* p = mstl::PthreadAllocatorTemplate<>::allocate(64);

    assert(p != nullptr);

    mstl::PthreadAllocatorTemplate<>::deallocate(p, 64);

    std::cout << "Basic allocation test passed!" << std::endl;
}

void test_stl_container() {
    std::cout << "Testing STL container integration..." << std::endl;

    // STL容器测试
    std::vector<int, mstl::StlPthreadAllocator<int>> vec;
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(i);
    }

    // 验证数据正确性
    for (int i = 0; i < 1000; ++i) {
        assert(vec[i] == i);
    }

    std::cout << "STL container test passed!" << std::endl;
}

void test_smart_pointer() {
    std::cout << "Testing smart pointer with custom allocator..." << std::endl;

    // 智能指针测试
    auto deleter = [](int* p) { mstl::PthreadAllocatorTemplate<>::deallocate(p, sizeof(int)); };

    std::unique_ptr<int, decltype(deleter)> ptr(
        static_cast<int*>(mstl::PthreadAllocatorTemplate<>::allocate(sizeof(int))), deleter);

    *ptr = 42;
    assert(*ptr == 42);

    std::cout << "Smart pointer test passed!" << std::endl;
}

// 运行多线程分配测试，返回耗时（微秒）
double run_multi_thread_test(int num_threads, int allocs_per_thread, bool use_custom_allocator) {
    auto thread_func_custom = [allocs_per_thread]() {
        std::vector<void*> ptrs;
        ptrs.reserve(allocs_per_thread);
        for (int i = 0; i < allocs_per_thread; ++i) {
            void* p = mstl::PthreadAllocatorTemplate<>::allocate(64);
            ptrs.push_back(p);
        }
        for (void* p : ptrs) {
            mstl::PthreadAllocatorTemplate<>::deallocate(p, 64);
        }
    };

    auto thread_func_std = [allocs_per_thread]() {
        std::vector<void*> ptrs;
        ptrs.reserve(allocs_per_thread);
        for (int i = 0; i < allocs_per_thread; ++i) {
            void* p = ::operator new(64);
            ptrs.push_back(p);
        }
        for (void* p : ptrs) {
            ::operator delete(p);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        if (use_custom_allocator) {
            threads.emplace_back(thread_func_custom);
        } else {
            threads.emplace_back(thread_func_std);
        }
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return static_cast<double>(duration) / 1000000.0; // 转换为秒
}

void test_multi_thread() {
    std::cout << "Testing multi-threaded allocation..." << std::endl;

    const int NUM_THREADS = 4;
    const int ALLOCS_PER_THREAD = 1000;

    run_multi_thread_test(NUM_THREADS, ALLOCS_PER_THREAD, true);

    std::cout << "Multi-threaded test passed!" << std::endl;
}

void benchmark() {
    const int num_threads = 4;
    const int allocs_per_thread = 1000000;
    const int num_runs = 5;

    std::cout << "开始性能测试..." << std::endl;
    std::cout << "线程数: " << num_threads << std::endl;
    std::cout << "每个线程分配次数: " << allocs_per_thread << std::endl;
    std::cout << "运行次数: " << num_runs << std::endl;

    double total_std_time = 0.0;
    double total_custom_time = 0.0;

    for (int i = 0; i < num_runs; ++i) {
        std::cout << "\n运行 " << i + 1 << "/" << num_runs << ":" << std::endl;

        double std_time = run_multi_thread_test(num_threads, allocs_per_thread, false);
        double custom_time = run_multi_thread_test(num_threads, allocs_per_thread, true);

        total_std_time += std_time;
        total_custom_time += custom_time;

        std::cout << "标准分配器时间: " << std::fixed << std::setprecision(6) << std_time << " 秒" << std::endl;
        std::cout << "自定义分配器时间: " << std::fixed << std::setprecision(6) << custom_time << " 秒" << std::endl;
        std::cout << "性能提升: " << std::fixed << std::setprecision(2) 
                  << (std_time - custom_time) / std_time * 100 << "%" << std::endl;
    }

    double avg_std_time = total_std_time / num_runs;
    double avg_custom_time = total_custom_time / num_runs;

    std::cout << "\n平均性能:" << std::endl;
    std::cout << "标准分配器: " << std::fixed << std::setprecision(6) << avg_std_time << " 秒" << std::endl;
    std::cout << "自定义分配器: " << std::fixed << std::setprecision(6) << avg_custom_time << " 秒" << std::endl;
    std::cout << "平均性能提升: " << std::fixed << std::setprecision(2) 
              << (avg_std_time - avg_custom_time) / avg_std_time * 100 << "%" << std::endl;
}

// 测试内存泄漏检测
void test_memory_leak() {
    std::cout << "\n===== 内存泄漏测试 =====" << std::endl;

    // 注意：这里故意不释放，用于测试泄漏检测功能
    void* leak_ptr = tracked_allocate(128);
    std::cout << "故意泄漏的内存: " << leak_ptr << std::endl;

    // 创建并正确释放，不应该报告泄漏
    void* normal_ptr = tracked_allocate(64);
    tracked_deallocate(normal_ptr, 64);

    std::cout << "内存泄漏测试完成" << std::endl;
    LeakTracker::instance().report_leaks();
}

int main() {
    std::cout << "Starting pthread allocator tests..." << std::endl;

    // 功能测试
    test_basic_allocation();
    test_stl_container();
    test_smart_pointer();
    test_multi_thread();

    std::cout << "All tests passed successfully!" << std::endl;

    // 性能测试
    benchmark();

    // 内存泄漏测试
    test_memory_leak();

    return 0;
}