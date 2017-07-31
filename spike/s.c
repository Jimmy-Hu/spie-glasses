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
     if (b == e) return b;
     int v = a[--e];
     int i = b;
     for (int j = b; j < e; ++j) {
          if (a[j] <= v) {
               swap_int(a + i, a + j);
               ++i;
          }
     }
     swap_int(a + i, a + e);
     return i;
}

void quick_sort_range(int * a, int b, int e)
{
     if (b == e) return;
     int q = partition_inplace(a, b, e);
     quick_sort_range(a, b, q);
     quick_sort_range(a, q + 1, e);
}

void quick_sort(int * a, int n)
{
     quick_sort_range(a, 0, n);
}

#define ARRAY_SIZE(_a) (sizeof((_a)) / sizeof(*(_a)))

int main()
{
     int a[] = {1, 5, 3, 7, 9, 0, 4, 8, 2, 6};
     /* int a[] = {1, 8, 2, 6}; */
     int n = ARRAY_SIZE(a);
     quick_sort(a, n);
     /* int q = partition_inplace(a, 0, n); */
     for (int i = 0; i < n; ++i) {
          printf("%d\n", a[i]);
     }
     /* printf("q=%d\n", q); */
     return 0;
}
