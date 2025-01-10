#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

#include <string.h>

// Dynamic array
// ..............
typedef struct {
    void *array;
    size_t length;
    size_t maxLength;
    size_t elementSize;
} Array;

Array initArray(size_t initialSize, size_t elementSize);
void append(Array *a, void *element);
void *getElement(Array a, size_t i);
void freeArray(Array *a);

#endif