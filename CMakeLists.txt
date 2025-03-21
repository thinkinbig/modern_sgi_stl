cmake_minimum_required(VERSION 3.10)
project(ModernSGI_STL VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加测试可执行文件
add_executable(mstl_alloc_test mstl_alloc_test.cpp)
add_executable(mstl_construct_test mstl_construct_test.cpp)
add_executable(mstl_uninitialized_test mstl_uninitialized_test.cpp)

# 添加测试
enable_testing()
add_test(NAME mstl_alloc_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mstl_alloc_test)
add_test(NAME mstl_construct_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mstl_construct_test)
add_test(NAME mstl_uninitialized_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mstl_uninitialized_test)

# 打印调试信息
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# 安装规则
install(TARGETS mstl_alloc_test mstl_construct_test mstl_uninitialized_test
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/mstl
    FILES_MATCHING PATTERN "*.h"
) 