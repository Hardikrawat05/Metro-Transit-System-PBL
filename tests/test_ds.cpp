#include <cstdio>
#include <cstring>

#include "metro.h"
#include <cstdlib>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        checks++;                                                              \
        if (!(cond)) {                                                         \
            failures++;                                                        \
            printf("  FAILED: %s (line %d)\n", #cond, __LINE__);              \
        }                                                                      \
    } while (0)

static void test_list(void)
{
    List *list = newList(sizeof(int));
    int i, out = 0;

    printf("list...\n");
    for (i = 1; i <= 5; i++)
        CHECK(listAddBack(list, &i));
    i = 0;
    CHECK(listAddFront(list, &i));
    CHECK(listSize(list) == 6);
    CHECK(*(int *)list->head->data == 0);
    CHECK(*(int *)list->tail->data == 5);

    listTrim(list, 3);
    CHECK(listSize(list) == 3);
    CHECK(*(int *)list->tail->data == 2);

    CHECK(listRemoveFront(list, &out) && out == 0);
    CHECK(listRemoveFront(list, &out) && out == 1);
    CHECK(listRemoveFront(list, &out) && out == 2);
    CHECK(!listRemoveFront(list, &out));
    CHECK(list->tail == nullptr);
    freeList(list);
}

static void test_queue(void)
{
    Queue *q = newQueue();
    int i, out = 0;

    printf("queue...\n");
    CHECK(queueIsEmpty(q));
    for (i = 10; i < 15; i++)
        enqueue(q, i);
    CHECK(queueSize(q) == 5);
    for (i = 10; i < 15; i++)
        CHECK(dequeue(q, &out) && out == i);
    CHECK(!dequeue(q, &out));
    CHECK(queueIsEmpty(q));
    enqueue(q, 99);
    CHECK(queuePeek(q, &out) && out == 99);
    freeQueue(q);
}

static void test_stack(void)
{
    Stack *s = newStack(sizeof(int));
    int i, out = 0;

    printf("stack...\n");
    for (i = 0; i < 1000; i++)
        CHECK(stackPush(s, &i));
    CHECK(stackSize(s) == 1000);
    CHECK(*(const int *)stackFromTop(s, 0) == 999);
    CHECK(*(const int *)stackFromTop(s, 999) == 0);
    CHECK(stackFromTop(s, 1000) == nullptr);
    for (i = 999; i >= 0; i--)
        CHECK(stackPop(s, &out) && out == i);
    CHECK(stackIsEmpty(s));
    CHECK(!stackPop(s, &out));
    freeStack(s);
}

static void test_heap(void)
{
    Heap *h = newHeap();
    Heap *copy;
    HeapItem item;
    int expected[] = {4, 1, 2, 3, 5};
    int i;

    printf("heap...\n");
    item.key1 = 1; item.key2 = 1; item.value = 1; heapPush(h, item);
    item.key1 = 1; item.key2 = 2; item.value = 2; heapPush(h, item);
    item.key1 = 1; item.key2 = 3; item.value = 3; heapPush(h, item);
    item.key1 = 0; item.key2 = 4; item.value = 4; heapPush(h, item);
    item.key1 = 1; item.key2 = 5; item.value = 5; heapPush(h, item);

    copy = heapCopy(h);
    CHECK(heapSize(copy) == 5);
    for (i = 0; i < 5; i++)
        CHECK(heapPop(h, &item) && item.value == expected[i]);
    CHECK(!heapPop(h, &item));
    CHECK(heapSize(copy) == 5);

    for (i = 0; i < 500; i++) {
        item.key1 = (i * 37) % 101;
        item.key2 = i;
        item.value = i;
        heapPush(h, item);
    }
    {
        long long last1 = -1, last2 = -1;
        int sorted = 1;
        while (heapPop(h, &item)) {
            if (item.key1 < last1 || (item.key1 == last1 && item.key2 < last2))
                sorted = 0;
            last1 = item.key1;
            last2 = item.key2;
        }
        CHECK(sorted);
    }
    freeHeap(copy);
    freeHeap(h);
}

