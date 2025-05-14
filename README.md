# Modern SGI STL

这是一个基于侯捷《STL源码剖析》的 SGI STL 重构练习项目，使用现代 C++ (20)特性重新实现，提高代码的可读性和可维护性。

## 项目特点

- 使用现代 C++ 20 特性重构 SGI STL 的实现
- 基于侯捷《STL源码剖析》一书
- 保持原有 SGI STL 的设计理念和接口
- 提高代码的可读性和可维护性
- 完整的单元测试覆盖
- 现代化的内存管理和线程安全实现

## 环境要求

- C++ 20 或更高版本
- CMake 3.15 或更高版本
- 支持 C++20 特性的现代编译器（如 GCC 10+、Clang 10+、MSVC 2019+）

## 已实现的功能

### 内存管理
- `mstl_alloc.h`: 内存分配器实现
  - 一级分配器：直接使用 malloc/free
  - 二级分配器：内存池管理
- `mpthread_alloc.h`: 线程安全的内存分配器实现

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

### 迭代器
- `mstl_iterator.h`: 迭代器基类
- `mstl_iterator_traits.h`: 迭代器特性萃取
- `mstl_iterator_tags.h`: 迭代器标签
- `mstl_concepts.h`: 迭代器概念约束

### 容器
- `mstl_vector.h`: 动态数组实现
- `mstl_list.h`: 双向链表实现
- `mstl_deque.h`: 双端队列实现
- `mstl_slist.h`: 单向链表实现
- `mstl_stack.h`: 栈实现
- `mstl_queue.h`: 队列实现
- `mstl_heap.h`: 堆实现
- `mstl_tree.h`: 红黑树实现

### 算法
- `mstl_functional.h`: 函数对象和函数适配器

## 测试

每个模块都配有相应的测试文件：
- `mstl_alloc_test.cpp`: 测试内存分配器
- `mstl_construct_test.cpp`: 测试对象构造与析构
- `mstl_uninitialized_test.cpp`: 测试未初始化空间处理
- `mstl_iterator_test.cpp`: 测试迭代器
- `mstl_vector_test.cpp`: 测试向量
- `mstl_list_test.cpp`: 测试链表
- `mstl_deque_test.cpp`: 测试双端队列
- `mstl_slist_test.cpp`: 测试单向链表
- `mstl_stack_test.cpp`: 测试栈
- `mstl_queue_test.cpp`: 测试队列
- `mstl_heap_test.cpp`: 测试堆
- `mpthread_alloc_test.cpp`: 测试线程安全的内存分配器

## 构建与运行

### 使用 CMake 构建（推荐）

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目（启用测试）
cmake -DBUILD_TESTING=ON ..

# 构建项目
cmake --build .

# 运行测试
ctest --output-on-failure
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
├── include/                 # 头文件目录
│   ├── allocator/          # 内存分配器相关
│   │   ├── mstl_alloc.h
│   │   └── mpthread_alloc.h
│   ├── iterator/           # 迭代器相关
│   │   ├── mstl_iterator.h
│   │   ├── mstl_iterator_traits.h
│   │   └── mstl_iterator_tags.h
│   ├── container/          # 容器相关
│   │   ├── mstl_vector.h
│   │   ├── mstl_list.h
│   │   └── ...
│   └── utils/              # 工具类
│       ├── mstl_construct.h
│       └── mstl_uninitialized.h
├── tests/                  # 测试文件目录
├── examples/              # 示例代码目录
├── docs/                 # 文档目录
├── CMakeLists.txt       # CMake 构建配置
└── README.md            # 项目说明文档
```

## 使用示例

```cpp
#include "mstl_alloc.h"
#include "mstl_construct.h"
#include "mstl_uninitialized.h"
#include "mstl_vector.h"
#include "mstl_list.h"

int main() {
    // 使用内存分配器
    int* p = static_cast<int*>(mstl::allocate(sizeof(int)));
    
    // 构造对象
    mstl::construct(p, 42);
    
    // 使用未初始化空间处理
    int* arr = static_cast<int*>(mstl::allocate(sizeof(int) * 5));
    mstl::uninitialized_fill_n(arr, 5, 42);
    
    // 使用向量
    mstl::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    // 使用链表
    mstl::list<int> lst;
    lst.push_back(3);
    lst.push_back(4);
    
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

## 贡献指南

欢迎贡献代码！如果你想为项目做出贡献，请：

1. Fork 本仓库
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的改动 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启一个 Pull Request

请确保：
- 代码符合项目的编码规范（遵循 .clang-format）
- 添加适当的测试用例
- 更新相关文档

## 开发计划

- [ ] 完善容器适配器实现
- [ ] 添加更多算法
- [ ] 优化内存分配器性能
- [ ] 添加更多使用示例
- [ ] 完善文档

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 致谢

- 感谢侯捷老师的《STL源码剖析》
- 感谢原 SGI STL 的开发者们
- 感谢所有贡献者

## 联系方式

如有问题或建议，欢迎提出 Issue 或 Pull Request。
