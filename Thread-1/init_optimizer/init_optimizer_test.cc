#include <gtest/gtest.h>
#include <vector>
#include "init_optimizer.h"


double vecA[20] = {-0.9166718150866153,0.43829227278389205,-0.6129205231425989,0.7738693110376684,-0.04456069156634124,0.8807831166061564,-0.8206278700342509,0.5013513447020947,0.8570578529268591,0.3906799520892723,0.6558968505921785,0.0011654760044317314,0.3008835791217257,-0.15983430198414705,-0.0693333837225647,-0.6526558237637923};
double vecB[20] = {0.9210598583969818,0.42579710374854174,0.4036360259825613,0.030714658443632636,-0.885772459758438,-0.24727514094877123,0.7355606108008894,0.10546495518591437,-0.8023562184533268,-0.49625645997262624,-0.9207327565798142,-0.5815206606673406,0.9351907452001367,0.3172382122419557,0.003001769800845988,-0.922981014248552};

namespace proj1 {
namespace testing{

class InitOptimizerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    embA = new Embedding(20, vecA);
	embB = new Embedding(20, vecB);
	optimA = new Init_Optimizer(*embA);
	optimB = new Init_Optimizer(*embB);
	optimA_ = new Init_Optimizer(*embA);
	optimB_ = new Init_Optimizer(*embB);
  }
  Embedding* embA;
  Embedding* embB;
  Init_Optimizer* optimA;
  Init_Optimizer* optimB;
  Init_Optimizer* optimA_;
  Init_Optimizer* optimB_;
};

bool compare(Embedding emb1, Embedding emb2){
	double * tmp1 = emb1.get_data();
	double * tmp2 = emb2.get_data();
	for(int i = 0; i<20; i++){
		if(tmp1[i]!=tmp2[i])return false;
	}
	return true;
}

TEST_F(InitOptimizerTest, test_compute){

	EmbeddingGradient gradA = optimA->output_grad();
	EmbeddingGradient gradB = optimB->output_grad();
	EmbeddingGradient grad0 = EmbeddingGradient(20, new double[20]);
	EXPECT_EQ(true, compare(gradA, gradB));
	EXPECT_EQ(true, compare(gradA, grad0));
	EXPECT_EQ(true, compare(gradB, grad0));

	EmbeddingGradient* gradA_A = cold_start(embA, embA);
	// gradA_A->write_to_stdout();
	EmbeddingGradient* gradA_B = cold_start(embA, embB);
	// gradA_B->write_to_stdout();
	EmbeddingGradient* gradB_A = cold_start(embB, embA);
	// gradB_A->write_to_stdout();
	EmbeddingGradient* gradB_B = cold_start(embB, embB);
	// gradB_B->write_to_stdout();

	optimA->add_item(embA);
	optimB->add_item(embA);
	optimA->compute();
	optimB->compute();
	gradA = gradA + gradA_A;
	gradB = gradB + gradB_A;
	EXPECT_EQ(true, compare(gradA, grad0+gradA_A));
	EXPECT_EQ(true, compare(gradA, optimA->output_grad()));
	EXPECT_EQ(true, compare(grad0+gradA_A, optimA->output_grad()));
	EXPECT_EQ(true, compare(gradB, grad0+gradB_A));
	EXPECT_EQ(true, compare(gradB, optimB->output_grad()));
	EXPECT_EQ(true, compare(grad0+gradB_A, optimB->output_grad()));

	optimA_->add_item(embA);
	optimA_->add_item(embB);
	optimB_->add_item(embA);
	optimB_->add_item(embB);

	optimA_->compute();
	optimB_->compute();
	gradA = gradA + gradA_B;
	gradB = gradB + gradB_B;
	EXPECT_EQ(true, compare(gradA, grad0+gradA_A+gradA_B));
	EXPECT_EQ(true, compare(gradA, optimA_->output_grad()));
	EXPECT_EQ(true, compare(grad0+gradA_A+gradA_B, optimA_->output_grad()));
	EXPECT_EQ(true, compare(gradB, grad0+gradB_A+gradB_B));
	EXPECT_EQ(true, compare(gradB, optimB_->output_grad()));
	EXPECT_EQ(true, compare(grad0+gradB_A+gradB_B, optimB_->output_grad()));
}
} // namespace testing
} // namespace proj1

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
