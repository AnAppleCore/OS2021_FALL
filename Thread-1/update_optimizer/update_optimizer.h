//This package parallelize the UPDATE_EMB instruction execution
#ifndef THREAD_UPDATE_OPTIMIZER_H_
#define THREAD_UPDATE_OPTIMIZER_H_

#include "../lib/embedding.h"
#include "../lib/model.h"



namespace proj1 {
    class Update_Optimizer {
        public:
            Update_Optimizer(Embedding, Embedding,int);
            
            void compute();


            

            EmbeddingGradient output_grad1();
            EmbeddingGradient output_grad2();

        private:

            void calc1();
            void calc2();

            Embedding A;
            Embedding B;
            int label;
            EmbeddingGradient grad1;
            EmbeddingGradient grad2;
    
    };
};


#endif // THREAD_LIB_EMBEDDING_H_