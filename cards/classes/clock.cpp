#include "metro.h"

#include <ctime>

using namespace std;

long long Clock::offsetSeconds_ = 0;

long long Clock::now()
{
    return (long long)(time(nullptr)) + offsetSeconds_;
}

void Clock::advanceMinutes(long long minutes)
{
    offsetSeconds_ += minutes * 60;
}

void Clock::reset()
{
    offsetSeconds_ = 0;
}

long long Clock::offsetMinutes()
{
    return offsetSeconds_ / 60;
}

string Clock::format(long long t)
{
    time_t tt = (time_t)t;
    tm* local = localtime(&tt);
    if (local == nullptr)
        return "?";
    char buf[32];
    strftime(buf, sizeof buf, "%d-%m-%Y %H:%M", local);
    return buf;
}

int Clock::minutesOfDay(long long t)
{
    time_t tt = (time_t)t;
    tm* local = localtime(&tt);
    if (local == nullptr)
        return 0;
    return local->tm_hour * 60 + local->tm_min;
}
