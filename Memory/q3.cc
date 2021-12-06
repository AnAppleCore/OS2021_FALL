#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <string>
#include <iostream>

#include "lib/utils.h"
#include "lib/array_list.h"
#include "lib/memory_manager.h"

// Modified from `mma_test.cc`, include tests required in Q3. 
// 100ms `slow_function()` overhead each page replacement op. 

namespace proj3 {
namespace testing{

class Q3 : public ::testing::Test {
 protected:
  void SetUp() override {
    range = 10;
    for (int i=1; i<=range; i++) {
        // Use FIFO Algorithm as replacement policy
        mma.push_back(new proj3::MemoryManager(i, FIFO, true));
        // Use CLOCk algorithm as replacement policy
        mma_clock.push_back(new proj3::MemoryManager(i, CLOCK, true));
    }
    
    workload_sz_2 = 2000;
    loop_times = 100;
  }
  void TearDown() override {
      for (int i=0; i<range; i++){
          delete mma[i];
          delete mma_clock[i];
      }
  }
    size_t workload_sz_2;
    int loop_times;
    int range;
    std::vector<proj3::MemoryManager *> mma;
    std::vector<proj3::MemoryManager *> mma_clock;
};

int task2(proj3::MemoryManager * mma, size_t workload_sz_2, int loop_times) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<proj3::ArrayList*>arr;
    for(int i = 0; i<loop_times; i++){
        arr.push_back(mma->Allocate(workload_sz_2));
        for(unsigned long j = 0; j < workload_sz_2; j++)arr[i]->Write(j, i);
    }
    for(int i = 0; i<loop_times; i++){
        if(i %2)mma->Release(arr[i]);
        else for(unsigned long j = 0; j < workload_sz_2; j++)EXPECT_EQ(i, arr[i]->Read(j));
    }
    for(int i = 0; i<loop_times; i++){
        if(i %2 == 0)mma->Release(arr[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    // auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return dur.count();
}

TEST_F(Q3,task2){
    for (int i=0; i<range; i++){
        int dur_1 = task2(mma[i], workload_sz_2, loop_times);
        int dur_2 = task2(mma_clock[i], workload_sz_2, loop_times);
        // printf("%d FIFO %d us\tCLOCK %d us\n", i+1, dur_1, dur_2);
        printf("%d FIFO %d ms\tCLOCK %d ms\n", i+1, dur_1, dur_2);
    }
}

} // namespace testing
} // namespace proj3

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
