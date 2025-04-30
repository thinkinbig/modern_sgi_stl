#include "mstl_lru.h"
#include <iostream>
#include <string>
#include <cassert>

void test_basic_operations() {
    std::cout << "Testing basic operations...\n";
    mstl::LRUCache<int, std::string> cache(2);
    
    cache.put(1, "one");
    cache.put(2, "two");
    assert(cache.get(1) == "one");
    assert(cache.get(2) == "two");
    std::cout << "Basic operations test passed!\n";
}

void test_lru_eviction() {
    std::cout << "Testing LRU eviction...\n";
    mstl::LRUCache<int, std::string> cache(2);
    
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");  // 应该淘汰 key=1
    
    assert(!cache.contains(1));
    assert(cache.get(2) == "two");
    assert(cache.get(3) == "three");
    std::cout << "LRU eviction test passed!\n";
}

void test_access_order() {
    std::cout << "Testing access order...\n";
    mstl::LRUCache<int, std::string> cache(2);
    
    cache.put(1, "one");
    cache.put(2, "two");
    cache.get(1);  // 访问 key=1，使其成为最近使用的
    cache.put(3, "three");  // 应该淘汰 key=2
    
    assert(cache.contains(1));
    assert(!cache.contains(2));
    assert(cache.get(3) == "three");
    std::cout << "Access order test passed!\n";
}

void test_update_value() {
    std::cout << "Testing value update...\n";
    mstl::LRUCache<int, std::string> cache(2);
    
    cache.put(1, "one");
    cache.put(1, "new_one");
    
    assert(cache.get(1) == "new_one");
    std::cout << "Value update test passed!\n";
}

void test_exception_handling() {
    std::cout << "Testing exception handling...\n";
    mstl::LRUCache<int, std::string> cache(2);
    
    bool exception_thrown = false;
    try {
        cache.get(1);
    } catch (const std::range_error&) {
        exception_thrown = true;
    }
    assert(exception_thrown);
    std::cout << "Exception handling test passed!\n";
}

int main() {
    try {
        test_basic_operations();
        test_lru_eviction();
        test_access_order();
        test_update_value();
        test_exception_handling();
        
        std::cout << "\nAll tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
} 