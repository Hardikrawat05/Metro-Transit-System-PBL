#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "metro.h"

using namespace std;

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        checks++;                                                                    \
        if (!(cond)) {                                                               \
            failures++;                                                              \
            cout << "  FAILED: " #cond " (line " << __LINE__ << ")\n";         \
        }                                                                            \
    } while (0)

static int station(const Metro& m, const char* name)
{
    return m.findStation(name);
}

static void freshClock()
{
    Clock::reset();
}

static void testFares()
{
    cout << "fares and polymorphism...\n";
    CHECK(fareForDistance(0) == 10);
    CHECK(fareForDistance(2000) == 10);
    CHECK(fareForDistance(2001) == 20);
    CHECK(fareForDistance(12000) == 30);
    CHECK(fareForDistance(40000) == 60);

    const FarePolicy& regular = farePolicy(REGULAR);
    const FarePolicy& student = farePolicy(STUDENT);
    const FarePolicy& senior = farePolicy(SENIOR);
    const FarePolicy& disabled = farePolicy(DISABLED);
    CHECK(regular.apply(30) == 30);
    CHECK(student.apply(30) == 15);
    CHECK(senior.apply(30) == 21);
    CHECK(disabled.apply(30) == 15);
    CHECK(senior.waitlistPriority() < regular.waitlistPriority());
    CHECK(student.name() == "Student");
}

static void testRouting()
{
    cout << "route finder...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    int isbt = station(m, "ISBT");
    int park = station(m, "city park");
    CHECK(isbt >= 0 && park >= 0);

    RouteHolder shortest, fewest;
    string error;
    CHECK(m.planRoute(isbt, park, false, shortest, error));
    CHECK(shortest.route.totalMeters == 2400);
    CHECK(shortest.route.length == 4);

    CHECK(m.planRoute(isbt, park, true, fewest, error));
    CHECK(fewest.route.length == 3);
    CHECK(fewest.route.totalMeters == 4800);

    CHECK(m.setStationOpen(station(m, "Hospital"), false).ok);
    RouteHolder detour;
    CHECK(m.planRoute(isbt, park, false, detour, error));
    CHECK(detour.route.totalMeters == 4800);

    RouteHolder none;
    CHECK(!m.planRoute(station(m, "Hospital"), park, false, none, error));
    CHECK(!m.planRoute(isbt, isbt, false, none, error));
}

static void testTimetable()
{
    cout << "timetable...\n";
    Metro m;
    m.loadDefaultNetwork();

    tm t{};
    t.tm_year = 126;
    t.tm_mon = 10;
    t.tm_mday = 9;
    t.tm_isdst = -1;

    t.tm_hour = 6;
    t.tm_min = 7;
    NextTrain a = m.nextTrain(0, (long long)(mktime(&t)));
    CHECK(a.running && a.minutes == 3);

    t.tm_hour = 5;
    t.tm_min = 0;
    NextTrain b = m.nextTrain(0, (long long)(mktime(&t)));
    CHECK(!b.running && b.minutes == 60);

    t.tm_hour = 23;
    t.tm_min = 30;
    NextTrain c = m.nextTrain(0, (long long)(mktime(&t)));
    CHECK(!c.running && c.minutes == 30 + 6 * 60);
}

