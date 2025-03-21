# Modern SGI STL

这是一个基于侯捷《STL源码剖析》的 SGI STL 重构练习项目，使用现代 C++ 特性重新实现，提高代码的可读性和可维护性。

## 项目特点

- 使用现代 C++ 特性重构 SGI STL 的实现
- 基于侯捷《STL源码剖析》一书
- 保持原有 SGI STL 的设计理念和接口
- 提高代码的可读性和可维护性

## 已实现的功能

### 内存管理
- `mstl_alloc.h`: 内存分配器实现
  - 一级分配器：直接使用 malloc/free
  - 二级分配器：内存池管理

### 构造与析构
- `mstl_construct.h`: 对象构造与析构
  - construct: 对象构造
  - destroy: 对象析构
  - 支持 POD 和非 POD 类型

### 未初始化空间处理
- `mstl_uninitialized.h`: 未初始化空间处理
  - uninitialized_copy: 复制到未初始化空间
  - uninitialized_fill: 填充未初始化空间
  - uninitialized_fill_n: 填充 n 个元素到未初始化空间
  - 支持 POD 和非 POD 类型
  - 支持完美转发

## 测试

每个模块都配有相应的测试文件：
- `mstl_alloc_test.cpp`: 测试内存分配器
- `mstl_construct_test.cpp`: 测试对象构造与析构
- `mstl_uninitialized_test.cpp`: 测试未初始化空间处理

## 构建与运行

### 使用 CMake 构建（推荐）

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 构建项目
cmake --build .

# 运行测试
ctest
```

### 直接编译（可选）

```bash
# 编译
g++ -std=c++17 -o test *.cpp

# 运行测试
./test
```

## 项目结构

```
modern_sgi_stl/
├── CMakeLists.txt        # CMake 构建配置
├── mstl_alloc.h          # 内存分配器
├── mstl_construct.h      # 构造与析构
├── mstl_uninitialized.h  # 未初始化空间处理
├── mstl_alloc_test.cpp   # 内存分配器测试
├── mstl_construct_test.cpp # 构造与析构测试
└── mstl_uninitialized_test.cpp # 未初始化空间处理测试
```

## 使用示例

```cpp
#include "mstl_alloc.h"
#include "mstl_construct.h"
#include "mstl_uninitialized.h"

int main() {
    // 使用内存分配器
    int* p = static_cast<int*>(mstl::allocate(sizeof(int)));
    
    // 构造对象
    mstl::construct(p, 42);
    
    // 使用未初始化空间处理
    int* arr = static_cast<int*>(mstl::allocate(sizeof(int) * 5));
    mstl::uninitialized_fill_n(arr, 5, 42);
    
    // 清理
    mstl::destroy(p);
    mstl::destroy(arr, arr + 5);
    mstl::deallocate(p);
    mstl::deallocate(arr);
    
    return 0;
}
```

## 注意事项

- 需要 C++17 或更高版本
- 仅用于学习和练习目的
- 不建议在生产环境中使用

## 参考

- 《STL源码剖析》- 侯捷
- SGI STL 源码
