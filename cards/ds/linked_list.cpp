#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

static ListNode *makeNode(size_t itemSize, const void *item)
{
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    if (node == nullptr)
        return nullptr;

    node->data = malloc(itemSize);
    if (node->data == nullptr) {
        free(node);
        return nullptr;
    }

    memcpy(node->data, item, itemSize);
    node->next = nullptr;
    return node;
}

static void freeNode(ListNode *node)
{
    free(node->data);
    free(node);
}

List *newList(size_t itemSize)
{
    List *list;

    if (itemSize == 0)
        return nullptr;

    list = (List *)malloc(sizeof(List));
    if (list == nullptr)
        return nullptr;

    list->head = nullptr;
    list->tail = nullptr;
    list->size = 0;
    list->itemSize = itemSize;
    return list;
}

bool listAddFront(List *list, const void *item)
{
    ListNode *node;

    if (list == nullptr || item == nullptr)
        return false;

    node = makeNode(list->itemSize, item);
    if (node == nullptr)
        return false;

    node->next = list->head;
    list->head = node;
    if (list->tail == nullptr)
        list->tail = node;
    list->size++;
    return true;
}

bool listAddBack(List *list, const void *item)
{
    ListNode *node;

    if (list == nullptr || item == nullptr)
        return false;

    node = makeNode(list->itemSize, item);
    if (node == nullptr)
        return false;

    if (list->tail == nullptr) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
    return true;
}

bool listRemoveFront(List *list, void *out)
{
    ListNode *node;

    if (list == nullptr || list->head == nullptr)
        return false;

    node = list->head;
    if (out != nullptr)
        memcpy(out, node->data, list->itemSize);

    list->head = node->next;
    if (list->head == nullptr)
        list->tail = nullptr;

    freeNode(node);
    list->size--;
    return true;
}

void listTrim(List *list, size_t maxSize)
{
    ListNode *cur;
    ListNode *rest;
    size_t i;

    if (list == nullptr || list->size <= maxSize)
        return;

    if (maxSize == 0) {
        listClear(list);
        return;
    }

    cur = list->head;
    for (i = 1; i < maxSize; i++)
        cur = cur->next;

    rest = cur->next;
    cur->next = nullptr;
    list->tail = cur;

    while (rest != nullptr) {
        ListNode *next = rest->next;
        freeNode(rest);
        rest = next;
    }
    list->size = maxSize;
}

size_t listSize(const List *list)
{
    return list == nullptr ? 0 : list->size;
}

void listClear(List *list)
{
    if (list == nullptr)
        return;

    while (list->head != nullptr) {
        ListNode *next = list->head->next;
        freeNode(list->head);
        list->head = next;
    }
    list->tail = nullptr;
    list->size = 0;
}

void freeList(List *list)
{
    if (list == nullptr)
        return;
    listClear(list);
    free(list);
}
