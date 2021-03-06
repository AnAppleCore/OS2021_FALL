#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <string>
#include <iostream>

#include "lib/utils.h"
#include "lib/array_list.h"
#include "lib/memory_manager.h"

// Modified from `mma_test.cc`, include tests required in Q4. 
// 10ms `slow_function()` overhead each page replacement op. 

namespace proj3 {
namespace testing{

class Q4 : public ::testing::Test {
 protected:
  void SetUp() override {
    // Use CLOCk algorithm as replacement policy
    mma_clock = new proj3::MemoryManager(10, CLOCK, false);
    workload_sz_4 = 2000;
    range = 10;
    for (int i=10; i<=10+range; i++) {
        thread_num.push_back(i);  
    }
  }
  void TearDown() override {delete mma_clock;}
    int range;
    size_t workload_sz_4;
    std::vector<int> thread_num;
    proj3::MemoryManager * mma_clock;
};

void workload(proj3::MemoryManager * my_mma, size_t workload_sz){
    proj3::ArrayList* arr = my_mma->Allocate(workload_sz);
    for(unsigned long j = 0; j < workload_sz; j++)arr->Write(j, j);
    for(unsigned long j = 0; j < workload_sz; j++)EXPECT_EQ(j, arr->Read(j));
    my_mma->Release(arr);
}

int task4(proj3::MemoryManager * mma, size_t workload_sz_4, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread*> pool;
    for(int i = 0; i<thread_num; i++) {
        pool.push_back(new std::thread(&workload, mma, workload_sz_4));
    }

    for (auto t: pool) {
        t->join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    // auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return dur.count();
}

TEST_F(Q4,task4){
    for (int i=0; i<=range; i++){
        int dur = task4(mma_clock, workload_sz_4, thread_num[i]);
        // printf("Allocation %d\tCLOCK %d us\n", i+1, dur);
        printf("Thread # %d\tCLOCK %d ms\n", i+10, dur);
    }
}

} // namespace testing
} // namespace proj3

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
