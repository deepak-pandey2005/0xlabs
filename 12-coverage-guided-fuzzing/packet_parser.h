#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <stdint.h>
#include <stddef.h>

#define PACKET_MAGIC 0x31544B50 // "PKT1" in little-endian

typedef enum {
    TYPE_TEXT  = 0x01,
    TYPE_DATA  = 0x02,
    TYPE_PING  = 0x03
} packet_type_t;

// Parses an untrusted binary stream.
// Returns 0 on success, -1 on invalid format.
int parse_packet(const uint8_t *data, size_t size);

#endif // PACKET_PARSER_H
