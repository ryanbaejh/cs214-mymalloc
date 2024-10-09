#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

// Compile with -DLEAK to leak memory
#ifndef LEAK
#define LEAK 0
#endif

#define MEMSIZE 4096
#define HEADERSIZE 24
#define OBJECTS 64
#define OBJSIZE (MEMSIZE / OBJECTS - HEADERSIZE)


void test_default() {
	char *obj[OBJECTS];
	int i, j, errors = 0;
	
	// fill memory with objects
	for (i = 0; i < OBJECTS; i++) {
		obj[i] = malloc(OBJSIZE);
		if (obj[i] == NULL) {
		    printf("Unable to allocate object %d\n", i);
		    exit(1);
		}
	}
	
	// fill each object with distinct bytes
	for (i = 0; i < OBJECTS; i++) {
		memset(obj[i], i, OBJSIZE);
	}
	
	// check that all objects contain the correct bytes
	for (i = 0; i < OBJECTS; i++) {
		for (j = 0; j < OBJSIZE; j++) {
			if (obj[i][j] != i) {
				errors++;
				printf("Object %d byte %d incorrect: %d\n", i, j, obj[i][j]);
			}
		}
	}

	// free all objects
	if (!LEAK) {
	    for (i = 0; i < OBJECTS; i++) {
		free(obj[i]);
	    }
	}
	
	printf("%d incorrect bytes\n", errors);
}

void test_allocation() {
    printf("Test 1: Allocation of Unallocated Memory\n");

    char *obj = malloc(OBJSIZE);
    if (obj == NULL) {
        fprintf(stderr, "Test 1 Failed: malloc() returned NULL\n");
        exit(1);
    } else {
        printf("Test 1 Passed: malloc() allocated memory at %p\n", (void *)obj);
    }

    free(obj);  // Clean up
}

void test_non_overlapping_allocations() {
    printf("Test 2: Non-overlapping Allocations\n");

    char *objs[OBJECTS];
    int i, j;

    // Allocate multiple objects
    for (i = 0; i < OBJECTS; i++) {
        objs[i] = malloc(OBJSIZE);
        if (objs[i] == NULL) {
            fprintf(stderr, "Test 2 Failed: malloc() returned NULL at iteration %d\n", i);
            exit(1);
        }
    }

    // Check for overlapping regions
    for (i = 0; i < OBJECTS; i++) {
        for (j = i + 1; j < OBJECTS; j++) {
            if (objs[i] < objs[j]) {
                if (objs[i] + OBJSIZE > objs[j]) {
                    fprintf(stderr, "Test 2 Failed: Objects %d and %d overlap\n", i, j);
                    exit(1);
                }
            } else {
                if (objs[j] + OBJSIZE > objs[i]) {
                    fprintf(stderr, "Test 2 Failed: Objects %d and %d overlap\n", i, j);
                    exit(1);
                }
            }
        }
    }

    printf("Test 2 Passed: No overlapping allocations detected\n");

    // Clean up
    for (i = 0; i < OBJECTS; i++) {
        free(objs[i]);
    }
}

void test_data_integrity() {
    printf("Test 3: Data Integrity\n");

    char *objs[OBJECTS];
    int i, j, errors = 0;

    // Allocate objects
    for (i = 0; i < OBJECTS; i++) {
        objs[i] = malloc(OBJSIZE);
        if (objs[i] == NULL) {
            fprintf(stderr, "Test 3 Failed: Unable to allocate object %d\n", i);
            exit(1);
        }
    }

    // Fill each object with a distinct byte pattern
    for (i = 0; i < OBJECTS; i++) {
        memset(objs[i], i, OBJSIZE);
    }

    // Verify that data remains intact
    for (i = 0; i < OBJECTS; i++) {
        for (j = 0; j < OBJSIZE; j++) {
            if (objs[i][j] != i) {
                errors++;
                fprintf(stderr, "Test 3 Failed: Object %d byte %d incorrect: %d\n", i, j, objs[i][j]);
            }
        }
    }

    if (errors == 0) {
        printf("Test 3 Passed: Data integrity maintained\n");
    } else {
        printf("Test 3 Failed: %d incorrect bytes\n", errors);
    }

    // Clean up
    for (i = 0; i < OBJECTS; i++) {
        free(objs[i]);
    }
}

