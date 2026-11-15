#include "metro.h"

#include <cstdlib>

using namespace std;

CircularQueue *newCircularQueue(size_t capacity)
{
    CircularQueue *q;

    if (capacity == 0)
        return nullptr;

    q = (CircularQueue *)malloc(sizeof(CircularQueue));
    if (q == nullptr)
        return nullptr;

    q->items = (int *)malloc(capacity * sizeof(int));
    if (q->items == nullptr) {
        free(q);
        return nullptr;
    }
    q->capacity = capacity;
    q->front = 0;
    q->count = 0;
    return q;
}

bool cqEnqueue(CircularQueue *q, int value)
{
    size_t rear;

    if (q == nullptr || q->count == q->capacity)
        return false;

    rear = (q->front + q->count) % q->capacity;
    q->items[rear] = value;
    q->count++;
    return true;
}

bool cqDequeue(CircularQueue *q, int *out)
{
    if (q == nullptr || q->count == 0)
        return false;

    if (out != nullptr)
        *out = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;
    return true;
}

bool cqPeek(const CircularQueue *q, int *out)
{
    if (q == nullptr || q->count == 0)
        return false;
    if (out != nullptr)
        *out = q->items[q->front];
    return true;
}

bool cqIsFull(const CircularQueue *q)
{
    return q != nullptr && q->count == q->capacity;
}

bool cqIsEmpty(const CircularQueue *q)
{
    return q == nullptr || q->count == 0;
}

size_t cqSize(const CircularQueue *q)
{
    return q == nullptr ? 0 : q->count;
}

size_t cqCapacity(const CircularQueue *q)
{
    return q == nullptr ? 0 : q->capacity;
}

void cqClear(CircularQueue *q)
{
    if (q != nullptr) {
        q->front = 0;
        q->count = 0;
    }
}

void freeCircularQueue(CircularQueue *q)
{
    if (q == nullptr)
        return;
    free(q->items);
    free(q);
}
