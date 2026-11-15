#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

using namespace std;

static bool toLowerKey(const char *text, char *out)
{
    size_t i, len;
    if (text == nullptr)
        return false;
    len = strlen(text);
    if (len >= GRAPH_NAME_MAX)
        return false;
    for (i = 0; i <= len; i++)
        out[i] = (char)tolower((unsigned char)text[i]);
    return true;
}

static int compareNames(const void *a, const void *b)
{
    return strcmp(((const StationIndexEntry *)a)->key, ((const StationIndexEntry *)b)->key);
}

StationIndex *newStationIndex(void)
{
    return (StationIndex *)calloc(1, sizeof(StationIndex));
}

bool buildStationIndex(StationIndex *index, const Graph *g)
{
    int i, n;
    StationIndexEntry *entries;

    if (index == nullptr || g == nullptr)
        return false;

    n = graphStationCount(g);
    entries = n > 0 ? (StationIndexEntry *)malloc((size_t)n * sizeof(StationIndexEntry)) : nullptr;
    if (n > 0 && entries == nullptr)
        return false;

    for (i = 0; i < n; i++) {
        toLowerKey(graphStationName(g, i), entries[i].key);
        entries[i].id = i;
    }
    if (!quickSort(entries, (size_t)n, sizeof(StationIndexEntry), compareNames)) {
        free(entries);
        return false;
    }

    free(index->entries);
    index->entries = entries;
    index->count = n;
    return true;
}

int findInIndex(const StationIndex *index, const char *name)
{
    char key[GRAPH_NAME_MAX];
    int low = 0, high;

    if (index == nullptr || !toLowerKey(name, key))
        return -1;

    high = index->count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int c = strcmp(key, index->entries[mid].key);
        if (c == 0)
            return index->entries[mid].id;
        if (c < 0)
            high = mid - 1;
        else
            low = mid + 1;
    }
    return -1;
}

static int searchRange(const StationIndex *index, const char *key, int low, int high)
{
    int mid, c;

    if (low > high)
        return -1;
    mid = low + (high - low) / 2;
    c = strcmp(key, index->entries[mid].key);
    if (c == 0)
        return index->entries[mid].id;
    if (c < 0)
        return searchRange(index, key, low, mid - 1);
    return searchRange(index, key, mid + 1, high);
}

int findInIndexRecursive(const StationIndex *index, const char *name)
{
    char key[GRAPH_NAME_MAX];
    if (index == nullptr || !toLowerKey(name, key))
        return -1;
    return searchRange(index, key, 0, index->count - 1);
}

int findByPrefix(const StationIndex *index, const char *prefix, int *outIds,
                         int maxOut)
{
    char key[GRAPH_NAME_MAX];
    size_t len;
    int low = 0, high, found = 0;

    if (index == nullptr || outIds == nullptr || maxOut <= 0 || !toLowerKey(prefix, key))
        return 0;
    len = strlen(key);
    if (len == 0)
        return 0;

    high = index->count;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (strcmp(index->entries[mid].key, key) < 0)
            low = mid + 1;
        else
            high = mid;
    }

    while (low < index->count && found < maxOut &&
           strncmp(index->entries[low].key, key, len) == 0) {
        outIds[found++] = index->entries[low].id;
        low++;
    }
    return found;
}

void freeStationIndex(StationIndex *index)
{
    if (index == nullptr)
        return;
    free(index->entries);
    free(index);
}
