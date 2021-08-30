#include <stdio.h>
#include <inttypes.h>

#include "maybe_int.h"

// Один из двух случаев:
// - valid = true и value содержит осмысленный результат
// - valid = false и value может быть любым

// Первый случай; создаем функцию в помощь
// Не бойтесь за производительность
struct maybe_int64 some_int64( int64_t i ) {
    return (struct maybe_int64) { .value = i, .valid = true };
}

// Второй случай; можно создать не функцию,
// а константный экземпляр структуры
// Все поля инициализированы нулями
// .value = 0, .valid = false
const struct maybe_int64 none_int64 = { 0 };

void maybe_int64_print( struct maybe_int64 i ) {
    if (i.valid) {
        printf("Some %" PRId64, i.value);
    }
    else {
        printf("None");
    }
}