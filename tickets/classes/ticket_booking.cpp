#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

Ticket* Metro::ticketPtr(int id)
{
    return (Ticket*)(tableGet(tickets_, id));
}

int Metro::createTicket(const string& name, int category, int studentId, int src,
                        int dst, int line, int fare, int womenCardId)
{
    Ticket t{};
    t.womenCardId = womenCardId;
    t.id = nextTicketId_++;
    copyText(t.name, sizeof t.name, name);
    t.category = category;
    t.studentId = studentId;
    t.source = src;
    t.destination = dst;
    t.line = line;
    t.trainId = lines_[line].trainId;
    t.fare = fare;
    t.status = T_CONFIRMED;
    t.issuedAt = Clock::now();
    t.validUntil = t.issuedAt + TICKET_VALID_MIN * 60LL;
    tablePut(tickets_, t.id, &t);
    bstInsert(ticketIndex_, t.id);
    avlInsert(ticketAvl_, t.id);
    lines_[line].booked++;
    ticketRevenue_ += fare;
    return t.id;
}

static const int RESERVED_SEATS = 2;
static const int RESERVE_FROM_CAPACITY = 4;

static bool hasPriority(int level)
{
    return level <= PRIORITY_WOMEN;
}

static bool onlyReservedLeft(const LineState& ls)
{
    return ls.capacity >= RESERVE_FROM_CAPACITY && ls.capacity - ls.booked <= RESERVED_SEATS;
}

static bool canTakeSeat(const LineState& ls, int level)
{
    if (ls.booked >= ls.capacity)
        return false;
    if (onlyReservedLeft(ls))
        return hasPriority(level);
    return true;
}

BookingResult Metro::bookTicket(const string& name, int category, int studentId,
                                int src, int dst, int womenCardId)
{
    BookingResult r;
    string clean = trim(name);
    expireOldTickets();

    if (clean.empty() || clean.size() >= NAME_LEN) {
        r.message = "Name must be 1 to " + to_string(NAME_LEN - 1) + " characters.";
        return r;
    }
    if (!isValidCategory(category)) {
        r.message = "Invalid passenger category.";
        return r;
    }
    if (category == STUDENT && !isValidStudent(studentId)) {
        r.message = "Student ID " + to_string(studentId) +
                    " is not registered or is inactive. Discount cannot be applied.";
        return r;
    }
    if (womenCardId != 0 && !isValidWomenCard(womenCardId)) {
        r.message = "Women's card " + to_string(womenCardId) +
                    " is not registered or is inactive. Priority cannot be given.";
        return r;
    }

    int fare = 0, line = 0;
    string error;
    if (!journeyFare(src, dst, category, fare, line, error)) {
        r.message = error;
        return r;
    }
    if (category != STUDENT)
        studentId = 0;

    LineState& ls = lines_[line];
    int level = waitPriority(category, womenCardId);
    if (canTakeSeat(ls, level)) {
        bool reserved = onlyReservedLeft(ls);
        r.ok = true;
        r.fare = fare;
        r.ticketId = createTicket(clean, category, studentId, src, dst, line, fare, womenCardId);
        r.message = reserved ? "Ticket confirmed (reserved seat)." : "Ticket confirmed.";
        log("Ticket " + to_string(r.ticketId) + " booked for " + clean);
        return r;
    }

    WaitEntry w{};
    w.id = nextWaitId_++;
    copyText(w.name, sizeof w.name, clean);
    w.category = category;
    w.studentId = studentId;
    w.source = src;
    w.destination = dst;
    w.line = line;
    w.seq = nextWaitSeq_++;
    w.active = 1;
    w.joinedAt = Clock::now();
    w.womenCardId = womenCardId;
    tablePut(waitEntries_, w.id, &w);

    HeapItem item;
    item.key1 = level;
    item.key2 = w.seq;
    item.value = w.id;
    heapPush(waitlists_[line], item);

    r.ok = true;
    r.waitlisted = true;
    r.waitId = w.id;
    r.fare = fare;
    r.position = waitlistPosition(line, w.id);
    if (ls.booked < ls.capacity)
        r.message = "Only reserved seats are left on the " + lineName(line) +
                    " Line train (for senior citizens, persons with disability and women "
                    "with a verified card). Added to waiting list.";
    else
        r.message = "Train on " + lineName(line) + " Line is full. Added to waiting list.";
    log("Waitlist #" + to_string(w.id) + " created for " + clean);
    return r;
}

