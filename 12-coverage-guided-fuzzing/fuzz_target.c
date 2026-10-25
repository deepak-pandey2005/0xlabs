#include <stdint.h>
#include <stddef.h>
#include "packet_parser.h"

/*
 * ============================================================================
 * LibFuzzer Harness (Fuzz Target)
 * ============================================================================
 * Guidelines for writing effective fuzz targets:
 *   1. Must accept arbitrary (Data, Size) streams.
 *   2. Must be deterministic (same input produces same execution paths).
 *   3. Must be fast and in-process (avoid disk I/O or persistent state).
 *   4. Must not exit or terminate on benign inputs.
 */

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Pass the fuzzer-generated mutant byte stream directly to the parser
    parse_packet(Data, Size);
    return 0; // Non-zero return values are reserved by libFuzzer
}
