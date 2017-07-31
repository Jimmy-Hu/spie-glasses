#include <stdio.h>

void swap_int(int * a, int * b)
{
    if (a == b) return;
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int partition_inplace(int * a, int b, int e)
{
    int v = a[--e];
    int i = b;
    for (int j = b; j < e; ++j) {
        if (a[j] <= v) {
            swap_int(a + i, a + j);
            ++i;
        }
    }
    swap_int(a + i, a + e);
    return i + 1;
}

void quick_sort_range(int * a, int b, int e)
{
    int q = partition_inplace(a, b, e);
    quick_sort_range(a, b, q);
    quick_sort_range(a, q, e);
}

int main()
{
    return 0;
}