static void test_hashmap(void)
{
    HashMap *map = newTable(sizeof(int));
    HmIter it;
    int i, key, value, count = 0;
    long long sum = 0;
    void *v;
    int *stable;

    printf("hashmap...\n");
    value = 7;
    stable = (int *)tablePut(map, 42, &value);
    for (i = 0; i < 10000; i++) {
        value = i * 2;
        if (i != 42)
            CHECK(tablePut(map, i, &value) != nullptr);
    }
    CHECK(tableSize(map) == 10000);
    CHECK(*stable == 7);
    CHECK(*(int *)tableGet(map, 9999) == 19998);
    CHECK(tableGet(map, 10000) == nullptr);

    value = 1;
    tablePut(map, 5, &value);
    CHECK(*(int *)tableGet(map, 5) == 1);
    CHECK(tableSize(map) == 10000);

    CHECK(tableRemove(map, 5));
    CHECK(!tableRemove(map, 5));
    CHECK(!tableHas(map, 5));
    CHECK(tableSize(map) == 9999);

    tableIterStart(map, &it);
    while (tableIterNext(&it, &key, &v)) {
        count++;
        sum += key;
    }
    CHECK(count == 9999);
    CHECK(sum == (9999LL * 10000 / 2) - 5);

    value = -3;
    tablePut(map, -100, &value);
    CHECK(*(int *)tableGet(map, -100) == -3);
    freeTable(map);
}

typedef struct {
    int key;
    int order;
} Pair;

static int cmp_pair(const void *a, const void *b)
{
    return ((const Pair *)a)->key - ((const Pair *)b)->key;
}

static void test_sort(void)
{
    Pair items[8] = {{3, 0}, {1, 1}, {3, 2}, {2, 3}, {1, 4}, {5, 5}, {2, 6}, {3, 7}};
    int i, ok = 1, stable = 1;

    printf("merge sort...\n");
    CHECK(mergeSort(items, 8, sizeof(Pair), cmp_pair));
    for (i = 1; i < 8; i++) {
        if (items[i - 1].key > items[i].key)
            ok = 0;
        if (items[i - 1].key == items[i].key && items[i - 1].order > items[i].order)
            stable = 0;
    }
    CHECK(ok);
    CHECK(stable);
}

