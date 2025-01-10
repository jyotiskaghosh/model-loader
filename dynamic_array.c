#include <stdlib.h>
#include <string.h>

#include "dynamic_array.h"

Array initArray(size_t initialSize, size_t elementSize)
{
    Array a;
   
    a.array = (void *)malloc(initialSize * elementSize);
    a.length = 0;
    a.maxLength = initialSize;
    a.elementSize = elementSize;

    return a;
}

void append(Array *a, void *element)
{
    if (a->length == a->maxLength) {
      a->maxLength *= 2;
      a->array = (void *)realloc(a->array, a->maxLength * a->elementSize);
    }

    memcpy((a->array + a->length++ * a->elementSize), element, a->elementSize);
}

void *getElement(Array a, size_t i)
{
    return (a.array + i * a.elementSize);
}

void freeArray(Array *a)
{
    free(a->array);
    a->array = NULL;
    a->length = a->maxLength = 0;
}
