#include "metro.h"

#include <iomanip>
#include <iostream>
#include <cstdio>

using namespace std;

static string km(long long meters)
{
    char text[32];
    snprintf(text, sizeof(text), "%.1f km", meters / 1000.0);
    return text;
}

static string categoryShort(int c)
{
    switch (c) {
    case STUDENT:
        return "Student";
    case SENIOR:
        return "Senior";
    case DISABLED:
        return "Disabled";
    default:
        return "Regular";
    }
}

static string ticketStatusName(int s)
{
    switch (s) {
    case T_CONFIRMED:
        return "Confirmed";
    case T_BOARDED:
        return "Boarded";
    case T_CANCELLED:
        return "Cancelled";
    case T_EXPIRED:
        return "Expired";
    default:
        return "?";
    }
}

static string oneDecimal(double v)
{
    char text[32];
    snprintf(text, sizeof(text), "%.1f", v);
    return text;
}

App::App(Metro& metro, string dataPath) : m_(metro), dataPath_(dataPath) {}

int App::menu(const string& heading, const Array<string>& options)
{
    ui::clearScreen();
    string clock = Clock::format(Clock::now());
    if (Clock::offsetMinutes() != 0)
        clock += "  (demo clock +" + to_string(Clock::offsetMinutes()) + " min)";
    cout << "METRO TRANSIT SYSTEM" << "   " << clock
              << "\n";
    ui::title(heading);

    int number = 0;
    for (const string& option : options) {
        if (option.rfind("--", 0) == 0) {
            cout << option << "\n";
            continue;
        }
        number++;
        cout << setw(3) << number << ". " << option << "\n";
    }
    cout << setw(3) << 0 << ". " << (heading == "Main Menu" ? "Exit" : "Back") << "\n\n";
    return ui::readInt("Choose an option: ", 0, number);
}

void App::autosave()
{
    if (!m_.isDirty())
        return;
    string error;
    if (Storage::save(m_, dataPath_, error))
        m_.clearDirty();
    else
        ui::error("Auto-save failed: " + error);
}

void App::finishAction()
{
    autosave();
}

void App::run()
{
    for (;;) {
        int choice = menu("Main Menu", {"Passenger services (journeys and tickets)",
                                        "Smart card services", "Admin panel"});
        if (choice == 0)
            break;
        if (choice == 1)
            passengerMenu();
        else if (choice == 2)
            cardMenu();
        else if (choice == 3 && adminLogin())
            adminMenu();
    }
    autosave();
    ui::info("Thank you for using the Metro Transit System.  ");
}

void App::show(const Result& r)
{
    if (r.ok)
        ui::success(r.message);
    else
        ui::error(r.message);
}

void App::showNotes(const Array<string>& notes)
{
    for (const string& n : notes)
        ui::warn(n);
}

string App::lineLabel(int line) const
{
    return m_.lineName(line) + " Line";
}

string App::linesText(const Array<int>& lines) const
{
    string out;
    for (size_t i = 0; i < lines.size(); i++)
        out += (i ? ", " : "") + m_.lineName(lines[i]);
    return out.empty() ? "(not connected)" : out;
}

int App::askStation(const string& prompt)
{
    for (int attempt = 0; attempt < 3; attempt++) {
        string text = ui::trim(ui::readLine(prompt + " (ID or name, Enter to cancel): "));
        if (text.empty())
            return -1;
        int id = m_.findStation(text);
        if (id >= 0)
            return id;
        string hint;
        for (int s : m_.suggestStations(text, 3))
            hint += (hint.empty() ? "" : ", ") + m_.stationName(s);
        ui::error("Station '" + text + "' not found." +
                  (hint.empty() ? string(" Use an ID or name from the station list.")
                                : " Did you mean: " + hint + "?"));
    }
    return -1;
}

int App::askLine(const string& prompt)
{
    if (m_.lineCount() == 0) {
        ui::error("No lines exist yet.");
        return -1;
    }
    for (int i = 0; i < m_.lineCount(); i++)
        cout << "  " << i << ". " << lineLabel(i) << "\n";
    return ui::readInt(prompt, 0, m_.lineCount() - 1);
}

int App::askCategory(int& studentId)
{
    cout << "Passenger type:\n"
              << "  1. Regular\n"
              << "  2. Student (50% off, registered student ID needed)\n"
              << "  3. Senior citizen (30% off, priority on waiting list)\n"
              << "  4. Person with disability (50% off, priority on waiting list)\n";
    int category = ui::readInt("Choose type: ", 1, 4) - 1;
    studentId = 0;
    if (category == STUDENT)
        studentId = ui::readInt("Student ID: ", 1, 999999999);
    return category;
}

int App::askCardId()
{
    int id = ui::readInt("Card number: ", 1, 999999999);
    if (m_.findCard(id) == nullptr) {
        ui::error("Card " + to_string(id) + " not found.");
        return -1;
    }
    return id;
}

int App::readIntDefault(const string& prompt, int minValue, int maxValue, int fallback)
{
    for (;;) {
        string text = ui::trim(
            ui::readLine(prompt + " [" + to_string(fallback) + "]: "));
        if (text.empty())
            return fallback;
        int value = 0;
        if (ui::parseInt(text, value) && value >= minValue && value <= maxValue)
            return value;
        ui::error("Please enter a number from " + to_string(minValue) + " to " +
                  to_string(maxValue) + ", or press Enter for the default.");
    }
}

