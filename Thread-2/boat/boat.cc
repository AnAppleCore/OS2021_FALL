#include <thread>
#include <unistd.h>
#include "boat.h"

namespace proj2{
void Boat:: begin(int a, int b, BoatGrader *bg){    
    if (a < 0 || b < 2)return;
    //creating adults
    for (int i = 0; i < a; i ++) {
        std::thread th(&Boat::adult_thread, this, bg);
        th.detach();
        bg -> initializeAdult();
    }
    //creating children
    for (int i = 0; i < b; i ++) {
        std::thread th(&Boat::child_thread, this, bg);
        th.detach();
        bg -> initializeChild();
    }
    //wait for them to be ready
    for (int i = 0; i < a + b; i ++) {
        this -> total_count.P();//count the total number
    }

    this -> children_on_Oahu.V();//wake up a child as the beginning

    this -> finished.P(); //wait to be informed that all have crossed

}//Boat::begin()

void Boat:: adult_thread(BoatGrader* bg) {
    
    this -> num_adults ++;
    this -> total_count.V();//inform the parent thread
    this -> adults_on_Oahu.P();//wait in the adults queue
    //wake up 
    this -> num_adults --;
    bg -> AdultRowToMolokai();
    this -> children_on_Molokai.V();//wake up a child on Molokai
    return;
}//Boat:: adult_thread()

void Boat:: child_thread(BoatGrader* bg) {    
    this -> num_children ++;
    this -> total_count.V();
    bool on_Oahu = true;
    this -> children_on_Oahu.P();//wait in the children queue

    while (true) {
        //wake up, first observe the situation
        unsigned int num_children = this -> num_children;
        unsigned int num_adults = this -> num_adults;
        //the above two will only be used on Oahu, requirements not violated
        bool pilot_exists = this -> pilot_exists;
        //Then, determine the next move
        unsigned int case_num;
        if (on_Oahu && num_children >= 2 && !pilot_exists)case_num = 1;
        else if (on_Oahu && pilot_exists)case_num = 2;
        else if (on_Oahu && num_children <= 1 && !pilot_exists)case_num = 3;
        else if (!on_Oahu)case_num = 4;

        switch (case_num) {
            case 1:
                //become the pilot and wake up a child as passenger
                this -> num_children --;
                this -> pilot_exists = true;
                bg -> ChildRowToMolokai();
                this -> children_on_Oahu.V();//wake up a child
                on_Oahu = false;
                this -> arrive.P();//wait for the passenger to arrive
                if (num_children == 2 && num_adults == 0) {   
                    this -> finished.V();
                    return;
                }
                //this child will be awake in the next round
                break;
            case 2:
                //ride along with the pilot
                this -> num_children --;
                this -> pilot_exists = false;
                bg -> ChildRideToMolokai();
                on_Oahu = false;
                this -> arrive.V();//inform the pilot that this child has arrived Molokai
                if (num_children > 1 || num_adults == 0)return ;
                //no need to stay if one of the above two conditions is met
                this -> children_on_Molokai.P();//wait on Molokai
                break;
            case 3:
                //wake up an adult, then sleep
                this -> adults_on_Oahu.V();
                this -> children_on_Oahu.P();
                break;
            case 4:
                //on molokai, row back to Oahu
                bg -> ChildRowToOahu();
                this -> num_children ++;
                on_Oahu = true;
                //this child will be awake in the next round
                break;
        }//switch
    }//while
}//Boat::child_thread
}//namespace proj2