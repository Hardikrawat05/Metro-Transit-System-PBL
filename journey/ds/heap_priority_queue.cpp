#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

#define HEAP_INITIAL_CAPACITY 16

static bool comesBefore(const HeapItem *a, const HeapItem *b)
{
    if (a->key1 != b->key1)
        return a->key1 < b->key1;
    return a->key2 < b->key2;
}

static void swapItems(HeapItem *a, HeapItem *b)
{
    HeapItem temp = *a;
    *a = *b;
    *b = temp;
}

Heap *newHeap(void)
{
    Heap *h = (Heap *)malloc(sizeof(Heap));
    if (h == nullptr)
        return nullptr;

    h->items = nullptr;
    h->count = 0;
    h->capacity = 0;
    return h;
}

static bool grow(Heap *h)
{
    size_t newCapacity = h->capacity == 0 ? HEAP_INITIAL_CAPACITY : h->capacity * 2;
    HeapItem *bigger;

    bigger = (HeapItem *)realloc(h->items, newCapacity * sizeof(HeapItem));
    if (bigger == nullptr)
        return false;

    h->items = bigger;
    h->capacity = newCapacity;
    return true;
}

bool heapPush(Heap *h, HeapItem item)
{
    size_t i;

    if (h == nullptr)
        return false;
    if (h->count == h->capacity && !grow(h))
        return false;

    i = h->count++;
    h->items[i] = item;
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (!comesBefore(&h->items[i], &h->items[parent]))
            break;
        swapItems(&h->items[i], &h->items[parent]);
        i = parent;
    }
    return true;
}

bool heapPop(Heap *h, HeapItem *out)
{
    size_t i = 0;

    if (h == nullptr || h->count == 0)
        return false;

    if (out != nullptr)
        *out = h->items[0];

    h->count--;
    if (h->count == 0)
        return true;
    h->items[0] = h->items[h->count];

    for (;;) {
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        size_t best = i;

        if (left < h->count && comesBefore(&h->items[left], &h->items[best]))
            best = left;
        if (right < h->count && comesBefore(&h->items[right], &h->items[best]))
            best = right;
        if (best == i)
            break;

        swapItems(&h->items[i], &h->items[best]);
        i = best;
    }
    return true;
}

bool heapPeek(const Heap *h, HeapItem *out)
{
    if (h == nullptr || h->count == 0)
        return false;
    if (out != nullptr)
        *out = h->items[0];
    return true;
}

size_t heapSize(const Heap *h)
{
    return h == nullptr ? 0 : h->count;
}

Heap *heapCopy(const Heap *h)
{
    Heap *copy;

    if (h == nullptr)
        return nullptr;

    copy = newHeap();
    if (copy == nullptr)
        return nullptr;

    if (h->count > 0) {
        copy->items = (HeapItem *)malloc(h->count * sizeof(HeapItem));
        if (copy->items == nullptr) {
            free(copy);
            return nullptr;
        }
        memcpy(copy->items, h->items, h->count * sizeof(HeapItem));
        copy->count = h->count;
        copy->capacity = h->count;
    }
    return copy;
}

void heapClear(Heap *h)
{
    if (h != nullptr)
        h->count = 0;
}

void freeHeap(Heap *h)
{
    if (h == nullptr)
        return;
    free(h->items);
    free(h);
}
