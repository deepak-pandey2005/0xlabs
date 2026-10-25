# Module 19: Embedded Network Protocol Daemon & HTTP Parser

Network daemons (such as Nginx, Apache, embedded IoT web servers, and API gateways) operate directly on untrusted network packets received over TCP/IP sockets.

This module explores real-world protocol tokenization, HTTP/1.1 message framing (RFC 7230), and how integer signedness bugs in header decoders create severe protocol vulnerabilities.

---

## 1. Anatomy of HTTP/1.1 Framing

A standard HTTP/1.1 request conforms to the following grammar:

```
[Method] [Request-URI] [HTTP-Version]\r\n     <-- Request Line
[Header-Name]: [Header-Value]\r\n            <-- Headers
...
\r\n                                         <-- Empty Line (CRLF CRLF)
[Message Body...]                            <-- Body (sized by Content-Length)
```

### The Role of `Content-Length`
The `Content-Length` header indicates the decimal number of octets (bytes) in the message body. In network protocols, parsing this field securely is critical for memory pre-allocation and request boundary detection (preventing Request Smuggling).

---

## 2. The Vulnerability: Signed Integer Header Parsing

Consider an embedded daemon parsing headers using standard C formatting:

```c
int content_length = 0;
sscanf(header_line, "Content-Length: %d", &content_length);

if (content_length < MAX_BODY_BUFFER) { // e.g. 256 bytes
    // Vulnerable copy!
    memcpy(body_buffer, body_data, (size_t)content_length);
}
```

### The Flaw
1. Using `%d` instructs `sscanf` to parse a **signed 32-bit integer**.
2. If an attacker supplies `Content-Length: -1`:
   - `content_length` evaluates to `-1`.
   - The comparison `-1 < 256` evaluates to **TRUE**!
3. When passed to `memcpy`:
   - The 3rd parameter of `memcpy` is `size_t` (an **unsigned 64-bit integer** on x86_64).
   - `-1` is cast to `(size_t)18446744073709551615ULL` ($2^{64}-1$ bytes).
   - The daemon immediately crashes or corrupts memory attempting an exabyte-wide copy.

---

## 3. RFC 7230 Compliant Defensive Parser

RFC 7230 Section 3.3.2 explicitly dictates:
> *Any Content-Length field-value containing characters other than digits is invalid. A recipient MUST reject any request with an invalid or negative Content-Length.*

### Defensive Implementation
```c
char *endptr = NULL;
errno = 0;
// Parse strictly as unsigned long with character verification
unsigned long cl = strtoul(val_str, &endptr, 10);

if (errno != 0 || endptr == val_str || *val_str == '-') {
    // Reject invalid / negative header
    return -1;
}

if (cl > MAX_BODY_BUFFER) {
    // Reject payload exceeding capacity
    return -1;
}
```

---

## 4. Hands-on Demonstrations

### Exercise 1: Run the Network Protocol Test Suite
```bash
make
./test_httpd.sh
```

Follow the three phases:
1. **Benign Request:** Valid `Content-Length: 16` passes and processes cleanly.
2. **Signedness Bug:** Demonstrates how `Content-Length: -1` bypasses `< 256` in the vulnerable parser.
3. **Hardened Parser:** Demonstrates the RFC 7230 compliant parser immediately rejecting negative values.
