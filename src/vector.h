#ifndef IHCT_VECTOR_H
#define IHCT_VECTOR_H

#include <stdlib.h>

// Datatype representing a vector. to be used internally in IHCT_RUN
typedef struct {
    void **data;
    size_t size;
} ihct_vector;

// Allocates a new vector with capacity cap.
ihct_vector *ihct_vector_init();

// Add a pointer to a allocated object at the end of the vector.
void ihct_vector_add(ihct_vector *v, void *obj);

// Gets the object at location index in vector v.
void *ihct_vector_get(ihct_vector *v, int index);

// Deallocates the vector.
void ihct_vector_free(ihct_vector *v);

#endif