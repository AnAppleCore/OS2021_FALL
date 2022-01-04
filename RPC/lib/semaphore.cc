#include "semaphore.h"

namespace proj4 {
    void Semaphore::P() {
        std::unique_lock<std::mutex> loc(m);
        if (--count < 0) {
            cv.wait(loc);
        }
    }

    void Semaphore::V() {
        std::unique_lock<std::mutex> loc(m);
        if (++count <= 0) {
            cv.notify_one();
        }
    }

}//proj4