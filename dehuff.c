#include "bitreader.h"
#include "bitwriter.h"
#include "node.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//format
//shitformat3

#define OPTIONS "i:o:h"

void dehuff_decompress_file(FILE *fout, BitReader *inbuf) {
    // Read header information
    uint8_t type1 = bit_read_uint8(inbuf);
    uint8_t type2 = bit_read_uint8(inbuf);
    uint32_t filesize = bit_read_uint32(inbuf);
    uint16_t num_leaves = bit_read_uint16(inbuf);

    // Verify header information
    assert(type1 == 'H');
    assert(type2 == 'C');

    // Calculate the number of nodes in the code tree
    uint32_t num_nodes = 2 * num_leaves - 1;

    // Initialize stack and top-of-stack pointer
    Node *stack[64]; // Assuming 64 is the maximum number of nodes
    int top = -1;

    // Build the code tree
    for (uint32_t i = 0; i < num_nodes; i++) {
        uint8_t bit = bit_read_bit(inbuf);
        Node *node;
        if (bit == 1) {
            uint8_t symbol = bit_read_uint8(inbuf);
            node = node_create(symbol, 0);
        } else {
            node = node_create(0, 0);
            node->right = stack[top--];
            node->left = stack[top--];
        }
        stack[++top] = node; // Push the node onto the stack
    }

    Node *code_tree = stack[top--]; // Pop the code tree from the stack

    // Decompress the file
    for (uint32_t i = 0; i < filesize; i++) {
        Node *node = code_tree;
        // Traverse the code tree until reaching a leaf node
        while (1) {
            uint8_t bit = bit_read_bit(inbuf);
            if (bit == 0) {
                node = node->left;
            } else {
                node = node->right;
            }
            if (node->left == NULL && node->right == NULL) {
                // Leaf node reached, write the symbol to the output file
                fputc(node->symbol, fout);
                break;
            }
        }
    }
}

int main(int argc, char **argv) {
    int opt = 0;
    char *infile = NULL;
    char *outfile = NULL;

    // Parse command line options
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = optarg; break;
        case 'o': outfile = optarg; break;
        case 'h':
            // Display help message and exit
            printf("dehuff: -i option is required\nUsage: dehuff -i infile -o outfile\n       "
                   "dehuff -v -i infile -o outfile\n       dehuff -h\n");
            return 0;
        }
    }

    // Check if input and output files are provided
    if (infile == NULL || outfile == NULL) {
        printf("dehuff:  -i option is required.\n");
        printf("Usage: dehuff -i infile -o outfile\n");
        printf("       dehuff -v -i infile -o outfile\n");
        printf("       dehuff -h\n");

        return 1;
    }

    // Open input file
    FILE *fin = fopen(infile, "rb");
    if (!fin) {
        printf("Failed to open input file: %s\n", infile);
        return 1;
    }

    // Open output file
    FILE *fout = fopen(outfile, "wb");
    if (!fout) {
        fclose(fin);
        printf("Failed to open output file: %s\n", outfile);
        return 1;
    }

    // Open input buffer
    BitReader *inbuf = bit_read_open(infile);

    if (!inbuf) {
        fclose(fin);
        fclose(fout);
        printf("Failed to open input buffer.\n");
        return 1;
    }

    // Decompress the file
    dehuff_decompress_file(fout, inbuf);

    // Clean up
    bit_read_close(&inbuf);
    //free(&node);
    fclose(fin);
    fclose(fout);

    return 0;
}