void Metro::processWaitlist(int line, Array<string>* notes)
{
    LineState& ls = lines_[line];
    HeapItem item;
    while (ls.booked < ls.capacity && heapPop(waitlists_[line], &item)) {
        WaitEntry* w = (WaitEntry*)(tableGet(waitEntries_, item.value));
        if (w == nullptr || !w->active)
            continue;
        if (!canTakeSeat(ls, (int)item.key1)) {
            heapPush(waitlists_[line], item);
            break;
        }

        int fare = 0, routeLine = 0;
        string error;
        bool studentOk = w->category != STUDENT || isValidStudent(w->studentId);
        if (!studentOk || !journeyFare(w->source, w->destination, w->category, fare,
                                          routeLine, error) ||
            routeLine != line) {
            w->active = 0;
            if (notes)
                notes->push_back("Waitlist #" + to_string(w->id) + " (" + w->name +
                                 ") could not be confirmed: " +
                                 (studentOk ? (error.empty() ? "route changed" : error)
                                            : "student ID no longer active"));
            continue;
        }

        int ticketId = createTicket(w->name, w->category, w->studentId, w->source,
                                    w->destination, line, fare, w->womenCardId);
        w->active = 0;
        w->ticketId = ticketId;
        if (notes)
            notes->push_back("Waitlist #" + to_string(w->id) + " (" + w->name +
                             ") confirmed -> Ticket " + to_string(ticketId));
        log("Waitlist #" + to_string(w->id) + " confirmed as ticket " +
            to_string(ticketId));
    }
}

int Metro::expireOldTickets(Array<string>* notes)
{
    long long now = Clock::now();
    bool affected[MAX_LINES] = {};
    int expired = 0;

    HmIter it;
    tableIterStart(tickets_, &it);
    int key = 0;
    void* value = nullptr;
    while (tableIterNext(&it, &key, &value)) {
        Ticket* t = (Ticket*)value;
        if (t->status != T_CONFIRMED || now <= t->validUntil)
            continue;
        t->status = T_EXPIRED;
        expired++;
        if (t->trainId == lines_[t->line].trainId && lines_[t->line].booked > 0) {
            lines_[t->line].booked--;
            affected[t->line] = true;
        }
    }

    for (int line = 0; line < MAX_LINES; line++)
        if (affected[line])
            processWaitlist(line, notes);
    if (expired > 0)
        log(to_string(expired) + " ticket(s) expired (validity over)");
    return expired;
}

Result Metro::cancelTicket(int ticketId, Array<string>& notes)
{
    expireOldTickets(&notes);
    Ticket* t = ticketPtr(ticketId);
    if (t == nullptr)
        return fail("Ticket not found.");
    if (t->status != T_CONFIRMED)
        return fail("Only confirmed tickets can be cancelled.");
    if (t->trainId != lines_[t->line].trainId)
        return fail("This ticket's train has already left.");

    t->status = T_CANCELLED;
    ticketRevenue_ -= t->fare;
    lines_[t->line].booked--;
    int line = t->line;
    int refund = t->fare;
    log("Ticket " + to_string(ticketId) + " cancelled");
    processWaitlist(line, &notes);
    return ok("Ticket cancelled. Refund: " + rs(refund) + ".");
}

