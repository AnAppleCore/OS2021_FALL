#include "array_list.h"

#include "memory_manager.h"

namespace proj3 {
    ArrayList::ArrayList(size_t sz, MemoryManager* cur_mma, int id){
        this -> mma = cur_mma;
        this -> array_id = id;
        this -> size = sz;

    }
    int ArrayList::Read (unsigned long idx){
        //read the value in the virtual index of 'idx' from mma's memory space
        return mma -> ReadPage(this -> array_id, idx / PageSize, idx % PageSize);
    }
    void ArrayList::Write (unsigned long idx, int value){
        //write 'value' in the virtual index of 'idx' into mma's memory space
        mma -> WritePage(this -> array_id, idx/PageSize, idx % PageSize, value);
    }
    ArrayList::~ArrayList(){
        mma -> Release(this);
    }
} // namespace: proj3