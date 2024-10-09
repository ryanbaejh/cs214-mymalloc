#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

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
    head = (chunk_header *)heap.bytes;  // Point head to the start of the heap and treat bytes pointer to chunk_header structure
    head->size = MEMLENGTH;             // Set the size to the total heap size
    head->is_free = true;               // Mark the entire heap as free initially
    head->next = NULL;                  // There is no next chunk initially
}

void coalesce(chunk_header *current);

void print_heap_state() {
    chunk_header *current = head;
    printf("Heap state:\n");
    while (current != NULL) {
        printf("Chunk at %p: size = %zu, is_free = %d\n", (void *)current, current->size, current->is_free);
        current = current->next;
    }
    printf("-------------------------\n");
}

void *mymalloc(size_t size, char *file, int line) {
    //Initialize the heap if it hasn't been done yet
    if (head == NULL) {
        initialize_heap();
    }
    //Align to multiple of 8
    size_t aligned_size = (size + 7) & ~7;  // Round up to nearest multiple of 8
    size_t chunk_size = aligned_size + sizeof(chunk_header);  // Include header size

    //Traverse the linked list to find a suitable free chunk
    chunk_header *current = head;
    while (current != NULL) {
        if (current->is_free && current->size >= chunk_size) {
            //If the chunk is larger than needed, split it
            if (current->size >= chunk_size + sizeof(chunk_header)) {
                //Create a new chunk in the remaining space
                chunk_header *new_chunk = (chunk_header *)((char *)current + chunk_size); //Increments new_chunk pointer by amount of allocated memory 
    
                new_chunk->size = current->size - chunk_size; //How much space is left in new_chunk
                new_chunk->is_free = true; //Sets new chunk to be free
                new_chunk->next = current->next; //Fixes structure of linked list - points new chunk to chunk right after
                current->next = new_chunk;
                current->size = chunk_size;
            }

            //Mark current chunk as allocated and returns a pointer at the allocated memory payload
            current->is_free = false;
            return (void *)((char *)current + sizeof(chunk_header)); //returns location of payload in heap as a pointer
        }
        current = current->next;
    }
    print_heap_state();
    // If no suitable chunk was found, print an error and return NULL
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    if (ptr == NULL) {
        return;  // Do nothing if the pointer is NULL
    }

    // Step 1: Locate the chunk header for the given pointer
    chunk_header *current = (chunk_header *)((char *)ptr - sizeof(chunk_header));

    // Step 2: Mark the chunk as free
    current->is_free = true;

    // Step 3: Coalesce adjacent free chunks
    coalesce(current);
    print_heap_state();
}

void coalesce(chunk_header *current) {
    // Coalesce with the next chunk if it's free
    if (current->next != NULL && current->next->is_free) {
        // Merge current chunk with the next chunk
        current->size += current->next->size + sizeof(chunk_header);
        current->next = current->next->next;
    }

    // Coalesce with the previous chunk if it's free
    chunk_header *prev = head;
    //Traverse Linked List until it's before current
    while (prev != NULL && prev->next != current) {
        prev = prev->next;
    }

    if (prev != NULL && prev->is_free) {
        prev->size += current->size + sizeof(chunk_header);
        prev->next = current->next;
    }
}
