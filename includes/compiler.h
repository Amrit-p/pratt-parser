#ifndef COMPILER_H
#define COMPILER_H

#include "object.h"
#include "token.h"
#include "table.h"

#define UINT8_COUNT (UINT8_MAX + 1)

typedef struct
{
    Token name;
    int depth;
} Local;

typedef enum
{
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

typedef struct
{
    ObjFunction *function;
    FunctionType type;
    bool had_error;
    char *file_path;
    int scope_depth;
    int local_count;
    Local locals[UINT8_COUNT];
    uint8_t break_stack[UINT8_COUNT];
    uint8_t break_stack_count;
    uint8_t continue_stack[UINT8_COUNT];
    uint8_t continue_stack_count;
    Table *strings;
    Table *globals;
    Table *variables;
} Compiler;

void init_compiler(Compiler *compiler, FunctionType type);
ObjFunction *compiler_end(Compiler *);
int compiler_resolve_local(Compiler *compiler, Token token);
void compiler_dump(Compiler *compiler, FILE *stream);
void compiler_free(Compiler *compiler);
#endif