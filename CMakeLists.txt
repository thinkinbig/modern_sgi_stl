cmake_minimum_required(VERSION 3.10)
project(ModernSGI_STL VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /EHsc /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra")
endif()

# 查找clang-format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    message(STATUS "clang-format: ${CLANG_FORMAT}")
else()
    message(WARNING "clang-format not found, formatting will be disabled")
endif()

# 添加格式化目标
if(CLANG_FORMAT)
    # 获取所有源文件和头文件
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )
    
    # 添加格式化目标
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "formatting code..."
        VERBATIM
    )
    
    # 添加检查格式化目标
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "checking code format..."
        VERBATIM
    )
endif()

# 设置调试选项
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# 添加Address Sanitizer支持
option(USE_SANITIZER "Enable Address Sanitizer" ON)
if(USE_SANITIZER)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        message(STATUS "Address Sanitizer enabled")
    else()
        message(WARNING "Address Sanitizer not supported for this compiler")
    endif()
endif()

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
if(NOT WIN32)
    add_executable(mpthread_alloc_test mpthread_alloc_test.cpp)
endif()
add_executable(mstl_iterator_test mstl_iterator_test.cpp)
add_executable(mstl_vector_test mstl_vector_test.cpp)
add_executable(mstl_list_test mstl_list_test.cpp)
add_executable(mstl_deque_test mstl_deque_test.cpp)
add_executable(mstl_stack_test mstl_stack_test.cpp)
add_executable(mstl_queue_test mstl_queue_test.cpp)
add_executable(mstl_heap_test mstl_heap_test.cpp)
add_executable(mstl_slist_test mstl_slist_test.cpp)

# 为所有测试添加调试信息
if(MSVC)
    set_target_properties(
        mstl_alloc_test
        mstl_construct_test
        mstl_uninitialized_test
        mstl_iterator_test
        mstl_vector_test
        mstl_list_test
        mstl_deque_test
        mstl_heap_test
        mstl_slist_test
        PROPERTIES
        COMPILE_FLAGS "/O1"
    )
else()
    set_target_properties(
        mstl_alloc_test
        mstl_construct_test
        mstl_uninitialized_test
        mstl_iterator_test
        mstl_vector_test
        mstl_list_test
        mstl_deque_test
        mstl_heap_test
        mstl_slist_test
        PROPERTIES
        COMPILE_FLAGS "-g -O1"
    )
endif()

if(NOT WIN32)
    set_target_properties(
        mpthread_alloc_test
        PROPERTIES
        COMPILE_FLAGS "-g -O1"
    )
endif()

# 链接pthread库
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(mpthread_alloc_test PRIVATE Threads::Threads)
endif()

# 添加测试
enable_testing()

# 设置测试输出详细程度
set(CTEST_OUTPUT_ON_FAILURE ON)
set(CTEST_VERBOSE ON)

# 添加测试
add_test(NAME mstl_alloc_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_alloc_test)
add_test(NAME mstl_construct_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_construct_test)
add_test(NAME mstl_uninitialized_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_uninitialized_test)
if(NOT WIN32)
    add_test(NAME mpthread_alloc_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mpthread_alloc_test)
endif()
add_test(NAME mstl_iterator_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_iterator_test)
add_test(NAME mstl_vector_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_vector_test)
add_test(NAME mstl_list_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_list_test)
add_test(NAME mstl_deque_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_deque_test)
add_test(NAME mstl_stack_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_stack_test)
add_test(NAME mstl_queue_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_queue_test)
add_test(NAME mstl_heap_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_heap_test)
add_test(NAME mstl_slist_test COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/mstl_slist_test)

# 设置测试属性
set_tests_properties(
    mstl_alloc_test
    mstl_construct_test
    mstl_uninitialized_test
    mstl_iterator_test
    mstl_vector_test
    mstl_list_test
    mstl_deque_test
    mstl_heap_test
    mstl_slist_test
    PROPERTIES
    ENVIRONMENT "CTEST_FULL_OUTPUT=1"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
    LABELS "debug"
)

if(NOT WIN32)
    set_tests_properties(
        mpthread_alloc_test
        PROPERTIES
        ENVIRONMENT "CTEST_FULL_OUTPUT=1"
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
        LABELS "debug"
    )
endif()

# 打印调试信息
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")

# 安装规则
install(TARGETS mstl_alloc_test mstl_construct_test mstl_uninitialized_test
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/mstl
    FILES_MATCHING PATTERN "*.h"
)

# 生成compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
