#include <thread>

#include "../lib/utils.h"
#include "../lib/embedding.h"

#include "update_optimizer.h"
 namespace proj1 {
     Update_Optimizer::Update_Optimizer( Embedding A, Embedding B, int label) {
        
         this -> A = A;
         this -> B = B;
         this -> label = label;
     }

     void Update_Optimizer::calc1() {
        this -> grad1 = *calc_gradient(&this -> A, &this -> B,label);
     }
     void Update_Optimizer::calc2() {
        this -> grad2 = *calc_gradient(&this -> B, &this -> A,label);
     }

     void Update_Optimizer::compute() {
        std::thread th1(&Update_Optimizer::calc1, this);
        std::thread th2(&Update_Optimizer::calc2, this);
        th1.join();
        th2.join();
     }

    EmbeddingGradient Update_Optimizer::output_grad1() {
        return this -> grad1;
    }
    EmbeddingGradient Update_Optimizer::output_grad2() {
        return this -> grad2;

    }

     
 }