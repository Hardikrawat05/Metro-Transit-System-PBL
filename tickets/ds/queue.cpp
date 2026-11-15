#include "metro.h"

#include <cstdlib>

using namespace std;

Queue *newQueue(void)
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == nullptr)
        return nullptr;

    q->front = nullptr;
    q->rear = nullptr;
    q->size = 0;
    return q;
}

bool enqueue(Queue *q, int value)
{
    QueueNode *node;

    if (q == nullptr)
        return false;

    node = (QueueNode *)malloc(sizeof(QueueNode));
    if (node == nullptr)
        return false;

    node->value = value;
    node->next = nullptr;

    if (q->rear == nullptr) {
        q->front = node;
        q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
    q->size++;
    return true;
}

bool dequeue(Queue *q, int *out)
{
    QueueNode *node;

    if (q == nullptr || q->front == nullptr)
        return false;

    node = q->front;
    if (out != nullptr)
        *out = node->value;

    q->front = node->next;
    if (q->front == nullptr)
        q->rear = nullptr;

    free(node);
    q->size--;
    return true;
}

bool queuePeek(const Queue *q, int *out)
{
    if (q == nullptr || q->front == nullptr)
        return false;
    if (out != nullptr)
        *out = q->front->value;
    return true;
}

bool queueIsEmpty(const Queue *q)
{
    return q == nullptr || q->front == nullptr;
}

size_t queueSize(const Queue *q)
{
    return q == nullptr ? 0 : q->size;
}

void queueClear(Queue *q)
{
    if (q == nullptr)
        return;
    while (dequeue(q, nullptr)) {
    }
}

void freeQueue(Queue *q)
{
    if (q == nullptr)
        return;
    queueClear(q);
    free(q);
}
