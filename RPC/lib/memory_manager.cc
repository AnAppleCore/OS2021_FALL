#include <string.h>

#include "memory_manager.h"

namespace proj4 {
    PageFrame::PageFrame(){
    }

    int& PageFrame::operator[] (unsigned long idx){
        //each page should provide random access like an array
        return mem[idx];
    }

    void PageFrame::WriteDisk(std::string filename) {    
        FILE* f = fopen(filename.c_str(), "wb");
        if(f == nullptr){
            throw std::runtime_error("error openning file "+filename+"!");
        } else {
            fwrite(mem, sizeof(int), PageSize, f);
            fclose(f);
        }
    }

    void PageFrame::ReadDisk(std::string filename) {
        FILE* f = fopen(filename.c_str(),"rb");
        if (f == nullptr) {
            memset(mem,0,sizeof(int)*PageSize);
            f = fopen(filename.c_str(),"wb");
            fwrite(mem, sizeof(int), PageSize, f);
            fclose(f);
        } else {
            fread(mem, sizeof(int), PageSize, f);
            fclose(f);
        }        
    }
    
    PageInfo::PageInfo(){
        holder = -1;
        virtual_page_id = -1;
    }

    void PageInfo::SetInfo(int cur_holder, int cur_vid){
        //modify the page states
        //you can add extra parameters if needed
        holder = cur_holder;
        virtual_page_id = cur_vid;
    }

    void PageInfo::ClearInfo(){
        //clear the page states
        //you can add extra parameters if needed
        holder = -1;
        virtual_page_id = -1;
    }

    int PageInfo::GetHolder(){return holder;}
    int PageInfo::GetVid(){return virtual_page_id;}

    MemoryManager::MemoryManager(size_t sz, ReplacementPolicy Policy){
        mma_sz = sz;
        mem = new PageFrame[sz];
        page_info = new PageInfo[sz];
        free = new bool[sz];
        used = new bool[sz];
        modified = new bool[sz];
        for (int i = 0 ; i < int(sz); i ++){
            free[i] = true;
            used[i] = false;
            modified[i] = false;
        }
        policy = Policy;
    }

    MemoryManager::~MemoryManager(){  
        for (auto it = filename_exist.cbegin(); it != filename_exist.cend(); ++it) {
            if ((*it).second)
                remove(((*it).first).c_str());
        }
        delete free;
        delete used;
        delete modified;
        delete mem;
        delete page_info;
    }

