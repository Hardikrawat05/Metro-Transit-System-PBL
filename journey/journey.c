#include "metro.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define GRAPH_INITIAL_CAPACITY 16

static bool valid_name(const char *name)
{
    size_t len;
    if (name == NULL)
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
    return g != NULL && line >= 0 && line < g->line_count;
}

Graph *graph_create(void)
{
    Graph *g = (Graph *)calloc(1, sizeof(Graph));
    return g;
}

static void free_edges(EdgeNode *e)
{
    while (e != NULL) {
        EdgeNode *next = e->next;
        free(e);
        e = next;
    }
}

void graph_destroy(Graph *g)
{
    int i;

    if (g == NULL)
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

    if (g == NULL || !valid_name(name))
        return -1;

    for (i = 0; i < g->line_count; i++)
        if (same_ignoring_case(g->lines[i].name, name))
            return -1;

    if (g->line_count == g->line_capacity) {
        int new_capacity = g->line_capacity == 0 ? 4 : g->line_capacity * 2;
        Line *bigger = (Line *)realloc(g->lines, (size_t)new_capacity * sizeof(Line));
        if (bigger == NULL)
            return -1;
        g->lines = bigger;
        g->line_capacity = new_capacity;
    }

    strcpy(g->lines[g->line_count].name, name);
    return g->line_count++;
}

int graph_line_count(const Graph *g)
{
    return g == NULL ? 0 : g->line_count;
}

const char *graph_line_name(const Graph *g, int line)
{
    return valid_line(g, line) ? g->lines[line].name : "Unknown";
}

int graph_add_station(Graph *g, const char *name)
{
    Station *s;

    if (g == NULL || !valid_name(name) || graph_find_station(g, name) != -1)
        return -1;

    if (g->station_count == g->station_capacity) {
        int new_capacity = g->station_capacity == 0 ? GRAPH_INITIAL_CAPACITY
                                                    : g->station_capacity * 2;
        Station *bigger = (Station *)realloc(g->stations,
                                             (size_t)new_capacity * sizeof(Station));
        if (bigger == NULL)
            return -1;
        g->stations = bigger;
        g->station_capacity = new_capacity;
    }

    s = &g->stations[g->station_count];
    strcpy(s->name, name);
    s->open = true;
    s->adj = NULL;
    return g->station_count++;
}

bool graph_remove_last_station(Graph *g)
{
    if (g == NULL || g->station_count == 0)
        return false;

    if (g->stations[g->station_count - 1].adj != NULL)
        return false;

    g->station_count--;
    return true;
}

int graph_station_count(const Graph *g)
{
    return g == NULL ? 0 : g->station_count;
}

