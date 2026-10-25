#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * ============================================================================
 * MODULE 17: Real-World File Format Parser (BMP Image Decoder)
 * ============================================================================
 * Demonstrates real-world binary file format parsing:
 *   - BITMAPFILEHEADER (14 bytes, packed)
 *   - BITMAPINFOHEADER (40 bytes)
 *   - Image stride and pixel array allocation
 *
 * Vulnerability Pattern:
 *   When calculating row stride and total pixel buffer size:
 *     total_bytes = height * stride;
 *   Large dimensions cause a 32-bit multiplication wrap, allocating an
 *   undersized heap buffer, followed by reading full rows from the file!
 */

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;      // Magic bytes: 0x4D42 ("BM")
    uint32_t bfSize;      // File size in bytes
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;   // Offset to pixel data
} BMPFileHeader;

typedef struct {
    uint32_t biSize;          // Size of info header (40 bytes)
    int32_t  biWidth;         // Image width in pixels
    int32_t  biHeight;        // Image height in pixels
    uint16_t biPlanes;        // Must be 1
    uint16_t biBitCount;      // Bits per pixel (e.g. 24 for RGB)
    uint32_t biCompression;   // 0 = uncompressed
    uint32_t biSizeImage;     // Image data size
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

// 1. GENERATE A VALID 8x8 SAMPLE BMP IMAGE
void create_sample_bmp(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    int width = 8;
    int height = 8;
    int row_stride = ((width * 3 + 3) / 4) * 4; // 24 bytes per row
    uint32_t pixel_bytes = row_stride * height;

    BMPFileHeader file_hdr = {
        .bfType = 0x4D42, // "BM"
        .bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + pixel_bytes,
        .bfReserved1 = 0,
        .bfReserved2 = 0,
        .bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
    };

    BMPInfoHeader info_hdr = {
        .biSize = sizeof(BMPInfoHeader),
        .biWidth = width,
        .biHeight = height,
        .biPlanes = 1,
        .biBitCount = 24,
        .biCompression = 0,
        .biSizeImage = pixel_bytes,
        .biXPelsPerMeter = 2835,
        .biYPelsPerMeter = 2835,
        .biClrUsed = 0,
        .biClrImportant = 0
    };

    fwrite(&file_hdr, sizeof(file_hdr), 1, f);
    fwrite(&info_hdr, sizeof(info_hdr), 1, f);

    // Write red/green pixel data
    uint8_t *row = (uint8_t *)calloc(1, row_stride);
    for (int x = 0; x < width * 3; x += 3) {
        row[x + 0] = 0x00; // Blue
        row[x + 1] = 0xFF; // Green
        row[x + 2] = 0x00; // Red
    }

    for (int y = 0; y < height; y++) {
        fwrite(row, 1, row_stride, f);
    }

    free(row);
    fclose(f);
}

// 2. PARSE BMP FILE
int parse_bmp(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return -1;
    }

    BMPFileHeader file_hdr;
    BMPInfoHeader info_hdr;

    if (fread(&file_hdr, sizeof(file_hdr), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (file_hdr.bfType != 0x4D42) {
        fprintf(stderr, "[-] Invalid file magic: 0x%04x (expected 0x4D42 'BM')\n", file_hdr.bfType);
        fclose(f);
        return -1;
    }

    if (fread(&info_hdr, sizeof(info_hdr), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    printf("[+] Valid BMP Header Detected:\n");
    printf("    Dimensions   : %d x %d pixels\n", info_hdr.biWidth, info_hdr.biHeight);
    printf("    Color Depth  : %d bits/pixel\n", info_hdr.biBitCount);
    printf("    Pixel Offset : %u bytes\n", file_hdr.bfOffBits);

    if (info_hdr.biBitCount != 24 || info_hdr.biCompression != 0) {
        printf("[-] Unsupported format (only 24-bit uncompressed RGB supported).\n");
        fclose(f);
        return -1;
    }

    // Calculate row stride (rows are padded to 4-byte boundaries)
    int32_t width = info_hdr.biWidth;
    int32_t height = (info_hdr.biHeight < 0) ? -info_hdr.biHeight : info_hdr.biHeight;
    uint32_t row_stride = ((width * 3 + 3) / 4) * 4;

    // VULNERABLE MULTIPLICATION:
    // If height * row_stride overflows 32-bit uint32_t, allocation is undersized!
    uint32_t total_pixel_bytes = row_stride * (uint32_t)height;

    printf("[*] Calculated Allocation Size: %u bytes (Stride: %u bytes)\n", total_pixel_bytes, row_stride);

    uint8_t *pixel_buffer = (uint8_t *)malloc(total_pixel_bytes);
    if (!pixel_buffer) {
        fprintf(stderr, "[-] malloc failed.\n");
        fclose(f);
        return -1;
    }

    fseek(f, file_hdr.bfOffBits, SEEK_SET);

    // Read pixel rows into buffer
    for (int32_t y = 0; y < height; y++) {
        uint8_t *target_row = pixel_buffer + (y * row_stride);
        size_t read = fread(target_row, 1, row_stride, f);
        if (read < row_stride) break;
    }

    printf("[+] Successfully decoded %d image rows.\n", height);

    free(pixel_buffer);
    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Real-World BMP Image Format Decoder ===\n\n");

    const char *target = "sample.bmp";

    if (argc > 1 && strcmp(argv[1], "--generate") == 0) {
        printf("[*] Generating valid sample image: %s\n", target);
        create_sample_bmp(target);
        printf("[+] Generated.\n\n");
    }

    if (argc > 1 && strcmp(argv[1], "--generate") != 0) {
        target = argv[1];
    } else {
        create_sample_bmp(target);
    }

    return parse_bmp(target);
}
