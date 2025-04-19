#include "mstl_alloc.h"
#include <iostream>
#include <vector>
#include "mstl_allocator.h"
#include "mstl_construct.h"

// 基本功能测试
void testBasicAllocation() {
    std::cout << "=== 基本内存分配测试 ===" << std::endl;

    // 测试小内存分配
    std::cout << "\n测试小内存分配 (16字节):" << std::endl;
    void* p1 = mstl::default_alloc::allocate(16);
    std::cout << "分配地址: " << p1 << std::endl;
    mstl::default_alloc::deallocate(p1, 16);
    std::cout << "释放成功" << std::endl;

    // 测试大内存分配
    std::cout << "\n测试大内存分配 (128字节):" << std::endl;
    void* p2 = mstl::default_alloc::allocate(128);
    std::cout << "分配地址: " << p2 << std::endl;
    mstl::default_alloc::deallocate(p2, 128);
    std::cout << "释放成功" << std::endl;
}

// 测试连续分配和释放
void testSequentialAllocation() {
    std::cout << "\n=== 连续内存分配测试 ===" << std::endl;

    const int numAllocations = 5;
    void* ptrs[numAllocations];

    // 连续分配
    for (int i = 0; i < numAllocations; ++i) {
        ptrs[i] = mstl::default_alloc::allocate(32);
        std::cout << "分配 #" << i << " 地址: " << ptrs[i] << std::endl;
    }

    // 连续释放
    for (int i = 0; i < numAllocations; ++i) {
        mstl::default_alloc::deallocate(ptrs[i], 32);
        std::cout << "释放 #" << i << " 成功" << std::endl;
    }
}

// 测试分配器类
void testAllocatorClass() {
    std::cout << "\n=== 分配器类测试 ===" << std::endl;

    mstl::Allocator<int> alloc;

    // 分配单个元素
    int* p1 = alloc.allocate(1);
    std::cout << "分配单个int地址: " << p1 << std::endl;
    alloc.deallocate(p1, 1);
    std::cout << "释放成功" << std::endl;

    // 分配数组
    const int arraySize = 5;
    int* p2 = alloc.allocate(arraySize);
    std::cout << "分配int数组地址: " << p2 << std::endl;
    alloc.deallocate(p2, arraySize);
    std::cout << "释放成功" << std::endl;
}

int main() {
    testBasicAllocation();
    testSequentialAllocation();
    testAllocatorClass();

    std::cout << "\n所有测试完成" << std::endl;
    return 0;
}