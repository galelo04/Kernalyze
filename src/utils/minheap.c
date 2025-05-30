#include "minheap.h"

#include <stdio.h>

// Function to get the priority of a Heap node
int get_node_priority(void* node_ptr) {
    if (node_ptr == NULL) {
        return 0;  // Default priority for NULL nodes
    }
    return ((struct heap_node*)node_ptr)->priority;
}

// Define a createHeap function
struct Heap* heap_create() {
    // Allocating memory to Heap h
    struct Heap* h = (struct Heap*)malloc(sizeof(struct Heap));

    // Checking if memory is allocated to h or not
    if (h == NULL) {
        printf("Memory error\n");
        return NULL;
    }

    // set the initial size to 0
    h->size = 0;

    // Allocating memory to vector
    h->vector = vector_create(10);

    // Checking if memory is allocated to vector or not
    if (h->vector == NULL) {
        printf("Memory error\n");
        free(h);
        return NULL;
    }

    return h;
}

// Defining insertHelper function (bubble up)
void insertHelper(struct Heap* h, int index) {
    if (index <= 0) {
        return;  // Base case: reached the root
    }

    // Store parent of element at index
    int parent = (index - 1) / 2;

    // Get node pointers from the vector
    struct heap_node* current_node = (struct heap_node*)vector_get(h->vector, index);
    struct heap_node* parent_node = (struct heap_node*)vector_get(h->vector, parent);

    if (parent_node == NULL || current_node == NULL) {
        return;  // Safety check
    }

    // If current node's priority is less than parent's priority, swap them
    if (current_node->priority < parent_node->priority) {
        // Swap the nodes in the vector
        vector_set(h->vector, current_node, parent);
        vector_set(h->vector, parent_node, index);

        // Recursively call insertHelper on the parent index
        insertHelper(h, parent);
    }
}

// Heapify function (bubble down)
void heapify(struct Heap* h, int index) {
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    int smallest = index;

    // Check if left or right children exist and are smaller than current node
    if (left < h->size) {
        struct heap_node* left_node = (struct heap_node*)vector_get(h->vector, left);
        struct heap_node* current_node = (struct heap_node*)vector_get(h->vector, smallest);

        if (left_node != NULL && current_node != NULL &&
            left_node->priority < current_node->priority) {
            smallest = left;
        }
    }

    if (right < h->size) {
        struct heap_node* right_node = (struct heap_node*)vector_get(h->vector, right);
        struct heap_node* smallest_node = (struct heap_node*)vector_get(h->vector, smallest);

        if (right_node != NULL && smallest_node != NULL &&
            right_node->priority < smallest_node->priority) {
            smallest = right;
        }
    }

    // If smallest is not the current index, swap and continue heapifying
    if (smallest != index) {
        // Get node pointers
        struct heap_node* smallest_node = (struct heap_node*)vector_get(h->vector, smallest);
        struct heap_node* current_node = (struct heap_node*)vector_get(h->vector, index);

        // Swap the nodes
        vector_set(h->vector, smallest_node, index);
        vector_set(h->vector, current_node, smallest);

        // Recursively call heapify on the smallest index
        heapify(h, smallest);
    }
}

// Extract the minimum element from the Heap
int heap_extract_min(struct Heap* h, void** data, int* priority) {
    // Checking if the Heap is empty
    if (h->size == 0) {
        printf("\nHeap is empty.\n");
        return 0;
    }

    if (data == NULL) {
        printf("Data pointer is NULL.\n");
        return 0;
    }

    // Get the minimum node (root of the Heap)
    struct heap_node* min_node = (struct heap_node*)vector_get(h->vector, 0);
    if (priority != NULL) {
        *priority = min_node->priority;
    }
    *data = min_node->data;

    // Replace the root with the last element
    struct heap_node* last_node = (struct heap_node*)vector_get(h->vector, h->size - 1);
    vector_set(h->vector, last_node, 0);

    // Decrement the size
    h->size--;
    h->vector->size = h->size;  // Keep vector size synchronized

    // Restore Heap property
    if (h->size > 0) heapify(h, 0);

    // Free the minimum node
    free(min_node);

    return 1;
}

// Insert a new element into the Heap
void heap_insert(struct Heap* h, void* data, int priority) {
    // Create a new Heap node
    struct heap_node* new_node = (struct heap_node*)malloc(sizeof(struct heap_node));
    if (new_node == NULL) {
        printf("Memory allocation error\n");
        return;
    }

    // Set the node's data and priority
    new_node->data = data;
    new_node->priority = priority;

    // Add the new node to the vector
    vector_push(h->vector, new_node);

    // Update Heap size
    h->size = h->vector->size;

    // Maintain Heap property by bubbling up the new node
    insertHelper(h, h->size - 1);
}

// Function to check if the Heap is empty
int heap_is_empty(const struct Heap* h) {
    if (h == NULL) return 1;

    return h->size == 0;
}

// Print the Heap's priorities (for debugging)
void heap_print(const struct Heap* h) {
    printf("Heap contents (priorities): ");
    for (int i = 0; i < h->size; i++) {
        struct heap_node* node = (struct heap_node*)vector_get(h->vector, i);
        if (node != NULL) {
            printf("%d ", node->priority);
        } else {
            printf("NULL ");
        }
    }
    printf("\n");
}

// Function to free the Heap and all its nodes
void heap_destroy(struct Heap* h) {
    if (h == NULL) {
        return;
    }

    // Free all Heap nodes
    for (int i = 0; i < h->vector->size; i++) {
        struct heap_node* node = (struct heap_node*)vector_get(h->vector, i);
        if (node != NULL) {
            free(node);
        }
    }

    // Free the vector
    vector_free(h->vector);

    // Free the Heap structure
    free(h);
}