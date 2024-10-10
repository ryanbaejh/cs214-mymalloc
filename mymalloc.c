#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void leak_detector() {
    size_t total_leaked_bytes = 0;
    size_t leaked_objects = 0;
    chunk_header *current = head;

    while (current != NULL) {
        if (!current->is_free) {
            total_leaked_bytes += current->size - sizeof(chunk_header);
            leaked_objects++;
        }
        current = current->next;
    }

    if (leaked_objects > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %zu objects.\n", total_leaked_bytes, leaked_objects);
    }
}

void initialize_heap() {
    head = (chunk_header *)heap.bytes;  // Point head to the start of the heap and treat bytes pointer to chunk_header structure
    head->size = MEMLENGTH;             // Set the size to the total heap size
    head->is_free = true;               // Mark the entire heap as free initially
    head->next = NULL;                  // There is no next chunk initially

    atexit(leak_detector);
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
            if (current->size > chunk_size + sizeof(chunk_header)) {
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
    // If no suitable chunk was found, print an error and return NULL
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void coalesce(chunk_header *current) {
    // Coalesce with the next chunk if it's free
    if (current->next != NULL && current->next->is_free) {
        // Merge current chunk with the next chunk
        current->size += current->next->size;
        current->next = current->next->next;
    }

    // Coalesce with the previous chunk if it's free
    chunk_header *prev = head;
    //Traverse Linked List until it's before current
    while (prev != NULL && prev->next != current) {
        prev = prev->next;
    }

    if (prev != NULL && prev->is_free) {
        prev->size += current->size;
        prev->next = current->next;
    }
}

void myfree(void *ptr, char *file, int line) {
    if (ptr == NULL) {
        return; // No action needed for NULL pointer
    }

    // Calculate the chunk header address from the payload pointer
    chunk_header *chunk = (chunk_header *)((char *)ptr - sizeof(chunk_header));

    //Calling free() with an address not obtained from malloc()
    chunk_header *current = head;
    bool valid_pointer = false;
    while (current != NULL) {
        if (current == chunk) {
            valid_pointer = true; //Pointer found in the linked list
            break;
        }
        current = current->next;
    }

    if (!valid_pointer) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    //Calling free() with an address not at the start of a chunk
    if ((char *)ptr != (char *)chunk + sizeof(chunk_header)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    //Calling free() a second time on the same pointer
    if (chunk->is_free) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    // Mark chunk as free
    chunk->is_free = true;
    coalesce(current);
}


