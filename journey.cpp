#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

#include <climits>
#include <cstring>

using namespace std;

#define GRAPH_INITIAL_CAPACITY 16

static bool valid_name(const char *name)
{
    size_t len;
    if (name == nullptr)
        return false;
    len = strlen(name);
    return len > 0 && len < GRAPH_NAME_MAX;
}

static bool same_ignoring_case(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
        a++;
        b++;
    }
    return *a == *b;
}

static bool valid_line(const Graph *g, int line)
{
    return g != nullptr && line >= 0 && line < g->line_count;
}

Graph *graph_create(void)
{
    Graph *g = (Graph *)calloc(1, sizeof(Graph));
    return g;
}

static void free_edges(EdgeNode *e)
{
    while (e != nullptr) {
        EdgeNode *next = e->next;
        free(e);
        e = next;
    }
}

void graph_destroy(Graph *g)
{
    int i;

    if (g == nullptr)
        return;

    for (i = 0; i < g->station_count; i++)
        free_edges(g->stations[i].adj);
    free(g->stations);
    free(g->lines);
    free(g);
}

int graph_add_line(Graph *g, const char *name)
{
    int i;

    if (g == nullptr || !valid_name(name))
        return -1;

    for (i = 0; i < g->line_count; i++)
        if (same_ignoring_case(g->lines[i].name, name))
            return -1;

    if (g->line_count == g->line_capacity) {
        int new_capacity = g->line_capacity == 0 ? 4 : g->line_capacity * 2;
        Line *bigger = (Line *)realloc(g->lines, (size_t)new_capacity * sizeof(Line));
        if (bigger == nullptr)
            return -1;
        g->lines = bigger;
        g->line_capacity = new_capacity;
    }

    strcpy(g->lines[g->line_count].name, name);
    return g->line_count++;
}

int graph_line_count(const Graph *g)
{
    return g == nullptr ? 0 : g->line_count;
}

const char *graph_line_name(const Graph *g, int line)
{
    return valid_line(g, line) ? g->lines[line].name : "Unknown";
}

int graph_add_station(Graph *g, const char *name)
{
    Station *s;

    if (g == nullptr || !valid_name(name) || graph_find_station(g, name) != -1)
        return -1;

    if (g->station_count == g->station_capacity) {
        int new_capacity = g->station_capacity == 0 ? GRAPH_INITIAL_CAPACITY
                                                    : g->station_capacity * 2;
        Station *bigger = (Station *)realloc(g->stations,
                                             (size_t)new_capacity * sizeof(Station));
        if (bigger == nullptr)
            return -1;
        g->stations = bigger;
        g->station_capacity = new_capacity;
    }

    s = &g->stations[g->station_count];
    strcpy(s->name, name);
    s->open = true;
    s->adj = nullptr;
    return g->station_count++;
}

bool graph_remove_last_station(Graph *g)
{
    if (g == nullptr || g->station_count == 0)
        return false;

    if (g->stations[g->station_count - 1].adj != nullptr)
        return false;

    g->station_count--;
    return true;
}

int graph_station_count(const Graph *g)
{
    return g == nullptr ? 0 : g->station_count;
}

bool graph_valid_station(const Graph *g, int id)
{
    return g != nullptr && id >= 0 && id < g->station_count;
}

const char *graph_station_name(const Graph *g, int id)
{
    return graph_valid_station(g, id) ? g->stations[id].name : "Unknown";
}

bool graph_is_open(const Graph *g, int id)
{
    return graph_valid_station(g, id) && g->stations[id].open;
}

bool graph_set_open(Graph *g, int id, bool open)
{
    if (!graph_valid_station(g, id))
        return false;
    g->stations[id].open = open;
    return true;
}

int graph_find_station(const Graph *g, const char *name)
{
    int i;

    if (g == nullptr || name == nullptr)
        return -1;

    for (i = 0; i < g->station_count; i++)
        if (same_ignoring_case(g->stations[i].name, name))
            return i;
    return -1;
}

