#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("   MEMORY ALLOCATOR TEST\n");
    printf("========================================\n");
    
    printf("\n[TEST 1] Allocating various sizes...\n");
    char *ptrs[10];
    int sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 100, 500};
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(sizes[i]);
        if (ptrs[i] == 0) {
            printf("  [FAIL] malloc(%d) failed!\n", sizes[i]);
        } else {
            printf("  [OK] malloc(%d) = %p\n", sizes[i], ptrs[i]);
        }
    }
    
    // ... rest of tests
    
    exit(0);
}