    void MemoryManager::PageOut(int physical_page_id, std::string filename) {
        mem[physical_page_id].WriteDisk(filename);
        filename_exist[filename] = true;
    }

    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id){
        std::string name = file_name(array_id, virtual_page_id);
        mem[physical_page_id].ReadDisk(name);
        filename_exist[name] = true;
    }

    int MemoryManager::PageReplace(int array_id, int virtual_page_id, bool is_write,  std::string& output_filename){
        //implement your page replacement policy here
        //try to find a free page
        int i;
        for ( i = 0 ; i < int(mma_sz) ; i ++ ) {
            if (free[i]) break;
        }
        if ( i < int(mma_sz)) {
            //A free page found
            page_map[array_id][virtual_page_id] = i;
            free[i] = false;
            used[i] = true;
            modified[i] = is_write;
            page_info[i].SetInfo(array_id, virtual_page_id);
            //////////////////////////////////
            int times_to_wait = 0;
            resource_queue[page_name(i)].push(array_id);
            if (resource_queue[page_name(i)].size() > 1) {
                times_to_wait ++;
            }
            resource_queue[file_name(array_id, virtual_page_id)].push(array_id);
            if (resource_queue[file_name(array_id, virtual_page_id)].size() > 1){
                times_to_wait ++;
            }
            data_lock.unlock();
            for (int j = 0; j < times_to_wait; j ++) {
                sp[array_id].P();
            }
            //////////////////////////////////
            PageIn(array_id, virtual_page_id, i);
            return i;
        } else {
            if (policy == CLOCK) {
                //clock algorithm
                while(true) {
                    if (used[clock_head]) {
                        used[clock_head] = false;
                        clock_head = (clock_head + 1) % int (mma_sz);
                    } else {
                        break;
                    }
                }
            }
            int pid = clock_head;
            clock_head = (clock_head + 1) % int(mma_sz);
            //update page_map
            int old_holder = page_info[pid].GetHolder();
            int old_vid = page_info[pid].GetVid();
            //page_map[old_holder][old_vid] = -1;//Page table cleared
            bool dirty = modified[pid];
            page_info[pid].SetInfo(array_id, virtual_page_id);
            page_map[array_id][virtual_page_id] = pid;
            page_map[old_holder][old_vid] = -1;
            free[pid] = false;
            used[pid] = true;
            modified[pid] = is_write;
            if(dirty)output_filename = file_name(old_holder, old_vid);
            //////////////////////////////////
            int times_to_wait = 0;
            resource_queue[page_name(pid)].push(array_id);
            if (resource_queue[page_name(pid)].size() > 1){
                times_to_wait ++;
            }
            resource_queue[file_name(array_id, virtual_page_id)].push(array_id);
            if (resource_queue[file_name(array_id, virtual_page_id)].size() > 1){
                times_to_wait ++;
            }
            if (dirty) {
                resource_queue[output_filename].push(array_id);
                if (resource_queue[output_filename].size() > 1) {
                    times_to_wait ++;
                }
            }
            data_lock.unlock();
            for (int j = 0; j < times_to_wait; j ++) {
                sp[array_id].P();
            }
            //////////////////////////////////
            if(dirty)PageOut(pid, output_filename);
            PageIn(array_id, virtual_page_id, pid);
            return pid;
        }

    }

    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset){
        // for arrayList of 'array_id', return the target value on its virtual space
        //////////////////////////////////
        data_lock.lock();
        //////////////////////////////////
        bool in_memory = true;
        if (page_map.count(array_id) == 0) {
            //the arraylist is first used
            page_map[array_id] = std::map<int, int>();
            in_memory = false;
        } else if (page_map[array_id].count(virtual_page_id) == 0){
            in_memory = false;
            //virtual page is first used
            if (int(page_map[array_id].size()) == num_max_pages[array_id]) {
                data_lock.unlock();
                throw std::runtime_error("Array List " + std::to_string(array_id)+" exceeds the allocated space!");
            }
        } else if (page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        if (in_memory) {
            int pid = page_map[array_id][virtual_page_id];
            used[pid] = true;
            //////////////////////////////////
            resource_queue[page_name(pid)].push(array_id);
            int queue_size = resource_queue[page_name(pid)].size();
            data_lock.unlock();
            if (queue_size > 1) {
                sp[array_id].P();
            }
            //////////////////////////////////
            int result = (mem[pid])[offset];
            //////////////////////////////////
            data_lock.lock();
            resource_queue[page_name(pid)].pop();
            if (resource_queue[page_name(pid)].size() > 0) {
                sp[resource_queue[page_name(pid)].front()].V();
            }
            data_lock.unlock();
            //////////////////////////////////
            return result; 
        } else {
            std::string input_filename = file_name(array_id, virtual_page_id);
            std::string output_filename;

            int pid = PageReplace(array_id, virtual_page_id, false, output_filename);
            
            int result = (mem[pid])[offset];
            //////////////////////////////////
            data_lock.lock();
            resource_queue[page_name(pid)].pop();
            if (resource_queue[page_name(pid)].size() > 0) {
                sp[resource_queue[page_name(pid)].front()].V();
            }
            resource_queue[input_filename].pop();
            if (resource_queue[input_filename].size() > 0) {
                sp[resource_queue[input_filename].front()].V();
            }

            if (output_filename.size() > 0) {
                resource_queue[output_filename].pop();
                if (resource_queue[output_filename].size() > 0) {
                    sp[resource_queue[output_filename].front()].V();
                }
            }
            data_lock.unlock();
            //////////////////////////////////
            return result; 
        }
    }

    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value){
        // for arrayList of 'array_id', write 'value' into the target position on its virtual space
        //////////////////////////////////
        data_lock.lock();
        //////////////////////////////////
        
        bool in_memory = true;
        if (page_map.count(array_id) == 0) {
            //the arraylist is first used
            page_map[array_id] = std::map<int, int>();
            in_memory = false;
        } else if (page_map[array_id].count(virtual_page_id) == 0){
            //virtual page is first used
            in_memory = false;
            if (int(page_map[array_id].size()) == num_max_pages[array_id]) {
                data_lock.unlock();
                throw std::runtime_error("Array List " + std::to_string(array_id)+" exceeds the allocated space!");
            } 
        } else if (page_map[array_id][virtual_page_id] == -1) {
            in_memory = false;
        }
        
        if (in_memory) {
            int pid = page_map[array_id][virtual_page_id];
            used[pid] = true;
            modified[pid] = true;
            
            //////////////////////////////////
            resource_queue[page_name(pid)].push(array_id);
            int queue_size = resource_queue[page_name(pid)].size();
            data_lock.unlock();
            if (queue_size > 1) {
                sp[array_id].P();
            }
            //////////////////////////////////

            (mem[pid])[offset] = value;

            //////////////////////////////////
            data_lock.lock();
            resource_queue[page_name(pid)].pop();
            if (resource_queue[page_name(pid)].size() > 0) {
                sp[resource_queue[page_name(pid)].front()].V();
            }
            data_lock.unlock();
            //////////////////////////////////
            
            return; 
        } else {
            std::string input_filename = file_name(array_id, virtual_page_id);
            std::string output_filename ;
            int pid = PageReplace(array_id, virtual_page_id, true, output_filename);//datalock -> pagelock
            (mem[pid])[offset] = value;
            //////////////////////////////////
            data_lock.lock();
            resource_queue[page_name(pid)].pop();
            if (resource_queue[page_name(pid)].size() > 0) {
                sp[resource_queue[page_name(pid)].front()].V();
            }
            resource_queue[input_filename].pop();
            if (resource_queue[input_filename].size() > 0) {
                sp[resource_queue[input_filename].front()].V();
            }

            if (output_filename.size() > 0) {
                resource_queue[output_filename].pop();
                if (resource_queue[output_filename].size() > 0) {
                    sp[resource_queue[output_filename].front()].V();
                }
            }
            data_lock.unlock();
            //////////////////////////////////
            return;
        }
    }

    int MemoryManager::Allocate(size_t sz){
        // when an application requires for memory, 
        //create an ArrayList and record mappings
        //from its virtual memory space to the physical memory space
        data_lock.lock();

        num_max_pages[next_array_id] = (int(sz) + int(PageSize) - 1)/int(PageSize);
        int array_id = next_array_id;
        next_array_id ++;

        data_lock.unlock();
        return array_id;
    }
    size_t MemoryManager::Release(int array_id){
        // an application will call release() function when destroying its arrayList
        // release the virtual space of the arrayList and erase the corresponding mappings

        data_lock.lock();
        std::map<int, int>::iterator it = page_map[array_id].begin();
        for (; it != page_map[array_id].end(); ++ it) {
            int vid = it -> first;
            int pid = it -> second;
            std::string name = file_name(array_id, vid);
            remove(name.c_str());
            filename_exist[name] = false;
            if (pid != -1) {
                free[pid] = true;
                used[pid] = false;
                modified[pid] = false;
                page_info[pid].ClearInfo();
            }
        }
        data_lock.unlock();
        return num_max_pages[array_id];
    }

    std::string file_name(int holder, int vid) {
        return "f" + std::to_string(holder) +'_'+std::to_string(vid);
    }

    std::string page_name(int pid) {
        return "p" + std::to_string(pid);
    }

} // namespace: proj4