static void testBookingAndWaitlist()
{
    cout << "booking and priority waiting list...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;
    int airport = station(m, "Airport");
    int rajiv = station(m, "Rajiv Chowk");
    CHECK(m.setCapacity(0, 2, notes).ok);

    BookingResult t1 = m.bookTicket("Ravi", REGULAR, 0, airport, rajiv);
    BookingResult t2 = m.bookTicket("Sita", REGULAR, 0, airport, rajiv);
    CHECK(t1.ok && !t1.waitlisted && t1.fare == 30);
    CHECK(t2.ok && !t2.waitlisted);
    CHECK(m.lineState(0).booked == 2);
    CHECK(Metro::crowdLevel(m.lineState(0)) == "High");

    BookingResult w1 = m.bookTicket("Aman", REGULAR, 0, airport, rajiv);
    BookingResult w2 = m.bookTicket("Dadi", SENIOR, 0, airport, rajiv);
    CHECK(w1.ok && w1.waitlisted && w1.position == 1);
    CHECK(w2.ok && w2.waitlisted && w2.position == 1);
    CHECK(m.waitlistPosition(0, w1.waitId) == 2);

    notes.clear();
    CHECK(m.cancelTicket(t1.ticketId, notes).ok);
    CHECK(notes.size() == 1);
    const WaitEntry* senior = m.findWaitEntry(w2.waitId);
    CHECK(senior != nullptr && !senior->active && senior->ticketId != 0);
    const Ticket* promoted = m.findTicket(senior->ticketId);
    CHECK(promoted != nullptr && promoted->fare == 21 && promoted->status == T_CONFIRMED);
    CHECK(m.waitlistPosition(0, w1.waitId) == 1);

    CHECK(m.leaveWaitlist(w1.waitId).ok);
    CHECK(m.waitlistInOrder(0).empty());
    CHECK(!m.leaveWaitlist(w1.waitId).ok);

    CHECK(!m.cancelTicket(t1.ticketId, notes).ok);

    BookingResult bad = m.bookTicket("Kid", STUDENT, 999, airport, rajiv);
    CHECK(!bad.ok);
    CHECK(m.setCapacity(0, 10, notes).ok);
    BookingResult good = m.bookTicket("Kid", STUDENT, 101, airport, rajiv);
    CHECK(good.ok && good.fare == 15);

    CHECK(m.setStationOpen(rajiv, false).ok);
    CHECK(!m.bookTicket("X", REGULAR, 0, airport, rajiv).ok);
}

static void testDispatchAndExpiry()
{
    cout << "train dispatch and ticket expiry...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;
    int airport = station(m, "Airport");
    int mall = station(m, "City Mall");

    BookingResult a = m.bookTicket("A", REGULAR, 0, airport, mall);
    BookingResult b = m.bookTicket("B", REGULAR, 0, airport, mall);
    int oldTrain = m.lineState(0).trainId;
    CHECK(m.boardTicket(a.ticketId).ok);
    CHECK(!m.boardTicket(a.ticketId).ok);

    CHECK(m.dispatchTrain(0, notes).ok);
    CHECK(m.findTicket(a.ticketId)->status == T_BOARDED);
    CHECK(m.findTicket(b.ticketId)->status == T_EXPIRED);
    CHECK(m.lineState(0).trainId != oldTrain);
    CHECK(m.lineState(0).booked == 0);

    BookingResult c = m.bookTicket("C", REGULAR, 0, airport, mall);
    CHECK(m.lineState(0).booked == 1);
    Clock::advanceMinutes(TICKET_VALID_MIN + 1);
    CHECK(!m.boardTicket(c.ticketId).ok);
    CHECK(m.findTicket(c.ticketId)->status == T_EXPIRED);
    CHECK(m.lineState(0).booked == 0);
    freshClock();
}