RouteSummary App::printRoute(const Route& route)
{
    RouteSummary summary;
    if (route.length < 2)
        return summary;

    int step = 1;
    int changes = 0;
    int i = 0;
    long long covered = 0;
    while (i < route.length - 1) {
        int line = route.lines[i];
        string segment = m_.stationName(route.stations[i]) + " (" + km(covered) + ")";
        int j = i;
        while (j < route.length - 1 && route.lines[j] == line) {
            int d = m_.trackDistance(route.stations[j], route.stations[j + 1], line);
            covered += d > 0 ? d : 0;
            segment += " -> " + m_.stationName(route.stations[j + 1]) + " (" + km(covered) + ")";
            j++;
        }
        int stops = j - i;
        string towards = m_.towardsStation(line, route.stations[i], route.stations[j]);
        string direction = towards.empty() ? "" : " (towards " + towards + ")";
        if (step == 1)
            cout << "  Step " << step << ": Board the " << lineLabel(line) << direction << " at "
                      << m_.stationName(route.stations[i]) << "\n";
        else {
            cout << "  Step " << step << ": " << "Change" << " to the "
                      << lineLabel(line) << direction << " at "
                      << m_.stationName(route.stations[i]) << "\n";
            changes++;
        }
        cout << "          " << segment << "  (" << stops << (stops == 1 ? " stop)" : " stops)")
                  << "\n";
        step++;
        i = j;
    }
    int stops = route.length - 1;
    int minutes = stops * 2 + changes * 4;
    cout << "  Arrive at " << m_.stationName(route.stations[route.length - 1]) << "\n\n"
              << "  Distance: " << km(route.totalMeters) << " | Stops: " << stops
              << " | Line changes: " << changes << " | About " << minutes << " min\n";

    summary.meters = route.totalMeters;
    summary.stops = stops;
    summary.changes = changes;
    summary.minutes = minutes;
    return summary;
}

void App::printFares(long long meters)
{
    int base = fareForDistance(meters);
    cout << "  Fares: ";
    for (int c = REGULAR; c <= DISABLED; c++)
        cout << (c ? " | " : "") << categoryShort(c) << " "
                  << rs(farePolicy(c).apply(base));
    cout << "\n";
}

static const size_t TICKET_WIDTH = 50;

static string repeatText(const string& piece, size_t times)
{
    string out;
    for (size_t i = 0; i < times; i++)
        out += piece;
    return out;
}

static string fitColumns(string text, size_t width)
{
    if (text.size() <= width)
        return text;
    while (!text.empty() && text.size() > width - 3) {
        while (!text.empty() && (text.back() & 0xC0) == 0x80)
            text.pop_back();
        if (!text.empty())
            text.pop_back();
    }
    return text + "...";
}

static string ticketSide()
{
    return "|";
}

static void ticketBorder()
{
    cout << "+" << repeatText("-", TICKET_WIDTH) << "+\n";
}

static void ticketLine(const string& text)
{
    string body = fitColumns(text, TICKET_WIDTH);
    size_t spaces = TICKET_WIDTH - body.size();
    cout << ticketSide() << body << string(spaces, ' ') << ticketSide() << "\n";
}

static void ticketCentered(const string& text)
{
    size_t left = (TICKET_WIDTH - text.size()) / 2;
    ticketLine(string(left, ' ') + text);
}

static void ticketTwoParts(const string& left, const string& right)
{
    size_t used = left.size() + right.size();
    size_t gap = (TICKET_WIDTH > used + 4) ? TICKET_WIDTH - used - 4 : 1;
    size_t spaces = (used + gap < TICKET_WIDTH) ? TICKET_WIDTH - used - gap : 0;
    cout << ticketSide() << left << string(gap, ' ') << right
         << string(spaces, ' ') << ticketSide() << "\n";
}