int graph_station_lines(const Graph *g, int id, int *out, int max_out)
{
    const EdgeNode *e;
    int count = 0;
    int i, j;

    if (!graph_valid_station(g, id) || out == nullptr || max_out <= 0)
        return 0;

    for (e = g->stations[id].adj; e != nullptr; e = e->next) {
        bool seen = false;
        for (i = 0; i < count; i++)
            if (out[i] == e->line)
                seen = true;
        if (!seen && count < max_out)
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

bool graph_has_edge(const Graph *g, int a, int b, int line)
{
    const EdgeNode *e;

    if (!graph_valid_station(g, a))
        return false;
    for (e = g->stations[a].adj; e != nullptr; e = e->next)
        if (e->to == b && e->line == line)
            return true;
    return false;
}

static bool push_edge(Graph *g, int from, int to, int dist_m, int line)
{
    EdgeNode *e = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (e == nullptr)
        return false;
    e->to = to;
    e->dist_m = dist_m;
    e->line = line;
    e->next = g->stations[from].adj;
    g->stations[from].adj = e;
    return true;
}

static bool unlink_edge(Graph *g, int from, int to, int line)
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

bool graph_add_edge(Graph *g, int a, int b, int dist_m, int line)
{
    if (!graph_valid_station(g, a) || !graph_valid_station(g, b) || a == b)
        return false;
    if (!valid_line(g, line) || dist_m <= 0 || graph_has_edge(g, a, b, line))
        return false;

    if (!push_edge(g, a, b, dist_m, line))
        return false;
    if (!push_edge(g, b, a, dist_m, line)) {
        unlink_edge(g, a, b, line);
        return false;
    }
    return true;
}

bool graph_remove_edge(Graph *g, int a, int b, int line)
{
    if (!graph_valid_station(g, a) || !graph_valid_station(g, b))
        return false;
    if (!graph_has_edge(g, a, b, line))
        return false;
    unlink_edge(g, a, b, line);
    unlink_edge(g, b, a, line);
    return true;
}

const EdgeNode *graph_edges(const Graph *g, int id)
{
    return graph_valid_station(g, id) ? g->stations[id].adj : nullptr;
}

void route_init(Route *r)
{
    if (r == nullptr)
        return;
    r->length = 0;
    r->stations = nullptr;
    r->lines = nullptr;
    r->total_m = 0;
}

void route_free(Route *r)
{
    if (r == nullptr)
        return;
    free(r->stations);
    free(r->lines);
    route_init(r);
}

static bool build_route(int src, int dst, const int *prev, const int *prev_line,
                        const int *prev_dist, Route *out)
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
        route_free(out);
        return false;
    }

    out->length = len;
    out->total_m = 0;
    cur = dst;
    for (i = len - 1; i >= 0; i--) {
        out->stations[i] = cur;
        if (i > 0) {
            out->lines[i - 1] = prev_line[cur];
            out->total_m += prev_dist[cur];
            cur = prev[cur];
        }
    }
    return true;
}

struct SearchArrays {
    int *prev;
    int *prev_line;
    int *prev_dist;
};

static bool alloc_search(SearchArrays *s, int n)
{
    int i;
    s->prev = (int *)malloc((size_t)n * sizeof(int));
    s->prev_line = (int *)malloc((size_t)n * sizeof(int));
    s->prev_dist = (int *)malloc((size_t)n * sizeof(int));
    if (s->prev == nullptr || s->prev_line == nullptr || s->prev_dist == nullptr)
        return false;
    for (i = 0; i < n; i++) {
        s->prev[i] = -1;
        s->prev_line[i] = -1;
        s->prev_dist[i] = 0;
    }
    return true;
}

static void free_search(SearchArrays *s)
{
    free(s->prev);
    free(s->prev_line);
    free(s->prev_dist);
}

static bool can_start(const Graph *g, int src, int dst, bool allow_closed, Route *out)
{
    if (out == nullptr)
        return false;
    route_free(out);
    if (!graph_valid_station(g, src) || !graph_valid_station(g, dst))
        return false;
    if (!allow_closed && (!g->stations[src].open || !g->stations[dst].open))
        return false;
    return true;
}