void test_coalescing() {
    printf("Test 3: Coalescing of Free Blocks\n");

    char *objs[4];
    int i;

    // Allocate four objects
    for (i = 0; i < 4; i++) {
        objs[i] = malloc(OBJSIZE);
        if (objs[i] == NULL) {
            fprintf(stderr, "Test 3 Failed: malloc() returned NULL at iteration %d\n", i);
            exit(1);
        }
    }

    // Free the middle two objects to create adjacent free blocks
    free(objs[1]);
    free(objs[2]);

    // Now attempt to allocate a larger object that requires the combined space
    size_t large_size = 2 * (OBJSIZE + HEADERSIZE) - HEADERSIZE;
    char *large_obj = malloc(large_size);
    if (large_obj == NULL) {
        fprintf(stderr, "Test 3 Failed: Unable to allocate large object after coalescing\n");
        exit(1);
    } else {
        printf("Test 3 Passed: Coalescing successful, large object allocated\n");
    }

    // Clean up
    free(objs[0]);
    free(objs[3]);
    free(large_obj);
}

void test_leak_detection() {
    printf("Test 4: Leak Detection\n");

    char *objs[OBJECTS];
    int i;

    // Allocate objects and intentionally not free them
    for (i = 0; i < OBJECTS; i++) {
        objs[i] = malloc(OBJSIZE);
        if (objs[i] == NULL) {
            fprintf(stderr, "Test 4 Failed: Unable to allocate object %d\n", i);
            exit(1);
        }
    }

    printf("Test 4: Allocated memory without freeing to test leak detection\n");

    // Leak detector should report the leaked memory upon program exit
    // No need to free objs[]
}

void test_free_invalid_pointer();
void test_free_not_start_of_chunk();
void test_double_free();

int test_error_detection() {
    // Run each test and print the result
    printf("Running test: Freeing invalid pointer...\n");
    test_free_invalid_pointer();

    printf("Running test: Freeing pointer not at start of chunk...\n");
    test_free_not_start_of_chunk();

    printf("Running test: Double free...\n");
    test_double_free();

    return 0;
}

//Calling free() with an address not obtained from malloc()
void test_free_invalid_pointer() {
    int x;  // x is not allocated via malloc
    printf("Attempting to free an invalid pointer...\n");
    free(&x);  // This should trigger an error and exit
}

//Calling free() with an address not at the start of a chunk
void test_free_not_start_of_chunk() {
    int *p = (int *)malloc(sizeof(int) * 2);  // Allocate space for 2 ints
    printf("Attempting to free pointer not at the start of the allocated chunk...\n");
    free(p + 1);  // This should trigger an error and exit
}

//Calling free() a second time on the same pointer
void test_double_free() {
    int *p = (int *)malloc(sizeof(int) * 100);  // Allocate space for 100 ints
    printf("Attempting to free the same pointer twice...\n");
    free(p);  // First free, should succeed
    free(p);  // Second free, should trigger an error and exit
}

int main(int argc, char **argv) {
    printf("Starting memory allocation tests...\n");

	test_default();

    // Test 1: Allocation of Unallocated Memory
    test_allocation();

    // Test 2: Non-overlapping Allocations
    test_non_overlapping_allocations();

    // Test 3: Coalescing of Free Blocks
    test_coalescing();

	// Test 4: Leak Detection
    test_leak_detection();
	
	printf("All tests completed.\n");

	//User chooses error detection (Implement)

    // Test 5: Error Detection
    test_error_detection();

    

    return EXIT_SUCCESS;
}