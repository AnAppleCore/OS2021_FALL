#include <vector>
#include <tuple>

#include <string>   // string
#include <chrono>   // timer
#include <iostream> // cout, endl

#include "lib/utils.h"
#include "lib/model.h" 
#include "lib/embedding.h" 
#include "lib/instruction.h"

#include "q5_lib/q5_optimizer.h"


int main(int argc, char *argv[]) {

    proj1::Optimizer* optim = new proj1::Optimizer("data/q4.in","data/q4.in","data/q4_instruction.tsv");
    proj1::AutoTimer timer("q5");
    optim -> execute_all();
    //scheduler -> write_to_stdout();
    optim -> clear();
    return 0;

}