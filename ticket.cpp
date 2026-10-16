#include "Ticket.h"
TicketList::TicketList()
{
    head = NULL;
    nextTicketID = 1001;
}

TicketList::~TicketList()
{
    tn *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;
        delete temp;
    }
}

// Add a new ticket
int TicketList::addTicket(int pass_id, string name,int type, int trainID,int source, int destination, int fare)
{
    tn *node = new tn;

    node->ticketID = nextTicketID++;
    node->pass_id = pass_id;
    node->passengerName = name;
    node->passengerType = type;
    node->trainID = trainID;
    node->source = source;
    node->destination = destination;
    node->fare = fare;
    node->status = "CONFIRMED";
    node->next = NULL;

    if (head == NULL)
    {
        head = node;
    }
    else
    {
        tn *temp = head;

        while (temp->next != NULL)
            temp = temp->next;

        temp->next = node;
    }

    return node->ticketID;
}

// Find ticket using ticket ID
tn *TicketList::findTicket(int id)
{
    tn *temp = head;

    while (temp != NULL)
    {
        if (temp->ticketID == id)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

// Display all tickets
void TicketList::displayTickets(StationList &stations)
{
    tn *temp = head;

    if (temp == NULL)
    {
        cout << "No ticket records available.\n";
        return;
    }

    cout << "\n--- Ticket Records ---\n";

    while (temp != NULL)
    {
        cout << "Ticket ID: " << temp->ticketID << endl;
        cout << "Passenger: " << temp->passengerName << endl;
        cout << "Passenger ID: " << temp->pass_id << endl;
        cout << "Train ID: " << temp->trainID << endl;
        cout << "Route: "<< stations.getName(temp->source)<< " -> "<< stations.getName(temp->destination)<< endl;
        cout << "Fare: Rs. " << temp->fare << endl;
        cout << "Status: " << temp->status << endl;

        temp = temp->next;
    }
}
