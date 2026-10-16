#ifndef PASSENGER_H
#define PASSENGER_H
#include "Common.h"

#include <string>
using namespace std;

class Passenger
{
protected:
    int pass_id;
    string name;

public:
    Passenger();
    Passenger(int id, string n);

    virtual int calculateFare(int distance);

    int getID();
    string getName();

    virtual ~Passenger();
};

class RegularPassenger : public Passenger
{
public:
    RegularPassenger(int id, string n);

    int calculateFare(int distance) override;
};

class StudentPassenger : public Passenger
{
public:
    StudentPassenger(int id, string n);

    int calculateFare(int distance) override;
};

#endif
