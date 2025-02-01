#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
#include <string>
#include <array>

#include "LruCache.h"
// #include "LfuCache.h"
// #include "ArcCache/ArcCache.h"
#include "CachePolicy.h"

class Timer{
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed(){
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock>start_;
};

// 辅助函数：打印结果
void printResults(const std::string& testName, int capacity,
                 const std::vector<int>& get_operations,
                 const std::vector<int>& hits) {
    std::cout << "缓存大小: " << capacity << std::endl;
    std::cout << "LRU - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[0] / get_operations[0]) << "%" << std::endl;
    std::cout << "LFU - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[1] / get_operations[1]) << "%" << std::endl;
    std::cout << "ARC - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[2] / get_operations[2]) << "%" << std::endl;
}

void testHotDataAccess(){

}

void testLoopPattern(){

}

void testWorkloadShift(){

}

void testLru(){
    Cache::LruCache<int, int> lru(2);
    
    std::cout << "TestLru begin\n";

    lru.put(1, 1);
    lru.put(2, 2);

    std::cout << "get(1): " << lru.get(1) << std::endl; // 返回 1
    lru.put(3, 3); // 该操作会使得密钥 2 作废
    std::cout << "get(2): " << lru.get(2) << std::endl; // 返回 -1 (未找到)
    lru.put(4, 4); // 该操作会使得密钥 1 作废
    std::cout << "get(1): " << lru.get(1) << std::endl; // 返回 -1 (未找到)
    std::cout << "get(3): " << lru.get(3) << std::endl; // 返回 3
    std::cout << "get(4): " << lru.get(4) << std::endl; // 返回 4
    std::cout << "TestLru end\n";
}

int main(){
    // testLru();
    testHotDataAccess();
    testLoopPattern();
    testWorkloadShift();
    return 0;
}