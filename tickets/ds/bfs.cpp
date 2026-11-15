#include "metro.h"

#include <cstdlib>

using namespace std;

static bool runBfs(const Graph *g, int src, int dst, bool allowClosed, bool *visited,
                   SearchArrays *search, Queue *queue, bool *found)
{
    int u;

    visited[src] = true;
    if (!enqueue(queue, src))
        return false;

    while (!*found && dequeue(queue, &u)) {
        for (const EdgeNode *track = g->stations[u].adj; track != nullptr; track = track->next) {
            int v = track->to;
            if (visited[v] || (!allowClosed && !g->stations[v].open))
                continue;

            visited[v] = true;
            rememberStep(search, u, track);
            if (v == dst) {
                *found = true;
                break;
            }
            if (!enqueue(queue, v))
                return false;
        }
    }
    return true;
}

bool bfsRoute(const Graph *g, int src, int dst, bool allowClosed, Route *out)
{
    if (!canStart(g, src, dst, allowClosed, out))
        return false;

    int n = g->stationCount;
    bool *visited = (bool *)calloc((size_t)n, sizeof(bool));
    SearchArrays search = {nullptr, nullptr, nullptr};
    Queue *queue = newQueue();
    bool found = false;
    bool ok = false;

    if (visited != nullptr && queue != nullptr && allocSearch(&search, n) &&
        runBfs(g, src, dst, allowClosed, visited, &search, queue, &found) && (src == dst || found))
        ok = makeRoute(src, dst, search.prev, search.prevLine, search.prevDist, out);

    free(visited);
    freeSearch(&search);
    freeQueue(queue);
    return ok;
}
