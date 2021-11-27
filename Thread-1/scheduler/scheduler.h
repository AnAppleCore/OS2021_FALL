#ifndef THREAD_SCHEDULER_H_
#define THREAD_SCHEDULER_H_

#include <string>
#include <vector>

//#include <thread>
#include <mutex>
//#include <condition_variable>

#include "../lib/embedding.h"
#include "../lib/instruction.h"
#include "../lib/model.h"

namespace proj1 {
    
    enum Thread_Status {
        WAITING = 0,
        RUNNING,
        TERMINATED
    };
    

    class Scheduler {
        public:
            Scheduler(std::string users_filename,std::string items_filename, std::string instructions_filename);
            void sort_instructions();
            //(init)*(recommand-1)*(update0)*(recommand0)*(update1)*(recommand1)*......
            //also update max_epoch_num
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

        private:
            EmbeddingHolder* users;
            EmbeddingHolder* items;
            Instructions instructions;

            int embedding_length;
            int instruction_num;
            int max_epoch;
            int num_original_users;
            int num_users;
            int num_items;

            //running state
            int first_instruction_idx;//the smallest index of instructions that needs to run
            int last_instruction_idx;//the largest index of instructions that needs to run
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
bool compare_instructions(Instruction, Instruction);
};


#endif // THREAD_LIB_EMBEDDING_H_