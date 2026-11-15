#include "metro.h"

#include <cctype>
#include <climits>
#include <cstdlib>
#include <cstring>

using namespace std;

#define GRAPH_INITIAL_CAPACITY 16

static bool isValidName(const char *name)
{
    size_t len;
    if (name == nullptr)
        return false;
    len = strlen(name);
    return len > 0 && len < GRAPH_NAME_MAX;
}

static bool sameName(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
        a++;
        b++;
    }
    return *a == *b;
}

static bool isValidLine(const Graph *g, int line)
{
    return g != nullptr && line >= 0 && line < g->lineCount;
}

Graph *newGraph(void)
{
    Graph *g = (Graph *)calloc(1, sizeof(Graph));
    return g;
}

static void freeTracks(EdgeNode *e)
{
    while (e != nullptr) {
        EdgeNode *next = e->next;
        free(e);
        e = next;
    }
}

void freeGraph(Graph *g)
{
    int i;

    if (g == nullptr)
        return;

    for (i = 0; i < g->stationCount; i++)
        freeTracks(g->stations[i].adj);
    free(g->stations);
    free(g->lines);
    free(g);
}

int graphAddLine(Graph *g, const char *name)
{
    int i;

    if (g == nullptr || !isValidName(name))
        return -1;

    for (i = 0; i < g->lineCount; i++)
        if (sameName(g->lines[i].name, name))
            return -1;

    if (g->lineCount == g->lineCapacity) {
        int newCapacity = g->lineCapacity == 0 ? 4 : g->lineCapacity * 2;
        Line *bigger = (Line *)realloc(g->lines, (size_t)newCapacity * sizeof(Line));
        if (bigger == nullptr)
            return -1;
        g->lines = bigger;
        g->lineCapacity = newCapacity;
    }

    strcpy(g->lines[g->lineCount].name, name);
    return g->lineCount++;
}

int graphLineCount(const Graph *g)
{
    return g == nullptr ? 0 : g->lineCount;
}

const char *graphLineName(const Graph *g, int line)
{
    return isValidLine(g, line) ? g->lines[line].name : "Unknown";
}

int graphAddStation(Graph *g, const char *name)
{
    Station *s;

    if (g == nullptr || !isValidName(name) || graphFindStation(g, name) != -1)
        return -1;

    if (g->stationCount == g->stationCapacity) {
        int newCapacity = g->stationCapacity == 0 ? GRAPH_INITIAL_CAPACITY
                                                    : g->stationCapacity * 2;
        Station *bigger = (Station *)realloc(g->stations,
                                             (size_t)newCapacity * sizeof(Station));
        if (bigger == nullptr)
            return -1;
        g->stations = bigger;
        g->stationCapacity = newCapacity;
    }

    s = &g->stations[g->stationCount];
    strcpy(s->name, name);
    s->open = true;
    s->adj = nullptr;
    return g->stationCount++;
}

bool graphRemoveLastStation(Graph *g)
{
    if (g == nullptr || g->stationCount == 0)
        return false;

    if (g->stations[g->stationCount - 1].adj != nullptr)
        return false;

    g->stationCount--;
    return true;
}

int graphStationCount(const Graph *g)
{
    return g == nullptr ? 0 : g->stationCount;
}

bool graphHasStation(const Graph *g, int id)
{
    return g != nullptr && id >= 0 && id < g->stationCount;
}

const char *graphStationName(const Graph *g, int id)
{
    return graphHasStation(g, id) ? g->stations[id].name : "Unknown";
}

bool graphIsOpen(const Graph *g, int id)
{
    return graphHasStation(g, id) && g->stations[id].open;
}

bool graphSetOpen(Graph *g, int id, bool open)
{
    if (!graphHasStation(g, id))
        return false;
    g->stations[id].open = open;
    return true;
}

int graphFindStation(const Graph *g, const char *name)
{
    int i;

    if (g == nullptr || name == nullptr)
        return -1;

    for (i = 0; i < g->stationCount; i++)
        if (sameName(g->stations[i].name, name))
            return i;
    return -1;
}

int graphStationLines(const Graph *g, int id, int *out, int maxOut)
{
    const EdgeNode *e;
    int count = 0;
    int i, j;

    if (!graphHasStation(g, id) || out == nullptr || maxOut <= 0)
        return 0;

    for (e = g->stations[id].adj; e != nullptr; e = e->next) {
        bool seen = false;
        for (i = 0; i < count; i++)
            if (out[i] == e->line)
                seen = true;
        if (!seen && count < maxOut)
            out[count++] = e->line;
    }

    for (i = 1; i < count; i++) {
        int key = out[i];
        j = i - 1;
        while (j >= 0 && out[j] > key) {
            out[j + 1] = out[j];
            j--;
        }
        out[j + 1] = key;
    }
    return count;
}

