#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "maybe_int.h"
#include "array_int.h"
#include "stack.h"
#include "mem.h"
#include "io.h"
#include "inttypes.h"

// --- interpreter-closed

size_t STACK_CAPACITY = 4096;

struct maybe_int64 maybe_read_int64() {
    int64_t x;
    int valid = scanf("%" SCNd64, &x);
    if (valid == 0 || valid == EOF) return none_int64;
    else return some_int64(x);
}

static int64_t i64_add(int64_t a, int64_t b) { return a + b; }
static int64_t i64_sub(int64_t a, int64_t b) { return a - b; }
static int64_t i64_mul(int64_t a, int64_t b) { return a * b; }
static int64_t i64_div(int64_t a, int64_t b) { return a / b; }
static int64_t i64_neg(int64_t a) { return -a; }
static int64_t i64_cmp(int64_t a, int64_t b) { if (a > b) return 1; else if (a < b) return -1; return 0; }

// --- начало решения на Stepik ---

// --- Интерпретатор ---


// Здесь перечислены все типы инструкций
enum opcode { 
    BC_PUSH, BC_POP, BC_SWAP, BC_DUP,
    BC_IADD, BC_ISUB, BC_IMUL, BC_IDIV, 
    BC_INEG,
    BC_IPRINT, BC_IREAD, 
    BC_ICMP, 
    BC_STOP 
};

// Инструкция union ins в одном из форматов
struct bc_noarg { 
    enum opcode opcode;
};
struct bc_arg64 { 
    enum opcode opcode;
    int64_t arg; 
};
union ins {
    enum opcode opcode;
    struct bc_arg64 as_arg64; 
    struct bc_noarg as_noarg;
};

// Теперь опишем инструкции языка
// Все инструкции попадают в формат "нет аргументов" или "один аргумент"
enum ins_format { IAT_NOARG, IAT_I64 };

// Эта структура позволяет декларативно описать инструкцию
struct ins_descr { const char* mnemonic; enum ins_format format; };

// Индекс в этом массиве соответствует типу инструкции
// Каждая структура описывает мнемонику инструкции (текстовое представление)
// и её тип
static const struct ins_descr instructions[] = {
  
  /* Добавьте описания PUSH и IADD */
    
  [BC_ISUB]   = { "isub",  IAT_NOARG },
  [BC_IMUL]   = { "imul",  IAT_NOARG },
  [BC_IDIV]   = { "idiv",  IAT_NOARG },
  [BC_INEG]   = { "ineg",  IAT_NOARG },
  [BC_IPRINT] = { "iprint",IAT_NOARG },
  [BC_IREAD]  = { "iread", IAT_NOARG },
  [BC_SWAP]   = { "swap",  IAT_NOARG },
  [BC_POP]    = { "pop",   IAT_NOARG },
  [BC_DUP]    = { "dup",   IAT_NOARG },
  [BC_ICMP]   = { "icmp",  IAT_NOARG },
  [BC_STOP]   = { "stop",  IAT_NOARG }
};


// Чтобы интерпретировать программу
// заведём структуру "состояние программы"
// Содержит указатель на текущую инструкцию и стек чисел
struct vm_state { 
    const union ins* ip; 
    struct stack data_stack; 
};

// Создаёт начальное состояние программы
// Вместимость стека задаётся препроцессорным определением STACK_CAPACITY
struct vm_state
state_create( union ins* ip ) {
    return (struct vm_state) { .ip = ip, .data_stack = stack_create(STACK_CAPACITY) };
}
void
state_destroy( struct vm_state * state ) {
    stack_destroy(&state->data_stack);
}

// Эти функции поднимают операции над числами на уровень стековых операций
// lift_unop применяет операцию к вершине стека;
void lift_unop( struct stack* s, int64_t (f)(int64_t))
{
    struct maybe_int64 a = stack_pop(s);
    if (a.valid) {
        int64_t result = f(a.value);
        stack_push(s, result);
    }
}
// lift_binop забирает из стека два аргумента,
// применяет к ним функцию от двух переменных и возвращает в стек результат
// Это позволяет единообразно реализовать команды IADD, ISUB, IMUL, ICMP и др.
void lift_binop( struct stack* s, int64_t (f)(int64_t, int64_t))
{
    struct maybe_int64 a = stack_pop(s);
    struct maybe_int64 b = stack_pop(s);
    if (a.valid && b.valid) {
        int64_t result = f(b.value, a.value);
        stack_push(s, result);
    }
}