void App::printTicket(const Ticket& t)
{

    string status;
    if (t.status == T_CONFIRMED) {
        status = "CONFIRMED";
    } else if (t.status == T_BOARDED) {
        status = "BOARDED";
    } else if (t.status == T_CANCELLED) {
        status = "CANCELLED";
    } else {
        status = "EXPIRED";
    }
    string number = "MTS-" + to_string(t.id);

    ticketBorder();
    ticketCentered("METRO TICKET");
    ticketBorder();
    ticketTwoParts("  Ticket No : " + number, status);
    ticketLine("  Passenger : " + string(t.name));
    string type = "  Type      : " + categoryShort(t.category);
    if (t.category == STUDENT)
        type += " (ID " + to_string(t.studentId) + ")";
    ticketLine(type);
    if (t.womenCardId != 0)
        ticketLine("  Priority  : Women's card " + to_string(t.womenCardId));
    ticketBorder();

    string pipe = "|";
    string dot = "*";
    string swapSign = "<>";
    string bullet = "  -  ";
    RouteHolder route;
    string error;
    bool haveRoute = m_.planRoute(t.source, t.destination, false, route, error);
    int changes = 0;

    ticketLine("  FROM  " + m_.stationName(t.source));
    if (haveRoute) {
        Route& r = route.route;
        int i = 0;
        while (i < r.length - 1) {
            int line = r.lines[i];
            int j = i;
            while (j < r.length - 1 && r.lines[j] == line)
                j++;
            if (i > 0) {
                ticketLine("    " + dot + "  " + m_.stationName(r.stations[i]) + "  " + swapSign +
                           " change to " + m_.lineName(line) + " Line");
                changes++;
            }
            string towards = m_.towardsStation(line, r.stations[i], r.stations[j]);
            string text = "    " + pipe + "  " + m_.lineName(line) + " Line";
            if (!towards.empty())
                text += " (towards " + towards + ")";
            ticketLine(text);
            i = j;
        }
    }
    ticketLine("  TO    " + m_.stationName(t.destination));

    if (haveRoute) {
        int stops = route.route.length - 1;
        string stopsText = to_string(stops) + (stops == 1 ? " stop" : " stops");
        string changeText = "no change";
        if (changes == 1)
            changeText = "1 change";
        else if (changes > 1)
            changeText = to_string(changes) + " changes";
        int minutes = stops * 2 + changes * 4;
        ticketLine("");
        ticketLine("  " + km(route.route.totalMeters) + bullet + stopsText + bullet + changeText +
                   bullet + "~" + to_string(minutes) + " min");
    }
    ticketBorder();

    const FarePolicy& policy = farePolicy(t.category);
    if (haveRoute) {
        int base = fareForDistance(route.route.totalMeters);
        if (policy.apply(base) == t.fare && base != t.fare) {
            ticketTwoParts("  Base fare (" + km(route.route.totalMeters) + ")", rs(base));
            ticketTwoParts("  " + categoryShort(t.category) + " discount (" +
                               to_string(policy.discountPercent()) + "%)",
                           "- " + rs(base - t.fare));
            ticketLine("  " + repeatText("-", TICKET_WIDTH - 8));
        }
    }
    ticketTwoParts("  TOTAL PAID", rs(t.fare));
    ticketBorder();

    ticketLine("  Board train " + to_string(t.trainId) + " on " + m_.lineName(t.line) + " Line");
    ticketLine("  Valid till " + Clock::format(t.validUntil) + " (" +
               to_string(TICKET_VALID_MIN / 60) + "h)");
    string bars;
    string bar = "|";
    string digits = to_string(t.id * 7919L);
    for (size_t i = 0; i < digits.size(); i++) {
        int height = (digits[i] - '0') % 3 + 1;
        bars += repeatText(bar, height) + " ";
    }
    ticketLine("  " + bars + "  " + number);
    ticketBorder();
}

void App::passengerMenu()
{
    for (;;) {
        int choice = menu("Passenger Services",
                          {"View stations and lines", "View a line in both directions",
                           "Plan a journey (route finder)",
                           "Next trains and crowd levels", "Book a ticket", "Cancel a ticket",
                           "Verify ticket and board", "Check ticket or waiting-list status",
                           "Leave the waiting list"});
        switch (choice) {
        case 0:
            return;
        case 1:
            showStations();
            break;
        case 2:
            showLine();
            break;
        case 3:
            planJourney();
            break;
        case 4:
            showNextTrains();
            break;
        case 5:
            bookTicket();
            break;
        case 6:
            cancelTicket();
            break;
        case 7:
            boardTrain();
            break;
        case 8:
            checkStatus();
            break;
        case 9:
            leaveWaitlist();
            break;
        }
        finishAction();
    }
}

void App::showStations()
{
    ui::Table table({"ID", "Station", "Lines", "Status"});
    for (int i = 0; i < m_.stationCount(); i++) {
        Array<int> lines = m_.linesAtStation(i);
        string name = m_.stationName(i);
        if (lines.size() > 1)
            name += " *";
        table.addRow({to_string(i), name, linesText(lines),
                      m_.isStationOpen(i) ? "Open" : "CLOSED"});
    }
    table.print();
    cout << "  * = interchange station (change lines here)\n";
}

string App::stationLabel(int station) const
{
    string name = m_.stationName(station);
    if (m_.linesAtStation(station).size() > 1)
        name += "*";
    if (!m_.isStationOpen(station))
        name += " (closed)";
    return name;
}

void App::showLine()
{
    int line = askLine("Line: ");
    if (line < 0)
        return;
    LineList ll;
    bool complete = true;
    if (!m_.makeLineList(line, ll, complete)) {
        ui::error(m_.lineName(line) + " Line has no stations yet.");
        return;
    }

    string forward, backward;
    for (const DNode* n = ll.list->head; n != nullptr; n = n->next)
        forward += (forward.empty() ? "" : " -> ") + stationLabel(n->value);
    for (const DNode* n = ll.list->tail; n != nullptr; n = n->prev)
        backward += (backward.empty() ? "" : " -> ") + stationLabel(n->value);

    ui::title(m_.lineName(line) + " Line: " + to_string(dlistSize(ll.list)) + " stations");
    bool loop = m_.isLoopLine(line);
    string firstWay = loop ? "One way round:" : "Towards " + m_.stationName(ll.list->tail->value) + ":";
    string otherWay = loop ? "Other way round:" : "Towards " + m_.stationName(ll.list->head->value) + ":";
    cout << firstWay << "\n  " << forward << "\n\n"
         << otherWay << "\n  " << backward << "\n\n"
         << "  * = interchange station\n";
    if (m_.isLoopLine(line))
        ui::info("  This is a loop (ring) line: after " + m_.stationName(ll.list->tail->value) +
                 " the train continues to " + m_.stationName(ll.list->head->value) + ".");
    if (!complete)
        ui::warn("This line has branches; only the main path is shown.");
}

