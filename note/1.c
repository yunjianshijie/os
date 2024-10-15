// //P1015
#include <stdio.h>
#include <string.h>
// // int main(){
// //     char p0[] = "I love Linux";
// //     const char *p1 = "I love Linux\0Group";
    
// }
int average(int nums[], int start, int end) {
  if (start == end)
    return nums[start];
  int mid = (start + end) / 2;
  int leftAvg = average(nums, start, mid);
  int rightAvg = average(nums, mid + 1, end);
  return (leftAvg & rightAvg) + ((leftAvg ^ rightAvg) >> 1);
}

int main(){
    int nums[] = {1, 2, 3,4,78};
    int start = 0;
    int end = 4;
     printf("%d\n", average(nums, start, end));

     printf("%d",(4& 12)+(4^12)>>1);
     return 0;
}

// // int i = 1;
// // static int j = 15;
// // int func() {
// //   int i = 10;
// //   if (i > 5)
// //     i++;
// //   printf("i = %d, j = %d\n", i, j);
// //   return i % j;
// // }
// // int main() {
// //   int a = func();
// //   printf("a = %d\n", a);
// //   printf("i = %d, j = %d\n", i, j);
// //   return 0;
// // }


// // int main(int argc, char **argv) {
// //   int a = 1, b = 2;
// //   const int *p = &a;
// //   int *const q = &b;
// //   *p = 3, q = &a;
// //   const int *const r = &a;
// //   *r = 4, r = &b;
// //   return 0;
// // }

// // int main() {
// //   int a[2] = {4, 8};
// //   int(*b)[2] = &a;
// //   int *c[2] = {a, a + 1};

// //   printf("%d,%d,%d",a,a+1,&a,&a+1);
// //   return 0;
// // }

// // #define SQUARE(x) x *x
// // #define MAX(a, b) (a > b) ? a : b;
// // #define PRINT(x) printf("嘻嘻，结果你猜对了吗，包%d滴\n", x);
// // #define CONCAT(a, b) a##b

// // int main() {
// //   int CONCAT(x, 1) = 5;
// //   int CONCAT(y, 2) = 3;
// //   int max = MAX(SQUARE(x1 + 1), SQUARE(y2)) PRINT(max) return 0;
// // }

// #include <stdlib.h>




// // int main() {
// //   int arr1[] = {2, 3, 1, 3, 2, 4, 6, 7, 9, 2, 10};
// //   int arr2[] = {2, 1, 4, 3, 9, 6, 8};
// //   int len1 = sizeof(arr1) / sizeof(arr1[0]);
// //   int len2 = sizeof(arr2) / sizeof(arr2[0]);

// //   result result;
// //   your_sort(arr1, len1, arr2, len2, &result);
// //   for (int i = 0; i < result.len; i++) {
// //     printf("%d ", result.arr[i]);
// //   }
// //   free(result.arr);
// //   return 0;
// // // }

// // int main() {
// //   void *a[] = {(void *)1, (void *)2, (void *)3, (void *)4, (void *)5};
// //   printf("%d\n", *((char *)a + 1));
// //   printf("%d\n", *(int *)(char *)a + 1);
// //   printf("%d\n", *((int *)a + 2));
// //   printf("%lld\n", *((long long *)a + 3));
// //   printf("%d\n", *((short *)a + 4));
// //   return 0;
// // }

// union data {
//   int a;
//   double b;
//   short c;
// };
// typedef struct node {
//   long long a;
//   union data b;
//   void (*change)(struct node *n);
//   char string[0];
// } Node;
// void func(Node *node) {
//   for (size_t i = 0; node->string[i] != '\0'; i++)
//     node->string[i] = tolower(node->string[i]);
// }
// int main() {
//   const char *s = "WELCOME TO XIYOULINUX_GROUP!";
//   Node *P = (Node *)malloc(sizeof(Node) + (strlen(s) + 1) * sizeof(char));
//   strcpy(P->string, s);
//   P->change = func;
//   P->change(P);
//   printf("%s\n", P->string);
//   return 0;
// }