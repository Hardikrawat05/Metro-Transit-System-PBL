#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

Metro::Metro()
{
    graph_ = newGraph();
    students_ = newTable(sizeof(int));
    womenCards_ = newTable(sizeof(int));
    cards_ = newTable(sizeof(Card));
    tickets_ = newTable(sizeof(Ticket));
    waitEntries_ = newTable(sizeof(WaitEntry));
    trips_ = newList(sizeof(Trip));
    activity_ = newDeque(sizeof(Activity));
    undo_ = newStack(sizeof(AdminAction));
    ticketIndex_ = newBst();
    ticketAvl_ = newAvl();
    stationIndex_ = newStationIndex();

    bool allOk = graph_ && students_ && womenCards_ && cards_ && tickets_ && waitEntries_ && trips_ &&
                 activity_ && undo_ && ticketIndex_ && ticketAvl_ && stationIndex_;
    for (int i = 0; i < MAX_LINES; i++) {
        waitlists_[i] = newHeap();
        allOk = allOk && waitlists_[i] != nullptr;
    }
    if (!allOk) {
        freeAll();
        throw bad_alloc();
    }
    adminHash_ = hashPassword("1234");
}

Metro::~Metro()
{
    freeAll();
}

void Metro::freeAll()
{
    freeGraph(graph_);
    freeTable(students_);
    freeTable(womenCards_);
    freeTable(cards_);
    freeTable(tickets_);
    freeTable(waitEntries_);
    freeList(trips_);
    freeDeque(activity_);
    freeStack(undo_);
    freeBst(ticketIndex_);
    freeAvl(ticketAvl_);
    freeStationIndex(stationIndex_);
    for (int i = 0; i < MAX_LINES; i++) {
        freeHeap(waitlists_[i]);
        waitlists_[i] = nullptr;
    }
    graph_ = nullptr;
    students_ = womenCards_ = cards_ = tickets_ = waitEntries_ = nullptr;
    trips_ = nullptr;
    activity_ = nullptr;
    undo_ = nullptr;
    ticketIndex_ = nullptr;
    ticketAvl_ = nullptr;
    stationIndex_ = nullptr;
}

void Metro::loadDefaultNetwork()
{
    addLine("Blue", 6, 5);
    addLine("Yellow", 6, 6);
    addLine("Red", 6, 8);

    const char* names[] = {"Airport",     "City Mall",  "Rajiv Chowk",      "ISBT",
                           "Railway Station", "Clock Tower", "DU",           "Civil Lines",
                           "City Park",   "Tech Park",  "Graphic Era",      "Hospital",
                           "Exhibition Ground", "Stadium"};
    for (const char* n : names)
        addStation(n);

    struct Track {
        const char* a;
        const char* b;
        int meters;
        int line;
    };
    const Track tracks[] = {
        {"Airport", "City Mall", 4200, 0},
        {"City Mall", "Rajiv Chowk", 2600, 0},
        {"Rajiv Chowk", "ISBT", 1800, 0},
        {"ISBT", "Railway Station", 1500, 0},
        {"Railway Station", "Clock Tower", 1200, 0},
        {"DU", "Civil Lines", 1600, 1},
        {"Civil Lines", "Rajiv Chowk", 1400, 1},
        {"Rajiv Chowk", "City Park", 3000, 1},
        {"City Park", "Tech Park", 2200, 1},
        {"Tech Park", "Graphic Era", 1900, 1},
        {"ISBT", "Hospital", 900, 2},
        {"Hospital", "Exhibition Ground", 800, 2},
        {"Exhibition Ground", "City Park", 700, 2},
        {"City Park", "Stadium", 2500, 2},
    };
    for (const Track& t : tracks)
        addConnection(findStation(t.a), findStation(t.b), t.meters, t.line);

    for (int id : {101, 102, 105, 110, 115})
        registerStudent(id);
    for (int id : {9001, 9002, 9003})
        registerWomenCard(id);

    int cardId = 0;
    createCard("Aarav Sharma", REGULAR, 0, 500, cardId);
    createCard("Priya Negi", STUDENT, 101, 300, cardId);

    finishSetup("System initialised with the built-in default network");
}

void Metro::finishSetup(const string& note)
{
    stackClear(undo_);
    dequeClear(activity_);
    log(note);
}

bool Metro::validLine(int line) const
{
    return line >= 0 && line < graphLineCount(graph_);
}

int Metro::stationCount() const { return graphStationCount(graph_); }

int Metro::lineCount() const { return graphLineCount(graph_); }

string Metro::stationName(int id) const { return graphStationName(graph_, id); }

string Metro::lineName(int line) const { return graphLineName(graph_, line); }

