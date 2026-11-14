#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

using namespace std;

Metro::Metro()
{
    graph_ = graph_create();
    students_ = hm_create(sizeof(int));
    womenCards_ = hm_create(sizeof(int));
    cards_ = hm_create(sizeof(Card));
    tickets_ = hm_create(sizeof(Ticket));
    waitEntries_ = hm_create(sizeof(WaitEntry));
    trips_ = list_create(sizeof(Trip));
    activity_ = deque_create(sizeof(Activity));
    undo_ = stack_create(sizeof(AdminAction));
    ticketIndex_ = bst_create();
    ticketAvl_ = avl_create();
    stationIndex_ = station_index_create();

    bool allOk = graph_ && students_ && womenCards_ && cards_ && tickets_ && waitEntries_ && trips_ &&
                 activity_ && undo_ && ticketIndex_ && ticketAvl_ && stationIndex_;
    for (int i = 0; i < MAX_LINES; i++) {
        waitlists_[i] = heap_create();
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
    graph_destroy(graph_);
    hm_destroy(students_);
    hm_destroy(womenCards_);
    hm_destroy(cards_);
    hm_destroy(tickets_);
    hm_destroy(waitEntries_);
    list_destroy(trips_);
    deque_destroy(activity_);
    stack_destroy(undo_);
    bst_destroy(ticketIndex_);
    avl_destroy(ticketAvl_);
    station_index_destroy(stationIndex_);
    for (int i = 0; i < MAX_LINES; i++) {
        heap_destroy(waitlists_[i]);
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

void Metro::seedDefaultNetwork()
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
    stack_clear(undo_);
    deque_clear(activity_);
    log(note);
}

bool Metro::validLine(int line) const
{
    return line >= 0 && line < graph_line_count(graph_);
}

int Metro::stationCount() const { return graph_station_count(graph_); }

int Metro::lineCount() const { return graph_line_count(graph_); }

string Metro::stationName(int id) const { return graph_station_name(graph_, id); }

string Metro::lineName(int line) const { return graph_line_name(graph_, line); }

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

bool Metro::isStationOpen(int id) const { return graph_is_open(graph_, id); }

int Metro::findStation(const string& idOrName) const
{
    string text = trim(idOrName);
    int id = -1;
    if (parseInt(text, id))
        return graph_valid_station(graph_, id) ? id : -1;
    return station_index_find(stationIndex_, text.c_str());
}

void Metro::rebuildStationIndex()
{
    station_index_build(stationIndex_, graph_);
}

Array<int> Metro::suggestStations(const string& prefix, int maxResults) const
{
    Array<int> out;
    string text = trim(prefix);
    if (text.empty() || maxResults <= 0)
        return out;
    out.resize(maxResults);
    int found = station_index_prefix(stationIndex_, text.c_str(), out.data(), maxResults);
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

Array<int> Metro::stationsCutOffIfClosed(int id) const
{
    Array<int> cutOff;
    int n = stationCount();
    if (!graph_valid_station(graph_, id) || !graph_is_open(graph_, id))
        return cutOff;

    Array<int> before(n), after(n);
    int groupsBefore = dfs_components(graph_, -1, before.data());
    int groupsAfter = dfs_components(graph_, id, after.data());
    if (groupsBefore < 0 || groupsAfter <= groupsBefore)
        return cutOff;

    int mainBefore = biggestGroup(before, groupsBefore, n);
    int mainAfter = biggestGroup(after, groupsAfter, n);
    for (int i = 0; i < n; i++)
        if (i != id && before[i] == mainBefore && after[i] != mainAfter)
            cutOff.push_back(i);
    return cutOff;
}

bool Metro::buildLineList(int line, LineList& out, bool& complete) const
{
    complete = true;
    if (!validLine(line) || out.list == nullptr)
        return false;
    dlist_clear(out.list);

    int n = stationCount();
    Array<int> degree(n, 0);
    int onLine = 0;
    for (int s = 0; s < n; s++) {
        for (const EdgeNode* e = graph_edges(graph_, s); e != nullptr; e = e->next)
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
        dlist_push_back(out.list, cur);
        seen[cur] = true;
        int next = -1;
        for (const EdgeNode* e = graph_edges(graph_, cur); e != nullptr; e = e->next)
            if (e->line == line && e->to != prev && !seen[e->to] && (next < 0 || e->to < next))
                next = e->to;
        prev = cur;
        cur = next;
    }
    complete = (int)(dlist_size(out.list)) == onLine;
    return true;
}

int Metro::trackDistance(int a, int b, int line) const
{
    for (const EdgeNode* e = graph_edges(graph_, a); e != nullptr; e = e->next)
        if (e->to == b && e->line == line)
            return e->dist_m;
    return -1;
}

bool Metro::isLoopLine(int line) const
{
    if (!validLine(line))
        return false;
    int onLine = 0;
    for (int s = 0; s < stationCount(); s++) {
        int degree = 0;
        for (const EdgeNode* e = graph_edges(graph_, s); e != nullptr; e = e->next)
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
    if (!buildLineList(line, ll, complete))
        return "";
    int a = dlist_index_of(ll.list, from);
    int b = dlist_index_of(ll.list, to);
    if (a < 0 || b < 0 || a == b)
        return "";
    return stationName(b > a ? ll.list->tail->value : ll.list->head->value);
}

Array<int> Metro::linesAtStation(int id) const
{
    int buf[MAX_LINES];
    int n = graph_station_lines(graph_, id, buf, MAX_LINES);
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
    int id = graph_add_station(graph_, clean.c_str());
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
    if (graph_line_count(graph_) >= MAX_LINES)
        return fail("Maximum of " + to_string(MAX_LINES) + " lines reached.");
    if (capacity < 1 || capacity > MAX_TRAIN_CAPACITY)
        return fail("Capacity must be 1 to " + to_string(MAX_TRAIN_CAPACITY) + ".");
    if (frequencyMin < 1 || frequencyMin > 60)
        return fail("Frequency must be 1 to 60 minutes.");
    int id = graph_add_line(graph_, clean.c_str());
    if (id < 0)
        return fail("Invalid or duplicate line name.");
    lines_[id] = LineState{nextTrainId_++, capacity, 0, 0, frequencyMin};
    log("Added line " + clean);
    return ok(clean + " Line added with ID " + to_string(id) + ".");
}

Result Metro::addConnection(int a, int b, int distanceM, int line)
{
    if (!graph_valid_station(graph_, a) || !graph_valid_station(graph_, b))
        return fail("Invalid station.");
    if (a == b)
        return fail("A station cannot be connected to itself.");
    if (!validLine(line))
        return fail("Invalid line.");
    if (distanceM < 100 || distanceM > 50000)
        return fail("Distance must be between 100 and 50000 metres.");
    if (graph_has_edge(graph_, a, b, line))
        return fail("These stations are already connected on this line.");
    if (!graph_add_edge(graph_, a, b, distanceM, line))
        return fail("Could not add the connection (out of memory).");

    string text = stationName(a) + " <-> " + stationName(b) + " on " + lineName(line);
    pushUndo(ACT_ADD_CONNECTION, a, b, distanceM, line, "Add connection " + text);
    log("Added connection " + text);
    return ok("Connected " + text + " (" + to_string(distanceM) + " m).");
}

Result Metro::setStationOpen(int id, bool open)
{
    if (!graph_valid_station(graph_, id))
        return fail("Invalid station.");
    if (graph_is_open(graph_, id) == open)
        return fail(stationName(id) + " is already " + (open ? "open." : "closed."));
    graph_set_open(graph_, id, open);
    string verb = open ? "Reopened " : "Closed ";
    pushUndo(open ? ACT_OPEN_STATION : ACT_CLOSE_STATION, id, 0, 0, 0, verb + stationName(id));
    log(verb + "station " + stationName(id));
    return ok(stationName(id) + (open ? " is open again." :
                                        " is closed. Routes will now avoid it."));
}

bool Metro::planRoute(int src, int dst, bool fewestStops, RouteHolder& out,
                      string& error) const
{
    if (!graph_valid_station(graph_, src) || !graph_valid_station(graph_, dst)) {
        error = "Invalid station.";
        return false;
    }
    if (src == dst) {
        error = "Source and destination must be different.";
        return false;
    }
    if (!graph_is_open(graph_, src) || !graph_is_open(graph_, dst)) {
        int closed = graph_is_open(graph_, src) ? dst : src;
        error = stationName(closed) + " is closed for maintenance.";
        return false;
    }
    bool found = fewestStops ? graph_fewest_stops(graph_, src, dst, false, &out.route)
                             : graph_shortest_path(graph_, src, dst, false, &out.route);
    if (!found) {
        error = "No open route between " + stationName(src) + " and " + stationName(dst) + ".";
        return false;
    }
    return true;
}

bool Metro::fareForJourney(int src, int dst, int category, int& fare, int& line,
                           string& error) const
{
    RouteHolder rh;
    if (!planRoute(src, dst, false, rh, error))
        return false;
    fare = farePolicy(category).apply(baseFareForDistance(rh.route.total_m));
    line = rh.route.lines[0];
    return true;
}

const FarePolicy& farePolicy(int category)
{
    static const RegularFare regular;
    static const StudentFare student;
    static const SeniorFare senior;
    static const DisabledFare disabled;

    switch (category) {
    case STUDENT:
        return student;
    case SENIOR:
        return senior;
    case DISABLED:
        return disabled;
    default:
        return regular;
    }
}

int baseFareForDistance(long long meters)
{
    if (meters <= 2000)
        return 10;
    if (meters <= 5000)
        return 20;
    if (meters <= 12000)
        return 30;
    if (meters <= 21000)
        return 40;
    if (meters <= 32000)
        return 50;
    return 60;
}

bool validCategory(int category)
{
    return category >= REGULAR && category <= DISABLED;
}

struct SeedLine {
    int number = 0;
    string text;
};

static string trimmed(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool readLines(const string& path, Array<SeedLine>& out)
{
    ifstream in(path);
    if (!in)
        return false;
    string text;
    int number = 0;
    while (getline(in, text)) {
        number++;
        if (number == 1 && text.size() >= 3 && text.compare(0, 3, "\xEF\xBB\xBF") == 0)
            text.erase(0, 3);
        text = trimmed(text);
        if (text.empty() || text[0] == '#')
            continue;
        SeedLine line;
        line.number = number;
        line.text = text;
        out.push_back(line);
    }
    return true;
}

static Array<string> fields(const string& text)
{
    Array<string> parts;
    string current;
    for (char c : text) {
        if (c == '|') {
            parts.push_back(trimmed(current));
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(trimmed(current));
    return parts;
}

static bool digitsToNumber(const string& text, long long& result)
{
    if (text.empty() || text.size() > 18)
        return false;
    size_t i = 0;
    bool negative = false;
    if (text[0] == '-') {
        negative = true;
        i = 1;
        if (text.size() == 1)
            return false;
    }
    long long value = 0;
    for (; i < text.size(); i++) {
        if (text[i] < '0' || text[i] > '9')
            return false;
        value = value * 10 + (text[i] - '0');
    }
    result = negative ? -value : value;
    return true;
}

static bool toInt(const string& text, int& out)
{
    long long value = 0;
    if (!digitsToNumber(text, value) || value < -2147483647LL || value > 2147483647LL)
        return false;
    out = (int)value;
    return true;
}

static int toCategory(string word)
{
    for (char& c : word)
        c = (char)(tolower((unsigned char)c));
    if (word == "regular")
        return REGULAR;
    if (word == "student")
        return STUDENT;
    if (word == "senior")
        return SENIOR;
    if (word == "disabled")
        return DISABLED;
    return -1;
}

static string where(const string& path, const SeedLine& line, const string& problem)
{
    return path + " line " + to_string(line.number) + ": " + problem;
}

SeedStatus loadSeedData(Metro& m, const string& folder, string& error, string& summary)
{
    const string base = folder.empty() ? string("") : folder + "/";
    const string linesPath = base + "lines.txt";
    const string stationsPath = base + "stations.txt";
    const string connectionsPath = base + "connections.txt";
    const string studentsPath = base + "students.txt";
    const string cardsPath = base + "cards.txt";
    const string womenPath = base + "women_cards.txt";

    Array<SeedLine> lines, stations, connections, students, cards, women;
    bool haveLines = readLines(linesPath, lines);
    bool haveStations = readLines(stationsPath, stations);
    if (!haveLines && !haveStations)
        return SeedStatus::FolderMissing;
    if (!haveLines || !haveStations || !readLines(connectionsPath, connections)) {
        error = "Missing file: need " + linesPath + ", " + stationsPath + " and " + connectionsPath;
        return SeedStatus::Error;
    }
    readLines(studentsPath, students);
    readLines(womenPath, women);
    readLines(cardsPath, cards);

    for (const SeedLine& l : lines) {
        Array<string> f = fields(l.text);
        int capacity = 0, frequency = 0;
        if (f.size() != 3 || !toInt(f[1], capacity) || !toInt(f[2], frequency)) {
            error = where(linesPath, l, "expected 'name | capacity | minutes', got '" + l.text + "'");
            return SeedStatus::Error;
        }
        Result r = m.addLine(f[0], capacity, frequency);
        if (!r.ok) {
            error = where(linesPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    for (const SeedLine& l : stations) {
        if (l.text.find('|') != string::npos) {
            error = where(stationsPath, l, "a station name cannot contain '|'");
            return SeedStatus::Error;
        }
        Result r = m.addStation(l.text);
        if (!r.ok) {
            error = where(stationsPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    for (const SeedLine& l : connections) {
        Array<string> f = fields(l.text);
        int meters = 0;
        if (f.size() != 4 || !toInt(f[2], meters)) {
            error = where(connectionsPath, l,
                          "expected 'station | station | metres | line', got '" + l.text + "'");
            return SeedStatus::Error;
        }
        int a = m.findStation(f[0]);
        int b = m.findStation(f[1]);
        int line = m.findLine(f[3]);
        if (a < 0 || b < 0) {
            error = where(connectionsPath, l,
                          "station '" + (a < 0 ? f[0] : f[1]) + "' is not in " + stationsPath);
            return SeedStatus::Error;
        }
        if (line < 0) {
            error = where(connectionsPath, l, "line '" + f[3] + "' is not in " + linesPath);
            return SeedStatus::Error;
        }
        Result r = m.addConnection(a, b, meters, line);
        if (!r.ok) {
            error = where(connectionsPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    for (const SeedLine& l : students) {
        int id = 0;
        if (!toInt(l.text, id)) {
            error = where(studentsPath, l, "expected a student ID number, got '" + l.text + "'");
            return SeedStatus::Error;
        }
        Result r = m.registerStudent(id);
        if (!r.ok) {
            error = where(studentsPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    for (const SeedLine& l : women) {
        int id = 0;
        if (!toInt(l.text, id)) {
            error = where(womenPath, l, "expected a women's card number, got '" + l.text + "'");
            return SeedStatus::Error;
        }
        Result r = m.registerWomenCard(id);
        if (!r.ok) {
            error = where(womenPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    for (const SeedLine& l : cards) {
        Array<string> f = fields(l.text);
        int studentId = 0, balance = 0;
        int category = f.size() == 4 ? toCategory(f[1]) : -1;
        if (f.size() != 4 || category < 0 || !toInt(f[2], studentId) || !toInt(f[3], balance)) {
            error = where(cardsPath, l,
                          "expected 'name | regular/student/senior/disabled | student ID | "
                          "balance', got '" + l.text + "'");
            return SeedStatus::Error;
        }
        int newId = 0;
        Result r = m.createCard(f[0], category, studentId, balance, newId);
        if (!r.ok) {
            error = where(cardsPath, l, r.message);
            return SeedStatus::Error;
        }
    }

    summary = to_string(lines.size()) + " lines, " + to_string(stations.size()) + " stations, " +
              to_string(connections.size()) + " connections, " + to_string(students.size()) +
              " students, " + to_string(women.size()) + " women's cards, " +
              to_string(cards.size()) + " smart cards";
    m.finishSetup("System initialised from " + (folder.empty() ? string(".") : folder) +
                  "/ (" + summary + ")");
    return SeedStatus::Loaded;
}