void App::planJourney()
{
    showStations();
    int src = askStation("From");
    if (src < 0)
        return;
    int dst = askStation("To");
    if (dst < 0)
        return;

    RouteHolder shortest;
    string error;
    if (!m_.planRoute(src, dst, false, shortest, error)) {
        ui::error(error);
        return;
    }
    ui::title("1. Shortest route (least distance) - Dijkstra");
    RouteSummary a = printRoute(shortest.route);

    RouteHolder fewest;
    if (!m_.planRoute(src, dst, true, fewest, error)) {
        ui::error(error);
        return;
    }

    bool same = fewest.route.length == shortest.route.length;
    for (int i = 0; same && i < shortest.route.length; i++)
        same = fewest.route.stations[i] == shortest.route.stations[i];

    ui::title("2. Route with the fewest stops - BFS");
    if (same) {
        ui::info("  Same as route 1: the shortest route also has the fewest stops.");
    } else {
        RouteSummary b = printRoute(fewest.route);

        ui::title("Comparison");
        ui::Table table({"Route", "Distance", "Stops", "Line changes", "About"});
        table.addRow({"1. Shortest distance", km(a.meters), to_string(a.stops),
                      to_string(a.changes), to_string(a.minutes) + " min"});
        table.addRow({"2. Fewest stops", km(b.meters), to_string(b.stops),
                      to_string(b.changes), to_string(b.minutes) + " min"});
        table.print();
        cout << "  Route 2 has " << (a.stops - b.stops) << " stop(s) less but is "
             << km(b.meters - a.meters) << " longer.\n";
    }

    cout << "\n";
    printFares(shortest.route.totalMeters);
    ui::info("  The fare is always based on the shortest distance (" + km(shortest.route.totalMeters) +
             "), whichever route you take.");
}

void App::showNextTrains()
{
    m_.expireOldTickets();
    long long now = Clock::now();
    ui::Table table({"Line", "Train", "Next train", "Seats left", "Crowd", "Waiting list"});
    for (int l = 0; l < m_.lineCount(); l++) {
        const LineState& ls = m_.lineState(l);
        NextTrain nt = m_.nextTrain(l, now);
        string next;
        if (!nt.running)
            next = "Closed (opens in " + to_string(nt.minutes / 60) + "h " +
                   to_string(nt.minutes % 60) + "m)";
        else if (nt.minutes == 0)
            next = "Arriving now";
        else
            next = "in " + to_string(nt.minutes) + " min";
        table.addRow({m_.lineName(l), to_string(ls.trainId), next,
                      to_string(ls.capacity - ls.booked) + " / " +
                          to_string(ls.capacity),
                      Metro::crowdLevel(ls), to_string(m_.waitlistInOrder(l).size())});
    }
    table.print();
    cout << "  Service hours 06:00 - 23:00. Crowd: Low < 40% booked, Medium < 80%, "
                 "High otherwise.\n";
}

void App::bookTicket()
{
    showStations();
    string name = ui::readText("Passenger name: ", NAME_LEN - 1);
    int src = askStation("From");
    if (src < 0)
        return;
    int dst = askStation("To");
    if (dst < 0)
        return;

    RouteHolder route;
    string error;
    if (!m_.planRoute(src, dst, false, route, error)) {
        ui::error(error);
        return;
    }
    printRoute(route.route);
    int studentId = 0;
    int category = askCategory(studentId);
    int womenCard = ui::readInt("Women's card number for women's priority (0 = none): ", 0,
                                999999999);
    if (womenCard != 0 && !m_.isValidWomenCard(womenCard)) {
        ui::error("Women's card " + to_string(womenCard) + " is not registered or is inactive.");
        if (!ui::confirm("Continue without women's priority?"))
            return;
        womenCard = 0;
    }
    int fare = farePolicy(category).apply(fareForDistance(route.route.totalMeters));
    if (!ui::confirm("Fare is " + rs(fare) + ". Confirm booking?"))
        return;

    BookingResult r = m_.bookTicket(name, category, studentId, src, dst, womenCard);
    if (!r.ok) {
        ui::error(r.message);
        return;
    }
    if (r.waitlisted) {
        ui::warn(r.message);
        cout << "  Waiting list number : " << r.waitId << "\n"
                  << "  Priority level      : "
                  << Metro::priorityName(Metro::waitPriority(category, womenCard)) << "\n"
                  << "  Position in line    : " << r.position << "\n"
                  << "  Use 'Check ticket or waiting-list status' to see when it is "
                     "confirmed.\n";
        return;
    }
    ui::success(r.message);
    printTicket(*m_.findTicket(r.ticketId));
}

void App::cancelTicket()
{
    int id = ui::readInt("Ticket ID to cancel: ", 1, 999999999);
    Array<string> notes;
    show(m_.cancelTicket(id, notes));
    showNotes(notes);
}

void App::boardTrain()
{
    int id = ui::readInt("Ticket ID: ", 1, 999999999);
    show(m_.boardTicket(id));
}

void App::checkStatus()
{
    cout << "  1. Ticket\n  2. Waiting-list entry\n";
    int kind = ui::readInt("Check: ", 1, 2);
    m_.expireOldTickets();
    if (kind == 1) {
        int id = ui::readInt("Ticket ID: ", 1, 999999999);
        const Ticket* t = m_.findTicket(id);
        if (t == nullptr)
            ui::error("Ticket not found.");
        else
            printTicket(*t);
        return;
    }

    int id = ui::readInt("Waiting-list number: ", 1, 999999999);
    const WaitEntry* w = m_.findWaitEntry(id);
    if (w == nullptr) {
        ui::error("Waiting-list entry not found.");
        return;
    }
    cout << "  Passenger : " << w->name << " (" << categoryShort(w->category) << ")\n"
              << "  Journey   : " << m_.stationName(w->source) << " -> "
              << m_.stationName(w->destination) << " on " << lineLabel(w->line) << "\n";
    cout << "  Priority  : " << Metro::priorityName(Metro::waitPriority(w->category, w->womenCardId))
         << "\n";
    if (w->active)
        ui::warn("Still waiting. Position " +
                 to_string(m_.waitlistPosition(w->line, w->id)) + " in line.");
    else if (w->ticketId != 0)
        ui::success("Confirmed! Your ticket ID is " + to_string(w->ticketId) + ".");
    else
        ui::error("No longer on the waiting list.");
}

