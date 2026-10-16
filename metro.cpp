#include "metro.h"
void Metro::adminMenu()
{
    if (!adminLogin())
        return;

    int choice;

    do
    {
        cout << endl << "========== ADMIN MENU ==========" << endl;
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
                    cout << "Station added successfully." << endl;
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
                        "Registered student: " + to_string(id)
                    );
                }
                else
                    cout << "Invalid or Duplicate ID." << endl;

                break;
            }

            case 5:
            {
                int id = read("Enter student ID to deactivate: ");

                if (students.deactivateStudent(id))
                {
                    cout << "Student ID deactivated." << endl;

                    actions.push(
                        "Deactivated student: " + to_string(id)
                    );
                }
                else
                    cout << "Active student ID not found." << endl;

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
        cout << endl << "========== PASSENGER MENU =========="<< endl;
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
                cout << "Thank you for using Metro Transit Manager."
                     << endl;
                break;

            default:
                cout << "Invalid choice." << endl;
        }

    } while (choice != 3);
}