Result Metro::boardTicket(int ticketId)
{
    expireOldTickets();
    Ticket* t = ticketPtr(ticketId);
    if (t == nullptr)
        return fail("Ticket not found. Entry denied.");
    if (t->status == T_EXPIRED)
        return fail("Ticket has expired. Entry denied.");
    if (t->status != T_CONFIRMED)
        return fail("Ticket is not valid for boarding (already used or cancelled).");
    if (t->trainId != lines_[t->line].trainId)
        return fail("Ticket belongs to another train.");
    if (!graphIsOpen(graph_, t->source))
        return fail(stationName(t->source) + " is closed. Please contact staff.");

    t->status = T_BOARDED;
    lines_[t->line].boarded++;

    Trip trip{};
    trip.ticketId = ticketId;
    trip.source = t->source;
    trip.destination = t->destination;
    trip.fare = t->fare;
    trip.time = Clock::now();
    listAddFront(trips_, &trip);

    log("Ticket " + to_string(ticketId) + " boarded train " +
        to_string(t->trainId));
    return ok("Ticket verified. " + string(t->name) + " may board train " +
              to_string(t->trainId) + " (" + lineName(t->line) + " Line).");
}

Result Metro::leaveWaitlist(int waitId)
{
    WaitEntry* w = (WaitEntry*)(tableGet(waitEntries_, waitId));
    if (w == nullptr)
        return fail("Waiting list entry not found.");
    if (!w->active)
        return fail(w->ticketId ? "Already confirmed as ticket " + to_string(w->ticketId) + "."
                                : "This entry is no longer on the waiting list.");
    w->active = 0;
    log("Waitlist #" + to_string(waitId) + " left the waiting list");
    return ok("Removed from the waiting list.");
}

const Ticket* Metro::findTicket(int id) const
{
    return (const Ticket*)(tableGet(tickets_, id));
}

const WaitEntry* Metro::findWaitEntry(int id) const
{
    return (const WaitEntry*)(tableGet(waitEntries_, id));
}

Array<WaitEntry> Metro::waitlistInOrder(int line) const
{
    Array<WaitEntry> out;
    if (!validLine(line))
        return out;
    Heap* copy = heapCopy(waitlists_[line]);
    HeapItem item;
    while (copy != nullptr && heapPop(copy, &item)) {
        const WaitEntry* w = findWaitEntry(item.value);
        if (w != nullptr && w->active)
            out.push_back(*w);
    }
    freeHeap(copy);
    return out;
}

int Metro::waitlistPosition(int line, int waitId) const
{
    Array<WaitEntry> order = waitlistInOrder(line);
    for (size_t i = 0; i < order.size(); i++)
        if (order[i].id == waitId)
            return (int)i + 1;
    return 0;
}

Array<Ticket> Metro::ticketsFromIds(const Array<int>& ids) const
{
    Array<Ticket> out;
    for (int id : ids) {
        const Ticket* t = findTicket(id);
        if (t != nullptr)
            out.push_back(*t);
    }
    return out;
}

Array<Ticket> Metro::allTickets() const
{
    Array<int> ids(avlSize(ticketAvl_));
    ids.resize(avlInorder(ticketAvl_, ids.data(), ids.size()));
    return ticketsFromIds(ids);
}

Array<Ticket> Metro::ticketsInRange(int lo, int hi) const
{
    Array<int> ids(avlSize(ticketAvl_));
    ids.resize(avlRange(ticketAvl_, lo, hi, ids.data(), ids.size()));
    return ticketsFromIds(ids);
}

static int compareByFare(const void* a, const void* b)
{
    const Ticket* x = (const Ticket*)a;
    const Ticket* y = (const Ticket*)b;
    if (x->fare != y->fare)
        return y->fare - x->fare;
    return x->id - y->id;
}

Array<Ticket> Metro::ticketsByFare() const
{
    Array<Ticket> out = collectValues<Ticket>(tickets_);
    if (out.size() > 1)
        quickSort(out.data(), out.size(), sizeof(Ticket), compareByFare);
    return out;
}

int Metro::ticketIndexHeight() const
{
    return bstHeight(ticketIndex_);
}

int Metro::ticketAvlHeight() const
{
    return avlHeight(ticketAvl_);
}
