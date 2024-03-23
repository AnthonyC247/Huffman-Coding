#include "pq.h"
//format2
#include "node.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ListElement ListElement;
struct ListElement {
    Node *tree;
    ListElement *next;
};

struct PriorityQueue {
    ListElement *list;
};

PriorityQueue *pq_create(void) {
    PriorityQueue *p = calloc(1, sizeof(PriorityQueue));
    return p;
}

void pq_free(PriorityQueue **q) {
    free(*q);
    *q = NULL;
}

bool pq_is_empty(PriorityQueue *q) {
    return q->list == NULL;
}

bool pq_size_is_1(PriorityQueue *q) {
    return q->list != NULL && q->list->next == NULL;
}

// Comparison function for Nodes based on tree weight and symbol
bool pq_less_than(ListElement *e1, ListElement *e2) {
    // Comparing based on weight of the trees stored in ListElement
    if (e1->tree->weight < e2->tree->weight)
        return true;
    if (e1->tree->weight > e2->tree->weight)
        return false;
    // If weights are equal, compare based on symbol
    return e1->tree->symbol < e2->tree->symbol;
}

void enqueue(PriorityQueue *q, Node *tree) {
    ListElement *e = calloc(1, sizeof(ListElement));
    if (e != NULL) {
        e->tree = tree;
        if (pq_is_empty(q) || pq_less_than(e, q->list)) {
            e->next = q->list;
            q->list = e;
        } else {
            ListElement *current = q->list;
            ListElement *previous = NULL;
            while (current != NULL && pq_less_than(current, e)) {
                previous = current;
                current = current->next;
            }
            if (previous != NULL) {
                previous->next = e;
            } else {
                q->list = e;
            }
            e->next = current;
        }
    }
}

Node *dequeue(PriorityQueue *q) {
    if (pq_is_empty(q)) {
        fprintf(stderr, "Error: Queue is empty.\n");
        exit(EXIT_FAILURE);
    }

    ListElement *element = q->list;
    q->list = q->list->next;
    element->next = NULL;
    Node *dequeued_node = element->tree;
    free(element); // Free the dequeued list element
    return dequeued_node;
}

void pq_print(PriorityQueue *q) {
    assert(q != NULL);
    ListElement *e = q->list;
    int position = 1;
    while (e != NULL) {
        if (position++ == 1) {
            printf("=============================================\n");
        } else {
            printf("---------------------------------------------\n");
        }
        node_print_tree(e->tree); // Correct function call to node_print_tree
        e = e->next;
    }
    printf("=============================================\n");
}
