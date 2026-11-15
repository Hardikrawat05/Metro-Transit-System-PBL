#include "metro.h"

#include <climits>
#include <cstdlib>

using namespace std;

static bool pushStation(Heap *heap, long long distance, int station)
{
    HeapItem item;
    item.key1 = distance;
    item.key2 = 0;
    item.value = station;
    return heapPush(heap, item);
}

static bool runDijkstra(const Graph *g, int src, int dst, bool allowClosed, long long *dist,
                        SearchArrays *search, Heap *heap)
{
    HeapItem item;

    for (int i = 0; i < g->stationCount; i++)
        dist[i] = LLONG_MAX;
    dist[src] = 0;
    if (!pushStation(heap, 0, src))
        return false;

    while (heapPop(heap, &item)) {
        int u = item.value;
        if (item.key1 > dist[u])
            continue;
        if (u == dst)
            break;

        for (const EdgeNode *track = g->stations[u].adj; track != nullptr; track = track->next) {
            int v = track->to;
            if (!allowClosed && !g->stations[v].open)
                continue;

            long long newDist = dist[u] + track->meters;
            if (newDist < dist[v]) {
                dist[v] = newDist;
                rememberStep(search, u, track);
                if (!pushStation(heap, newDist, v))
                    return false;
            }
        }
    }
    return true;
}

bool dijkstraRoute(const Graph *g, int src, int dst, bool allowClosed, Route *out)
{
    if (!canStart(g, src, dst, allowClosed, out))
        return false;

    int n = g->stationCount;
    long long *dist = (long long *)malloc((size_t)n * sizeof(long long));
    SearchArrays search = {nullptr, nullptr, nullptr};
    Heap *heap = newHeap();
    bool ok = false;

    if (dist != nullptr && heap != nullptr && allocSearch(&search, n) &&
        runDijkstra(g, src, dst, allowClosed, dist, &search, heap) && dist[dst] != LLONG_MAX)
        ok = makeRoute(src, dst, search.prev, search.prevLine, search.prevDist, out);

    free(dist);
    freeSearch(&search);
    freeHeap(heap);
    return ok;
}
