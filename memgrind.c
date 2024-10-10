#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define NUM_ITERATIONS 60
#define NUM_RUNS 50

//Test Test 1: malloc() and immediately free() a 1-byte object, 60 times
void test_case_1() {
    struct timeval start, end;
    long total_time = 0;
    printf("Test 1:\n");

    for (int run = 0; run < NUM_RUNS; run++) {
        gettimeofday(&start, NULL);

        for (int i = 0; i < NUM_ITERATIONS; i++) {
            char *ptr = malloc(1);
            if (ptr == NULL) {
                fprintf(stderr, "Test 1 Failed: malloc() returned NULL at iteration %d\n", i);
                exit(1);
            }
            free(ptr);
        }

        gettimeofday(&end, NULL);

        // Calculate total elapsed time
        total_time += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    double average_time = total_time / (double)NUM_RUNS;
    printf("Test 1 Completed: Average time per run: %.2f microseconds\n", average_time);
}

//Test Test 2: Use malloc() to get 60 1-byte objects, storing the pointers in an array, then use free() to deallocate the chunks.
void test_case_2() {
    struct timeval start, end;
    long total_time = 0;
    printf("Test 2:\n");

    for (int run = 0; run < NUM_RUNS; run++) {
        char *ptrs[NUM_ITERATIONS] = {NULL};

        gettimeofday(&start, NULL);

        // Allocate 120 1-byte objects
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            ptrs[i] = malloc(1);
            if (ptrs[i] == NULL) {
                fprintf(stderr, "Test 2 Failed: malloc() returned NULL at iteration %d\n", i);
                exit(1);
            }
        }

        // Free the allocated objects
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            free(ptrs[i]);
        }

        gettimeofday(&end, NULL);

        // Calculate total elapsed time
        total_time += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    double average_time = total_time / (double)NUM_RUNS;
    printf("Test 2 Completed: Average time per run: %.2f microseconds\n", average_time);
}

//Test Case 3: Create an array of 60 pointers. Repeatedly make a random choice between allocating a 1-byte object and adding the pointer to the array and
//deallocating a previously allocated object (if any), until you have allocated 60 times. Deallocate any remaining objects.
void test_case_3() {
    struct timeval start, end;
    long total_time = 0;
    printf("Test 3:\n");

    for (int run = 0; run < NUM_RUNS; run++) {
        char *ptrs[NUM_ITERATIONS] = {NULL};
        int allocations = 0;
        int deallocations = 0;
        int total_ops = 0; // Total operations performed
        srand((unsigned int)time(NULL) + run); // Seed random number generator differently each run

        gettimeofday(&start, NULL);

        while (allocations < NUM_ITERATIONS) {
            int action = rand() % 2; // Randomly choose between 0 (allocate) and 1 (free)
            if (action == 0) {
                // Allocate if there's space
                if (allocations < NUM_ITERATIONS) {
                    // Find the next available slot
                    int index = 0;
                    while (index < NUM_ITERATIONS && ptrs[index] != NULL) {
                        index++;
                    }
                    if (index < NUM_ITERATIONS) {
                        ptrs[index] = malloc(1);
                        if (ptrs[index] == NULL) {
                            fprintf(stderr, "Test 3 Failed: malloc() returned NULL at allocation %d\n", allocations);
                            exit(1);
                        }
                        allocations++;
                        total_ops++;
                    }
                }
            } else {
                // Free a previously allocated object
                if (allocations > deallocations) {
                    // Find an allocated slot
                    int index = 0;
                    while (index < NUM_ITERATIONS && ptrs[index] == NULL) {
                        index++;
                    }
                    if (index < NUM_ITERATIONS && ptrs[index] != NULL) {
                        free(ptrs[index]);
                        ptrs[index] = NULL;
                        deallocations++;
                        total_ops++;
                    }
                }
            }
        }

        // Free any remaining allocated objects
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            if (ptrs[i] != NULL) {
                free(ptrs[i]);
                ptrs[i] = NULL;
                deallocations++;
            }
        }

        gettimeofday(&end, NULL);

        // Calculate total elapsed time
        total_time += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    double average_time = total_time / (double)NUM_RUNS;
    printf("Test 3 Completed: Average time per run: %.2f microseconds\n", average_time);
}

// Test Case 4: Repeated Allocation and Deallocation with Random Sizes
void test_case_4() {
    struct timeval start, end;
    long total_time = 0;
    const int MAX_ALLOC_SIZE = 64; // Maximum size for allocations

    printf("Case 4:\n");

    for (int run = 0; run < NUM_RUNS; run++) {
        char *ptrs[NUM_ITERATIONS];
        int sizes[NUM_ITERATIONS];
        srand((unsigned int)(time(NULL) + run)); // Different seed for variability

        // Generate random sizes between 1 and 64 bytes
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            sizes[i] = (rand() % MAX_ALLOC_SIZE) + 1;
        }

        // start time
        gettimeofday(&start, NULL);

        // Allocate memory blocks with random sizes
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            ptrs[i] = malloc(sizes[i]);
            if (ptrs[i] == NULL) {
                fprintf(stderr, "Test 4 Failed: malloc() returned NULL at iteration %d, run %d\n", i, run + 1);
                exit(1);
            }
            // Optionally initialize allocated memory
            // memset(ptrs[i], 0, sizes[i]);
        }

        // Free all allocated memory blocks sequentially
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }

        // Capture end time
        gettimeofday(&end, NULL);

        // Calculate total elapsed time
        total_time += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    double average_time = total_time / (double)NUM_RUNS;
    printf("Test 4 Completed: Average time per run (Random Sizes): %.2f microseconds\n", average_time);
}

// Test Case 5: Allocating max memory
void test_case_5() {
    struct timeval start, end;
    long total_time = 0;
    const size_t FIXED_ALLOC_SIZE = 8; // Allocate 8 bytes

    printf("Test 5:\n");

    for (int run = 0; run < NUM_RUNS; run++) {
        char *ptrs[128];

        gettimeofday(&start, NULL);

        for (int i = 0; i < 128; i++) {
            ptrs[i] = malloc(FIXED_ALLOC_SIZE);
            if (ptrs[i] == NULL) {
                fprintf(stderr, "Test 5 Failed: malloc() returned NULL at iteration %d, run %d\n", i, run + 1);
                exit(1);
            }

        }

        // Free all allocated objects
        for (int i = 0; i < 128; i++) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }

        // Capture end time
        gettimeofday(&end, NULL);

        // Calculate total elapsed time
        total_time += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    double average_time = total_time / (double)NUM_RUNS;
    printf("Test 5 Completed: Average time per run (Max Memory Allocation): %.2f microseconds\n", average_time);
}

int main() {
    printf("Starting performance tests with custom malloc and free:\n");

    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4(); //Allocate and deallocate with random sizes
    test_case_5(); //Allocate max memory into heap
    return 0;
}
