#include "metro.h"

#include <cstdlib>

using namespace std;

DList *newDList(void)
{
    return (DList *)calloc(1, sizeof(DList));
}

static DNode *makeNode(int value)
{
    DNode *node = (DNode *)malloc(sizeof(DNode));
    if (node != nullptr) {
        node->value = value;
        node->prev = nullptr;
        node->next = nullptr;
    }
    return node;
}

bool dlistAddBack(DList *list, int value)
{
    DNode *node;

    if (list == nullptr || (node = makeNode(value)) == nullptr)
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

bool dlistAddFront(DList *list, int value)
{
    DNode *node;

    if (list == nullptr || (node = makeNode(value)) == nullptr)
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

bool dlistRemove(DList *list, int value)
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

int dlistIndexOf(const DList *list, int value)
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

bool dlistContains(const DList *list, int value)
{
    return dlistIndexOf(list, value) >= 0;
}

size_t dlistSize(const DList *list)
{
    return list == nullptr ? 0 : list->size;
}

void dlistClear(DList *list)
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

void freeDList(DList *list)
{
    if (list == nullptr)
        return;
    dlistClear(list);
    free(list);
}
