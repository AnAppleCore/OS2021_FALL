#include <chrono>
#include <thread>

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
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            for (int i = 0; i < int(PageSize); i ++) {
                ofs << std::to_string(this -> mem[i]) <<"\n";
            }
            ofs.close();
        } else {
            throw std::runtime_error("error openning file "+filename+"!");
        }
    }
    void PageFrame::ReadDisk(std::string filename) {
        // read page content from disk files
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            this -> Clear();
            ifs.close();
            std::ofstream ofs(filename);
            for (int i = 0; i < int(PageSize); i ++) {
                ofs <<"0\n";
            }
            ofs.close();
            return;
        } else {
            std::string line;
            int length = 0;
            while (std::getline(ifs, line) && length < int(PageSize)) {
                this -> mem[length] = atoi(line.data());
                length ++ ;
            }
            ifs.close();
        }
    }
    void PageFrame::Clear() {
        for (size_t i = 0; i < PageSize; i ++){
            this -> mem[i] = 0;
        }
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


    MemoryManager::MemoryManager(size_t sz, ReplacementPolicy Policy, bool Test_mode){
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
        this -> test_mode = Test_mode;
    }
    MemoryManager::~MemoryManager(){  
        
        for (auto it = this -> filename_exist.cbegin(); it != this -> filename_exist.cend(); ++it) {
            if ((*it).second)
                remove(((*it).first).c_str());
        }

        delete this -> free;
        delete this -> used;
        delete this -> modified;
        delete this -> mem;
        delete this -> page_info;
    }
    void MemoryManager::PageOut(int physical_page_id, std::string filename) {

        this -> mem[physical_page_id].WriteDisk(filename);
        this -> filename_exist[filename] = true;
        if (this -> test_mode)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    }
    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id){

        std::string name = file_name(array_id, virtual_page_id);
        this -> mem[physical_page_id].ReadDisk(name);
        this -> filename_exist[name] = true;
        if (this -> test_mode)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }
    int MemoryManager::PageReplace(int array_id, int virtual_page_id, bool is_write,  std::string& output_filename){
        //implement your page replacement policy here
        //try to find a free page
        int i;
        for ( i = 0 ; i < int(this -> mma_sz) ; i ++ ) {
            if (this -> free[i]) break;
        }
        if ( i < int(this -> mma_sz)) {
            //A free page found

            this -> page_map[array_id][virtual_page_id] = i;
            this -> free[i] = false;
            this -> used[i] = true;
            this -> modified[i] = is_write;
            this -> page_info[i].SetInfo(array_id, virtual_page_id);
            //////////////////////////////////
            int times_to_wait = 0;
            this -> resource_queue[page_name(i)].push(array_id);
            if (this -> resource_queue[page_name(i)].size() > 1) {
                times_to_wait ++;
            }
            this -> resource_queue[file_name(array_id, virtual_page_id)].push(array_id);
            if (this -> resource_queue[file_name(array_id, virtual_page_id)].size() > 1){
                times_to_wait ++;
            }
            this -> data_lock.unlock();
            for (int j = 0; j < times_to_wait; j ++) {
                this -> sp[array_id].P();
            }
            //////////////////////////////////

            PageIn(array_id, virtual_page_id, i);
            return i;
        } else {
            if (this -> policy == CLOCK) {
                //clock algorithm
                while(true) {
                    if (this -> used[this -> clock_head]) {
                        this -> used[this -> clock_head] = false;
                        this -> clock_head = (this -> clock_head + 1) % int (this -> mma_sz);
                    } else {
                        break;
                    }
                }
            }
            
            int pid = this -> clock_head;
            this -> clock_head = (this -> clock_head + 1) % int(this ->mma_sz);

            //update page_map
            int old_holder = this -> page_info[pid].GetHolder();
            int old_vid = this -> page_info[pid].GetVid();
            //this -> page_map[old_holder][old_vid] = -1;//Page table cleared
            bool dirty = this -> modified[pid];

            this -> page_info[pid].SetInfo(array_id, virtual_page_id);
            this -> page_map[array_id][virtual_page_id] = pid;
            this -> page_map[old_holder][old_vid] = -1;
            this -> free[pid] = false;
            this -> used[pid] = true;
            this -> modified[pid] = is_write;
            if(dirty)output_filename = file_name(old_holder, old_vid);

            //////////////////////////////////
            int times_to_wait = 0;
            this -> resource_queue[page_name(pid)].push(array_id);
            if (this -> resource_queue[page_name(pid)].size() > 1){
                times_to_wait ++;
            }
            this -> resource_queue[file_name(array_id, virtual_page_id)].push(array_id);
            if (this -> resource_queue[file_name(array_id, virtual_page_id)].size() > 1){
                times_to_wait ++;
            }
            if (dirty) {
                this -> resource_queue[output_filename].push(array_id);
                if (this -> resource_queue[output_filename].size() > 1) {
                    times_to_wait ++;
                }
            }
            this -> data_lock.unlock();
            for (int j = 0; j < times_to_wait; j ++) {
                this -> sp[array_id].P();
            }
            //////////////////////////////////

            if(dirty)this -> PageOut(pid, output_filename);
            this -> PageIn(array_id, virtual_page_id, pid);
            return pid;
        }

    }
    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset){
        // for arrayList of 'array_id', return the target value on its virtual space
        //////////////////////////////////
        this -> data_lock.lock();
        //////////////////////////////////

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
                throw std::runtime_error("Array List " + std::to_string(array_id)+" exceeds the allocated space!");
            }
        } else if (this -> page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        
        if (in_memory) {
            
            int pid = this -> page_map[array_id][virtual_page_id];
            this -> used[pid] = true;

            //////////////////////////////////
            this -> resource_queue[page_name(pid)].push(array_id);
            int queue_size = this -> resource_queue[page_name(pid)].size();
            this -> data_lock.unlock();
            if (queue_size > 1) {
                this -> sp[array_id].P();
            }
            //////////////////////////////////
            int result = (this -> mem[pid])[offset];
            //////////////////////////////////
            this -> data_lock.lock();
            this -> resource_queue[page_name(pid)].pop();
            if (this -> resource_queue[page_name(pid)].size() > 0) {
                this -> sp[this -> resource_queue[page_name(pid)].front()].V();
            }
            this -> data_lock.unlock();
            //////////////////////////////////
            return result; 
        } else {
            std::string input_filename = file_name(array_id, virtual_page_id);
            std::string output_filename;

            int pid = this -> PageReplace(array_id, virtual_page_id, false, output_filename);
            
            int result = (this -> mem[pid])[offset];
            //////////////////////////////////
            this -> data_lock.lock();
            this -> resource_queue[page_name(pid)].pop();
            if (this -> resource_queue[page_name(pid)].size() > 0) {
                this -> sp[this -> resource_queue[page_name(pid)].front()].V();
            }
            this -> resource_queue[input_filename].pop();
            if (this -> resource_queue[input_filename].size() > 0) {
                this -> sp[this -> resource_queue[input_filename].front()].V();
            }

            if (output_filename.size() > 0) {
                this -> resource_queue[output_filename].pop();
                if (this -> resource_queue[output_filename].size() > 0) {
                    this -> sp[this -> resource_queue[output_filename].front()].V();
                }
            }
            this -> data_lock.unlock();
            //////////////////////////////////
            return result; 
        }
    }
    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value){
        // for arrayList of 'array_id', write 'value' into the target position on its virtual space
        //////////////////////////////////
        this -> data_lock.lock();
        //////////////////////////////////
        
        std::map<int, int> empty;
        bool in_memory = true;
        if (this -> page_map.count(array_id) == 0) {
            //the arraylist is first used
            this -> page_map[array_id] = empty;
            in_memory = false;
        } else if (this -> page_map[array_id].count(virtual_page_id) == 0){
            //virtual page is first used
            in_memory = false;
            if (int(this -> page_map[array_id].size()) == this -> num_max_pages[array_id]) {
                throw std::runtime_error("Array List " + std::to_string(array_id)+" exceeds the allocated space!");
            } 
        } else if (this -> page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        
        if (in_memory) {
            int pid = this -> page_map[array_id][virtual_page_id];
            this -> used[pid] = true;
            this -> modified[pid] = true;
            
            //////////////////////////////////
            this -> resource_queue[page_name(pid)].push(array_id);
            int queue_size = this -> resource_queue[page_name(pid)].size();
            this -> data_lock.unlock();
            if (queue_size > 1) {
                this -> sp[array_id].P();
            }
            //////////////////////////////////

            (this -> mem[pid])[offset] = value;

            //////////////////////////////////
            this -> data_lock.lock();
            this -> resource_queue[page_name(pid)].pop();
            if (this -> resource_queue[page_name(pid)].size() > 0) {
                this -> sp[this -> resource_queue[page_name(pid)].front()].V();
            }
            this -> data_lock.unlock();
            ///////////////////////
            
            return; 
        } else {
            std::string input_filename = file_name(array_id, virtual_page_id);
            std::string output_filename ;
            int pid = this -> PageReplace(array_id, virtual_page_id, true, output_filename);//datalock -> pagelock
            (this -> mem[pid])[offset] = value;
            //////////////////////////////////
            this -> data_lock.lock();
            this -> resource_queue[page_name(pid)].pop();
            if (this -> resource_queue[page_name(pid)].size() > 0) {
                this -> sp[this -> resource_queue[page_name(pid)].front()].V();
            }
            this -> resource_queue[input_filename].pop();
            if (this -> resource_queue[input_filename].size() > 0) {
                this -> sp[this -> resource_queue[input_filename].front()].V();
            }

            if (output_filename .size() > 0) {
                this -> resource_queue[output_filename].pop();
                if (this -> resource_queue[output_filename].size() > 0) {
                    this -> sp[this -> resource_queue[output_filename].front()].V();
                }
            }
            this -> data_lock.unlock();
            //////////////////////////////////
            return;
        }
    }
    ArrayList* MemoryManager::Allocate(size_t sz){
        // when an application requires for memory, 
        //create an ArrayList and record mappings
        //from its virtual memory space to the physical memory space
        this -> data_lock.lock();

        this -> num_max_pages[this -> next_array_id] = (int(sz) + int(PageSize) - 1)/int(PageSize);
        this -> next_array_id ++;
        ArrayList* list = new ArrayList(sz, this, this -> next_array_id - 1);

        this -> data_lock.unlock();
        return list;

    }
    void MemoryManager::Release(ArrayList* arr){
        // an application will call release() function when destroying its arrayList
        // release the virtual space of the arrayList and erase the corresponding mappings

        int array_id = arr -> array_id;
        this -> data_lock.lock();
        std::map<int, int>::iterator it = this -> page_map[array_id].begin();
        for (; it != this -> page_map[array_id].end(); ++ it) {
            int vid = it -> first;
            int pid = it -> second;
            std::string name = file_name(array_id, vid);
            remove(name.c_str());
            this -> filename_exist[name] = false;
            if (pid != -1) {
                this -> free[pid] = true;
                this -> used[pid] = false;
                this -> modified[pid] = false;
                this -> page_info[pid].ClearInfo();
            }
        }
        this -> data_lock.unlock();
    }


    std::string file_name(int holder, int vid) {
        return "f" + std::to_string(holder) +'_'+std::to_string(vid);
    }

    std::string page_name(int pid) {
        return "p" + std::to_string(pid);
    }

} // namespce: proj3