static void testCards()
{
    cout << "smart cards, passes, penalties, lost cards...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    int airport = station(m, "Airport");
    int rajiv = station(m, "Rajiv Chowk");
    int era = station(m, "Graphic Era");

    int id = 0;
    CHECK(m.createCard("Neha", REGULAR, 0, 100, id).ok);
    CHECK(m.tapIn(id, airport).ok);
    CHECK(!m.tapIn(id, airport).ok);
    TapOutResult out = m.tapOut(id, rajiv);
    CHECK(out.ok && out.fare == 30 && out.balanceLeft == 70 && out.distanceM == 6800);
    CHECK(!m.tapOut(id, rajiv).ok);
    CHECK(m.cardHistory(id, 10).size() == 1);

    int poor = 0;
    CHECK(m.createCard("Low Balance", REGULAR, 0, 10, poor).ok);
    CHECK(m.tapIn(poor, airport).ok);
    TapOutResult stuck = m.tapOut(poor, era);
    CHECK(!stuck.ok);
    CHECK(m.findCard(poor)->tappedIn == 1);
    CHECK(m.recharge(poor, 100).ok);
    CHECK(m.tapOut(poor, era).ok);

    CHECK(!m.buyPass(id, PASS_DAY).ok);
    CHECK(m.recharge(id, 200).ok);
    CHECK(m.buyPass(id, PASS_DAY).ok);
    CHECK(m.findCard(id)->balance == 70);
    CHECK(!m.buyPass(id, PASS_DAY).ok);
    CHECK(m.tapIn(id, airport).ok);
    TapOutResult freeRide = m.tapOut(id, era);
    CHECK(freeRide.ok && freeRide.fare == 0 && freeRide.passUsed);

    int slow = 0;
    CHECK(m.createCard("Slow", REGULAR, 0, 500, slow).ok);
    CHECK(m.tapIn(slow, airport).ok);
    Clock::advanceMinutes(MAX_TRIP_MIN + 1);
    TapOutResult late = m.tapOut(slow, rajiv);
    CHECK(late.ok && late.penalty == OVERSTAY_PENALTY && late.balanceLeft == 500 - 30 - 50);
    freshClock();

    int newId = 0;
    CHECK(m.blockAndTransfer(slow, newId).ok);
    CHECK(m.findCard(slow)->status == C_BLOCKED && m.findCard(slow)->balance == 0);
    CHECK(m.findCard(newId)->balance == 420);
    CHECK(!m.tapIn(slow, airport).ok);
    CHECK(m.tapIn(newId, airport).ok);

    int s = 0;
    CHECK(!m.createCard("Fake", STUDENT, 4242, 50, s).ok);
    CHECK(m.createCard("Real", STUDENT, 102, 50, s).ok);
    CHECK(m.tapIn(s, airport).ok);
    CHECK(m.tapOut(s, rajiv).fare == 15);
}

static void testUndo()
{
    cout << "undo stack...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;
    int before = m.stationCount();
    CHECK(!m.undoLast().ok);

    CHECK(m.addStation("New Town").ok);
    int town = station(m, "New Town");
    CHECK(m.addConnection(town, station(m, "Stadium"), 1200, 2).ok);
    CHECK(m.undoHistory().size() == 2);

    CHECK(m.undoLast().ok);
    RouteHolder r;
    string error;
    CHECK(!m.planRoute(town, station(m, "Stadium"), false, r, error));
    CHECK(m.undoLast().ok);
    CHECK(m.stationCount() == before);

    CHECK(m.addStation("Busy Halt").ok);
    int busy = station(m, "Busy Halt");
    CHECK(m.addConnection(busy, station(m, "Stadium"), 900, 2).ok);
    CHECK(m.bookTicket("P", REGULAR, 0, station(m, "Stadium"), busy).ok);
    CHECK(m.undoLast().ok);
    CHECK(!m.undoLast().ok);
    CHECK(m.stationCount() == before + 1);

    CHECK(m.setCapacity(1, 50, notes).ok);
    CHECK(m.undoLast().ok);
    CHECK(m.lineState(1).capacity == 6);

    int park = station(m, "City Park");
    CHECK(m.setStationOpen(park, false).ok);
    CHECK(m.undoLast().ok);
    CHECK(m.isStationOpen(park));

    CHECK(m.deactivateStudent(101).ok);
    CHECK(!m.isValidStudent(101));
    CHECK(m.undoLast().ok);
    CHECK(m.isValidStudent(101));
}

