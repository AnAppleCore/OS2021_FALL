#include <thread>
#include <iostream>
#include "init_optimizer.h"
namespace proj1 {
    Init_Optimizer::Init_Optimizer(Embedding newuser) {
        this -> user = newuser;
        double* data = new double[this -> user.get_length()];
        this -> gradient = EmbeddingGradient(this -> user.get_length(),data );
     }

    void Init_Optimizer::add_item(Embedding* item) {
        this -> items -> append(item);
    }

    Embedding Init_Optimizer::output_grad() {
        return this -> gradient;
    }

    void Init_Optimizer::compute() {
        int num_items = this -> items -> get_n_embeddings();
        std::vector<std::thread> thread_list;
        for (int i = 0; i < num_items; i ++) {
            Embedding* item = this -> items -> get_embedding(i);
            thread_list.push_back(std::thread(&Init_Optimizer::single_thread, this, item));
        }

        for (int i = 0; i < num_items; i ++) {
            thread_list[i].join();
        }
    }

    void Init_Optimizer::single_thread(Embedding* item) {
        EmbeddingGradient* grad = cold_start(&this -> user, item);
        // std::cout<<"grad:"<<std::endl;
        // grad->write_to_stdout();
        this -> mux.lock();
        this -> gradient = this -> gradient + grad;
        // std::cout<<"gradient:"<<std::endl;
        // this -> gradient.write_to_stdout();
        this -> mux.unlock();
    }
}