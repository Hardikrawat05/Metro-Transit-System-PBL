#include "common.h"
#include "Train.h"

Train::Train()
{
    trainID = 101;
    bookedseats = 0;
    boardedPassengers = 0;
    capacity = train_capacity;
}

int Train::getTrainID()
{
    return trainID;
}

int Train::getbookedseats()
{
    return bookedseats;
}

int Train::getAvailableSeats()
{
    return capacity - bookedseats;
}

bool Train::hasSeat()
{
    return bookedseats < capacity;
}

bool Train::bookSeat()
{
    if (!hasSeat())
        return false;

    bookedseats++;
    return true;
}

void Train::releaseSeat()
{
    if (bookedseats > 0)
        bookedseats--;
}

void Train::boardPassenger()
{
    boardedPassengers++;
}

void Train::startNextTrain()
{
    trainID++;
    bookedseats = 0;
    boardedPassengers = 0;
}

void Train::displayStatus()
{
    cout << "\n--- Active Train Status ---\n";
    cout << "Train ID: " << trainID << endl;
    cout << "Capacity: " << capacity << endl;
    cout << "Booked seats: " << bookedseats << endl;
    cout << "Available seats: " << getAvailableSeats() << endl;
    cout << "Boarded passengers: " << boardedPassengers << endl;
}