static void testAdminLogin()
{
    cout << "admin login lockout...\n";
    freshClock();
    Metro m;
    CHECK(m.adminLogin("1234").ok);
    CHECK(!m.adminLogin("x").ok);
    CHECK(!m.adminLogin("y").ok);
    CHECK(!m.adminLogin("z").ok);
    CHECK(!m.adminLogin("1234").ok);
    Clock::advanceMinutes(2);
    CHECK(m.adminLogin("1234").ok);
    CHECK(m.changePassword("1234", "metro2026").ok);
    CHECK(!m.adminLogin("1234").ok);
    CHECK(m.adminLogin("metro2026").ok);
    freshClock();
}

static void testSaveAndLoad()
{
    cout << "save and load...\n";
    freshClock();
    const string path = "test_data.txt";
    Array<string> notes;
    int cardId = 0;
    int seniorWait = 0, regularWait = 0;
    {
        Metro m;
        m.loadDefaultNetwork();
        int airport = station(m, "Airport");
        int rajiv = station(m, "Rajiv Chowk");
        CHECK(m.setCapacity(0, 1, notes).ok);
        CHECK(m.bookTicket("First", REGULAR, 0, airport, rajiv).ok);
        regularWait = m.bookTicket("Second", REGULAR, 0, airport, rajiv).waitId;
        seniorWait = m.bookTicket("Third", SENIOR, 0, airport, rajiv).waitId;
        CHECK(m.createCard("Saver", REGULAR, 0, 250, cardId).ok);
        CHECK(m.tapIn(cardId, airport).ok);
        CHECK(m.tapOut(cardId, rajiv).ok);
        CHECK(m.setStationOpen(station(m, "Stadium"), false).ok);
        CHECK(m.changePassword("1234", "secret").ok);

        string error;
        CHECK(Storage::save(m, path, error));
    }
    {
        Metro m;
        string error;
        CHECK(Storage::load(m, path, error) == LoadStatus::Loaded);
        CHECK(m.stationCount() == 14 && m.lineCount() == 3);
        CHECK(!m.isStationOpen(station(m, "Stadium")));
        CHECK(m.findCard(cardId) != nullptr && m.findCard(cardId)->balance == 220);
        CHECK(m.cardHistory(cardId, 10).size() == 1);
        CHECK(m.lineState(0).capacity == 1 && m.lineState(0).booked == 1);
        Array<WaitEntry> order = m.waitlistInOrder(0);
        CHECK(order.size() == 2 && order[0].id == seniorWait && order[1].id == regularWait);
        CHECK(m.adminLogin("secret").ok);
        Report r = m.buildReport();
        CHECK(r.ticketRevenue == 30 && r.cardRevenue == 30 && r.totalTrips == 1);

        int next = 0;
        CHECK(m.createCard("After", REGULAR, 0, 0, next).ok);
        CHECK(next == cardId + 1);
    }
    remove(path.c_str());

    {
        Metro m;
        string error;
        CHECK(Storage::load(m, "does_not_exist.txt", error) == LoadStatus::NotFound);
    }
    {
        ofstream bad(path);
        bad << "METRO_DATA v1\nSTATION|0|Lonely|1\nCARD|1|oops\n";
    }
    {
        Metro m;
        string error;
        CHECK(Storage::load(m, path, error) == LoadStatus::Corrupt);
        CHECK(!error.empty());
    }
    remove(path.c_str());
}

static void testReports()
{
    cout << "reports...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    int airport = station(m, "Airport");
    int rajiv = station(m, "Rajiv Chowk");
    int du = station(m, "DU");
    int card = 5001;
    for (int i = 0; i < 3; i++) {
        CHECK(m.tapIn(card, airport).ok);
        CHECK(m.tapOut(card, rajiv).ok);
    }
    CHECK(m.tapIn(card, du).ok);
    CHECK(m.tapOut(card, rajiv).ok);

    Report r = m.buildReport();
    CHECK(r.totalTrips == 4);
    CHECK(!r.busiestStations.empty() && r.busiestStations[0].station == rajiv);
    CHECK(r.busiestStations[0].count == 4);
    CHECK(!r.popularRoutes.empty() && r.popularRoutes[0].source == airport &&
          r.popularRoutes[0].destination == rajiv && r.popularRoutes[0].count == 3);
}

