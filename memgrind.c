#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#include "mymalloc.h"

int main() {
    int *ptr = (int *)mymalloc(sizeof(int) * 5);  // Allocate memory for 5 integers

    if (ptr != NULL) {
        printf("Memory allocation successful!\n");

        // Use the allocated memory
        for (int i = 0; i < 5; i++) {
            ptr[i] = i;
            printf("ptr[%d] = %d\n", i, ptr[i]);
        }
    } else {
        printf("Memory allocation failed!\n");
    }

    return 0;
}

int *arr = (int *)mymalloc(10 * sizeof(int));  // Allocating memory
// Use the memory...
myfree(arr);  // Freeing the memory when no longer needed

