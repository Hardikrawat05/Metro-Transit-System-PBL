#ifndef TICKET_H
#define TICKET_H
#include "common.h"
#include "StationList.h"

typedef struct TicketNode
{
    int ticketID;
    int pass_id;
    string passengerName;
    int passengerType;
    int trainID;
    int source;
    int destination;
    int fare;
    string status;
    TicketNode *next;
} tn;

class TicketList
{
private:
    tn *head;
    int nextTicketID;

public:
    TicketList();
    ~TicketList();

    int addTicket(int pass_id, string name,
                  int type, int trainID,
                  int source, int destination, int fare);

    tn *findTicket(int id);

    void displayTickets(StationList &stations);
};
#endif
