//This package parallelize the INIT_EMB instruction execution
#ifndef THREAD_INIT_OPTIMIZER_H_
#define THREAD_INIT_OPTIMIZER_H_

#include <mutex>
#include <vector>
#include <stdlib.h>

#include "../lib/embedding.h"
#include "../lib/model.h"



namespace proj1 {
    class Init_Optimizer {
        public:
            Init_Optimizer(Embedding newuser);
            //~Init_Optimizer() 
            
            void add_item(Embedding*);
            EmbeddingGradient output_grad();
            void compute();
            void single_thread(Embedding* item);//single cold start
        private:
            EmbeddingHolder* items =new EmbeddingHolder(*(new EmbeddingMatrix));
            Embedding user;
            EmbeddingGradient gradient;
            std::mutex mux;
    };
};


#endif // THREAD_LIB_EMBEDDING_H_