void App::leaveWaitlist()
{
    int id = ui::readInt("Waiting-list number: ", 1, 999999999);
    show(m_.leaveWaitlist(id));
}

void App::cardMenu()
{
    for (;;) {
        int choice = menu("Smart Card Services",
                          {"Get a new smart card", "Card details and balance", "Recharge card",
                           "Tap in (enter a station)", "Tap out (exit a station)",
                           "Buy a day / monthly pass", "Journey history",
                           "Report lost card (block and transfer balance)"});
        switch (choice) {
        case 0:
            return;
        case 1:
            newCard();
            break;
        case 2:
            cardDetails();
            break;
        case 3:
            rechargeCard();
            break;
        case 4:
            tapIn();
            break;
        case 5:
            tapOut();
            break;
        case 6:
            buyPass();
            break;
        case 7:
            journeyHistory();
            break;
        case 8:
            reportLostCard();
            break;
        }
        finishAction();
    }
}

void App::newCard()
{
    string name = ui::readText("Card holder name: ", NAME_LEN - 1);
    int studentId = 0;
    int category = askCategory(studentId);
    int balance = ui::readInt("Initial balance (0 - " + to_string(MAX_RECHARGE) + "): ", 0,
                              MAX_RECHARGE);
    int id = 0;
    Result r = m_.createCard(name, category, studentId, balance, id);
    show(r);
    if (r.ok)
        ui::info("Please note your card number: " + to_string(id));
}

void App::cardDetails()
{
    int id = askCardId();
    if (id < 0)
        return;
    const Card& c = *m_.findCard(id);
    cout << "  Card number : " << c.id << "\n"
              << "  Holder      : " << c.name << "\n"
              << "  Type        : " << farePolicy(c.category).name();
    if (c.category == STUDENT)
        cout << " (student ID " << c.studentId << ")";
    cout << "\n  Balance     : " << rs(c.balance) << "\n"
              << "  Status      : " << (c.status == C_BLOCKED ? "BLOCKED" : "Active") << "\n"
              << "  Location    : "
              << (c.tappedIn ? "Inside, entered at " + m_.stationName(c.entryStation) + " at " +
                                   Clock::format(c.entryTime)
                             : string("Outside the system"))
              << "\n  Pass        : ";
    if (m_.hasValidPass(c))
        cout << (c.passType == PASS_DAY ? "Day pass" : "Monthly pass") << ", valid until "
                  << Clock::format(c.passExpiry) << "\n";
    else
        cout << "None\n";
}

void App::rechargeCard()
{
    int id = askCardId();
    if (id < 0)
        return;
    cout << "  Current balance: " << rs(m_.findCard(id)->balance) << "\n";
    int amount = ui::readInt("Amount (1 - " + to_string(MAX_RECHARGE) + "): ", 1,
                             MAX_RECHARGE);
    show(m_.recharge(id, amount));
}

void App::tapIn()
{
    int id = askCardId();
    if (id < 0)
        return;
    int station = askStation("Entry station");
    if (station < 0)
        return;
    show(m_.tapIn(id, station));
}

void App::tapOut()
{
    int id = askCardId();
    if (id < 0)
        return;
    int station = askStation("Exit station");
    if (station < 0)
        return;
    TapOutResult r = m_.tapOut(id, station);
    if (!r.ok) {
        ui::error(r.message);
        return;
    }
    ui::success(r.message);
    cout << "  Distance travelled : " << km(r.distanceM) << "\n"
              << "  Fare               : " << (r.passUsed ? "Rs. 0 (pass used)" : rs(r.fare))
              << "\n";
    if (r.penalty > 0)
        cout << "  Overstay penalty   : " + rs(r.penalty) + " (more than " +
                                   to_string(MAX_TRIP_MIN) + " min inside)"
                  << "\n";
    cout << "  Balance left       : " << rs(r.balanceLeft) << "\n";
}

void App::buyPass()
{
    int id = askCardId();
    if (id < 0)
        return;
    const Card& c = *m_.findCard(id);
    cout << "  1. Day pass     (24 hours) " << rs(m_.passPrice(c.category, PASS_DAY)) << "\n"
              << "  2. Monthly pass (30 days)  " << rs(m_.passPrice(c.category, PASS_MONTHLY))
              << "\n  Unlimited journeys while valid. Paid from card balance ("
              << rs(c.balance) << ").\n";
    int type = ui::readInt("Choose pass: ", 1, 2);
    if (ui::confirm("Buy this pass?"))
        show(m_.buyPass(id, type == 1 ? PASS_DAY : PASS_MONTHLY));
}

void App::journeyHistory()
{
    int id = askCardId();
    if (id < 0)
        return;
    Array<Trip> trips = m_.cardHistory(id, 10);
    if (trips.empty()) {
        ui::info("No journeys yet on this card.");
        return;
    }
    ui::Table table({"Date and time", "From", "To", "Charged"});
    for (const Trip& t : trips)
        table.addRow({Clock::format(t.time), m_.stationName(t.source),
                      m_.stationName(t.destination), rs(t.fare)});
    cout << "Last " << trips.size() << " journey(s), newest first:\n";
    table.print();
}

