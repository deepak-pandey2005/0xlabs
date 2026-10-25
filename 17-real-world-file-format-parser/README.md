# Module 17: Real-World File Format Parser (BMP Image Decoder)

File format parsers (image decoders, media containers, document formats) represent one of the largest attack surfaces in operating systems, browsers, and mobile devices.

This module explores binary file format decoding using a realistic, functional **BMP (Bitmap) image decoder**, demonstrating how arithmetic bugs during header decoding lead to critical heap vulnerabilities.

---

## 1. Anatomy of the BMP File Format

A standard Windows/OS2 Bitmap consists of consecutive binary structures:

```
+-----------------------------------------------------------+
| BITMAPFILEHEADER (14 bytes)                               |
|   - bfType      : 0x4D42 ("BM" ASCII signature)           |
|   - bfSize      : Total file size                         |
|   - bfOffBits   : Byte offset from file start to pixels   |
+-----------------------------------------------------------+
| BITMAPINFOHEADER (40 bytes)                               |
|   - biWidth     : Image width in pixels                   |
|   - biHeight    : Image height in pixels                  |
|   - biBitCount  : Color depth (e.g. 24 bits for BGR)      |
+-----------------------------------------------------------+
| Color Table / Palette (Optional, used if <= 8 bpp)        |
+-----------------------------------------------------------+
| Pixel Array (Raster Data)                                 |
|   - Stride      : Row bytes aligned to 4-byte boundaries  |
|     stride = ((biWidth * biBitCount + 31) / 32) * 4       |
+-----------------------------------------------------------+
```

---

## 2. The Vulnerability Pattern: Multiplicative Stride Wrap

When calculating how much memory to allocate for the pixel array:

```c
uint32_t row_stride = ((width * 3 + 3) / 4) * 4;
uint32_t total_pixel_bytes = row_stride * (uint32_t)height;

uint8_t *pixel_buffer = (uint8_t *)malloc(total_pixel_bytes);
```

### The Arithmetic Wrap
If an attacker crafts the image dimensions:
- `width = 100` $\implies$ `row_stride = 300` bytes.
- `height = 14,316,558` (`0x00DA740E`).
- Multiplication:
  $$300 \times 14,316,558 = 4,294,967,400 \equiv 104 \pmod{2^{32}}$$
- The decoder allocates only **104 bytes** on the heap (`malloc(104)`).
- When the row reading loop executes:
  ```c
  for (int32_t y = 0; y < height; y++) {
      uint8_t *target_row = pixel_buffer + (y * row_stride);
      fread(target_row, 1, row_stride, f); // Writes 300 bytes into a 104-byte buffer!
  }
  ```
- The very first row write spills far past the 104-byte chunk boundary into adjacent heap memory!

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Complete BMP Analysis Suite
```bash
make
./run_bmp_lab.sh
```

Follow the three phases:
1. **Valid Image Parsing:** Creates and successfully decodes a valid $8 \times 8$ sample bitmap (`sample.bmp`).
2. **Malicious Image Crafting:** Uses Python's `struct` module to pack a BMP header with the integer-overflow dimensions (`100 x 14,316,558`).
3. **AddressSanitizer Interception:** Parses the crafted image; AddressSanitizer halts execution immediately with an exact heap-buffer-overflow report:
   ```
   ==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x7bc53c9e00a8
   WRITE of size 300 at 0x7bc53c9e00a8 thread T0
       #1 in parse_bmp bmp_decoder.c:158
   0x7bc53c9e00a8 is located 0 bytes after 104-byte region
   ```

---

## 4. Production Defenses for File Parsers

1. **Hardware-Checked Multiplication:**
   ```c
   uint32_t total_bytes;
   if (__builtin_mul_overflow(row_stride, height, &total_bytes)) {
       return -1; // Integer wrap detected, reject file safely
   }
   ```
2. **Dimension Sanity Bounds:**
   Enforce strict upper limits on image resolution (e.g. `if (width > 16384 || height > 16384) return -1;`).
3. **File Size Consistency Check:**
   Verify that `file_hdr.bfSize >= file_hdr.bfOffBits + total_bytes`.
