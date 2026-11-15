#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

static int compareStationCount(const void* a, const void* b)
{
    const StationCount* x = (const StationCount*)a;
    const StationCount* y = (const StationCount*)b;
    if (x->count != y->count)
        return y->count - x->count;
    return x->station - y->station;
}

static int compareRouteCount(const void* a, const void* b)
{
    const RouteCount* x = (const RouteCount*)a;
    const RouteCount* y = (const RouteCount*)b;
    if (x->count != y->count)
        return y->count - x->count;
    if (x->source != y->source)
        return x->source - y->source;
    return x->destination - y->destination;
}

Report Metro::buildReport() const
{
    Report r;
    r.ticketRevenue = ticketRevenue_;
    r.cardRevenue = cardRevenue_;
    r.passRevenue = passRevenue_;
    r.ticketIndexSize = (int)(bstSize(ticketIndex_));
    r.ticketIndexHeight = bstHeight(ticketIndex_);
    r.ticketAvlHeight = avlHeight(ticketAvl_);

    for (const Ticket& t : collectValues<Ticket>(tickets_))
        if (t.status >= 0 && t.status < 4)
            r.ticketsByStatus[t.status]++;

    for (const Card& c : collectValues<Card>(cards_)) {
        r.totalCards++;
        if (c.status == C_BLOCKED)
            r.blockedCards++;
        if (c.tappedIn)
            r.cardsInside++;
        if (hasValidPass(c))
            r.activePasses++;
    }

    int n = stationCount();
    Array<StationCount> stations(n);
    for (int i = 0; i < n; i++)
        stations[i] = StationCount{i, 0};

    HashMap* routeCounts = newTable(sizeof(int));
    for (ListNode* node = trips_->head; node != nullptr; node = node->next) {
        const Trip* t = (const Trip*)node->data;
        r.totalTrips++;
        if (t->source >= 0 && t->source < n)
            stations[t->source].count++;
        if (t->destination >= 0 && t->destination < n)
            stations[t->destination].count++;
        if (routeCounts != nullptr) {
            int key = t->source * n + t->destination;
            int* count = (int*)(tableGet(routeCounts, key));
            int one = 1;
            if (count != nullptr)
                (*count)++;
            else
                tablePut(routeCounts, key, &one);
        }
    }

    sortRecords(stations, compareStationCount);
    for (const StationCount& s : stations)
        if (s.count > 0 && r.busiestStations.size() < 5)
            r.busiestStations.push_back(s);

    if (routeCounts != nullptr && n > 0) {
        Array<RouteCount> routes;
        HmIter it;
        tableIterStart(routeCounts, &it);
        int key = 0;
        void* value = nullptr;
        while (tableIterNext(&it, &key, &value))
            routes.push_back(RouteCount{key / n, key % n, *(int*)value});
        sortRecords(routes, compareRouteCount);
        for (const RouteCount& rc : routes)
            if (r.popularRoutes.size() < 5)
                r.popularRoutes.push_back(rc);
    }
    freeTable(routeCounts);
    return r;
}