static void testSimulation()
{
    cout << "rush hour simulation...\n";
    Array<string> lines = {"Blue", "Yellow", "Red"};
    SimConfig cfg;
    cfg.passengers = 3000;
    cfg.durationMin = 60;
    cfg.capacity = 100;
    cfg.frequencyMin = 10;
    cfg.seed = 7;

    SimResult a = runSimulation(cfg, lines);
    SimResult b = runSimulation(cfg, lines);
    CHECK(a.served + a.leftBehind == a.passengers);
    CHECK(a.served == b.served && a.avgWait == b.avgWait);

    cfg.frequencyMin = 3;
    SimResult faster = runSimulation(cfg, lines);
    CHECK(faster.avgWait < a.avgWait);
}

static void testNewStructuresInApp()
{
    cout << "DFS, BST, binary search, doubly linked list, quick sort, deque in the app...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;

    Array<int> cut = m.cutOffStations(station(m, "Rajiv Chowk"));
    CHECK(cut.size() == 4);
    for (const char* name : {"Airport", "City Mall", "DU", "Civil Lines"}) {
        bool listed = false;
        for (int s : cut)
            if (s == station(m, name))
                listed = true;
        CHECK(listed);
    }
    CHECK(m.cutOffStations(station(m, "Hospital")).empty());
    CHECK(m.cutOffStations(station(m, "Clock Tower")).empty());

    CHECK(m.findStation("GRAPHIC era") == station(m, "Graphic Era"));
    Array<int> sug = m.suggestStations("ci", 5);
    CHECK(sug.size() == 3);
    CHECK(m.stationName(sug[0]) == "City Mall" && m.stationName(sug[2]) == "Civil Lines");
    CHECK(m.addStation("Zoo Road").ok);
    CHECK(m.findStation("zoo road") >= 0);
    CHECK(m.undoLast().ok);
    CHECK(m.findStation("Zoo Road") == -1);
    CHECK(m.addStation("Yard").ok);
    CHECK(m.findStation("Yard") >= 0 && m.findStation("Zoo Road") == -1);

    LineList blue;
    bool complete = false;
    CHECK(m.makeLineList(0, blue, complete) && complete);
    CHECK(dlistSize(blue.list) == 6);
    CHECK(m.stationName(blue.list->head->value) == "Airport");
    CHECK(m.stationName(blue.list->tail->value) == "Clock Tower");
    CHECK(m.stationName(blue.list->tail->prev->value) == "Railway Station");
    CHECK(m.towardsStation(0, station(m, "Airport"), station(m, "Rajiv Chowk")) == "Clock Tower");
    CHECK(m.towardsStation(0, station(m, "ISBT"), station(m, "City Mall")) == "Airport");
    CHECK(m.towardsStation(0, station(m, "Airport"), station(m, "DU")).empty());

    int airport = station(m, "Airport");
    BookingResult a = m.bookTicket("Short", REGULAR, 0, airport, station(m, "City Mall"));
    BookingResult b = m.bookTicket("Long", REGULAR, 0, airport, station(m, "Clock Tower"));
    BookingResult c = m.bookTicket("Mid", REGULAR, 0, airport, station(m, "Rajiv Chowk"));
    CHECK(a.ok && b.ok && c.ok);
    Array<Ticket> byId = m.allTickets();
    CHECK(byId.size() == 3 && byId[0].id == a.ticketId && byId[2].id == c.ticketId);
    CHECK(m.ticketIndexHeight() == 3);
    CHECK(m.ticketAvlHeight() == 2);
    Array<Ticket> range = m.ticketsInRange(b.ticketId, c.ticketId);
    CHECK(range.size() == 2 && range[0].id == b.ticketId);
    Array<Ticket> byFare = m.ticketsByFare();
    CHECK(byFare.size() == 3 && byFare[0].id == b.ticketId && byFare[2].id == a.ticketId);
    CHECK(byFare[0].fare >= byFare[1].fare && byFare[1].fare >= byFare[2].fare);

    for (int i = 0; i < ACTIVITY_LOG_MAX + 50; i++)
        m.registerStudent(10000 + i);
    Array<Activity> log = m.recentActivity(1000);
    CHECK((int)(log.size()) == ACTIVITY_LOG_MAX);
    CHECK(string(log[0].text).find(to_string(10000 + ACTIVITY_LOG_MAX + 49)) != string::npos);
}