bool graph_shortest_path(const Graph *g, int src, int dst, bool allow_closed, Route *out)
{
    int n, i;
    long long *dist = nullptr;
    SearchArrays s = {nullptr, nullptr, nullptr};
    Heap *heap = nullptr;
    HeapItem item;
    bool memory_ok;
    bool ok = false;

    if (!can_start(g, src, dst, allow_closed, out))
        return false;

    n = g->station_count;
    dist = (long long *)malloc((size_t)n * sizeof(long long));
    heap = heap_create();
    memory_ok = dist != nullptr && heap != nullptr && alloc_search(&s, n);

    if (memory_ok) {
        for (i = 0; i < n; i++)
            dist[i] = LLONG_MAX;
        dist[src] = 0;

        item.key1 = 0;
        item.key2 = 0;
        item.value = src;
        memory_ok = heap_push(heap, item);
    }

    while (memory_ok && heap_pop(heap, &item)) {
        int u = item.value;
        const EdgeNode *e;

        if (item.key1 > dist[u])
            continue;
        if (u == dst)
            break;

        for (e = g->stations[u].adj; e != nullptr && memory_ok; e = e->next) {
            int v = e->to;
            long long nd = dist[u] + e->dist_m;

            if (!allow_closed && !g->stations[v].open)
                continue;
            if (nd < dist[v]) {
                HeapItem next;
                dist[v] = nd;
                s.prev[v] = u;
                s.prev_line[v] = e->line;
                s.prev_dist[v] = e->dist_m;
                next.key1 = nd;
                next.key2 = 0;
                next.value = v;
                memory_ok = heap_push(heap, next);
            }
        }
    }

    if (memory_ok && dist[dst] != LLONG_MAX)
        ok = build_route(src, dst, s.prev, s.prev_line, s.prev_dist, out);

    free(dist);
    free_search(&s);
    heap_destroy(heap);
    return ok;
}

bool graph_fewest_stops(const Graph *g, int src, int dst, bool allow_closed, Route *out)
{
    int n, u;
    bool *visited = nullptr;
    SearchArrays s = {nullptr, nullptr, nullptr};
    Queue *q = nullptr;
    bool found = false;
    bool memory_ok;
    bool ok = false;

    if (!can_start(g, src, dst, allow_closed, out))
        return false;

    n = g->station_count;
    visited = (bool *)calloc((size_t)n, sizeof(bool));
    q = queue_create();
    memory_ok = visited != nullptr && q != nullptr && alloc_search(&s, n);

    if (memory_ok) {
        visited[src] = true;
        memory_ok = queue_enqueue(q, src);
    }

    while (memory_ok && !found && queue_dequeue(q, &u)) {
        const EdgeNode *e;
        for (e = g->stations[u].adj; e != nullptr; e = e->next) {
            int v = e->to;
            if (visited[v] || (!allow_closed && !g->stations[v].open))
                continue;
            visited[v] = true;
            s.prev[v] = u;
            s.prev_line[v] = e->line;
            s.prev_dist[v] = e->dist_m;
            if (v == dst) {
                found = true;
                break;
            }
            if (!queue_enqueue(q, v)) {
                memory_ok = false;
                break;
            }
        }
    }

    if (memory_ok && (src == dst || found))
        ok = build_route(src, dst, s.prev, s.prev_line, s.prev_dist, out);

    free(visited);
    free_search(&s);
    queue_destroy(q);
    return ok;
}

static bool usable(const Graph *g, int station, int blocked)
{
    return graph_is_open(g, station) && station != blocked;
}

static int visit(const Graph *g, int station, int blocked, bool *visited)
{
    const EdgeNode *e;
    int count = 1;

    visited[station] = true;
    for (e = graph_edges(g, station); e != nullptr; e = e->next)
        if (!visited[e->to] && usable(g, e->to, blocked))
            count += visit(g, e->to, blocked, visited);
    return count;
}

int dfs_count_reachable_recursive(const Graph *g, int start, int blocked, bool *visited)
{
    if (g == nullptr || visited == nullptr || !usable(g, start, blocked) || visited[start])
        return 0;
    return visit(g, start, blocked, visited);
}

static int explore(const Graph *g, int start, int blocked, bool *visited, int *component_of,
                   int component)
{
    Stack *stack = stack_create(sizeof(int));
    int station, count = 0;

    if (stack == nullptr)
        return -1;

    visited[start] = true;
    stack_push(stack, &start);
    while (stack_pop(stack, &station)) {
        const EdgeNode *e;
        count++;
        if (component_of != nullptr)
            component_of[station] = component;
        for (e = graph_edges(g, station); e != nullptr; e = e->next) {
            if (!visited[e->to] && usable(g, e->to, blocked)) {
                visited[e->to] = true;
                if (!stack_push(stack, &e->to)) {
                    stack_destroy(stack);
                    return -1;
                }
            }
        }
    }
    stack_destroy(stack);
    return count;
}