int Metro::findLine(const string& name) const
{
    string wanted = trim(name);
    for (int i = 0; i < lineCount(); i++) {
        string have = lineName(i);
        if (have.size() != wanted.size())
            continue;
        bool same = true;
        for (size_t k = 0; k < have.size() && same; k++)
            same = tolower((unsigned char)(have[k])) ==
                   tolower((unsigned char)(wanted[k]));
        if (same)
            return i;
    }
    return -1;
}

bool Metro::isStationOpen(int id) const { return graphIsOpen(graph_, id); }

int Metro::findStation(const string& idOrName) const
{
    string text = trim(idOrName);
    int id = -1;
    if (parseInt(text, id))
        return graphHasStation(graph_, id) ? id : -1;
    return findInIndex(stationIndex_, text.c_str());
}

void Metro::rebuildStationIndex()
{
    buildStationIndex(stationIndex_, graph_);
}

Array<int> Metro::suggestStations(const string& prefix, int maxResults) const
{
    Array<int> out;
    string text = trim(prefix);
    if (text.empty() || maxResults <= 0)
        return out;
    out.resize(maxResults);
    int found = findByPrefix(stationIndex_, text.c_str(), out.data(), maxResults);
    out.resize(found);
    return out;
}

static int biggestGroup(const Array<int>& group, int groups, int n)
{
    Array<int> size(groups, 0);
    for (int i = 0; i < n; i++)
        if (group[i] >= 0)
            size[group[i]]++;
    int best = 0;
    for (int g = 1; g < groups; g++)
        if (size[g] > size[best])
            best = g;
    return best;
}

Array<int> Metro::cutOffStations(int id) const
{
    Array<int> cutOff;
    int n = stationCount();
    if (!graphHasStation(graph_, id) || !graphIsOpen(graph_, id))
        return cutOff;

    Array<int> before(n), after(n);
    int groupsBefore = dfsGroups(graph_, -1, before.data());
    int groupsAfter = dfsGroups(graph_, id, after.data());
    if (groupsBefore < 0 || groupsAfter <= groupsBefore)
        return cutOff;

    int mainBefore = biggestGroup(before, groupsBefore, n);
    int mainAfter = biggestGroup(after, groupsAfter, n);
    for (int i = 0; i < n; i++)
        if (i != id && before[i] == mainBefore && after[i] != mainAfter)
            cutOff.push_back(i);
    return cutOff;
}

bool Metro::makeLineList(int line, LineList& out, bool& complete) const
{
    complete = true;
    if (!validLine(line) || out.list == nullptr)
        return false;
    dlistClear(out.list);

    int n = stationCount();
    Array<int> degree(n, 0);
    int onLine = 0;
    for (int s = 0; s < n; s++) {
        for (const EdgeNode* e = graphTracks(graph_, s); e != nullptr; e = e->next)
            if (e->line == line)
                degree[s]++;
        if (degree[s] > 0)
            onLine++;
    }
    if (onLine == 0)
        return false;

    int start = -1;
    for (int s = 0; s < n && start < 0; s++)
        if (degree[s] == 1)
            start = s;
    for (int s = 0; s < n && start < 0; s++)
        if (degree[s] > 0)
            start = s;

    Array<bool> seen(n, false);
    int prev = -1, cur = start;
    while (cur >= 0) {
        dlistAddBack(out.list, cur);
        seen[cur] = true;
        int next = -1;
        for (const EdgeNode* e = graphTracks(graph_, cur); e != nullptr; e = e->next)
            if (e->line == line && e->to != prev && !seen[e->to] && (next < 0 || e->to < next))
                next = e->to;
        prev = cur;
        cur = next;
    }
    complete = (int)(dlistSize(out.list)) == onLine;
    return true;
}

int Metro::trackDistance(int a, int b, int line) const
{
    for (const EdgeNode* e = graphTracks(graph_, a); e != nullptr; e = e->next)
        if (e->to == b && e->line == line)
            return e->meters;
    return -1;
}

bool Metro::isLoopLine(int line) const
{
    if (!validLine(line))
        return false;
    int onLine = 0;
    for (int s = 0; s < stationCount(); s++) {
        int degree = 0;
        for (const EdgeNode* e = graphTracks(graph_, s); e != nullptr; e = e->next)
            if (e->line == line)
                degree++;
        if (degree == 0)
            continue;
        if (degree != 2)
            return false;
        onLine++;
    }
    return onLine >= 3;
}

string Metro::towardsStation(int line, int from, int to) const
{
    if (isLoopLine(line))
        return "";
    LineList ll;
    bool complete = true;
    if (!makeLineList(line, ll, complete))
        return "";
    int a = dlistIndexOf(ll.list, from);
    int b = dlistIndexOf(ll.list, to);
    if (a < 0 || b < 0 || a == b)
        return "";
    return stationName(b > a ? ll.list->tail->value : ll.list->head->value);
}