void App::reportLostCard()
{
    int id = askCardId();
    if (id < 0)
        return;
    if (!ui::confirm("Block card " + to_string(id) +
                     " permanently and move its balance to a new card?"))
        return;
    int newId = 0;
    show(m_.blockAndTransfer(id, newId));
}

bool App::adminLogin()
{
    Result r = m_.adminLogin(ui::readLine("Admin password: "));
    show(r);
    return r.ok;
}

void App::adminMenu()
{
    for (;;) {
        int choice = menu("Admin Panel",
                          {"-- Network --", "Add station", "Add line", "Add connection",
                           "Close / reopen station", "-- Students and women's cards --",
                           "Register student ID", "Deactivate student ID", "View student registry",
                           "Register women's card", "Deactivate women's card",
                           "View women's card registry", "-- Trains and tickets --",
                           "Train and waiting-list status", "Dispatch train (train departs)",
                           "Set train capacity", "View all tickets (sorted by ID)",
                           "View tickets sorted by fare", "View tickets in an ID range",
                           "-- Reports and tools --", "Reports", "Recent activity",
                           "Undo last admin action", "Rush hour simulation",
                           "Advance clock (demo)",
                           "Change admin password", "Save data now"});
        switch (choice) {
        case 0:
            return;
        case 1:
            addStation();
            break;
        case 2:
            addLine();
            break;
        case 3:
            addConnection();
            break;
        case 4:
            toggleStation();
            break;
        case 5:
            registerStudent();
            break;
        case 6:
            deactivateStudent();
            break;
        case 7:
            showStudents();
            break;
        case 8:
            registerWomenCard();
            break;
        case 9:
            deactivateWomenCard();
            break;
        case 10:
            showWomenCards();
            break;
        case 11:
            showTrainStatus();
            break;
        case 12:
            dispatchTrain();
            break;
        case 13:
            setCapacity();
            break;
        case 14:
            showTickets();
            break;
        case 15:
            showTicketsByFare();
            break;
        case 16:
            showTicketsInRange();
            break;
        case 17:
            showReports();
            break;
        case 18:
            showActivity();
            break;
        case 19:
            undo();
            break;
        case 20:
            simulation();
            break;
        case 21:
            advanceClock();
            break;
        case 22:
            changePassword();
            break;
        case 23:
            saveNow();
            break;
        }
        finishAction();
    }
}

void App::addStation()
{
    show(m_.addStation(ui::readText("New station name: ", NAME_LEN - 1)));
}

void App::addLine()
{
    string name = ui::readText("Line name (e.g. Green): ", NAME_LEN - 1);
    int capacity = ui::readInt("Train capacity (1 - 2000): ", 1, MAX_TRAIN_CAPACITY);
    int freq = ui::readInt("Train every how many minutes (1 - 60): ", 1, 60);
    show(m_.addLine(name, capacity, freq));
}

void App::addConnection()
{
    showStations();
    int a = askStation("First station");
    if (a < 0)
        return;
    int b = askStation("Second station");
    if (b < 0)
        return;
    int line = askLine("Line: ");
    if (line < 0)
        return;
    int meters = ui::readInt("Distance in metres (100 - 50000): ", 100, 50000);
    show(m_.addConnection(a, b, meters, line));
}

void App::toggleStation()
{
    showStations();
    int id = askStation("Station");
    if (id < 0)
        return;
    bool open = m_.isStationOpen(id);
    if (open) {
        Array<int> cutOff = m_.cutOffStations(id);
        if (!cutOff.empty()) {
            string names;
            for (int s : cutOff)
                names += (names.empty() ? "" : ", ") + m_.stationName(s);
            ui::warn("Closing " + m_.stationName(id) + " would cut these " +
                     to_string(cutOff.size()) + " station(s) off from the network: " + names);
        } else {
            ui::info("Safety check passed: the rest of the network stays connected.");
        }
    }
    if (ui::confirm(string(open ? "Close " : "Reopen ") + m_.stationName(id) +
                    (open ? " anyway?" : "?")))
        show(m_.setStationOpen(id, !open));
}

void App::registerStudent()
{
    show(m_.registerStudent(ui::readInt("Student ID: ", 1, 999999999)));
}

void App::deactivateStudent()
{
    show(m_.deactivateStudent(ui::readInt("Student ID to deactivate: ", 1, 999999999)));
}

void App::showStudents()
{
    ui::Table table({"Student ID", "Status"});
    for (const StudentRecord& s : m_.students())
        table.addRow({to_string(s.id), s.active ? "Active" : "Inactive"});
    if (table.empty())
        ui::info("No students registered.");
    else
        table.print();
}

void App::registerWomenCard()
{
    show(m_.registerWomenCard(ui::readInt("Women's card number to register: ", 1, 999999999)));
}

void App::deactivateWomenCard()
{
    show(m_.deactivateWomenCard(ui::readInt("Women's card number to deactivate: ", 1, 999999999)));
}

void App::showWomenCards()
{
    ui::Table table({"Women's card", "Status"});
    for (const StudentRecord& w : m_.womenCards())
        table.addRow({to_string(w.id), w.active ? "Active" : "Inactive"});
    if (table.empty())
        ui::info("No women's cards registered.");
    else
        table.print();
}

