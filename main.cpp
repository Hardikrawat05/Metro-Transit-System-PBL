#include <iostream>
#include <string>
using namespace std;
#define MAX_STUDENTS 100
#define MAX_ACTIONS  100
#define train_capacity 2



typedef struct StationNode
{
    int id;
    string name;
    StationNode *next;
}sn;

class StationList
{
private:
    sn *head;
    int count;

public:
    StationList()
    {
        head = NULL;
        count = 0;
    }

    ~StationList()
    {
        sn *temp;

        while (head != NULL)
        {
            temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool addStation(string name)
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

    int searchStation(string name)
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

    bool validStation(int id)
    {
        return id >= 0 && id < count;
    }

    string getName(int id)
    {
        sn *temp = head;

        while (temp != NULL)
        {
            if (temp->id == id)
                return temp->name;

            temp = temp->next;
        }

        return "Unknown";
    }

    int getCount()
    {
        return count;
    }

    void displayStations()
    {
        if (head == NULL)
        {
            cout << "No stations available."<<endl;
            return;
        }

        sn *temp = head;

        cout << "\n--- Metro Stations ---\n";

        while (temp != NULL)
        {
            cout << temp->id << " - " << temp->name << '\n';
            temp = temp->next;
        }
    }
};



class Passenger
{
protected:
    int pass_id;
    string name;

public:
    Passenger()
    {
        pass_id = 0;
        name = "";
    }

    Passenger(int id, string n)
    {
        pass_id = id;
        name = n;
    }

    virtual int calculateFare(int distance)
    {
        return distance * 10;
    }

    int getID()
    {
        return pass_id;
    }

    string getName()
    {
        return name;
    }

    virtual ~Passenger() {}
};

class RegularPassenger : public Passenger
{
public:
    RegularPassenger(int id, string n)
        : Passenger(id, n) {}

    int calculateFare(int distance)
    {
        return distance * 10;
    }
};

class StudentPassenger : public Passenger
{
public:
    StudentPassenger(int id, string n)
        : Passenger(id, n) {}

    int calculateFare(int distance)
    {
        return (distance * 10) / 2;
    }
};



class StudentRegistry
{
private:
    int studentIDs[MAX_STUDENTS];
    bool active[MAX_STUDENTS];
    int count;

public:
    StudentRegistry()
    {
        count = 0;

        registerStudent(101);
        registerStudent(102);
        registerStudent(105);
        registerStudent(110);
        registerStudent(115);
    }

    bool registerStudent(int id)
    {
        if (id <= 0 || count >= MAX_STUDENTS)
            return false;

        for (int i = 0; i < count; i++)
        {
            if (studentIDs[i] == id)
            {
                if (active[i])
                    return false;

                active[i] = true;
                return true;
            }
        }

        studentIDs[count] = id;
        active[count] = true;
        count++;

        return true;
    }

    bool isValidStudent(int id)
    {
        for (int i = 0; i < count; i++)
        {
            if (studentIDs[i] == id && active[i])
                return true;
        }

        return false;
    }

    bool deactivateStudent(int id)
    {
        for (int i = 0; i < count; i++)
        {
            if (studentIDs[i] == id && active[i])
            {
                active[i] = false;
                return true;
            }
        }

        return false;
    }

    void displayStudents()
    {
        cout << "\n--- Registered Students ---\n";

        for (int i = 0; i < count; i++)
        {
            cout << studentIDs[i] << " - "
                 << (active[i] ? "Active" : "Inactive")
                 << '\n';
        }
    }
};



class Train
{
private:
    int trainID;
    int bookedseats;
    int boardedPassengers;
    int capacity;

public:
    Train()
    {
        trainID = 101;
        bookedseats = 0;
        boardedPassengers = 0;
        capacity = train_capacity;
    }

    int getTrainID()
    {
        return trainID;
    }

    int getbookedseats()
    {
        return bookedseats;
    }

    int getAvailableSeats()
    {
        return capacity - bookedseats;
    }

    bool hasSeat()
    {
        return bookedseats < capacity;
    }

    bool bookSeat()
    {
        if (!hasSeat())
            return false;

        bookedseats++;
        return true;
    }

    void releaseSeat()
    {
        if (bookedseats > 0)
            bookedseats--;
    }

    void boardPassenger()
    {
        boardedPassengers++;
    }

    void startNextTrain()
    {
        trainID++;
        bookedseats = 0;
        boardedPassengers = 0;
    }

    void displayStatus()
    {
        cout << "\n--- Active Train Status ---\n";
        cout << "Train ID: " << trainID << endl;
        cout << "Capacity: " << capacity << endl;
        cout << "Booked seats: " << bookedseats <<endl;
        cout << "Available seats: " << getAvailableSeats() << endl;
        cout << "Boarded passengers: " << boardedPassengers << endl;
    }
};


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
}tn ;

class TicketList
{
private:
    tn *head;
    int nextTicketID;

public:
    TicketList()
    {
        head = NULL;
        nextTicketID = 1001;
    }

