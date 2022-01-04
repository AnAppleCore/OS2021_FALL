#include "array_list.h"
#include "mma_client.h"

namespace proj4 {
    ArrayList::ArrayList(size_t sz, MmaClient* cur_mma, int id){
        this -> mma_client = cur_mma;
        this -> array_id = id;
        this -> size = sz;

    }
    int ArrayList::Read (unsigned long idx){
        //read the value in the virtual index of 'idx' from mma's memory space
        return mma_client -> ReadPage(this -> array_id, idx / PageSize, idx % PageSize);
    }
    void ArrayList::Write (unsigned long idx, int value){
        //write 'value' in the virtual index of 'idx' into mma's memory space
        mma_client -> WritePage(this -> array_id, idx/PageSize, idx % PageSize, value);
    }
    ArrayList::~ArrayList(){
    }
} // namespace: proj4