void App::showTrainStatus()
{
    m_.expireOldTickets();
    ui::Table table({"Line", "Train", "Capacity", "Booked", "Boarded", "Crowd", "Every"});
    for (int l = 0; l < m_.lineCount(); l++) {
        const LineState& ls = m_.lineState(l);
        table.addRow({m_.lineName(l), to_string(ls.trainId), to_string(ls.capacity),
                      to_string(ls.booked), to_string(ls.boarded),
                      Metro::crowdLevel(ls), to_string(ls.frequencyMin) + " min"});
    }
    table.print();

    for (int l = 0; l < m_.lineCount(); l++) {
        Array<WaitEntry> waiting = m_.waitlistInOrder(l);
        if (waiting.empty())
            continue;
        cout << "\nWaiting list for " << lineLabel(l) << " (served in this order):\n";
        ui::Table wl({"Pos", "Wait no.", "Name", "Type", "Priority", "Journey"});
        int pos = 1;
        for (const WaitEntry& w : waiting)
            wl.addRow({to_string(pos++), to_string(w.id), w.name,
                       categoryShort(w.category) + (w.womenCardId ? ", woman" : ""),
                       Metro::priorityName(Metro::waitPriority(w.category, w.womenCardId)),
                       m_.stationName(w.source) + " -> " + m_.stationName(w.destination)});
        wl.print();
    }
}

void App::dispatchTrain()
{
    int line = askLine("Dispatch train on line: ");
    if (line < 0)
        return;
    Array<string> notes;
    show(m_.dispatchTrain(line, notes));
    showNotes(notes);
}

void App::setCapacity()
{
    int line = askLine("Line: ");
    if (line < 0)
        return;
    cout << "  Current capacity: " << m_.lineState(line).capacity << "\n";
    int capacity = ui::readInt("New capacity (1 - 2000): ", 1, MAX_TRAIN_CAPACITY);
    Array<string> notes;
    show(m_.setCapacity(line, capacity, notes));
    showNotes(notes);
}

void App::printTicketTable(const Array<Ticket>& tickets)
{
    ui::Table table({"ID", "Name", "Type", "From", "To", "Line", "Train", "Fare", "Status"});
    for (const Ticket& t : tickets)
        table.addRow({to_string(t.id), t.name, categoryShort(t.category),
                      m_.stationName(t.source), m_.stationName(t.destination),
                      m_.lineName(t.line), to_string(t.trainId), rs(t.fare),
                      ticketStatusName(t.status)});
    table.print();
}

void App::showTickets()
{
    m_.expireOldTickets();
    Array<Ticket> tickets = m_.allTickets();
    if (tickets.empty()) {
        ui::info("No tickets yet.");
        return;
    }
    cout << "Sorted by ID using the AVL ticket index (in-order traversal):\n";
    printTicketTable(tickets);
}

void App::showTicketsByFare()
{
    m_.expireOldTickets();
    Array<Ticket> tickets = m_.ticketsByFare();
    if (tickets.empty()) {
        ui::info("No tickets yet.");
        return;
    }
    cout << "Highest fare first (sorted with quick sort):\n";
    printTicketTable(tickets);
}

void App::showTicketsInRange()
{
    int lo = ui::readInt("From ticket ID: ", 1, 999999999);
    int hi = ui::readInt("To ticket ID: ", lo, 999999999);
    m_.expireOldTickets();
    Array<Ticket> tickets = m_.ticketsInRange(lo, hi);
    if (tickets.empty()) {
        ui::info("No tickets with IDs " + to_string(lo) + " to " + to_string(hi) + ".");
        return;
    }
    cout << tickets.size() << " ticket(s) found with an AVL range search:\n";
    printTicketTable(tickets);
}

void App::showReports()
{
    Report r = m_.buildReport();
    ui::title("Revenue");
    ui::Table money({"Source", "Amount"});
    money.addRow({"Tickets (after refunds)", rs(r.ticketRevenue)});
    money.addRow({"Smart card fares and penalties", rs(r.cardRevenue)});
    money.addRow({"Passes sold", rs(r.passRevenue)});
    money.addRow({"TOTAL", rs(r.ticketRevenue + r.cardRevenue + r.passRevenue)});
    money.print();

    ui::title("Tickets and cards");
    ui::Table counts({"Item", "Count"});
    counts.addRow({"Tickets confirmed (not used yet)", to_string(r.ticketsByStatus[T_CONFIRMED])});
    counts.addRow({"Tickets boarded", to_string(r.ticketsByStatus[T_BOARDED])});
    counts.addRow({"Tickets cancelled", to_string(r.ticketsByStatus[T_CANCELLED])});
    counts.addRow({"Tickets expired", to_string(r.ticketsByStatus[T_EXPIRED])});
    counts.addRow({"Smart cards issued", to_string(r.totalCards)});
    counts.addRow({"Cards blocked", to_string(r.blockedCards)});
    counts.addRow({"Cards inside the system now", to_string(r.cardsInside)});
    counts.addRow({"Active passes", to_string(r.activePasses)});
    counts.addRow({"Journeys recorded", to_string(r.totalTrips)});
    counts.addRow({"Ticket index (BST): tickets / height",
                   to_string(r.ticketIndexSize) + " / " + to_string(r.ticketIndexHeight)});
    counts.addRow({"Ticket index (AVL): tickets / height",
                   to_string(r.ticketIndexSize) + " / " + to_string(r.ticketAvlHeight)});
    counts.print();
    if (r.ticketIndexSize > 2 && r.ticketIndexHeight == r.ticketIndexSize)
        ui::info("  Ticket IDs only increase, so the BST has become a chain (height = size).\n"
                 "  The AVL tree uses rotations to stay balanced, so its height stays close to log2(n).");

    ui::title("Busiest stations (entries + exits)");
    if (r.busiestStations.empty()) {
        ui::info("No journeys recorded yet.");
    } else {
        ui::Table busy({"Rank", "Station", "Passengers"});
        int rank = 1;
        for (const StationCount& s : r.busiestStations)
            busy.addRow({to_string(rank++), m_.stationName(s.station),
                         to_string(s.count)});
        busy.print();
    }

    ui::title("Most popular routes");
    if (r.popularRoutes.empty()) {
        ui::info("No journeys recorded yet.");
    } else {
        ui::Table routes({"Rank", "Route", "Journeys"});
        int rank = 1;
        for (const RouteCount& rc : r.popularRoutes)
            routes.addRow({to_string(rank++),
                           m_.stationName(rc.source) + " -> " + m_.stationName(rc.destination),
                           to_string(rc.count)});
        routes.print();
    }
}