    ~TicketList()
    {
        tn *temp;

        while (head != NULL)
        {
            temp = head;
            head = head->next;
            delete temp;
        }
    }

    int addTicket(int pass_id, string name,
                  int type, int trainID,
                  int source, int destination, int fare)
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
            head = node;
        else
        {
            tn *temp = head;

            while (temp->next != NULL)
                temp = temp->next;

            temp->next = node;
        }

        return node->ticketID;
    }

    tn *findTicket(int id)
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

    void displayTickets(StationList &stations)
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
            cout << "Ticket ID: " << temp->ticketID<<endl;
            cout << "Passenger: " << temp->passengerName<<endl;
            cout << "Passenger ID: " << temp->pass_id<<endl;
            cout << "Train ID: " << temp->trainID<<endl;
            cout << "Route: " << stations.getName(temp->source)<< " -> " << stations.getName(temp->destination)<<endl;
            cout << "Fare: Rs. " << temp->fare<<endl;
            cout << "Status: " << temp->status << endl;;

            temp = temp->next;
        }
    }
};



typedef struct WaitingNode
{
    int pass_id;
    string name;
    int type;
    int source;
    int destination;
    WaitingNode *next;
}wn ;

class WaitingQueue
{
private:
    wn *front;
    wn *rear;

public:
    WaitingQueue()
    {
        front = NULL;
        rear = NULL;
    }

    ~WaitingQueue()
    {
        while (front != NULL)
        {
            wn *temp = front;
            front = front->next;
            delete temp;
        }
    }

    bool isEmpty()
    {
        return front == NULL;
    }

    void enqueue(int id, string name, int type,
                 int source, int destination)
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

        cout << "Passenger added to waiting queue."<<endl;
    }

    wn *dequeue()
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

    void displayQueue()
    {
        if (front == NULL)
        {
            cout << "Waiting queue is empty."<<endl;
            return;
        }

        wn *temp = front;
        int pos = 1;

        cout <<endl<< "---- Waiting Queue (FIFO) ----"<<endl;

        while (temp != NULL)
        {
            cout << pos++ << ". "<< temp->name<< " | Passenger ID: " << temp->pass_id<< endl;
            temp = temp->next;
        }
    }
};



class ActionStack
{
private:
    string actions[MAX_ACTIONS];
    int top;

public:
    ActionStack()
    {
        top = -1;
    }

    void push(string action)
    {
        if (top < MAX_ACTIONS - 1)
            actions[++top] = action;
    }