static void test_graph(void)
{
    Graph *g = newGraph();
    Route r;
    int a, b, c, d, e, lines[4];

    printf("graph (Dijkstra and BFS)...\n");
    CHECK(graphAddLine(g, "Blue") == 0);
    CHECK(graphAddLine(g, "Red") == 1);
    CHECK(graphAddLine(g, "blue") == -1);

    a = graphAddStation(g, "A");
    b = graphAddStation(g, "B");
    c = graphAddStation(g, "C");
    d = graphAddStation(g, "D");
    CHECK(graphAddStation(g, "a") == -1);
    CHECK(graphAddStation(g, "") == -1);
    CHECK(graphFindStation(g, "c") == c);

    CHECK(graphAddTrack(g, a, b, 1000, 0));
    CHECK(graphAddTrack(g, b, c, 1000, 0));
    CHECK(graphAddTrack(g, c, d, 1000, 0));
    CHECK(graphAddTrack(g, a, d, 5000, 1));
    CHECK(!graphAddTrack(g, a, d, 5000, 1));
    CHECK(!graphAddTrack(g, a, a, 100, 0));
    CHECK(graphStationLines(g, a, lines, 4) == 2);

    initRoute(&r);
    CHECK(dijkstraRoute(g, a, d, false, &r));
    CHECK(r.length == 4 && r.totalMeters == 3000);
    CHECK(r.stations[0] == a && r.stations[3] == d);
    CHECK(r.lines[0] == 0);

    CHECK(bfsRoute(g, a, d, false, &r));
    CHECK(r.length == 2 && r.totalMeters == 5000);
    CHECK(r.lines[0] == 1);

    graphSetOpen(g, c, false);
    CHECK(dijkstraRoute(g, a, d, false, &r));
    CHECK(r.length == 2 && r.totalMeters == 5000);
    CHECK(dijkstraRoute(g, a, d, true, &r));
    CHECK(r.totalMeters == 3000);
    CHECK(!dijkstraRoute(g, a, c, false, &r));
    graphSetOpen(g, c, true);

    e = graphAddStation(g, "E");
    CHECK(!dijkstraRoute(g, a, e, false, &r));
    CHECK(!bfsRoute(g, a, e, false, &r));

    CHECK(graphAddTrack(g, d, e, 800, 1));
    CHECK(!graphRemoveLastStation(g));
    CHECK(graphRemoveTrack(g, d, e, 1));
    CHECK(!graphHasTrack(g, e, d, 1));
    CHECK(graphRemoveLastStation(g));
    CHECK(graphStationCount(g) == 4);

    freeRoute(&r);
    freeGraph(g);
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static void test_quick_sort(void)
{
    int sizes[] = {0, 1, 2, 5, 11, 100, 5000};
    int s, i;

    printf("quick sort...\n");
    for (s = 0; s < 7; s++) {
        int n = sizes[s];
        int *random_data = (int *)malloc(sizeof(int) * (n > 0 ? n : 1));
        int *sorted_input = (int *)malloc(sizeof(int) * (n > 0 ? n : 1));
        int *reversed = (int *)malloc(sizeof(int) * (n > 0 ? n : 1));
        int *dups = (int *)malloc(sizeof(int) * (n > 0 ? n : 1));
        int ok = 1;
        unsigned seed = 12345u;
        for (i = 0; i < n; i++) {
            seed = seed * 1103515245u + 12345u;
            random_data[i] = (int)(seed % 1000u) - 500;
            sorted_input[i] = i;
            reversed[i] = n - i;
            dups[i] = i % 3;
        }
        CHECK(quickSort(random_data, (size_t)n, sizeof(int), cmp_int));
        CHECK(quickSort(sorted_input, (size_t)n, sizeof(int), cmp_int));
        CHECK(quickSort(reversed, (size_t)n, sizeof(int), cmp_int));
        CHECK(quickSort(dups, (size_t)n, sizeof(int), cmp_int));
        for (i = 1; i < n; i++) {
            if (random_data[i - 1] > random_data[i] || sorted_input[i - 1] > sorted_input[i] ||
                reversed[i - 1] > reversed[i] || dups[i - 1] > dups[i])
                ok = 0;
        }
        CHECK(ok);
        free(random_data);
        free(sorted_input);
        free(reversed);
        free(dups);
    }
}

static Graph *small_network(void)
{
    Graph *g = newGraph();
    graphAddLine(g, "Main");
    graphAddLine(g, "Branch");
    graphAddStation(g, "Alpha");
    graphAddStation(g, "Bravo");
    graphAddStation(g, "Charlie");
    graphAddStation(g, "Delta");
    graphAddStation(g, "Echo");
    graphAddTrack(g, 0, 1, 1000, 0);
    graphAddTrack(g, 1, 2, 1000, 0);
    graphAddTrack(g, 2, 3, 1000, 0);
    graphAddTrack(g, 1, 4, 1000, 1);
    return g;
}

static void test_dfs(void)
{
    Graph *g = small_network();
    bool visited[5];
    int comp[5];

    printf("DFS (recursive and iterative)...\n");
    memset(visited, 0, sizeof visited);
    CHECK(dfsCountRecursive(g, 0, -1, visited) == 5);
    memset(visited, 0, sizeof visited);
    CHECK(dfsCountLoop(g, 0, -1, visited) == 5);

    memset(visited, 0, sizeof visited);
    CHECK(dfsCountRecursive(g, 2, 1, visited) == 2);
    memset(visited, 0, sizeof visited);
    CHECK(dfsCountLoop(g, 2, 1, visited) == 2);
    CHECK(dfsGroups(g, -1, comp) == 1);
    CHECK(dfsGroups(g, 1, comp) == 3);
    CHECK(comp[1] == -1);
    CHECK(comp[2] == comp[3] && comp[0] != comp[2] && comp[4] != comp[2]);

    graphSetOpen(g, 3, false);
    CHECK(dfsGroups(g, -1, comp) == 1 && comp[3] == -1);
    memset(visited, 0, sizeof visited);
    CHECK(dfsCountLoop(g, 3, -1, visited) == 0);
    freeGraph(g);
}

static void test_bst(void)
{
    Bst *t = newBst();
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    int out[16];
    int i;

    printf("BST...\n");
    for (i = 0; i < 7; i++)
        CHECK(bstInsert(t, keys[i]));
    CHECK(!bstInsert(t, 40));
    CHECK(bstSize(t) == 7);
    CHECK(bstHeight(t) == 3);
    CHECK(bstContains(t, 60) && !bstContains(t, 65));
    CHECK(bstContainsRecursive(t, 20) && !bstContainsRecursive(t, 25));

    CHECK(bstInorder(t, out, 16) == 7);
    CHECK(out[0] == 20 && out[3] == 50 && out[6] == 80);
    CHECK(bstRange(t, 35, 65, out, 16) == 3);
    CHECK(out[0] == 40 && out[1] == 50 && out[2] == 60);
    CHECK(bstRange(t, 90, 99, out, 16) == 0);
    CHECK(bstInorder(t, out, 2) == 2 && out[1] == 30);
    freeBst(t);

    t = newBst();
    for (i = 1; i <= 5000; i++)
        bstInsert(t, 1000 + i);
    CHECK(bstHeight(t) == 5000);
    CHECK(bstContains(t, 6000) && !bstContains(t, 6001));
    {
        int *all = (int *)malloc(sizeof(int) * 5000);
        int sorted = 1;
        CHECK(bstInorder(t, all, 5000) == 5000);
        for (i = 1; i < 5000; i++)
            if (all[i - 1] >= all[i])
                sorted = 0;
        CHECK(sorted);
        free(all);
    }
    freeBst(t);
}

static void test_circular_queue(void)
{
    CircularQueue *q = newCircularQueue(3);
    int out = 0, i;

    printf("circular queue...\n");
    CHECK(newCircularQueue(0) == nullptr);
    CHECK(cqIsEmpty(q) && cqCapacity(q) == 3);
    CHECK(cqEnqueue(q, 1) && cqEnqueue(q, 2) && cqEnqueue(q, 3));
    CHECK(cqIsFull(q));
    CHECK(!cqEnqueue(q, 4));
    CHECK(cqDequeue(q, &out) && out == 1);
    CHECK(cqEnqueue(q, 4));
    for (i = 2; i <= 4; i++)
        CHECK(cqDequeue(q, &out) && out == i);
    CHECK(!cqDequeue(q, &out));
    for (i = 0; i < 100; i++) {
        cqEnqueue(q, i);
        CHECK(cqDequeue(q, &out) && out == i);
    }
    freeCircularQueue(q);
}

static void test_deque(void)
{
    Deque *d = newDeque(sizeof(int));
    int i, out = 0;

    printf("deque...\n");
    for (i = 1; i <= 20; i++)
        CHECK(dequeAddFront(d, &i));
    i = 99;
    CHECK(dequeAddBack(d, &i));
    CHECK(dequeSize(d) == 21);
    CHECK(*(const int *)dequeAt(d, 0) == 20);
    CHECK(*(const int *)dequeAt(d, 19) == 1);
    CHECK(*(const int *)dequeAt(d, 20) == 99);
    CHECK(dequeAt(d, 21) == nullptr);
    CHECK(dequeRemoveBack(d, &out) && out == 99);
    CHECK(dequeRemoveBack(d, &out) && out == 1);
    CHECK(dequeRemoveFront(d, &out) && out == 20);
    CHECK(dequePeekFront(d, &out) && out == 19);
    CHECK(dequePeekBack(d, &out) && out == 2);
    while (dequeRemoveFront(d, nullptr)) {
    }
    CHECK(dequeIsEmpty(d));
    CHECK(!dequeRemoveBack(d, &out));
    freeDeque(d);
}

static void test_doubly_linked_list(void)
{
    DList *l = newDList();
    const DNode *n;
    int i, forward = 1, backward = 1;

    printf("doubly linked list...\n");
    for (i = 1; i <= 5; i++)
        CHECK(dlistAddBack(l, i));
    CHECK(dlistAddFront(l, 0));
    CHECK(dlistSize(l) == 6);
    for (i = 0, n = l->head; n != nullptr; n = n->next, i++)
        if (n->value != i)
            forward = 0;
    for (i = 5, n = l->tail; n != nullptr; n = n->prev, i--)
        if (n->value != i)
            backward = 0;
    CHECK(forward && backward);
    CHECK(dlistIndexOf(l, 3) == 3 && dlistIndexOf(l, 9) == -1);

    CHECK(dlistRemove(l, 0));
    CHECK(dlistRemove(l, 5));
    CHECK(dlistRemove(l, 3));
    CHECK(!dlistRemove(l, 3));
    CHECK(l->head->value == 1 && l->tail->value == 4 && l->head->prev == nullptr);
    CHECK(l->head->next->next->value == 4 && l->tail->prev->value == 2);
    CHECK(dlistSize(l) == 3);
    freeDList(l);
}

static void test_binary_search(void)
{
    Graph *g = small_network();
    StationIndex *idx = newStationIndex();
    int ids[8], i;

    printf("binary search...\n");
    CHECK(buildStationIndex(idx, g));
    CHECK(findInIndex(idx, "Charlie") == 2);
    CHECK(findInIndex(idx, "eCHo") == 4);
    CHECK(findInIndex(idx, "Zulu") == -1);
    CHECK(findInIndex(idx, "") == -1);
    for (i = 0; i < 5; i++)
        CHECK(findInIndexRecursive(idx, graphStationName(g, i)) == i);
    CHECK(findInIndexRecursive(idx, "Bravissimo") == -1);

    graphAddStation(g, "Charlotte");
    CHECK(findInIndex(idx, "Charlotte") == -1);
    CHECK(buildStationIndex(idx, g));
    CHECK(findByPrefix(idx, "char", ids, 8) == 2);
    CHECK(ids[0] == 2 && ids[1] == 5);
    CHECK(findByPrefix(idx, "x", ids, 8) == 0);
    CHECK(findByPrefix(idx, "char", ids, 1) == 1);
    freeStationIndex(idx);
    freeGraph(g);
}

static int avl_check_balanced(const AvlNode *node, int *ok)
{
    int left, right;
    if (node == nullptr)
        return 0;
    left = avl_check_balanced(node->left, ok);
    right = avl_check_balanced(node->right, ok);
    if (left - right > 1 || right - left > 1)
        *ok = 0;
    if (node->height != 1 + (left > right ? left : right))
        *ok = 0;
    if (node->left != nullptr && node->left->key >= node->key)
        *ok = 0;
    if (node->right != nullptr && node->right->key <= node->key)
        *ok = 0;
    return 1 + (left > right ? left : right);
}

static void test_avl(void)
{
    int orders[4][3] = {{30, 20, 10}, {10, 20, 30}, {30, 10, 20}, {10, 30, 20}};
    int k, i, ok;
    int out[16];
    Avl *t;

    printf("AVL tree...\n");
    for (k = 0; k < 4; k++) {
        t = newAvl();
        for (i = 0; i < 3; i++)
            CHECK(avlInsert(t, orders[k][i]));
        CHECK(t->root->key == 20);
        CHECK(t->root->left->key == 10 && t->root->right->key == 30);
        CHECK(avlHeight(t) == 2);
        freeAvl(t);
    }

    t = newAvl();
    CHECK(avlHeight(t) == 0);
    CHECK(avlInsert(t, 5));
    CHECK(!avlInsert(t, 5));
    CHECK(avlSize(t) == 1);
    freeAvl(t);

    t = newAvl();
    for (i = 1; i <= 10000; i++)
        avlInsert(t, 1000 + i);
    CHECK(avlSize(t) == 10000);
    CHECK(avlHeight(t) <= 15);
    ok = 1;
    avl_check_balanced(t->root, &ok);
    CHECK(ok);
    CHECK(avlContains(t, 1001) && avlContains(t, 11000) && !avlContains(t, 11001));
    CHECK(avlRange(t, 1050, 1060, out, 16) == 11);
    CHECK(out[0] == 1050 && out[10] == 1060);
    CHECK(avlInorder(t, out, 3) == 3 && out[0] == 1001 && out[2] == 1003);
    freeAvl(t);

    t = newAvl();
    for (i = 0; i < 2000; i++)
        avlInsert(t, (i * 7919) % 2003);
    ok = 1;
    avl_check_balanced(t->root, &ok);
    CHECK(ok);
    {
        int *all = (int *)malloc(sizeof(int) * 2000);
        size_t n = avlInorder(t, all, 2000);
        int sorted = 1;
        for (i = 1; i < (int)n; i++)
            if (all[i - 1] >= all[i])
                sorted = 0;
        CHECK(n == 2000 && sorted);
        free(all);
    }
    freeAvl(t);
}

int main(void)
{
    printf("Running C data structure tests\n");
    test_list();
    test_queue();
    test_stack();
    test_heap();
    test_hashmap();
    test_sort();
    test_graph();
    test_quick_sort();
    test_dfs();
    test_bst();
    test_avl();
    test_circular_queue();
    test_deque();
    test_doubly_linked_list();
    test_binary_search();
    printf("%d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
