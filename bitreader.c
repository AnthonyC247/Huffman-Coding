#include "bitreader.h"
//format
#include <stdio.h>
#include <stdlib.h>
//dumb moment ithink
struct BitReader {
    FILE *underlying_stream;
    uint8_t byte;
    uint8_t bit_position;
};

BitReader *bit_read_open(const char *filename) {
    BitReader *reader = (BitReader *) calloc(1, sizeof(BitReader));
    if (reader == NULL) {
        return NULL; // Memory allocation failed
    }

    reader->underlying_stream = fopen(filename, "rb"); // Open file in binary read mode
    if (reader->underlying_stream == NULL) {
        free(reader);
        return NULL; // Error opening file
    }

    // Initialize other fields if needed
    reader->byte = 0;
    reader->bit_position = 8; // Set bit_position to 8 to force reading the first byte

    return reader;
}

void bit_read_close(BitReader **pbuf) {
    if (*pbuf != NULL) {
        if ((*pbuf)->underlying_stream != NULL) {
            fclose((*pbuf)->underlying_stream);
        }
        free(*pbuf);
        *pbuf = NULL;
    }
}

uint8_t bit_read_bit(BitReader *buf) {
    if (buf->bit_position > 7) {
        int byte_read = fgetc(buf->underlying_stream);
        if (byte_read == EOF) {
            // Handle error or EOF condition
            // For now, let's return 0 as the default value
            return 0;
        }
        buf->byte = (uint8_t) byte_read;
        buf->bit_position = 0;
    }

    uint8_t bit = (buf->byte >> buf->bit_position) & 1;
    buf->bit_position++;
    return bit;
}

uint8_t bit_read_uint8(BitReader *buf) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (bit_read_bit(buf) << i);
    }
    return byte;
}

uint16_t bit_read_uint16(BitReader *buf) {
    uint16_t word = 0;
    for (int i = 0; i < 16; i++) {
        word |= ((uint16_t) bit_read_bit(buf) << i);
    }
    return word;
}

uint32_t bit_read_uint32(BitReader *buf) {
    uint32_t word = 0;
    for (int i = 0; i < 32; i++) {
        word |= ((uint32_t) bit_read_bit(buf) << i);
    }
    return word;
}
