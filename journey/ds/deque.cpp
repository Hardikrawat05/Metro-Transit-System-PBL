#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

#define DEQUE_INITIAL_CAPACITY 8

static unsigned char *slot(const Deque *d, size_t i)
{
    return d->items + ((d->head + i) % d->capacity) * d->itemSize;
}

Deque *newDeque(size_t itemSize)
{
    Deque *d;

    if (itemSize == 0)
        return nullptr;

    d = (Deque *)malloc(sizeof(Deque));
    if (d == nullptr)
        return nullptr;

    d->items = nullptr;
    d->itemSize = itemSize;
    d->capacity = 0;
    d->head = 0;
    d->count = 0;
    return d;
}

static bool grow(Deque *d)
{
    size_t newCapacity = d->capacity == 0 ? DEQUE_INITIAL_CAPACITY : d->capacity * 2;
    unsigned char *bigger;
    size_t i;

    bigger = (unsigned char *)malloc(newCapacity * d->itemSize);
    if (bigger == nullptr)
        return false;

    for (i = 0; i < d->count; i++)
        memcpy(bigger + i * d->itemSize, slot(d, i), d->itemSize);

    free(d->items);
    d->items = bigger;
    d->capacity = newCapacity;
    d->head = 0;
    return true;
}

bool dequeAddFront(Deque *d, const void *item)
{
    if (d == nullptr || item == nullptr)
        return false;
    if (d->count == d->capacity && !grow(d))
        return false;

    d->head = (d->head + d->capacity - 1) % d->capacity;
    memcpy(d->items + d->head * d->itemSize, item, d->itemSize);
    d->count++;
    return true;
}

bool dequeAddBack(Deque *d, const void *item)
{
    if (d == nullptr || item == nullptr)
        return false;
    if (d->count == d->capacity && !grow(d))
        return false;

    memcpy(slot(d, d->count), item, d->itemSize);
    d->count++;
    return true;
}

bool dequeRemoveFront(Deque *d, void *out)
{
    if (d == nullptr || d->count == 0)
        return false;
    if (out != nullptr)
        memcpy(out, slot(d, 0), d->itemSize);
    d->head = (d->head + 1) % d->capacity;
    d->count--;
    return true;
}

bool dequeRemoveBack(Deque *d, void *out)
{
    if (d == nullptr || d->count == 0)
        return false;
    if (out != nullptr)
        memcpy(out, slot(d, d->count - 1), d->itemSize);
    d->count--;
    return true;
}

bool dequePeekFront(const Deque *d, void *out)
{
    if (d == nullptr || d->count == 0)
        return false;
    if (out != nullptr)
        memcpy(out, slot(d, 0), d->itemSize);
    return true;
}

bool dequePeekBack(const Deque *d, void *out)
{
    if (d == nullptr || d->count == 0)
        return false;
    if (out != nullptr)
        memcpy(out, slot(d, d->count - 1), d->itemSize);
    return true;
}

const void *dequeAt(const Deque *d, size_t index)
{
    if (d == nullptr || index >= d->count)
        return nullptr;
    return slot(d, index);
}

size_t dequeSize(const Deque *d)
{
    return d == nullptr ? 0 : d->count;
}

bool dequeIsEmpty(const Deque *d)
{
    return d == nullptr || d->count == 0;
}

void dequeClear(Deque *d)
{
    if (d != nullptr) {
        d->head = 0;
        d->count = 0;
    }
}

void freeDeque(Deque *d)
{
    if (d == nullptr)
        return;
    free(d->items);
    free(d);
}
