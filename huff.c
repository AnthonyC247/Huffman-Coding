#include "bitwriter.h"
#include "node.h"
#include "pq.h"

#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OPTIONS "i:o:h"

// Format


typedef struct {
    uint64_t code;
    uint8_t code_length;
} Code;

/* 
 * Function: fill_histogram
 * -------------------------
 * Updates a histogram array with the number of occurrences of each byte value in the input file.
 * Also returns the total size of the input file.
 *
 * fin: FILE pointer to the input file
 * histogram: Array to store the occurrences of each byte value
 *
 * Returns: Total size of the input file
 */

uint32_t fill_histogram(FILE *fin, uint32_t *histogram) {
    uint32_t filesize = 0;
    int c;
    while ((c = fgetc(fin)) != EOF) {
        ++histogram[c];
        ++filesize;
    }
    fseek(fin, 0, SEEK_SET);
    // Hack to ensure at least two non-zero values in the histogram
    ++histogram[0x00];
    ++histogram[0xff];
    return filesize;
}

/* 
 * Function: create_tree
 * ---------------------
 * Creates a Huffman tree from the histogram and returns the root node of the tree.
 * Also updates the number of leaf nodes in the tree.
 *
 * histogram: Array containing the occurrences of each byte value
 * num_leaves: Pointer to store the number of leaf nodes in the tree
 *
 * Returns: Root node of the Huffman tree
 */
Node *create_tree(uint32_t *histogram, uint16_t *num_leaves) {
    PriorityQueue *pq = pq_create();
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] != 0) {
            Node *node = node_create(i, histogram[i]);
            enqueue(pq, node); // Enqueue node into priority queue
            ++(*num_leaves);
        }
    }
    // Huffman coding algorithm
    while (!pq_is_empty(pq) && !pq_size_is_1(pq)) {
        Node *left = dequeue(pq); // Dequeue left node
        Node *right = dequeue(pq); // Dequeue right node
        if (left != NULL && right != NULL) {
            Node *parent = node_create(0, left->weight + right->weight);
            if (parent != NULL) {
                parent->left = left;
                parent->right = right;
                enqueue(pq, parent); // Enqueue parent node
            } else {
                // Handle memory allocation failure
                // Free 'left' and 'right' nodes
                node_free(&left);
                node_free(&right);
                node_free(&parent);
                // Optionally, break out of the loop or return an error
                break;
            }
        } else {
            // Handle error condition where one of the nodes is NULL
            if (left != NULL) {
                node_free(&left);
            }
            if (right != NULL) {
                node_free(&right);
            }

            // Optionally, break out of the loop or return an error
            break;
        }
    }
    Node *root = dequeue(pq); // Dequeue root of Huffman tree
    pq_free(&pq);
    return root;
}

/* 
 * Function: fill_code_table
 * -------------------------
 * Recursively fills in the code table with Huffman codes for each symbol in the tree.
 *
 * code_table: Array to store the Huffman codes for each symbol
 * node: Current node being traversed in the Huffman tree
 * code: Huffman code being constructed for the current symbol
 * code_length: Length of the Huffman code
 */
void fill_code_table(Code *code_table, Node *node, uint64_t code, uint8_t code_length) {
    if (node->left == NULL) {
        code_table[node->symbol].code = code;
        code_table[node->symbol].code_length = code_length;
    } else {
        fill_code_table(code_table, node->left, code, code_length + 1);
        fill_code_table(
            code_table, node->right, code | ((uint64_t) 1 << code_length), code_length + 1);
    }
}

// The recursive function to write the tree to BitWriter
void huff_write_tree(BitWriter *outbuf, Node *node) {
    if (node->left == NULL) {
        // Leaf node
        bit_write_bit(outbuf, 1);
        bit_write_uint8(outbuf, node->symbol);
    } else {
        // Internal node
        huff_write_tree(outbuf, node->left);
        huff_write_tree(outbuf, node->right);
        bit_write_bit(outbuf, 0);
    }
}

/* 
 * Function: huff_compress_file
 * ----------------------------
 * Compresses the input file using Huffman coding and writes the compressed data to the output file.
 *
 * outbuf: BitWriter pointer for writing compressed data to the output file
 * fin: FILE pointer to the input file
 * filesize: Size of the input file
 * num_leaves: Number of leaf nodes in the Huffman tree
 * code_tree: Root node of the Huffman tree
 * code_table: Array containing the Huffman codes for each symbol
 */
void huff_compress_file(BitWriter *outbuf, FILE *fin, uint32_t filesize, uint16_t num_leaves,
    Node *code_tree, Code *code_table) {

    bit_write_uint8(outbuf, 'H');
    bit_write_uint8(outbuf, 'C');

    // Write the filesize and num_leaves to the output buffer
    bit_write_uint32(outbuf, filesize);
    bit_write_uint16(outbuf, num_leaves);

    // Write the code tree to the output buffer
    huff_write_tree(outbuf, code_tree);

    // Write the Huffman coded file
    while (true) {
        int b = fgetc(fin);
        if (b == EOF) {
            break;
        }
        uint64_t code = code_table[b].code;
        uint8_t code_length = code_table[b].code_length;
        for (int i = 0; i < code_length; i++) {
            // Write each bit of the Huffman code to the output buffer
            bit_write_bit(outbuf, (code >> i) & 1);
        }
    }
}

/* 
 * Function: main
 * --------------
 * Entry point of the program.
 */
int main(int argc, char **argv) {
    char *infile = NULL;
    char *outfile = NULL;
    int i = 0;
    int o = 0;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            infile = optarg;
            i = 1;
            break;
        case 'o':
            outfile = optarg;
            o = 1;
            break;
        case 'h':
            printf("Usage: huff -i infile -o outfile\n");
            printf("       huff -v -i infile -o outfile\n");
            printf("       huff -h\n");
            return EXIT_SUCCESS;
        }
    }

    // Check if both input and output files are provided
    if (i == 0 || o == 0) {
        fprintf(stderr, "huff: -i and -o options are required\n");
        fprintf(stderr, "Usage: %s -i infile -o outfile\n", basename(argv[0]));
        return EXIT_FAILURE;
    }

    // Initialize histogram
    uint32_t histogram[256] = { 0 };

    // Open input file
    FILE *fin = fopen(infile, "rb");
    if (!fin) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    // Fill histogram and get filesize
    uint32_t filesize = fill_histogram(fin, histogram);
    // Create Huffman tree
    uint16_t num_leaves = 0;
    Node *code_tree = create_tree(histogram, &num_leaves);

    // Fill code table
    Code code_table[256];
    fill_code_table(code_table, code_tree, 0, 0);

    // Open output file
    FILE *fout = fopen(outfile, "wb");
    if (!fout) {
        perror("Error opening output file");
        fclose(fin);
        return EXIT_FAILURE;
    }

    // Create bit writer
    BitWriter *outbuf = bit_write_open(outfile);
    if (!outbuf) {
        perror("Error opening output bit stream");
        fclose(fin);
        fclose(fout);
        return EXIT_FAILURE;
    }

    // Compress file using Huffman coding
    huff_compress_file(outbuf, fin, filesize, num_leaves, code_tree, code_table);

    // Cleanup
    fclose(fin);
    fclose(fout);
    bit_write_close(&outbuf);
    node_free(&code_tree);

    return EXIT_SUCCESS;
}
