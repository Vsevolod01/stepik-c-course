#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "maybe_int.h"

struct list* node_create(int64_t value) {
    struct list* node = malloc(sizeof(struct list));
    node->value = value;
    node->next = NULL;
    return node;
}

void list_destroy( struct list* list ) {
    struct list* cur = list;
    if (cur != NULL) {
        list_destroy(cur->next);
        free(cur);
    }
}

void list_add_front(struct list** old, int64_t value) {
    struct list* new = node_create(value);
    new->next = *old;
    *old = new;
}

struct list* list_last(struct list* list) {
    struct list* cur = list;
    if (cur) {
        while (cur->next) cur = cur->next;
    }
    return cur;
}

void list_add_back( struct list** old, int64_t value ) {
    struct list* new = node_create(value);
    struct list* last = list_last(*old);
    if (last) last->next = new;
    else *old = new;
}

size_t list_length(const struct list* l) {
    size_t n = 0;
    while (l) {
        n++;
        l = l->next;
    }
    return n;
}

struct list* list_node_at( struct list* list, size_t idx ) {
    if (idx > list_length(list) || list == NULL) return NULL;
    struct list* cur = list;
    for (size_t i = 0; i < idx; i++) {
        cur = cur->next;
    }
    return cur;
}

struct maybe_int64 list_at( struct list* list, size_t idx ) {
    if (idx > list_length(list) || list == NULL) return none_int64;
    const struct list* cur = list;
    for (size_t i = 0; i < idx; i++) {
        cur = cur->next;
    }
    return some_int64(cur->value);
}

struct list* list_reverse( const struct list* list ) {
    struct list* new = NULL;
    const struct list* cur = list;
    for (size_t i = 0; i < list_length(list); i++) {
        list_add_front(&new, cur->value);
        cur = cur->next;
    }
    return new;
}

void list_print(const struct list* l) {
    while (l) {
        printf("%" PRId64 " ", l->value);
        l = l->next;
    }
}