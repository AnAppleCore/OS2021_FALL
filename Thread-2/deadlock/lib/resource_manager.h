#ifndef DEADLOCK_LIB_RESOURCE_MANAGER_H_
#define DEADLOCK_LIB_RESOURCE_MANAGER_H_

#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include "thread_manager.h"

namespace proj2 {

enum RESOURCE {
    GPU = 0,
    MEMORY,
    DISK,
    NETWORK
};

class ResourceManager {
public:
    ResourceManager(ThreadManager *t, std::map<RESOURCE, int> init_count): \
        resource_amount(init_count), tmgr(t) {}
    void budget_claim(std::map<RESOURCE, int> budget);
    int request(RESOURCE, int amount);
    void release(RESOURCE, int amount);
private:
    std::map<RESOURCE, int> resource_amount;
    std::map<RESOURCE, std::mutex> resource_mutex;
    std::map<RESOURCE, std::condition_variable> resource_cv;
    ThreadManager *tmgr;

    // Banker's Algorithm for Deadlock Prevention
    std::mutex thread_mutex;
    std::condition_variable thread_cv;
    std::map<std::thread::id, std::map<RESOURCE, int>> max, allocated;
    bool is_complete(std::thread::id);
    bool safe_detect(std::thread::id, RESOURCE, int);
};

}  // namespace: proj2

#endif