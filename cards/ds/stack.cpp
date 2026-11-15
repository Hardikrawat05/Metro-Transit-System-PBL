#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

#define STACK_INITIAL_CAPACITY 8

Stack *newStack(size_t itemSize)
{
    Stack *s;

    if (itemSize == 0)
        return nullptr;

    s = (Stack *)malloc(sizeof(Stack));
    if (s == nullptr)
        return nullptr;

    s->items = nullptr;
    s->itemSize = itemSize;
    s->count = 0;
    s->capacity = 0;
    return s;
}

static bool grow(Stack *s)
{
    size_t newCapacity = s->capacity == 0 ? STACK_INITIAL_CAPACITY : s->capacity * 2;
    unsigned char *bigger;

    bigger = (unsigned char *)realloc(s->items, newCapacity * s->itemSize);
    if (bigger == nullptr)
        return false;

    s->items = bigger;
    s->capacity = newCapacity;
    return true;
}

bool stackPush(Stack *s, const void *item)
{
    if (s == nullptr || item == nullptr)
        return false;

    if (s->count == s->capacity && !grow(s))
        return false;

    memcpy(s->items + s->count * s->itemSize, item, s->itemSize);
    s->count++;
    return true;
}

bool stackPop(Stack *s, void *out)
{
    if (s == nullptr || s->count == 0)
        return false;

    s->count--;
    if (out != nullptr)
        memcpy(out, s->items + s->count * s->itemSize, s->itemSize);
    return true;
}

bool stackPeek(const Stack *s, void *out)
{
    if (s == nullptr || s->count == 0)
        return false;
    if (out != nullptr)
        memcpy(out, s->items + (s->count - 1) * s->itemSize, s->itemSize);
    return true;
}

const void *stackFromTop(const Stack *s, size_t index)
{
    if (s == nullptr || index >= s->count)
        return nullptr;
    return s->items + (s->count - 1 - index) * s->itemSize;
}

size_t stackSize(const Stack *s)
{
    return s == nullptr ? 0 : s->count;
}

bool stackIsEmpty(const Stack *s)
{
    return s == nullptr || s->count == 0;
}

void stackClear(Stack *s)
{
    if (s != nullptr)
        s->count = 0;
}

void freeStack(Stack *s)
{
    if (s == nullptr)
        return;
    free(s->items);
    free(s);
}