// Интерпретатор каждой инструкции это функция над состоянием машины
// Определите тип такой функции по имени ins_interpreter

typedef void ins_interpreter(struct vm_state*);

void interpret(struct vm_state* state, ins_interpreter * const  (actions)[]) {
  for (;;) {
    actions[state->ip->opcode](state);
    // Если команда записывает NULL в state->ip, надо остановиться
    if (!state->ip) break;
    state->ip = state->ip + 1;
  }
}
void vm_stop( struct vm_state* state ) { state->ip = NULL; }

void interpret_program( union ins* program, ins_interpreter * const  (actions)[]) {
  struct vm_state state = state_create( program );
  interpret( &state, actions );
  state_destroy( &state );
}

/*  Интерпретаторы команд */
void interpret_push  ( struct vm_state* state ) {
    stack_push(&state->data_stack, state->ip->as_arg64.arg);
}

void interpret_iprint( struct vm_state* state ) {
    for (size_t i = 0; i < state->data_stack.count; i++) {
        print_int64(state->data_stack.data.data[i]);
        printf(" ");
    }
}
void interpret_iread ( struct vm_state* state ) {
    struct maybe_int64 a = maybe_read_int64();
    stack_push(&state->data_stack, a.value);
}

void interpret_swap  ( struct vm_state* state )  {
    struct maybe_int64 a = stack_pop(&state->data_stack);
    struct maybe_int64 b = stack_pop(&state->data_stack);
    if (a.valid && b.valid) {
        stack_push(&state->data_stack, a.value);
        stack_push(&state->data_stack, b.value);
    }
    else {
        err("SWAP doesn't work");
    }
}

void interpret_pop   ( struct vm_state* state )  {
    stack_pop(&state->data_stack);
}
void interpret_dup   ( struct vm_state* state )  {
    struct maybe_int64 a = stack_pop(&state->data_stack);
    if (a.valid) {
        stack_push(&state->data_stack, a.value);
        stack_push(&state->data_stack, a.value);
    }
    else {
        err("DUP doesn't work");
    }
}

void interpret_stop  ( struct vm_state* state )  { vm_stop( state );  }

/*  Чтобы заработали эти функции необходимо реализовать lift_binop и lift_unop */

void interpret_iadd  ( struct vm_state* state );
/*{
     lift_binop(& state->data_stack, i64_add );
  }
При этом i64_add выглядит так:
    
int64_t i64_add( int64_t a, int64_t b ) { return a + b; }

  Следующиe функции реализованы аналогично 
  (и их не нужно реализовывать, достаточно lift_binop):
*/
void interpret_isub  ( struct vm_state* state );
void interpret_imul  ( struct vm_state* state );
void interpret_idiv  ( struct vm_state* state );
void interpret_ineg  ( struct vm_state* state );
void interpret_icmp  ( struct vm_state* state );
/* {
   lift_binop(& state->data_stack, i64_cmp);
    
   } */


/* Массив указателей на функции; индекс = номер инструкции */
ins_interpreter* const ins_interpreters[] = {
  [BC_PUSH]   = interpret_push,
  [BC_IADD]   = interpret_iadd,
  [BC_ISUB]   = interpret_isub,
  [BC_IMUL]   = interpret_imul,
  [BC_IDIV]   = interpret_idiv,
  [BC_INEG]   = interpret_ineg,
  [BC_IPRINT] = interpret_iprint,
  [BC_IREAD]  = interpret_iread,
  [BC_SWAP]   = interpret_swap,
  [BC_POP]    = interpret_pop,
  [BC_DUP]    = interpret_dup,
  [BC_ICMP]   = interpret_icmp,
  [BC_STOP]   = interpret_stop
};

/* С помощью interpret можно также напечатать программу на экран
   Принтер работает так же как интерпретатор,
  только смысл инструкций становится другим: каждая инструкция
  просто себя печатает.
  Реализуйте массив ins_printers, который можно передавать в функцию
  interpret_program.
  Подсказка: да, потребуются ещё функции
*/

/*  Принтеры команд */
void print_push  ( struct vm_state* state ) {
    printf("push ");
    print_int64(state->ip->as_arg64.arg);
    print_newline();
}

void print_iprint( struct vm_state* state ) {
    printf("iprint");
    print_newline();
}
void print_iread ( struct vm_state* state ) {
    printf("iread");
    print_newline();
}

