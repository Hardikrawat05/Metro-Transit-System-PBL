#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

using namespace std;

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

SeedStatus loadStartingData(Metro& m, const string& folder, string& error, string& summary)
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
