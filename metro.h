#ifndef metro_h
#define metro_h
#include "common.h"
#include "StationList.h"
#include "StudentRegistery.h"
#include "Train.h"
#include "ticket.h"
#include "Passenger.h"
#include "WaitingQueue.h"
#include "Actionstack.h"

using namespace std;

class Metro
{
private:
    StationList stations;
    StudentRegistry students;
    Train train;
    TicketList tickets;
    WaitingQueue waitingQueue;
    ActionStack actions;

    string adminpass = "1234";

    int read(string message);
    string readName(string message);

    bool validRoute(int source, int destination);
    int calculateFare(int type, int source, int destination);

    void createConfirmedTicket(
        int id, string name, int type,
        int source, int destination
    );

    void bookTicket();
    void cancelTicket();
    void verifyBoarding();
    void processWaitingQueue();
    void startNextTrain();

    bool adminLogin();
    void adminMenu();

public:
    Metro();

    void passengerMenu();
    void run();
};

#endif
