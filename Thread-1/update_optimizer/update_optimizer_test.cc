#include <gtest/gtest.h>
#include <vector>
#include "update_optimizer.h"


double vecA[20] = {-0.9166718150866153,0.43829227278389205,-0.6129205231425989,0.7738693110376684,-0.04456069156634124,0.8807831166061564,-0.8206278700342509,0.5013513447020947,0.8570578529268591,0.3906799520892723,0.6558968505921785,0.0011654760044317314,0.3008835791217257,-0.15983430198414705,-0.0693333837225647,-0.6526558237637923};
double vecB[20] = {0.9210598583969818,0.42579710374854174,0.4036360259825613,0.030714658443632636,-0.885772459758438,-0.24727514094877123,0.7355606108008894,0.10546495518591437,-0.8023562184533268,-0.49625645997262624,-0.9207327565798142,-0.5815206606673406,0.9351907452001367,0.3172382122419557,0.003001769800845988,-0.922981014248552};

namespace proj1 {
namespace testing{

class UpdateOptimizerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    embA = new Embedding(20, vecA);
	embB = new Embedding(20, vecB);
	optim0 = new Update_Optimizer(*embA, *embB, 0);
	optim1 = new Update_Optimizer(*embA, *embB, 1);
  }
  Embedding* embA;
  Embedding* embB;
  Update_Optimizer* optim0;
  Update_Optimizer* optim1;
};

bool compare(Embedding emb1, Embedding emb2){
	double * tmp1 = emb1.get_data();
	double * tmp2 = emb2.get_data();
	for(int i = 0; i<20; i++){
		if(tmp1[i]!=tmp2[i])return false;
	}
	return true;
}

TEST_F(UpdateOptimizerTest, test_compute_output_grad){

	EmbeddingGradient gradA_B_0 = calc_gradient(embA, embB,0);
	EmbeddingGradient gradB_A_0 = calc_gradient(embB, embA,0);
	EmbeddingGradient gradA_B_1 = calc_gradient(embA, embB,1);
	EmbeddingGradient gradB_A_1 = calc_gradient(embB, embA,1);

	optim0->compute();
	optim1->compute();

	EXPECT_EQ(true, compare(optim0->output_grad1(), gradA_B_0));
	EXPECT_EQ(true, compare(optim0->output_grad2(), gradB_A_0));
	EXPECT_EQ(true, compare(optim1->output_grad1(), gradA_B_1));
	EXPECT_EQ(true, compare(optim1->output_grad2(), gradB_A_1));
}

} // namespace testing
} // namespace proj1

int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
