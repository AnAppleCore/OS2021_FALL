#include "memory_manager.h"

#include "array_list.h"

namespace proj3 {
    PageFrame::PageFrame(){
    }
    int& PageFrame::operator[] (unsigned long idx){
        //each page should provide random access like an array
        return this -> mem[idx];
    }
    void PageFrame::WriteDisk(std::string filename) {
        // write page content into disk files
        //std::cout <<"WriteDisk: "<<filename<<std::endl;
        const char* name = filename.data();
        FILE * pagefile;
        pagefile = fopen(name, "r+b");
        if (pagefile == NULL) {
            //std::cout <<"WriteDisk error "<<std::endl;
            throw std::runtime_error ("Error openning file "+ filename + "!");
        }
        //std::cout <<"WriteDisk create file: "<<filename<<std::endl;
        fwrite(this -> mem, sizeof(int), PageSize, pagefile);
        //std::cout <<"WriteDisk write file: "<<filename<<std::endl;
        fclose(pagefile);
    }
    void PageFrame::ReadDisk(std::string filename) {
        // read page content from disk files
        //std::cout <<"ReadDisk: "<<filename<<std::endl;
        const char* name = filename.data();
        FILE * pagefile;
        pagefile = fopen(name, "rb");

        if (pagefile == NULL) {
            //file not created yet
            //std::cout <<"ReadDisk: Create File: "<<filename<<std::endl;
            pagefile = fopen(name, "wb+");//create file
            fclose(pagefile);
            this -> Clear();
            this -> WriteDisk(name);
            return;
        }
        size_t result = fread(this -> mem, sizeof(int), PageSize, pagefile);
        if (result != PageSize) {
            //std::cout <<"ReadDisk Error: result "<<result <<std::endl;
            throw std::runtime_error ("Error reading file "+ filename + "!");
        }
        fclose(pagefile);
    }

    void PageFrame::Clear() {
        //std::cout <<"Clear Memory"<<std::endl;
        for (size_t i = 0; i < PageSize; i ++){
            this -> mem[i] = 0;
        }
    }

    void ClearDisk(std::string filename) {
        // write zeros into disk files
        //std::cout <<"Clear Disk "<< filename<<std::endl;
        const char* name = filename.data();
        FILE * pagefile;
        pagefile = fopen(name, "r+b");
        if (pagefile == NULL) {
            throw std::runtime_error ("Error openning file "+ filename + "!");
        }
        int* zeros = new int[PageSize];
        fwrite(zeros, sizeof(int), PageSize, pagefile);
        delete zeros;
        fclose(pagefile);
    }
    PageInfo::PageInfo(){
        this -> holder = -1;
        this -> virtual_page_id = -1;
    }
    void PageInfo::SetInfo(int cur_holder, int cur_vid){
        //modify the page states
        //you can add extra parameters if needed
        this -> holder = cur_holder;
        this -> virtual_page_id = cur_vid;
    }
    void PageInfo::ClearInfo(){
        //clear the page states
        //you can add extra parameters if needed
        this -> holder = -1;
        this -> virtual_page_id = -1;
    }

    int PageInfo::GetHolder(){return this -> holder;}
    int PageInfo::GetVid(){return this -> virtual_page_id;}
    

