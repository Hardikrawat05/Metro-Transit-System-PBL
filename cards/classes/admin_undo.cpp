#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

unsigned long long hashPassword(const string& password)
{
    unsigned long long h = 5381;
    const string salted = "metro-pbl:" + password;
    for (unsigned char c : salted)
        h = h * 33 + c;
    return h;
}

void Metro::log(const string& text)
{
    Activity a{};
    a.time = Clock::now();
    copyText(a.text, sizeof a.text, text);
    dequeAddFront(activity_, &a);
    if (dequeSize(activity_) > ACTIVITY_LOG_MAX)
        dequeRemoveBack(activity_, nullptr);
    dirty_ = true;
}

void Metro::pushUndo(int type, int a, int b, int c, int d, const string& text)
{
    AdminAction action{};
    action.type = type;
    action.a = a;
    action.b = b;
    action.c = c;
    action.d = d;
    copyText(action.text, sizeof action.text, text);
    stackPush(undo_, &action);
}

Result Metro::adminLogin(const string& password)
{
    long long now = Clock::now();
    if (now < lockedUntil_)
        return fail("Too many wrong attempts. Try again in " +
                    to_string(lockedUntil_ - now) + " seconds.");

    if (hashPassword(password) == adminHash_) {
        failedLogins_ = 0;
        return ok("Login successful.");
    }

    failedLogins_++;
    if (failedLogins_ >= LOGIN_MAX_ATTEMPTS) {
        failedLogins_ = 0;
        lockedUntil_ = now + LOGIN_LOCK_SECONDS;
        log("Admin login locked after repeated wrong passwords");
        return fail("Wrong password. Admin login locked for " +
                    to_string(LOGIN_LOCK_SECONDS) + " seconds.");
    }
    return fail("Wrong password (attempt " + to_string(failedLogins_) + " of " +
                to_string(LOGIN_MAX_ATTEMPTS) + ").");
}

Result Metro::changePassword(const string& oldPassword, const string& newPassword)
{
    if (hashPassword(oldPassword) != adminHash_)
        return fail("Current password is wrong.");
    if (newPassword.size() < 4)
        return fail("New password must have at least 4 characters.");
    adminHash_ = hashPassword(newPassword);
    log("Admin password changed");
    return ok("Password changed.");
}

bool Metro::stationInUse(int station) const
{
    HmIter it;
    int key = 0;
    void* value = nullptr;

    tableIterStart(tickets_, &it);
    while (tableIterNext(&it, &key, &value)) {
        const Ticket* t = (const Ticket*)value;
        if (t->source == station || t->destination == station)
            return true;
    }
    tableIterStart(waitEntries_, &it);
    while (tableIterNext(&it, &key, &value)) {
        const WaitEntry* w = (const WaitEntry*)value;
        if (w->source == station || w->destination == station)
            return true;
    }
    tableIterStart(cards_, &it);
    while (tableIterNext(&it, &key, &value)) {
        const Card* c = (const Card*)value;
        if (c->tappedIn && c->entryStation == station)
            return true;
    }
    for (ListNode* n = trips_->head; n != nullptr; n = n->next) {
        const Trip* t = (const Trip*)n->data;
        if (t->source == station || t->destination == station)
            return true;
    }
    return false;
}

Result Metro::applyUndo(const AdminAction& a)
{
    switch (a.type) {
    case ACT_ADD_STATION:
        if (a.a != graphStationCount(graph_) - 1)
            return fail("Station is no longer the newest one.");
        if (stationInUse(a.a))
            return fail("Station is already used by tickets, cards or trips.");
        if (!graphRemoveLastStation(graph_))
            return fail("Station still has connections. Undo those first.");
        rebuildStationIndex();
        return ok("Undone: " + string(a.text));
    case ACT_ADD_CONNECTION:
        if (!graphRemoveTrack(graph_, a.a, a.b, a.d))
            return fail("Connection not found.");
        return ok("Undone: " + string(a.text));
    case ACT_CLOSE_STATION:
        graphSetOpen(graph_, a.a, true);
        return ok("Undone: " + string(a.text));
    case ACT_OPEN_STATION:
        graphSetOpen(graph_, a.a, false);
        return ok("Undone: " + string(a.text));
    case ACT_REGISTER_STUDENT: {
        if (a.b == 1) {
            tableRemove(students_, a.a);
        } else {
            int zero = 0;
            tablePut(students_, a.a, &zero);
        }
        return ok("Undone: " + string(a.text));
    }
    case ACT_DEACTIVATE_STUDENT: {
        int one = 1;
        tablePut(students_, a.a, &one);
        return ok("Undone: " + string(a.text));
    }
    case ACT_REGISTER_WOMEN_CARD: {
        if (a.b == 1) {
            tableRemove(womenCards_, a.a);
        } else {
            int zero = 0;
            tablePut(womenCards_, a.a, &zero);
        }
        return ok("Undone: " + string(a.text));
    }
    case ACT_DEACTIVATE_WOMEN_CARD: {
        int one = 1;
        tablePut(womenCards_, a.a, &one);
        return ok("Undone: " + string(a.text));
    }
    case ACT_SET_CAPACITY:
        if (!validLine(a.a))
            return fail("Line not found.");
        if (a.b < lines_[a.a].booked)
            return fail("More seats are booked than the old capacity allows.");
        lines_[a.a].capacity = a.b;
        return ok("Undone: " + string(a.text));
    default:
        return fail("Unknown action.");
    }
}

Result Metro::undoLast()
{
    AdminAction action{};
    if (!stackPop(undo_, &action))
        return fail("Nothing to undo.");
    Result r = applyUndo(action);
    if (!r.ok) {
        stackPush(undo_, &action);
        return r;
    }
    log(r.message);
    return r;
}

Array<string> Metro::undoHistory() const
{
    Array<string> out;
    for (size_t i = 0; i < stackSize(undo_); i++) {
        const AdminAction* a = (const AdminAction*)(stackFromTop(undo_, i));
        out.push_back(a->text);
    }
    return out;
}

Array<Activity> Metro::recentActivity(int maxItems) const
{
    Array<Activity> out;
    for (size_t i = 0; i < dequeSize(activity_) && (int)out.size() < maxItems; i++)
        out.push_back(*(const Activity*)(dequeAt(activity_, i)));
    return out;
}
