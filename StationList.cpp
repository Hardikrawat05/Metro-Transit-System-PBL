#include "common.h"
#include "StationList.h"

StationList::StationList()
{
    head = NULL;
    count = 0;
}

StationList::~StationList()
{
    sn *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;
        delete temp;
    }
}

bool StationList::addStation(string name)
{
    if (name.empty())
        return false;

    if (searchStation(name) != -1)
        return false;

    sn *node = new sn;
    node->id = count;
    node->name = name;
    node->next = NULL;

    if (head == NULL)
        head = node;
    else
    {
        sn *temp = head;

        while (temp->next != NULL)
            temp = temp->next;

        temp->next = node;
    }

    count++;
    return true;
}

int StationList::searchStation(string name)
{
    sn *temp = head;

    while (temp != NULL)
    {
        if (temp->name == name)
            return temp->id;

        temp = temp->next;
    }

    return -1;
}

bool StationList::validStation(int id)
{
    return id >= 0 && id < count;
}

string StationList::getName(int id)
{
    StationNode *temp = head;

    while (temp != NULL)
    {
        if (temp->id == id)
            return temp->name;

        temp = temp->next;
    }

    return "Unknown";
}

int StationList::getCount()
{
    return count;
}

void StationList::displayStations()
{
    if (head == NULL)
    {
        cout << "No stations available.\n";
        return;
    }

    StationNode *temp = head;

    cout << "\n--- Metro Stations ---\n";

    while (temp != NULL)
    {
        cout << temp->id << " - " << temp->name << '\n';
        temp = temp->next;
    }
}
