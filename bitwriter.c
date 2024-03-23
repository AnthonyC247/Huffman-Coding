#include "bitwriter.h"
//format3
#include <stdio.h>
#include <stdlib.h>

struct BitWriter {
    FILE *underlying_stream;
    uint8_t byte;
    uint8_t bit_position;
};

BitWriter *bit_write_open(const char *filename) {
    BitWriter *f = calloc(1, sizeof(BitWriter));
    f->underlying_stream = fopen(filename, "wb"); // Open file for writing in binary mode
    if (f->underlying_stream == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        free(f);
        return NULL;
    }
    return f;
}

void bit_write_close(BitWriter **pbuf) {
    if (*pbuf != NULL) {
        //if ((*pbuf)->underlying_stream != NULL) {
        if ((*pbuf)->bit_position > 0) {
            fputc((*pbuf)->byte, (*pbuf)->underlying_stream);
        }
        //fclose((*pbuf)->underlying_stream);
    }
    fclose((*pbuf)->underlying_stream);
    free(*pbuf);
    *pbuf = NULL;
}

void bit_write_bit(BitWriter *buf, uint8_t bit) {
    if (buf->bit_position > 7) {
        fputc(buf->byte, buf->underlying_stream);
        buf->byte = 0x00;
        buf->bit_position = 0;
    }

    if (bit & 1) {
        buf->byte |= (bit & 1) << buf->bit_position;
    }

    buf->bit_position++;
}

void bit_write_uint8(BitWriter *buf, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        bit_write_bit(buf, (byte >> i) & 1);
    }
}

void bit_write_uint16(BitWriter *buf, uint16_t x) {
    for (int i = 0; i < 16; i++) {
        bit_write_bit(buf, (x >> i) & 1);
    }
}

void bit_write_uint32(BitWriter *buf, uint32_t x) {
    for (int i = 0; i < 32; i++) {
        bit_write_bit(buf, (x >> i) & 1);
    }
}
