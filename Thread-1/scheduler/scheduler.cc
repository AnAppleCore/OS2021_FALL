#include <string>
#include <vector>
#include <iostream>

#include <thread>
#include <mutex>



#include "scheduler.h"
#include "../lib/embedding.h"
#include "../lib/instruction.h"
#include "../lib/model.h"
#include "../init_optimizer/init_optimizer.h"

#include "../update_optimizer/update_optimizer.h"

namespace proj1{
    Scheduler::Scheduler(std::string users_filename, std::string items_filename,std::string instructions_filename) {

        this->users = new EmbeddingHolder(users_filename);
        this->items = new EmbeddingHolder(items_filename);
        this->instructions = read_instructions(instructions_filename);

        this->embedding_length = this->users->get_emb_length();

        int num_init = 0; //count #init instructions
        this-> max_epoch = 0;
        this -> instruction_num = this->instructions.size();
        this -> first_instruction_idx = 0;
        this -> last_instruction_idx = 0;
        this -> current_epoch_num = -2;//the epch number of init
        //count #init instructions, max_epoch
        for (Instruction inst: this -> instructions) {
            int epoch = 0;
            if (inst.order == INIT_EMB) {
                num_init ++;
            } else {
                if (inst.order == UPDATE_EMB) {
                    if (inst.payloads.size() > 3){
                        epoch = inst.payloads[3];
                    } else {
                        epoch = 0;
                    }
                } else {
                    epoch = inst.payloads[1];
                }
            }
            if (epoch > this->max_epoch) this->max_epoch = epoch;
        }
        this -> num_original_users = this-> users->get_n_embeddings();
        this -> num_users = this -> num_original_users + num_init;
        this -> num_items = this->items->get_n_embeddings();

        //initialize all new user embeddings
        //cold starts will be executed later
        for (int i = 0; i < num_init; i ++){
            Embedding* new_user = new Embedding(this -> embedding_length);
            this -> users -> append(new_user);
        }

        //initialize the metadata
        this -> thread_status = std::vector<Thread_Status>(this -> instruction_num);
        for (int i = 0; i < this->instruction_num; i ++) {
            this -> thread_status[i] = WAITING;
        }

        this -> users_free = std::vector<bool>(this->num_users);
        for (int i = 0; i < this -> num_users; i ++) {
            this -> users_free[i] = true;
        }

        this -> items_free = std::vector<bool>(this->num_items);
        for (int i = 0; i < this -> num_items; i ++) {
            this -> items_free[i] = true;
        }

        this -> current_instruction_order = INIT_EMB;

       

        
    }

    void Scheduler::sort_instructions() {
        Instruction temp(" ");
        bool changed = true;
        while (changed) {
            changed = false;
            for(int i = 0; i < this -> instruction_num -1; i ++) {
                if (! compare_instructions(this -> instructions[i],this -> instructions[i+1])){
                    temp = this -> instructions[i];
                    this -> instructions[i] = this -> instructions[i+1];
                    this -> instructions[i+1] = temp;

                    changed = true;
                }
            }
        }
    }

    //A comperison function used for sorting
    bool compare_instructions(Instruction inst1, Instruction inst2) {
        
        if (inst1.order == INIT_EMB) {
            return true;
        }
        if (inst2.order == INIT_EMB) {
            return false;
        }
        int epoch1,epoch2;
        if(inst1.order == UPDATE_EMB) {
            epoch1 = inst1.payloads[3];
        } else {
            epoch1 = inst1.payloads[1];
        }
        if(inst2.order == UPDATE_EMB) {
            epoch2 = inst2.payloads[3];
        } else {
            epoch2 = inst2.payloads[1];
        }
        if (epoch1 < epoch2)return true;
        if (epoch1 > epoch2)return false;
        if (inst1.order ==UPDATE_EMB) return true;
        if (inst2.order ==RECOMMEND) return true;
        return false;

    }


