#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <condition_variable>
#include "resource_manager.h"

namespace proj2 {

int ResourceManager::request(RESOURCE r, int amount) {
    if (amount <= 0)  return 1;

    // safe state detect
    std::unique_lock<std::mutex> lk(this->thread_mutex);
    this->thread_cv.wait(lk, [this, r, amount]{ return this->safe_detect(std::this_thread::get_id(), r, amount); });

    // update
    this->resource_amount[r] -= amount;
    this->allocated[std::this_thread::get_id()][r] += amount;
    this->thread_cv.notify_all();
    return 0;
}

void ResourceManager::release(RESOURCE r, int amount) {
    if (amount <= 0)  return;

    std::thread::id id = std::this_thread::get_id();
    std::unique_lock<std::mutex> lk(this->thread_mutex);
    this->resource_amount[r] += amount;
    this->max[id][r] -= amount;
    this->allocated[id][r] -= amount;
    if (this->is_complete(id)) {
        this->max.erase(id);
        this->allocated.erase(id);
    }
    this->thread_cv.notify_all();
}

void ResourceManager::budget_claim(std::map<RESOURCE, int> budget) {
    // This function is called when some workload starts.
    // The workload will eventually consume all resources it claims
    std::unique_lock<std::mutex> lk(this->thread_mutex);
    std::thread::id id = std::this_thread::get_id();
    this->max[id] = budget;
}

bool ResourceManager::is_complete(std::thread::id id){
    for(auto&r: this->max[id])
        if(r.second != 0) return false;
    return true;
}

bool ResourceManager::safe_detect(std::thread::id id, RESOURCE resource, int amount) {
    if (this->resource_amount[resource] < amount) return false;

    // search safe execution sequence
    std::map<RESOURCE, int> avail = this->resource_amount;
    std::map<std::thread::id, std::map<RESOURCE, int>> alloc = this->allocated;
    std::map<std::thread::id, std::map<RESOURCE, int>> unfinished = this -> max;

    avail[resource] -= amount;
    alloc[id][resource] += amount;

    while (true) {
        std::map<std::thread::id, bool> safe;
        for(auto&th: unfinished){
            std::thread::id th_id = th.first;
            safe[th_id] = true;
            for(auto&r: th.second) {
                if(alloc[th_id][r.first] + avail[r.first] < r.second){
                    safe[th_id] = false;
                    break;
                }
            }
            if (safe[th_id]) {
                for(auto&r: th.second) {
                    avail[r.first] += alloc[th_id][r.first];
                }
            } else safe.erase(th_id);
        }
        for (auto&th: safe){
            unfinished.erase(th.first);
        }
        if (safe.empty()) break;
    }

    return unfinished.empty();
}

} // namespace: proj2
