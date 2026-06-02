#ifndef VM_H
#define VM_H

#include "compiler.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_SIZE (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
    Value items[STACK_SIZE];
    size_t count;
} Stack;

typedef struct
{
    ObjFunction *function;
    uint8_t *ip;
    Value *slots;
} CallFrame;

struct VM
{
    Stack stack;
    Value *sp;
    int row;
    int col;
    char *message;
    Table *strings;
    Table *globals;
    Table *variables;
    char *file_path;
    CallFrame frames[FRAMES_MAX];
    int frame_count;
};

typedef enum
{
    VM_OK,
    VM_TYPE_ERROR,
    VM_ILLEGAL_INSTRUCTION,
    VM_STACK_OVERFLOW,
    VM_STACK_UNDERFLOW,
    VM_REFERENCE_ERROR,
    VM_TOO_FEW_ARGUMENTS,
    VM_TOO_MANY_ARGUMENTS,
} VM_Error;
typedef struct VM VM;

VM *init_vm(Compiler *compiler);
void vm_free(VM *vm);
VM_Error vm_interpret(VM *vm);
void vm_print_stack_trace(VM *vm);
void vm_print_error(VM *vm, VM_Error error);
#endif