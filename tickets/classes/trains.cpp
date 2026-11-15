#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

const LineState& Metro::lineState(int line) const
{
    static const LineState none{};
    return validLine(line) ? lines_[line] : none;
}

Result Metro::dispatchTrain(int line, Array<string>& notes)
{
    if (!validLine(line))
        return fail("Invalid line.");
    expireOldTickets(&notes);

    LineState& ls = lines_[line];
    int missed = 0;
    HmIter it;
    tableIterStart(tickets_, &it);
    int key = 0;
    void* value = nullptr;
    while (tableIterNext(&it, &key, &value)) {
        Ticket* t = (Ticket*)value;
        if (t->line == line && t->trainId == ls.trainId && t->status == T_CONFIRMED) {
            t->status = T_EXPIRED;
            missed++;
        }
    }

    int oldTrain = ls.trainId;
    int boarded = ls.boarded;
    ls.trainId = nextTrainId_++;
    ls.booked = 0;
    ls.boarded = 0;
    log("Train " + to_string(oldTrain) + " departed on " + lineName(line) + " Line");
    processWaitlist(line, &notes);

    return ok("Train " + to_string(oldTrain) + " departed with " +
              to_string(boarded) + " passenger(s). " + to_string(missed) +
              " unused ticket(s) expired. Train " + to_string(ls.trainId) +
              " is now at the platform.");
}

Result Metro::setCapacity(int line, int capacity, Array<string>& notes)
{
    if (!validLine(line))
        return fail("Invalid line.");
    LineState& ls = lines_[line];
    if (capacity < 1 || capacity > MAX_TRAIN_CAPACITY)
        return fail("Capacity must be 1 to " + to_string(MAX_TRAIN_CAPACITY) + ".");
    if (capacity < ls.booked)
        return fail("Capacity cannot be below the " + to_string(ls.booked) +
                    " seats already booked.");
    int old = ls.capacity;
    ls.capacity = capacity;
    pushUndo(ACT_SET_CAPACITY, line, old, 0, 0,
             "Set " + lineName(line) + " capacity to " + to_string(capacity));
    log("Capacity of " + lineName(line) + " Line set to " + to_string(capacity));
    processWaitlist(line, &notes);
    return ok(lineName(line) + " Line capacity changed from " + to_string(old) + " to " +
              to_string(capacity) + ".");
}

NextTrain Metro::nextTrain(int line, long long now) const
{
    NextTrain nt;
    if (!validLine(line))
        return nt;
    int minute = Clock::minutesOfDay(now);
    int freq = lines_[line].frequencyMin > 0 ? lines_[line].frequencyMin : 1;

    if (minute < SERVICE_START_MIN) {
        nt.minutes = SERVICE_START_MIN - minute;
    } else if (minute >= SERVICE_END_MIN) {
        nt.minutes = (24 * 60 - minute) + SERVICE_START_MIN;
    } else {
        nt.running = true;
        nt.minutes = (freq - (minute - SERVICE_START_MIN) % freq) % freq;
    }
    return nt;
}

string Metro::crowdLevel(const LineState& ls)
{
    if (ls.capacity <= 0)
        return "-";
    int percent = ls.booked * 100 / ls.capacity;
    if (percent < 40)
        return "Low";
    if (percent < 80)
        return "Medium";
    return "High";
}
