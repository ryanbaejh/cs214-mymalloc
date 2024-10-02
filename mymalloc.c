#include "mymalloc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define MEMLENGTH 4096

static union {
    char bytes[MEMLENGTH];  //Heap array
    double not_used;        //For alignment of 8 bytes
} heap;

// Header structure for each memory chunk
typedef struct chunk_header {
    size_t size;                // Total size of the chunk (header + data)
    bool is_free;                // true is free, false if allocated
    struct chunk_header *next;  // Pointer to the next chunk in the list
} chunk_header;

static chunk_header *head = NULL; // Pointer to the head of the linked list

void initialize_heap() {
    // Initialize the heap as previously discussed
}

void *mymalloc(size_t size, char *file, int line) {
    // Implement malloc logic
}

void myfree(void *ptr, char *file, int line) {
    // Implement free logic
}

