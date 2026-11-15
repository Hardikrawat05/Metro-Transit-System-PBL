#include "metro.h"

#include <cstdlib>

using namespace std;

static bool canVisit(const Graph *g, int station, int blocked)
{
    return graphIsOpen(g, station) && station != blocked;
}

static int visit(const Graph *g, int station, int blocked, bool *visited)
{
    const EdgeNode *e;
    int count = 1;

    visited[station] = true;
    for (e = graphTracks(g, station); e != nullptr; e = e->next)
        if (!visited[e->to] && canVisit(g, e->to, blocked))
            count += visit(g, e->to, blocked, visited);
    return count;
}

int dfsCountRecursive(const Graph *g, int start, int blocked, bool *visited)
{
    if (g == nullptr || visited == nullptr || !canVisit(g, start, blocked) || visited[start])
        return 0;
    return visit(g, start, blocked, visited);
}

static int explore(const Graph *g, int start, int blocked, bool *visited, int *groupOf,
                   int component)
{
    Stack *stack = newStack(sizeof(int));
    int station, count = 0;

    if (stack == nullptr)
        return -1;

    visited[start] = true;
    stackPush(stack, &start);
    while (stackPop(stack, &station)) {
        const EdgeNode *e;
        count++;
        if (groupOf != nullptr)
            groupOf[station] = component;
        for (e = graphTracks(g, station); e != nullptr; e = e->next) {
            if (!visited[e->to] && canVisit(g, e->to, blocked)) {
                visited[e->to] = true;
                if (!stackPush(stack, &e->to)) {
                    freeStack(stack);
                    return -1;
                }
            }
        }
    }
    freeStack(stack);
    return count;
}

int dfsCountLoop(const Graph *g, int start, int blocked, bool *visited)
{
    int count;
    if (g == nullptr || visited == nullptr || !canVisit(g, start, blocked) || visited[start])
        return 0;
    count = explore(g, start, blocked, visited, nullptr, 0);
    return count < 0 ? 0 : count;
}

int dfsGroups(const Graph *g, int blocked, int *groupOf)
{
    int n, i, groups = 0;
    bool *visited;

    if (g == nullptr || groupOf == nullptr)
        return -1;

    n = graphStationCount(g);
    visited = (bool *)calloc(n > 0 ? (size_t)n : 1, sizeof(bool));
    if (visited == nullptr)
        return -1;

    for (i = 0; i < n; i++)
        groupOf[i] = -1;

    for (i = 0; i < n; i++) {
        if (visited[i] || !canVisit(g, i, blocked))
            continue;
        if (explore(g, i, blocked, visited, groupOf, groups) < 0) {
            free(visited);
            return -1;
        }
        groups++;
    }
    free(visited);
    return groups;
}
