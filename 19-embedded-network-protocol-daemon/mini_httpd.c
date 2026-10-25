#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>

/*
 * ============================================================================
 * MODULE 19: Embedded Network Protocol Daemon (Mini HTTP/1.1 Server)
 * ============================================================================
 * Network daemons process untrusted inputs over TCP sockets.
 * This module demonstrates:
 *   1. Socket lifecycle: socket(), bind(), listen(), accept().
 *   2. HTTP/1.1 request line and header tokenization.
 *   3. Real-world Protocol Vulnerability: Signed Content-Length parsing bug
 *      where a negative length passes sanity checks but expands during copy!
 *   4. RFC 7230 Compliant Defensive Parser: Bounded unsigned validation.
 */

#define MAX_REQ_SIZE 2048
#define BODY_BUFFER_SIZE 256

// 1. VULNERABLE REQUEST PROCESSOR
int process_http_request_vulnerable(const char *request_data, size_t data_len) {
    printf("[*] Parsing HTTP Request (%zu bytes)...\n", data_len);

    char method[16], uri[256], version[16];
    if (sscanf(request_data, "%15s %255s %15s", method, uri, version) < 3) {
        printf("[-] Malformed HTTP request line.\n");
        return -1;
    }

    printf("[+] Request Line: Method=%s, URI=%s, Version=%s\n", method, uri, version);

    // Scan for Content-Length header
    const char *cl_header = strstr(request_data, "Content-Length:");
    int content_length = 0;

    if (cl_header) {
        // VULNERABLE: %d parses a SIGNED integer!
        // If client sends "Content-Length: -1", content_length is negative!
        if (sscanf(cl_header, "Content-Length: %d", &content_length) == 1) {
            printf("[*] Parsed Content-Length: %d\n", content_length);

            // FLAWED CHECK: Only checks upper bound!
            // If content_length is -1, -1 < 256 is TRUE!
            if (content_length < BODY_BUFFER_SIZE) {
                printf("[+] Content-Length passed check: %d < %d\n", content_length, BODY_BUFFER_SIZE);

                // Locate start of body (\r\n\r\n)
                const char *body_start = strstr(request_data, "\r\n\r\n");
                if (body_start) {
                    body_start += 4;
                    char body_buffer[BODY_BUFFER_SIZE];

                    // Passing negative content_length to size_t parameter triggers
                    // massive unsigned integer expansion ((size_t)-1 = 18.4 Exabytes)!
                    printf("[!] WARNING: When passed to memcpy(..., (size_t)content_length):\n");
                    printf("    (size_t)(%d) = %zu bytes!\n", content_length, (size_t)content_length);

                    if (content_length > 0) {
                        memcpy(body_buffer, body_start, (size_t)content_length);
                    } else {
                        printf("[!] DEFENSIVE INTERCEPTION: Prevented actual crash for demonstration.\n");
                    }
                }
            } else {
                printf("[-] Payload rejected: Content-Length exceeds buffer capacity.\n");
                return -1;
            }
        }
    }

    return 0;
}

// 2. SECURE RFC 7230 PARSER
int process_http_request_secure(const char *request_data, size_t data_len) {
    printf("[*] [Secure Parser] Parsing HTTP Request safely...\n");

    const char *cl_header = strstr(request_data, "Content-Length:");
    if (cl_header) {
        const char *val_str = cl_header + 15;
        while (*val_str == ' ' || *val_str == '\t') val_str++;

        char *endptr = NULL;
        errno = 0;
        // DEFENSIVE: Use strtoul with explicit character validation
        unsigned long cl = strtoul(val_str, &endptr, 10);

        if (errno != 0 || endptr == val_str || *val_str == '-') {
            printf("[-] RFC 7230 Violation: Negative or invalid Content-Length rejected!\n");
            return -1;
        }

        if (cl > BODY_BUFFER_SIZE) {
            printf("[-] Rejected: Content-Length (%lu) exceeds maximum allowed (%d).\n", cl, BODY_BUFFER_SIZE);
            return -1;
        }

        printf("[+] Validated Content-Length: %lu bytes (In-Bounds).\n", cl);
    }

    (void)data_len;
    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Embedded Network Daemon & HTTP Parser ===\n\n");

    bool secure = false;
    const char *sample_req = 
        "POST /api/upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Length: -1\r\n"
        "\r\n"
        "hello payload";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--secure") == 0) {
            secure = true;
        } else {
            sample_req = argv[i];
        }
    }

    if (secure) {
        return process_http_request_secure(sample_req, strlen(sample_req));
    } else {
        return process_http_request_vulnerable(sample_req, strlen(sample_req));
    }
}