static void testBstSurvivesReload()
{
    cout << "BST index after save and load...\n";
    freshClock();
    const string path = "test_bst_data.txt";
    {
        Metro m;
        m.loadDefaultNetwork();
        for (int i = 0; i < 4; i++)
            CHECK(m.bookTicket("P", REGULAR, 0, station(m, "DU"), station(m, "Tech Park")).ok);
        string error;
        CHECK(Storage::save(m, path, error));
    }
    {
        Metro m;
        string error;
        CHECK(Storage::load(m, path, error) == LoadStatus::Loaded);
        CHECK(m.allTickets().size() == 4);
        CHECK(m.ticketIndexHeight() == 4);
        CHECK(m.ticketAvlHeight() == 3);
        CHECK(m.ticketsInRange(1002, 1003).size() == 2);
    }
    remove(path.c_str());
}

static void testPlatform()
{
    cout << "platform circular queue...\n";
    Array<string> lines = {"Blue", "Yellow", "Red"};
    SimConfig cfg;
    cfg.passengers = 3000;
    cfg.capacity = 100;
    cfg.frequencyMin = 5;
    cfg.seed = 42;

    cfg.platformCapacity = 5000;
    SimResult roomy = runSimulation(cfg, lines);
    cfg.platformCapacity = 150;
    SimResult tight = runSimulation(cfg, lines);
    CHECK(roomy.heldAtGate == 0 && roomy.maxAtGate == 0);
    CHECK(tight.heldAtGate > 0 && tight.maxAtGate > 0);
    CHECK(tight.served + tight.leftBehind == tight.passengers);

    cfg.platformCapacity = 40;
    SimResult tiny = runSimulation(cfg, lines);
    CHECK(tiny.avgWait > roomy.avgWait);
    CHECK(tiny.served + tiny.leftBehind == tiny.passengers);
}

static void testArrayTemplate()
{
    cout << "Array<T> template (our replacement for vector)...\n";
    Array<int> a;
    CHECK(a.empty());
    for (int i = 0; i < 1000; i++)
        a.push_back(i * 2);
    CHECK(a.size() == 1000 && a[0] == 0 && a[999] == 1998 && a.back() == 1998);

    Array<int> copy = a;
    copy[0] = 42;
    CHECK(a[0] == 0 && copy[0] == 42);

    Array<int> assigned;
    assigned = a;
    a[1] = 7;
    CHECK(assigned[1] == 2);
    assigned = assigned;
    CHECK(assigned.size() == 1000 && assigned[1] == 2);

    Array<string> names = {"Blue", "Yellow", "Red"};
    string joined;
    for (string n : names)
        joined += n;
    CHECK(joined == "BlueYellowRed");

    Array<int> zeros(5);
    CHECK(zeros.size() == 5 && zeros[4] == 0);
    zeros.resize(2);
    zeros.resize(4);
    CHECK(zeros.size() == 4 && zeros[3] == 0);
    zeros.pop_back();
    CHECK(zeros.size() == 3);

    Array<Array<int>> grid(3, Array<int>(4, 1));
    grid[1][2] = 9;
    CHECK(grid[0][2] == 1 && grid[1][2] == 9 && grid[2].size() == 4);
}

