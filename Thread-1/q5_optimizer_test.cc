#include <gtest/gtest.h>
#include <vector>
#include "q5_lib/q5_optimizer.h"


std::string s0 = "0 0 1 2 3 4 5 6 7 8";
std::string s1 = "1 3 5 1";
std::string s2 = "2 2 -1 4 5 6 3 7 9";

namespace proj1 {
namespace testing{

class OptimizerTest : public ::testing::Test {
 protected:
  void SetUp() override {
	
    optim = new Optimizer("data/q3.in","data/q3.in", "data/q3_instruction.tsv");
	users = new EmbeddingHolder("data/q3.in");
	items = new EmbeddingHolder("data/q3.in");
  }
  Instruction inst0 = Instruction(s0);
  Instruction inst1 = Instruction(s1);
  Instruction inst2 = Instruction(s2);
  Optimizer* optim;
  EmbeddingHolder* users;
  EmbeddingHolder* items;
};

bool compare(EmbeddingStack s, EmbeddingHolder* h){
	if (s.size() != h->get_n_embeddings()) return false;
	if (s[0].get_length() != h->get_emb_length()) return false;
	const int length = s.size();
	for (int i = 0; i < length; i++){
		if (!(s[i] == *(h->get_embedding(i)))) return false;
	}
	return true;
}

TEST_F(OptimizerTest, test_compare_instructions){
	EXPECT_EQ(true, compare_instructions(inst0, inst1));
	EXPECT_EQ(true, compare_instructions(inst0, inst2));
	EXPECT_EQ(false, compare_instructions(inst1, inst2));
}

TEST_F(OptimizerTest, test_get_epoch_num){

	EXPECT_EQ(0, optim->get_epoch_num(0));
	EXPECT_EQ(0, optim->get_epoch_num(1));
	EXPECT_EQ(0, optim->get_epoch_num(2));
	EXPECT_EQ(-2, optim->get_epoch_num(3));
	EXPECT_EQ(-2, optim->get_epoch_num(4));
	EXPECT_EQ(0, optim->get_epoch_num(5));
	EXPECT_EQ(1, optim->get_epoch_num(6));
	EXPECT_EQ(1, optim->get_epoch_num(7));
	EXPECT_EQ(-2, optim->get_epoch_num(8));
}

TEST_F(OptimizerTest, test_copy_holder){
	EmbeddingStack old_users = copy_holder(users);
	EmbeddingStack old_items = copy_holder(items);
	EXPECT_EQ(1, compare(old_users, users));
	EXPECT_EQ(1, compare(old_items, items));
}

} // namespace testing
} // namespace proj1

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
