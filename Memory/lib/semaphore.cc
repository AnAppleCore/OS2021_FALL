#include "semaphore.h"

namespace proj3 {
    void Semaphore::P() {
        std::unique_lock<std::mutex> loc(this -> m);
        if (--count < 0) {
            cv.wait(loc);
        }

    }
    void Semaphore::V() {
        std::unique_lock<std::mutex> loc(this -> m);
        if (++count <= 0) {
            cv.notify_one();
        }
    }

}//proj3