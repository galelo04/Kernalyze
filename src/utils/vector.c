#include "vector.h"

// Function to create a new vector with initial capacity
Vector* vector_create(int initial_capacity) {
    Vector* vec = malloc(sizeof(Vector));
    if (vec == NULL) {
        return NULL;
    }

    vec->data = malloc(sizeof(void*) * initial_capacity);
    if (vec->data == NULL) {
        free(vec);
        return NULL;
    }

    vec->size = 0;
    vec->capacity = initial_capacity;
    return vec;
}

// Function to push an element to the vector, resizing if necessary
void vector_push(Vector* vec, void* element) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        void** new_data = realloc(vec->data, sizeof(void*) * vec->capacity);
        if (new_data == NULL) {
            return;  // Handle realloc failure
        }
        vec->data = new_data;
    }
    vec->data[vec->size] = element;
    vec->size++;
}

// Function to pop an element from the vector
void* vector_pop(Vector* vec) {
    if (vec->size > 0) {
        void* element = vec->data[vec->size - 1];
        vec->size--;
        return element;
    }

    return NULL;
}

// Function to get an element at a specific index (void pointer for generality)
void* vector_get(Vector* vec, int index) {
    if (index >= 0 && index < vec->size) {
        return vec->data[index];
    }
    return NULL;  // Out of bounds
}

// Function to set an element at a specific index
void vector_set(Vector* vec, void* element, int index) {
    if (index >= 0 && index < vec->size) {
        vec->data[index] = element;
    }
}

// Function to add an element at a specific index
void vector_insert(Vector* vec, void* element, int index) {
    if (index >= 0 && index <= vec->size) {
        if (vec->size == vec->capacity) {
            vec->capacity *= 2;
            void** new_data = realloc(vec->data, sizeof(void*) * vec->capacity);
            if (new_data == NULL) {
                return;  // Handle realloc failure
            }
            vec->data = new_data;
        }

        // Shift elements to make room for the new element
        for (int i = vec->size; i > index; i--) {
            vec->data[i] = vec->data[i - 1];
        }

        vec->data[index] = element;
        vec->size++;
    }
}

// Function to remove an element at a specific index
void vector_remove(Vector* vec, int index) {
    if (index >= 0 && index < vec->size) {
        // Shift elements to fill the gap
        for (int i = index; i < vec->size - 1; i++) {
            vec->data[i] = vec->data[i + 1];
        }
        vec->size--;
    }
}

// Function to free the memory used by the vector
void vector_free(Vector* vec) {
    if (vec != NULL) {
        free(vec->data);
        free(vec);
    }
}