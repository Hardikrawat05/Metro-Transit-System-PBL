#include "WaitingQueue.h"

WaitingQueue::WaitingQueue()
{
    front = NULL;
    rear = NULL;
}

WaitingQueue::~WaitingQueue()
{
    while (front != NULL)
    {
        wn *temp = front;
        front = front->next;
        delete temp;
    }
}

bool WaitingQueue::isEmpty()
{
    return front == NULL;
}

void WaitingQueue::enqueue(int id, string name, int type, int source, int destination)
{
    wn *node = new wn;

    node->pass_id = id;
    node->name = name;
    node->type = type;
    node->source = source;
    node->destination = destination;
    node->next = NULL;

    if (rear == NULL)
    {
        front = rear = node;
    }
    else
    {
        rear->next = node;
        rear = node;
    }

    cout << "Ticket in waiting." << endl;
}

wn *WaitingQueue::dequeue()
{
    if (front == NULL)
        return NULL;

    wn *temp = front;
    front = front->next;

    if (front == NULL)
        rear = NULL;

    temp->next = NULL;
    return temp;
}

void WaitingQueue::displayQueue()
{
    if (front == NULL)
    {
        cout << "Waiting queue is empty." << endl;
        return;
    }

    wn *temp = front;
    int pos = 1;

    cout << endl << "---- Waiting Queue (FIFO) ----" << endl;

    while (temp != NULL)
    {
        cout << pos++ << ". " << temp->name<< "  Passenger ID: " << temp->pass_id << endl;
        temp = temp->next;
    }
}