Array<int> Metro::linesAtStation(int id) const
{
    int buf[MAX_LINES];
    int n = graphStationLines(graph_, id, buf, MAX_LINES);
    Array<int> out;
    for (int i = 0; i < n; i++)
        out.push_back(buf[i]);
    return out;
}

Result Metro::addStation(const string& name)
{
    string clean = trim(name);
    if (clean.empty() || clean.size() >= NAME_LEN)
        return fail("Station name must be 1 to " + to_string(NAME_LEN - 1) +
                    " characters.");
    int id = graphAddStation(graph_, clean.c_str());
    if (id < 0)
        return fail("A station called '" + clean + "' already exists.");
    rebuildStationIndex();
    pushUndo(ACT_ADD_STATION, id, 0, 0, 0, "Add station " + clean);
    log("Added station " + clean + " (ID " + to_string(id) + ")");
    return ok("Station '" + clean + "' added with ID " + to_string(id) +
              ". Connect it to the network with 'Add connection'.");
}

Result Metro::addLine(const string& name, int capacity, int frequencyMin)
{
    string clean = trim(name);
    if (graphLineCount(graph_) >= MAX_LINES)
        return fail("Maximum of " + to_string(MAX_LINES) + " lines reached.");
    if (capacity < 1 || capacity > MAX_TRAIN_CAPACITY)
        return fail("Capacity must be 1 to " + to_string(MAX_TRAIN_CAPACITY) + ".");
    if (frequencyMin < 1 || frequencyMin > 60)
        return fail("Frequency must be 1 to 60 minutes.");
    int id = graphAddLine(graph_, clean.c_str());
    if (id < 0)
        return fail("Invalid or duplicate line name.");
    lines_[id] = LineState{nextTrainId_++, capacity, 0, 0, frequencyMin};
    log("Added line " + clean);
    return ok(clean + " Line added with ID " + to_string(id) + ".");
}

Result Metro::addConnection(int a, int b, int distanceM, int line)
{
    if (!graphHasStation(graph_, a) || !graphHasStation(graph_, b))
        return fail("Invalid station.");
    if (a == b)
        return fail("A station cannot be connected to itself.");
    if (!validLine(line))
        return fail("Invalid line.");
    if (distanceM < 100 || distanceM > 50000)
        return fail("Distance must be between 100 and 50000 metres.");
    if (graphHasTrack(graph_, a, b, line))
        return fail("These stations are already connected on this line.");
    if (!graphAddTrack(graph_, a, b, distanceM, line))
        return fail("Could not add the connection (out of memory).");

    string text = stationName(a) + " <-> " + stationName(b) + " on " + lineName(line);
    pushUndo(ACT_ADD_CONNECTION, a, b, distanceM, line, "Add connection " + text);
    log("Added connection " + text);
    return ok("Connected " + text + " (" + to_string(distanceM) + " m).");
}

Result Metro::setStationOpen(int id, bool open)
{
    if (!graphHasStation(graph_, id))
        return fail("Invalid station.");
    if (graphIsOpen(graph_, id) == open)
        return fail(stationName(id) + " is already " + (open ? "open." : "closed."));
    graphSetOpen(graph_, id, open);
    string verb = open ? "Reopened " : "Closed ";
    pushUndo(open ? ACT_OPEN_STATION : ACT_CLOSE_STATION, id, 0, 0, 0, verb + stationName(id));
    log(verb + "station " + stationName(id));
    return ok(stationName(id) + (open ? " is open again." :
                                        " is closed. Routes will now avoid it."));
}

bool Metro::planRoute(int src, int dst, bool fewestStops, RouteHolder& out,
                      string& error) const
{
    if (!graphHasStation(graph_, src) || !graphHasStation(graph_, dst)) {
        error = "Invalid station.";
        return false;
    }
    if (src == dst) {
        error = "Source and destination must be different.";
        return false;
    }
    if (!graphIsOpen(graph_, src) || !graphIsOpen(graph_, dst)) {
        int closed = graphIsOpen(graph_, src) ? dst : src;
        error = stationName(closed) + " is closed for maintenance.";
        return false;
    }
    bool found = fewestStops ? bfsRoute(graph_, src, dst, false, &out.route)
                             : dijkstraRoute(graph_, src, dst, false, &out.route);
    if (!found) {
        error = "No open route between " + stationName(src) + " and " + stationName(dst) + ".";
        return false;
    }
    return true;
}

bool Metro::journeyFare(int src, int dst, int category, int& fare, int& line,
                           string& error) const
{
    RouteHolder rh;
    if (!planRoute(src, dst, false, rh, error))
        return false;
    fare = farePolicy(category).apply(fareForDistance(rh.route.totalMeters));
    line = rh.route.lines[0];
    return true;
}