int dfs_count_reachable_iterative(const Graph *g, int start, int blocked, bool *visited)
{
    int count;
    if (g == nullptr || visited == nullptr || !usable(g, start, blocked) || visited[start])
        return 0;
    count = explore(g, start, blocked, visited, nullptr, 0);
    return count < 0 ? 0 : count;
}

int dfs_components(const Graph *g, int blocked, int *component_of)
{
    int n, i, groups = 0;
    bool *visited;

    if (g == nullptr || component_of == nullptr)
        return -1;

    n = graph_station_count(g);
    visited = (bool *)calloc(n > 0 ? (size_t)n : 1, sizeof(bool));
    if (visited == nullptr)
        return -1;

    for (i = 0; i < n; i++)
        component_of[i] = -1;

    for (i = 0; i < n; i++) {
        if (visited[i] || !usable(g, i, blocked))
            continue;
        if (explore(g, i, blocked, visited, component_of, groups) < 0) {
            free(visited);
            return -1;
        }
        groups++;
    }
    free(visited);
    return groups;
}

static bool to_lower_key(const char *text, char *out)
{
    size_t i, len;
    if (text == nullptr)
        return false;
    len = strlen(text);
    if (len >= GRAPH_NAME_MAX)
        return false;
    for (i = 0; i <= len; i++)
        out[i] = (char)tolower((unsigned char)text[i]);
    return true;
}

static int compare_entries(const void *a, const void *b)
{
    return strcmp(((const StationIndexEntry *)a)->key, ((const StationIndexEntry *)b)->key);
}

StationIndex *station_index_create(void)
{
    return (StationIndex *)calloc(1, sizeof(StationIndex));
}

bool station_index_build(StationIndex *index, const Graph *g)
{
    int i, n;
    StationIndexEntry *entries;

    if (index == nullptr || g == nullptr)
        return false;

    n = graph_station_count(g);
    entries = n > 0 ? (StationIndexEntry *)malloc((size_t)n * sizeof(StationIndexEntry)) : nullptr;
    if (n > 0 && entries == nullptr)
        return false;

    for (i = 0; i < n; i++) {
        to_lower_key(graph_station_name(g, i), entries[i].key);
        entries[i].id = i;
    }
    if (!quick_sort(entries, (size_t)n, sizeof(StationIndexEntry), compare_entries)) {
        free(entries);
        return false;
    }

    free(index->entries);
    index->entries = entries;
    index->count = n;
    return true;
}

int station_index_find(const StationIndex *index, const char *name)
{
    char key[GRAPH_NAME_MAX];
    int lo = 0, hi;

    if (index == nullptr || !to_lower_key(name, key))
        return -1;

    hi = index->count - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int c = strcmp(key, index->entries[mid].key);
        if (c == 0)
            return index->entries[mid].id;
        if (c < 0)
            hi = mid - 1;
        else
            lo = mid + 1;
    }
    return -1;
}

static int find_in_range(const StationIndex *index, const char *key, int lo, int hi)
{
    int mid, c;

    if (lo > hi)
        return -1;
    mid = lo + (hi - lo) / 2;
    c = strcmp(key, index->entries[mid].key);
    if (c == 0)
        return index->entries[mid].id;
    if (c < 0)
        return find_in_range(index, key, lo, mid - 1);
    return find_in_range(index, key, mid + 1, hi);
}

int station_index_find_recursive(const StationIndex *index, const char *name)
{
    char key[GRAPH_NAME_MAX];
    if (index == nullptr || !to_lower_key(name, key))
        return -1;
    return find_in_range(index, key, 0, index->count - 1);
}

