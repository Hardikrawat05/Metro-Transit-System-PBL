#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

#define HM_INITIAL_BUCKETS 16

static size_t bucketIndex(int key, size_t bucketCount)
{
    unsigned int positive = (unsigned int)key;
    return positive % bucketCount;
}

HashMap *newTable(size_t itemSize)
{
    HashMap *map;

    if (itemSize == 0)
        return nullptr;

    map = (HashMap *)malloc(sizeof(HashMap));
    if (map == nullptr)
        return nullptr;

    map->buckets = (HmEntry **)calloc(HM_INITIAL_BUCKETS, sizeof(HmEntry *));
    if (map->buckets == nullptr) {
        free(map);
        return nullptr;
    }

    map->bucketCount = HM_INITIAL_BUCKETS;
    map->size = 0;
    map->itemSize = itemSize;
    return map;
}

static HmEntry *findEntry(const HashMap *map, int key)
{
    HmEntry *e = map->buckets[bucketIndex(key, map->bucketCount)];
    while (e != nullptr) {
        if (e->key == key)
            return e;
        e = e->next;
    }
    return nullptr;
}

static bool resize(HashMap *map, size_t newCount)
{
    HmEntry **new_buckets;
    size_t i;

    new_buckets = (HmEntry **)calloc(newCount, sizeof(HmEntry *));
    if (new_buckets == nullptr)
        return false;

    for (i = 0; i < map->bucketCount; i++) {
        HmEntry *e = map->buckets[i];
        while (e != nullptr) {
            HmEntry *next = e->next;
            size_t idx = bucketIndex(e->key, newCount);
            e->next = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->bucketCount = newCount;
    return true;
}

void *tablePut(HashMap *map, int key, const void *value)
{
    HmEntry *e;
    size_t idx;

    if (map == nullptr || value == nullptr)
        return nullptr;

    e = findEntry(map, key);
    if (e != nullptr) {
        memcpy(e->value, value, map->itemSize);
        return e->value;
    }

    if ((map->size + 1) * 4 > map->bucketCount * 3)
        resize(map, map->bucketCount * 2);

    e = (HmEntry *)malloc(sizeof(HmEntry));
    if (e == nullptr)
        return nullptr;

    e->value = malloc(map->itemSize);
    if (e->value == nullptr) {
        free(e);
        return nullptr;
    }

    memcpy(e->value, value, map->itemSize);
    e->key = key;
    idx = bucketIndex(key, map->bucketCount);
    e->next = map->buckets[idx];
    map->buckets[idx] = e;
    map->size++;
    return e->value;
}

void *tableGet(const HashMap *map, int key)
{
    HmEntry *e;

    if (map == nullptr)
        return nullptr;

    e = findEntry(map, key);
    return e == nullptr ? nullptr : e->value;
}

bool tableHas(const HashMap *map, int key)
{
    return tableGet(map, key) != nullptr;
}

bool tableRemove(HashMap *map, int key)
{
    size_t idx;
    HmEntry *prev = nullptr;
    HmEntry *cur;

    if (map == nullptr)
        return false;

    idx = bucketIndex(key, map->bucketCount);
    cur = map->buckets[idx];
    while (cur != nullptr) {
        if (cur->key == key) {
            if (prev == nullptr)
                map->buckets[idx] = cur->next;
            else
                prev->next = cur->next;
            free(cur->value);
            free(cur);
            map->size--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

size_t tableSize(const HashMap *map)
{
    return map == nullptr ? 0 : map->size;
}

void tableIterStart(const HashMap *map, HmIter *it)
{
    if (it == nullptr)
        return;
    it->map = map;
    it->bucket = 0;
    it->entry = nullptr;
}

bool tableIterNext(HmIter *it, int *keyOut, void **valueOut)
{
    if (it == nullptr || it->map == nullptr)
        return false;

    if (it->entry != nullptr)
        it->entry = it->entry->next;

    while (it->entry == nullptr) {
        if (it->bucket >= it->map->bucketCount)
            return false;
        it->entry = it->map->buckets[it->bucket++];
    }

    if (keyOut != nullptr)
        *keyOut = it->entry->key;
    if (valueOut != nullptr)
        *valueOut = it->entry->value;
    return true;
}

void tableClear(HashMap *map)
{
    size_t i;

    if (map == nullptr)
        return;

    for (i = 0; i < map->bucketCount; i++) {
        HmEntry *e = map->buckets[i];
        while (e != nullptr) {
            HmEntry *next = e->next;
            free(e->value);
            free(e);
            e = next;
        }
        map->buckets[i] = nullptr;
    }
    map->size = 0;
}

void freeTable(HashMap *map)
{
    if (map == nullptr)
        return;
    tableClear(map);
    free(map->buckets);
    free(map);
}
