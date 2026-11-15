#include "metro.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

using namespace std;

static const char* const HEADER = "METRO_DATA v1";

static string clean(const char* text)
{
    string s(text);
    for (char& c : s)
        if (c == '|' || c == '\n' || c == '\r')
            c = ' ';
    return s;
}

static Array<string> split(const string& line)
{
    Array<string> parts;
    string current;
    for (char c : line) {
        if (c == '|') {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
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

static long long toNumber(const string& text)
{
    long long value = 0;
    if (!digitsToNumber(text, value))
        throw runtime_error("bad number '" + text + "'");
    return value;
}

static unsigned long long toBigNumber(const string& text)
{
    if (text.empty() || text.size() > 20)
        throw runtime_error("bad number '" + text + "'");
    unsigned long long value = 0;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] < '0' || text[i] > '9')
            throw runtime_error("bad number '" + text + "'");
        value = value * 10 + (text[i] - '0');
    }
    return value;
}

static int toInt(const string& text)
{
    long long v = toNumber(text);
    if (v < -2147483647LL || v > 2147483647LL)
        throw runtime_error("number out of range '" + text + "'");
    return (int)v;
}

static void need(const Array<string>& f, size_t count)
{
    if (f.size() != count)
        throw runtime_error(f[0] + " record needs " + to_string(count) +
                                 " fields, found " + to_string(f.size()));
}

static void needOneOf(const Array<string>& f, size_t a, size_t b)
{
    if (f.size() != a && f.size() != b)
        throw runtime_error(f[0] + " record needs " + to_string(a) + " or " + to_string(b) +
                            " fields, found " + to_string(f.size()));
}

static void check(bool condition, const string& message)
{
    if (!condition)
        throw runtime_error(message);
}

bool Storage::save(const Metro& m, const string& path, string& error)
{
    const string tmp = path + ".tmp";
    {
        ofstream out(tmp, ios::trunc);
        if (!out) {
            error = "Cannot write to " + tmp;
            return false;
        }

        out << HEADER << "\n";
        out << "META|" << m.nextCardId_ << "|" << m.nextTicketId_ << "|" << m.nextWaitId_ << "|"
            << m.nextTrainId_ << "|" << m.nextWaitSeq_ << "|" << m.ticketRevenue_ << "|"
            << m.cardRevenue_ << "|" << m.passRevenue_ << "|" << m.adminHash_ << "\n";

        for (int i = 0; i < graphLineCount(m.graph_); i++) {
            const LineState& ls = m.lines_[i];
            out << "LINE|" << i << "|" << clean(graphLineName(m.graph_, i)) << "|"
                << ls.trainId << "|" << ls.capacity << "|" << ls.booked << "|" << ls.boarded
                << "|" << ls.frequencyMin << "\n";
        }

        for (int i = 0; i < graphStationCount(m.graph_); i++)
            out << "STATION|" << i << "|" << clean(graphStationName(m.graph_, i)) << "|"
                << (graphIsOpen(m.graph_, i) ? 1 : 0) << "\n";

        for (int i = 0; i < graphStationCount(m.graph_); i++)
            for (const EdgeNode* e = graphTracks(m.graph_, i); e != nullptr; e = e->next)
                if (i < e->to)
                    out << "EDGE|" << i << "|" << e->to << "|" << e->meters << "|" << e->line
                        << "\n";

        for (const StudentRecord& s : m.students())
            out << "STUDENT|" << s.id << "|" << s.active << "\n";

        for (const StudentRecord& w : m.womenCards())
            out << "WOMEN|" << w.id << "|" << w.active << "\n";

        Array<Card> cards = collectValues<Card>(m.cards_);
        sortRecords(cards, compareById<Card>);
        for (const Card& c : cards)
            out << "CARD|" << c.id << "|" << clean(c.name) << "|" << c.category << "|"
                << c.studentId << "|" << c.balance << "|" << c.status << "|" << c.tappedIn
                << "|" << c.entryStation << "|" << c.entryTime << "|" << c.passType << "|"
                << c.passExpiry << "|" << c.createdAt << "\n";

        for (const Ticket& t : m.allTickets())
            out << "TICKET|" << t.id << "|" << clean(t.name) << "|" << t.category << "|"
                << t.studentId << "|" << t.source << "|" << t.destination << "|" << t.line
                << "|" << t.trainId << "|" << t.fare << "|" << t.status << "|" << t.issuedAt
                << "|" << t.validUntil << "|" << t.womenCardId << "\n";

        Array<WaitEntry> waits = collectValues<WaitEntry>(m.waitEntries_);
        sortRecords(waits, compareById<WaitEntry>);
        for (const WaitEntry& w : waits)
            out << "WAIT|" << w.id << "|" << clean(w.name) << "|" << w.category << "|"
                << w.studentId << "|" << w.source << "|" << w.destination << "|" << w.line
                << "|" << w.seq << "|" << w.active << "|" << w.joinedAt << "|" << w.ticketId
                << "|" << w.womenCardId << "\n";

        for (ListNode* n = m.trips_->head; n != nullptr; n = n->next) {
            const Trip* t = (const Trip*)n->data;
            out << "TRIP|" << t->cardId << "|" << t->ticketId << "|" << t->source << "|"
                << t->destination << "|" << t->fare << "|" << t->time << "\n";
        }

        out << "END\n";
        out.flush();
        if (!out) {
            error = "Writing " + tmp + " failed (disk full?)";
            return false;
        }
    }

    remove(path.c_str());
    if (rename(tmp.c_str(), path.c_str()) != 0) {
        error = "Could not replace " + path;
        return false;
    }
    return true;
}