int station_index_prefix(const StationIndex *index, const char *prefix, int *out_ids,
                         int max_out)
{
    char key[GRAPH_NAME_MAX];
    size_t len;
    int lo = 0, hi, found = 0;

    if (index == nullptr || out_ids == nullptr || max_out <= 0 || !to_lower_key(prefix, key))
        return 0;
    len = strlen(key);
    if (len == 0)
        return 0;

    hi = index->count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (strcmp(index->entries[mid].key, key) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    while (lo < index->count && found < max_out &&
           strncmp(index->entries[lo].key, key, len) == 0) {
        out_ids[found++] = index->entries[lo].id;
        lo++;
    }
    return found;
}

void station_index_destroy(StationIndex *index)
{
    if (index == nullptr)
        return;
    free(index->entries);
    free(index);
}

DList *dlist_create(void)
{
    return (DList *)calloc(1, sizeof(DList));
}

static DNode *make_node(int value)
{
    DNode *node = (DNode *)malloc(sizeof(DNode));
    if (node != nullptr) {
        node->value = value;
        node->prev = nullptr;
        node->next = nullptr;
    }
    return node;
}

bool dlist_push_back(DList *list, int value)
{
    DNode *node;

    if (list == nullptr || (node = make_node(value)) == nullptr)
        return false;

    node->prev = list->tail;
    if (list->tail != nullptr)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
    list->size++;
    return true;
}

bool dlist_push_front(DList *list, int value)
{
    DNode *node;

    if (list == nullptr || (node = make_node(value)) == nullptr)
        return false;

    node->next = list->head;
    if (list->head != nullptr)
        list->head->prev = node;
    else
        list->tail = node;
    list->head = node;
    list->size++;
    return true;
}

bool dlist_remove(DList *list, int value)
{
    DNode *node;

    if (list == nullptr)
        return false;

    for (node = list->head; node != nullptr; node = node->next) {
        if (node->value != value)
            continue;
        if (node->prev != nullptr)
            node->prev->next = node->next;
        else
            list->head = node->next;
        if (node->next != nullptr)
            node->next->prev = node->prev;
        else
            list->tail = node->prev;
        free(node);
        list->size--;
        return true;
    }
    return false;
}

int dlist_index_of(const DList *list, int value)
{
    const DNode *node;
    int index = 0;

    if (list == nullptr)
        return -1;
    for (node = list->head; node != nullptr; node = node->next, index++)
        if (node->value == value)
            return index;
    return -1;
}

bool dlist_contains(const DList *list, int value)
{
    return dlist_index_of(list, value) >= 0;
}

size_t dlist_size(const DList *list)
{
    return list == nullptr ? 0 : list->size;
}

void dlist_clear(DList *list)
{
    DNode *node;

    if (list == nullptr)
        return;
    node = list->head;
    while (node != nullptr) {
        DNode *next = node->next;
        free(node);
        node = next;
    }
    list->head = nullptr;
    list->tail = nullptr;
    list->size = 0;
}

void dlist_destroy(DList *list)
{
    if (list == nullptr)
        return;
    dlist_clear(list);
    free(list);
}

Metro::Metro()
{
    graph_ = graph_create();
    students_ = hm_create(sizeof(int));
    womenCards_ = hm_create(sizeof(int));
    cards_ = hm_create(sizeof(Card));
    tickets_ = hm_create(sizeof(Ticket));
    waitEntries_ = hm_create(sizeof(WaitEntry));
    trips_ = list_create(sizeof(Trip));
    activity_ = deque_create(sizeof(Activity));
    undo_ = stack_create(sizeof(AdminAction));
    ticketIndex_ = bst_create();
    ticketAvl_ = avl_create();
    stationIndex_ = station_index_create();

    bool allOk = graph_ && students_ && womenCards_ && cards_ && tickets_ && waitEntries_ && trips_ &&
                 activity_ && undo_ && ticketIndex_ && ticketAvl_ && stationIndex_;
    for (int i = 0; i < MAX_LINES; i++) {
        waitlists_[i] = heap_create();
        allOk = allOk && waitlists_[i] != nullptr;
    }
    if (!allOk) {
        freeAll();
        throw bad_alloc();
    }
    adminHash_ = hashPassword("1234");
}

Metro::~Metro()
{
    freeAll();
}

void Metro::freeAll()
{
    graph_destroy(graph_);
    hm_destroy(students_);
    hm_destroy(womenCards_);
    hm_destroy(cards_);
    hm_destroy(tickets_);
    hm_destroy(waitEntries_);
    list_destroy(trips_);
    deque_destroy(activity_);
    stack_destroy(undo_);
    bst_destroy(ticketIndex_);
    avl_destroy(ticketAvl_);
    station_index_destroy(stationIndex_);
    for (int i = 0; i < MAX_LINES; i++) {
        heap_destroy(waitlists_[i]);
        waitlists_[i] = nullptr;
    }
    graph_ = nullptr;
    students_ = womenCards_ = cards_ = tickets_ = waitEntries_ = nullptr;
    trips_ = nullptr;
    activity_ = nullptr;
    undo_ = nullptr;
    ticketIndex_ = nullptr;
    ticketAvl_ = nullptr;
    stationIndex_ = nullptr;
}

void Metro::seedDefaultNetwork()
{
    addLine("Blue", 6, 5);
    addLine("Yellow", 6, 6);
    addLine("Red", 6, 8);

    const char* names[] = {"Airport",     "City Mall",  "Rajiv Chowk",      "ISBT",
                           "Railway Station", "Clock Tower", "DU",           "Civil Lines",
                           "City Park",   "Tech Park",  "Graphic Era",      "Hospital",
                           "Exhibition Ground", "Stadium"};
    for (const char* n : names)
        addStation(n);

    struct Track {
        const char* a;
        const char* b;
        int meters;
        int line;
    };
    const Track tracks[] = {
        {"Airport", "City Mall", 4200, 0},
        {"City Mall", "Rajiv Chowk", 2600, 0},
        {"Rajiv Chowk", "ISBT", 1800, 0},
        {"ISBT", "Railway Station", 1500, 0},
        {"Railway Station", "Clock Tower", 1200, 0},
        {"DU", "Civil Lines", 1600, 1},
        {"Civil Lines", "Rajiv Chowk", 1400, 1},
        {"Rajiv Chowk", "City Park", 3000, 1},
        {"City Park", "Tech Park", 2200, 1},
        {"Tech Park", "Graphic Era", 1900, 1},
        {"ISBT", "Hospital", 900, 2},
        {"Hospital", "Exhibition Ground", 800, 2},
        {"Exhibition Ground", "City Park", 700, 2},
        {"City Park", "Stadium", 2500, 2},
    };
    for (const Track& t : tracks)
        addConnection(findStation(t.a), findStation(t.b), t.meters, t.line);

    for (int id : {101, 102, 105, 110, 115})
        registerStudent(id);
    for (int id : {9001, 9002, 9003})
        registerWomenCard(id);

    int cardId = 0;
    createCard("Aarav Sharma", REGULAR, 0, 500, cardId);
    createCard("Priya Negi", STUDENT, 101, 300, cardId);

    finishSetup("System initialised with the built-in default network");
}

void Metro::finishSetup(const string& note)
{
    stack_clear(undo_);
    deque_clear(activity_);
    log(note);
}

bool Metro::validLine(int line) const
{
    return line >= 0 && line < graph_line_count(graph_);
}

int Metro::stationCount() const { return graph_station_count(graph_); }

int Metro::lineCount() const { return graph_line_count(graph_); }

string Metro::stationName(int id) const { return graph_station_name(graph_, id); }

string Metro::lineName(int line) const { return graph_line_name(graph_, line); }

int Metro::findLine(const string& name) const
{
    string wanted = trim(name);
    for (int i = 0; i < lineCount(); i++) {
        string have = lineName(i);
        if (have.size() != wanted.size())
            continue;
        bool same = true;
        for (size_t k = 0; k < have.size() && same; k++)
            same = tolower((unsigned char)(have[k])) ==
                   tolower((unsigned char)(wanted[k]));
        if (same)
            return i;
    }
    return -1;
}

bool Metro::isStationOpen(int id) const { return graph_is_open(graph_, id); }

int Metro::findStation(const string& idOrName) const
{
    string text = trim(idOrName);
    int id = -1;
    if (parseInt(text, id))
        return graph_valid_station(graph_, id) ? id : -1;
    return station_index_find(stationIndex_, text.c_str());
}

void Metro::rebuildStationIndex()
{
    station_index_build(stationIndex_, graph_);
}

Array<int> Metro::suggestStations(const string& prefix, int maxResults) const
{
    Array<int> out;
    string text = trim(prefix);
    if (text.empty() || maxResults <= 0)
        return out;
    out.resize(maxResults);
    int found = station_index_prefix(stationIndex_, text.c_str(), out.data(), maxResults);
    out.resize(found);
    return out;
}

static int biggestGroup(const Array<int>& group, int groups, int n)
{
    Array<int> size(groups, 0);
    for (int i = 0; i < n; i++)
        if (group[i] >= 0)
            size[group[i]]++;
    int best = 0;
    for (int g = 1; g < groups; g++)
        if (size[g] > size[best])
            best = g;
    return best;
}

Array<int> Metro::stationsCutOffIfClosed(int id) const
{
    Array<int> cutOff;
    int n = stationCount();
    if (!graph_valid_station(graph_, id) || !graph_is_open(graph_, id))
        return cutOff;

    Array<int> before(n), after(n);
    int groupsBefore = dfs_components(graph_, -1, before.data());
    int groupsAfter = dfs_components(graph_, id, after.data());
    if (groupsBefore < 0 || groupsAfter <= groupsBefore)
        return cutOff;

    int mainBefore = biggestGroup(before, groupsBefore, n);
    int mainAfter = biggestGroup(after, groupsAfter, n);
    for (int i = 0; i < n; i++)
        if (i != id && before[i] == mainBefore && after[i] != mainAfter)
            cutOff.push_back(i);
    return cutOff;
}

bool Metro::buildLineList(int line, LineList& out, bool& complete) const
{
    complete = true;
    if (!validLine(line) || out.list == nullptr)
        return false;
    dlist_clear(out.list);

    int n = stationCount();
    Array<int> degree(n, 0);
    int onLine = 0;
    for (int s = 0; s < n; s++) {
        for (const EdgeNode* e = graph_edges(graph_, s); e != nullptr; e = e->next)
            if (e->line == line)
                degree[s]++;
        if (degree[s] > 0)
            onLine++;
    }
    if (onLine == 0)
        return false;

    int start = -1;
    for (int s = 0; s < n && start < 0; s++)
        if (degree[s] == 1)
            start = s;
    for (int s = 0; s < n && start < 0; s++)
        if (degree[s] > 0)
            start = s;

    Array<bool> seen(n, false);
    int prev = -1, cur = start;
    while (cur >= 0) {
        dlist_push_back(out.list, cur);
        seen[cur] = true;
        int next = -1;
        for (const EdgeNode* e = graph_edges(graph_, cur); e != nullptr; e = e->next)
            if (e->line == line && e->to != prev && !seen[e->to] && (next < 0 || e->to < next))
                next = e->to;
        prev = cur;
        cur = next;
    }
    complete = (int)(dlist_size(out.list)) == onLine;
    return true;
}

int Metro::trackDistance(int a, int b, int line) const
{
    for (const EdgeNode* e = graph_edges(graph_, a); e != nullptr; e = e->next)
        if (e->to == b && e->line == line)
            return e->dist_m;
    return -1;
}

bool Metro::isLoopLine(int line) const
{
    if (!validLine(line))
        return false;
    int onLine = 0;
    for (int s = 0; s < stationCount(); s++) {
        int degree = 0;
        for (const EdgeNode* e = graph_edges(graph_, s); e != nullptr; e = e->next)
            if (e->line == line)
                degree++;
        if (degree == 0)
            continue;
        if (degree != 2)
            return false;
        onLine++;
    }
    return onLine >= 3;
}

string Metro::towardsStation(int line, int from, int to) const
{
    if (isLoopLine(line))
        return "";
    LineList ll;
    bool complete = true;
    if (!buildLineList(line, ll, complete))
        return "";
    int a = dlist_index_of(ll.list, from);
    int b = dlist_index_of(ll.list, to);
    if (a < 0 || b < 0 || a == b)
        return "";
    return stationName(b > a ? ll.list->tail->value : ll.list->head->value);
}

Array<int> Metro::linesAtStation(int id) const
{
    int buf[MAX_LINES];
    int n = graph_station_lines(graph_, id, buf, MAX_LINES);
    Array<int> out;
    for (int i = 0; i < n; i++)
        out.push_back(buf[i]);
    return out;
}

Result Metro::addStation(const string& name)
{
    string clean = trim(name);
    if (clean.empty() || clean.size() >= NAME_LEN)
        return fail("Station name must be 1 to " + to_string(NAME_LEN - 1) +
                    " characters.");
    int id = graph_add_station(graph_, clean.c_str());
    if (id < 0)
        return fail("A station called '" + clean + "' already exists.");
    rebuildStationIndex();
    pushUndo(ACT_ADD_STATION, id, 0, 0, 0, "Add station " + clean);
    log("Added station " + clean + " (ID " + to_string(id) + ")");
    return ok("Station '" + clean + "' added with ID " + to_string(id) +
              ". Connect it to the network with 'Add connection'.");
}

Result Metro::addLine(const string& name, int capacity, int frequencyMin)
{
    string clean = trim(name);
    if (graph_line_count(graph_) >= MAX_LINES)
        return fail("Maximum of " + to_string(MAX_LINES) + " lines reached.");
    if (capacity < 1 || capacity > MAX_TRAIN_CAPACITY)
        return fail("Capacity must be 1 to " + to_string(MAX_TRAIN_CAPACITY) + ".");
    if (frequencyMin < 1 || frequencyMin > 60)
        return fail("Frequency must be 1 to 60 minutes.");
    int id = graph_add_line(graph_, clean.c_str());
    if (id < 0)
        return fail("Invalid or duplicate line name.");
    lines_[id] = LineState{nextTrainId_++, capacity, 0, 0, frequencyMin};
    log("Added line " + clean);
    return ok(clean + " Line added with ID " + to_string(id) + ".");
}

Result Metro::addConnection(int a, int b, int distanceM, int line)
{
    if (!graph_valid_station(graph_, a) || !graph_valid_station(graph_, b))
        return fail("Invalid station.");
    if (a == b)
        return fail("A station cannot be connected to itself.");
    if (!validLine(line))
        return fail("Invalid line.");
    if (distanceM < 100 || distanceM > 50000)
        return fail("Distance must be between 100 and 50000 metres.");
    if (graph_has_edge(graph_, a, b, line))
        return fail("These stations are already connected on this line.");
    if (!graph_add_edge(graph_, a, b, distanceM, line))
        return fail("Could not add the connection (out of memory).");

    string text = stationName(a) + " <-> " + stationName(b) + " on " + lineName(line);
    pushUndo(ACT_ADD_CONNECTION, a, b, distanceM, line, "Add connection " + text);
    log("Added connection " + text);
    return ok("Connected " + text + " (" + to_string(distanceM) + " m).");
}

Result Metro::setStationOpen(int id, bool open)
{
    if (!graph_valid_station(graph_, id))
        return fail("Invalid station.");
    if (graph_is_open(graph_, id) == open)
        return fail(stationName(id) + " is already " + (open ? "open." : "closed."));
    graph_set_open(graph_, id, open);
    string verb = open ? "Reopened " : "Closed ";
    pushUndo(open ? ACT_OPEN_STATION : ACT_CLOSE_STATION, id, 0, 0, 0, verb + stationName(id));
    log(verb + "station " + stationName(id));
    return ok(stationName(id) + (open ? " is open again." :
                                        " is closed. Routes will now avoid it."));
}

bool Metro::planRoute(int src, int dst, bool fewestStops, RouteHolder& out,
                      string& error) const
{
    if (!graph_valid_station(graph_, src) || !graph_valid_station(graph_, dst)) {
        error = "Invalid station.";
        return false;
    }
    if (src == dst) {
        error = "Source and destination must be different.";
        return false;
    }
    if (!graph_is_open(graph_, src) || !graph_is_open(graph_, dst)) {
        int closed = graph_is_open(graph_, src) ? dst : src;
        error = stationName(closed) + " is closed for maintenance.";
        return false;
    }
    bool found = fewestStops ? graph_fewest_stops(graph_, src, dst, false, &out.route)
                             : graph_shortest_path(graph_, src, dst, false, &out.route);
    if (!found) {
        error = "No open route between " + stationName(src) + " and " + stationName(dst) + ".";
        return false;
    }
    return true;
}

bool Metro::fareForJourney(int src, int dst, int category, int& fare, int& line,
                           string& error) const
{
    RouteHolder rh;
    if (!planRoute(src, dst, false, rh, error))
        return false;
    fare = farePolicy(category).apply(baseFareForDistance(rh.route.total_m));
    line = rh.route.lines[0];
    return true;
}

const FarePolicy& farePolicy(int category)
{
    static const RegularFare regular;
    static const StudentFare student;
    static const SeniorFare senior;
    static const DisabledFare disabled;

    switch (category) {
    case STUDENT:
        return student;
    case SENIOR:
        return senior;
    case DISABLED:
        return disabled;
    default:
        return regular;
    }
}

int baseFareForDistance(long long meters)
{
    if (meters <= 2000)
        return 10;
    if (meters <= 5000)
        return 20;
    if (meters <= 12000)
        return 30;
    if (meters <= 21000)
        return 40;
    if (meters <= 32000)
        return 50;
    return 60;
}

bool validCategory(int category)
{
    return category >= REGULAR && category <= DISABLED;
}

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

SeedStatus loadSeedData(Metro& m, const string& folder, string& error, string& summary)
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