    void Scheduler::execute_all() {
        if (this -> instruction_num == 0)return ;
        //start executing instruction blocks of the same type
        this -> sort_instructions();
        this -> first_instruction_idx = 0;
        this -> last_instruction_idx = 0;
        while (this -> first_instruction_idx < this -> instruction_num) {
            this -> current_instruction_order = this -> instructions[this -> first_instruction_idx].order;
            this -> last_instruction_idx = this -> first_instruction_idx;
            this -> current_epoch_num = this -> get_epoch_num(this -> first_instruction_idx);
            while(this -> instructions[this->last_instruction_idx].order == this -> current_instruction_order &&
                    this -> last_instruction_idx < this -> instruction_num &&
                    this -> get_epoch_num(this -> last_instruction_idx) == this -> current_epoch_num) {
                this -> last_instruction_idx ++ ;
            }
            this -> dispatch_instructions();
            this -> first_instruction_idx = this -> last_instruction_idx;
        }
    }
    void Scheduler::execute_one_instruction(int idx) {
        Instruction inst = this -> instructions[idx];
        
        switch(inst.order) {
        case INIT_EMB: {
            int user_idx = this -> num_original_users + idx;
            Embedding new_user = *this -> users -> get_embedding(user_idx);
            Init_Optimizer optimizer (new_user);
            for (int item_index: inst.payloads) {
                optimizer.add_item(this -> items -> get_embedding(item_index));
            }
            optimizer.compute();
            EmbeddingGradient gradient = optimizer.output_grad();
            this -> users -> update_embedding(user_idx, &gradient, 0.01);
            break;
            
            /* linear version
            int user_idx = this -> num_original_users + idx;
            for (int item_index: inst.payloads) {
                Embedding* item_emb = this -> items->get_embedding(item_index);
                // Call cold start for downstream applications, slow
                //Embedding* item = this -> items -> get_embedding(user_idx);
                EmbeddingGradient* gradient = cold_start(this -> items -> get_embedding(item_index), item_emb);
                this -> users->update_embedding(user_idx, gradient, 0.01);
                delete gradient;
            }
            break;*/
            
        }
        case UPDATE_EMB: {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];
            Embedding user = *this -> users -> get_embedding(user_idx);
            Embedding item = *this -> items -> get_embedding(item_idx);
            Update_Optimizer optimizer(user,item,label);
            optimizer.compute();

            EmbeddingGradient grad1 = optimizer.output_grad1();
            EmbeddingGradient grad2 = optimizer.output_grad2();

            this -> users->update_embedding(user_idx,&grad1,0.01);
            this -> items->update_embedding(item_idx,&grad2,0.001);
            break;
        }
        case RECOMMEND: {
            int user_idx = inst.payloads[0];
            Embedding* user = this->users->get_embedding(user_idx);
            std::vector<Embedding*> item_pool;
            for (unsigned int i = 2; i < inst.payloads.size(); ++i) {
                int item_idx = inst.payloads[i];
                item_pool.push_back(this->items->get_embedding(item_idx));
            }
            Embedding* recommendation = recommend(user, item_pool);
            this -> mux.lock();
            recommendation->write_to_stdout();
            this -> mux.unlock();
            break;
        }

        }
    }
   

    void Scheduler::dispatch_instructions() {
        this -> mux.lock();
        if (this -> all_terminated()) {
            this -> mux.unlock();
            return;
        }
        std::vector<int> instruction_idx_list = this -> find_instructions();
        std::vector<std::thread> thread_list;
        for(unsigned int i = 0; i < instruction_idx_list.size(); i ++) {
            thread_list.push_back(std::thread(&Scheduler::single_thread, this, instruction_idx_list[i]));
            this -> thread_status[instruction_idx_list[i]] = RUNNING;
        }
        this->mux.unlock();
        for(unsigned int i = 0; i < instruction_idx_list.size(); i ++) {
            thread_list[i].join();
        }

    }
    void Scheduler::release_resource(int instruction_idx) {
        InstructionOrder order = this -> instructions[instruction_idx].order;
        this -> mux.lock();
        switch (order) {
            case RECOMMEND:{
                this -> thread_status[instruction_idx] = TERMINATED;
                break;
            }
            case INIT_EMB: {
                this -> thread_status[instruction_idx] = TERMINATED;
                int user_involved = this -> user_involved(instruction_idx);
                this -> users_free[user_involved] = true;
                break;
            }
            case UPDATE_EMB: {
                this -> thread_status[instruction_idx] = TERMINATED;
                int user_involved = this -> user_involved(instruction_idx);
                this -> users_free[user_involved] = true;
                std::vector<int> items_involved = this -> items_involved(instruction_idx);
                for (int item_idx : items_involved) {
                    this -> items_free[item_idx] = true;
                }
                break;
            }
        }

        this -> mux.unlock();
    }

    int Scheduler::get_epoch_num (int instruction_idx) {
        InstructionOrder order = this -> instructions[instruction_idx].order;
        switch (order) {
            case RECOMMEND: {
                return this -> instructions[instruction_idx].payloads[1];
            }
            case UPDATE_EMB: {
                Instruction inst = this -> instructions[instruction_idx];
                if (inst.payloads.size()>3) {
                    return inst.payloads[3];
                } else {
                    return 0;
                }
            }
            case INIT_EMB: {
                return -2;
            }
        }
        return 0;
    }

    std::vector<int> Scheduler::find_instructions() {
        std::vector<int> instruction_list;
        int first = this -> first_instruction_idx;
        int last = this -> last_instruction_idx;
        for (int i = first; i < last; i ++ ) {
            if (this -> thread_status[i] == WAITING) {
                switch (this->current_instruction_order) {
                    case RECOMMEND: {
                        instruction_list.push_back(i);
                        break;
                    }
                    case INIT_EMB: {
                        int user_involved = this -> user_involved(i);
                        if (this -> users_free[user_involved]) {
                            this -> users_free[user_involved] = false;
                            instruction_list.push_back(i);
                        }
                        break;
                    }
                    case UPDATE_EMB: {
                        int user_involved = this -> user_involved(i);
                        std::vector<int> items_involved = this -> items_involved(i);
                        if ( ! this -> users_free[user_involved] ) break;
                        for (int item : items_involved) {
                            if( ! this -> items_free[item]) break;
                        }
                        this -> users_free[user_involved] = false;
                        for (int item : items_involved) {
                            this -> items_free[item] = false;
                        }
                        instruction_list.push_back(i);
                        break;
                    }

                }
                
            }
        }
        return instruction_list;

    };


    void Scheduler::single_thread(int instruction_idx) {
        this -> execute_one_instruction(instruction_idx);
        this -> release_resource(instruction_idx);
        this -> dispatch_instructions();
    };


    int Scheduler::user_involved(int instruction_idx){
        Instruction inst = this -> instructions[instruction_idx];
        if (inst.order == INIT_EMB) return instruction_idx + this -> num_original_users;
        else return inst.payloads[0];
    }
    std::vector<int> Scheduler::items_involved(int instruction_idx){
        Instruction inst = this -> instructions[instruction_idx];
        switch( inst.order ) {
            case INIT_EMB : {
                return inst.payloads;
            }
                
            case UPDATE_EMB :{
                int item_idx = inst.payloads[1];
                return std::vector<int>(1, item_idx);
            }
            case RECOMMEND : {
                std::vector<int> item_pool_idx;
                for (unsigned int i = 2; i < inst.payloads.size(); ++ i) {
                    item_pool_idx.push_back(inst.payloads[i]);
                }
                return item_pool_idx;
            }
            default : return inst.payloads;     
        }
    }

    bool Scheduler::all_terminated() {
        for (int i = this -> first_instruction_idx; i < this->last_instruction_idx; i ++) {
            if (this -> thread_status[i] != TERMINATED) return false;
        }
        return true;
    }
}//namespace proj1