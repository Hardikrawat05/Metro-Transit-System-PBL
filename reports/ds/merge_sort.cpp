#include "metro.h"

#include <cstdlib>
#include <cstring>

using namespace std;

typedef int (*CompareFn)(const void *, const void *);

static void mergeRange(unsigned char *a, unsigned char *temp, size_t low, size_t high,
                        size_t itemSize, CompareFn cmp)
{
    size_t mid, i, j, k;

    if (high - low < 2)
        return;

    mid = low + (high - low) / 2;
    mergeRange(a, temp, low, mid, itemSize, cmp);
    mergeRange(a, temp, mid, high, itemSize, cmp);

    i = low;
    j = mid;
    k = low;
    while (i < mid && j < high) {
        if (cmp(a + j * itemSize, a + i * itemSize) < 0) {
            memcpy(temp + k * itemSize, a + j * itemSize, itemSize);
            j++;
        } else {
            memcpy(temp + k * itemSize, a + i * itemSize, itemSize);
            i++;
        }
        k++;
    }
    while (i < mid) {
        memcpy(temp + k * itemSize, a + i * itemSize, itemSize);
        i++;
        k++;
    }
    while (j < high) {
        memcpy(temp + k * itemSize, a + j * itemSize, itemSize);
        j++;
        k++;
    }

    memcpy(a + low * itemSize, temp + low * itemSize, (high - low) * itemSize);
}

bool mergeSort(void *base, size_t count, size_t itemSize, CompareFn cmp)
{
    unsigned char *temp;

    if (base == nullptr || cmp == nullptr || itemSize == 0 || count < 2)
        return true;

    temp = (unsigned char *)malloc(count * itemSize);
    if (temp == nullptr)
        return false;

    mergeRange((unsigned char *)base, temp, 0, count, itemSize, cmp);
    free(temp);
    return true;
}