    void displayRecentActions()
    {
        if (top == -1)
        {
            cout << "No recent actions."<<endl;
            return;
        }

        cout << endl<<"---- Recent Actions ----"<<endl;

        for (int i = top; i >= 0; i--)
            cout << actions[i] << '\n';
    }
};



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

    int read(string message)
    {
        int value;
        while (true)
        {
            cout << message;
            if (cin >> value)
                return value;
            cout << "Invalid input. Enter a number."<<endl;
            cin.clear();
            cin.ignore();
        }
    }

    string readName(string message)
    {
        string name;
        cout << message;
        cin >> ws;
        getline(cin, name);
        return name;
    }

    bool validRoute(int source, int destination)
    {
        if (!stations.validStation(source) ||
            !stations.validStation(destination))
        {
            cout << "Invalid station ID."<<endl;
            return false;
        }

        if (source == destination)
        {
            cout << "Source and destination must be different."<<endl;
            return false;
        }

        return true;
    }

    int calculateFare(int type, int source, int destination)
    {
        int distance = destination - source;
        if (distance < 0)
            distance = -distance;
        int fare = distance * 10;
        if (type == 2)
            fare = fare / 2;
        return fare;
    }

    void createConfirmedTicket(int id, string name, int type, int source, int destination)
    {
        if (!train.bookSeat())
        {
            waitingQueue.enqueue(id, name, type, source, destination);
            return;
        }

        int fare = calculateFare(type, source, destination);
        int ticketID = tickets.addTicket( id, name, type, train.getTrainID(),source, destination, fare );
        cout << endl<<"========== METRO TICKET =========="<<endl;
        cout << "Ticket ID: " << ticketID << endl;
        cout << "Passenger: " << name << endl;
        cout << "Passenger ID: " << id << endl;
        cout << "Train ID: " << train.getTrainID() << endl;
        cout << "Source: " << stations.getName(source) << endl;
        cout << "Destination: " << stations.getName(destination) << endl;
        cout << "Fare: Rs. " << fare << endl;
        cout << "Status: CONFIRMED"<<endl;
        cout << "=================================="<<endl;

        actions.push("Ticket booked: " + to_string(ticketID));
    }

    void bookTicket()
    {
        stations.displayStations();

        int id = read("Enter passenger ID: ");
        string name = readName("Enter passenger name: ");

        int source = read("Enter source station ID: ");
        int destination = read("Enter destination station ID: ");

        if (!validRoute(source, destination))
            return;

        int type = read("1. Regular passenger\n2. Student passenger\nChoose type : ");

        if (type != 1 && type != 2)
        {
            cout << "Invalid passenger type."<<endl;
            return;
        }

        if (type == 2 && !students.isValidStudent(id))
        {
            cout << "Student ID is not registered or is inactive."<<endl;
            cout << "Student discount cannot be applied."<<endl;
            return;
        }

        createConfirmedTicket(id, name, type, source, destination);
    }

    void cancelTicket()
    {
        int id = read("Enter Ticket ID to cancel : ");

        tn *ticket = tickets.findTicket(id);

        if (ticket == NULL)
        {
            cout << "Ticket not found."<<endl;
            return;
        }

        if (ticket->status != "CONFIRMED")
        {
            cout << "Only confirmed tickets can be cancelled."<<endl;
            return;
        }

        if (ticket->trainID != train.getTrainID())
        {
            cout << "This ticket does not belong to the active train."<<endl;
            return;
        }

        ticket->status = "CANCELLED";
        train.releaseSeat();

        cout << "Ticket cancelled successfully."<<endl;
        cout << "One seat is now available."<<endl;

        actions.push("Ticket cancelled : " + to_string(id));

        processWaitingQueue();
    }

    void verifyBoarding()
    {
        int id = read("Enter Ticket ID : ");

        tn *ticket = tickets.findTicket(id);

        if (ticket == NULL)
        {
            cout << "Ticket not found. Entry denied."<<endl;
            return;
        }

        if (ticket->status != "CONFIRMED")
        {
            cout << "Ticket is not valid for boarding."<<endl;
            cout << "Current status : " << ticket->status << endl;
            return;
        }

        if (ticket->trainID != train.getTrainID())
        {
            cout << "Ticket belongs to another train."<<endl;
            return;
        }

        ticket->status = "BOARDED";
        train.boardPassenger();

        cout << "Ticket verified. Boarding allowed."<<endl;
        cout << "Passenger : " << ticket->passengerName << endl;
        cout << "Train ID : " << ticket->trainID << endl;

        actions.push("Passenger boarded using ticket : " + to_string(id));
    }

    void processWaitingQueue()
    {
        while (!waitingQueue.isEmpty() && train.hasSeat())
        {
            wn *passenger = waitingQueue.dequeue();

            cout << endl<< "Offering a seat to waiting passenger : "<< passenger->name << endl;

            createConfirmedTicket( passenger->pass_id, passenger->name, passenger->type, passenger->source, passenger->destination );
            delete passenger;
        }

        if (!waitingQueue.isEmpty())
            cout << "Some passengers remain in the waiting queue."<<endl;
    }

    void startNextTrain()
    {
        train.startNextTrain();

        cout << "Next train started successfully."<<endl;
        cout << "New Train ID : " << train.getTrainID() << endl;

        actions.push("Started train : " + to_string(train.getTrainID()));

        processWaitingQueue();
    }

    bool adminLogin()
    {
        string password = readName("Enter admin password : ");

        if (password == adminpass)
        {
            cout << "Logging In."<<endl;
            return true;
        }

        cout << "Incorrect password."<<endl;
        return false;
    }

    void adminMenu()
    {
        if (!adminLogin())
            return;
        int choice;
        do
        {
            cout << endl<<"========== ADMIN MENU =========="<<endl;
            cout << "1. Add Station"<<endl;
            cout << "2. Display Stations"<<endl;
            cout << "3. Search Station"<<endl;
            cout << "4. Register Student ID"<<endl;
            cout << "5. Deactivate Student ID"<<endl;
            cout << "6. Display Student Registry"<<endl;
            cout << "7. Display Train Status"<<endl;
            cout << "8. Display Waiting Queue"<<endl;
            cout << "9. Start Next Train"<<endl;
            cout << "10. Display All Ticket Records"<<endl;
            cout << "11. Display Recent Actions"<<endl;
            cout << "12. Logout"<<endl;

            choice = read("Choose : ");

            switch (choice)
            {
                case 1:
                {
                    string name = readName("Enter station name : ");

                    if (stations.addStation(name))
                    {
                        cout << "Station added successfully."<<endl;
                        actions.push("Added station: " + name);
                    }
                    else
                        cout << "Invalid or duplicate station name."<<endl;

                    break;
                }

                case 2:
                    stations.displayStations();
                    break;

                case 3:
                {
                    string name = readName("Enter station name to search : ");
                    int id = stations.searchStation(name);

                    if (id == -1)
                        cout << "Station not found."<<endl;
                    else
                        cout << "Station found. ID : " << id << endl;

                    break;
                }

                case 4:
                {
                    int id = read("Enter student ID : ");

                    if (students.registerStudent(id))
                    {
                        cout << "Student ID registered/reactivated."<<endl;
                        actions.push("Registered student : " + to_string(id));
                    }
                    else
                        cout << "Invalid or Duplicate ID."<<endl;

                    break;
                }

                case 5:
                {
                    int id = read("Enter student ID to deactivate : ");

                    if (students.deactivateStudent(id))
                    {
                        cout << "Student ID deactivated."<<endl;
                        actions.push("Deactivated student : " + to_string(id));
                    }
                    else
                        cout << "Active student ID not found."<<endl;

                    break;
                }

                case 6:
                    students.displayStudents();
                    break;

                case 7:
                    train.displayStatus();
                    break;

                case 8:
                    waitingQueue.displayQueue();
                    break;

                case 9:
                    startNextTrain();
                    break;

                case 10:
                    tickets.displayTickets(stations);
                    break;

                case 11:
                    actions.displayRecentActions();
                    break;

                case 12:
                    cout << "Logging out."<<endl;
                    break;

                default:
                    cout << "Invalid choice."<<endl;
            }

        } while (choice != 12);
    }