static void testAvlInApp()
{
    cout << "AVL ticket index in the app...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;
    CHECK(m.setCapacity(0, 2000, notes).ok);
    int airport = station(m, "Airport");
    int mall = station(m, "City Mall");
    for (int i = 0; i < 1000; i++)
        CHECK(m.bookTicket("P", REGULAR, 0, airport, mall).ok);
    Report r = m.buildReport();
    CHECK(r.ticketIndexSize == 1000);
    CHECK(r.ticketIndexHeight == 1000);
    CHECK(r.ticketAvlHeight == 10);
    Array<Ticket> all = m.allTickets();
    CHECK(all.size() == 1000 && all[0].id == 1001 && all[999].id == 2000);
    Array<Ticket> some = m.ticketsInRange(1500, 1509);
    CHECK(some.size() == 10 && some[0].id == 1500 && some[9].id == 1509);
}

static void testWomenPriority()
{
    cout << "women's cards and the four priority levels...\n";
    freshClock();
    Metro m;
    m.loadDefaultNetwork();
    Array<string> notes;
    int airport = station(m, "Airport");
    int mall = station(m, "City Mall");
    CHECK(m.setCapacity(0, 1, notes).ok);
    CHECK(m.bookTicket("Seat taker", REGULAR, 0, airport, mall).ok);

    BookingResult reg = m.bookTicket("Regular", REGULAR, 0, airport, mall);
    BookingResult stu = m.bookTicket("Student", STUDENT, 102, airport, mall);
    BookingResult wom = m.bookTicket("Woman", REGULAR, 0, airport, mall, 9001);
    BookingResult sen = m.bookTicket("Senior", SENIOR, 0, airport, mall);
    BookingResult wstu = m.bookTicket("Woman student", STUDENT, 105, airport, mall, 9002);
    CHECK(reg.waitlisted && stu.waitlisted && wom.waitlisted && sen.waitlisted && wstu.waitlisted);

    Array<WaitEntry> order = m.waitlistInOrder(0);
    CHECK(order.size() == 5);
    CHECK(order[0].id == sen.waitId);
    CHECK(order[1].id == wom.waitId);
    CHECK(order[2].id == wstu.waitId);
    CHECK(order[3].id == stu.waitId);
    CHECK(order[4].id == reg.waitId);
    CHECK(wstu.fare == 10);

    CHECK(Metro::waitPriority(SENIOR, 9001) == PRIORITY_SENIOR_DISABLED);
    CHECK(Metro::waitPriority(REGULAR, 9001) == PRIORITY_WOMEN);
    CHECK(Metro::waitPriority(STUDENT, 0) == PRIORITY_STUDENT);
    CHECK(Metro::waitPriority(REGULAR, 0) == PRIORITY_REGULAR);

    CHECK(!m.bookTicket("X", REGULAR, 0, airport, mall, 4242).ok);
    CHECK(m.deactivateWomenCard(9003).ok);
    CHECK(!m.bookTicket("X", REGULAR, 0, airport, mall, 9003).ok);
    CHECK(m.undoLast().ok);
    CHECK(m.isValidWomenCard(9003));
    CHECK(m.registerWomenCard(9100).ok && m.isValidWomenCard(9100));
    CHECK(!m.registerWomenCard(9100).ok);

    notes.clear();
    CHECK(m.setCapacity(0, 3, notes).ok);
    const WaitEntry* w = m.findWaitEntry(wom.waitId);
    CHECK(w != nullptr && w->ticketId != 0);
    CHECK(m.findTicket(w->ticketId)->womenCardId == 9001);

    const string path = "test_women_data.txt";
    string error;
    CHECK(Storage::save(m, path, error));
    Metro loaded;
    CHECK(Storage::load(loaded, path, error) == LoadStatus::Loaded);
    CHECK(loaded.isValidWomenCard(9100) && loaded.isValidWomenCard(9003));
    CHECK(loaded.findTicket(w->ticketId)->womenCardId == 9001);
    Array<WaitEntry> left = loaded.waitlistInOrder(0);
    CHECK(left.size() == 3 && left[0].id == wstu.waitId && left[2].id == reg.waitId);
    remove(path.c_str());
}