void App::showActivity()
{
    Array<Activity> items = m_.recentActivity(20);
    if (items.empty()) {
        ui::info("No activity yet.");
        return;
    }
    ui::Table table({"Time", "Activity"});
    for (const Activity& a : items)
        table.addRow({Clock::format(a.time), a.text});
    cout << "Most recent first:\n";
    table.print();
}

void App::undo()
{
    Array<string> history = m_.undoHistory();
    if (history.empty()) {
        ui::info("Nothing to undo.");
        return;
    }
    cout << "Undo stack (top is undone first):\n";
    for (size_t i = 0; i < history.size() && i < 5; i++)
        cout << (i == 0 ? "  -> " : "     ") << history[i] << "\n";
    if (ui::confirm("Undo '" + history[0] + "'?"))
        show(m_.undoLast());
}

void App::simulation()
{
    ui::info("Rush hour simulation. Press Enter to keep a default value.");
    SimConfig cfg;
    cfg.passengers = readIntDefault("Number of passengers", 1, 100000, 3000);
    cfg.durationMin = readIntDefault("Rush period in minutes", 10, 240, 60);
    cfg.capacity = readIntDefault("Train capacity", 10, 2000, 100);
    cfg.frequencyMin = readIntDefault("Train every N minutes", 1, 30, 5);
    cfg.platformCapacity = readIntDefault("People the platform can hold", 10, 5000, 200);
    cfg.seed = (unsigned int)(readIntDefault("Random seed", 0, 999999, 42));

    Array<string> names;
    for (int l = 0; l < m_.lineCount(); l++)
        names.push_back(m_.lineName(l));
    SimResult r = runSimulation(cfg, names);

    ui::title("Simulation results");
    ui::Table table({"Line", "Passengers", "Served", "Trains", "Full trains", "Avg wait",
                     "Max wait", "Avg occupancy", "Held at gate", "Max gate queue"});
    for (const SimLineResult& l : r.lines)
        table.addRow({l.name, to_string(l.arrived), to_string(l.served),
                      to_string(l.trains), to_string(l.fullTrains),
                      oneDecimal(l.avgWait) + " min", to_string(l.maxWait) + " min",
                      oneDecimal(l.avgOccupancy) + "%", to_string(l.heldAtGate),
                      to_string(l.maxAtGate)});
    table.addRow({"ALL", to_string(r.passengers), to_string(r.served),
                  to_string(r.trains), to_string(r.fullTrains),
                  oneDecimal(r.avgWait) + " min", to_string(r.maxWait) + " min",
                  oneDecimal(r.avgOccupancy) + "%", to_string(r.heldAtGate),
                  to_string(r.maxAtGate)});
    table.print();
    if (r.heldAtGate > 0)
        ui::info("  " + to_string(r.heldAtGate) + " passengers found the platform full (circular "
                 "queue) and waited at the gate first.");
    if (r.leftBehind > 0)
        ui::warn(to_string(r.leftBehind) + " passengers were still waiting when the "
                 "simulation stopped. Trains are too small or too infrequent.");
    cout << "  Simulated " << r.minutesSimulated << " minutes. Tip: run again with a "
              << "different frequency or capacity and compare the average wait.\n";
}

void App::advanceClock()
{
    cout << "  Current time: " << Clock::format(Clock::now()) << "\n";
    int minutes = ui::readInt("Move the clock forward by how many minutes (1 - 10080)? ", 1,
                              10080);
    Clock::advanceMinutes(minutes);
    ui::success("Clock is now " + Clock::format(Clock::now()) + ".");
    Array<string> notes;
    int expired = m_.expireOldTickets(&notes);
    if (expired > 0)
        ui::warn(to_string(expired) + " ticket(s) expired.");
    showNotes(notes);
}

void App::changePassword()
{
    string oldPw = ui::readLine("Current password: ");
    string newPw = ui::readLine("New password (at least 4 characters): ");
    string again = ui::readLine("Repeat new password: ");
    if (newPw != again) {
        ui::error("The new passwords do not match.");
        return;
    }
    show(m_.changePassword(oldPw, newPw));
}

void App::saveNow()
{
    string error;
    if (Storage::save(m_, dataPath_, error)) {
        m_.clearDirty();
        ui::success("Data saved to " + dataPath_ + ".");
    } else {
        ui::error("Save failed: " + error);
    }
}
