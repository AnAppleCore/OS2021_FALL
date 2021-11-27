#include <gtest/gtest.h>
#include <vector>
#include "scheduler/scheduler.h"


std::string s0 = "0 0 1 2 3 4 5 6 7 8";
std::string s1 = "1 3 5 1";
std::string s2 = "2 2 -1 4 5 6 3 7 9";

namespace proj1 {
namespace testing{

class SchedulerTest : public ::testing::Test {
 protected:
  void SetUp() override {
	
    scheduler = new Scheduler("data/q3.in","data/q3.in", "data/q3_instruction.tsv");
  }
  Instruction inst0 = Instruction(s0);
  Instruction inst1 = Instruction(s1);
  Instruction inst2 = Instruction(s2);
  Scheduler* scheduler;
};

bool compare(Embedding emb1, Embedding emb2){
	double * tmp1 = emb1.get_data();
	double * tmp2 = emb2.get_data();
	for(int i = 0; i<20; i++){
		if(tmp1[i]!=tmp2[i])return false;
	}
	return true;
}

TEST_F(SchedulerTest, test_compare_instructions){
	EXPECT_EQ(true, compare_instructions(inst0, inst1));
	EXPECT_EQ(true, compare_instructions(inst0, inst2));
	EXPECT_EQ(false, compare_instructions(inst1, inst2));
}

TEST_F(SchedulerTest, test_get_epoch_num){

	EXPECT_EQ(0, scheduler->get_epoch_num(0));
	EXPECT_EQ(0, scheduler->get_epoch_num(1));
	EXPECT_EQ(0, scheduler->get_epoch_num(2));
	EXPECT_EQ(-2, scheduler->get_epoch_num(3));
	EXPECT_EQ(-2, scheduler->get_epoch_num(4));
	EXPECT_EQ(0, scheduler->get_epoch_num(5));
	EXPECT_EQ(1, scheduler->get_epoch_num(6));
	EXPECT_EQ(1, scheduler->get_epoch_num(7));
	EXPECT_EQ(-2, scheduler->get_epoch_num(8));
}

} // namespace testing
} // namespace proj1

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
