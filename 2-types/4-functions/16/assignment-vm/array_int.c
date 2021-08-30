#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "io.h"
#include "maybe_int.h"
#include "array_int.h"

struct array_int array_int_create( size_t sz ) {
    int64_t* data = malloc( sizeof(int64_t) * sz);
    return (struct array_int) {.data = data, .size = sz};
}

int64_t* array_int_last( struct array_int a ) {
    size_t index = a.size - 1;
    if (index == -1) return NULL;
    return &(a.data[index]);
}

// возвращает ошибку если индекс за пределами массива
struct maybe_int64 array_int_get( struct array_int a, size_t i ) {
    if (i < a.size) return some_int64(a.data[i]);
    else return none_int64;
}

// возвращает false если индекс за пределами массива
bool array_int_set( struct array_int a, size_t i, int64_t value ) {
    if (i < a.size) {
        a.data[i] = value;
        return true;
    }
    else return false;
}

void array_int_print( struct array_int array ) {
    for (size_t i = 0; i < array.size; i = i + 1) {
        printf("%" PRId64 " " , array_int_get( array, i).value);
    }
}

void array_int_free( struct array_int* a ) {
    if (a) {
        if (a->size > 0) {
            free(a->data);
            a->size = 0;
        }
    }
}