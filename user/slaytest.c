#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_ALLOCS 50
#define MAX_PAGES_PER_ALLOC 16
#define ITERATIONS 2

// Simple random number generator
unsigned int next_rand = 1;
int rand() {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}

// Function to draw memory map to visualize fragmentation
void draw_memory_map(void *ptrs[], int counts) {
    printf("\n--- Visual Memory Map (Allocation Distribution) ---\n");
    printf("Legend: [O] Allocated Block | [.] Empty/Freed Slot\n\n");

    for (int i = 0; i < counts; i++) {
        if (ptrs[i] != 0) {
            printf("[O]"); // Block is currently occupied
        } else {
            printf("[.]"); // Block has been freed
        }

        // New line every 25 blocks for a neat layout
        if ((i + 1) % 25 == 0) {
            printf("\n");
        }
    }
    printf("\n--------------------------------------------------\n\n");
}

void random_stress_test(int iter) {
    void *ptrs[MAX_ALLOCS];
    
    printf("--- [Round %d] Starting Random Stress Test ---\n", iter);

    // Step 1: Random Allocation
    for (int i = 0; i < MAX_ALLOCS; i++) {
        int num_pages = (rand() % MAX_PAGES_PER_ALLOC) + 1;
        int bytes = num_pages * 4096;
        
        ptrs[i] = malloc(bytes);
        if (ptrs[i] == 0) {
            printf("[%d] Allocation failed: %d bytes\n", i, bytes);
            continue;
        }
        
        // Fill with sample data to verify integrity later
        memset(ptrs[i], (i + iter) % 255, bytes);
        
        if (i % 10 == 0) printf("  Allocated %d blocks...\n", i);
    }

    // Draw map when memory is at its peak (random sizes)
    printf("\nMemory status after random allocation:");
    draw_memory_map(ptrs, MAX_ALLOCS);

    // Step 2: Verify data integrity
    printf("  Checking data integrity...\n");
    for (int i = 0; i < MAX_ALLOCS; i++) {
        if (ptrs[i] != 0) {
            unsigned char *p = (unsigned char *)ptrs[i];
            if (p[0] != ((i + iter) % 255)) {
                printf("  ERROR: Data at block %d is corrupted!\n", i);
            }
        }
    }

    // Step 3: Random deallocation to force Buddy merging
    printf("  Randomly freeing to create fragmentation...\n");
    int freed_count = 0;
    int attempted = 0;
    while (freed_count < MAX_ALLOCS / 2 && attempted < MAX_ALLOCS * 5) {
        int idx = rand() % MAX_ALLOCS;
        if (ptrs[idx] != 0) {
            free(ptrs[idx]);
            ptrs[idx] = 0;
            freed_count++;
        }
        attempted++;
    }

    // Draw map after 50% deallocation to see random fragmentation
    printf("\nMemory status after freeing 50%% (Fragmentation):");
    draw_memory_map(ptrs, MAX_ALLOCS);
    
    // Clean up remaining blocks
    for(int i = 0; i < MAX_ALLOCS; i++){
        if(ptrs[i] != 0){
            free(ptrs[i]);
            ptrs[i] = 0;
        }
    }

    printf("--- [Round %d] SUCCESS ---\n\n", iter);
}

int main(int argc, char *argv[]) {
    printf("MEMTEST: Starting Stress Test with memory map visualization.\n");
    
    for(int i = 1; i <= ITERATIONS; i++) {
        random_stress_test(i);
    }
    
    printf("MEMTEST: All tests completed.\n");
    exit(0);
}

