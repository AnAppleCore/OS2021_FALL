#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_
#include <condition_variable>
#include <mutex>

namespace proj3{
class Semaphore{
private:
    int count;
    std::mutex m;
    std::condition_variable cv;
public:
    Semaphore(int init = 0):count(init){}
    void P();
    void V();	
};
}//proj3

#endif // SEMAPHORE_H_
