#ifndef BOAT_H_
#define BOAT_H_

#include<stdio.h>
#include <unistd.h>

#include "atomic"
#include "boatGrader.h"
#include "semaphore/semaphore.h"

namespace proj2{
class Boat{
public:
	Boat(){};
    ~Boat(){};
	void begin(int, int, BoatGrader*);

	void child_thread(BoatGrader*);
	void adult_thread(BoatGrader*);
private:
	//BoatGrader* bg;
	Semaphore adults_on_Oahu;//waiting queue
	Semaphore children_on_Oahu;
	Semaphore children_on_Molokai;
	Semaphore arrive;//pilot wait for passenger to arrive Molokai before it rows back
	Semaphore total_count;//used to inform the parent thread that all new threads are ready
	Semaphore finished;//used to inform the parent thread that all threads are finished

	std::atomic_int num_children;//on Oahu
	std::atomic_int num_adults;//on Oahu

	bool pilot_exists = false;
};
}

#endif // BOAT_H_