bool graphHasTrack(const Graph *g, int a, int b, int line)
{
    const EdgeNode *e;

    if (!graphHasStation(g, a))
        return false;
    for (e = g->stations[a].adj; e != nullptr; e = e->next)
        if (e->to == b && e->line == line)
            return true;
    return false;
}

static bool addOneTrack(Graph *g, int from, int to, int meters, int line)
{
    EdgeNode *e = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (e == nullptr)
        return false;
    e->to = to;
    e->meters = meters;
    e->line = line;
    e->next = g->stations[from].adj;
    g->stations[from].adj = e;
    return true;
}

static bool removeOneTrack(Graph *g, int from, int to, int line)
{
    EdgeNode *prev = nullptr;
    EdgeNode *cur = g->stations[from].adj;
    while (cur != nullptr) {
        if (cur->to == to && cur->line == line) {
            if (prev == nullptr)
                g->stations[from].adj = cur->next;
            else
                prev->next = cur->next;
            free(cur);
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

bool graphAddTrack(Graph *g, int a, int b, int meters, int line)
{
    if (!graphHasStation(g, a) || !graphHasStation(g, b) || a == b)
        return false;
    if (!isValidLine(g, line) || meters <= 0 || graphHasTrack(g, a, b, line))
        return false;

    if (!addOneTrack(g, a, b, meters, line))
        return false;
    if (!addOneTrack(g, b, a, meters, line)) {
        removeOneTrack(g, a, b, line);
        return false;
    }
    return true;
}

bool graphRemoveTrack(Graph *g, int a, int b, int line)
{
    if (!graphHasStation(g, a) || !graphHasStation(g, b))
        return false;
    if (!graphHasTrack(g, a, b, line))
        return false;
    removeOneTrack(g, a, b, line);
    removeOneTrack(g, b, a, line);
    return true;
}

const EdgeNode *graphTracks(const Graph *g, int id)
{
    return graphHasStation(g, id) ? g->stations[id].adj : nullptr;
}

void initRoute(Route *r)
{
    if (r == nullptr)
        return;
    r->length = 0;
    r->stations = nullptr;
    r->lines = nullptr;
    r->totalMeters = 0;
}

void freeRoute(Route *r)
{
    if (r == nullptr)
        return;
    free(r->stations);
    free(r->lines);
    initRoute(r);
}

static bool makeRoute(int src, int dst, const int *prev, const int *prevLine,
                        const int *prevDist, Route *out)
{
    int len = 1;
    int cur = dst;
    int i;

    while (cur != src) {
        cur = prev[cur];
        if (cur < 0)
            return false;
        len++;
    }

    out->stations = (int *)malloc((size_t)len * sizeof(int));
    out->lines = len > 1 ? (int *)malloc((size_t)(len - 1) * sizeof(int)) : nullptr;
    if (out->stations == nullptr || (len > 1 && out->lines == nullptr)) {
        freeRoute(out);
        return false;
    }

    out->length = len;
    out->totalMeters = 0;
    cur = dst;
    for (i = len - 1; i >= 0; i--) {
        out->stations[i] = cur;
        if (i > 0) {
            out->lines[i - 1] = prevLine[cur];
            out->totalMeters += prevDist[cur];
            cur = prev[cur];
        }
    }
    return true;
}

struct SearchArrays {
    int *prev;
    int *prevLine;
    int *prevDist;
};

static bool allocSearch(SearchArrays *search, int n)
{
    search->prev = (int *)malloc((size_t)n * sizeof(int));
    search->prevLine = (int *)malloc((size_t)n * sizeof(int));
    search->prevDist = (int *)malloc((size_t)n * sizeof(int));
    if (search->prev == nullptr || search->prevLine == nullptr || search->prevDist == nullptr)
        return false;
    for (int i = 0; i < n; i++) {
        search->prev[i] = -1;
        search->prevLine[i] = -1;
        search->prevDist[i] = 0;
    }
    return true;
}

static void freeSearch(SearchArrays *search)
{
    free(search->prev);
    free(search->prevLine);
    free(search->prevDist);
}

static bool canStart(const Graph *g, int src, int dst, bool allowClosed, Route *out)
{
    if (out == nullptr)
        return false;
    freeRoute(out);
    if (!graphHasStation(g, src) || !graphHasStation(g, dst))
        return false;
    if (!allowClosed && (!g->stations[src].open || !g->stations[dst].open))
        return false;
    return true;
}

static void rememberStep(SearchArrays *search, int from, const EdgeNode *track)
{
    search->prev[track->to] = from;
    search->prevLine[track->to] = track->line;
    search->prevDist[track->to] = track->meters;
}

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
