#ifndef CHUNK_H
#define CHUNK_H

#include "value.h"

typedef uint8_t byte;

typedef struct
{
    byte *items;
    size_t count;
    size_t capacity;
} Chunk;

typedef enum
{
    OP_CONSTANT,
    OP_NEGATE,
    OP_RETURN,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,
    OP_NOT,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_NOT,
    OP_PRINT,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_POP,
    OP_GET_LOCAL,
    OP_JMP_IF_FALSE,
    OP_JMP,
    OP_LOOP,
    OP_LEFT_SHIFT,
    OP_RIGHT_SHIFT,
    OP_DUP,
    OP_CALL,
} OpCode;

typedef enum
{
    OPERAND_IMMEDIATE,
    OPERAND_MEMORY,
} OperandType;

char *chunk_operand_type_to_str();
Chunk *init_chunk();
size_t chunk_push(Chunk *chunk, byte insturction); /*returns index of the instruction */
byte chunk_pop(Chunk *chunk);
const char *chunk_byte_to_str(byte insturction);
void chunk_dump(Chunk *chunk, FILE *stream);
void chunk_free(Chunk *chunk);
size_t chunk_print_instruction(Chunk *chunk, size_t offset, FILE *stream);
#endif