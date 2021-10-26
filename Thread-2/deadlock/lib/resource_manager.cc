#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include "resource_manager.h"

namespace proj2 {

void ResourceManager::request(RESOURCE r, int amount) {
    if (amount <= 0)  return;

    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    while (true) {
        if (this->resource_cv[r].wait_for(
            lk, std::chrono::milliseconds(100),
            [this, r, amount] { return this->resource_amount[r] >= amount; }
        )) {
            break;
        } else {
            auto this_id = std::this_thread::get_id();
            /* NOTE: If you choose to detect the deadlock and recover,
                     implement your code here to kill and restart threads.
                     Note that you should recycle this thread's resources
                     properly.
            */
        }
    }
    this->resource_amount[r] -= amount;
    this->resource_mutex[r].unlock();
}

void ResourceManager::release(RESOURCE r, int amount) {
    if (amount <= 0)  return;
    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    this->resource_amount[r] += amount;
    this->resource_cv[r].notify_all();
}

} // namespace: proj2