LoadStatus Storage::load(Metro& m, const string& path, string& error)
{
    ifstream in(path);
    string usedPath = path;
    if (!in) {
        in.clear();
        in.open(path + ".tmp");
        usedPath = path + ".tmp";
        if (!in)
            return LoadStatus::NotFound;
    }

    if (graphStationCount(m.graph_) != 0 || graphLineCount(m.graph_) != 0) {
        error = "Load must be done on an empty system.";
        return LoadStatus::Corrupt;
    }

    string line;
    int lineNo = 0;
    bool sawHeader = false;
    bool sawEnd = false;

    try {
        while (getline(in, line)) {
            lineNo++;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                continue;
            if (!sawHeader) {
                check(line == HEADER, "missing header '" + string(HEADER) + "'");
                sawHeader = true;
                continue;
            }
            check(!sawEnd, "data after END");

            Array<string> f = split(line);
            const string& type = f[0];

            if (type == "END") {
                sawEnd = true;
            } else if (type == "META") {
                need(f, 10);
                m.nextCardId_ = toInt(f[1]);
                m.nextTicketId_ = toInt(f[2]);
                m.nextWaitId_ = toInt(f[3]);
                m.nextTrainId_ = toInt(f[4]);
                m.nextWaitSeq_ = toNumber(f[5]);
                m.ticketRevenue_ = toNumber(f[6]);
                m.cardRevenue_ = toNumber(f[7]);
                m.passRevenue_ = toNumber(f[8]);
                m.adminHash_ = toBigNumber(f[9]);
            } else if (type == "LINE") {
                need(f, 8);
                int id = toInt(f[1]);
                check(id == graphLineCount(m.graph_) && id < MAX_LINES, "line ids out of order");
                check(graphAddLine(m.graph_, f[2].c_str()) == id, "bad line name");
                LineState ls{toInt(f[3]), toInt(f[4]), toInt(f[5]), toInt(f[6]), toInt(f[7])};
                check(ls.capacity > 0 && ls.frequencyMin > 0 && ls.booked >= 0 &&
                          ls.booked <= ls.capacity && ls.boarded >= 0,
                      "bad line values");
                m.lines_[id] = ls;
            } else if (type == "STATION") {
                need(f, 4);
                int id = toInt(f[1]);
                check(id == graphStationCount(m.graph_), "station ids out of order");
                check(graphAddStation(m.graph_, f[2].c_str()) == id, "bad station name");
                graphSetOpen(m.graph_, id, toInt(f[3]) != 0);
            } else if (type == "EDGE") {
                need(f, 5);
                check(graphAddTrack(m.graph_, toInt(f[1]), toInt(f[2]), toInt(f[3]),
                                     toInt(f[4])),
                      "bad connection");
            } else if (type == "STUDENT") {
                need(f, 3);
                int active = toInt(f[2]) != 0 ? 1 : 0;
                tablePut(m.students_, toInt(f[1]), &active);
            } else if (type == "WOMEN") {
                need(f, 3);
                int active = toInt(f[2]) != 0 ? 1 : 0;
                tablePut(m.womenCards_, toInt(f[1]), &active);
            } else if (type == "CARD") {
                need(f, 13);
                Card c{};
                c.id = toInt(f[1]);
                copyText(c.name, sizeof c.name, f[2]);
                c.category = toInt(f[3]);
                c.studentId = toInt(f[4]);
                c.balance = toInt(f[5]);
                c.status = toInt(f[6]);
                c.tappedIn = toInt(f[7]);
                c.entryStation = toInt(f[8]);
                c.entryTime = toNumber(f[9]);
                c.passType = toInt(f[10]);
                c.passExpiry = toNumber(f[11]);
                c.createdAt = toNumber(f[12]);
                check(isValidCategory(c.category), "bad card category");
                check(!c.tappedIn || graphHasStation(m.graph_, c.entryStation),
                      "bad card entry station");
                tablePut(m.cards_, c.id, &c);
            } else if (type == "TICKET") {
                needOneOf(f, 13, 14);
                Ticket t{};
                t.id = toInt(f[1]);
                copyText(t.name, sizeof t.name, f[2]);
                t.category = toInt(f[3]);
                t.studentId = toInt(f[4]);
                t.source = toInt(f[5]);
                t.destination = toInt(f[6]);
                t.line = toInt(f[7]);
                t.trainId = toInt(f[8]);
                t.fare = toInt(f[9]);
                t.status = toInt(f[10]);
                t.issuedAt = toNumber(f[11]);
                t.validUntil = toNumber(f[12]);
                t.womenCardId = f.size() > 13 ? toInt(f[13]) : 0;
                check(isValidCategory(t.category) && t.status >= 0 && t.status <= T_EXPIRED,
                      "bad ticket values");
                check(t.line >= 0 && t.line < graphLineCount(m.graph_), "bad ticket line");
                check(graphHasStation(m.graph_, t.source) &&
                          graphHasStation(m.graph_, t.destination),
                      "bad ticket station");
                tablePut(m.tickets_, t.id, &t);
                bstInsert(m.ticketIndex_, t.id);
                avlInsert(m.ticketAvl_, t.id);
            } else if (type == "WAIT") {
                needOneOf(f, 12, 13);
                WaitEntry w{};
                w.id = toInt(f[1]);
                copyText(w.name, sizeof w.name, f[2]);
                w.category = toInt(f[3]);
                w.studentId = toInt(f[4]);
                w.source = toInt(f[5]);
                w.destination = toInt(f[6]);
                w.line = toInt(f[7]);
                w.seq = toNumber(f[8]);
                w.active = toInt(f[9]);
                w.joinedAt = toNumber(f[10]);
                w.ticketId = toInt(f[11]);
                w.womenCardId = f.size() > 12 ? toInt(f[12]) : 0;
                check(isValidCategory(w.category), "bad waitlist category");
                check(w.line >= 0 && w.line < graphLineCount(m.graph_), "bad waitlist line");
                tablePut(m.waitEntries_, w.id, &w);
                if (w.active) {
                    HeapItem item;
                    item.key1 = Metro::waitPriority(w.category, w.womenCardId);
                    item.key2 = w.seq;
                    item.value = w.id;
                    heapPush(m.waitlists_[w.line], item);
                }
            } else if (type == "TRIP") {
                need(f, 7);
                Trip t{toInt(f[1]), toInt(f[2]), toInt(f[3]), toInt(f[4]), toInt(f[5]),
                       toNumber(f[6])};
                listAddBack(m.trips_, &t);
            } else {
                throw runtime_error("unknown record type '" + type + "'");
            }
        }
        check(sawHeader, "file is empty");
        check(sawEnd, "file is incomplete (no END line)");
    } catch (const exception& ex) {
        error = usedPath + " line " + to_string(lineNo) + ": " + ex.what();
        return LoadStatus::Corrupt;
    }

    m.rebuildStationIndex();
    m.log("Data loaded from " + usedPath);
    m.clearDirty();
    return LoadStatus::Loaded;
}
