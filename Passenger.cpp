#include "Passenger.h"

Passenger::Passenger()
{
    pass_id = 0;
    name = "";
}

Passenger::Passenger(int id, string n)
{
    pass_id = id;
    name = n;
}

int Passenger::calculateFare(int distance)
{
    return distance * 10;
}

int Passenger::getID()
{
    return pass_id;
}

string Passenger::getName()
{
    return name;
}

Passenger::~Passenger() {}

RegularPassenger::RegularPassenger(int id, string n)
    : Passenger(id, n)
{
}

int RegularPassenger::calculateFare(int distance)
{
    return distance * 10;
}

StudentPassenger::StudentPassenger(int id, string n)
    : Passenger(id, n)
{
}

int StudentPassenger::calculateFare(int distance)
{
    return (distance * 10) / 2;
}
