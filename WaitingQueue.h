#ifndef WAITING_QUEUE_H
#define WAITING_QUEUE_H
#include "common.h"

#include <iostream>
#include <string>
using namespace std;

typedef struct WaitingNode
{
    int pass_id;
    string name;
    int type;
    int source;
    int destination;
    WaitingNode *next;
} wn;

class WaitingQueue
{
private:
    wn *front;
    wn *rear;

public:
    WaitingQueue();
    ~WaitingQueue();

    bool isEmpty();
    void enqueue(int id, string name, int type, int source, int destination);
    wn *dequeue();
    void displayQueue();
};
#endif