void print_swap  ( struct vm_state* state )  {
    printf("swap");
    print_newline();
}

void print_pop   ( struct vm_state* state )  {
    printf("pop");
    print_newline();
}
void print_dup   ( struct vm_state* state )  {
    printf("dup");
    print_newline();
}

void print_stop  ( struct vm_state* state )  {
    printf("stop");
    print_newline();
    vm_stop(state);
}

/*  Чтобы заработали эти функции необходимо реализовать lift_binop и lift_unop */

void print_iadd  ( struct vm_state* state );
void print_isub  ( struct vm_state* state );
void print_imul  ( struct vm_state* state );
void print_idiv  ( struct vm_state* state );
void print_ineg  ( struct vm_state* state );
void print_icmp  ( struct vm_state* state );

static ins_interpreter* const ins_printers[] = {
        [BC_PUSH]   = print_push,
        [BC_IADD]   = print_iadd,
        [BC_ISUB]   = print_isub,
        [BC_IMUL]   = print_imul,
        [BC_IDIV]   = print_idiv,
        [BC_INEG]   = print_ineg,
        [BC_IPRINT] = print_iprint,
        [BC_IREAD]  = print_iread,
        [BC_SWAP]   = print_swap,
        [BC_POP]    = print_pop,
        [BC_DUP]    = print_dup,
        [BC_ICMP]   = print_icmp,
        [BC_STOP]   = print_stop
};

// --- конец решения на Stepik ---

void test_stack_capacity() {
  STACK_CAPACITY = 1;
  struct vm_state state = state_create(NULL);
  stack_push( & state.data_stack , 42 );
  if (stack_push( & state.data_stack , 42 ) ) {
    err( "Stack capacity for initial state is incorrect");
  }

  state_destroy( & state );
  STACK_CAPACITY = 4096;
}


void interpret_iadd  ( struct vm_state* state )  {lift_binop(& state->data_stack, i64_add); }
void interpret_isub  ( struct vm_state* state )  {lift_binop(& state->data_stack, i64_sub); }
void interpret_imul  ( struct vm_state* state )  {lift_binop(& state->data_stack, i64_mul); }
void interpret_idiv  ( struct vm_state* state )  {lift_binop(& state->data_stack, i64_div); }
void interpret_ineg  ( struct vm_state* state )  {lift_unop(& state->data_stack, i64_neg); }
void interpret_icmp  ( struct vm_state* state )  { lift_binop(& state->data_stack, i64_cmp); }

void print_iadd  ( struct vm_state* state )  {printf("iadd\n"); }
void print_isub  ( struct vm_state* state )  {printf("isub\n"); }
void print_imul  ( struct vm_state* state )  {printf("imul\n"); }
void print_idiv  ( struct vm_state* state )  {printf("idiv\n"); }
void print_ineg  ( struct vm_state* state )  {printf("ineg\n"); }
void print_icmp  ( struct vm_state* state )  { printf("icmp\n"); }

#undef int

union ins test_programs[][100] = {
  { // 8 98 -> 988
    { BC_IREAD },
    { BC_IREAD },
    { BC_DUP },
    { .as_arg64 = { BC_PUSH, .arg = 10 } },
    { BC_IMUL },
    { BC_IADD },
    { BC_IADD },
    { BC_IPRINT },
    { BC_STOP }
  },
  { 
    { BC_IREAD },
    { .as_arg64 = { BC_PUSH, .arg = 10 } },
    { BC_ISUB },
    { .as_arg64 = { BC_PUSH, .arg = 2 } },
    { BC_IDIV },
    { BC_IPRINT },
    { BC_STOP }
  },
  {
    { BC_IREAD },
    { BC_IREAD },
    { BC_POP },
    { BC_INEG },
    { BC_IPRINT },
    { BC_STOP }
  },
  {
    { BC_IREAD },
    { BC_IREAD },
    { BC_SWAP },
    { BC_IPRINT },
    { BC_IPRINT },
    { BC_STOP }
  },
  {
    { BC_IREAD },
    { BC_IREAD },
    { BC_ICMP },
    { BC_IPRINT },
    { BC_STOP }
  },
};

int main() {
  test_stack_capacity();

  const size_t i = read_size();
  interpret_program( test_programs[i], ins_printers );
  print_newline();
  interpret_program( test_programs[i], ins_interpreters);

  return 0;
}

