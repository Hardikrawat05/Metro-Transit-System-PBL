#include "metro.h"

int Metro::read(string message)
{
    int value;

    while (true)
    {
        cout << message;

        if (cin >> value)
            return value;

        cout << "Invalid input. Enter a number." << endl;
        cin.clear();
        cin.ignore(10000, '\n');
    }
}

string Metro::readName(string message)
{
    string name;

    cout << message;
    cin >> ws;
    getline(cin, name);

    return name;
}

bool Metro::validRoute(int source, int destination)
{
    if (!stations.validStation(source) ||
        !stations.validStation(destination))
    {
        cout << "Invalid station ID." << endl;
        return false;
    }

    if (source == destination)
    {
        cout << "Source and destination must be different."
             << endl;
        return false;
    }

    return true;
}

int Metro::calculateFare(int type, int source, int destination)
{
    int distance = destination - source;

    if (distance < 0)
        distance = -distance;

    int fare = distance * 10;

    if (type == 2)
        fare = fare / 2;

    return fare;
}

void Metro::createConfirmedTicket(
    int id, string name, int type, int source, int destination)
{
    if (!train.bookSeat())
    {
        waitingQueue.enqueue(
            id, name, type, source, destination);
        return;
    }

    int fare = calculateFare(type, source, destination);

    int ticketID = tickets.addTicket(
        id, name, type, train.getTrainID(),
        source, destination, fare);

    cout << endl << "========== METRO TICKET =========="
         << endl;
    cout << "Ticket ID: " << ticketID << endl;
    cout << "Passenger: " << name << endl;
    cout << "Passenger ID: " << id << endl;
    cout << "Train ID: " << train.getTrainID() << endl;
    cout << "Source: " << stations.getName(source) << endl;
    cout << "Destination: " << stations.getName(destination)
         << endl;
    cout << "Fare: Rs. " << fare << endl;
    cout << "Status: CONFIRMED" << endl;
    cout << "==================================" << endl;

    actions.push("Ticket booked: " + to_string(ticketID));
}

void Metro::bookTicket()
{
    stations.displayStations();

    int id = read("Enter passenger ID: ");
    string name = readName("Enter passenger name: ");

    int source = read("Enter source station ID: ");
    int destination = read("Enter destination station ID: ");

    if (!validRoute(source, destination))
        return;

    int type = read(
        "1. Regular passenger\n"
        "2. Student passenger\n"
        "Choose type: ");

    if (type != 1 && type != 2)
    {
        cout << "Invalid passenger type." << endl;
        return;
    }

    if (type == 2 && !students.isValidStudent(id))
    {
        cout << "Student ID is not registered or is inactive."
             << endl;
        cout << "Student discount cannot be applied." << endl;
        return;
    }

    createConfirmedTicket(
        id, name, type, source, destination);
}

void Metro::cancelTicket()
{
    int id = read("Enter Ticket ID to cancel: ");

    tn *ticket = tickets.findTicket(id);

    if (ticket == NULL)
    {
        cout << "Ticket not found." << endl;
        return;
    }

    if (ticket->status != "CONFIRMED")
    {
        cout << "Only confirmed tickets can be cancelled."
             << endl;
        return;
    }

    if (ticket->trainID != train.getTrainID())
    {
        cout << "This ticket does not belong to the active train."
             << endl;
        return;
    }

    ticket->status = "CANCELLED";
    train.releaseSeat();

    cout << "Ticket cancelled successfully." << endl;
    cout << "One seat is now available." << endl;

    actions.push("Ticket cancelled: " + to_string(id));

    processWaitingQueue();
}

void Metro::verifyBoarding()
{
    int id = read("Enter Ticket ID: ");

    tn *ticket = tickets.findTicket(id);

    if (ticket == NULL)
    {
        cout << "Ticket not found. Entry denied." << endl;
        return;
    }

    if (ticket->status != "CONFIRMED")
    {
        cout << "Ticket is not valid for boarding." << endl;
        cout << "Current status: " << ticket->status << endl;
        return;
    }

    if (ticket->trainID != train.getTrainID())
    {
        cout << "Ticket belongs to another train." << endl;
        return;
    }

    ticket->status = "BOARDED";
    train.boardPassenger();

    cout << "Ticket verified. Boarding allowed." << endl;
    cout << "Passenger: " << ticket->passengerName << endl;
    cout << "Train ID: " << ticket->trainID << endl;

    actions.push(
        "Passenger boarded using ticket: " + to_string(id));
}

void Metro::processWaitingQueue()
{
    while (!waitingQueue.isEmpty() && train.hasSeat())
    {
        wn *passenger = waitingQueue.dequeue();

        cout << endl
             << "Offering a seat to waiting passenger: "
             << passenger->name << endl;

        createConfirmedTicket(
            passenger->pass_id,
            passenger->name,
            passenger->type,
            passenger->source,
            passenger->destination);

        delete passenger;
    }

    if (!waitingQueue.isEmpty())
        cout << "Some passengers remain in the waiting queue."
             << endl;
}

