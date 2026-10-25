#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * MODULE 20: Patch Diffing Target (Version 2.0 - Security Patched)
 * ============================================================================
 * Function: process_user_records
 * Patch: Added bounds guard 'if (count <= 0 || count > 16) return -1;'
 * before executing the copy loop.
 */

#define MAX_RECORDS 16

int process_user_records(const int *input_records, int count) {
    int storage[MAX_RECORDS];

    // SECURITY PATCH: Strict boundary and negativity validation
    if (count <= 0 || count > MAX_RECORDS) {
        return -1; // -EINVAL
    }

    for (int i = 0; i < count; i++) {
        storage[i] = input_records[i] * 2;
    }

    return storage[0];
}

int main(int argc, char *argv[]) {
    int sample[] = {1, 2, 3, 4};
    int count = (argc > 1) ? atoi(argv[1]) : 4;
    printf("[v2.0] Result: %d\n", process_user_records(sample, count));
    return 0;
}
