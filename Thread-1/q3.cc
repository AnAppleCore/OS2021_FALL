#include <vector>
#include <tuple>

#include <string>   // string
#include <chrono>   // timer
#include <iostream> // cout, endl

#include "lib/utils.h"
#include "lib/model.h" 
#include "lib/embedding.h" 
#include "lib/instruction.h"

#include "scheduler/scheduler.h"


int main(int argc, char *argv[]) {

    
    
    
    proj1::Scheduler* scheduler = new proj1::Scheduler("data/q3.in","data/q3.in","data/q3_instruction.tsv");
    proj1::AutoTimer timer("q3");
    scheduler -> execute_all();
    scheduler -> write_to_stdout();
    return 0;
}