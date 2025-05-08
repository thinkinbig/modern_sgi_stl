# 在 project() 之前设置编译器
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

cmake_minimum_required(VERSION 3.10)
project(ModernSGI_STL VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

# 查找clang-format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    message(STATUS "clang-format: ${CLANG_FORMAT}")
else()
    message(WARNING "clang-format not found, formatting will be disabled")
endif()

# 添加格式化目标
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "formatting code..."
        VERBATIM
    )
    
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

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# 添加Address Sanitizer支持
option(USE_SANITIZER "Enable Address Sanitizer" ON)
if(USE_SANITIZER)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    message(STATUS "Address Sanitizer enabled")
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
add_executable(mpthread_alloc_test mpthread_alloc_test.cpp)
add_executable(mstl_iterator_test mstl_iterator_test.cpp)
add_executable(mstl_vector_test mstl_vector_test.cpp)
add_executable(mstl_list_test mstl_list_test.cpp)
add_executable(mstl_deque_test mstl_deque_test.cpp)
add_executable(mstl_stack_test mstl_stack_test.cpp)
add_executable(mstl_queue_test mstl_queue_test.cpp)
add_executable(mstl_heap_test mstl_heap_test.cpp)
add_executable(mstl_slist_test mstl_slist_test.cpp)
add_executable(mstl_tree_test mstl_tree_test.cpp)
add_executable(mstl_lru_test mstl_lru_test.cpp)
add_executable(mstl_set_test mstl_set_test.cpp)

# 为所有测试添加调试信息
set(DEBUG_FLAGS "-g -O1")

set(ALL_TESTS
    mstl_alloc_test
    mstl_construct_test
    mstl_uninitialized_test
    mpthread_alloc_test
    mstl_iterator_test
    mstl_vector_test
    mstl_list_test
    mstl_deque_test
    mstl_stack_test
    mstl_queue_test
    mstl_heap_test
    mstl_slist_test
    mstl_tree_test
    mstl_lru_test
    mstl_set_test
)

foreach(TEST ${ALL_TESTS})
    set_target_properties(${TEST}
        PROPERTIES
        COMPILE_FLAGS ${DEBUG_FLAGS}
    )
endforeach()

# 链接pthread库
find_package(Threads REQUIRED)
target_link_libraries(mpthread_alloc_test PRIVATE Threads::Threads)

# 添加测试
enable_testing()
set(CTEST_OUTPUT_ON_FAILURE ON)
set(CTEST_VERBOSE ON)

# 添加所有测试
foreach(TEST ${ALL_TESTS})
    add_test(NAME ${TEST} 
             COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST}
             WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
    set_tests_properties(${TEST}
        PROPERTIES
        ENVIRONMENT "CTEST_FULL_OUTPUT=1"
        LABELS "debug"
    )
endforeach()

# 打印调试信息
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C Compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ Debug Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# 安装规则
install(TARGETS ${ALL_TESTS}
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/mstl
    FILES_MATCHING PATTERN "*.h"
)

# 生成compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
