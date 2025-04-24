#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

typedef struct {
    void** data;   // Pointer to array of elements (void pointers for flexibility)
    int size;      // Current number of elements
    int capacity;  // Current capacity of the vector
} Vector;

// Function to create a new vector with initial capacity
Vector* vector_create(int initial_capacity);

// Function to push an element to the vector, resizing if necessary
void vector_push(Vector* vec, void* element);

// Function to pop an element from the vector
void* vector_pop(Vector* vec);

// Function to get an element at a specific index (void pointer for generality)
void* vector_get(Vector* vec, int index);

// Function to set an element at a specific index
void vector_set(Vector* vec, void* element, int index);

// Function to add an element at a specific index
void vector_insert(Vector* vec, void* element, int index);

// Function to remove an element at a specific index
void vector_remove(Vector* vec, int index);

// Function to free the memory used by the vector
void vector_free(Vector* vec);

#endif