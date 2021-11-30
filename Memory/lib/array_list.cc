#include "array_list.h"

#include "memory_manager.h"

namespace proj3 {
    ArrayList::ArrayList(size_t sz, MemoryManager* cur_mma, int id){
        //std::cout << "arraylist "<<id<<" created"<<std::endl;
        this -> mma = cur_mma;
        this -> array_id = id;

    }
    int ArrayList::Read (unsigned long idx){
        //read the value in the virtual index of 'idx' from mma's memory space
        //std::cout << "arraylist "<<this -> array_id<<" Read virtual address" << idx<<std::endl;
        return this -> mma -> ReadPage(this -> array_id, idx / PageSize, idx % PageSize);
    }
    void ArrayList::Write (unsigned long idx, int value){
        //std::cout << "arraylist "<<this -> array_id<<" write "<< value <<" to virtual address " << idx<<std::endl;
        //write 'value' in the virtual index of 'idx' into mma's memory space
        this -> mma -> WritePage(this -> array_id, idx/PageSize, idx % PageSize, value);
    }
    ArrayList::~ArrayList(){
        //std::cout << "arraylist "<<this -> array_id<<" destroied" <<std::endl;
        this -> mma -> Release(this);
    }
} // namespace: proj3