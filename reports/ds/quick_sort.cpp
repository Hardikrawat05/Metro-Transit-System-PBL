#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

typedef int (*CompareFn)(const void *, const void *);

#define SMALL_RANGE 10

struct Sorter {
    unsigned char *a;
    size_t itemSize;
    CompareFn cmp;
    unsigned char *temp;
    unsigned char *pivot;
};

#define AT(s, i) ((s)->a + (size_t)(i) * (s)->itemSize)

static void swapItems(Sorter *s, long long i, long long j)
{
    if (i == j)
        return;
    memcpy(s->temp, AT(s, i), s->itemSize);
    memcpy(AT(s, i), AT(s, j), s->itemSize);
    memcpy(AT(s, j), s->temp, s->itemSize);
}

static void insertionSort(Sorter *s, long long low, long long high)
{
    long long i, j;
    for (i = low + 1; i <= high; i++) {
        memcpy(s->temp, AT(s, i), s->itemSize);
        j = i - 1;
        while (j >= low && s->cmp(AT(s, j), s->temp) > 0) {
            memcpy(AT(s, j + 1), AT(s, j), s->itemSize);
            j--;
        }
        memcpy(AT(s, j + 1), s->temp, s->itemSize);
    }
}

static void quickRange(Sorter *s, long long low, long long high)
{
    while (high - low + 1 > SMALL_RANGE) {
        long long mid = low + (high - low) / 2;
        long long i = low;
        long long j = high;

        if (s->cmp(AT(s, mid), AT(s, low)) < 0)
            swapItems(s, mid, low);
        if (s->cmp(AT(s, high), AT(s, low)) < 0)
            swapItems(s, high, low);
        if (s->cmp(AT(s, high), AT(s, mid)) < 0)
            swapItems(s, high, mid);
        memcpy(s->pivot, AT(s, mid), s->itemSize);

        while (i <= j) {
            while (s->cmp(AT(s, i), s->pivot) < 0)
                i++;
            while (s->cmp(AT(s, j), s->pivot) > 0)
                j--;
            if (i <= j) {
                swapItems(s, i, j);
                i++;
                j--;
            }
        }

        if (j - low < high - i) {
            quickRange(s, low, j);
            low = i;
        } else {
            quickRange(s, i, high);
            high = j;
        }
    }
    if (low < high)
        insertionSort(s, low, high);
}

bool quickSort(void *base, size_t count, size_t itemSize, CompareFn cmp)
{
    Sorter s;

    if (base == nullptr || cmp == nullptr || itemSize == 0 || count < 2)
        return true;

    s.a = (unsigned char *)base;
    s.itemSize = itemSize;
    s.cmp = cmp;
    s.temp = (unsigned char *)malloc(itemSize);
    s.pivot = (unsigned char *)malloc(itemSize);
    if (s.temp == nullptr || s.pivot == nullptr) {
        free(s.temp);
        free(s.pivot);
        return false;
    }

    quickRange(&s, 0, (long long)count - 1);

    free(s.temp);
    free(s.pivot);
    return true;
}
