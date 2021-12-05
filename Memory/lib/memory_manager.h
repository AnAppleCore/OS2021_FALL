#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <assert.h>
#include <stdio.h>
#include <map>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <fstream>

#include <mutex>
#include <condition_variable>
#include <queue>

#include "semaphore.h"

#include <iostream>

#define PageSize 1024

namespace proj3 {

class PageFrame {
public:
    PageFrame();
    int& operator[] (unsigned long);
    void WriteDisk(std::string);
    void ReadDisk(std::string);
    void Clear();
private:
    int mem[PageSize];
};

class PageInfo {
public:
    PageInfo();
    void SetInfo(int,int);
    void ClearInfo();
    int GetHolder();
    int GetVid();
private:
    int holder; //page holder id (array_id)
    int virtual_page_id; // page virtual #
    /*add your extra states here freely for implementation*/

};

class ArrayList;

enum ReplacementPolicy{
    FIFO = 0,
    CLOCK
};

class MemoryManager {
public:
    // you should not modify the public interfaces used in tests
    MemoryManager(size_t, ReplacementPolicy Policy = FIFO);
    int ReadPage(int array_id, int virtual_page_id, int offset);
    void WritePage(int array_id, int virtual_page_id, int offset, int value);
    ArrayList* Allocate(size_t);
    void Release(ArrayList*);
    ~MemoryManager();

    
private:
    std::map<std::string, int*> disk;

    std::map<int, std::map<int, int>> page_map;
    // mapping from ArrayList's virtual page # to physical page #
    PageFrame* mem; // physical pages, using 'PageFrame* mem' is also acceptable 
    PageInfo* page_info; // physical page info

    int next_array_id = 0;
    size_t mma_sz;
    void PageIn(int array_id, int virtual_page_id, int physical_page_id);
    void PageOut(int physical_page_id, std::string filename);
    int PageReplace(int array_id, int virtual_page_id, bool is_write, std::string& output_filename);


    //extra states//


    //states of the memory
    bool* free;
    bool* used;
    bool* modified;

    ReplacementPolicy policy;
    int clock_head = 0;
    //states of the array_list
    std::map<int, int> num_max_pages;
    //array_id -> number of available pages
    //extra functions
    std::map<std::string, bool> filename_exist;

    //Manage the queues
    std::map<std::string, std::queue<int>> resource_queue;
    std::map<int, Semaphore> sp;
    std::mutex data_lock;//guards the metadata
};

std::string file_name(int,int);
std::string page_name(int);

}  // namespce: proj3

#endif