public:
    Metro()
    {
        stations.addStation("Rajiv Chowk");
        stations.addStation("ISBT");
        stations.addStation("City Park");
        stations.addStation("DU");
        stations.addStation("Airport");
        stations.addStation("City Mall");
    }

    void passengerMenu()
    {
        int choice;
        do
        {
            cout << endl<<"========== PASSENGER MENU =========="<<endl;
            cout << "1. Display Stations"<<endl;
            cout << "2. Search Station"<<endl;
            cout << "3. Book Ticket"<<endl;
            cout << "4. Cancel Ticket"<<endl;
            cout << "5. Verify Ticket and Board"<<endl;
            cout << "6. Display Train Status"<<endl;
            cout << "7. Display Waiting Queue"<<endl;
            cout << "8. Back to Main Menu"<<endl;

            choice = read("Choose : ");

            switch (choice)
            {
                case 1:
                    stations.displayStations();
                    break;

                case 2:
                {
                    string name = readName("Enter station name : ");
                    int id = stations.searchStation(name);

                    if (id == -1)
                        cout << "Station not found."<<endl;
                    else
                        cout << "Station ID : " << id << endl;

                    break;
                }

                case 3:
                    bookTicket();
                    break;

                case 4:
                    cancelTicket();
                    break;

                case 5:
                    verifyBoarding();
                    break;

                case 6:
                    train.displayStatus();
                    break;

                case 7:
                    waitingQueue.displayQueue();
                    break;

                case 8:
                    break;

                default:
                    cout << "Invalid choice."<<endl;
            }

        } while (choice != 8);
    }

    void run()
    {
        int choice;
        do
        {
            cout << endl<<"===================================="<<endl;
            cout << "       METRO TRANSIT MANAGER"<<endl;
            cout << endl<<"===================================="<<endl;
            cout << "1. Passenger Menu"<<endl;
            cout << "2. Admin Menu"<<endl;
            cout << "3. Exit"<<endl;
            choice = read("Choose : ");
            switch (choice)
            {
                case 1:
                    passengerMenu();
                    break;

                case 2:
                    adminMenu();
                    break;

                case 3:
                    cout << "Thank you for using Metro Transit Manager."<<endl;
                    break;

                default:
                    cout << "Invalid choice."<<endl;
            }

        } while (choice != 3);
    }
};



int main()
{
    Metro m;
    m.run();
    return 0;
}
