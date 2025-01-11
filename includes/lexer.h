#ifndef LEXER_H
#define LEXER_H
#include "token.h"
#include <stddef.h>

typedef struct
{
    Token token;
    char current_char;
    size_t index;
    size_t row;
    size_t col;
    size_t src_size;
    char *src;
    char *file_path;
} Lexer;

Lexer *init_lexer(char *source, char *path);
Token lexer_next_token(Lexer *lexer);
Token lexer_advance_with(Lexer *lexer, Token token);
void lexer_skip_space(Lexer *lexer);
void lexer_advance(Lexer *lexer);
Token lexer_parse_id(Lexer *lexer);
Token lexer_parse_number(Lexer *lexer);
void lexer_free(Lexer *lexer);
#endif