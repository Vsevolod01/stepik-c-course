
#include <stdio.h>
#include <inttypes.h>
#include "io.h"
#include "stack.h"

// Количество элементов в стеке
size_t stack_count( const struct stack* s ) {
    return s->count;
}

// Создаем и деинициализируем стек
struct stack stack_create( size_t size ) {
    struct array_int data = array_int_create(size);
    return (struct stack) {.count = 0, .data = data};
}

void stack_destroy( struct stack* s ) {
    array_int_free(&s->data);
    s->count = 0;
}

// Стек полный
bool stack_full( struct stack s) {
    return s.count == s.data.size;
}

// Стек пустой
bool stack_empty( struct stack s) {
    return s.count == 0;
}

// Поместить значение в стек
bool stack_push( struct stack* s, int64_t value ) {
    if (!stack_full(*s)) {
        array_int_set(s->data, s->count, value);
        s->count++;
        return true;
    } else return false;
}

// Вынуть значение с вершины стека. Может вернуть none_int64
struct maybe_int64 stack_pop( struct stack* s ) {
    if (!stack_empty(*s)) {
        s->count--;
        return array_int_get(s->data, s->count);
    } else {
        return none_int64;
    }
}

void stack_print( struct stack s ) {
    for (size_t i = 0; i < stack_count(&s); i++) {
        print_int64(array_int_get(s.data, i).value);
        printf(" ");
    }
}