#ifndef THREAD_SCHEDULER_H_
#define THREAD_SCHEDULER_H_

#include <string>
#include <vector>
#include <mutex>
#include <stdlib.h>

#include "../lib/embedding.h"
#include "../lib/instruction.h"
#include "../lib/model.h"

namespace proj1 {

    using EmbeddingStack = std::vector<Embedding>;
    
    enum Thread_Status {
        WAITING =     0,
        RUNNING,   // 1
        TERMINATED // 2
    };


    // A scheduler that optimizes the execution order to run updates and recommendations concurrently
    class Optimizer {
        public:
            Optimizer(std::string users_filename,std::string items_filename, std::string instructions_filename);
            void sort_instructions();
            //(init)*(recommand-1)*(update0)*(recommand0)*(update1)*(recommand1)*......
            //also update max_epoch_num
            void store_snapshot();
            void execute_all();
            //start executing all instructions
            //Do not write to stdout
            void execute_one_instruction(int idx);//executing one instruction, not thread safe
            void dispatch_instructions();
            void single_thread(int idx);

            std::vector<int> find_instructions();
            //return the indices of instructions that can be executed concurrently
            //if recommend, then execute all
            //if init, only consider user dependencies
            //if update, consider all dependencies

            int user_involved(int instruction_idx);
            std::vector<int> items_involved(int instr_idx);
            bool all_terminated();//num must be locked
            void release_resource(int instruction_idx);

            int get_epoch_num(int instruction_idx);

            void write_to_stdout() {
                this -> users -> write_to_stdout();
                this -> items -> write_to_stdout();
            };

            void clear(){
                delete this -> users;
                delete this -> items;
            }

        private:
            EmbeddingHolder* users;
            EmbeddingHolder* items;

            // store the data of last epoch
            EmbeddingStack old_users;
            EmbeddingStack old_items;

            Instructions instructions;

            int embedding_length;
            int instruction_num;
            int max_epoch;
            int num_original_users;
            int num_users;
            int num_items;

            //running state
            int first_instruction_idx; //the smallest index of instructions that needs to run
            int last_instruction_idx;  //the largest index of instructions that needs to run
            int last_epoch_num;
            int current_epoch_num;

            std::mutex mux;//protect the following members
            /////////////////metadata//////////////////////
            //std::vector<std::thread>* threads;
            std::vector<Thread_Status> thread_status;//record number
            std::vector<bool> users_free;
            std::vector<bool> items_free;
            InstructionOrder current_instruction_order;
            //////////////////////////////////////////////

            //std::condition_variable finished;
    };


    class Init {
        public:
            Init(Embedding newuser);
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


    class Update {
        public:
            Update(Embedding*, Embedding*, int);

            void compute();
            EmbeddingGradient output_grad1();
            EmbeddingGradient output_grad2();

        private:
            void calc1();
            void calc2();
            Embedding* A;
            Embedding* B;
            int label;
            EmbeddingGradient grad1;
            EmbeddingGradient grad2;
    };


    bool compare_instructions(Instruction, Instruction);

    EmbeddingStack copy_holder(EmbeddingHolder*);

}; // namespace proj1


#endif // THREAD_LIB_EMBEDDING_H_