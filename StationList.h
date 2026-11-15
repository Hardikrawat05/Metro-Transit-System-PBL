#ifndef STATIONLIST_H
#define STATIONLIST_H
#include "common.h"

#include <iostream>
#include <string>
using namespace std;

typedef struct StationNode
{
    int id;
    string name;
    StationNode *next;
} sn;

class StationList
{
private:
    sn *head;
    int count;

public:
    StationList();
    ~StationList();

    bool addStation(string name);
    int searchStation(string name);
    bool validStation(int id);
    string getName(int id);
    int getCount();
    void displayStations();
};

#endif