bool graph_valid_station(const Graph *g, int id)
{
    return g != NULL && id >= 0 && id < g->station_count;
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

    if (g == NULL || name == NULL)
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

    if (!graph_valid_station(g, id) || out == NULL || max_out <= 0)
        return 0;

    for (e = g->stations[id].adj; e != NULL; e = e->next) {
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
    for (e = g->stations[a].adj; e != NULL; e = e->next)
        if (e->to == b && e->line == line)
            return true;
    return false;
}

static bool push_edge(Graph *g, int from, int to, int dist_m, int line)
{
    EdgeNode *e = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (e == NULL)
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
    EdgeNode *prev = NULL;
    EdgeNode *cur = g->stations[from].adj;
    while (cur != NULL) {
        if (cur->to == to && cur->line == line) {
            if (prev == NULL)
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
    return graph_valid_station(g, id) ? g->stations[id].adj : NULL;
}

void route_init(Route *r)
{
    if (r == NULL)
        return;
    r->length = 0;
    r->stations = NULL;
    r->lines = NULL;
    r->total_m = 0;
}

void route_free(Route *r)
{
    if (r == NULL)
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
    out->lines = len > 1 ? (int *)malloc((size_t)(len - 1) * sizeof(int)) : NULL;
    if (out->stations == NULL || (len > 1 && out->lines == NULL)) {
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

typedef struct SearchArrays {
    int *prev;
    int *prev_line;
    int *prev_dist;
} SearchArrays;

static bool alloc_search(SearchArrays *s, int n)
{
    int i;
    s->prev = (int *)malloc((size_t)n * sizeof(int));
    s->prev_line = (int *)malloc((size_t)n * sizeof(int));
    s->prev_dist = (int *)malloc((size_t)n * sizeof(int));
    if (s->prev == NULL || s->prev_line == NULL || s->prev_dist == NULL)
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
    if (out == NULL)
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
    long long *dist = NULL;
    SearchArrays s = {NULL, NULL, NULL};
    Heap *heap = NULL;
    HeapItem item;
    bool memory_ok;
    bool ok = false;

    if (!can_start(g, src, dst, allow_closed, out))
        return false;

    n = g->station_count;
    dist = (long long *)malloc((size_t)n * sizeof(long long));
    heap = heap_create();
    memory_ok = dist != NULL && heap != NULL && alloc_search(&s, n);

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

        for (e = g->stations[u].adj; e != NULL && memory_ok; e = e->next) {
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
    bool *visited = NULL;
    SearchArrays s = {NULL, NULL, NULL};
    Queue *q = NULL;
    bool found = false;
    bool memory_ok;
    bool ok = false;

    if (!can_start(g, src, dst, allow_closed, out))
        return false;

    n = g->station_count;
    visited = (bool *)calloc((size_t)n, sizeof(bool));
    q = queue_create();
    memory_ok = visited != NULL && q != NULL && alloc_search(&s, n);

    if (memory_ok) {
        visited[src] = true;
        memory_ok = queue_enqueue(q, src);
    }

    while (memory_ok && !found && queue_dequeue(q, &u)) {
        const EdgeNode *e;
        for (e = g->stations[u].adj; e != NULL; e = e->next) {
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
    for (e = graph_edges(g, station); e != NULL; e = e->next)
        if (!visited[e->to] && usable(g, e->to, blocked))
            count += visit(g, e->to, blocked, visited);
    return count;
}

int dfs_count_reachable_recursive(const Graph *g, int start, int blocked, bool *visited)
{
    if (g == NULL || visited == NULL || !usable(g, start, blocked) || visited[start])
        return 0;
    return visit(g, start, blocked, visited);
}

static int explore(const Graph *g, int start, int blocked, bool *visited, int *component_of,
                   int component)
{
    Stack *stack = stack_create(sizeof(int));
    int station, count = 0;

    if (stack == NULL)
        return -1;

    visited[start] = true;
    stack_push(stack, &start);
    while (stack_pop(stack, &station)) {
        const EdgeNode *e;
        count++;
        if (component_of != NULL)
            component_of[station] = component;
        for (e = graph_edges(g, station); e != NULL; e = e->next) {
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
    if (g == NULL || visited == NULL || !usable(g, start, blocked) || visited[start])
        return 0;
    count = explore(g, start, blocked, visited, NULL, 0);
    return count < 0 ? 0 : count;
}

int dfs_components(const Graph *g, int blocked, int *component_of)
{
    int n, i, groups = 0;
    bool *visited;

    if (g == NULL || component_of == NULL)
        return -1;

    n = graph_station_count(g);
    visited = (bool *)calloc(n > 0 ? (size_t)n : 1, sizeof(bool));
    if (visited == NULL)
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
    if (text == NULL)
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

    if (index == NULL || g == NULL)
        return false;

    n = graph_station_count(g);
    entries = n > 0 ? (StationIndexEntry *)malloc((size_t)n * sizeof(StationIndexEntry)) : NULL;
    if (n > 0 && entries == NULL)
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

    if (index == NULL || !to_lower_key(name, key))
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
    if (index == NULL || !to_lower_key(name, key))
        return -1;
    return find_in_range(index, key, 0, index->count - 1);
}

int station_index_prefix(const StationIndex *index, const char *prefix, int *out_ids,
                         int max_out)
{
    char key[GRAPH_NAME_MAX];
    size_t len;
    int lo = 0, hi, found = 0;

    if (index == NULL || out_ids == NULL || max_out <= 0 || !to_lower_key(prefix, key))
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
    if (index == NULL)
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
    if (node != NULL) {
        node->value = value;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

bool dlist_push_back(DList *list, int value)
{
    DNode *node;

    if (list == NULL || (node = make_node(value)) == NULL)
        return false;

    node->prev = list->tail;
    if (list->tail != NULL)
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

    if (list == NULL || (node = make_node(value)) == NULL)
        return false;

    node->next = list->head;
    if (list->head != NULL)
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

    if (list == NULL)
        return false;

    for (node = list->head; node != NULL; node = node->next) {
        if (node->value != value)
            continue;
        if (node->prev != NULL)
            node->prev->next = node->next;
        else
            list->head = node->next;
        if (node->next != NULL)
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

    if (list == NULL)
        return -1;
    for (node = list->head; node != NULL; node = node->next, index++)
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
    return list == NULL ? 0 : list->size;
}

void dlist_clear(DList *list)
{
    DNode *node;

    if (list == NULL)
        return;
    node = list->head;
    while (node != NULL) {
        DNode *next = node->next;
        free(node);
        node = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void dlist_destroy(DList *list)
{
    if (list == NULL)
        return;
    dlist_clear(list);
    free(list);
}
