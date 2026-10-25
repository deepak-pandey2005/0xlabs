#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * MODULE 20: Patch Diffing Target (Version 1.0 - Unpatched)
 * ============================================================================
 * Function: process_user_records
 * Flaw: Accepts an unvalidated 'count' parameter without verifying that
 * count <= MAX_RECORDS (16), leading to an unconstrained stack write.
 */

int process_user_records(const int *input_records, int count) {
    int storage[16];

    // VULNERABLE: Copies 'count' elements directly without validation
    for (int i = 0; i < count; i++) {
        storage[i] = input_records[i] * 2;
    }

    return storage[0];
}

int main(int argc, char *argv[]) {
    int sample[] = {1, 2, 3, 4};
    int count = (argc > 1) ? atoi(argv[1]) : 4;
    printf("[v1.0] Result: %d\n", process_user_records(sample, count));
    return 0;
}