static void testOlderDataFileStillLoads()
{
    cout << "an older data file (without the women's card fields) still loads...\n";
    const string path = "test_old_format.txt";
    {
        ofstream out(path);
        out << "METRO_DATA v1\n"
            << "LINE|0|Blue|101|6|1|0|5\n"
            << "STATION|0|A|1\nSTATION|1|B|1\n"
            << "EDGE|0|1|1000|0\n"
            << "TICKET|1001|Old Ticket|0|0|0|1|0|101|10|0|1700000000|1700007200\n"
            << "WAIT|1|Old Wait|2|0|0|1|0|5|1|1700000000|0\n"
            << "END\n";
    }
    Metro m;
    string error;
    CHECK(Storage::load(m, path, error) == LoadStatus::Loaded);
    CHECK(m.findTicket(1001) != nullptr && m.findTicket(1001)->womenCardId == 0);
    CHECK(m.waitlistInOrder(0).size() == 1);
    remove(path.c_str());
}

static void testStartingDataFiles()
{
    cout << "starting data from the data/ folder...\n";
    freshClock();
    {
        Metro m;
        string error, summary;
        CHECK(loadStartingData(m, "data", error, summary) == SeedStatus::Loaded);
        CHECK(error.empty());
        CHECK(m.lineCount() == 6 && m.stationCount() == 29);
        CHECK(m.isValidStudent(101) && m.isValidWomenCard(9001));
        CHECK(m.findCard(5001) != nullptr && m.findCard(5002)->category == STUDENT);
        CHECK(m.undoHistory().empty());

        int ring = m.findLine("ring");
        CHECK(ring >= 0 && m.isLoopLine(ring) && !m.isLoopLine(m.findLine("Blue")));
        LineList ll;
        bool complete = false;
        CHECK(m.makeLineList(ring, ll, complete) && complete && dlistSize(ll.list) == 9);
        CHECK(m.towardsStation(ring, station(m, "City Mall"), station(m, "Lake View")).empty());

        RouteHolder r;
        for (int s = 1; s < m.stationCount(); s++)
            CHECK(m.planRoute(0, s, false, r, error));

        CHECK(m.cutOffStations(station(m, "Rajiv Chowk")).empty());
        Array<int> cut = m.cutOffStations(station(m, "Tech Park"));
        CHECK(cut.size() == 1 && cut[0] == station(m, "Graphic Era"));
    }
    {
        Metro m;
        string error, summary;
        CHECK(loadStartingData(m, "no_such_folder", error, summary) == SeedStatus::FolderMissing);
    }
    {
        filesystem::create_directory("test_seed_bad");
        ofstream("test_seed_bad/lines.txt") << "Blue | 6 | 5\n";
        ofstream("test_seed_bad/stations.txt") << "# stations\nA\nB\n";
        ofstream("test_seed_bad/connections.txt") << "A | B | 1000 | Blue\n\nA | Bee | 500 | Blue\n";
        Metro m;
        string error, summary;
        CHECK(loadStartingData(m, "test_seed_bad", error, summary) == SeedStatus::Error);
        CHECK(error.find("connections.txt line 3") != string::npos);
        CHECK(error.find("Bee") != string::npos);
        filesystem::remove_all("test_seed_bad");
    }
}

int main()
{
    cout << "Running C++ metro logic tests\n";
    testArrayTemplate();
    testFares();
    testRouting();
    testTimetable();
    testBookingAndWaitlist();
    testDispatchAndExpiry();
    testCards();
    testUndo();
    testAdminLogin();
    testSaveAndLoad();
    testReports();
    testSimulation();
    testNewStructuresInApp();
    testBstSurvivesReload();
    testPlatform();
    testAvlInApp();
    testWomenPriority();
    testOlderDataFileStillLoads();
    testStartingDataFiles();
    cout << checks << " checks, " << failures << " failed\n";
    return failures == 0 ? 0 : 1;
}