    MemoryManager::MemoryManager(size_t sz, ReplacementPolicy Policy){
        this -> mma_sz = sz;
        this -> mem = new PageFrame[sz];
        this -> page_info = new PageInfo[sz];
        this -> free = new bool[sz];
        this -> used = new bool[sz];
        this -> modified = new bool[sz];

        for (int i = 0 ; i < int(sz); i ++){
            this -> free[i] = true;
            this -> used[i] = false;
            this -> modified[i] = false;
        }
        this -> policy = Policy;
    }
    MemoryManager::~MemoryManager(){        

        delete this -> free;
        delete this -> used;
        delete this -> modified;
        delete this -> mem;
        delete this -> page_info;
    }
    void MemoryManager::PageOut(int physical_page_id){
        //swap out the physical page with the indx of 'physical_page_id' out into a disk file
        //std::cout <<"Page out PID: "<< physical_page_id<<std::endl;
        if( this -> modified[physical_page_id] ) {
            int holder = this -> page_info[physical_page_id].GetHolder();
            int vid = this -> page_info[physical_page_id].GetVid();
            std::string name = filename(holder, vid);
            this -> mem[physical_page_id] . WriteDisk(name);
        }
        this -> free[physical_page_id] = true;
        this -> used[physical_page_id] = false;
        this -> modified[physical_page_id] = false;
    }
    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id){
        //swap the target page from the disk file into a physical page with the index of 'physical_page_id" out
        //std::cout <<"PageIn FILENAME: "<<filename(array_id, virtual_page_id)<<" PID "<<physical_page_id<<std::endl;
        this -> PageOut(physical_page_id);
        std::string name = filename(array_id, virtual_page_id);
        this -> mem[physical_page_id] . ReadDisk(name);
        this -> free[physical_page_id] = false;
        this -> used[physical_page_id] = false;
        this -> modified[physical_page_id] = false;
    }
    void MemoryManager::PageReplace(int array_id, int virtual_page_id){
        //implement your page replacement policy here
        //try to find a free page
        int i;
        for ( i = 0 ; i < int(this -> mma_sz) ; i ++ ) {
            if (this -> free[i]) break;
        }
        if ( i < int(this -> mma_sz)) {
            //A free page found
            //std::cout << "Found free page "<<i<<std::endl;
            PageIn(array_id, virtual_page_id, i);
            this -> page_map[array_id][virtual_page_id] = i;
            this -> free[i] = false;
            this -> used[i] = false;
            this -> modified[i] = false;

            this -> page_info[i].SetInfo(array_id, virtual_page_id);
            return;
        } else {
            if (this -> policy == CLOCK) {
                //clock algorithm
                while(true) {
                    if (this -> used[this -> clock_head]) {
                        this -> used[this ->clock_head] = false;
                        this -> clock_head = (this -> clock_head + 1)% int (this -> mma_sz);
                    } else {
                        break;
                    }
                }
            }
            //update page_map
            int old_holder = this -> page_info[this -> clock_head].GetHolder();
            int old_vid = this -> page_info[this -> clock_head].GetVid();
            //this -> page_map[old_holder][old_vid] = -1;//Page table cleared
            this -> page_map[array_id][virtual_page_id] = this -> clock_head;
            //evict page
            this -> PageIn(array_id, virtual_page_id, this -> clock_head);
            this -> page_info[this -> clock_head].SetInfo(array_id, virtual_page_id);
            this -> page_map[old_holder][old_vid] = -1;
            this -> free[this -> clock_head] = false;
            this -> used[this -> clock_head] = false;
            this -> modified[this -> clock_head] = false;
            this -> clock_head = (this -> clock_head + 1)% int (this -> mma_sz);
            
        }

    }
    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset){
        // for arrayList of 'array_id', return the target value on its virtual space
        std::map<int, int> empty;
        bool in_memory = true;
        if (this -> page_map.count(array_id) == 0) {
            //the arraylist is first used
            this -> page_map[array_id] = empty;
            in_memory = false;
        } else if (this -> page_map[array_id].count(virtual_page_id) == 0){
            in_memory = false;
            //virtual page is first used
            if (int(this -> page_map[array_id].size()) == this -> num_max_pages[array_id]) {
                throw std::runtime_error("Array List exceeds the allocated space");
            }
        } else if (this -> page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        
        if (in_memory) {
            int pid = this -> page_map[array_id][virtual_page_id];
            int result = (this -> mem[pid])[offset];
            this -> used[pid] = true;
            return result; 
        } else {
            this -> PageReplace(array_id, virtual_page_id);
            int pid = this -> page_map[array_id][virtual_page_id];
            int result = (this -> mem[pid])[offset];
            this -> used[pid] = true;
            return result; 
        }
    }
    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value){
        // for arrayList of 'array_id', write 'value' into the target position on its virtual space
        std::map<int, int> empty;
        bool in_memory = true;
        if (this -> page_map.count(array_id) == 0) {
            //the arraylist is first used
            this -> page_map[array_id] = empty;
            in_memory = false;
        } else if (this -> page_map[array_id].count(virtual_page_id) == 0){
            in_memory = false;
        } else if (this -> page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        
        if (in_memory) {
            int pid = this -> page_map[array_id][virtual_page_id];
            (this -> mem[pid])[offset] = value;
            this -> used[pid] = true;
            this -> modified[pid] = true;
            return; 
        } else {
            this -> PageReplace(array_id, virtual_page_id);
            int pid = this -> page_map[array_id][virtual_page_id];
            (this -> mem[pid])[offset] = value;
            this -> used[pid] = true;
            this -> modified[pid] = true;
            return;
        }
    }
    ArrayList* MemoryManager::Allocate(size_t sz){
        // when an application requires for memory, 
        //create an ArrayList and record mappings
        //from its virtual memory space to the physical memory space
        //std::cout << "Allocate an arrayList "<<this -> next_array_id<<std::endl;
        this -> num_max_pages[this -> next_array_id] = int(sz);
        this -> next_array_id ++;
        ArrayList* list = new ArrayList(sz, this, this -> next_array_id - 1);
        return list;

    }
    void MemoryManager::Release(ArrayList* arr){
        // an application will call release() function when destroying its arrayList
        // release the virtual space of the arrayList and erase the corresponding mappings
        //std::cout << "Release an arrayList "<<arr -> array_id<<std::endl;
        int array_id = arr -> array_id;
        std::map<int, int>::iterator it = this -> page_map[array_id].begin();
        for (; it != this -> page_map[array_id].end(); ++ it) {
            int vid = it -> first;
            int pid = it -> second;
            const char* name = filename(array_id, vid).data();
            remove(name);
            if (pid != -1) {
                this -> free[pid] = true;
                this -> used[pid] = false;
                this -> modified[pid] = false;
                this -> page_info[pid].ClearInfo();
            }
        }
    }

    

    std::string filename(int holder, int vid) {
        return std::to_string(holder) +'_'+std::to_string(vid);
    }

    
} // namespce: proj3