void Metro::startNextTrain()
{
    train.startNextTrain();

    cout << "Next train started successfully." << endl;
    cout << "New Train ID: " << train.getTrainID() << endl;

    actions.push(
        "Started train: " + to_string(train.getTrainID()));

    processWaitingQueue();
}

bool Metro::adminLogin()
{
    string password = readName("Enter admin password: ");

    if (password == adminpass)
    {
        cout << "Logging in." << endl;
        return true;
    }

    cout << "Incorrect password." << endl;
    return false;
}

void Metro::adminMenu()
{
    if (!adminLogin())
        return;

    int choice;

    do
    {
        cout << endl << "========== ADMIN MENU =========="
             << endl;
        cout << "1. Add Station" << endl;
        cout << "2. Display Stations" << endl;
        cout << "3. Search Station" << endl;
        cout << "4. Register Student ID" << endl;
        cout << "5. Deactivate Student ID" << endl;
        cout << "6. Display Student Registry" << endl;
        cout << "7. Display Train Status" << endl;
        cout << "8. Display Waiting Queue" << endl;
        cout << "9. Start Next Train" << endl;
        cout << "10. Display All Ticket Records" << endl;
        cout << "11. Display Recent Actions" << endl;
        cout << "12. Logout" << endl;

        choice = read("Choose: ");

        switch (choice)
        {
            case 1:
            {
                string name = readName("Enter station name: ");

                if (stations.addStation(name))
                {
                    cout << "Station added successfully."
                         << endl;
                    actions.push("Added station: " + name);
                }
                else
                    cout << "Invalid or duplicate station name."
                         << endl;

                break;
            }

            case 2:
                stations.displayStations();
                break;

            case 3:
            {
                string name =
                    readName("Enter station name to search: ");

                int id = stations.searchStation(name);

                if (id == -1)
                    cout << "Station not found." << endl;
                else
                    cout << "Station found. ID: " << id << endl;

                break;
            }

            case 4:
            {
                int id = read("Enter student ID: ");

                if (students.registerStudent(id))
                {
                    cout << "Student ID registered/reactivated."
                         << endl;
                    actions.push(
                        "Registered student: " + to_string(id));
                }
                else
                    cout << "Invalid or Duplicate ID." << endl;

                break;
            }

            case 5:
            {
                int id =
                    read("Enter student ID to deactivate: ");

                if (students.deactivateStudent(id))
                {
                    cout << "Student ID deactivated." << endl;
                    actions.push(
                        "Deactivated student: " + to_string(id));
                }
                else
                    cout << "Active student ID not found."
                         << endl;

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
                cout << "Logging out." << endl;
                break;

            default:
                cout << "Invalid choice." << endl;
        }

    } while (choice != 12);
}

Metro::Metro()
{
    stations.addStation("Rajiv Chowk");
    stations.addStation("ISBT");
    stations.addStation("City Park");
    stations.addStation("DU");
    stations.addStation("Airport");
    stations.addStation("City Mall");
}

void Metro::passengerMenu()
{
    int choice;

    do
    {
        cout << endl << "========== PASSENGER MENU =========="
             << endl;
        cout << "1. Display Stations" << endl;
        cout << "2. Search Station" << endl;
        cout << "3. Book Ticket" << endl;
        cout << "4. Cancel Ticket" << endl;
        cout << "5. Verify Ticket and Board" << endl;
        cout << "6. Display Train Status" << endl;
        cout << "7. Display Waiting Queue" << endl;
        cout << "8. Back to Main Menu" << endl;

        choice = read("Choose: ");

        switch (choice)
        {
            case 1:
                stations.displayStations();
                break;

            case 2:
            {
                string name = readName("Enter station name: ");
                int id = stations.searchStation(name);

                if (id == -1)
                    cout << "Station not found." << endl;
                else
                    cout << "Station ID: " << id << endl;

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
                cout << "Invalid choice." << endl;
        }

    } while (choice != 8);
}

void Metro::run()
{
    int choice;

    do
    {
        cout << endl;
        cout << "====================================" << endl;
        cout << "       METRO TRANSIT SYSTEM         " << endl;
        cout << "====================================" << endl;
        cout << "1. Passenger Menu" << endl;
        cout << "2. Admin Menu" << endl;
        cout << "3. Exit" << endl;

        choice = read("Choose: ");

        switch (choice)
        {
            case 1:
                passengerMenu();
                break;

            case 2:
                adminMenu();
                break;

            case 3:
                cout << "Thank you . Visit again !"<< endl;
                break;

            default:
                cout << "Invalid choice." << endl;
        }

    } while (choice != 3);
}
