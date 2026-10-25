#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet_parser.h"

/*
 * ============================================================================
 * MODULE 12: Target Application - Binary Packet Parser
 * ============================================================================
 * Packet Layout:
 *   [0..3] Magic Bytes ("PKT1")
 *   [4]    Type (0x01 = TEXT, 0x02 = DATA, 0x03 = PING)
 *   [5]    Flags
 *   [6..7] Payload Length (uint16_t, little-endian)
 *   [8..]  Payload bytes
 */

int parse_packet(const uint8_t *data, size_t size) {
    if (data == NULL || size < 8) {
        return -1; // Insufficient header bytes
    }

    // 1. Verify Magic Header
    uint32_t magic;
    memcpy(&magic, data, sizeof(magic));
    if (magic != PACKET_MAGIC) {
        return -1; // Reject unknown format
    }

    uint8_t type = data[4];
    size_t payload_len = size - 6;
    const uint8_t *payload = data + 6;


    if (type == TYPE_PING) {
        return 0; // Benign ping
    } else if (type == TYPE_DATA) {
        return 0;
    } else if (type == TYPE_TEXT) {
        // VULNERABILITY:
        // Allocates a 32-byte heap buffer, but allows payload_len up to 256 bytes!
        // AddressSanitizer detects heap-buffer-overflow reliably.
        if (payload_len > 256) {
            return -1;
        }

        char *text_buffer = (char *)malloc(32);
        if (!text_buffer) {
            return -1;
        }

        memcpy(text_buffer, payload, payload_len);

        // Process message: access last byte of copied payload
        // This prevents the compiler from optimizing away the allocation,
        // ensuring AddressSanitizer traps the out-of-bounds write/read!
        volatile char last_byte = text_buffer[payload_len - 1];
        (void)last_byte;

        free(text_buffer);
        return 0;
    }

    return -1;
}
