#ifndef TRAIN_H
#define TRAIN_H
#include "common.h"

#include <iostream>
using namespace std;

#define train_capacity 2

class Train
{
private:
    int trainID;
    int bookedseats;
    int boardedPassengers;
    int capacity;

public:
    Train();

    int getTrainID();
    int getbookedseats();
    int getAvailableSeats();
    bool hasSeat();
    bool bookSeat();
    void releaseSeat();
    void boardPassenger();
    void startNextTrain();
    void displayStatus();
};
#endif
